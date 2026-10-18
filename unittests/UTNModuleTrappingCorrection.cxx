/*
 * UTNModuleTrappingCorrection.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <algorithm>
#include <functional>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MSupervisor.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleTrappingCorrection.h"
#include "MModuleDepthCalibration.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MHit.h"


//! Exposes the protected helpers of MModuleTrappingCorrection for direct testing
class UTNModuleTrappingCorrectionAccess : public MModuleTrappingCorrection
{
 public:
  //! Forward to the protected LoadTrappingCorrectionFile()
  bool CallLoadTrappingCorrectionFile(MString FileName) { return LoadTrappingCorrectionFile(FileName); }
  //! Forward to the protected Interpolate()
  double CallInterpolate(double X, const vector<double>& XP, const vector<double>& FP) { return Interpolate(X, XP, FP); }
  //! Forward to the protected GetSimBasedCorrectedEnergy()
  double CallGetSimBasedCorrectedEnergy(double Depth, double Energy, const vector<double>& Depths, const vector<double>& E, const vector<double>& H, double B, double C) { return GetSimBasedCorrectedEnergy(Depth, Energy, Depths, E, H, B, C); }
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test class for MModuleTrappingCorrection
class UTNModuleTrappingCorrection : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleTrappingCorrection() : MUnitTest("UTNModuleTrappingCorrection") {}
  //! Default destructor
  virtual ~UTNModuleTrappingCorrection() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and defaults
  bool TestDefaultConstruction();
  //! Test all setter/getter pairs
  bool TestGettersSetters();
  //! Test the XML configuration round trip
  bool TestXmlRoundTrip();
  //! Test Interpolate()
  bool TestInterpolate();
  //! Test GetSimBasedCorrectedEnergy()
  bool TestGetSimBasedCorrectedEnergy();
  //! Test LoadTrappingCorrectionFile()
  bool TestLoadTrappingCorrectionFile();
  //! Test Initialize() with and without an available depth calibration module
  bool TestInitialize();
  //! Test the correction in AnalyzeEvent()
  bool TestAnalyzeEvent();
  //! Test a detector section without depth curve
  bool TestSectionWithoutDepthCurve();

  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);
  //! Register a depth calibration module with the supervisor, required by Initialize()
  void RegisterDepthCalibration();
  //! Write the standard trapping file and return its name, or an empty string on failure
  MString WriteStandardFile(bool Descending = false);
  //! Create an event with one hit at the given depth with one LV and one HV strip hit
  void FillEvent(MReadOutAssembly& Event, unsigned int DetectorID, double Depth, double LVEnergy, double HVEnergy);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestInterpolate() && Passed;
  Passed = TestGetSimBasedCorrectedEnergy() && Passed;
  Passed = TestLoadTrappingCorrectionFile() && Passed;
  // TestInitialize() needs the supervisor without depth calibration module: run it before RegisterDepthCalibration()
  Passed = TestInitialize() && Passed;
  RegisterDepthCalibration();
  Passed = TestAnalyzeEvent() && Passed;
  Passed = TestSectionWithoutDepthCurve() && Passed;

  // ShowOptionsGUI() and CreateExpos() create GUI elements and are not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleTrappingCorrection::WriteStandardFile(bool Descending)
{
  MString C;
  C += "### 0\n";
  C += "# A_HV, A_LV, B, C\n";
  C += "1.0,1.0,0.5,0.25\n";
  C += "# z_depth_cm, e_CCE_HV, h_CCE_HV, e_CCE_LV, h_CCE_LV\n";
  if (Descending == false) {
    C += "-1.0,0.9,1.0,0.8,1.0\n";
    C += "0.0,0.8,0.9,0.9,0.8\n";
    C += "1.0,0.7,0.8,1.0,0.6\n";
  } else {
    C += "1.0,0.7,0.8,1.0,0.6\n";
    C += "0.0,0.8,0.9,0.9,0.8\n";
    C += "-1.0,0.9,1.0,0.8,1.0\n";
  }

  MString FileName = GetTemporaryFileName(Descending == false ? "trapping.csv" : "trapping_descending.csv");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


void UTNModuleTrappingCorrection::RegisterDepthCalibration()
{
  MSupervisor* S = MSupervisor::GetSupervisor();
  if (S->GetAvailableModuleByXmlTag("XmlTagDepthCalibration") == nullptr) {
    S->AddAvailableModule(new MModuleDepthCalibration());
  }
}


////////////////////////////////////////////////////////////////////////////////


void UTNModuleTrappingCorrection::FillEvent(MReadOutAssembly& Event, unsigned int DetectorID, double Depth, double LVEnergy, double HVEnergy)
{
  MStripHit* LV = new MStripHit();
  LV->SetDetectorID(DetectorID);
  LV->SetStripID(10);
  LV->IsLowVoltageStrip(true);
  LV->SetEnergy(LVEnergy);
  Event.AddStripHit(LV);

  MStripHit* HV = new MStripHit();
  HV->SetDetectorID(DetectorID);
  HV->SetStripID(20);
  HV->IsLowVoltageStrip(false);
  HV->SetEnergy(HVEnergy);
  Event.AddStripHit(HV);

  MHit* H = new MHit();
  H->AddStripHit(LV);
  H->AddStripHit(HV);
  H->SetLocalPosition(MVector(0.0, 0.0, Depth));
  H->SetEnergy(0.5*(LVEnergy + HVEnergy));
  Event.AddHit(H);
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleTrappingCorrection M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Trapping Correction'", M.GetName(), MString("Trapping Correction")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagTrappingCorrection'", M.GetXmlTag(), MString("XmlTagTrappingCorrection")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_TrappingCorrection", M.ProvidesModuleType(MAssembly::c_TrappingCorrection)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The energy calibration is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EnergyCalibration)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The strip pairing is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_StripPairing)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The depth correction is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_DepthCorrection)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = Evaluate("GetTrappingCorrectionFileName()", "default", "The default file name is empty", M.GetTrappingCorrectionFileName(), MString("")) && Passed;
  Passed = EvaluateNear("GetPhotopeakMu()", "default", "The default photopeak centroid is 661.7 keV", M.GetPhotopeakMu(), 661.7, 1e-12) && Passed;

  MModuleTrappingCorrection* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::TestGettersSetters()
{
  bool Passed = true;

  MModuleTrappingCorrection M;
  M.SetTrappingCorrectionFileName("trap.csv");
  Passed = Evaluate("SetTrappingCorrectionFileName/GetTrappingCorrectionFileName", "trap.csv", "GetTrappingCorrectionFileName returns the set name", M.GetTrappingCorrectionFileName(), MString("trap.csv")) && Passed;
  M.SetPhotopeakMu(511.0);
  Passed = EvaluateNear("SetPhotopeakMu/GetPhotopeakMu", "511", "GetPhotopeakMu returns 511", M.GetPhotopeakMu(), 511.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleTrappingCorrection Writer;
  Writer.SetTrappingCorrectionFileName("/data/trap.csv");
  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleTrappingCorrection Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "configured", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "TrappingCorrectionFileName", "The file name survives", Reader.GetTrappingCorrectionFileName(), MString("/data/trap.csv")) && Passed;
  delete Node;

  MXmlNode* Empty = new MXmlNode(nullptr, "XmlTagTrappingCorrection");
  MModuleTrappingCorrection EmptyReader;
  EmptyReader.SetTrappingCorrectionFileName("keep.csv");
  EmptyReader.ReadXmlConfiguration(Empty);
  Passed = Evaluate("ReadXmlConfiguration()", "empty node", "A missing file name keeps the previous value", EmptyReader.GetTrappingCorrectionFileName(), MString("keep.csv")) && Passed;
  delete Empty;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::TestInterpolate()
{
  bool Passed = true;

  UTNModuleTrappingCorrectionAccess M;
  vector<double> X = { 0.0, 1.0, 3.0 };
  vector<double> F = { 10.0, 20.0, 0.0 };
  Passed = EvaluateNear("Interpolate()", "x = 0.5", "Linear interpolation in the first interval", M.CallInterpolate(0.5, X, F), 15.0, 1e-12) && Passed;
  Passed = EvaluateNear("Interpolate()", "x = 2", "Linear interpolation in the second interval", M.CallInterpolate(2.0, X, F), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("Interpolate()", "x = 1", "An exact grid point returns its value", M.CallInterpolate(1.0, X, F), 20.0, 1e-12) && Passed;
  Passed = EvaluateNear("Interpolate()", "x = -1", "Below the grid the first value is returned", M.CallInterpolate(-1.0, X, F), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("Interpolate()", "x = 5", "Above the grid the last value is returned", M.CallInterpolate(5.0, X, F), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Interpolate()", "empty grid", "An empty grid returns 0", M.CallInterpolate(1.0, vector<double>(), vector<double>()), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Interpolate()", "single point", "A single-point grid returns its value", M.CallInterpolate(7.0, { 2.0 }, { 4.0 }), 4.0, 1e-12) && Passed;

  // The depth grid can be stored in descending order (AnalyzeEvent() explicitly handles that for the bounds)
  vector<double> XD = { 3.0, 1.0, 0.0 };
  vector<double> FD = { 0.0, 20.0, 10.0 };
  Passed = ExpectedToFail(EvaluateNear("Interpolate()", "descending grid, x = 0.5", "Interpolation works for a descending grid", M.CallInterpolate(0.5, XD, FD), 15.0, 1e-12)) && Passed;
  Passed = ExpectedToFail(EvaluateNear("Interpolate()", "descending grid, x = 2", "Interpolation works for a descending grid", M.CallInterpolate(2.0, XD, FD), 10.0, 1e-12)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::TestGetSimBasedCorrectedEnergy()
{
  bool Passed = true;

  UTNModuleTrappingCorrectionAccess M;
  vector<double> D = { -1.0, 0.0, 1.0 };
  vector<double> E = { 0.9, 0.8, 0.7 };
  vector<double> H = { 1.0, 0.9, 0.8 };

  // Depth 0.5: CCE_e = 0.75, CCE_h = 0.85; scale = (1 - 0.5 * 0.25) * (1 - 0.25 * 0.15)
  double Scale = (1 - 0.5*0.25)*(1 - 0.25*0.15);
  Passed = EvaluateNear("GetSimBasedCorrectedEnergy()", "depth 0.5, 500 keV", "The energy is divided by the expected centroid scale", M.CallGetSimBasedCorrectedEnergy(0.5, 500.0, D, E, H, 0.5, 0.25), 500.0/Scale, 1e-9) && Passed;
  Passed = EvaluateNear("GetSimBasedCorrectedEnergy()", "B = C = 0", "Without trapping the energy is unchanged", M.CallGetSimBasedCorrectedEnergy(0.5, 500.0, D, E, H, 0.0, 0.0), 500.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetSimBasedCorrectedEnergy()", "empty CCE curves", "Empty CCE curves leave the energy unchanged", M.CallGetSimBasedCorrectedEnergy(0.5, 500.0, D, vector<double>(), vector<double>(), 0.5, 0.25), 500.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetSimBasedCorrectedEnergy()", "non-physical scale", "A non-positive scale leaves the energy unchanged", M.CallGetSimBasedCorrectedEnergy(0.5, 500.0, D, E, H, 10.0, 0.0), 500.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::TestLoadTrappingCorrectionFile()
{
  bool Passed = true;

  UTNModuleTrappingCorrectionAccess M;
  MString Standard = WriteStandardFile();
  Passed = EvaluateFalse("WriteStandardFile()", "standard file", "The fixture file is written", Standard.IsEmpty()) && Passed;
  Passed = EvaluateTrue("LoadTrappingCorrectionFile()", "standard file", "Loading the standard file succeeds", M.CallLoadTrappingCorrectionFile(Standard)) && Passed;

  // Two detectors with the documented "### <Detector ID>" header
  MString Two = GetTemporaryFileName("two.csv");
  Passed = EvaluateTrue("WriteTextFile()", "two.csv", "The fixture file is written", WriteTextFile(Two, "### 0\n1,1,0.1,0.1\n0,1,1,1,1\n1,1,1,1,1\n### 1\n1,1,0.2,0.2\n0,1,1,1,1\n1,1,1,1,1\n")) && Passed;
  UTNModuleTrappingCorrectionAccess MTwo;
  Passed = EvaluateTrue("LoadTrappingCorrectionFile()", "two detectors", "Loading two detector sections succeeds", MTwo.CallLoadTrappingCorrectionFile(Two)) && Passed;

  // Section headers with a descriptive word before the ID
  MString Words = GetTemporaryFileName("words.csv");
  Passed = EvaluateTrue("WriteTextFile()", "words.csv", "The fixture file is written", WriteTextFile(Words, "### Detector 0\n1,1,0.1,0.1\n0,1,1,1,1\n### Detector 1\n1,1,0.2,0.2\n0,1,1,1,1\n")) && Passed;
  UTNModuleTrappingCorrectionAccess MWords;
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool WordsResult = MWords.CallLoadTrappingCorrectionFile(Words);
  g_Verbosity = OldVerbosity;
  Passed = ExpectedToFail(EvaluateTrue("LoadTrappingCorrectionFile()", "'### Detector <ID>' headers", "Different detector IDs are not reported as duplicates", WordsResult)) && Passed;

  // Error paths
  MString Duplicate = GetTemporaryFileName("duplicate.csv");
  Passed = EvaluateTrue("WriteTextFile()", "duplicate.csv", "The fixture file is written", WriteTextFile(Duplicate, "### 0\n1,1,0.1,0.1\n0,1,1,1,1\n### 0\n1,1,0.2,0.2\n0,1,1,1,1\n")) && Passed;
  MString NoSection = GetTemporaryFileName("nosection.csv");
  Passed = EvaluateTrue("WriteTextFile()", "nosection.csv", "The fixture file is written", WriteTextFile(NoSection, "1,1,0.1,0.1\n0,1,1,1,1\n")) && Passed;
  g_Verbosity = c_Quiet;
  UTNModuleTrappingCorrectionAccess MDuplicate;
  bool DuplicateResult = MDuplicate.CallLoadTrappingCorrectionFile(Duplicate);
  UTNModuleTrappingCorrectionAccess MNoSection;
  bool NoSectionResult = MNoSection.CallLoadTrappingCorrectionFile(NoSection);
  UTNModuleTrappingCorrectionAccess MMissing;
  bool MissingResult = MMissing.CallLoadTrappingCorrectionFile(GetTemporaryFileName("missing.csv"));
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("LoadTrappingCorrectionFile()", "duplicate detector", "A duplicate detector section is rejected", DuplicateResult) && Passed;
  Passed = EvaluateFalse("LoadTrappingCorrectionFile()", "no detector section", "A file without detector section is rejected", NoSectionResult) && Passed;
  Passed = EvaluateFalse("LoadTrappingCorrectionFile()", "missing file", "A missing file is rejected", MissingResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // Timeout in seconds after which a hanging child is killed
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


bool UTNModuleTrappingCorrection::TestSectionWithoutDepthCurve()
{
  bool Passed = true;

  // Detector 3 has the parameter line but no depth curve
  MString FileName = GetTemporaryFileName("trapping_nocurve.csv");
  MString Standard = WriteStandardFile();
  Passed = EvaluateFalse("WriteStandardFile()", "standard file", "The fixture file is written", Standard.IsEmpty()) && Passed;
  MString C = ReadTextFile(Standard);
  C += "### 3\n";
  C += "1.0,1.0,0.5,0.25\n";
  Passed = EvaluateTrue("WriteTextFile()", "trapping_nocurve.csv", "The fixture file is written", WriteTextFile(FileName, C)) && Passed;

  bool Handled = RunInChild([FileName]() -> bool {
    MModuleTrappingCorrection M;
    M.SetTrappingCorrectionFileName(FileName);
    if (M.Initialize() == false) return true;
    MReadOutAssembly E;
    MStripHit* LV = new MStripHit();
    LV->SetDetectorID(3);
    LV->SetStripID(10);
    LV->IsLowVoltageStrip(true);
    LV->SetEnergy(500.0);
    E.AddStripHit(LV);
    MHit* H = new MHit();
    H->AddStripHit(LV);
    H->SetLocalPosition(MVector(0.0, 0.0, 0.5));
    H->SetEnergy(500.0);
    E.AddHit(H);
    M.AnalyzeEvent(&E);
    return true;
  });
  Passed = ExpectedToFail(EvaluateTrue("Initialize()/AnalyzeEvent()", "detector 3 without depth curve", "A detector section without depth curve is rejected or ignored, without crashing", Handled)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::TestInitialize()
{
  bool Passed = true;

  // The depth calibration module is looked up in the available modules of the supervisor; Run() registers it only after this sub-test
  MSupervisor* S = MSupervisor::GetSupervisor();
  bool HasDepthCalibration = (S->GetAvailableModuleByXmlTag("XmlTagDepthCalibration") != nullptr);
  Passed = EvaluateFalse("Initialize()", "no depth calibration module", "No depth calibration module is registered yet", HasDepthCalibration) && Passed;
  MString Standard = WriteStandardFile();
  Passed = EvaluateFalse("WriteStandardFile()", "standard file", "The fixture file is written", Standard.IsEmpty()) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MModuleTrappingCorrection Missing;
  Missing.SetTrappingCorrectionFileName(GetTemporaryFileName("missing.csv"));
  bool MissingResult = Missing.Initialize();

  bool NoDepthResult = true;
  if (HasDepthCalibration == false && Standard.IsEmpty() == false) {
    MModuleTrappingCorrection NoDepth;
    NoDepth.SetTrappingCorrectionFileName(Standard);
    NoDepthResult = NoDepth.Initialize();
  }

  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "missing file", "Initialize returns false for a missing file", MissingResult) && Passed;
  if (HasDepthCalibration == false && Standard.IsEmpty() == false) {
    Passed = EvaluateFalse("Initialize()", "no depth calibration module", "Initialize returns false without an available depth calibration module", NoDepthResult) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTrappingCorrection::TestAnalyzeEvent()
{
  bool Passed = true;

  MModuleTrappingCorrection M;
  M.SetTrappingCorrectionFileName(WriteStandardFile());
  if (M.Initialize() == false) {
    return EvaluateTrue("Initialize()", "standard file", "Initialize succeeds with an available depth calibration module", false);
  }

  // Depth 0.5: HV CCE_e = 0.75, CCE_h = 0.85; LV CCE_e = 0.95, CCE_h = 0.7
  double HVScale = (1 - 0.5*0.25)*(1 - 0.25*0.15);
  double LVScale = (1 - 0.5*0.05)*(1 - 0.25*0.30);
  {
    MReadOutAssembly E;
    FillEvent(E, 0, 0.5, 500.0, 400.0);
    Passed = EvaluateTrue("AnalyzeEvent()", "depth 0.5", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "depth 0.5", "The c_TrappingCorrection analysis progress is set", E.HasAnalysisProgress(MAssembly::c_TrappingCorrection)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "depth 0.5", "The two strip hits are kept", E.GetNStripHits(), 2u) && Passed;
    if (E.GetNStripHits() == 2) {
      Passed = EvaluateNear("AnalyzeEvent()", "depth 0.5, LV 500 keV", "The LV energy is corrected with the LV CCE curves", E.GetStripHit(0)->GetEnergy(), 500.0/LVScale, 1e-9) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "depth 0.5, HV 400 keV", "The HV energy is corrected with the HV CCE curves", E.GetStripHit(1)->GetEnergy(), 400.0/HVScale, 1e-9) && Passed;
    }
    // The hit energy is what the saver and Revan use: it must follow the corrected strips, not keep the uncorrected 450 keV
    Passed = Evaluate("AnalyzeEvent()", "depth 0.5", "The hit is kept", E.GetNHits(), 1u) && Passed;
    if (E.GetNHits() == 1) {
      double Low = min(400.0/HVScale, 500.0/LVScale);
      double High = max(400.0/HVScale, 500.0/LVScale);
      Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "depth 0.5, hit of 450 keV", "The hit energy lies between the corrected HV (474.9 keV) and LV (554.4 keV) strip energies", E.GetHit(0)->GetEnergy(), 0.5*(Low + High), 0.5*(High - Low) + 1e-9)) && Passed;
    }
  }

  // Out of bounds: clamped to the grid edges
  {
    MReadOutAssembly E;
    FillEvent(E, 0, 2.5, 500.0, 500.0);
    M.AnalyzeEvent(&E);
    double HVScaleTop = (1 - 0.5*0.3)*(1 - 0.25*0.2);
    double LVScaleTop = (1 - 0.5*0.0)*(1 - 0.25*0.4);
    if (E.GetNStripHits() == 2) {
      Passed = EvaluateNear("AnalyzeEvent()", "depth 2.5", "Depths above the grid use the top grid point (LV)", E.GetStripHit(0)->GetEnergy(), 500.0/LVScaleTop, 1e-9) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "depth 2.5", "Depths above the grid use the top grid point (HV)", E.GetStripHit(1)->GetEnergy(), 500.0/HVScaleTop, 1e-9) && Passed;
    } else {
      Passed = Evaluate("AnalyzeEvent()", "depth 2.5", "The two strip hits are kept", E.GetNStripHits(), 2u) && Passed;
    }
  }
  {
    MReadOutAssembly E;
    FillEvent(E, 0, -3.0, 500.0, 500.0);
    M.AnalyzeEvent(&E);
    double HVScaleBottom = (1 - 0.5*0.1)*(1 - 0.25*0.0);
    if (E.GetNStripHits() == 2) {
      Passed = EvaluateNear("AnalyzeEvent()", "depth -3", "Depths below the grid use the bottom grid point (HV)", E.GetStripHit(1)->GetEnergy(), 500.0/HVScaleBottom, 1e-9) && Passed;
    } else {
      Passed = Evaluate("AnalyzeEvent()", "depth -3", "The two strip hits are kept", E.GetNStripHits(), 2u) && Passed;
    }
  }

  // No depth: unchanged
  {
    MReadOutAssembly E;
    FillEvent(E, 0, 0.5, 500.0, 400.0);
    E.GetHit(0)->SetNoDepth(true);
    M.AnalyzeEvent(&E);
    if (E.GetNStripHits() == 2) {
      Passed = EvaluateNear("AnalyzeEvent()", "no depth", "Hits without depth are not corrected (LV)", E.GetStripHit(0)->GetEnergy(), 500.0, 1e-12) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "no depth", "Hits without depth are not corrected (HV)", E.GetStripHit(1)->GetEnergy(), 400.0, 1e-12) && Passed;
    } else {
      Passed = Evaluate("AnalyzeEvent()", "no depth", "The two strip hits are kept", E.GetNStripHits(), 2u) && Passed;
    }
  }

  // Unknown detector: unchanged
  {
    MReadOutAssembly E;
    FillEvent(E, 5, 0.5, 500.0, 400.0);
    int OldVerbosity = g_Verbosity;
    g_Verbosity = c_Quiet;
    M.AnalyzeEvent(&E);
    g_Verbosity = OldVerbosity;
    if (E.GetNStripHits() == 2) {
      Passed = EvaluateNear("AnalyzeEvent()", "unknown detector 5", "Hits in detectors without trapping data are not corrected", E.GetStripHit(0)->GetEnergy(), 500.0, 1e-12) && Passed;
    } else {
      Passed = Evaluate("AnalyzeEvent()", "unknown detector 5", "The two strip hits are kept", E.GetNStripHits(), 2u) && Passed;
    }
  }

  // Guard ring veto: the event passes unchanged, but still through the module
  {
    MReadOutAssembly E;
    FillEvent(E, 0, 0.5, 500.0, 400.0);
    E.SetGuardRingVeto(true);
    bool Result = M.AnalyzeEvent(&E);
    if (E.GetNStripHits() == 2) {
      Passed = EvaluateNear("AnalyzeEvent()", "guard ring veto", "Vetoed events are not corrected", E.GetStripHit(0)->GetEnergy(), 500.0, 1e-12) && Passed;
    } else {
      Passed = Evaluate("AnalyzeEvent()", "guard ring veto", "The two strip hits are kept", E.GetNStripHits(), 2u) && Passed;
    }
    Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "guard ring veto", "AnalyzeEvent returns true for vetoed events", Result)) && Passed;
    Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "guard ring veto", "The c_TrappingCorrection analysis progress is set for vetoed events", E.HasAnalysisProgress(MAssembly::c_TrappingCorrection))) && Passed;
  }

  // A strip hit shared by two hits must only be corrected once
  {
    MReadOutAssembly E;
    FillEvent(E, 0, 0.5, 500.0, 400.0);
    MStripHit* HV2 = new MStripHit();
    HV2->SetDetectorID(0);
    HV2->SetStripID(21);
    HV2->IsLowVoltageStrip(false);
    HV2->SetEnergy(100.0);
    E.AddStripHit(HV2);
    MHit* H2 = new MHit();
    H2->AddStripHit(E.GetStripHit(0));
    H2->AddStripHit(HV2);
    H2->SetLocalPosition(MVector(0.0, 0.0, 0.5));
    E.AddHit(H2);
    M.AnalyzeEvent(&E);
    if (E.GetNStripHits() == 3) {
      Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "LV strip shared by two hits", "A strip hit shared by two hits is corrected only once", E.GetStripHit(0)->GetEnergy(), 500.0/LVScale, 1e-9)) && Passed;
    } else {
      Passed = Evaluate("AnalyzeEvent()", "LV strip shared by two hits", "The three strip hits are kept", E.GetNStripHits(), 3u) && Passed;
    }
  }

  M.Finalize();

  // Descending depth grid gives the same correction
  MModuleTrappingCorrection D;
  D.SetTrappingCorrectionFileName(WriteStandardFile(true));
  if (D.Initialize() == true) {
    MReadOutAssembly E;
    FillEvent(E, 0, 0.5, 500.0, 400.0);
    D.AnalyzeEvent(&E);
    if (E.GetNStripHits() == 2) {
      Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "descending depth grid", "A descending depth grid gives the same LV correction", E.GetStripHit(0)->GetEnergy(), 500.0/LVScale, 1e-9)) && Passed;
      Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "descending depth grid", "A descending depth grid gives the same HV correction", E.GetStripHit(1)->GetEnergy(), 400.0/HVScale, 1e-9)) && Passed;
    } else {
      Passed = Evaluate("AnalyzeEvent()", "descending depth grid", "The two strip hits are kept", E.GetNStripHits(), 2u) && Passed;
    }
    D.Finalize();
  } else {
    Passed = EvaluateTrue("Initialize()", "descending depth grid", "Initialize succeeds for a descending depth grid", false) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleTrappingCorrection Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleTrappingCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
