/*
 * UTNSubModuleDepthReadout.cxx
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

// ROOT libs:
#include "TRandom.h"

// ROOT:
#include "TSystem.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MSubModuleDepthReadout.h"
#include "MReadOutAssembly.h"
#include "MDEEStripHit.h"


//! Unit test class for MSubModuleDepthReadout
class UTNSubModuleDepthReadout : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleDepthReadout() : MUnitTest("UTNSubModuleDepthReadout") {}
  //! Default destructor
  virtual ~UTNSubModuleDepthReadout() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the defaults, the setters/getters and the XML configuration
  bool TestConfiguration();
  //! Test the Initialize() error paths
  bool TestInitialize();
  //! Test the timing and TAC computation without smearing
  bool TestTimingAndTAC();
  //! Test the limits of the TAC values
  bool TestTACLimits();
  //! Test a TAC calibration which does not cover all strips
  bool TestPartialTACCalibration();
  //! Test the timing smearing
  bool TestTimingResolution();

  //! Write the strip map file and return its name, or an empty string on failure
  MString WriteStripMap();
  //! Return the read-out ID of a strip in the strip map fixture
  unsigned int GetReadOutID(unsigned int DetID, bool IsLowVoltage, unsigned int StripID) const;
  //! Return the DIB a detector of the strip map fixture sits on (not the detector ID)
  unsigned int GetDIB(unsigned int DetID) const;
  //! Write the TAC calibration file and return its name, or an empty string on failure
  MString WriteTACCalibration();
  //! Write the depth coefficients file and return its name, or an empty string on failure
  MString WriteCoeffs();
  //! Initialize a module with the standard files
  bool InitializeStandard(MSubModuleDepthReadout& M, bool ApplyTimingResolution, MString TACFile = "");
  //! Create a DEE strip hit
  MDEEStripHit CreateHit(unsigned int DetID, unsigned int StripID, bool IsLowVoltage, double Energy, double FastPeakTime, unsigned int OppositeStripID = 0);
  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDepthReadout::Run()
{
  bool Passed = true;

  Passed = TestConfiguration() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestTimingAndTAC() && Passed;
  Passed = TestTACLimits() && Passed;
  Passed = TestPartialTACCalibration() && Passed;
  Passed = TestTimingResolution() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTNSubModuleDepthReadout::GetReadOutID(unsigned int DetID, bool IsLowVoltage, unsigned int StripID) const
{
  // MStripMap encoding: DIB << 7 | primary << 6 | ASIC << 5 | channel, with the HV side as the primary ASIC section
  return (GetDIB(DetID) << 7) + (IsLowVoltage == true ? 0 : 64) + StripID;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTNSubModuleDepthReadout::GetDIB(unsigned int DetID) const
{
  // Deliberately different from the detector ID: the DIB column of the TAC calibration file must not be used as detector ID
  return DetID + 4;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleDepthReadout::WriteStripMap()
{
  // Detectors 0-2, LV and HV strips 0-63
  MString C = "# Columns are strip_id, detector, side (0=LV, 1=HV), strip\n";
  for (unsigned int d = 0; d < 3; ++d) {
    for (unsigned int s = 0; s < 64; ++s) {
      C += MString("") + GetReadOutID(d, true, s) + " " + d + " 0 " + s + "\n";
      C += MString("") + GetReadOutID(d, false, s) + " " + d + " 1 " + s + "\n";
    }
  }
  MString FileName = GetTemporaryFileName("dr.stripmap.map");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleDepthReadout::WriteTACCalibration()
{
  // Detector 0, all strips: Timing = 0.5 * TAC - 500
  // Detector 1, all strips: Timing = 0.2 * TAC + 0 (saturates quickly)
  // Detector 2, all strips: Timing = 0.5 * TAC + 5000 (negative TACs for short timings)
  // The second column holds the DIB ID (detector + 4), not the detector ID: detector, side, and strip are
  // taken from the strip map via the read-out ID (see #239)
  MString C = "#STRIP_ID,DIB,Side,Strip,TAC_calibration,TAC_calibration_error,TAC_offset,TAC_offset_error\n";
  for (unsigned int d = 0; d < 3; ++d) {
    MString Slope = (d == 1) ? "0.2" : "0.5";
    MString Offset = (d == 0) ? "-500.0" : ((d == 1) ? "0.0" : "5000.0");
    for (unsigned int s = 0; s < 64; ++s) {
      C += MString("") + GetReadOutID(d, true, s) + "," + GetDIB(d) + ",0," + s + "," + Slope + ",0.001," + Offset + ",1.0\n";
      C += MString("") + GetReadOutID(d, false, s) + "," + GetDIB(d) + ",1," + s + "," + Slope + ",0.001," + Offset + ",1.0\n";
    }
  }
  MString FileName = GetTemporaryFileName("dr.taccal.csv");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleDepthReadout::WriteCoeffs()
{
  // Pixel LV 10 / HV 20 in detector 0: CTD sigma 10 ns at 59.5 keV
  MString C = "### 800 V 80 K 59.5 keV\n1020,1.0,0.0,10.0,1.0\n";
  MString FileName = GetTemporaryFileName("dr.coeffs.csv");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDepthReadout::InitializeStandard(MSubModuleDepthReadout& M, bool ApplyTimingResolution, MString TACFile)
{
  M.SetDepthCoefficientsFileName(WriteCoeffs());
  M.SetTACCalFileName(TACFile == "" ? WriteTACCalibration() : TACFile);
  M.SetStripMapFileName(WriteStripMap());
  M.SetApplyTimingResolutionCalibration(ApplyTimingResolution);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Result = M.Initialize();
  g_Verbosity = OldVerbosity;
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


MDEEStripHit UTNSubModuleDepthReadout::CreateHit(unsigned int DetID, unsigned int StripID, bool IsLowVoltage, double Energy, double FastPeakTime, unsigned int OppositeStripID)
{
  MDEEStripHit H;
  H.m_ROE.SetDetectorID(DetID);
  H.m_ROE.SetStripID(StripID);
  H.m_ROE.IsLowVoltageStrip(IsLowVoltage);
  H.m_Energy = Energy;
  H.m_FastPeakTime = FastPeakTime;
  H.m_OppositeStripID = OppositeStripID;
  return H;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDepthReadout::RunInChild(function<bool()> Function)
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


bool UTNSubModuleDepthReadout::TestConfiguration()
{
  bool Passed = true;

  MSubModuleDepthReadout M;
  Passed = Evaluate("GetStripMapFileName()", "default", "The default strip map file name is empty", M.GetStripMapFileName(), MString("")) && Passed;
  Passed = EvaluateTrue("GetStripMap()", "default", "No strip map is available before Initialize()", M.GetStripMap() == nullptr) && Passed;
  M.SetDepthCoefficientsFileName("c.csv");
  M.SetTACCalFileName("t.csv");
  M.SetStripMapFileName("m.map");
  Passed = Evaluate("SetDepthCoefficientsFileName/GetDepthCoefficientsFileName", "c.csv", "Returns the set name", M.GetDepthCoefficientsFileName(), MString("c.csv")) && Passed;
  Passed = Evaluate("SetTACCalFileName/GetTACCalFileName", "t.csv", "Returns the set name", M.GetTACCalFileName(), MString("t.csv")) && Passed;
  Passed = Evaluate("SetStripMapFileName/GetStripMapFileName", "m.map", "Returns the set name", M.GetStripMapFileName(), MString("m.map")) && Passed;
  M.SetApplyTimingResolutionCalibration(true);
  Passed = EvaluateTrue("SetApplyTimingResolutionCalibration/GetApplyTimingResolutionCalibration", "true", "Returns true", M.GetApplyTimingResolutionCalibration()) && Passed;
  M.SetApplyTimingResolutionCalibration(false);
  Passed = EvaluateFalse("SetApplyTimingResolutionCalibration/GetApplyTimingResolutionCalibration", "false", "Returns false", M.GetApplyTimingResolutionCalibration()) && Passed;

  MXmlNode* Node = new MXmlNode(nullptr, "DEE");
  M.SetTACCalFileName("/data/t.csv");
  M.SetStripMapFileName("/data/m.map");
  M.CreateXmlConfiguration(Node);
  MSubModuleDepthReadout Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "TAC file name", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "TACCalFileName", "The TAC calibration file name survives", Reader.GetTACCalFileName(), MString("/data/t.csv")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "StripMapFileName", "The strip map file name survives", Reader.GetStripMapFileName(), MString("/data/m.map")) && Passed;
  delete Node;

  // GCC removes the memory fills before the constructor as dead stores (-flifetime-dse), which leaves random bytes: not checked with GCC
#if !defined(__GNUC__) || defined(__clang__)
  // The timing resolution flag must be initialized by the constructor
  alignas(MSubModuleDepthReadout) unsigned char Zero[sizeof(MSubModuleDepthReadout)];
  alignas(MSubModuleDepthReadout) unsigned char Ones[sizeof(MSubModuleDepthReadout)];
  memset(Zero, 0x00, sizeof(Zero));
  memset(Ones, 0xFF, sizeof(Ones));
  MSubModuleDepthReadout* A = new (Zero) MSubModuleDepthReadout;
  MSubModuleDepthReadout* B = new (Ones) MSubModuleDepthReadout;
  bool ValueA = A->GetApplyTimingResolutionCalibration();
  bool ValueB = B->GetApplyTimingResolutionCalibration();
  Passed = EvaluateTrue("MSubModuleDepthReadout()", "uninitialized memory", "The timing resolution flag has a defined default independent of the memory content", ValueA == ValueB) && Passed;
  A->~MSubModuleDepthReadout();
  B->~MSubModuleDepthReadout();
#endif

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDepthReadout::TestInitialize()
{
  bool Passed = true;

  MString TACFile = WriteTACCalibration();
  Passed = EvaluateFalse("WriteTACCalibration()", "TAC calibration file", "The fixture file is written", TACFile.IsEmpty()) && Passed;
  MString CoeffsFile = WriteCoeffs();
  Passed = EvaluateFalse("WriteCoeffs()", "coefficients file", "The fixture file is written", CoeffsFile.IsEmpty()) && Passed;
  MString StripMapFile = WriteStripMap();
  Passed = EvaluateFalse("WriteStripMap()", "strip map file", "The fixture file is written", StripMapFile.IsEmpty()) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MSubModuleDepthReadout MissingCoeffs;
  MissingCoeffs.SetDepthCoefficientsFileName(GetTemporaryFileName("missing.csv"));
  MissingCoeffs.SetTACCalFileName(TACFile);
  MissingCoeffs.SetStripMapFileName(StripMapFile);
  MissingCoeffs.SetApplyTimingResolutionCalibration(false);
  bool MissingCoeffsResult = MissingCoeffs.Initialize();
  MSubModuleDepthReadout MissingTAC;
  MissingTAC.SetDepthCoefficientsFileName(CoeffsFile);
  MissingTAC.SetTACCalFileName(GetTemporaryFileName("missing.taccal.csv"));
  MissingTAC.SetStripMapFileName(StripMapFile);
  MissingTAC.SetApplyTimingResolutionCalibration(false);
  bool MissingTACResult = MissingTAC.Initialize();
  MSubModuleDepthReadout NoStripMap;
  NoStripMap.SetDepthCoefficientsFileName(CoeffsFile);
  NoStripMap.SetTACCalFileName(TACFile);
  NoStripMap.SetApplyTimingResolutionCalibration(false);
  bool NoStripMapResult = NoStripMap.Initialize();
  MSubModuleDepthReadout MissingStripMap;
  MissingStripMap.SetDepthCoefficientsFileName(CoeffsFile);
  MissingStripMap.SetTACCalFileName(TACFile);
  MissingStripMap.SetStripMapFileName(GetTemporaryFileName("missing.map"));
  MissingStripMap.SetApplyTimingResolutionCalibration(false);
  bool MissingStripMapResult = MissingStripMap.Initialize();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "missing coefficients file", "Initialize returns false", MissingCoeffsResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing TAC calibration file", "Initialize returns false", MissingTACResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "no strip map file name", "Initialize returns false", NoStripMapResult) && Passed;
  Passed = EvaluateTrue("GetStripMap()", "no strip map file name", "No strip map is available", NoStripMap.GetStripMap() == nullptr) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing strip map file", "Initialize returns false", MissingStripMapResult) && Passed;
  Passed = EvaluateTrue("GetStripMap()", "missing strip map file", "No strip map is available", MissingStripMap.GetStripMap() == nullptr) && Passed;

  MSubModuleDepthReadout Good;
  Passed = EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", InitializeStandard(Good, false)) && Passed;
  Passed = EvaluateTrue("GetStripMap()", "after Initialize()", "The strip map is available", Good.GetStripMap() != nullptr) && Passed;
  if (Good.GetStripMap() != nullptr) {
    Passed = EvaluateTrue("GetStripMap()", "after Initialize()", "The strip map contains the read-out ID of HV strip 5 of detector 2", Good.GetStripMap()->HasReadOutID(GetReadOutID(2, false, 5))) && Passed;
  }
  Good.Finalize();
  Passed = EvaluateTrue("GetStripMap()", "after Finalize()", "No strip map is available", Good.GetStripMap() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDepthReadout::TestTimingAndTAC()
{
  bool Passed = true;

  MSubModuleDepthReadout M;
  if (InitializeStandard(M, false) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MReadOutAssembly E;
  // The timing is derived from the drift time via the hard-coded conversion 4200 - drift time of MSubModuleDepthReadout:
  // if the timing checks below fail, first check whether this conversion has changed
  E.AddDEEStripHitLV(CreateHit(0, 10, true, 100.0, 1200.0));    // Timing 4200 - 1200 = 3000 ns, TAC (3000 + 500) / 0.5 = 7000
  E.AddDEEStripHitLV(CreateHit(0, 11, true, 5.0, 1200.0));      // low energy: no fast timing on the LV side
  E.AddDEEStripHitLV(CreateHit(0, 12, true, 100.0, 20000.0));   // unphysical drift time
  MDEEStripHit GR = CreateHit(0, 64, true, 100.0, 1200.0);
  GR.m_IsGuardRing = true;
  GR.m_TAC = 123;
  E.AddDEEStripHitLV(GR);
  E.AddDEEStripHitHV(CreateHit(0, 20, false, 100.0, 1000.0));   // Timing 4200 - 1000 = 3200 ns, TAC 7400
  Passed = EvaluateTrue("AnalyzeEvent()", "standard hits", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  g_Verbosity = OldVerbosity;

  auto& LV = E.GetDEEStripHitLVListReference();
  auto& HV = E.GetDEEStripHitHVListReference();
  Passed = EvaluateSize("AnalyzeEvent()", "standard hits", "The four LV hits are kept", LV.size(), 4) && Passed;
  Passed = EvaluateSize("AnalyzeEvent()", "standard hits", "The HV hit is kept", HV.size(), 1) && Passed;
  if (LV.size() == 4) {
    auto It = LV.begin();
    MDEEStripHit& L10 = *It++;
    MDEEStripHit& L11 = *It++;
    MDEEStripHit& L12 = *It++;
    MDEEStripHit& LGR = *It++;
    Passed = EvaluateNear("AnalyzeEvent()", "LV strip 10", "The timing is 4200 - 1200 = 3000 ns (hard-coded conversion 4200 - drift time)", L10.m_Timing, 3000.0, 1e-9) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV strip 10", "The TAC is (3000 + 500) / 0.5 = 7000", L10.m_TAC, 7000u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "LV strip 10, 100 keV", "The LV strip has fast timing", L10.m_HasFastTiming) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "LV strip 11, 5 keV", "A 5 keV LV strip has no fast timing", L11.m_HasFastTiming) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV strip 12, unphysical drift time", "The TAC is 0", L12.m_TAC, 0u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "LV strip 12, unphysical drift time", "There is no fast timing", L12.m_HasFastTiming) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "guard ring", "Guard ring hits are not modified", LGR.m_TAC, 123u) && Passed;
  }
  if (HV.size() == 1) {
    MDEEStripHit& H20 = HV.front();
    Passed = EvaluateNear("AnalyzeEvent()", "HV strip 20", "The timing is 4200 - 1000 = 3200 ns (hard-coded conversion 4200 - drift time)", H20.m_Timing, 3200.0, 1e-9) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "HV strip 20", "The TAC is (3200 + 500) / 0.5 = 7400", H20.m_TAC, 7400u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "HV strip 20", "The HV strip has fast timing", H20.m_HasFastTiming) && Passed;
  }

  // Clear() between events keeps the run configuration: the same event gives the same TAC values
  M.Clear();
  MReadOutAssembly Next;
  Next.AddDEEStripHitLV(CreateHit(0, 10, true, 100.0, 1200.0));
  Next.AddDEEStripHitHV(CreateHit(0, 20, false, 100.0, 1000.0));
  g_Verbosity = c_Quiet;
  Passed = EvaluateTrue("AnalyzeEvent()", "after Clear()", "AnalyzeEvent returns true", M.AnalyzeEvent(&Next)) && Passed;
  g_Verbosity = OldVerbosity;
  Passed = EvaluateSize("Clear()", "next event", "The LV hit is kept", Next.GetDEEStripHitLVListReference().size(), 1) && Passed;
  Passed = EvaluateSize("Clear()", "next event", "The HV hit is kept", Next.GetDEEStripHitHVListReference().size(), 1) && Passed;
  if (Next.GetDEEStripHitLVListReference().size() == 1) {
    Passed = Evaluate("Clear()", "LV strip 10, next event", "The TAC is still 7000", Next.GetDEEStripHitLVListReference().front().m_TAC, 7000u) && Passed;
  }
  if (Next.GetDEEStripHitHVListReference().size() == 1) {
    Passed = Evaluate("Clear()", "HV strip 20, next event", "The TAC is still 7400", Next.GetDEEStripHitHVListReference().front().m_TAC, 7400u) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDepthReadout::TestTACLimits()
{
  bool Passed = true;

  MSubModuleDepthReadout M;
  if (InitializeStandard(M, false) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(1, 10, true, 100.0, 0.0));     // detector 1: TAC 4200 / 0.2 = 21000 -> 16383
  E.AddDEEStripHitHV(CreateHit(1, 10, false, 100.0, 0.0));    // detector 1: TAC 4200 / 0.2 = 21000 -> 16383
  E.AddDEEStripHitLV(CreateHit(2, 10, true, 100.0, 1000.0));  // detector 2: (3200 - 5000) / 0.5 < 0 -> 0
  E.AddDEEStripHitHV(CreateHit(2, 10, false, 100.0, 1000.0)); // detector 2: (3200 - 5000) / 0.5 < 0 -> 0
  M.AnalyzeEvent(&E);
  g_Verbosity = OldVerbosity;

  auto& LV = E.GetDEEStripHitLVListReference();
  auto& HV = E.GetDEEStripHitHVListReference();
  Passed = EvaluateSize("AnalyzeEvent()", "TAC limits", "The two LV hits are kept", LV.size(), 2) && Passed;
  Passed = EvaluateSize("AnalyzeEvent()", "TAC limits", "The two HV hits are kept", HV.size(), 2) && Passed;
  if (LV.size() == 2) {
    Passed = Evaluate("AnalyzeEvent()", "LV TAC above range", "The LV TAC saturates at 16383", LV.front().m_TAC, 16383u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV TAC below zero", "A negative LV TAC is set to 0", LV.back().m_TAC, 0u) && Passed;
  }
  if (HV.size() == 2) {
    Passed = Evaluate("AnalyzeEvent()", "HV TAC above range", "The HV TAC saturates at 16383", HV.front().m_TAC, 16383u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "HV TAC below zero", "A negative HV TAC is set to 0", HV.back().m_TAC, 0u) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDepthReadout::TestPartialTACCalibration()
{
  bool Passed = true;

  // Real TAC calibrations cover all strips; this is an incomplete file, which only covers LV and HV strips 32-63 of
  // detector 0 (on DIB 4), to test the look-up by strip ID and the handling of strips without calibration
  MString C = "#STRIP_ID,DIB,Side,Strip,TAC_calibration,TAC_calibration_error,TAC_offset,TAC_offset_error\n";
  for (unsigned int s = 32; s < 64; ++s) {
    C += MString("") + GetReadOutID(0, true, s) + "," + GetDIB(0) + ",0," + s + ",0.5,0.001,-500.0,1.0\n";
    C += MString("") + GetReadOutID(0, false, s) + "," + GetDIB(0) + ",0," + s + ",0.5,0.001,-500.0,1.0\n";
  }
  MString TACFile = GetTemporaryFileName("dr.partial.taccal.csv");
  Passed = EvaluateTrue("WriteTextFile()", "dr.partial.taccal.csv", "The fixture file is written", WriteTextFile(TACFile, C)) && Passed;

  MSubModuleDepthReadout M;
  if (InitializeStandard(M, false, TACFile) == false) {
    return EvaluateTrue("Initialize()", "partial TAC calibration", "Initialize succeeds", false);
  }

  int OldVerbosity = g_Verbosity;

  // LV hit
  g_Verbosity = c_Quiet;
  MReadOutAssembly ELV;
  ELV.AddDEEStripHitLV(CreateHit(0, 40, true, 100.0, 1200.0));
  M.AnalyzeEvent(&ELV);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateSize("AnalyzeEvent()", "LV strip 40 with TAC calibration", "The LV hit is kept", ELV.GetDEEStripHitLVListReference().size(), 1) && Passed;
  if (ELV.GetDEEStripHitLVListReference().size() == 1) {
    Passed = Evaluate("AnalyzeEvent()", "LV strip 40 with TAC calibration", "The calibrated strip 40 gets the TAC (3000 + 500) / 0.5 = 7000", ELV.GetDEEStripHitLVListReference().front().m_TAC, 7000u) && Passed;
  }

  // HV hit
  g_Verbosity = c_Quiet;
  MReadOutAssembly EHV;
  EHV.AddDEEStripHitHV(CreateHit(0, 40, false, 100.0, 1200.0));
  M.AnalyzeEvent(&EHV);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateSize("AnalyzeEvent()", "HV strip 40 with TAC calibration", "The HV hit is kept", EHV.GetDEEStripHitHVListReference().size(), 1) && Passed;
  if (EHV.GetDEEStripHitHVListReference().size() == 1) {
    Passed = Evaluate("AnalyzeEvent()", "HV strip 40 with TAC calibration", "The calibrated strip 40 gets the TAC (3000 + 500) / 0.5 = 7000", EHV.GetDEEStripHitHVListReference().front().m_TAC, 7000u) && Passed;
  }
  M.Finalize();

  // A strip without TAC calibration must not crash the module
  bool NoCrash = RunInChild([this, TACFile]() -> bool {
    MSubModuleDepthReadout X;
    if (InitializeStandard(X, false, TACFile) == false) return false;
    MReadOutAssembly F;
    F.AddDEEStripHitLV(CreateHit(0, 5, true, 100.0, 1200.0));
    X.AnalyzeEvent(&F);
    if (F.GetDEEStripHitLVListReference().size() != 1) return false;
    return F.GetDEEStripHitLVListReference().front().m_TAC == 0;
  });
  Passed = EvaluateTrue("AnalyzeEvent()", "LV strip 5 without TAC calibration", "An uncalibrated strip gets TAC 0 without crashing", NoCrash) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDepthReadout::TestTimingResolution()
{
  bool Passed = true;

  MSubModuleDepthReadout M;
  if (InitializeStandard(M, true) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  // Pixel LV 10 / HV 20: CTD sigma 10 ns at 59.5 keV => timing sigma 10 / sqrt(2) ns at 59.5 keV
  gRandom->SetSeed(1234);
  const unsigned int N = 4000;
  double Sum = 0;
  double Sum2 = 0;
  unsigned int NMissing = 0;
  for (unsigned int i = 0; i < N; ++i) {
    MReadOutAssembly E;
    E.AddDEEStripHitLV(CreateHit(0, 10, true, 59.5, 1200.0, 20));
    M.AnalyzeEvent(&E);
    if (E.GetDEEStripHitLVListReference().size() == 1) {
      double T = E.GetDEEStripHitLVListReference().front().m_Timing;
      Sum += T;
      Sum2 += T*T;
    } else {
      ++NMissing;
    }
  }
  double Mean = Sum/N;
  double Sigma = sqrt(Sum2/N - Mean*Mean);
  Passed = Evaluate("AnalyzeEvent()", "timing resolution on", "Every event keeps its LV hit", NMissing, 0u) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "timing resolution on", "The smeared timing is centered on 3000 ns", Mean, 3000.0, 0.5) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "timing resolution on", "The timing width is the CTD sigma / sqrt(2)", Sigma, 10.0/sqrt(2.0), 0.4) && Passed;

  // Pixels without coefficients are not smeared
  MReadOutAssembly U;
  U.AddDEEStripHitLV(CreateHit(0, 30, true, 59.5, 1200.0, 31));
  M.AnalyzeEvent(&U);
  Passed = EvaluateSize("AnalyzeEvent()", "pixel without coefficients", "The LV hit is kept", U.GetDEEStripHitLVListReference().size(), 1) && Passed;
  if (U.GetDEEStripHitLVListReference().size() == 1) {
    Passed = EvaluateNear("AnalyzeEvent()", "pixel without coefficients", "The timing is not smeared", U.GetDEEStripHitLVListReference().front().m_Timing, 3000.0, 1e-9) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleDepthReadout Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleDepthReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
