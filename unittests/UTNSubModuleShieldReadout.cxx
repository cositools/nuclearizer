/*
 * UTNSubModuleShieldReadout.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MSubModuleShieldReadout.h"
#include "MReadOutAssembly.h"
#include "MDEECrystalHit.h"


//! Unit test class for MSubModuleShieldReadout
class UTNSubModuleShieldReadout : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleShieldReadout() : MUnitTest("UTNSubModuleShieldReadout") {}
  //! Default destructor
  virtual ~UTNSubModuleShieldReadout() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the default values
  bool TestDefaultConstruction();
  //! Test the getter and setter of the calibration file name
  bool TestGettersSetters();
  //! Test the XML configuration
  bool TestXmlConfiguration();
  //! Test the error paths of Initialize()
  bool TestInitialize();
  //! Test the energy to ADC conversion
  bool TestADCConversion();
  //! Test the parsing of skipped, invalid and duplicate lines
  bool TestParsing();
  //! Test that a new Initialize() replaces the previous calibration
  bool TestReinitialize();
  //! Test that the committed NRL calibration file can be used
  bool TestNRLCalibrationFile();

  //! Return the two header lines of the NRL calibration CSV
  MString CalibrationHeader() const;
  //! Return one line of the NRL calibration CSV with 16 columns: ADC = A*Energy^2 + B*Energy + C
  MString CalibrationLine(const MString& Channel, const MString& A, const MString& B, const MString& C) const;
  //! Convert one energy deposit in the given crystal and return its ADC value
  unsigned int ConvertEnergy(MSubModuleShieldReadout& M, const MString& DetectorID, unsigned int CrystalID, double Energy) const;
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlConfiguration() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestADCConversion() && Passed;
  Passed = TestParsing() && Passed;
  Passed = TestReinitialize() && Passed;
  Passed = TestNRLCalibrationFile() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleShieldReadout::CalibrationHeader() const
{
  // The two header lines of the NRL calibration CSV, which the module skips by their first token
  MString Header = "Detector Meta Data,,,,,,,,ADC to Energy Fit Parameters (ax^2 + bx + c),,,,,,Resolution,\n";
  Header += "scb,gamma_channel,sipm_crystal,dac_id,cathod,asic,pannel,measured_cathod_voltage,a,a_err,b,b_err,c,c_err,Resolution @ 662 keV,Resolution Err @ 662 keV\n";
  return Header;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleShieldReadout::CalibrationLine(const MString& Channel, const MString& A, const MString& B, const MString& C) const
{
  // Columns: the channel (e.g. X0-3) is in column 2, the coefficients a, b, c in the columns 8, 10, 12,
  // all other columns are not used by the module
  return MString("C,8,") + Channel + ",7,1,1,X0,27.5," + A + ",0," + B + ",0," + C + ",0,15.0,0.1\n";
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTNSubModuleShieldReadout::ConvertEnergy(MSubModuleShieldReadout& M, const MString& DetectorID, unsigned int CrystalID, double Energy) const
{
  // The module identifies the crystal by m_DetectorID and m_CrystalID of the DEE crystal hit
  MReadOutAssembly E;
  MDEECrystalHit C;
  C.m_DetectorID = DetectorID;
  C.m_CrystalID = CrystalID;
  C.m_Energy = Energy;
  E.AddDEECrystalHit(C);
  M.AnalyzeEvent(&E);

  if (E.GetNDEECrystalHits() != 1) return g_UnsignedIntNotDefined;
  return E.GetDEECrystalHitListReference().front().m_ADC;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestDefaultConstruction()
{
  bool Passed = true;

  MSubModuleShieldReadout M;
  Passed = Evaluate("MSubModuleShieldReadout()", "default", "The default calibration file name is empty", M.GetShieldEnergyCalibrationFileName(), MString("")) && Passed;

  // Clear() resets the event data only, the configuration is kept
  M.SetShieldEnergyCalibrationFileName("calibration.csv");
  M.Clear();
  Passed = Evaluate("Clear()", "file name set", "Clear() keeps the calibration file name", M.GetShieldEnergyCalibrationFileName(), MString("calibration.csv")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestGettersSetters()
{
  bool Passed = true;

  MSubModuleShieldReadout M;
  M.SetShieldEnergyCalibrationFileName("ShieldEnergyCalibration_NRL.csv");
  Passed = Evaluate("SetShieldEnergyCalibrationFileName/GetShieldEnergyCalibrationFileName", "ShieldEnergyCalibration_NRL.csv", "Returns the set name", M.GetShieldEnergyCalibrationFileName(), MString("ShieldEnergyCalibration_NRL.csv")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestXmlConfiguration()
{
  bool Passed = true;

  MSubModuleShieldReadout Writer;
  Writer.SetShieldEnergyCalibrationFileName("/d/shield.csv");
  MXmlNode* Node = new MXmlNode(nullptr, "DEE");
  Passed = EvaluateTrue("CreateXmlConfiguration()", "mother node", "The mother node is returned", Writer.CreateXmlConfiguration(Node) == Node) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "mother node", "Exactly one configuration node is added", Node->GetNNodes(), 1u) && Passed;
  Passed = EvaluateTrue("CreateXmlConfiguration()", "mother node", "The node ShieldEnergyCalibrationFileName is added", Node->GetNode("ShieldEnergyCalibrationFileName") != nullptr) && Passed;
  if (Node->GetNode("ShieldEnergyCalibrationFileName") != nullptr) {
    Passed = Evaluate("CreateXmlConfiguration()", "mother node", "The node contains the file name", Node->GetNode("ShieldEnergyCalibrationFileName")->GetValue(), MString("/d/shield.csv")) && Passed;
  }

  MSubModuleShieldReadout Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "written node", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "written node", "The file name survives the round trip", Reader.GetShieldEnergyCalibrationFileName(), MString("/d/shield.csv")) && Passed;
  delete Node;

  // A node without the file name keeps the current value
  MXmlNode* Empty = new MXmlNode(nullptr, "DEE");
  Passed = EvaluateTrue("ReadXmlConfiguration()", "node without file name", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Empty)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "node without file name", "The file name is kept", Reader.GetShieldEnergyCalibrationFileName(), MString("/d/shield.csv")) && Passed;
  delete Empty;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestInitialize()
{
  bool Passed = true;

  MString HeaderOnlyFile = GetTemporaryFileName("shield.headeronly.csv");
  Passed = EvaluateTrue("Initialize()", "write fixture", "The header-only fixture file can be created", WriteTextFile(HeaderOnlyFile, CalibrationHeader())) && Passed;
  MString InvalidOnlyFile = GetTemporaryFileName("shield.invalidonly.csv");
  Passed = EvaluateTrue("Initialize()", "write fixture", "The fixture file with only invalid lines can be created", WriteTextFile(InvalidOnlyFile, CalibrationHeader() + "C,8,X0-3,7\n" + CalibrationLine("X03", "-0.0009765625", "8", "1600"))) && Passed;
  MString ValidFile = GetTemporaryFileName("shield.valid.csv");
  Passed = EvaluateTrue("Initialize()", "write fixture", "The valid fixture file can be created", WriteTextFile(ValidFile, CalibrationHeader() + CalibrationLine("X0-3", "-0.0009765625", "8", "1600"))) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MSubModuleShieldReadout NoFileName;
  bool NoFileNameResult = NoFileName.Initialize();

  MSubModuleShieldReadout MissingFile;
  MissingFile.SetShieldEnergyCalibrationFileName(GetTemporaryFileName("missing.shield.csv"));
  bool MissingFileResult = MissingFile.Initialize();

  MSubModuleShieldReadout HeaderOnly;
  HeaderOnly.SetShieldEnergyCalibrationFileName(HeaderOnlyFile);
  bool HeaderOnlyResult = HeaderOnly.Initialize();

  MSubModuleShieldReadout InvalidOnly;
  InvalidOnly.SetShieldEnergyCalibrationFileName(InvalidOnlyFile);
  bool InvalidOnlyResult = InvalidOnly.Initialize();

  MSubModuleShieldReadout Valid;
  Valid.SetShieldEnergyCalibrationFileName(ValidFile);
  bool ValidResult = Valid.Initialize();

  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "no file name", "Initialize returns false without a calibration file name", NoFileNameResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing file", "Initialize returns false when the calibration file does not exist", MissingFileResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "header lines only", "Initialize returns false when the file contains no calibration line", HeaderOnlyResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "invalid lines only", "Initialize returns false when no line can be parsed", InvalidOnlyResult) && Passed;
  Passed = EvaluateTrue("Initialize()", "one valid line", "Initialize returns true with one valid calibration line", ValidResult) && Passed;

  if (ValidResult == true) {
    MReadOutAssembly Empty;
    Passed = EvaluateTrue("AnalyzeEvent()", "empty event", "AnalyzeEvent returns true for an empty event", Valid.AnalyzeEvent(&Empty)) && Passed;
    Valid.Finalize();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestADCConversion()
{
  bool Passed = true;

  // ADC = A*Energy^2 + B*Energy + C, with the coefficients of each channel from the calibration file:
  // X0 crystal 3: A = -2^-10, B = 8, C = 1600
  // Y1 crystal 2: A = -2^-10, B = 14, C = 784
  // The coefficients are close to the NRL fits (small negative A) and exact binary values, thus the ADC values are exact.
  // Two more channels test the conversion of the ADC value to an integer:
  // Z0 crystal 1: A = 0, B = 0.5, C = 1000, which gives ADC values with a fraction
  // Z0 crystal 2: A = 0, B = -1, C = 100, which gives negative ADC values above 100 keV
  MString FileName = GetTemporaryFileName("shield.calibration.csv");
  MString Content = CalibrationHeader();
  Content += CalibrationLine("X0-3", "-0.0009765625", "8", "1600");
  Content += CalibrationLine("Y1-2", "-0.0009765625", "14", "784");
  Content += CalibrationLine("Z0-1", "0", "0.5", "1000");
  Content += CalibrationLine("Z0-2", "0", "-1", "100");
  Passed = EvaluateTrue("Initialize()", "write fixture", "The calibration fixture file can be created", WriteTextFile(FileName, Content)) && Passed;

  MSubModuleShieldReadout M;
  M.SetShieldEnergyCalibrationFileName(FileName);
  bool Init = M.Initialize();
  Passed = EvaluateTrue("Initialize()", "four channels", "Initialize returns true", Init) && Passed;
  if (Init == false) return Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  // X0 crystal 3: at 4096 keV the ADC value 17984 is above the end of the range (14 bit) and is limited to 16383
  vector<double> X0Energies = { 0.0, 256.0, 512.0, 1024.0, 4096.0 };
  vector<unsigned int> X0Expected = { 1600, 3584, 5440, 8768, 16383 };
  vector<unsigned int> X0ADC;
  for (double Energy : X0Energies) {
    X0ADC.push_back(ConvertEnergy(M, "X0", 3, Energy));
  }

  // Y1 crystal 2
  vector<double> Y1Energies = { 0.0, 256.0, 512.0, 1024.0 };
  vector<unsigned int> Y1Expected = { 784, 4304, 7696, 14096 };
  vector<unsigned int> Y1ADC;
  for (double Energy : Y1Energies) {
    Y1ADC.push_back(ConvertEnergy(M, "Y1", 2, Energy));
  }

  // Z0 crystal 1: the ADC value is rounded to the nearest channel, not truncated:
  // at 0.5 keV the ADC value is 1000.25 -> 1000, at 1.5 keV 1000.75 -> 1001 (truncation would give 1000 for both)
  unsigned int RoundDownADC = ConvertEnergy(M, "Z0", 1, 0.5);
  unsigned int RoundUpADC = ConvertEnergy(M, "Z0", 1, 1.5);

  // Z0 crystal 2: a negative ADC value is set to 0, at 100 keV the ADC value is exactly 0
  vector<double> Z0Energies = { 50.0, 100.0, 200.0 };
  vector<unsigned int> Z0Expected = { 50, 0, 0 };
  vector<unsigned int> Z0ADC;
  for (double Energy : Z0Energies) {
    Z0ADC.push_back(ConvertEnergy(M, "Z0", 2, Energy));
  }

  // Crystals without calibration
  unsigned int OtherCrystalADC = ConvertEnergy(M, "X0", 0, 256.0);
  unsigned int OtherDetectorADC = ConvertEnergy(M, "Z1", 3, 256.0);

  // Several hits in one event: each hit is converted with the coefficients of its own crystal, the energies are not modified
  MReadOutAssembly E;
  MDEECrystalHit First;
  First.m_DetectorID = "X0";
  First.m_CrystalID = 3;
  First.m_Energy = 256.0;
  E.AddDEECrystalHit(First);
  MDEECrystalHit Second;
  Second.m_DetectorID = "Y1";
  Second.m_CrystalID = 2;
  Second.m_Energy = 256.0;
  E.AddDEECrystalHit(Second);
  bool Analyzed = M.AnalyzeEvent(&E);

  g_Verbosity = OldVerbosity;

  for (unsigned int i = 0; i < X0Energies.size(); ++i) {
    Passed = Evaluate("AnalyzeEvent()", MString("X0 crystal 3, energy ") + X0Energies[i], "The ADC value is A*E^2 + B*E + C with the coefficients of X0 crystal 3, limited to 16383", X0ADC[i], X0Expected[i]) && Passed;
  }
  for (unsigned int i = 0; i < Y1Energies.size(); ++i) {
    Passed = Evaluate("AnalyzeEvent()", MString("Y1 crystal 2, energy ") + Y1Energies[i], "The ADC value is A*E^2 + B*E + C with the coefficients of Y1 crystal 2", Y1ADC[i], Y1Expected[i]) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "Z0 crystal 1, ADC value 1000.25", "The ADC value is rounded down to 1000", RoundDownADC, 1000u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "Z0 crystal 1, ADC value 1000.75", "The ADC value is rounded up to 1001, not truncated", RoundUpADC, 1001u) && Passed;
  for (unsigned int i = 0; i < Z0Energies.size(); ++i) {
    Passed = Evaluate("AnalyzeEvent()", MString("Z0 crystal 2, energy ") + Z0Energies[i], "The ADC value is A*E^2 + B*E + C with the coefficients of Z0 crystal 2, a negative value is set to 0", Z0ADC[i], Z0Expected[i]) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "X0 crystal 0 without calibration", "The ADC value is 0", OtherCrystalADC, 0u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "Z1 crystal 3 without calibration", "The ADC value is 0", OtherDetectorADC, 0u) && Passed;

  Passed = EvaluateTrue("AnalyzeEvent()", "two hits", "AnalyzeEvent returns true", Analyzed) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "two hits", "No crystal hit is added or removed", E.GetNDEECrystalHits(), 2u) && Passed;
  if (E.GetNDEECrystalHits() == 2) {
    const MDEECrystalHit& H1 = E.GetDEECrystalHitListReference().front();
    const MDEECrystalHit& H2 = E.GetDEECrystalHitListReference().back();
    Passed = Evaluate("AnalyzeEvent()", "two hits, X0 crystal 3 at 256 keV", "The ADC value uses the coefficients of X0 crystal 3: 3584", H1.m_ADC, 3584u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "two hits, Y1 crystal 2 at 256 keV", "The ADC value uses the coefficients of Y1 crystal 2: 4304", H2.m_ADC, 4304u) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "two hits, X0 crystal 3 at 256 keV", "The energy is not modified", H1.m_Energy, 256.0, 1e-12) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "two hits, Y1 crystal 2 at 256 keV", "The energy is not modified", H2.m_Energy, 256.0, 1e-12) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestParsing()
{
  bool Passed = true;

  // Valid lines between invalid ones are still parsed, the invalid lines are skipped:
  // a line with too few columns, a channel without detector-crystal separator, a channel with three parts.
  // A duplicate channel replaces the previous entry: X0 crystal 3 is first given with C = 1600, then with C = 1000
  MString FileName = GetTemporaryFileName("shield.mixed.csv");
  MString Content = CalibrationHeader();
  Content += CalibrationLine("X0-3", "-0.0009765625", "8", "1600");
  Content += "C,8,X0-1,7,1,1,X0\n";
  Content += CalibrationLine("X01", "-0.0009765625", "8", "1600");
  Content += CalibrationLine("X0-2-1", "-0.0009765625", "8", "1600");
  Content += CalibrationLine("Y1-2", "-0.0009765625", "14", "784");
  Content += CalibrationLine("X0-3", "-0.0009765625", "8", "1000");
  Passed = EvaluateTrue("Initialize()", "write fixture", "The fixture file with invalid and duplicate lines can be created", WriteTextFile(FileName, Content)) && Passed;

  MSubModuleShieldReadout M;
  M.SetShieldEnergyCalibrationFileName(FileName);

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  bool Init = M.Initialize();
  unsigned int DuplicateADC = ConvertEnergy(M, "X0", 3, 0.0);
  unsigned int ValidADC = ConvertEnergy(M, "Y1", 2, 256.0);
  unsigned int TooFewColumnsADC = ConvertEnergy(M, "X0", 1, 256.0);
  unsigned int ThreePartsADC = ConvertEnergy(M, "X0", 2, 256.0);

  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("Initialize()", "valid, invalid and duplicate lines", "Initialize returns true when at least one line is valid", Init) && Passed;
  Passed = Evaluate("Initialize()", "X0 crystal 3 given twice", "The later line replaces the earlier one: the ADC value at 0 keV is 1000", DuplicateADC, 1000u) && Passed;
  Passed = Evaluate("Initialize()", "Y1 crystal 2 after invalid lines", "The valid line after the invalid lines is parsed: the ADC value at 256 keV is 4304", ValidADC, 4304u) && Passed;
  Passed = Evaluate("Initialize()", "X0 crystal 1 with too few columns", "The line is skipped: the ADC value is 0", TooFewColumnsADC, 0u) && Passed;
  Passed = Evaluate("Initialize()", "channel X0-2-1 with three parts", "The line is skipped: X0 crystal 2 has no calibration, the ADC value is 0", ThreePartsADC, 0u) && Passed;

  if (Init == true) M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestReinitialize()
{
  bool Passed = true;

  MString FirstFile = GetTemporaryFileName("shield.first.csv");
  Passed = EvaluateTrue("Initialize()", "write fixture", "The first fixture file can be created", WriteTextFile(FirstFile, CalibrationHeader() + CalibrationLine("X0-3", "-0.0009765625", "8", "1600"))) && Passed;
  MString SecondFile = GetTemporaryFileName("shield.second.csv");
  Passed = EvaluateTrue("Initialize()", "write fixture", "The second fixture file can be created", WriteTextFile(SecondFile, CalibrationHeader() + CalibrationLine("Y1-2", "-0.0009765625", "14", "784"))) && Passed;

  MSubModuleShieldReadout M;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  M.SetShieldEnergyCalibrationFileName(FirstFile);
  bool FirstInit = M.Initialize();
  unsigned int FirstADC = ConvertEnergy(M, "X0", 3, 256.0);

  // The second calibration replaces the first one completely
  M.SetShieldEnergyCalibrationFileName(SecondFile);
  bool SecondInit = M.Initialize();
  unsigned int OldChannelADC = ConvertEnergy(M, "X0", 3, 256.0);
  unsigned int NewChannelADC = ConvertEnergy(M, "Y1", 2, 256.0);

  // A failed Initialize() leaves no calibration from before
  M.SetShieldEnergyCalibrationFileName(GetTemporaryFileName("missing.shield.csv"));
  bool FailedInit = M.Initialize();
  unsigned int AfterFailureADC = ConvertEnergy(M, "Y1", 2, 256.0);

  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("Initialize()", "first file", "Initialize returns true", FirstInit) && Passed;
  Passed = Evaluate("Initialize()", "first file, X0 crystal 3 at 256 keV", "The ADC value is 3584", FirstADC, 3584u) && Passed;
  Passed = EvaluateTrue("Initialize()", "second file", "Initialize returns true", SecondInit) && Passed;
  Passed = Evaluate("Initialize()", "second file, X0 crystal 3 at 256 keV", "The calibration of the first file is removed: the ADC value is 0", OldChannelADC, 0u) && Passed;
  Passed = Evaluate("Initialize()", "second file, Y1 crystal 2 at 256 keV", "The ADC value is 4304", NewChannelADC, 4304u) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing file after a valid one", "Initialize returns false", FailedInit) && Passed;
  Passed = Evaluate("Initialize()", "missing file, Y1 crystal 2 at 256 keV", "No calibration of the previous file is kept: the ADC value is 0", AfterFailureADC, 0u) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestNRLCalibrationFile()
{
  bool Passed = true;

  const char* NuclearizerEnv = getenv("NUCLEARIZER");
  if (NuclearizerEnv == nullptr || NuclearizerEnv[0] == '\0') {
    mout << "TestNRLCalibrationFile: NUCLEARIZER not set - skipping test" << endl;
    return Passed;
  }

  // The committed NRL calibration file: two header lines, 16 columns, and Windows line endings.
  // The checks don't use the fit values, thus they stay valid when the calibration is updated
  MSubModuleShieldReadout M;
  M.SetShieldEnergyCalibrationFileName(MString(NuclearizerEnv) + "/resource/dee/ShieldEnergyCalibration_NRL.csv");

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Init = M.Initialize();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("Initialize()", "ShieldEnergyCalibration_NRL.csv", "Initialize returns true with the committed NRL calibration file", Init) && Passed;
  if (Init == false) return Passed;

  // All 24 read-out channels of the ACS: X0, X1, Y0, Y1 with crystals 0-2, Z0, Z1 with crystals 0-4, D0 and D1 with crystal 0
  vector<MString> Detectors = { "X0", "X1", "Y0", "Y1", "Z0", "Z1", "D0", "D1" };
  vector<unsigned int> NCrystals = { 3, 3, 3, 3, 5, 5, 1, 1 };

  // At 662 keV, the ADC value of every channel is inside the range: a value of 0 would mean no calibration
  // was found for the channel, and 16383 would mean the value was clamped
  g_Verbosity = c_Quiet;
  vector<MString> Channels;
  vector<unsigned int> ADCValues;
  for (unsigned int d = 0; d < Detectors.size(); ++d) {
    for (unsigned int c = 0; c < NCrystals[d]; ++c) {
      Channels.push_back(Detectors[d] + "-" + c);
      ADCValues.push_back(ConvertEnergy(M, Detectors[d], c, 662.0));
    }
  }
  g_Verbosity = OldVerbosity;

  Passed = Evaluate("Initialize()", "ShieldEnergyCalibration_NRL.csv", "The ACS has 24 read-out channels", (unsigned int) Channels.size(), 24u) && Passed;
  for (unsigned int i = 0; i < Channels.size(); ++i) {
    Passed = EvaluateTrue("AnalyzeEvent()", MString("channel ") + Channels[i] + " at 662 keV", "The channel has a calibration: the ADC value is above 0", ADCValues[i] > 0) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", MString("channel ") + Channels[i] + " at 662 keV", "The ADC value is below the end of the range 16383", ADCValues[i] < 16383) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleShieldReadout Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleShieldReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
