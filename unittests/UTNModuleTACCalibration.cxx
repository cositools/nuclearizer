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
#include <fstream>
#include <unistd.h>
#include <unordered_map>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MFile.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MModuleTACCalibration.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MStripMap.h"
#include "MModuleLoaderMeasurementsHDF.h"
#include "MModuleLoaderMeasurementsL0.h"
#include "MModuleLoaderSimulationsCosima.h"
#include "MModuleDEESMEX.h"
#include "MSupervisor.h"


//! The TAC calibration parameter table: DetID -> Side (LV=0, HV=1) -> Strip ID -> parameters
typedef unordered_map<int, vector<unordered_map<int, vector<double>>>> TACCalTable;


//! Unit test class for MModuleTACCalibration
class UTNModuleTACCalibration : public MUnitTest
{
public:
  UTNModuleTACCalibration() : MUnitTest("UTNModuleTACCalibration") {}
  virtual ~UTNModuleTACCalibration() {}

  virtual bool Run();

private:
  //! Test the default values
  bool TestDefaultConstruction();
  //! Test getter and setter methods
  bool TestGettersSetters();
  //! Test XML configuration round-trip
  bool TestXmlRoundTrip();
  //! Test that detector, side, and strip are taken from the strip map via the read-out ID
  bool TestLoadWithStripMap();
  //! Test that entries whose read-out ID is not in the strip map are skipped
  bool TestLoadSkipsUnmappedReadOutIDs();
  //! Test that the detector, side, and strip columns are used without a strip map
  bool TestLoadWithoutStripMap();
  //! Test the error paths of LoadTACCalFile()
  bool TestLoadErrors();
  //! Test Initialize() with and without a strip map
  bool TestInitialize();
  //! Test that only a preceding HDF or L0 loader is found as strip map provider
  bool TestFindStripMapProvider();
  //! Test that AnalyzeEvent() calibrates a hit with the constants of its strip-map channel
  bool TestAnalyzeEvent();

