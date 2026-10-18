/*
 * UTNSubModuleShieldTrigger.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdio>
#include <cstring>
#include <functional>
#include <new>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
using namespace std;

// ROOT:
#include "TSystem.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MTime.h"

// Nuclearizer:
#include "MSubModuleShieldTrigger.h"
#include "MReadOutAssembly.h"
#include "MDEECrystalHit.h"


//! Unit test class for MSubModuleShieldTrigger
class UTNSubModuleShieldTrigger : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleShieldTrigger() : MUnitTest("UTNSubModuleShieldTrigger") {}
  //! Default destructor
  virtual ~UTNSubModuleShieldTrigger() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the defaults, the setters/getters and the XML configuration
  bool TestDefaults();
  //! Test the Initialize() error paths
  bool TestInitialize();
  //! Test the veto after a shield hit
  bool TestVeto();
  //! Test the shield dead time
  bool TestDeadTime();
  //! Test the shield detector to panel assignment
  bool TestPanels();
  //! Test the behavior without dead time file
  bool TestWithoutDeadtimeFile();

  //! Write the dead time file and return its name
  MString WriteDeadtimeFile();
  //! Initialize a module with the standard file
  bool InitializeStandard(MSubModuleShieldTrigger& M);
  //! Analyze an event at the given time with an optional shield hit
  void Analyze(MSubModuleShieldTrigger& M, double Time, const MString& DetectorID = "", int CrystalID = 0, double Energy = 500.0);
  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldTrigger::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestVeto() && Passed;
  Passed = TestDeadTime() && Passed;
  Passed = TestPanels() && Passed;
  Passed = TestWithoutDeadtimeFile() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleShieldTrigger::WriteDeadtimeFile()
{
  // Shield: 1 us per channel, 5 us pulse duration, 1 us before, 2 us after, 0.5 us veto window extension
  MString C;
  C += "# strip: window, per channel, delay 1, delay 2, fast clear\n";
  C += "1e-6 2e-6 3e-6 4e-6 5e-6\n";
  C += "# shield: per channel, pulse duration, delay before, delay after, veto window delta\n";
  C += "1e-6 5e-6 1e-6 2e-6 0.5e-6\n";
  MString FileName = GetTemporaryFileName("shield.deadtime.txt");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldTrigger::InitializeStandard(MSubModuleShieldTrigger& M)
{
  MString FileName = WriteDeadtimeFile();
  if (FileName == "") return false;
  M.SetDeadtimeFileName(FileName);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Result = M.Initialize();
  g_Verbosity = OldVerbosity;
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


void UTNSubModuleShieldTrigger::Analyze(MSubModuleShieldTrigger& M, double Time, const MString& DetectorID, int CrystalID, double Energy)
{
  MReadOutAssembly E;
  E.SetTimeUTC(MTime(Time));
  if (DetectorID != "") {
    MDEECrystalHit C;
    C.m_DetectorID = DetectorID;
    C.m_CrystalID = CrystalID;
    C.m_Energy = Energy;
    C.m_ROE.SetDetectorID(DetectorID);
    E.AddDEECrystalHit(C);
  }
  M.Clear();
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  M.AnalyzeEvent(&E);
  g_Verbosity = OldVerbosity;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldTrigger::TestDefaults()
{
  bool Passed = true;

  MSubModuleShieldTrigger M;
  Passed = EvaluateFalse("HasTrigger()", "default", "No trigger by default", M.HasTrigger()) && Passed;
  Passed = EvaluateFalse("HasShieldVeto()", "default", "No veto by default", M.HasShieldVeto()) && Passed;
  Passed = EvaluateFalse("IsShieldDead()", "default", "The shield is not dead by default", M.IsShieldDead()) && Passed;
  Passed = Evaluate("GetShieldHitCounts()", "default", "No shield hits", M.GetShieldHitCounts(), 0) && Passed;
  Passed = Evaluate("GetBGOHitsErased()", "default", "No erased hits", M.GetBGOHitsErased(), 0) && Passed;
  Passed = EvaluateNear("GetTotalShieldDeadtime()", "panel 0", "No dead time", M.GetTotalShieldDeadtime(0), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetTotalShieldDeadtime()", "panel 6", "An invalid panel returns 0", M.GetTotalShieldDeadtime(6), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetShieldDeadTimeEnd()", "default", "The dead time end is 0", M.GetShieldDeadTimeEnd().GetAsDouble(), 0.0, 1e-15) && Passed;
  Passed = Evaluate("GetDeadtimeFileName()", "default", "The default file name is empty", M.GetDeadtimeFileName(), MString("")) && Passed;
  M.SetDeadtimeFileName("dt.txt");
  Passed = Evaluate("SetDeadtimeFileName/GetDeadtimeFileName", "dt.txt", "Returns the set name", M.GetDeadtimeFileName(), MString("dt.txt")) && Passed;

  // The dead time file is stored by MModuleDEESMEX, the sub module adds nothing
  MXmlNode* Node = new MXmlNode(nullptr, "DEE");
  Passed = EvaluateTrue("CreateXmlConfiguration()", "mother node", "The mother node is returned", M.CreateXmlConfiguration(Node) == Node) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "mother node", "ReadXmlConfiguration returns true", M.ReadXmlConfiguration(Node)) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldTrigger::TestInitialize()
{
  bool Passed = true;

  MString Short = GetTemporaryFileName("short.txt");
  bool ShortWritten = WriteTextFile(Short, "# strip\n1 2 3 4 5\n");
  MString Wrong = GetTemporaryFileName("wrong.txt");
  bool WrongWritten = WriteTextFile(Wrong, "# strip\n1 2 3 4 5\n# shield\n1 2 3\n");
  Passed = EvaluateTrue("Initialize()", "file without shield rows", "The fixture file can be written", ShortWritten) && Passed;
  Passed = EvaluateTrue("Initialize()", "shield row with 3 values", "The fixture file can be written", WrongWritten) && Passed;

  // MParser reports the missing file via merr
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  MSubModuleShieldTrigger Missing;
  Missing.SetDeadtimeFileName(GetTemporaryFileName("missing.txt"));
  bool MissingResult = Missing.Initialize();
  MSubModuleShieldTrigger ShortFile;
  ShortFile.SetDeadtimeFileName(Short);
  bool ShortResult = ShortFile.Initialize();
  MSubModuleShieldTrigger WrongFile;
  WrongFile.SetDeadtimeFileName(Wrong);
  bool WrongResult = WrongFile.Initialize();
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "missing file", "Initialize returns false", MissingResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "file without shield rows", "Initialize returns false", ShortResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "shield row with 3 values", "Initialize returns false", WrongResult) && Passed;

  MSubModuleShieldTrigger Good;
  Passed = EvaluateTrue("Initialize()", "valid file", "Initialize succeeds", InitializeStandard(Good)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldTrigger::TestVeto()
{
  bool Passed = true;

  MSubModuleShieldTrigger M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "valid file", "Initialize succeeds", false);
  }

  // Shield hit at 100 s in X0 crystal 3: dead time 1 + 2 * 1 + 2 = 5 us (equal to the pulse duration)
  Analyze(M, 100.0, "X0", 3);
  Passed = ExpectedToFail(EvaluateTrue("HasTrigger()", "shield hit", "A shield hit above threshold raises a shield trigger", M.HasTrigger())) && Passed;
  Passed = EvaluateTrue("HasShieldVeto()", "shield hit", "The shield hit vetoes its own event", M.HasShieldVeto()) && Passed;
  Passed = EvaluateNear("GetShieldVetoTime()", "shield hit", "The veto time is the shield hit time", M.GetShieldVetoTime().GetAsDouble(), 100.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetShieldDeadTimeEnd()", "shield hit", "The shield dead time ends 5 us after the hit", M.GetShieldDeadTimeEnd().GetAsDouble(), 100.0 + 5e-6, 2e-9) && Passed;
  Passed = Evaluate("GetShieldHitCounts()", "shield hit", "One shield hit is counted", M.GetShieldHitCounts(), 1) && Passed;

  // GeD event 3 us later: within dead time + veto window extension
  Analyze(M, 100.000003);
  Passed = EvaluateTrue("HasShieldVeto()", "GeD event 3 us after the shield hit", "The event is vetoed", M.HasShieldVeto()) && Passed;
  Passed = EvaluateFalse("HasTrigger()", "GeD event 3 us after the shield hit", "There is no shield trigger without shield hit", M.HasTrigger()) && Passed;

  // GeD event 5.2 us later: still within the 0.5 us veto window extension
  Analyze(M, 100.0000052);
  Passed = EvaluateTrue("HasShieldVeto()", "GeD event 5.2 us after the shield hit", "The event is vetoed by the extended veto window", M.HasShieldVeto()) && Passed;

  // GeD event 10 us later: no veto
  Analyze(M, 100.00001);
  Passed = EvaluateFalse("HasShieldVeto()", "GeD event 10 us after the shield hit", "The event is not vetoed", M.HasShieldVeto()) && Passed;

  // Shield hit below the threshold does not create a trigger
  MSubModuleShieldTrigger T;
  Passed = EvaluateTrue("Initialize()", "valid file, below threshold test", "Initialize succeeds", InitializeStandard(T)) && Passed;
  Analyze(T, 200.0, "X0", 3, -5.0);
  Passed = EvaluateFalse("HasShieldVeto()", "shield hit below threshold", "A hit below threshold does not veto", T.HasShieldVeto()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldTrigger::TestDeadTime()
{
  bool Passed = true;

  MSubModuleShieldTrigger M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "valid file", "Initialize succeeds", false);
  }

  Analyze(M, 100.0, "X0", 3);
  // Second crystal within the 1 us coincidence: 4 channels -> 1 + 4 + 2 = 7 us
  Analyze(M, 100.0000005, "X0", 4);
  Passed = EvaluateNear("GetShieldDeadTimeEnd()", "second crystal in coincidence", "The dead time is extended to 7 us", M.GetShieldDeadTimeEnd().GetAsDouble(), 100.0 + 7e-6, 2e-9) && Passed;
  Passed = EvaluateFalse("IsShieldDead()", "second crystal in coincidence", "The shield is not dead for a coincident hit", M.IsShieldDead()) && Passed;

  // Third hit within the dead time: erased
  Analyze(M, 100.000003, "X0", 5);
  Passed = EvaluateTrue("IsShieldDead()", "hit within dead time", "The shield is dead", M.IsShieldDead()) && Passed;
  Passed = Evaluate("GetBGOHitsErased()", "hit within dead time", "One hit is erased", M.GetBGOHitsErased(), 1) && Passed;

  // Fourth hit after the dead time: the previous 7 us are accumulated
  Analyze(M, 100.001, "X0", 3);
  Passed = EvaluateNear("GetTotalShieldDeadtime()", "panel 0 after a new hit", "The total dead time of panel 0 is 7 us", M.GetTotalShieldDeadtime(0), 7e-6, 1e-12) && Passed;
  Passed = Evaluate("GetShieldHitCounts()", "four hits", "Four shield hits are counted", M.GetShieldHitCounts(), 4) && Passed;

  // Finalize() prints the run statistics at info verbosity; stdout of the child process is discarded
  bool FinalizeOK = RunInChild([&M]() -> bool {
    if (freopen("/dev/null", "w", stdout) == nullptr) return false;
    g_Verbosity = c_Info;
    M.Finalize();
    return true;
  });
  Passed = EvaluateTrue("Finalize()", "after four hits, info verbosity", "Finalize prints the statistics without crashing", FinalizeOK) && Passed;

  // A new run starts without the dead time of the previous run
  M.Finalize();
  Passed = EvaluateTrue("Initialize()", "after Finalize", "A second Initialize succeeds", InitializeStandard(M)) && Passed;
  Analyze(M, 100.001 + 1e-6, "X0", 4);
  Passed = EvaluateFalse("IsShieldDead()", "first hit of a new run", "The dead time of the previous run does not carry over", M.IsShieldDead()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldTrigger::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // A crash is a possible outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Time-out of the child process in seconds
    const unsigned int ChildTimeout = 60;
    alarm(ChildTimeout);
    bool Result = Function();
    _exit(Result == true ? 0 : 1);
  }

  int Status = 0;
  if (waitpid(Pid, &Status, 0) != Pid) return false;
  if (WIFEXITED(Status) == false) return false;
  return WEXITSTATUS(Status) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldTrigger::TestPanels()
{
  bool Passed = true;

  // Each known shield detector is assigned to a panel and creates a veto
  vector<MString> Detectors = { "X0", "X1", "Y0", "Y1" };
  for (const MString& D : Detectors) {
    MSubModuleShieldTrigger M;
    Passed = EvaluateTrue("Initialize()", MString("valid file, shield detector ") + D, "Initialize succeeds", InitializeStandard(M)) && Passed;
    Analyze(M, 300.0, D, 1);
    Passed = EvaluateTrue("AnalyzeEvent()", MString("shield detector ") + D, "A hit creates a veto", M.HasShieldVeto()) && Passed;
  }

  // The bottom shield Z0 / Z1 has the crystals 0-4 (ACS_Z0_0 .. ACS_Z0_4 in COSISMEX.ACS.det): every crystal must veto
  vector<MString> Bottom = { "Z0", "Z1" };
  for (const MString& D : Bottom) {
    for (int Crystal = 0; Crystal <= 4; ++Crystal) {
      MSubModuleShieldTrigger Z;
      Passed = EvaluateTrue("Initialize()", MString("valid file, shield detector ") + D + " crystal " + Crystal, "Initialize succeeds", InitializeStandard(Z)) && Passed;
      Analyze(Z, 300.0, D, Crystal);
      if (Crystal < 4) {
        // Known bug: only crystal 4 of the bottom shield vetoes
        Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", MString("shield detector ") + D + " crystal " + Crystal, "A hit creates a veto", Z.HasShieldVeto())) && Passed;
      } else {
        Passed = EvaluateTrue("AnalyzeEvent()", MString("shield detector ") + D + " crystal " + Crystal, "A hit creates a veto", Z.HasShieldVeto()) && Passed;
      }
    }
  }

  // A panel's dead time from an earlier window must not veto a later event after a short window on another panel
  MSubModuleShieldTrigger P;
  Passed = EvaluateTrue("Initialize()", "valid file, two panels", "Initialize succeeds", InitializeStandard(P)) && Passed;
  Analyze(P, 100.0, "X0", 3);
  Analyze(P, 100.0000005, "X0", 4);
  double LongWindow = P.GetShieldDeadTimeEnd().GetAsDouble() - 100.0;
  Analyze(P, 200.0, "X1", 1);
  double ShortWindow = P.GetShieldDeadTimeEnd().GetAsDouble() - 200.0;
  Passed = EvaluateTrue("GetShieldDeadTimeEnd()", "two crystals of X0 vs one crystal of X1", "Precondition: the two-crystal window is more than 1 us longer than the one-crystal window", LongWindow > ShortWindow + 1e-6) && Passed;
  if (LongWindow > ShortWindow + 1e-6) {
    // GeD-only event after the X1 window (and its 0.5 us extension), but within the old X0 window length
    Analyze(P, 200.0 + LongWindow, "");
    Passed = ExpectedToFail(EvaluateFalse("HasShieldVeto()", "GeD-only event after the X1 window, within the length of an old X0 window", "No panel is busy, thus the event is not vetoed", P.HasShieldVeto())) && Passed;
  }

  // An unknown detector is ignored
  MSubModuleShieldTrigger U;
  Passed = EvaluateTrue("Initialize()", "valid file, unknown detector", "Initialize succeeds", InitializeStandard(U)) && Passed;
  Analyze(U, 300.0, "Q9", 1);
  Passed = EvaluateFalse("AnalyzeEvent()", "unknown shield detector Q9", "No veto is created", U.HasShieldVeto()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldTrigger::TestWithoutDeadtimeFile()
{
  bool Passed = true;

  // MModuleDEESMEX passes an empty dead time file name by default: the timing parameters must have defined defaults
  alignas(MSubModuleShieldTrigger) unsigned char Zero[sizeof(MSubModuleShieldTrigger)];
  alignas(MSubModuleShieldTrigger) unsigned char Ones[sizeof(MSubModuleShieldTrigger)];
  memset(Zero, 0x00, sizeof(Zero));
  memset(Ones, 0xFF, sizeof(Ones));
  MSubModuleShieldTrigger* A = new (Zero) MSubModuleShieldTrigger;
  MSubModuleShieldTrigger* B = new (Ones) MSubModuleShieldTrigger;
  A->Initialize();
  B->Initialize();
  Analyze(*A, 100.0, "X0", 3);
  Analyze(*B, 100.0, "X0", 3);
  double EndA = A->GetShieldDeadTimeEnd().GetAsDouble();
  double EndB = B->GetShieldDeadTimeEnd().GetAsDouble();
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "no dead time file", "The dead time is independent of the memory content (all parameters initialized)", EndA == EndB)) && Passed;
  A->~MSubModuleShieldTrigger();
  B->~MSubModuleShieldTrigger();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleShieldTrigger Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleShieldTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
