/*
 * UTNModuleTACCalibration.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <unordered_map>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleTACCalibration.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MModuleTACCalibration
class UTNModuleTACCalibration : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleTACCalibration() : MUnitTest("UTNModuleTACCalibration") {}
  //! Default destructor
  virtual ~UTNModuleTACCalibration() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and defaults
  bool TestDefaultConstruction();
  //! Test all setter/getter pairs
  bool TestGettersSetters();
  //! Test the XML configuration round trip
  bool TestXmlRoundTrip();
  //! Test LoadTACCalFile() with both file formats and error cases
  bool TestLoadTACCalFile();
  //! Test the Initialize() error paths
  bool TestInitialize();
  //! Test the TAC to ns conversion in AnalyzeEvent()
  bool TestAnalyzeEventCalibration();
  //! Test the handling of missing calibrations in AnalyzeEvent()
  bool TestAnalyzeEventMissingCalibration();
  //! Test the TAC cuts in AnalyzeEvent()
  bool TestAnalyzeEventCuts();
  //! Test Finalize()
  bool TestFinalize();

  //! Write the standard calibration file and return its name, or an empty string on failure
  MString WriteStandardCalibration();
  //! Create a strip hit
  MStripHit* CreateStripHit(unsigned int DetectorID, unsigned int StripID, bool IsLowVoltage, double TAC, bool FastTiming = true);
  //! Return the strip hit with the given strip ID and side or nullptr
  MStripHit* FindStripHit(MReadOutAssembly& Event, unsigned int StripID, bool IsLowVoltage);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestLoadTACCalFile() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestAnalyzeEventCalibration() && Passed;
  Passed = TestAnalyzeEventMissingCalibration() && Passed;
  Passed = TestAnalyzeEventCuts() && Passed;
  Passed = TestFinalize() && Passed;

  // ShowOptionsGUI() and CreateExpos() create GUI elements and are not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleTACCalibration::WriteStandardCalibration()
{
  // Detector 0: LV strips 1-10 and HV strips 1-10: Timing [ns] = 0.5 * TAC - 500
  // Detector 0 HV strip 20: Timing = 1.0 * TAC + 0
  MString Content = "#STRIP_ID,Detector,Side,Strip,TAC_calibration,TAC_calibration_error,TAC_offset,TAC_offset_error\n";
  for (unsigned int s = 1; s <= 10; ++s) {
    Content += MString("") + s + ",0,0," + s + ",0.5,0.001,-500.0,10.0\n";
    Content += MString("") + (s + 100) + ",0,1," + s + ",0.5,0.001,-500.0,10.0\n";
  }
  Content += "200,0,1,20,1.0,0.002,0.0,5.0\n";

  MString FileName = GetTemporaryFileName("standard.taccal.csv");
  if (WriteTextFile(FileName, Content) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleTACCalibration::CreateStripHit(unsigned int DetectorID, unsigned int StripID, bool IsLowVoltage, double TAC, bool FastTiming)
{
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(DetectorID);
  SH->SetStripID(StripID);
  SH->IsLowVoltageStrip(IsLowVoltage);
  SH->SetTAC(TAC);
  SH->HasFastTiming(FastTiming);
  SH->SetEnergy(100.0);
  return SH;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleTACCalibration::FindStripHit(MReadOutAssembly& Event, unsigned int StripID, bool IsLowVoltage)
{
  for (unsigned int i = 0; i < Event.GetNStripHits(); ++i) {
    MStripHit* SH = Event.GetStripHit(i);
    if (SH->GetStripID() == StripID && SH->IsLowVoltageStrip() == IsLowVoltage) return SH;
  }
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleTACCalibration M;
  Passed = Evaluate("GetName()", "default", "The module name is 'TAC Calibration'", M.GetName(), MString("TAC Calibration")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagTACCalibration'", M.GetXmlTag(), MString("XmlTagTACCalibration")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_TACCalibration", M.ProvidesModuleType(MAssembly::c_TACCalibration)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "An event loader is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "default", "The module allows multi-threading", M.AllowsMultiThreading()) && Passed;
  Passed = EvaluateTrue("AllowsMultipleInstances()", "default", "The module allows multiple instances", M.AllowsMultipleInstances()) && Passed;
  Passed = Evaluate("GetTACCalFileName()", "default", "The default calibration file name is empty", M.GetTACCalFileName(), MString("")) && Passed;
  Passed = EvaluateTrue("GetApplyTACCuts()", "default", "TAC cuts are applied by default", M.GetApplyTACCuts()) && Passed;
  Passed = EvaluateNear("GetCoincidenceWindow()", "default", "The default coincidence window is 600 ns", M.GetCoincidenceWindow(), 600.0, 1e-12) && Passed;
  Passed = Evaluate("GetTACCalParameters()", "default", "The default calibration is empty", (unsigned int) M.GetTACCalParameters().size(), 0u) && Passed;

  MModuleTACCalibration* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestGettersSetters()
{
  bool Passed = true;

  MModuleTACCalibration M;
  M.SetTACCalFileName("my.taccal.csv");
  Passed = Evaluate("SetTACCalFileName/GetTACCalFileName", "my.taccal.csv", "GetTACCalFileName returns the set name", M.GetTACCalFileName(), MString("my.taccal.csv")) && Passed;
  M.SetApplyTACCuts(false);
  Passed = EvaluateFalse("SetApplyTACCuts/GetApplyTACCuts", "false", "GetApplyTACCuts returns false", M.GetApplyTACCuts()) && Passed;
  M.SetApplyTACCuts(true);
  Passed = EvaluateTrue("SetApplyTACCuts/GetApplyTACCuts", "true", "GetApplyTACCuts returns true", M.GetApplyTACCuts()) && Passed;
  M.SetCoincidenceWindow(350.5);
  Passed = EvaluateNear("SetCoincidenceWindow/GetCoincidenceWindow", "350.5", "GetCoincidenceWindow returns 350.5", M.GetCoincidenceWindow(), 350.5, 1e-12) && Passed;

  unordered_map<int, vector<unordered_map<int, vector<double>>>> Cal;
  Cal[3] = vector<unordered_map<int, vector<double>>>(2);
  Cal[3][1][17] = { 0.45, -600.0, 0.001, 5.0 };
  M.SetTACCalParameters(Cal);
  auto Got = M.GetTACCalParameters();
  Passed = Evaluate("SetTACCalParameters/GetTACCalParameters", "one entry", "The calibration contains one detector", (unsigned int) Got.size(), 1u) && Passed;
  if (Got.find(3) != Got.end() && Got[3].size() == 2 && Got[3][1].find(17) != Got[3][1].end()) {
    Passed = EvaluateNear("SetTACCalParameters/GetTACCalParameters", "one entry", "The slope is 0.45", Got[3][1][17][0], 0.45, 1e-12) && Passed;
    Passed = EvaluateNear("SetTACCalParameters/GetTACCalParameters", "one entry", "The offset is -600", Got[3][1][17][1], -600.0, 1e-12) && Passed;
  } else {
    Passed = EvaluateTrue("SetTACCalParameters/GetTACCalParameters", "one entry", "Detector 3, HV, strip 17 exists", false) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleTACCalibration Writer;
  Writer.SetTACCalFileName("/some/path/cal.csv");
  Writer.SetApplyTACCuts(false);
  Writer.SetCoincidenceWindow(450.25);
  MXmlNode* Node = Writer.CreateXmlConfiguration();

  MModuleTACCalibration Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "configured", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "TACCalFileName", "The file name survives", Reader.GetTACCalFileName(), MString("/some/path/cal.csv")) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "ApplyTACCuts", "ApplyTACCuts false survives", Reader.GetApplyTACCuts()) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "CoincidenceWindow", "The coincidence window survives", Reader.GetCoincidenceWindow(), 450.25, 1e-9) && Passed;
  delete Node;

  // Partial node keeps defaults
  MXmlNode* Partial = new MXmlNode(nullptr, "XmlTagTACCalibration");
  new MXmlNode(Partial, "TACCalFileName", MString("x.csv"));
  MModuleTACCalibration PartialReader;
  PartialReader.ReadXmlConfiguration(Partial);
  Passed = Evaluate("ReadXmlConfiguration()", "partial node", "The file name is read", PartialReader.GetTACCalFileName(), MString("x.csv")) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "partial node", "ApplyTACCuts keeps its default", PartialReader.GetApplyTACCuts()) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "partial node", "The coincidence window keeps its default", PartialReader.GetCoincidenceWindow(), 600.0, 1e-12) && Passed;
  delete Partial;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestLoadTACCalFile()
{
  bool Passed = true;

  // 8-column format: ReadOutID, Detector, Side, Strip, Cal, CalError, Offset, OffsetError
  MString File8 = GetTemporaryFileName("eight.csv");
  Passed = EvaluateTrue("WriteTextFile()", "eight.csv", "The fixture file is written", WriteTextFile(File8, "#header\n64,0,0,32,0.43,0.0006,-519.4,6.5\n65,2,1,33,0.44,0.0002,-510.4,1.8\n")) && Passed;
  MModuleTACCalibration M8;
  Passed = EvaluateTrue("LoadTACCalFile()", "8-column format", "Loading the 8-column format succeeds", M8.LoadTACCalFile(File8)) && Passed;
  auto C8 = M8.GetTACCalParameters();
  Passed = Evaluate("LoadTACCalFile()", "8-column format", "Two detectors are loaded", (unsigned int) C8.size(), 2u) && Passed;
  if (C8.find(0) != C8.end() && C8[0].size() == 2 && C8[0][0].find(32) != C8[0][0].end()) {
    vector<double>& V = C8[0][0][32];
    Passed = Evaluate("LoadTACCalFile()", "8-column format", "Four values are stored", (unsigned int) V.size(), 4u) && Passed;
    if (V.size() == 4) {
      Passed = EvaluateNear("LoadTACCalFile()", "8-column format", "The first value is the slope", V[0], 0.43, 1e-12) && Passed;
      Passed = EvaluateNear("LoadTACCalFile()", "8-column format", "The second value is the offset", V[1], -519.4, 1e-12) && Passed;
      Passed = EvaluateNear("LoadTACCalFile()", "8-column format", "The third value is the slope error", V[2], 0.0006, 1e-12) && Passed;
      Passed = EvaluateNear("LoadTACCalFile()", "8-column format", "The fourth value is the offset error", V[3], 6.5, 1e-12) && Passed;
    }
  } else {
    Passed = EvaluateTrue("LoadTACCalFile()", "8-column format", "Detector 0, LV, strip 32 exists", false) && Passed;
  }
  Passed = EvaluateTrue("LoadTACCalFile()", "8-column format", "Detector 2 is loaded", C8.find(2) != C8.end()) && Passed;
  if (C8.find(2) != C8.end()) {
    Passed = Evaluate("LoadTACCalFile()", "8-column format", "Detector 2 has LV and HV calibrations", (unsigned int) C8[2].size(), 2u) && Passed;
  }
  if (C8.find(2) != C8.end() && C8[2].size() == 2) {
    Passed = EvaluateTrue("LoadTACCalFile()", "8-column format", "Side 1 is stored as HV", C8[2][1].find(33) != C8[2][1].end()) && Passed;
    Passed = EvaluateTrue("LoadTACCalFile()", "8-column format", "Side 1 is not stored as LV", C8[2][0].find(33) == C8[2][0].end()) && Passed;
  }

  // 7-column format with letter sides
  MString File7 = GetTemporaryFileName("seven.csv");
  Passed = EvaluateTrue("WriteTextFile()", "seven.csv", "The fixture file is written", WriteTextFile(File7, "1,l,5,0.5,0.01,-500,1\n1,h,6,0.6,0.01,-400,1\n1,p,7,0.7,0.01,-300,1\n1,n,8,0.8,0.01,-200,1\n")) && Passed;
  MModuleTACCalibration M7;
  Passed = EvaluateTrue("LoadTACCalFile()", "7-column format", "Loading the 7-column format succeeds", M7.LoadTACCalFile(File7)) && Passed;
  auto C7 = M7.GetTACCalParameters();
  if (C7.find(1) != C7.end() && C7[1].size() == 2) {
    Passed = EvaluateTrue("LoadTACCalFile()", "7-column format, side l", "Side 'l' is LV", C7[1][0].find(5) != C7[1][0].end()) && Passed;
    Passed = EvaluateTrue("LoadTACCalFile()", "7-column format, side h", "Side 'h' is HV", C7[1][1].find(6) != C7[1][1].end()) && Passed;
    Passed = EvaluateTrue("LoadTACCalFile()", "7-column format, side p", "Side 'p' is LV", C7[1][0].find(7) != C7[1][0].end()) && Passed;
    Passed = EvaluateTrue("LoadTACCalFile()", "7-column format, side n", "Side 'n' is HV", C7[1][1].find(8) != C7[1][1].end()) && Passed;
    if (C7[1][1].find(6) != C7[1][1].end()) {
      Passed = EvaluateNear("LoadTACCalFile()", "7-column format, side h", "The slope of strip 6 is 0.6", C7[1][1][6][0], 0.6, 1e-12) && Passed;
      Passed = EvaluateNear("LoadTACCalFile()", "7-column format, side h", "The offset of strip 6 is -400", C7[1][1][6][1], -400.0, 1e-12) && Passed;
    }
  } else {
    Passed = EvaluateTrue("LoadTACCalFile()", "7-column format", "Detector 1 exists", false) && Passed;
  }

  // Spaces after the commas, as written by spreadsheet tools
  MString FileSpaces = GetTemporaryFileName("spaces.csv");
  Passed = EvaluateTrue("WriteTextFile()", "spaces.csv", "The fixture file is written", WriteTextFile(FileSpaces, "3, l, 9, 0.5, 0.01, -500, 1\n")) && Passed;
  MModuleTACCalibration MSpaces;
  Passed = EvaluateTrue("LoadTACCalFile()", "spaces after commas", "Loading a file with spaces after the commas succeeds", MSpaces.LoadTACCalFile(FileSpaces)) && Passed;
  auto CS = MSpaces.GetTACCalParameters();
  Passed = EvaluateTrue("LoadTACCalFile()", "spaces after commas", "Detector 3 is loaded", CS.find(3) != CS.end()) && Passed;
  if (CS.find(3) != CS.end() && CS[3].size() == 2) {
    Passed = EvaluateTrue("LoadTACCalFile()", "spaces after commas", "Detector 3, LV strip 9 is loaded", CS[3][0].find(9) != CS[3][0].end()) && Passed;
  }

  // Lines with the wrong number of columns are ignored
  MString FileWrong = GetTemporaryFileName("wrong.csv");
  Passed = EvaluateTrue("WriteTextFile()", "wrong.csv", "The fixture file is written", WriteTextFile(FileWrong, "1,l,5,0.5\n1,l,6,0.5,0.01,-500,1\n")) && Passed;
  MModuleTACCalibration MWrong;
  Passed = EvaluateTrue("LoadTACCalFile()", "short line", "A line with too few columns does not stop the loading", MWrong.LoadTACCalFile(FileWrong)) && Passed;
  auto CW = MWrong.GetTACCalParameters();
  Passed = EvaluateTrue("LoadTACCalFile()", "short line", "Detector 1 is loaded", CW.find(1) != CW.end()) && Passed;
  if (CW.find(1) != CW.end()) {
    Passed = Evaluate("LoadTACCalFile()", "short line", "Detector 1 has LV and HV calibrations", (unsigned int) CW[1].size(), 2u) && Passed;
  }
  if (CW.find(1) != CW.end() && CW[1].size() == 2) {
    Passed = Evaluate("LoadTACCalFile()", "short line", "Only the complete line is loaded", (unsigned int) CW[1][0].size(), 1u) && Passed;
  }

  // Error cases
  MString FileBadSide = GetTemporaryFileName("badside.csv");
  Passed = EvaluateTrue("WriteTextFile()", "badside.csv", "The fixture file is written", WriteTextFile(FileBadSide, "1,x,5,0.5,0.01,-500,1\n")) && Passed;
  MString FileLongSide = GetTemporaryFileName("longside.csv");
  Passed = EvaluateTrue("WriteTextFile()", "longside.csv", "The fixture file is written", WriteTextFile(FileLongSide, "1,lv,5,0.5,0.01,-500,1\n")) && Passed;
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MModuleTACCalibration MBadSide;
  bool BadSide = MBadSide.LoadTACCalFile(FileBadSide);
  MModuleTACCalibration MLongSide;
  bool LongSide = MLongSide.LoadTACCalFile(FileLongSide);
  MModuleTACCalibration MMissing;
  bool Missing = MMissing.LoadTACCalFile(GetTemporaryFileName("doesnotexist.csv"));
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("LoadTACCalFile()", "side 'x'", "An unknown side character is rejected", BadSide) && Passed;
  Passed = EvaluateFalse("LoadTACCalFile()", "side 'lv'", "A multi-character side is rejected", LongSide) && Passed;
  Passed = EvaluateFalse("LoadTACCalFile()", "missing file", "A missing file is rejected", Missing) && Passed;

  // Loading a second file replaces the first calibration
  MModuleTACCalibration MTwice;
  MTwice.LoadTACCalFile(File8);
  MTwice.LoadTACCalFile(File7);
  auto CT = MTwice.GetTACCalParameters();
  Passed = ExpectedToFail(Evaluate("LoadTACCalFile()", "two files", "Loading a second file replaces the calibration of the first file", (unsigned int) CT.size(), 1u)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestInitialize()
{
  bool Passed = true;

  MString EmptyFile = GetTemporaryFileName("empty.csv");
  Passed = EvaluateTrue("WriteTextFile()", "empty.csv", "The fixture file is written", WriteTextFile(EmptyFile, "#STRIP_ID,Detector,Side,Strip,TAC_calibration,TAC_calibration_error,TAC_offset,TAC_offset_error\n")) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MModuleTACCalibration Missing;
  Missing.SetTACCalFileName(GetTemporaryFileName("missing.csv"));
  bool MissingResult = Missing.Initialize();

  MModuleTACCalibration Empty;
  Empty.SetTACCalFileName(EmptyFile);
  bool EmptyResult = Empty.Initialize();

  MModuleTACCalibration NoName;
  bool NoNameResult = NoName.Initialize();

  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "missing file", "Initialize returns false for a missing calibration file", MissingResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "comments only", "Initialize returns false for a calibration file without entries", EmptyResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "no file name", "Initialize returns false without a file name", NoNameResult) && Passed;

  MModuleTACCalibration Good;
  Good.SetTACCalFileName(WriteStandardCalibration());
  Passed = EvaluateTrue("Initialize()", "standard file", "Initialize returns true for a valid calibration file", Good.Initialize()) && Passed;
  Passed = Evaluate("Initialize()", "standard file", "The calibration of detector 0 is loaded", (unsigned int) Good.GetTACCalParameters().size(), 1u) && Passed;
  Good.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestAnalyzeEventCalibration()
{
  bool Passed = true;

  MModuleTACCalibration M;
  M.SetTACCalFileName(WriteStandardCalibration());
  M.SetApplyTACCuts(false);
  if (M.Initialize() == false) {
    return EvaluateTrue("Initialize()", "standard file", "Initialize succeeds for the calibration test", false);
  }

  MReadOutAssembly E;
  E.AddStripHit(CreateStripHit(0, 3, true, 10000));
  E.AddStripHit(CreateStripHit(0, 4, false, 9000));
  E.AddStripHit(CreateStripHit(0, 20, false, 4321));
  MStripHit* GR = CreateStripHit(0, 5, true, 7000);
  GR->IsGuardRing(true);
  GR->SetTiming(-1.0);
  E.AddStripHit(GR);

  Passed = EvaluateTrue("AnalyzeEvent()", "calibration only", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "calibration only", "The c_TACCalibration analysis progress is set", E.HasAnalysisProgress(MAssembly::c_TACCalibration)) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "calibration only", "No TAC calibration error is set", E.HasTACCalibrationError()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "calibration only", "Without cuts no strip hit is removed", E.GetNStripHits(), 4u) && Passed;

  MStripHit* LV3 = FindStripHit(E, 3, true);
  MStripHit* HV4 = FindStripHit(E, 4, false);
  MStripHit* HV20 = FindStripHit(E, 20, false);
  MStripHit* GR5 = FindStripHit(E, 5, true);
  Passed = EvaluateTrue("AnalyzeEvent()", "calibration only", "Strip hit LV 3 is found", LV3 != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "calibration only", "Strip hit HV 4 is found", HV4 != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "calibration only", "Strip hit HV 20 is found", HV20 != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "calibration only", "Guard ring hit 5 is found", GR5 != nullptr) && Passed;
  if (LV3 != nullptr) Passed = EvaluateNear("AnalyzeEvent()", "LV strip 3, TAC 10000", "The timing is 0.5 * 10000 - 500 = 4500 ns", LV3->GetTiming(), 4500.0, 1e-9) && Passed;
  if (HV4 != nullptr) Passed = EvaluateNear("AnalyzeEvent()", "HV strip 4, TAC 9000", "The timing is 0.5 * 9000 - 500 = 4000 ns", HV4->GetTiming(), 4000.0, 1e-9) && Passed;
  if (HV20 != nullptr) Passed = EvaluateNear("AnalyzeEvent()", "HV strip 20, TAC 4321", "The timing uses the HV strip 20 calibration: 4321 ns", HV20->GetTiming(), 4321.0, 1e-9) && Passed;
  if (GR5 != nullptr) Passed = EvaluateNear("AnalyzeEvent()", "guard ring", "Guard ring hits are not TAC calibrated", GR5->GetTiming(), -1.0, 1e-12) && Passed;
  if (LV3 != nullptr) Passed = EvaluateNear("AnalyzeEvent()", "LV strip 3", "The raw TAC value is not modified", LV3->GetTAC(), 10000.0, 1e-12) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestAnalyzeEventMissingCalibration()
{
  bool Passed = true;

  MModuleTACCalibration M;
  M.SetTACCalFileName(WriteStandardCalibration());
  if (M.Initialize() == false) {
    return EvaluateTrue("Initialize()", "standard file", "Initialize succeeds for the missing-calibration test", false);
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  // Unknown detector
  MReadOutAssembly E1;
  E1.AddStripHit(CreateStripHit(7, 3, true, 10000));
  E1.AddStripHit(CreateStripHit(7, 4, false, 1000));
  bool R1 = M.AnalyzeEvent(&E1);

  // Unknown strip in a known detector
  MReadOutAssembly E2;
  E2.AddStripHit(CreateStripHit(0, 3, true, 10000));
  E2.AddStripHit(CreateStripHit(0, 55, false, 9000));
  bool R2 = M.AnalyzeEvent(&E2);

  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("AnalyzeEvent()", "unknown detector 7", "AnalyzeEvent returns true", R1) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "unknown detector 7", "A TAC calibration error is set", E1.HasTACCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "unknown detector 7", "The c_TACCalibration analysis progress is set", E1.HasAnalysisProgress(MAssembly::c_TACCalibration)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "unknown detector 7", "No TAC cut is applied to an event with a calibration error", E1.GetNStripHits(), 2u) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "unknown detector 7", "The event with a TAC calibration error is bad", E1.IsBad()) && Passed;

  Passed = EvaluateTrue("AnalyzeEvent()", "unknown strip 55", "AnalyzeEvent returns true", R2) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "unknown strip 55", "A TAC calibration error is set", E2.HasTACCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "unknown strip 55", "The c_TACCalibration analysis progress is set", E2.HasAnalysisProgress(MAssembly::c_TACCalibration)) && Passed;
  MStripHit* LV3 = FindStripHit(E2, 3, true);
  Passed = EvaluateTrue("AnalyzeEvent()", "unknown strip 55", "The calibrated strip hit LV 3 is found", LV3 != nullptr) && Passed;
  if (LV3 != nullptr) Passed = EvaluateNear("AnalyzeEvent()", "unknown strip 55", "The calibrated strip in the same event is still calibrated", LV3->GetTiming(), 4500.0, 1e-9) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestAnalyzeEventCuts()
{
  bool Passed = true;

  MModuleTACCalibration M;
  M.SetTACCalFileName(WriteStandardCalibration());
  M.SetApplyTACCuts(true);
  M.SetCoincidenceWindow(600.0);
  if (M.Initialize() == false) {
    return EvaluateTrue("Initialize()", "standard file", "Initialize succeeds for the cut test", false);
  }

  // Timing = 0.5 * TAC - 500
  MReadOutAssembly E;
  E.AddStripHit(CreateStripHit(0, 1, true, 10000));        // 4500 ns: maximum, kept
  E.AddStripHit(CreateStripHit(0, 2, false, 9000));        // 4000 ns: within 600 ns of the maximum, kept
  E.AddStripHit(CreateStripHit(0, 3, true, 8600));         // 3800 ns: outside the coincidence window, removed
  E.AddStripHit(CreateStripHit(0, 4, false, 6000));        // 2500 ns: below 3000 ns, removed
  E.AddStripHit(CreateStripHit(0, 5, true, 1300, false));  // 150 ns slow timing: below the noise cut, removed
  E.AddStripHit(CreateStripHit(0, 6, false, 1500, false)); // 250 ns slow timing: kept
  MStripHit* NN = CreateStripHit(0, 7, true, 12000);       // 5500 ns nearest neighbor: does not define the maximum, kept
  NN->IsNearestNeighbor(true);
  E.AddStripHit(NN);
  MStripHit* GR = CreateStripHit(0, 8, false, 100);        // guard ring: never cut
  GR->IsGuardRing(true);
  E.AddStripHit(GR);

  Passed = EvaluateTrue("AnalyzeEvent()", "with cuts", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "with cuts", "Five strip hits survive the cuts", E.GetNStripHits(), 5u) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "with cuts, 4500 ns", "The latest fast hit is kept", FindStripHit(E, 1, true) != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "with cuts, 4000 ns", "A fast hit within the coincidence window is kept", FindStripHit(E, 2, false) != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "with cuts, 3800 ns", "A fast hit outside the coincidence window is removed", FindStripHit(E, 3, true) == nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "with cuts, 2500 ns", "A fast hit below 3000 ns is removed", FindStripHit(E, 4, false) == nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "with cuts, slow 150 ns", "A slow hit below 200 ns is removed", FindStripHit(E, 5, true) == nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "with cuts, slow 250 ns", "A slow hit above 200 ns is kept", FindStripHit(E, 6, false) != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "with cuts, nearest neighbor", "The nearest neighbor hit is kept", FindStripHit(E, 7, true) != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "with cuts, guard ring", "The guard ring hit is kept", FindStripHit(E, 8, false) != nullptr) && Passed;

  // Boundary: exactly at the edge of the coincidence window and at 3000 ns
  MReadOutAssembly B;
  B.AddStripHit(CreateStripHit(0, 1, true, 10000));        // 4500 ns
  B.AddStripHit(CreateStripHit(0, 2, false, 8800));        // 3900 ns = 4500 - 600: kept
  B.AddStripHit(CreateStripHit(0, 3, true, 7000, false));  // 3000 ns slow
  B.AddStripHit(CreateStripHit(0, 4, false, 1400, false)); // 200 ns slow: removed (<= 200)
  M.AnalyzeEvent(&B);
  Passed = EvaluateTrue("AnalyzeEvent()", "boundary 3900 ns", "A fast hit exactly at the coincidence window edge is kept", FindStripHit(B, 2, false) != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "boundary slow 200 ns", "A slow hit at exactly 200 ns is removed", FindStripHit(B, 4, false) == nullptr) && Passed;

  // A smaller coincidence window removes more hits
  M.SetCoincidenceWindow(100.0);
  MReadOutAssembly W;
  W.AddStripHit(CreateStripHit(0, 1, true, 10000));  // 4500 ns
  W.AddStripHit(CreateStripHit(0, 2, false, 9000));  // 4000 ns: outside 100 ns
  M.AnalyzeEvent(&W);
  Passed = Evaluate("AnalyzeEvent()", "window 100 ns", "Only the latest hit survives a 100 ns window", W.GetNStripHits(), 1u) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestFinalize()
{
  bool Passed = true;

  MModuleTACCalibration M;
  M.SetTACCalFileName(WriteStandardCalibration());
  Passed = EvaluateTrue("Initialize()", "standard file", "Initialize succeeds", M.Initialize()) && Passed;
  Passed = Evaluate("Initialize()", "standard file", "The calibration is loaded", (unsigned int) M.GetTACCalParameters().size(), 1u) && Passed;
  M.Finalize();
  Passed = Evaluate("Finalize()", "after Initialize", "Finalize clears the calibration", (unsigned int) M.GetTACCalParameters().size(), 0u) && Passed;

  // Re-initialization after Finalize works
  Passed = EvaluateTrue("Initialize()", "after Finalize", "A second Initialize succeeds", M.Initialize()) && Passed;
  Passed = Evaluate("Initialize()", "after Finalize", "The calibration is loaded again", (unsigned int) M.GetTACCalParameters().size(), 1u) && Passed;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleTACCalibration Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleTACCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
