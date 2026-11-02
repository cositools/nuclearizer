/*
 * UTNSubModuleStripTrigger.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <cstring>
#include <functional>
#include <new>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT:
#include "TSystem.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MTime.h"

// Nuclearizer:
#include "MSubModuleStripTrigger.h"
#include "MReadOutAssembly.h"
#include "MDEEStripHit.h"


//! Unit test class for MSubModuleStripTrigger
class UTNSubModuleStripTrigger : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleStripTrigger() : MUnitTest("UTNSubModuleStripTrigger") {}
  //! Default destructor
  virtual ~UTNSubModuleStripTrigger() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the defaults and the setters/getters
  bool TestDefaults();
  //! Test the Initialize() error paths and the XML configuration
  bool TestInitialize();
  //! Test the trigger condition
  bool TestTriggerCondition();
  //! Test the guard ring veto
  bool TestGuardRingVeto();
  //! Test the dead time handling
  bool TestDeadTime();
  //! Test the fast-clear dead time after a shield veto
  bool TestFastClear();
  //! Test the dead time without dead time file (the MModuleDEESMEX default)
  bool TestWithoutDeadtimeFile();

  //! Write the dead time file and return its name, or an empty string on failure
  MString WriteDeadtimeFile();
  //! Add a DEE strip hit to the event
  void AddHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, bool IsGuardRing = false);
  //! Create an initialized module
  bool InitializeModule(MSubModuleStripTrigger& M);
  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestTriggerCondition() && Passed;
  Passed = TestGuardRingVeto() && Passed;
  Passed = TestDeadTime() && Passed;
  Passed = TestFastClear() && Passed;
  Passed = TestWithoutDeadtimeFile() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleStripTrigger::WriteDeadtimeFile()
{
  // Coincidence window 1 us, 2 us per read-out channel, delays 3 us + 4 us, fast clear 5 us
  MString FileName = GetTemporaryFileName("deadtime.txt");
  if (WriteTextFile(FileName, "# window, per channel, delay 1, delay 2, fast clear\n1e-6 2e-6 3e-6 4e-6 5e-6\n") == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


void UTNSubModuleStripTrigger::AddHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, bool IsGuardRing)
{
  MDEEStripHit H;
  H.m_ROE.SetDetectorID(DetID);
  H.m_ROE.SetStripID(StripID);
  H.m_ROE.IsLowVoltageStrip(IsLowVoltage);
  H.m_IsGuardRing = IsGuardRing;
  // The MDEEStripHit default is a nearest neighbor: these are triggered hits
  H.m_IsNearestNeighbor = false;
  H.m_Energy = 100.0;
  if (IsLowVoltage == true) {
    Event.AddDEEStripHitLV(H);
  } else {
    Event.AddDEEStripHitHV(H);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::InitializeModule(MSubModuleStripTrigger& M)
{
  // An empty dead time file name would silently select the built-in defaults
  MString FileName = WriteDeadtimeFile();
  if (FileName.IsEmpty() == true) return false;
  M.SetDeadtimeFileName(FileName);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Result = M.Initialize();
  g_Verbosity = OldVerbosity;
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // A crash is a possible outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Seconds after which a hanging child is killed
    const unsigned int ChildTimeout = 60;
    alarm(ChildTimeout);
    g_Verbosity = c_Quiet;
    bool Result = Function();
    _exit(Result == true ? 0 : 1);
  }

  int Status = 0;
  if (waitpid(Pid, &Status, 0) != Pid) return false;
  if (WIFEXITED(Status) == false) return false;
  return WEXITSTATUS(Status) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::TestDefaults()
{
  bool Passed = true;

  MSubModuleStripTrigger M;
  Passed = EvaluateFalse("HasTrigger()", "default", "No trigger by default", M.HasTrigger()) && Passed;
  Passed = EvaluateFalse("HasGuardRingVeto()", "default", "No guard ring veto by default", M.HasGuardRingVeto()) && Passed;
  Passed = EvaluateFalse("IsGeDDead()", "default", "The detectors are not dead by default", M.IsGeDDead()) && Passed;
  Passed = EvaluateNear("GetStripsTotalDeadtime()", "default", "The total dead time is 0", M.GetStripsTotalDeadtime(), 0.0, 1e-15) && Passed;
  Passed = EvaluateNear("GetStripDeadTimeEnd()", "default", "The dead time end is 0", M.GetStripDeadTimeEnd().GetAsDouble(), 0.0, 1e-15) && Passed;
  Passed = Evaluate("GetTriggerRate()", "detector 0", "No triggers by default", M.GetTriggerRate(0), 0) && Passed;
  Passed = Evaluate("GetTriggerRate()", "detector -1", "An invalid detector returns 0", M.GetTriggerRate(-1), 0) && Passed;
  Passed = Evaluate("GetTriggerRate()", "detector 16", "An invalid detector returns 0", M.GetTriggerRate(16), 0) && Passed;
  Passed = Evaluate("GetDeadtimeFileName()", "default", "The default file name is empty", M.GetDeadtimeFileName(), MString("")) && Passed;
  M.SetDeadtimeFileName("dt.txt");
  Passed = Evaluate("SetDeadtimeFileName/GetDeadtimeFileName", "dt.txt", "Returns the set name", M.GetDeadtimeFileName(), MString("dt.txt")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::TestInitialize()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MSubModuleStripTrigger Missing;
  Missing.SetDeadtimeFileName(GetTemporaryFileName("missing.txt"));
  bool MissingResult = Missing.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("Initialize()", "missing dead time file", "Initialize returns false", MissingResult) && Passed;

  // A file without value line must be rejected, not crash the program
  MString Short = GetTemporaryFileName("short.txt");
  Passed = EvaluateTrue("WriteTextFile()", "short.txt", "The fixture file is written", WriteTextFile(Short, "# only a header\n")) && Passed;
  bool ShortRejected = RunInChild([Short]() -> bool {
    MSubModuleStripTrigger ShortFile;
    ShortFile.SetDeadtimeFileName(Short);
    return ShortFile.Initialize() == false;
  });
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "dead time file with only a header", "Initialize returns false without crashing", ShortRejected)) && Passed;

  MString FewValues = GetTemporaryFileName("fewvalues.txt");
  Passed = EvaluateTrue("WriteTextFile()", "fewvalues.txt", "The fixture file is written", WriteTextFile(FewValues, "# window, per channel, delay 1, delay 2, fast clear\n1e-6 2e-6\n")) && Passed;
  bool FewRejected = RunInChild([FewValues]() -> bool {
    MSubModuleStripTrigger FewFile;
    FewFile.SetDeadtimeFileName(FewValues);
    return FewFile.Initialize() == false;
  });
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "dead time file with two of five values", "Initialize returns false without crashing", FewRejected)) && Passed;

  MSubModuleStripTrigger Good;
  Passed = EvaluateTrue("Initialize()", "dead time file", "Initialize succeeds", InitializeModule(Good)) && Passed;

  // The dead time file name is stored by MModuleDEESMEX, the sub module adds nothing to the configuration
  MXmlNode* Node = new MXmlNode(nullptr, "StripTrigger");
  Passed = EvaluateTrue("CreateXmlConfiguration()", "mother node", "The mother node is returned", Good.CreateXmlConfiguration(Node) == Node) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "mother node", "ReadXmlConfiguration returns true", Good.ReadXmlConfiguration(Node)) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::TestTriggerCondition()
{
  bool Passed = true;

  MSubModuleStripTrigger M;
  Passed = EvaluateTrue("Initialize()", "dead time file", "Initialize succeeds", InitializeModule(M)) && Passed;

  // Both sides in detector 0, only the LV side in detector 1
  MReadOutAssembly E;
  E.SetTimeUTC(MTime(10.0));
  AddHit(E, 0, 5, true);
  AddHit(E, 0, 40, false);
  AddHit(E, 1, 7, true);
  Passed = EvaluateTrue("AnalyzeEvent()", "detector 0 both sides, detector 1 LV only", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = EvaluateTrue("HasTrigger()", "detector 0 both sides", "The event triggers", M.HasTrigger()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "detector 1 LV only", "The single-sided detector 1 hit is removed", E.GetNDEEStripHitsLV(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "detector 0 both sides", "The HV hit is kept", E.GetNDEEStripHitsHV(), 1u) && Passed;
  Passed = Evaluate("GetTriggerRate()", "detector 0", "Detector 0 has one trigger", M.GetTriggerRate(0), 1) && Passed;
  Passed = Evaluate("GetTriggerRate()", "detector 1", "Detector 1 has no trigger", M.GetTriggerRate(1), 0) && Passed;

  // Only one side: no trigger, all hits removed
  MSubModuleStripTrigger N;
  Passed = EvaluateTrue("Initialize()", "dead time file", "Initialize succeeds", InitializeModule(N)) && Passed;
  MReadOutAssembly F;
  F.SetTimeUTC(MTime(20.0));
  AddHit(F, 2, 5, true);
  AddHit(F, 2, 6, true);
  N.Clear();
  N.AnalyzeEvent(&F);
  Passed = EvaluateFalse("HasTrigger()", "LV only", "An event with only LV strips does not trigger", N.HasTrigger()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "LV only", "All hits are removed", F.GetNDEEStripHitsLV() + F.GetNDEEStripHitsHV(), 0u) && Passed;

  // Clear() resets the flags
  M.Clear();
  Passed = EvaluateFalse("Clear()", "after trigger", "Clear resets the trigger flag", M.HasTrigger()) && Passed;

  M.Finalize();
  N.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::TestGuardRingVeto()
{
  bool Passed = true;

  MSubModuleStripTrigger M;
  Passed = EvaluateTrue("Initialize()", "dead time file", "Initialize succeeds", InitializeModule(M)) && Passed;
  MReadOutAssembly E;
  E.SetTimeUTC(MTime(30.0));
  AddHit(E, 0, 5, true);
  AddHit(E, 0, 40, false);
  AddHit(E, 0, 64, false, true);
  M.AnalyzeEvent(&E);
  Passed = EvaluateTrue("HasGuardRingVeto()", "strip 64 hit", "A hit on strip 64 raises the guard ring veto", M.HasGuardRingVeto()) && Passed;
  Passed = EvaluateTrue("HasTrigger()", "strip 64 hit", "The event still triggers", M.HasTrigger()) && Passed;

  // A guard ring hit alone does not make a side trigger
  MSubModuleStripTrigger N;
  Passed = EvaluateTrue("Initialize()", "dead time file", "Initialize succeeds", InitializeModule(N)) && Passed;
  MReadOutAssembly F;
  F.SetTimeUTC(MTime(40.0));
  AddHit(F, 0, 5, true);
  AddHit(F, 0, 64, false, true);
  N.AnalyzeEvent(&F);
  Passed = EvaluateFalse("HasTrigger()", "LV strip and HV guard ring", "A guard ring hit does not count as an HV strip", N.HasTrigger()) && Passed;

  // A sub-threshold guard ring neighbor of a triggered strip 63 (created by the charge transport) is no guard ring trigger
  MSubModuleStripTrigger O;
  Passed = EvaluateTrue("Initialize()", "dead time file", "Initialize succeeds", InitializeModule(O)) && Passed;
  MReadOutAssembly G;
  G.SetTimeUTC(MTime(50.0));
  AddHit(G, 0, 63, true);
  AddHit(G, 0, 40, false);
  MDEEStripHit NN;
  NN.m_ROE.SetDetectorID(0);
  NN.m_ROE.SetStripID(64);
  NN.m_ROE.IsLowVoltageStrip(true);
  NN.m_IsGuardRing = true;
  NN.m_IsNearestNeighbor = true;
  NN.m_Energy = 0.0;
  G.AddDEEStripHitLV(NN);
  O.AnalyzeEvent(&G);
  Passed = ExpectedToFail(EvaluateFalse("HasGuardRingVeto()", "LV strip 63 with sub-threshold guard ring neighbor", "A nearest-neighbor guard ring hit does not raise the guard ring veto", O.HasGuardRingVeto())) && Passed;

  M.Finalize();
  N.Finalize();
  O.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::TestWithoutDeadtimeFile()
{
  bool Passed = true;

  // GCC removes the memory fills before the constructor as dead stores (-flifetime-dse), which leaves random bytes: not checked with GCC
#if !defined(__GNUC__) || defined(__clang__)
  // MModuleDEESMEX passes an empty dead time file name by default: all timing parameters must have defined defaults
  alignas(MSubModuleStripTrigger) unsigned char Zero[sizeof(MSubModuleStripTrigger)];
  alignas(MSubModuleStripTrigger) unsigned char Ones[sizeof(MSubModuleStripTrigger)];
  memset(Zero, 0x00, sizeof(Zero));
  memset(Ones, 0xFF, sizeof(Ones));
  MSubModuleStripTrigger* A = new (Zero) MSubModuleStripTrigger;
  MSubModuleStripTrigger* B = new (Ones) MSubModuleStripTrigger;
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool InitA = A->Initialize();
  bool InitB = B->Initialize();
  MReadOutAssembly EA;
  EA.SetTimeUTC(MTime(100.0));
  AddHit(EA, 0, 5, true);
  AddHit(EA, 0, 40, false);
  A->AnalyzeEvent(&EA);
  MReadOutAssembly EB;
  EB.SetTimeUTC(MTime(100.0));
  AddHit(EB, 0, 5, true);
  AddHit(EB, 0, 40, false);
  B->AnalyzeEvent(&EB);
  g_Verbosity = OldVerbosity;

  // Both memory contents give the dead time end 0 (with 0xFF the NaN dead time never exceeds 0), so only the absolute value detects the missing defaults
  double EndA = A->GetStripDeadTimeEnd().GetAsDouble();
  double EndB = B->GetStripDeadTimeEnd().GetAsDouble();
  Passed = EvaluateTrue("Initialize()", "no dead time file, 0x00 memory", "Initialize succeeds without dead time file", InitA) && Passed;
  Passed = EvaluateTrue("Initialize()", "no dead time file, 0xFF memory", "Initialize succeeds without dead time file", InitB) && Passed;
  Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "no dead time file, 0x00 memory, event at 100 s", "The dead time ends after the event and within 1 ms", EndA, 100.0005, 0.0005)) && Passed;
  Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "no dead time file, 0xFF memory, event at 100 s", "The dead time ends after the event and within 1 ms", EndB, 100.0005, 0.0005)) && Passed;
  A->~MSubModuleStripTrigger();
  B->~MSubModuleStripTrigger();
#endif

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::TestDeadTime()
{
  bool Passed = true;

  MSubModuleStripTrigger M;
  if (InitializeModule(M) == false) {
    return EvaluateTrue("Initialize()", "dead time", "Initialize succeeds", false);
  }

  // Event A at 100 s: one channel in ASIC 0 and one in ASIC 3 => dead time 1 + 2 + 3 + 4 = 10 us
  MReadOutAssembly A;
  A.SetTimeUTC(MTime(100.0));
  AddHit(A, 0, 5, true);
  AddHit(A, 0, 40, false);
  M.Clear();
  M.AnalyzeEvent(&A);
  Passed = EvaluateTrue("HasTrigger()", "event A", "Event A triggers", M.HasTrigger()) && Passed;
  Passed = EvaluateFalse("IsGeDDead()", "event A", "The detector is not dead for event A", M.IsGeDDead()) && Passed;
  Passed = EvaluateNear("GetStripDeadTimeEnd()", "event A", "The dead time ends 10 us after event A", M.GetStripDeadTimeEnd().GetAsDouble(), 100.0 + 10e-6, 1e-9) && Passed;

  // Event B within the coincidence window: two channels per ASIC => dead time 1 + 4 + 7 = 12 us
  MReadOutAssembly B;
  B.SetTimeUTC(MTime(100.0000005));
  AddHit(B, 0, 6, true);
  AddHit(B, 0, 41, false);
  M.Clear();
  M.AnalyzeEvent(&B);
  Passed = EvaluateTrue("HasTrigger()", "event B in coincidence window", "Event B triggers", M.HasTrigger()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event B in coincidence window", "The hits of event B are kept", B.GetNDEEStripHitsLV() + B.GetNDEEStripHitsHV(), 2u) && Passed;
  Passed = EvaluateNear("GetStripDeadTimeEnd()", "event B in coincidence window", "The dead time is extended to 12 us after event A (MTime has ns resolution)", M.GetStripDeadTimeEnd().GetAsDouble(), 100.0 + 12e-6, 2e-9) && Passed;

  // Event C within the dead time: the hits are lost
  MReadOutAssembly C;
  C.SetTimeUTC(MTime(100.000005));
  AddHit(C, 0, 7, true);
  AddHit(C, 0, 42, false);
  M.Clear();
  M.AnalyzeEvent(&C);
  Passed = EvaluateTrue("IsGeDDead()", "event C in dead time", "The detector is dead for event C", M.IsGeDDead()) && Passed;
  Passed = EvaluateFalse("HasTrigger()", "event C in dead time", "Event C does not trigger", M.HasTrigger()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event C in dead time", "The hits of event C are removed", C.GetNDEEStripHitsLV() + C.GetNDEEStripHitsHV(), 0u) && Passed;

  // Event D after the dead time: the dead time of the previous period is accumulated
  MReadOutAssembly D;
  D.SetTimeUTC(MTime(100.001));
  AddHit(D, 0, 8, true);
  AddHit(D, 0, 43, false);
  M.Clear();
  M.AnalyzeEvent(&D);
  Passed = EvaluateTrue("HasTrigger()", "event D after dead time", "Event D triggers", M.HasTrigger()) && Passed;
  Passed = EvaluateFalse("IsGeDDead()", "event D after dead time", "The detector is alive for event D", M.IsGeDDead()) && Passed;
  Passed = EvaluateNear("GetStripsTotalDeadtime()", "after event D", "The total dead time is the 12 us of the first period", M.GetStripsTotalDeadtime(), 12e-6, 1e-12) && Passed;
  Passed = EvaluateNear("GetStripDeadTimeEnd()", "event D", "The new dead time ends 10 us after event D", M.GetStripDeadTimeEnd().GetAsDouble(), 100.001 + 10e-6, 1e-9) && Passed;
  Passed = Evaluate("GetTriggerRate()", "detector 0", "Three events triggered detector 0", M.GetTriggerRate(0), 3) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripTrigger::TestFastClear()
{
  bool Passed = true;

  MSubModuleStripTrigger M;
  if (InitializeModule(M) == false) {
    return EvaluateTrue("Initialize()", "fast clear", "Initialize succeeds", false);
  }

  M.ApplyFastClearDeadtime(MTime(200.25));
  Passed = EvaluateNear("ApplyFastClearDeadtime()", "veto at 200.25 s", "The dead time ends 5 us after the veto", M.GetStripDeadTimeEnd().GetAsDouble(), 200.25 + 5e-6, 1e-9) && Passed;
  Passed = EvaluateNear("ApplyFastClearDeadtime()", "veto at 200.25 s", "The fast clear dead time is added to the total", M.GetStripsTotalDeadtime(), 5e-6, 1e-12) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleStripTrigger Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleStripTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
