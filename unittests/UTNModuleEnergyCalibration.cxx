/*
 * UTNModuleEnergyCalibration.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <map>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MReadOutElementDoubleStrip.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleEnergyCalibration.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MModuleEnergyCalibration
class UTNModuleEnergyCalibration : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleEnergyCalibration() : MUnitTest("UTNModuleEnergyCalibration") {}
  //! Default destructor
  virtual ~UTNModuleEnergyCalibration() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and defaults
  bool TestDefaultConstruction();
  //! Test all setter/getter pairs
  bool TestGettersSetters();
  //! Test the XML configuration round trip
  bool TestXmlRoundTrip();
  //! Test ReadEnergyCalibrationFile() with all calibrator types
  bool TestReadEnergyCalibrationFile();
  //! Test ReadSlowThresholdCutFile()
  bool TestReadSlowThresholdCutFile();
  //! Test GetEnergy(), GetADC(), and LookupEnergyResolution()
  bool TestStandaloneLookups();
  //! Test Initialize() including its error paths
  bool TestInitialize();
  //! Test the calibration in AnalyzeEvent()
  bool TestAnalyzeEventCalibration();
  //! Test the thresholds in AnalyzeEvent()
  bool TestAnalyzeEventThresholds();
  //! Test Finalize() and re-initialization
  bool TestFinalize();

  //! Write the standard energy calibration file and return its name
  MString WriteStandardCalibration();
  //! Write the standard threshold file and return its name
  MString WriteStandardThresholds();
  //! Create a read-out element
  MReadOutElementDoubleStrip CreateROE(unsigned int DetectorID, unsigned int StripID, bool IsLowVoltage);
  //! Create a strip hit
  MStripHit* CreateStripHit(unsigned int DetectorID, unsigned int StripID, bool IsLowVoltage, double ADC, bool IsNearestNeighbor = false);
  //! Return the strip hit with the given strip ID and side or nullptr
  MStripHit* FindStripHit(MReadOutAssembly& Event, unsigned int StripID, bool IsLowVoltage);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestReadEnergyCalibrationFile() && Passed;
  Passed = TestReadSlowThresholdCutFile() && Passed;
  Passed = TestStandaloneLookups() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestAnalyzeEventCalibration() && Passed;
  Passed = TestAnalyzeEventThresholds() && Passed;
  Passed = TestFinalize() && Passed;

  // ShowOptionsGUI() and CreateExpos() create GUI elements and are not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleEnergyCalibration::WriteStandardCalibration()
{
  MString C;
  C += "# Test energy calibration\n";
  C += "TYPE ECAL\n";
  // Strip 1 LV: E = 0.5 * ADC, resolution FWHM = 1 + 0.002 E
  C += "CP dss 0 1 l pakw 2 200 100 1 400 200 1\n";
  C += "CM dss 0 1 l poly1 0 0.5\n";
  C += "CR dss 0 1 l p1 1.0 0.002\n";
  // Strip 2 HV: E = 1 + 0.5 ADC + 0.0001 ADC^2, resolution FWHM = 2 + 0 E + 0.00001 E^2
  C += "CP dss 0 2 h pakw 2 200 100 1 400 200 1\n";
  C += "CM dss 0 2 h poly2 1 0.5 0.0001\n";
  C += "CR dss 0 2 h poly2 2 0 0.00001\n";
  // Strip 3 LV: E = 0.25 ADC
  C += "CP dss 0 3 l pakw 2 200 100 1 400 200 1\n";
  C += "CM dss 0 3 l poly1zero 0.25\n";
  // Strip 4 'p' is LV: E = ADC (poly3)
  C += "CP dss 0 4 p pakw 2 200 100 1 400 200 1\n";
  C += "CM dss 0 4 p poly3 0 1 0 0\n";
  // Strip 5 'n' is HV: E = 2 ADC (poly4)
  C += "CP dss 0 5 n pakw 2 200 100 1 400 200 1\n";
  C += "CM dss 0 5 n poly4 0 2 0 0 0\n";
  // Strip 6: unknown calibration point descriptor - skipped
  C += "CP dss 0 6 l unknown 2 200 100 1 400 200 1\n";
  C += "CM dss 0 6 l poly1 0 1\n";
  // Strip 7: no calibration points - skipped
  C += "CM dss 0 7 l poly1 0 1\n";
  // Strip 8: unknown calibrator type - skipped
  C += "CP dss 0 8 l pakw 2 200 100 1 400 200 1\n";
  C += "CM dss 0 8 l spline 0 1\n";
  // Strip 9 LV: E = -100 + 0.5 ADC, can become negative
  C += "CP dss 0 9 l pakw 2 200 100 1 400 200 1\n";
  C += "CM dss 0 9 l poly1 -100 0.5\n";
  // Strip 10 HV in detector 1: E = 0.5 ADC
  C += "CP dss 1 10 h pakw 2 200 100 1 400 200 1\n";
  C += "CM dss 1 10 h poly1 0 0.5\n";

  MString FileName = GetTemporaryFileName("standard.ecal");
  WriteTextFile(FileName, C);
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleEnergyCalibration::WriteStandardThresholds()
{
  MString C;
  C += "STRIP_ID,Detector,detector_side,Strip,threshold_ADC,threshold_keV\n";
  C += "1,0,l,1,1600,30.0\n";
  C += "2,0,h,2,1500,0.0\n";
  C += "10,1,h,10,1700,12.5\n";

  MString FileName = GetTemporaryFileName("thresholds.csv");
  WriteTextFile(FileName, C);
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MReadOutElementDoubleStrip UTNModuleEnergyCalibration::CreateROE(unsigned int DetectorID, unsigned int StripID, bool IsLowVoltage)
{
  MReadOutElementDoubleStrip R;
  R.SetDetectorID(DetectorID);
  R.SetStripID(StripID);
  R.IsLowVoltageStrip(IsLowVoltage);
  return R;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleEnergyCalibration::CreateStripHit(unsigned int DetectorID, unsigned int StripID, bool IsLowVoltage, double ADC, bool IsNearestNeighbor)
{
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(DetectorID);
  SH->SetStripID(StripID);
  SH->IsLowVoltageStrip(IsLowVoltage);
  SH->SetADCUnits(ADC);
  SH->IsNearestNeighbor(IsNearestNeighbor);
  return SH;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleEnergyCalibration::FindStripHit(MReadOutAssembly& Event, unsigned int StripID, bool IsLowVoltage)
{
  for (unsigned int i = 0; i < Event.GetNStripHits(); ++i) {
    MStripHit* SH = Event.GetStripHit(i);
    if (SH->GetStripID() == StripID && SH->IsLowVoltageStrip() == IsLowVoltage) return SH;
  }
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleEnergyCalibration M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Energy calibrator'", M.GetName(), MString("Energy calibrator")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagEnergyCalibration'", M.GetXmlTag(), MString("XmlTagEnergyCalibration")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_EnergyCalibration", M.ProvidesModuleType(MAssembly::c_EnergyCalibration)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "An event loader is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "default", "The module allows multi-threading", M.AllowsMultiThreading()) && Passed;
  Passed = EvaluateTrue("AllowsMultipleInstances()", "default", "The module allows multiple instances", M.AllowsMultipleInstances()) && Passed;
  Passed = Evaluate("GetFileName()", "default", "The default file name is empty", M.GetFileName(), MString("")) && Passed;
  Passed = EvaluateTrue("GetSlowThresholdCutMode()", "default", "The default slow threshold mode is e_Ignore", M.GetSlowThresholdCutMode() == MSlowThresholdCutModes::e_Ignore) && Passed;
  Passed = EvaluateNear("GetSlowThresholdCutFixedValue()", "default", "The default fixed slow threshold is 15 keV", M.GetSlowThresholdCutFixedValue(), 15.0, 1e-12) && Passed;
  Passed = Evaluate("GetSlowThresholdCutFileName()", "default", "The default threshold file name is empty", M.GetSlowThresholdCutFileName(), MString("")) && Passed;
  Passed = EvaluateTrue("GetNearestNeighborCutMode()", "default", "The default nearest-neighbor mode is e_Ignore", M.GetNearestNeighborCutMode() == MNearestNeighborCutModes::e_Ignore) && Passed;
  Passed = EvaluateNear("GetNearestNeighborThreshold()", "default", "The default nearest-neighbor threshold is -1000 keV", M.GetNearestNeighborThreshold(), -1000.0, 1e-12) && Passed;
  Passed = Evaluate("GetCalibration()", "default", "The default calibration is empty", (unsigned int) M.GetCalibration().size(), 0u) && Passed;
  Passed = Evaluate("GetResolutionCalibration()", "default", "The default resolution calibration is empty", (unsigned int) M.GetResolutionCalibration().size(), 0u) && Passed;
  Passed = Evaluate("GetHardwareThresholdMap()", "default", "The default hardware threshold map is empty", (unsigned int) M.GetHardwareThresholdMap().size(), 0u) && Passed;

  // The integer values of the enums are used in the UI and the configuration files
  Passed = Evaluate("MSlowThresholdCutModes", "e_Ignore", "e_Ignore is 0", static_cast<int>(MSlowThresholdCutModes::e_Ignore), 0) && Passed;
  Passed = Evaluate("MSlowThresholdCutModes", "e_Fixed", "e_Fixed is 1", static_cast<int>(MSlowThresholdCutModes::e_Fixed), 1) && Passed;
  Passed = Evaluate("MSlowThresholdCutModes", "e_File", "e_File is 2", static_cast<int>(MSlowThresholdCutModes::e_File), 2) && Passed;
  Passed = Evaluate("MNearestNeighborCutModes", "e_Ignore", "e_Ignore is 100", static_cast<int>(MNearestNeighborCutModes::e_Ignore), 100) && Passed;
  Passed = Evaluate("MNearestNeighborCutModes", "e_Fixed", "e_Fixed is 101", static_cast<int>(MNearestNeighborCutModes::e_Fixed), 101) && Passed;

  MModuleEnergyCalibration* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestGettersSetters()
{
  bool Passed = true;

  MModuleEnergyCalibration M;
  M.SetFileName("my.ecal");
  Passed = Evaluate("SetFileName/GetFileName", "my.ecal", "GetFileName returns the set name", M.GetFileName(), MString("my.ecal")) && Passed;
  M.SetSlowThresholdCutMode(MSlowThresholdCutModes::e_File);
  Passed = EvaluateTrue("SetSlowThresholdCutMode/GetSlowThresholdCutMode", "e_File", "GetSlowThresholdCutMode returns e_File", M.GetSlowThresholdCutMode() == MSlowThresholdCutModes::e_File) && Passed;
  M.SetSlowThresholdCutFixedValue(22.5);
  Passed = EvaluateNear("SetSlowThresholdCutFixedValue/GetSlowThresholdCutFixedValue", "22.5", "GetSlowThresholdCutFixedValue returns 22.5", M.GetSlowThresholdCutFixedValue(), 22.5, 1e-12) && Passed;
  M.SetSlowThresholdCutFileName("thr.csv");
  Passed = Evaluate("SetSlowThresholdCutFileName/GetSlowThresholdCutFileName", "thr.csv", "GetSlowThresholdCutFileName returns the set name", M.GetSlowThresholdCutFileName(), MString("thr.csv")) && Passed;
  M.SetNearestNeighborCutMode(MNearestNeighborCutModes::e_Fixed);
  Passed = EvaluateTrue("SetNearestNeighborCutMode/GetNearestNeighborCutMode", "e_Fixed", "GetNearestNeighborCutMode returns e_Fixed", M.GetNearestNeighborCutMode() == MNearestNeighborCutModes::e_Fixed) && Passed;
  M.SetNearestNeighborThreshold(6.0);
  Passed = EvaluateNear("SetNearestNeighborThreshold/GetNearestNeighborThreshold", "6", "GetNearestNeighborThreshold returns 6", M.GetNearestNeighborThreshold(), 6.0, 1e-12) && Passed;

  // SetCalibration/SetResolutionCalibration: the caller keeps ownership here, thus the module is not finalized
  TF1* F = new TF1("UTNLinear", "[0] + [1]*x", 0., 16383.);
  F->FixParameter(0, 0.0);
  F->FixParameter(1, 2.0);
  TF1* FR = new TF1("UTNRes", "[0]", 0., 2000.);
  FR->FixParameter(0, 1.5);
  map<MReadOutElementDoubleStrip, TF1*> Cal;
  Cal[CreateROE(3, 4, true)] = F;
  map<MReadOutElementDoubleStrip, TF1*> Res;
  Res[CreateROE(3, 4, true)] = FR;
  M.SetCalibration(Cal);
  M.SetResolutionCalibration(Res);
  Passed = Evaluate("SetCalibration/GetCalibration", "one function", "GetCalibration has one entry", (unsigned int) M.GetCalibration().size(), 1u) && Passed;
  Passed = Evaluate("SetResolutionCalibration/GetResolutionCalibration", "one function", "GetResolutionCalibration has one entry", (unsigned int) M.GetResolutionCalibration().size(), 1u) && Passed;
  Passed = EvaluateNear("SetCalibration/GetEnergy", "ADC 100", "GetEnergy uses the set calibration function", M.GetEnergy(CreateROE(3, 4, true), 100.0), 200.0, 1e-9) && Passed;
  delete F;
  delete FR;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleEnergyCalibration Writer;
  Writer.SetFileName("/data/cal.ecal");
  Writer.SetSlowThresholdCutMode(MSlowThresholdCutModes::e_Fixed);
  Writer.SetSlowThresholdCutFixedValue(18.5);
  Writer.SetSlowThresholdCutFileName("/data/thr.csv");
  Writer.SetNearestNeighborCutMode(MNearestNeighborCutModes::e_Fixed);
  Writer.SetNearestNeighborThreshold(7.25);
  MXmlNode* Node = Writer.CreateXmlConfiguration();

  MModuleEnergyCalibration Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "configured", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "FileName", "The file name survives", Reader.GetFileName(), MString("/data/cal.ecal")) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "SlowThresholdCutMode", "The slow threshold mode survives", Reader.GetSlowThresholdCutMode() == MSlowThresholdCutModes::e_Fixed) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "SlowThresholdCutFixedValue", "The fixed slow threshold survives", Reader.GetSlowThresholdCutFixedValue(), 18.5, 1e-9) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "SlowThresholdCutFileName", "The threshold file name survives", Reader.GetSlowThresholdCutFileName(), MString("/data/thr.csv")) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "NearestNeighborCutMode", "The nearest-neighbor mode survives", Reader.GetNearestNeighborCutMode() == MNearestNeighborCutModes::e_Fixed) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "NearestNeighborThreshold", "The nearest-neighbor threshold survives", Reader.GetNearestNeighborThreshold(), 7.25, 1e-9) && Passed;
  delete Node;

  // The committed configuration files use the integer values directly
  MXmlNode* Ints = new MXmlNode(nullptr, "XmlTagEnergyCalibration");
  new MXmlNode(Ints, "SlowThresholdCutMode", 2);
  new MXmlNode(Ints, "NearestNeighborCutMode", 100);
  MModuleEnergyCalibration IntReader;
  IntReader.SetNearestNeighborCutMode(MNearestNeighborCutModes::e_Fixed);
  IntReader.ReadXmlConfiguration(Ints);
  Passed = EvaluateTrue("ReadXmlConfiguration()", "SlowThresholdCutMode 2", "The integer 2 is e_File", IntReader.GetSlowThresholdCutMode() == MSlowThresholdCutModes::e_File) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "NearestNeighborCutMode 100", "The integer 100 is e_Ignore", IntReader.GetNearestNeighborCutMode() == MNearestNeighborCutModes::e_Ignore) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "partial node", "Missing values keep their default", IntReader.GetSlowThresholdCutFixedValue(), 15.0, 1e-12) && Passed;
  delete Ints;

  // Unknown enum values must not be accepted silently
  MXmlNode* Bad = new MXmlNode(nullptr, "XmlTagEnergyCalibration");
  new MXmlNode(Bad, "SlowThresholdCutMode", 7);
  MModuleEnergyCalibration BadReader;
  BadReader.ReadXmlConfiguration(Bad);
  int BadMode = static_cast<int>(BadReader.GetSlowThresholdCutMode());
  Passed = ExpectedToFail(EvaluateTrue("ReadXmlConfiguration()", "SlowThresholdCutMode 7", "An unknown slow threshold mode is not stored", BadMode == 0 || BadMode == 1 || BadMode == 2)) && Passed;
  delete Bad;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestReadEnergyCalibrationFile()
{
  bool Passed = true;

  MModuleEnergyCalibration M;
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Result = M.ReadEnergyCalibrationFile(WriteStandardCalibration());
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("ReadEnergyCalibrationFile()", "standard file", "Reading the standard calibration succeeds", Result) && Passed;

  map<MReadOutElementDoubleStrip, TF1*> Cal = M.GetCalibration();
  Passed = Evaluate("ReadEnergyCalibrationFile()", "standard file", "Seven strips have a valid calibration", (unsigned int) Cal.size(), 7u) && Passed;
  Passed = EvaluateTrue("ReadEnergyCalibrationFile()", "strip 1 l", "Strip 1 LV is calibrated", Cal.find(CreateROE(0, 1, true)) != Cal.end()) && Passed;
  Passed = EvaluateTrue("ReadEnergyCalibrationFile()", "strip 2 h", "Strip 2 HV is calibrated", Cal.find(CreateROE(0, 2, false)) != Cal.end()) && Passed;
  Passed = EvaluateTrue("ReadEnergyCalibrationFile()", "strip 4 p", "Side 'p' is the LV side", Cal.find(CreateROE(0, 4, true)) != Cal.end()) && Passed;
  Passed = EvaluateTrue("ReadEnergyCalibrationFile()", "strip 5 n", "Side 'n' is the HV side", Cal.find(CreateROE(0, 5, false)) != Cal.end()) && Passed;
  Passed = EvaluateTrue("ReadEnergyCalibrationFile()", "strip 6 unknown descriptor", "A strip with an unknown calibration point descriptor is skipped", Cal.find(CreateROE(0, 6, true)) == Cal.end()) && Passed;
  Passed = EvaluateTrue("ReadEnergyCalibrationFile()", "strip 7 without CP", "A strip without calibration points is skipped", Cal.find(CreateROE(0, 7, true)) == Cal.end()) && Passed;
  Passed = EvaluateTrue("ReadEnergyCalibrationFile()", "strip 8 spline", "A strip with an unknown calibrator type is skipped", Cal.find(CreateROE(0, 8, true)) == Cal.end()) && Passed;
  Passed = Evaluate("ReadEnergyCalibrationFile()", "standard file", "Two strips have a resolution calibration", (unsigned int) M.GetResolutionCalibration().size(), 2u) && Passed;

  // Error paths
  g_Verbosity = c_Quiet;
  MString BadROE = GetTemporaryFileName("badroe.ecal");
  WriteTextFile(BadROE, "CM xyz 0 1 l poly1 0 0.5\n");
  MModuleEnergyCalibration MBad;
  bool BadResult = MBad.ReadEnergyCalibrationFile(BadROE);
  MModuleEnergyCalibration MMissing;
  bool MissingResult = MMissing.ReadEnergyCalibrationFile(GetTemporaryFileName("missing.ecal"));
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("ReadEnergyCalibrationFile()", "unknown read-out element", "An unknown read-out element type is rejected", BadResult) && Passed;
  Passed = EvaluateFalse("ReadEnergyCalibrationFile()", "missing file", "A missing file is rejected", MissingResult) && Passed;

  M.Finalize();
  MBad.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestReadSlowThresholdCutFile()
{
  bool Passed = true;

  MModuleEnergyCalibration M;
  Passed = EvaluateTrue("ReadSlowThresholdCutFile()", "standard file", "Reading the threshold file succeeds", M.ReadSlowThresholdCutFile(WriteStandardThresholds())) && Passed;
  map<MReadOutElementDoubleStrip, double> H = M.GetHardwareThresholdMap();
  // The first line is a column header, not data
  Passed = ExpectedToFail(Evaluate("ReadSlowThresholdCutFile()", "standard file", "Three hardware thresholds are read (the header line is not data)", (unsigned int) H.size(), 3u)) && Passed;
  if (H.find(CreateROE(0, 1, true)) != H.end()) {
    Passed = EvaluateNear("ReadSlowThresholdCutFile()", "strip 1 l", "The hardware threshold is 1600 ADC", H[CreateROE(0, 1, true)], 1600.0, 1e-12) && Passed;
  } else {
    Passed = EvaluateTrue("ReadSlowThresholdCutFile()", "strip 1 l", "Strip 1 LV has a hardware threshold", false) && Passed;
  }
  if (H.find(CreateROE(1, 10, false)) != H.end()) {
    Passed = EvaluateNear("ReadSlowThresholdCutFile()", "detector 1 strip 10 h", "The hardware threshold is 1700 ADC", H[CreateROE(1, 10, false)], 1700.0, 1e-12) && Passed;
  } else {
    Passed = EvaluateTrue("ReadSlowThresholdCutFile()", "detector 1 strip 10 h", "Detector 1 strip 10 HV has a hardware threshold", false) && Passed;
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MModuleEnergyCalibration MMissing;
  bool MissingResult = MMissing.ReadSlowThresholdCutFile(GetTemporaryFileName("missing.csv"));
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("ReadSlowThresholdCutFile()", "missing file", "A missing file is rejected", MissingResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestStandaloneLookups()
{
  bool Passed = true;

  MModuleEnergyCalibration M;
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  M.ReadEnergyCalibrationFile(WriteStandardCalibration());

  Passed = EvaluateNear("GetEnergy()", "poly1, ADC 1000", "E = 0.5 * 1000 = 500 keV", M.GetEnergy(CreateROE(0, 1, true), 1000.0), 500.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergy()", "poly2, ADC 1000", "E = 1 + 500 + 100 = 601 keV", M.GetEnergy(CreateROE(0, 2, false), 1000.0), 601.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergy()", "poly1zero, ADC 1000", "E = 0.25 * 1000 = 250 keV", M.GetEnergy(CreateROE(0, 3, true), 1000.0), 250.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergy()", "poly3, ADC 1000", "E = 1000 keV", M.GetEnergy(CreateROE(0, 4, true), 1000.0), 1000.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergy()", "poly4, ADC 300", "E = 2 * 300 = 600 keV", M.GetEnergy(CreateROE(0, 5, false), 300.0), 600.0, 1e-9) && Passed;
  Passed = EvaluateNear("GetEnergy()", "negative energy", "GetEnergy clamps negative energies to 0", M.GetEnergy(CreateROE(0, 9, true), 100.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergy()", "wrong side", "GetEnergy returns 0 for a strip without calibration on this side", M.GetEnergy(CreateROE(0, 1, false), 1000.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergy()", "unknown detector", "GetEnergy returns 0 for an unknown detector", M.GetEnergy(CreateROE(9, 1, true), 1000.0), 0.0, 1e-12) && Passed;

  Passed = EvaluateNear("GetADC()", "poly1, 500 keV", "ADC = 500 / 0.5 = 1000", M.GetADC(CreateROE(0, 1, true), 500.0), 1000.0, 1e-3) && Passed;
  Passed = EvaluateNear("GetADC()", "poly2, 601 keV", "GetADC inverts the poly2 calibration", M.GetADC(CreateROE(0, 2, false), 601.0), 1000.0, 1e-3) && Passed;
  Passed = EvaluateNear("GetADC(GetEnergy())", "poly1zero, ADC 3210", "GetADC inverts GetEnergy", M.GetADC(CreateROE(0, 3, true), M.GetEnergy(CreateROE(0, 3, true), 3210.0)), 3210.0, 1e-3) && Passed;
  Passed = EvaluateNear("GetADC()", "unknown strip", "GetADC returns 0 for an unknown strip", M.GetADC(CreateROE(9, 9, true), 500.0), 0.0, 1e-12) && Passed;

  MStripHit SH1;
  SH1.SetDetectorID(0);
  SH1.SetStripID(1);
  SH1.IsLowVoltageStrip(true);
  Passed = EvaluateNear("LookupEnergyResolution()", "p1, 500 keV", "Sigma = (1 + 0.002 * 500) / 2.355", M.LookupEnergyResolution(&SH1, 500.0), 2.0/2.355, 1e-9) && Passed;
  MStripHit SH2;
  SH2.SetDetectorID(0);
  SH2.SetStripID(2);
  SH2.IsLowVoltageStrip(false);
  Passed = EvaluateNear("LookupEnergyResolution()", "p2, 1000 keV", "Sigma = (2 + 0.00001 * 1000^2) / 2.355", M.LookupEnergyResolution(&SH2, 1000.0), 12.0/2.355, 1e-9) && Passed;
  MStripHit SH3;
  SH3.SetDetectorID(0);
  SH3.SetStripID(3);
  SH3.IsLowVoltageStrip(true);
  Passed = EvaluateNear("LookupEnergyResolution()", "no resolution calibration", "LookupEnergyResolution returns -1 without resolution calibration", M.LookupEnergyResolution(&SH3, 500.0), -1.0, 1e-12) && Passed;

  // Lookups must not modify the calibration
  Passed = ExpectedToFail(Evaluate("GetEnergy()", "lookups of unknown strips", "Lookups of unknown strips do not add entries to the calibration", (unsigned int) M.GetCalibration().size(), 7u)) && Passed;
  Passed = ExpectedToFail(Evaluate("LookupEnergyResolution()", "lookup of unknown strip", "Lookups of unknown strips do not add entries to the resolution calibration", (unsigned int) M.GetResolutionCalibration().size(), 2u)) && Passed;

  g_Verbosity = OldVerbosity;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestInitialize()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MModuleEnergyCalibration Missing;
  Missing.SetFileName(GetTemporaryFileName("missing.ecal"));
  bool MissingResult = Missing.Initialize();

  MModuleEnergyCalibration MissingThresholds;
  MissingThresholds.SetFileName(WriteStandardCalibration());
  MissingThresholds.SetSlowThresholdCutMode(MSlowThresholdCutModes::e_File);
  MissingThresholds.SetSlowThresholdCutFileName(GetTemporaryFileName("missing.csv"));
  bool MissingThresholdsResult = MissingThresholds.Initialize();

  // The threshold file is only needed in file mode
  MModuleEnergyCalibration FixedMode;
  FixedMode.SetFileName(WriteStandardCalibration());
  FixedMode.SetSlowThresholdCutMode(MSlowThresholdCutModes::e_Fixed);
  FixedMode.SetSlowThresholdCutFileName(GetTemporaryFileName("missing.csv"));
  bool FixedModeResult = FixedMode.Initialize();

  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "missing calibration file", "Initialize returns false for a missing calibration file", MissingResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "file mode, missing threshold file", "Initialize returns false for a missing threshold file in file mode", MissingThresholdsResult) && Passed;
  Passed = EvaluateTrue("Initialize()", "fixed mode, missing threshold file", "Initialize ignores the threshold file in fixed mode", FixedModeResult) && Passed;

  Missing.Finalize();
  MissingThresholds.Finalize();
  FixedMode.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestAnalyzeEventCalibration()
{
  bool Passed = true;

  MModuleEnergyCalibration M;
  M.SetFileName(WriteStandardCalibration());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Init = M.Initialize();
  g_Verbosity = OldVerbosity;
  if (Init == false) {
    return EvaluateTrue("Initialize()", "standard file", "Initialize succeeds for the calibration test", false);
  }

  MReadOutAssembly E;
  E.AddStripHit(CreateStripHit(0, 1, true, 1000));
  E.AddStripHit(CreateStripHit(0, 2, false, 1000));
  E.AddStripHit(CreateStripHit(0, 3, true, 1000));

  Passed = EvaluateTrue("AnalyzeEvent()", "calibrated strips", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "calibrated strips", "The c_EnergyCalibration analysis progress is set", E.HasAnalysisProgress(MAssembly::c_EnergyCalibration)) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "calibrated strips", "No energy calibration error is set", E.HasEnergyCalibrationError()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "calibrated strips", "All strip hits are kept", E.GetNStripHits(), 3u) && Passed;
  MStripHit* S1 = FindStripHit(E, 1, true);
  MStripHit* S2 = FindStripHit(E, 2, false);
  MStripHit* S3 = FindStripHit(E, 3, true);
  Passed = EvaluateTrue("AnalyzeEvent()", "calibrated strips", "Strip hit LV 1 is found", S1 != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "calibrated strips", "Strip hit HV 2 is found", S2 != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "calibrated strips", "Strip hit LV 3 is found", S3 != nullptr) && Passed;
  if (S1 != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "strip 1, ADC 1000", "The energy is 500 keV", S1->GetEnergy(), 500.0, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "strip 1, ADC 1000", "The energy resolution is (1 + 0.002 * 500) / 2.355", S1->GetEnergyResolution(), 2.0/2.355, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "strip 1, ADC 1000", "The ADC value is not modified", S1->GetADCUnits(), 1000.0, 1e-12) && Passed;
  }
  if (S2 != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "strip 2, ADC 1000", "The energy is 601 keV", S2->GetEnergy(), 601.0, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "strip 2, ADC 1000", "The energy resolution is (2 + 0.00001 * 601^2) / 2.355", S2->GetEnergyResolution(), (2 + 0.00001*601*601)/2.355, 1e-9) && Passed;
  }
  if (S3 != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "strip 3, ADC 1000", "The energy is 250 keV", S3->GetEnergy(), 250.0, 1e-9) && Passed;
  }

  // Uncalibrated strip: error, but the strip hit is kept
  g_Verbosity = c_Quiet;
  MReadOutAssembly U;
  U.AddStripHit(CreateStripHit(0, 1, true, 1000));
  U.AddStripHit(CreateStripHit(0, 33, true, 1000));
  bool UResult = M.AnalyzeEvent(&U);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("AnalyzeEvent()", "uncalibrated strip 33", "AnalyzeEvent returns true", UResult) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "uncalibrated strip 33", "An energy calibration error is set", U.HasEnergyCalibrationError()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "uncalibrated strip 33", "The uncalibrated strip hit is kept", U.GetNStripHits(), 2u) && Passed;
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "uncalibrated strip 33", "Analyzing an uncalibrated strip does not add an entry to the calibration", (unsigned int) M.GetCalibration().size(), 7u)) && Passed;
  MStripHit* U1 = FindStripHit(U, 1, true);
  Passed = EvaluateTrue("AnalyzeEvent()", "uncalibrated strip 33", "The calibrated strip hit LV 1 is found", U1 != nullptr) && Passed;
  if (U1 != nullptr) Passed = EvaluateNear("AnalyzeEvent()", "uncalibrated strip 33", "The calibrated strip in the same event is calibrated", U1->GetEnergy(), 500.0, 1e-9) && Passed;

  // High ADC values are flagged but kept
  g_Verbosity = c_Quiet;
  MReadOutAssembly H;
  H.AddStripHit(CreateStripHit(0, 1, true, 14500));
  M.AnalyzeEvent(&H);
  MReadOutAssembly N;
  N.AddStripHit(CreateStripHit(0, 1, true, 14000));
  M.AnalyzeEvent(&N);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("AnalyzeEvent()", "ADC 14500", "A high ADC value is flagged", H.HasHighADC_QualityFlag()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "ADC 14500", "A high ADC strip hit is kept", H.GetNStripHits(), 1u) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "ADC 14000", "ADC 14000 is not flagged as high", N.HasHighADC_QualityFlag()) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestAnalyzeEventThresholds()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  MString CalFile = WriteStandardCalibration();
  MString ThresholdFile = WriteStandardThresholds();

  // Ignore mode: triggered strips below 0 keV are removed
  {
    MModuleEnergyCalibration M;
    M.SetFileName(CalFile);
    g_Verbosity = c_Quiet;
    bool Init = M.Initialize();
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, 9, true, 100));   // -50 keV
    E.AddStripHit(CreateStripHit(0, 1, true, 10));    // 5 keV
    M.AnalyzeEvent(&E);
    g_Verbosity = OldVerbosity;
    Passed = EvaluateTrue("Initialize()", "ignore mode", "Initialize succeeds", Init) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "ignore mode, -50 keV", "A triggered strip with negative energy is removed", FindStripHit(E, 9, true) == nullptr) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "ignore mode, -50 keV", "The below-threshold quality flag is set", E.HasStripHitBelowThreshold_QualityFlag()) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "ignore mode, 5 keV", "A triggered strip with 5 keV is kept", FindStripHit(E, 1, true) != nullptr) && Passed;
    M.Finalize();
  }

  // Fixed mode
  {
    MModuleEnergyCalibration M;
    M.SetFileName(CalFile);
    M.SetSlowThresholdCutMode(MSlowThresholdCutModes::e_Fixed);
    M.SetSlowThresholdCutFixedValue(20.0);
    g_Verbosity = c_Quiet;
    bool Init = M.Initialize();
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, 1, true, 30));    // 15 keV
    E.AddStripHit(CreateStripHit(0, 3, true, 100));   // 25 keV
    E.AddStripHit(CreateStripHit(0, 2, false, 38));   // 1 + 19 + 0.1444 = 20.1444 keV
    M.AnalyzeEvent(&E);
    g_Verbosity = OldVerbosity;
    Passed = EvaluateTrue("Initialize()", "fixed 20 keV", "Initialize succeeds", Init) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "fixed 20 keV, 15 keV", "A strip below the fixed threshold is removed", FindStripHit(E, 1, true) == nullptr) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "fixed 20 keV, 25 keV", "A strip above the fixed threshold is kept", FindStripHit(E, 3, true) != nullptr) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "fixed 20 keV, 20.14 keV", "A strip just above the fixed threshold is kept", FindStripHit(E, 2, false) != nullptr) && Passed;
    M.Finalize();
  }

  // File mode
  {
    MModuleEnergyCalibration M;
    M.SetFileName(CalFile);
    M.SetSlowThresholdCutMode(MSlowThresholdCutModes::e_File);
    M.SetSlowThresholdCutFileName(ThresholdFile);
    g_Verbosity = c_Quiet;
    bool Init = M.Initialize();
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, 1, true, 50));    // 25 keV, threshold 30 keV
    E.AddStripHit(CreateStripHit(0, 3, true, 80));    // 20 keV, no threshold in file: default 15 keV
    E.AddStripHit(CreateStripHit(1, 10, false, 20));  // 10 keV, threshold 12.5 keV
    E.AddStripHit(CreateStripHit(0, 2, false, 10));   // 1 + 5 + 0.01 = 6.01 keV, threshold 0 keV means not found: default 15 keV
    M.AnalyzeEvent(&E);
    MReadOutAssembly K;
    K.AddStripHit(CreateStripHit(0, 1, true, 70));    // 35 keV, threshold 30 keV
    M.AnalyzeEvent(&K);
    g_Verbosity = OldVerbosity;
    Passed = EvaluateTrue("Initialize()", "file mode", "Initialize succeeds in file mode", Init) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "file, 25 keV < 30 keV", "A strip below its file threshold is removed", FindStripHit(E, 1, true) == nullptr) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "file, 35 keV > 30 keV", "A strip above its file threshold is kept", FindStripHit(K, 1, true) != nullptr) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "file, 20 keV, no entry", "A strip without file entry uses the default 15 keV threshold", FindStripHit(E, 3, true) != nullptr) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "file, 10 keV < 12.5 keV", "The threshold of detector 1 is applied", FindStripHit(E, 10, false) == nullptr) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "file, 6 keV, threshold 0 keV", "A file threshold of 0 keV means not found, thus the default 15 keV threshold removes the strip", FindStripHit(E, 2, false) == nullptr) && Passed;
    M.Finalize();
  }

  // Nearest neighbors
  {
    MModuleEnergyCalibration M;
    M.SetFileName(CalFile);
    g_Verbosity = c_Quiet;
    bool Init = M.Initialize();
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, 9, true, 100, true));  // -50 keV nearest neighbor
    E.AddStripHit(CreateStripHit(0, 9, true, 0, true));    // -100 keV nearest neighbor: not below -100
    M.AnalyzeEvent(&E);
    g_Verbosity = OldVerbosity;
    Passed = EvaluateTrue("Initialize()", "NN ignore mode", "Initialize succeeds", Init) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "NN ignore mode, -50 and -100 keV", "Nearest neighbors down to -100 keV are kept in ignore mode", E.GetNStripHits(), 2u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "NN ignore mode", "No below-threshold flag is set", E.HasStripHitBelowThreshold_QualityFlag()) && Passed;
    M.Finalize();
  }
  {
    MModuleEnergyCalibration M;
    M.SetFileName(CalFile);
    M.SetNearestNeighborCutMode(MNearestNeighborCutModes::e_Fixed);
    M.SetNearestNeighborThreshold(6.0);
    M.SetSlowThresholdCutMode(MSlowThresholdCutModes::e_Fixed);
    M.SetSlowThresholdCutFixedValue(100.0);
    g_Verbosity = c_Quiet;
    bool Init = M.Initialize();
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, 1, true, 10, true));   // 5 keV nearest neighbor: removed
    E.AddStripHit(CreateStripHit(0, 3, true, 40, true));   // 10 keV nearest neighbor: kept, the slow threshold does not apply
    M.AnalyzeEvent(&E);
    g_Verbosity = OldVerbosity;
    Passed = EvaluateTrue("Initialize()", "NN fixed 6 keV", "Initialize succeeds", Init) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "NN fixed 6 keV, 5 keV", "A nearest neighbor below its threshold is removed", FindStripHit(E, 1, true) == nullptr) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "NN fixed 6 keV, 10 keV", "A nearest neighbor above its threshold is kept and not cut by the slow threshold", FindStripHit(E, 3, true) != nullptr) && Passed;
    M.Finalize();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEnergyCalibration::TestFinalize()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MModuleEnergyCalibration M;
  M.SetFileName(WriteStandardCalibration());
  M.SetSlowThresholdCutMode(MSlowThresholdCutModes::e_File);
  M.SetSlowThresholdCutFileName(WriteStandardThresholds());
  M.Initialize();
  M.Finalize();

  // Finalize deletes the calibration functions, thus the maps must not keep the dangling pointers
  Passed = ExpectedToFail(Evaluate("Finalize()", "after Initialize", "Finalize clears the calibration map", (unsigned int) M.GetCalibration().size(), 0u)) && Passed;
  Passed = ExpectedToFail(Evaluate("Finalize()", "after Initialize", "Finalize clears the resolution calibration map", (unsigned int) M.GetResolutionCalibration().size(), 0u)) && Passed;
  Passed = Evaluate("Finalize()", "after Initialize", "Finalize clears the hardware threshold map", (unsigned int) M.GetHardwareThresholdMap().size(), 0u) && Passed;

  // Re-initialization with the same files
  bool Again = M.Initialize();
  Passed = EvaluateTrue("Initialize()", "after Finalize", "A second Initialize succeeds", Again) && Passed;
  Passed = EvaluateNear("GetEnergy()", "after re-initialization", "The calibration is usable after re-initialization", M.GetEnergy(CreateROE(0, 1, true), 1000.0), 500.0, 1e-9) && Passed;
  M.Finalize();

  g_Verbosity = OldVerbosity;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleEnergyCalibration Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleEnergyCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