  //! Return a process-unique fixture file path
  MString GetFixturePath(const MString& Suffix) const;
  //! Write the lines to a fixture file - return false if the file cannot be created
  bool WriteFixture(const MString& FileName, const vector<MString>& Lines) const;
  //! Write the strip map and TAC calibration fixtures shared by several sub-tests
  bool WriteStandardFixtures(const MString& StripMapFile, const MString& TACCalFile) const;
};


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleTACCalibration::GetFixturePath(const MString& Suffix) const
{
  return MString("/tmp/UTNModuleTACCalibration_") + (unsigned int) getpid() + Suffix;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::WriteFixture(const MString& FileName, const vector<MString>& Lines) const
{
  ofstream Out(FileName.Data());
  if (Out.is_open() == false) return false;
  for (const MString& Line : Lines) {
    Out << Line << endl;
  }
  Out.close();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::WriteStandardFixtures(const MString& StripMapFile, const MString& TACCalFile) const
{
  // The strip map moves read-out IDs 10 and 11 to detector 3 and swaps their sides,
  // while read-out ID 12 agrees with the TAC calibration file
  vector<MString> StripMapLines = {
    "# Columns are strip_id, detector, side (0=LV, 1=HV), strip",
    "10 3 0 32",
    "11 3 1 5",
    "12 7 0 40"
  };

  vector<MString> TACCalLines = {
    "#STRIP_ID,Detector,Side,Strip,TAC_calibration,TAC_calibration_error,TAC_offset,TAC_offset_error",
    "10,0,1,32,0.45,0.001,-600.0,7.5",
    "11,0,0,5,0.43,0.002,-550.0,6.0",
    "12,7,0,40,0.47,0.003,-500.0,5.0"
  };

  return WriteFixture(StripMapFile, StripMapLines) && WriteFixture(TACCalFile, TACCalLines);
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestLoadWithStripMap() && Passed;
  Passed = TestLoadSkipsUnmappedReadOutIDs() && Passed;
  Passed = TestLoadWithoutStripMap() && Passed;
  Passed = TestLoadErrors() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestFindStripMapProvider() && Passed;
  Passed = TestAnalyzeEvent() && Passed;

  Summarize();
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleTACCalibration Module;

  Passed = Evaluate("GetTACCalFileName()", "default", "The default TAC calibration file name is empty", Module.GetTACCalFileName(), MString("")) && Passed;
  Passed = Evaluate("GetFileNameStripMap()", "default", "The default strip map file name is empty", Module.GetFileNameStripMap(), MString("")) && Passed;
  Passed = EvaluateTrue("GetApplyTACCuts()", "default", "TAC cuts are applied by default", Module.GetApplyTACCuts()) && Passed;
  Passed = EvaluateNear("GetCoincidenceWindow()", "default", "The default coincidence window is 600 ns", Module.GetCoincidenceWindow(), 600.0, 1e-9) && Passed;
  Passed = Evaluate("GetTACCalParameters()", "default", "The default TAC calibration table is empty", (unsigned int) Module.GetTACCalParameters().size(), 0U) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestGettersSetters()
{
  bool Passed = true;

  MModuleTACCalibration Module;

  Module.SetTACCalFileName("/path/to/taccal.csv");
  Passed = Evaluate("SetTACCalFileName/GetTACCalFileName", "representative value", "GetTACCalFileName returns the representative file name", Module.GetTACCalFileName(), MString("/path/to/taccal.csv")) && Passed;

  Module.SetFileNameStripMap("/path/to/strip.map");
  Passed = Evaluate("SetFileNameStripMap/GetFileNameStripMap", "representative value", "GetFileNameStripMap returns the representative file name", Module.GetFileNameStripMap(), MString("/path/to/strip.map")) && Passed;

  Module.SetApplyTACCuts(false);
  Passed = EvaluateFalse("SetApplyTACCuts/GetApplyTACCuts", "false", "GetApplyTACCuts returns false after SetApplyTACCuts(false)", Module.GetApplyTACCuts()) && Passed;

  Module.SetCoincidenceWindow(450.0);
  Passed = EvaluateNear("SetCoincidenceWindow/GetCoincidenceWindow", "representative value", "GetCoincidenceWindow returns the representative 450 ns", Module.GetCoincidenceWindow(), 450.0, 1e-9) && Passed;

  TACCalTable Table;
  Table[5].resize(2);
  Table[5][1][17] = { 0.4, -500.0, 0.01, 2.0 };
  Module.SetTACCalParameters(Table);
  TACCalTable Read = Module.GetTACCalParameters();
  Passed = EvaluateTrue("SetTACCalParameters/GetTACCalParameters", "representative table", "GetTACCalParameters returns the entry for detector 5, HV, strip 17", Read.count(5) == 1 && Read[5].size() == 2 && Read[5][1].count(17) == 1) && Passed;
  if (Read.count(5) == 1 && Read[5].size() == 2 && Read[5][1].count(17) == 1) {
    Passed = EvaluateNear("SetTACCalParameters/GetTACCalParameters", "representative table", "GetTACCalParameters returns the representative offset", Read[5][1][17][1], -500.0, 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleTACCalibration Writer;
  Writer.SetTACCalFileName("/path/to/taccal.csv");
  Writer.SetFileNameStripMap("/path/to/strip.map");
  Writer.SetApplyTACCuts(false);
  Writer.SetCoincidenceWindow(450.0);

  MXmlNode* Node = Writer.CreateXmlConfiguration();
  Passed = EvaluateTrue("CreateXmlConfiguration()", "representative values", "CreateXmlConfiguration() returns a non-null node for representative settings", Node != nullptr) && Passed;
  if (Node == nullptr) return false;

  MModuleTACCalibration Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "representative values", "ReadXmlConfiguration() returns true for a valid configuration node", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "TACCalFileName", "ReadXmlConfiguration() restores the representative TAC calibration file name", Reader.GetTACCalFileName(), MString("/path/to/taccal.csv")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "FileNameStripMap", "ReadXmlConfiguration() restores the representative strip map file name", Reader.GetFileNameStripMap(), MString("/path/to/strip.map")) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "ApplyTACCuts", "ReadXmlConfiguration() restores the representative ApplyTACCuts flag", Reader.GetApplyTACCuts()) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "CoincidenceWindow", "ReadXmlConfiguration() restores the representative coincidence window", Reader.GetCoincidenceWindow(), 450.0, 1e-9) && Passed;

  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestLoadWithStripMap()
{
  bool Passed = true;

  const MString StripMapFile = GetFixturePath("_remap.map");
  const MString TACCalFile = GetFixturePath("_remap.csv");
  Passed = EvaluateTrue("LoadTACCalFile()", "write fixtures", "The strip map and TAC calibration fixtures can be created", WriteStandardFixtures(StripMapFile, TACCalFile)) && Passed;

  MStripMap StripMap;
  Passed = EvaluateTrue("LoadTACCalFile()", "open strip map", "The strip map fixture can be opened", StripMap.Open(StripMapFile)) && Passed;

  // The disagreement with the strip map prints a warning
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MModuleTACCalibration Module;
  bool Result = Module.LoadTACCalFile(TACCalFile, &StripMap);
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("LoadTACCalFile()", "strip map disagrees with file", "LoadTACCalFile() succeeds when the strip map disagrees with the detector, side, and strip columns", Result) && Passed;

  TACCalTable Table = Module.GetTACCalParameters();
  Passed = Evaluate("LoadTACCalFile()", "strip map disagrees with file", "The table contains exactly the strip-map detectors 3 and 7", (unsigned int) Table.size(), 2U) && Passed;
  Passed = EvaluateTrue("LoadTACCalFile()", "strip map disagrees with file", "No entry is stored under the file's detector column value 0", Table.count(0) == 0) && Passed;

  bool HasDet3 = Table.count(3) == 1 && Table[3].size() == 2;
  Passed = EvaluateTrue("LoadTACCalFile()", "read-out ID 10", "Read-out ID 10 is stored under strip-map detector 3, LV, strip 32", HasDet3 && Table[3][0].count(32) == 1) && Passed;
  Passed = EvaluateTrue("LoadTACCalFile()", "read-out ID 10", "Read-out ID 10 is not stored under the file's side (HV)", HasDet3 && Table[3][1].count(32) == 0) && Passed;
  Passed = EvaluateTrue("LoadTACCalFile()", "read-out ID 11", "Read-out ID 11 is stored under strip-map detector 3, HV, strip 5", HasDet3 && Table[3][1].count(5) == 1) && Passed;
  Passed = EvaluateTrue("LoadTACCalFile()", "read-out ID 11", "Read-out ID 11 is not stored under the file's side (LV)", HasDet3 && Table[3][0].count(5) == 0) && Passed;

  if (HasDet3 && Table[3][0].count(32) == 1 && Table[3][0][32].size() == 4) {
    // The parameters are stored as slope, offset, slope error, offset error
    Passed = EvaluateNear("LoadTACCalFile()", "read-out ID 10", "The TAC calibration slope of read-out ID 10 is 0.45", Table[3][0][32][0], 0.45, 1e-9) && Passed;
    Passed = EvaluateNear("LoadTACCalFile()", "read-out ID 10", "The TAC calibration offset of read-out ID 10 is -600", Table[3][0][32][1], -600.0, 1e-9) && Passed;
    Passed = EvaluateNear("LoadTACCalFile()", "read-out ID 10", "The TAC calibration slope error of read-out ID 10 is 0.001", Table[3][0][32][2], 0.001, 1e-9) && Passed;
    Passed = EvaluateNear("LoadTACCalFile()", "read-out ID 10", "The TAC calibration offset error of read-out ID 10 is 7.5", Table[3][0][32][3], 7.5, 1e-9) && Passed;
  }
  if (HasDet3 && Table[3][1].count(5) == 1 && Table[3][1][5].size() == 4) {
    Passed = EvaluateNear("LoadTACCalFile()", "read-out ID 11", "The TAC calibration slope of read-out ID 11 is 0.43", Table[3][1][5][0], 0.43, 1e-9) && Passed;
    Passed = EvaluateNear("LoadTACCalFile()", "read-out ID 11", "The TAC calibration offset of read-out ID 11 is -550", Table[3][1][5][1], -550.0, 1e-9) && Passed;
  }

  bool HasDet7 = Table.count(7) == 1 && Table[7].size() == 2;
  Passed = EvaluateTrue("LoadTACCalFile()", "read-out ID 12", "Read-out ID 12, which agrees with the file, is stored under detector 7, LV, strip 40", HasDet7 && Table[7][0].count(40) == 1) && Passed;
  if (HasDet7 && Table[7][0].count(40) == 1 && Table[7][0][40].size() == 4) {
    Passed = EvaluateNear("LoadTACCalFile()", "read-out ID 12", "The TAC calibration slope of read-out ID 12 is 0.47", Table[7][0][40][0], 0.47, 1e-9) && Passed;
  }

  MFile::Remove(StripMapFile);
  MFile::Remove(TACCalFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestLoadSkipsUnmappedReadOutIDs()
{
  bool Passed = true;

  const MString StripMapFile = GetFixturePath("_skip.map");
  const MString TACCalFile = GetFixturePath("_skip.csv");
  const MString UnmappedFile = GetFixturePath("_skip_all.csv");

  vector<MString> StripMapLines = { "12 7 0 40" };
  vector<MString> TACCalLines = {
    "99,0,0,10,0.40,0.001,-400.0,4.0",
    "12,7,0,40,0.47,0.003,-500.0,5.0",
    "-1,0,0,11,0.41,0.001,-410.0,4.0"
  };
  vector<MString> UnmappedLines = { "99,0,0,10,0.40,0.001,-400.0,4.0" };

  Passed = EvaluateTrue("LoadTACCalFile()", "write fixtures", "The fixtures can be created", WriteFixture(StripMapFile, StripMapLines) && WriteFixture(TACCalFile, TACCalLines) && WriteFixture(UnmappedFile, UnmappedLines)) && Passed;

  MStripMap StripMap;
  Passed = EvaluateTrue("LoadTACCalFile()", "open strip map", "The strip map fixture can be opened", StripMap.Open(StripMapFile)) && Passed;

  // Unknown and negative read-out IDs are skipped, the mapped entry is still loaded
  MModuleTACCalibration Module;
  Passed = EvaluateTrue("LoadTACCalFile()", "unknown and negative read-out IDs", "LoadTACCalFile() succeeds when some read-out IDs are not in the strip map", Module.LoadTACCalFile(TACCalFile, &StripMap)) && Passed;
  TACCalTable Table = Module.GetTACCalParameters();
  Passed = Evaluate("LoadTACCalFile()", "unknown and negative read-out IDs", "Only the detector of the mapped read-out ID is in the table", (unsigned int) Table.size(), 1U) && Passed;
  Passed = EvaluateTrue("LoadTACCalFile()", "unknown and negative read-out IDs", "The mapped read-out ID 12 is stored under detector 7, LV, strip 40", Table.count(7) == 1 && Table[7].size() == 2 && Table[7][0].count(40) == 1) && Passed;
  Passed = EvaluateTrue("LoadTACCalFile()", "unknown and negative read-out IDs", "The skipped entries are not stored under their file detector 0", Table.count(0) == 0) && Passed;

  // A file without any mapped read-out ID loads an empty table
  MModuleTACCalibration Empty;
  Passed = EvaluateTrue("LoadTACCalFile()", "no mapped read-out ID", "LoadTACCalFile() succeeds when no read-out ID is in the strip map", Empty.LoadTACCalFile(UnmappedFile, &StripMap)) && Passed;
  Passed = Evaluate("LoadTACCalFile()", "no mapped read-out ID", "The table is empty when no read-out ID is in the strip map", (unsigned int) Empty.GetTACCalParameters().size(), 0U) && Passed;

  MFile::Remove(StripMapFile);
  MFile::Remove(TACCalFile);
  MFile::Remove(UnmappedFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestLoadWithoutStripMap()
{
  bool Passed = true;

  const MString StripMapFile = GetFixturePath("_nomap.map");
  const MString TACCalFile = GetFixturePath("_nomap.csv");
  const MString SevenColumnFile = GetFixturePath("_nomap_seven.csv");

  vector<MString> SevenColumnLines = {
    "# DetID,Strip voltage, StripID,TAC_calibration,TAC_calibration_error,TAC_offset,TAC_offset_error",
    "2,h,31,0.42,0.001,-520.0,5.5"
  };
  Passed = EvaluateTrue("LoadTACCalFile()", "write fixtures", "The fixtures can be created", WriteStandardFixtures(StripMapFile, TACCalFile) && WriteFixture(SevenColumnFile, SevenColumnLines)) && Passed;

  // Without a strip map the detector, side, and strip columns of the file are used - this prints a notice
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MModuleTACCalibration Module;
  bool Result = Module.LoadTACCalFile(TACCalFile);
  MModuleTACCalibration SevenColumns;
  bool SevenColumnResult = SevenColumns.LoadTACCalFile(SevenColumnFile);
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("LoadTACCalFile()", "no strip map", "LoadTACCalFile() succeeds without a strip map", Result) && Passed;
  TACCalTable Table = Module.GetTACCalParameters();
  Passed = Evaluate("LoadTACCalFile()", "no strip map", "The table contains the file's detectors 0 and 7", (unsigned int) Table.size(), 2U) && Passed;
  bool HasDet0 = Table.count(0) == 1 && Table[0].size() == 2;
  Passed = EvaluateTrue("LoadTACCalFile()", "no strip map", "Read-out ID 10 is stored under the file's detector 0, HV, strip 32", HasDet0 && Table[0][1].count(32) == 1) && Passed;
  Passed = EvaluateTrue("LoadTACCalFile()", "no strip map", "Read-out ID 11 is stored under the file's detector 0, LV, strip 5", HasDet0 && Table[0][0].count(5) == 1) && Passed;
  Passed = EvaluateTrue("LoadTACCalFile()", "no strip map", "No entry is stored under the strip-map detector 3", Table.count(3) == 0) && Passed;

  // The seven-column format has no read-out ID and can only be used without a strip map
  Passed = EvaluateTrue("LoadTACCalFile()", "seven columns, no strip map", "LoadTACCalFile() accepts the seven-column format without a strip map", SevenColumnResult) && Passed;
  TACCalTable SevenTable = SevenColumns.GetTACCalParameters();
  Passed = EvaluateTrue("LoadTACCalFile()", "seven columns, no strip map", "The seven-column entry is stored under detector 2, HV, strip 31", SevenTable.count(2) == 1 && SevenTable[2].size() == 2 && SevenTable[2][1].count(31) == 1) && Passed;

  MFile::Remove(StripMapFile);
  MFile::Remove(TACCalFile);
  MFile::Remove(SevenColumnFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestLoadErrors()
{
  bool Passed = true;

  const MString StripMapFile = GetFixturePath("_errors.map");
  const MString TACCalFile = GetFixturePath("_errors.csv");
  const MString SevenColumnFile = GetFixturePath("_errors_seven.csv");
  const MString BadSideFile = GetFixturePath("_errors_side.csv");

  vector<MString> SevenColumnLines = { "3,l,32,0.42,0.001,-520.0,5.5" };
  vector<MString> BadSideLines = { "10,0,x,32,0.45,0.001,-600.0,7.5" };
  Passed = EvaluateTrue("LoadTACCalFile()", "write fixtures", "The fixtures can be created", WriteStandardFixtures(StripMapFile, TACCalFile) && WriteFixture(SevenColumnFile, SevenColumnLines) && WriteFixture(BadSideFile, BadSideLines)) && Passed;

  MStripMap StripMap;
  Passed = EvaluateTrue("LoadTACCalFile()", "open strip map", "The strip map fixture can be opened", StripMap.Open(StripMapFile)) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MModuleTACCalibration SevenColumns;
  Passed = EvaluateFalse("LoadTACCalFile()", "seven columns with strip map", "LoadTACCalFile() fails for the seven-column format, which has no read-out ID, when a strip map is given", SevenColumns.LoadTACCalFile(SevenColumnFile, &StripMap)) && Passed;

  MModuleTACCalibration BadSide;
  Passed = EvaluateFalse("LoadTACCalFile()", "unknown side character", "LoadTACCalFile() fails for an unknown side character", BadSide.LoadTACCalFile(BadSideFile, &StripMap)) && Passed;

  MModuleTACCalibration Missing;
  DisableDefaultStreams();
  bool MissingResult = Missing.LoadTACCalFile(GetFixturePath("_does_not_exist.csv"), &StripMap);
  EnableDefaultStreams();
  Passed = EvaluateFalse("LoadTACCalFile()", "missing file", "LoadTACCalFile() fails when the TAC calibration file does not exist", MissingResult) && Passed;

  g_Verbosity = OldVerbosity;

  MFile::Remove(StripMapFile);
  MFile::Remove(TACCalFile);
  MFile::Remove(SevenColumnFile);
  MFile::Remove(BadSideFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestInitialize()
{
  bool Passed = true;

  // No module precedes the TAC calibration in the (empty) supervisor sequence,
  // thus the strip map has to come from the module's own option

  const MString StripMapFile = GetFixturePath("_init.map");
  const MString TACCalFile = GetFixturePath("_init.csv");
  const MString UnmappedFile = GetFixturePath("_init_unmapped.csv");

  vector<MString> UnmappedLines = { "99,0,0,10,0.40,0.001,-400.0,4.0" };
  Passed = EvaluateTrue("Initialize()", "write fixtures", "The fixtures can be created", WriteStandardFixtures(StripMapFile, TACCalFile) && WriteFixture(UnmappedFile, UnmappedLines)) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MModuleTACCalibration NoStripMap;
  NoStripMap.SetTACCalFileName(TACCalFile);
  Passed = EvaluateFalse("Initialize()", "no strip map", "Initialize() fails when neither a preceding module nor the options provide a strip map", NoStripMap.Initialize()) && Passed;

  MModuleTACCalibration MissingStripMap;
  MissingStripMap.SetTACCalFileName(TACCalFile);
  MissingStripMap.SetFileNameStripMap(GetFixturePath("_does_not_exist.map"));
  DisableDefaultStreams();
  bool MissingStripMapResult = MissingStripMap.Initialize();
  EnableDefaultStreams();
  Passed = EvaluateFalse("Initialize()", "missing strip map file", "Initialize() fails when the strip map file does not exist", MissingStripMapResult) && Passed;

  MModuleTACCalibration Unmapped;
  Unmapped.SetTACCalFileName(UnmappedFile);
  Unmapped.SetFileNameStripMap(StripMapFile);
  Passed = EvaluateFalse("Initialize()", "no mapped read-out ID", "Initialize() fails when no TAC calibration entry is in the strip map", Unmapped.Initialize()) && Passed;

  // Initialize twice to check that Finalize() leaves no stale state behind
  MModuleTACCalibration Module;
  Module.SetTACCalFileName(TACCalFile);
  Module.SetFileNameStripMap(StripMapFile);
  for (unsigned int Round = 1; Round <= 2; ++Round) {
    MString Input = MString("own strip map option, round ") + Round;
    bool Result = Module.Initialize();
    Passed = EvaluateTrue("Initialize()", Input, "Initialize() succeeds with the module's own strip map option", Result) && Passed;
    TACCalTable Table = Module.GetTACCalParameters();
    Passed = Evaluate("Initialize()", Input, "The table contains exactly the strip-map detectors 3 and 7", (unsigned int) Table.size(), 2U) && Passed;
    Passed = EvaluateTrue("Initialize()", Input, "Read-out ID 10 is stored under strip-map detector 3, LV, strip 32", Table.count(3) == 1 && Table[3].size() == 2 && Table[3][0].count(32) == 1) && Passed;
    Module.Finalize();
    Passed = Evaluate("Finalize()", Input, "Finalize() clears the TAC calibration table", (unsigned int) Module.GetTACCalParameters().size(), 0U) && Passed;
  }

  g_Verbosity = OldVerbosity;

  MFile::Remove(StripMapFile);
  MFile::Remove(TACCalFile);
  MFile::Remove(UnmappedFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestFindStripMapProvider()
{
  bool Passed = true;

  // The options GUI only offers the strip map option if FindStripMapProvider() returns nullptr
  MSupervisor* Supervisor = MSupervisor::GetSupervisor();
  Supervisor->Clear();

  MModuleTACCalibration Module;
  Passed = EvaluateTrue("FindStripMapProvider()", "empty sequence", "No provider is found when the TAC calibration is not in a sequence", Module.FindStripMapProvider() == nullptr) && Passed;

  MModuleLoaderMeasurementsHDF HDF;
  DisableDefaultStreams();
  Supervisor->SetModule(&HDF, 0);
  Supervisor->SetModule(&Module, 1);
  EnableDefaultStreams();
  Passed = Evaluate("FindStripMapProvider()", "HDF loader, TAC calibration", "The sequence contains both modules", Supervisor->GetNModules(), 2U) && Passed;
  Passed = EvaluateTrue("FindStripMapProvider()", "HDF loader, TAC calibration", "The preceding HDF loader is the strip map provider", Module.FindStripMapProvider() == &HDF) && Passed;
  Supervisor->Clear();

  MModuleLoaderMeasurementsL0 L0;
  DisableDefaultStreams();
  Supervisor->SetModule(&L0, 0);
  Supervisor->SetModule(&Module, 1);
  EnableDefaultStreams();
  Passed = Evaluate("FindStripMapProvider()", "L0 loader, TAC calibration", "The sequence contains both modules", Supervisor->GetNModules(), 2U) && Passed;
  Passed = EvaluateTrue("FindStripMapProvider()", "L0 loader, TAC calibration", "The preceding L0 loader is the strip map provider", Module.FindStripMapProvider() == &L0) && Passed;
  Supervisor->Clear();

  MModuleLoaderSimulationsCosima Simulations;
  DisableDefaultStreams();
  Supervisor->SetModule(&Simulations, 0);
  Supervisor->SetModule(&Module, 1);
  EnableDefaultStreams();
  Passed = Evaluate("FindStripMapProvider()", "simulation loader, TAC calibration", "The sequence contains both modules", Supervisor->GetNModules(), 2U) && Passed;
  Passed = EvaluateTrue("FindStripMapProvider()", "simulation loader, TAC calibration", "A preceding simulation loader is no strip map provider", Module.FindStripMapProvider() == nullptr) && Passed;
  Supervisor->Clear();

  MModuleDEESMEX DEE;
  DisableDefaultStreams();
  Supervisor->SetModule(&Simulations, 0);
  Supervisor->SetModule(&DEE, 1);
  Supervisor->SetModule(&Module, 2);
  EnableDefaultStreams();
  Passed = Evaluate("FindStripMapProvider()", "simulation loader, DEE, TAC calibration", "The sequence contains all three modules", Supervisor->GetNModules(), 3U) && Passed;
  Passed = EvaluateTrue("FindStripMapProvider()", "simulation loader, DEE, TAC calibration", "The preceding DEE is the strip map provider", Module.FindStripMapProvider() == &DEE) && Passed;

  // With a preceding provider, our own strip map option is never used - here the DEE has not loaded a strip map,
  // thus Initialize() fails even though the TAC calibration's own (hidden) option points to a valid strip map
  const MString StripMapFile = GetFixturePath("_provider.map");
  const MString TACCalFile = GetFixturePath("_provider.csv");
  Passed = EvaluateTrue("Initialize()", "write fixtures", "The fixtures can be created", WriteStandardFixtures(StripMapFile, TACCalFile)) && Passed;

  MModuleTACCalibration StaleOption;
  StaleOption.SetTACCalFileName(TACCalFile);
  StaleOption.SetFileNameStripMap(StripMapFile);
  DisableDefaultStreams();
  Supervisor->SetModule(&StaleOption, 2);
  EnableDefaultStreams();

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool StaleOptionResult = StaleOption.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("Initialize()", "DEE without strip map, own strip map option set", "Initialize() fails and does not fall back to its own strip map option when the preceding DEE has no strip map", StaleOptionResult) && Passed;
  Supervisor->Clear();

  MFile::Remove(StripMapFile);
  MFile::Remove(TACCalFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleTACCalibration::TestAnalyzeEvent()
{
  bool Passed = true;

  const MString StripMapFile = GetFixturePath("_analyze.map");
  const MString TACCalFile = GetFixturePath("_analyze.csv");
  Passed = EvaluateTrue("AnalyzeEvent()", "write fixtures", "The fixtures can be created", WriteStandardFixtures(StripMapFile, TACCalFile)) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MModuleTACCalibration Module;
  Module.SetTACCalFileName(TACCalFile);
  Module.SetFileNameStripMap(StripMapFile);
  Module.SetApplyTACCuts(false);
  bool Initialized = Module.Initialize();
  Passed = EvaluateTrue("AnalyzeEvent()", "initialize", "Initialize() succeeds with the module's own strip map option", Initialized) && Passed;

  if (Initialized == true) {
    // A hit on the strip-map channel of read-out ID 10 is calibrated with the constants of read-out ID 10
    MReadOutAssembly Mapped;
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(3);
    SH->IsLowVoltageStrip(true);
    SH->SetStripID(32);
    SH->SetTAC(2000.0);
    Mapped.AddStripHit(SH);

    Passed = EvaluateTrue("AnalyzeEvent()", "hit on strip-map channel", "AnalyzeEvent() succeeds for a hit on detector 3, LV, strip 32", Module.AnalyzeEvent(&Mapped)) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "hit on strip-map channel", "No TAC calibration error is flagged for a hit on detector 3, LV, strip 32", Mapped.HasTACCalibrationError()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "hit on strip-map channel", "The timing is 2000 * 0.45 - 600 = 300 ns", Mapped.GetStripHit(0)->GetTiming(), 300.0, 1e-9) && Passed;

    // A hit on the detector named in the file's detector column has no calibration
    MReadOutAssembly FileColumn;
    MStripHit* FileColumnSH = new MStripHit();
    FileColumnSH->SetDetectorID(0);
    FileColumnSH->IsLowVoltageStrip(false);
    FileColumnSH->SetStripID(32);
    FileColumnSH->SetTAC(2000.0);
    FileColumn.AddStripHit(FileColumnSH);

    Passed = EvaluateFalse("AnalyzeEvent()", "hit on file-column detector", "AnalyzeEvent() fails for a hit on detector 0, which only the file's detector column names", Module.AnalyzeEvent(&FileColumn)) && Passed;

    Module.Finalize();
  }

  g_Verbosity = OldVerbosity;

  MFile::Remove(StripMapFile);
  MFile::Remove(TACCalFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNModuleTACCalibration", "Unit tests for MModuleTACCalibration") == false) return 1;

  UTNModuleTACCalibration Test;
  return Test.Run() == true ? 0 : 1;
}
