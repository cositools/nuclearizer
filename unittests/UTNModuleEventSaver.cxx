/*
 * UTNModuleEventSaver.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MFile.h"
#include "MTime.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleEventSaver.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MModuleEventSaver
class UTNModuleEventSaver : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleEventSaver() : MUnitTest("UTNModuleEventSaver") {}
  //! Default destructor
  virtual ~UTNModuleEventSaver() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and the defaults
  bool TestDefaultConstruction();
  //! Test all setter/getter pairs
  bool TestGettersSetters();
  //! Test the XML configuration round trip
  bool TestXmlRoundTrip();
  //! Test the Initialize() error paths
  bool TestInitializeErrors();
  //! Test the file naming for all modes
  bool TestFileNames();
  //! Test writing a roa file
  bool TestWriteRoa();
  //! Test writing evta, dat, and tra files
  bool TestWriteOtherFormats();
  //! Test the selection of bad, poor quality and veto events
  bool TestEventSelection();
  //! Test the time-based file splitting
  bool TestSplitFile();
  //! Test the time-based file splitting for measured events which only have a UTC time
  bool TestSplitFileUTCOnly();
  //! Test the splitting by detector and side
  bool TestSplitByDetectorSide();
  //! Test the simulation header and trailer
  bool TestSimulationInfo();

  //! Create an event with strip hits in the given detectors
  MReadOutAssembly* CreateEvent(unsigned long ID, double Time, vector<unsigned int> Detectors = { 0 });
  //! Return the event IDs of a written file in file order, separated by single spaces
  MString ReadEventIDs(const MString& FileName);
  //! Count the lines in a text file which start with the given prefix
  unsigned int CountLines(const MString& FileName, const MString& Prefix);
  //! Return the last non-empty line of a text file
  MString LastLine(const MString& FileName);
  //! Return all file names in a directory
  vector<MString> ListDirectory(const MString& Directory);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestInitializeErrors() && Passed;
  Passed = TestFileNames() && Passed;
  Passed = TestWriteRoa() && Passed;
  Passed = TestWriteOtherFormats() && Passed;
  Passed = TestEventSelection() && Passed;
  Passed = TestSplitFile() && Passed;
  Passed = TestSplitFileUTCOnly() && Passed;
  Passed = TestSplitByDetectorSide() && Passed;
  Passed = TestSimulationInfo() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* UTNModuleEventSaver::CreateEvent(unsigned long ID, double Time, vector<unsigned int> Detectors)
{
  MReadOutAssembly* E = new MReadOutAssembly();
  E->SetID(ID);
  E->SetTime(MTime(Time));
  E->SetTimeUTC(MTime(Time));
  for (unsigned int D : Detectors) {
    MStripHit* LV = new MStripHit();
    LV->SetDetectorID(D);
    LV->SetStripID(10);
    LV->IsLowVoltageStrip(true);
    LV->SetADCUnits(4053);
    LV->SetTAC(10452);
    LV->SetEnergy(511.0);
    E->AddStripHit(LV);
    MStripHit* HV = new MStripHit();
    HV->SetDetectorID(D);
    HV->SetStripID(20);
    HV->IsLowVoltageStrip(false);
    HV->SetADCUnits(4000);
    HV->SetTAC(10400);
    HV->SetEnergy(511.0);
    E->AddStripHit(HV);
  }
  return E;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleEventSaver::ReadEventIDs(const MString& FileName)
{
  istringstream In(ReadTextFile(FileName).GetString());
  string Line;
  MString IDs;
  while (getline(In, Line).fail() == false) {
    MString L(Line);
    if (L.BeginsWith("ID ") == true) {
      if (IDs.IsEmpty() == false) IDs += " ";
      IDs += L.GetSubString(3).Strip();
    }
  }
  return IDs;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTNModuleEventSaver::CountLines(const MString& FileName, const MString& Prefix)
{
  istringstream In(ReadTextFile(FileName).GetString());
  string Line;
  unsigned int N = 0;
  while (getline(In, Line).fail() == false) {
    if (MString(Line).BeginsWith(Prefix) == true) ++N;
  }
  return N;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleEventSaver::LastLine(const MString& FileName)
{
  istringstream In(ReadTextFile(FileName).GetString());
  string Line;
  MString Last;
  while (getline(In, Line).fail() == false) {
    if (Line.empty() == false) Last = Line;
  }
  return Last;
}


////////////////////////////////////////////////////////////////////////////////


vector<MString> UTNModuleEventSaver::ListDirectory(const MString& Directory)
{
  vector<MString> Names;
  error_code Error;
  for (const auto& Entry : filesystem::directory_iterator(Directory.Data(), Error)) {
    Names.push_back(MString(Entry.path().filename().string()));
  }
  return Names;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleEventSaver M;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagEventSaver'", M.GetXmlTag(), MString("XmlTagEventSaver")) && Passed;
  Passed = EvaluateFalse("GetName()", "default", "The name is not empty", M.GetName().IsEmpty()) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_EventSaver", M.ProvidesModuleType(MAssembly::c_EventSaver)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "An event loader is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = Evaluate("GetMode()", "default", "The default mode is evta", M.GetMode(), MModuleEventSaver::c_EvtaFile) && Passed;
  Passed = Evaluate("GetFileName()", "default", "The default file name is 'MyEvtaFileForRevan.evta'", M.GetFileName(), MString("MyEvtaFileForRevan.evta")) && Passed;
  Passed = EvaluateTrue("GetSaveBadEvents()", "default", "Bad events are saved by default", M.GetSaveBadEvents()) && Passed;
  Passed = EvaluateTrue("GetSavePoorQualityEvents()", "default", "Poor quality events are saved by default", M.GetSavePoorQualityEvents()) && Passed;
  Passed = EvaluateTrue("GetSaveVetoEvents()", "default", "Veto events are saved by default", M.GetSaveVetoEvents()) && Passed;
  Passed = EvaluateFalse("GetAddTimeTag()", "default", "No time tag by default", M.GetAddTimeTag()) && Passed;
  Passed = EvaluateTrue("GetSplitFile()", "default", "The file is split by default", M.GetSplitFile()) && Passed;
  Passed = EvaluateNear("GetSplitFileTime()", "default", "The default split time is 600 s", M.GetSplitFileTime().GetAsDouble(), 600.0, 1e-9) && Passed;
  Passed = EvaluateFalse("GetSplitByDetectorSide()", "default", "No split by detector side by default", M.GetSplitByDetectorSide()) && Passed;
  Passed = EvaluateTrue("GetRoaWithADCs()", "default", "ADCs are written by default", M.GetRoaWithADCs()) && Passed;
  Passed = EvaluateTrue("GetRoaWithTACs()", "default", "TACs are written by default", M.GetRoaWithTACs()) && Passed;
  Passed = EvaluateFalse("GetRoaWithEnergies()", "default", "Energies are not written by default", M.GetRoaWithEnergies()) && Passed;
  Passed = EvaluateFalse("GetRoaWithTimings()", "default", "Timings are not written by default", M.GetRoaWithTimings()) && Passed;
  Passed = EvaluateFalse("GetRoaWithTemperatures()", "default", "Temperatures are not written by default", M.GetRoaWithTemperatures()) && Passed;
  Passed = EvaluateFalse("GetRoaWithFlags()", "default", "Flags are not written by default", M.GetRoaWithFlags()) && Passed;
  Passed = EvaluateFalse("GetRoaWithOrigins()", "default", "Origins are not written by default", M.GetRoaWithOrigins()) && Passed;
  Passed = EvaluateTrue("GetRoaWithNearestNeighbors()", "default", "Nearest neighbors are written by default", M.GetRoaWithNearestNeighbors()) && Passed;

  Passed = Evaluate("Mode constants", "c_RoaFile", "c_RoaFile is 0", MModuleEventSaver::c_RoaFile, 0u) && Passed;
  Passed = Evaluate("Mode constants", "c_DatFile", "c_DatFile is 1", MModuleEventSaver::c_DatFile, 1u) && Passed;
  Passed = Evaluate("Mode constants", "c_EvtaFile", "c_EvtaFile is 2", MModuleEventSaver::c_EvtaFile, 2u) && Passed;
  Passed = Evaluate("Mode constants", "c_TraFile", "c_TraFile is 3", MModuleEventSaver::c_TraFile, 3u) && Passed;
  Passed = Evaluate("Mode constants", "c_SimFile", "c_SimFile is 4", MModuleEventSaver::c_SimFile, 4u) && Passed;

  MModuleEventSaver* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestGettersSetters()
{
  bool Passed = true;

  MModuleEventSaver M;
  M.SetMode(MModuleEventSaver::c_TraFile);
  Passed = Evaluate("SetMode/GetMode", "tra", "GetMode returns c_TraFile", M.GetMode(), MModuleEventSaver::c_TraFile) && Passed;
  M.SetFileName("out.tra");
  Passed = Evaluate("SetFileName/GetFileName", "out.tra", "GetFileName returns the set name", M.GetFileName(), MString("out.tra")) && Passed;
  M.SetSplitFileTime(MTime(42));
  Passed = EvaluateNear("SetSplitFileTime/GetSplitFileTime", "42 s", "GetSplitFileTime returns 42 s", M.GetSplitFileTime().GetAsDouble(), 42.0, 1e-9) && Passed;

  for (int i = 0; i < 2; ++i) {
    bool F = (i == 1);
    MString Input = (F == true) ? "true" : "false";
    M.SetSaveBadEvents(F);
    Passed = EvaluateTrue("SetSaveBadEvents/GetSaveBadEvents", Input, "Returns the set value", M.GetSaveBadEvents() == F) && Passed;
    M.SetSavePoorQualityEvents(F);
    Passed = EvaluateTrue("SetSavePoorQualityEvents/GetSavePoorQualityEvents", Input, "Returns the set value", M.GetSavePoorQualityEvents() == F) && Passed;
    M.SetSaveVetoEvents(F);
    Passed = EvaluateTrue("SetSaveVetoEvents/GetSaveVetoEvents", Input, "Returns the set value", M.GetSaveVetoEvents() == F) && Passed;
    M.SetAddTimeTag(F);
    Passed = EvaluateTrue("SetAddTimeTag/GetAddTimeTag", Input, "Returns the set value", M.GetAddTimeTag() == F) && Passed;
    M.SetSplitFile(F);
    Passed = EvaluateTrue("SetSplitFile/GetSplitFile", Input, "Returns the set value", M.GetSplitFile() == F) && Passed;
    M.SetSplitByDetectorSide(F);
    Passed = EvaluateTrue("SetSplitByDetectorSide/GetSplitByDetectorSide", Input, "Returns the set value", M.GetSplitByDetectorSide() == F) && Passed;
    M.SetRoaWithADCs(F);
    Passed = EvaluateTrue("SetRoaWithADCs/GetRoaWithADCs", Input, "Returns the set value", M.GetRoaWithADCs() == F) && Passed;
    M.SetRoaWithTACs(F);
    Passed = EvaluateTrue("SetRoaWithTACs/GetRoaWithTACs", Input, "Returns the set value", M.GetRoaWithTACs() == F) && Passed;
    M.SetRoaWithEnergies(F);
    Passed = EvaluateTrue("SetRoaWithEnergies/GetRoaWithEnergies", Input, "Returns the set value", M.GetRoaWithEnergies() == F) && Passed;
    M.SetRoaWithTimings(F);
    Passed = EvaluateTrue("SetRoaWithTimings/GetRoaWithTimings", Input, "Returns the set value", M.GetRoaWithTimings() == F) && Passed;
    M.SetRoaWithTemperatures(F);
    Passed = EvaluateTrue("SetRoaWithTemperatures/GetRoaWithTemperatures", Input, "Returns the set value", M.GetRoaWithTemperatures() == F) && Passed;
    M.SetRoaWithFlags(F);
    Passed = EvaluateTrue("SetRoaWithFlags/GetRoaWithFlags", Input, "Returns the set value", M.GetRoaWithFlags() == F) && Passed;
    M.SetRoaWithOrigins(F);
    Passed = EvaluateTrue("SetRoaWithOrigins/GetRoaWithOrigins", Input, "Returns the set value", M.GetRoaWithOrigins() == F) && Passed;
    M.SetRoaWithNearestNeighbors(F);
    Passed = EvaluateTrue("SetRoaWithNearestNeighbors/GetRoaWithNearestNeighbors", Input, "Returns the set value", M.GetRoaWithNearestNeighbors() == F) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleEventSaver Writer;
  Writer.SetMode(MModuleEventSaver::c_RoaFile);
  Writer.SetFileName("/data/out.roa");
  Writer.SetSaveBadEvents(false);
  Writer.SetSavePoorQualityEvents(false);
  Writer.SetSaveVetoEvents(false);
  Writer.SetAddTimeTag(true);
  Writer.SetSplitFile(false);
  Writer.SetSplitFileTime(MTime(1800));
  Writer.SetSplitByDetectorSide(true);
  Writer.SetRoaWithADCs(false);
  Writer.SetRoaWithTACs(false);
  Writer.SetRoaWithEnergies(true);
  Writer.SetRoaWithTimings(true);
  Writer.SetRoaWithTemperatures(true);
  Writer.SetRoaWithFlags(true);
  Writer.SetRoaWithOrigins(true);
  Writer.SetRoaWithNearestNeighbors(false);
  MXmlNode* Node = Writer.CreateXmlConfiguration();

  MModuleEventSaver Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "all options inverted", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "Mode", "The mode survives", Reader.GetMode(), MModuleEventSaver::c_RoaFile) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "FileName", "The file name survives", Reader.GetFileName(), MString("/data/out.roa")) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "SaveBadEvents", "SaveBadEvents survives", Reader.GetSaveBadEvents()) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "SavePoorQualityEvents", "SavePoorQualityEvents survives", Reader.GetSavePoorQualityEvents()) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "SaveVetoEvents", "SaveVetoEvents survives", Reader.GetSaveVetoEvents()) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "AddTimeTag", "AddTimeTag survives", Reader.GetAddTimeTag()) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "SplitFile", "SplitFile survives", Reader.GetSplitFile()) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "SplitFileTime", "SplitFileTime survives", Reader.GetSplitFileTime().GetAsDouble(), 1800.0, 1e-9) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "SplitByDetectorSide", "SplitByDetectorSide survives", Reader.GetSplitByDetectorSide()) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "RoaWithADCs", "RoaWithADCs survives", Reader.GetRoaWithADCs()) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "RoaWithTACs", "RoaWithTACs survives", Reader.GetRoaWithTACs()) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "RoaWithEnergies", "RoaWithEnergies survives", Reader.GetRoaWithEnergies()) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "RoaWithTimings", "RoaWithTimings survives", Reader.GetRoaWithTimings()) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "RoaWithTemperatures", "RoaWithTemperatures survives", Reader.GetRoaWithTemperatures()) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "RoaWithFlags", "RoaWithFlags survives", Reader.GetRoaWithFlags()) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "RoaWithOrigins", "RoaWithOrigins survives", Reader.GetRoaWithOrigins()) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "RoaWithNearestNeighbors", "RoaWithNearestNeighbors survives", Reader.GetRoaWithNearestNeighbors()) && Passed;
  delete Node;

  // Split times are whole seconds by design
  MModuleEventSaver SecondsWriter;
  SecondsWriter.SetSplitFileTime(MTime(90));
  MXmlNode* SecondsNode = SecondsWriter.CreateXmlConfiguration();
  MModuleEventSaver SecondsReader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "SplitFileTime 90 s", "ReadXmlConfiguration returns true", SecondsReader.ReadXmlConfiguration(SecondsNode)) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "SplitFileTime 90 s", "An integer split time survives", SecondsReader.GetSplitFileTime().GetAsDouble(), 90.0, 1e-9) && Passed;
  delete SecondsNode;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestInitializeErrors()
{
  bool Passed = true;

  PrepareTemporaryDirectory("errors");
  MString Dir = GetTemporaryDirectoryName("errors");

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MModuleEventSaver Sim;
  Sim.SetMode(MModuleEventSaver::c_SimFile);
  Sim.SetFileName(Dir + "/sim.sim");
  bool SimResult = Sim.Initialize();

  MModuleEventSaver Split;
  Split.SetMode(MModuleEventSaver::c_TraFile);
  Split.SetSplitByDetectorSide(true);
  Split.SetFileName(Dir + "/split.tra");
  bool SplitResult = Split.Initialize();

  MModuleEventSaver NoData;
  NoData.SetMode(MModuleEventSaver::c_RoaFile);
  NoData.SetRoaWithADCs(false);
  NoData.SetRoaWithEnergies(false);
  NoData.SetFileName(Dir + "/nodata.roa");
  bool NoDataResult = NoData.Initialize();

  MModuleEventSaver BadDir;
  BadDir.SetMode(MModuleEventSaver::c_TraFile);
  BadDir.SetFileName(Dir + "/does/not/exist/out.tra");
  bool BadDirResult = BadDir.Initialize();

  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "sim mode", "The unsupported sim mode is rejected", SimResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "split by detector side, tra mode", "Splitting by detector side is rejected for non-roa files", SplitResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "roa without ADCs and energies", "A roa file without crystal data is rejected", NoDataResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "non-existing directory", "An unwritable output file is rejected", BadDirResult) && Passed;

  RemoveTemporaryDirectory("errors");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestFileNames()
{
  bool Passed = true;

  PrepareTemporaryDirectory("names");
  MString Dir = GetTemporaryDirectoryName("names");

  // The suffix of the file name is replaced by the suffix of the mode
  MModuleEventSaver Dat;
  Dat.SetMode(MModuleEventSaver::c_DatFile);
  Dat.SetSplitFile(false);
  Dat.SetFileName(Dir + "/a.evta");
  Passed = EvaluateTrue("Initialize()", "a.evta in dat mode", "Initialize succeeds", Dat.Initialize()) && Passed;
  Dat.Finalize();
  Passed = EvaluateTrue("Initialize()", "a.evta in dat mode", "The file a.dat is created", MFile::Exists(Dir + "/a.dat")) && Passed;
  Passed = EvaluateFalse("Initialize()", "a.evta in dat mode", "The file a.evta is not created", MFile::Exists(Dir + "/a.evta")) && Passed;

  // No suffix
  MModuleEventSaver Tra;
  Tra.SetMode(MModuleEventSaver::c_TraFile);
  Tra.SetSplitFile(false);
  Tra.SetFileName(Dir + "/b");
  Passed = EvaluateTrue("Initialize()", "b in tra mode", "Initialize succeeds", Tra.Initialize()) && Passed;
  Tra.Finalize();
  Passed = EvaluateTrue("Initialize()", "b in tra mode", "The file b.tra is created", MFile::Exists(Dir + "/b.tra")) && Passed;

  // Unknown suffix is kept
  MModuleEventSaver Unknown;
  Unknown.SetMode(MModuleEventSaver::c_EvtaFile);
  Unknown.SetSplitFile(false);
  Unknown.SetFileName(Dir + "/c.txt");
  Passed = EvaluateTrue("Initialize()", "c.txt in evta mode", "Initialize succeeds", Unknown.Initialize()) && Passed;
  Unknown.Finalize();
  Passed = EvaluateTrue("Initialize()", "c.txt in evta mode", "The file c.txt.evta is created", MFile::Exists(Dir + "/c.txt.evta")) && Passed;

  // gzip
  MModuleEventSaver Zip;
  Zip.SetMode(MModuleEventSaver::c_RoaFile);
  Zip.SetSplitFile(false);
  Zip.SetFileName(Dir + "/d.roa.gz");
  Passed = EvaluateTrue("Initialize()", "d.roa.gz in roa mode", "Initialize succeeds", Zip.Initialize()) && Passed;
  Zip.Finalize();
  Passed = EvaluateTrue("Initialize()", "d.roa.gz in roa mode", "The file d.roa.gz is created", MFile::Exists(Dir + "/d.roa.gz")) && Passed;

  // Time tag
  MModuleEventSaver Tag;
  Tag.SetMode(MModuleEventSaver::c_TraFile);
  Tag.SetSplitFile(false);
  Tag.SetAddTimeTag(true);
  Tag.SetFileName(Dir + "/e.tra");
  Passed = EvaluateTrue("Initialize()", "e.tra with time tag", "Initialize succeeds", Tag.Initialize()) && Passed;
  Tag.Finalize();
  bool TaggedFound = false;
  for (MString& Name : ListDirectory(Dir)) {
    if (Name.BeginsWith("e.") == true && Name.EndsWith(".tra") == true && Name != "e.tra") TaggedFound = true;
  }
  Passed = EvaluateTrue("Initialize()", "e.tra with time tag", "A time-tagged file e.<time>.tra is created", TaggedFound) && Passed;
  Passed = EvaluateFalse("Initialize()", "e.tra with time tag", "The untagged file e.tra is not created", MFile::Exists(Dir + "/e.tra")) && Passed;

  RemoveTemporaryDirectory("names");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestWriteRoa()
{
  bool Passed = true;

  PrepareTemporaryDirectory("roa");
  MString Dir = GetTemporaryDirectoryName("roa");

  MModuleEventSaver M;
  M.SetMode(MModuleEventSaver::c_RoaFile);
  M.SetSplitFile(false);
  M.SetRoaWithEnergies(true);
  M.SetFileName(Dir + "/out.roa");
  Passed = EvaluateTrue("Initialize()", "roa", "Initialize succeeds", M.Initialize()) && Passed;

  for (unsigned long e = 1; e <= 3; ++e) {
    MReadOutAssembly* E = CreateEvent(e, 100.0 + e);
    Passed = EvaluateTrue("AnalyzeEvent()", MString("roa event ") + e, "AnalyzeEvent returns true", M.AnalyzeEvent(E)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", MString("roa event ") + e, "The c_EventSaver analysis progress is set", E->HasAnalysisProgress(MAssembly::c_EventSaver)) && Passed;
    delete E;
  }
  M.Finalize();

  MString File = Dir + "/out.roa";
  Passed = EvaluateTrue("Finalize()", "roa", "The roa file exists", MFile::Exists(File)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "roa", "The header declares the file type", CountLines(File, "TYPE ROA"), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "roa", "Two read-out unit definitions (strips and crystals) are written", CountLines(File, "UF "), 2u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "roa", "Three events are written", CountLines(File, "SE"), 3u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "roa", "Six strip hits are written", CountLines(File, "UH "), 6u) && Passed;
  Passed = Evaluate("Finalize()", "roa", "The file ends with EN", LastLine(File), MString("EN")) && Passed;

  RemoveTemporaryDirectory("roa");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestWriteOtherFormats()
{
  bool Passed = true;

  PrepareTemporaryDirectory("formats");
  MString Dir = GetTemporaryDirectoryName("formats");

  const vector<unsigned int> Modes = { MModuleEventSaver::c_EvtaFile, MModuleEventSaver::c_DatFile, MModuleEventSaver::c_TraFile };
  const vector<MString> Suffixes = { ".evta", ".dat", ".tra" };
  const vector<MString> Types = { "Type EVTA", "Type DAT", "TYPE TRA" };
  for (unsigned int m = 0; m < Modes.size(); ++m) {
    MModuleEventSaver M;
    M.SetMode(Modes[m]);
    M.SetSplitFile(false);
    M.SetFileName(Dir + "/out" + Suffixes[m]);
    Passed = EvaluateTrue("Initialize()", Suffixes[m], "Initialize succeeds", M.Initialize()) && Passed;
    for (unsigned long e = 1; e <= 2; ++e) {
      MReadOutAssembly* E = CreateEvent(e, 10.0*e);
      M.AnalyzeEvent(E);
      delete E;
    }
    M.Finalize();
    MString File = Dir + "/out" + Suffixes[m];
    Passed = EvaluateTrue("Finalize()", Suffixes[m], "The file exists", MFile::Exists(File)) && Passed;
    Passed = Evaluate("Initialize()", Suffixes[m], "The header declares the file type", CountLines(File, Types[m]), 1u) && Passed;
    Passed = Evaluate("Finalize()", Suffixes[m], "The file ends with EN", LastLine(File), MString("EN")) && Passed;
    if (Modes[m] != MModuleEventSaver::c_DatFile) {
      Passed = Evaluate("AnalyzeEvent()", Suffixes[m], "Two events are written", CountLines(File, "SE"), 2u) && Passed;
    }
  }

  RemoveTemporaryDirectory("formats");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestEventSelection()
{
  bool Passed = true;

  PrepareTemporaryDirectory("selection");
  MString Dir = GetTemporaryDirectoryName("selection");

  // All combinations of the three options: each option must only control its own event category
  for (unsigned int Combination = 0; Combination < 8; ++Combination) {
    bool SaveBad = (Combination & 1) != 0;
    bool SavePoor = (Combination & 2) != 0;
    bool SaveVeto = (Combination & 4) != 0;
    MModuleEventSaver M;
    M.SetMode(MModuleEventSaver::c_RoaFile);
    M.SetSplitFile(false);
    M.SetSaveBadEvents(SaveBad);
    M.SetSavePoorQualityEvents(SavePoor);
    M.SetSaveVetoEvents(SaveVeto);
    MString File = Dir + "/selection" + Combination + ".roa";
    M.SetFileName(File);
    Passed = EvaluateTrue("Initialize()", MString("selection ") + Combination, "Initialize succeeds", M.Initialize()) && Passed;

    MReadOutAssembly* Good = CreateEvent(1, 1.0);
    M.AnalyzeEvent(Good);
    MReadOutAssembly* Bad = CreateEvent(2, 2.0);
    Bad->SetEnergyCalibrationError("test");
    M.AnalyzeEvent(Bad);
    MReadOutAssembly* Poor = CreateEvent(3, 3.0);
    Poor->SetStripPairing_QualityFlag("test");
    M.AnalyzeEvent(Poor);
    MReadOutAssembly* Veto = CreateEvent(4, 4.0);
    Veto->SetShieldVeto(true);
    M.AnalyzeEvent(Veto);
    M.Finalize();

    MString Input = MString("save bad ") + (SaveBad == true ? "on" : "off") + ", poor quality " + (SavePoor == true ? "on" : "off") + ", veto " + (SaveVeto == true ? "on" : "off");
    MString Expected = "1";
    if (SaveBad == true) Expected += " 2";
    if (SavePoor == true) Expected += " 3";
    if (SaveVeto == true) Expected += " 4";
    Passed = Evaluate("AnalyzeEvent()", Input, "Exactly the good event (1) and the enabled categories bad (2), poor quality (3), veto (4) are written", ReadEventIDs(File), Expected) && Passed;
    delete Good;
    delete Bad;
    delete Poor;
    delete Veto;
  }

  RemoveTemporaryDirectory("selection");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestSplitFile()
{
  bool Passed = true;

  PrepareTemporaryDirectory("split");
  MString Dir = GetTemporaryDirectoryName("split");

  MModuleEventSaver M;
  M.SetMode(MModuleEventSaver::c_RoaFile);
  M.SetSplitFile(true);
  M.SetSplitFileTime(MTime(10));
  M.SetFileName(Dir + "/split.roa");
  Passed = EvaluateTrue("Initialize()", "split every 10 s", "Initialize succeeds", M.Initialize()) && Passed;

  // Times: 5 s is before the first split time, 100 and 105 go into one sub-file, 120 into the next
  const vector<double> Times = { 5.0, 100.0, 105.0, 120.0 };
  unsigned long ID = 0;
  for (double T : Times) {
    MReadOutAssembly* E = CreateEvent(++ID, T);
    M.AnalyzeEvent(E);
    delete E;
  }
  M.Finalize();

  MString Main = Dir + "/split.roa";
  Passed = EvaluateTrue("Finalize()", "split every 10 s", "The main file exists", MFile::Exists(Main)) && Passed;
  Passed = Evaluate("Finalize()", "split every 10 s", "The main file does not contain events itself", CountLines(Main, "SE"), 0u) && Passed;

  unsigned int SubFiles = 0;
  unsigned int Events = 0;
  bool AllEndWithEN = true;
  vector<MString> Groups;
  for (MString& Name : ListDirectory(Dir)) {
    if (Name == "split.roa") continue;
    ++SubFiles;
    Events += CountLines(Dir + "/" + Name, "SE");
    if (LastLine(Dir + "/" + Name) != "EN") AllEndWithEN = false;
    MString IDs = ReadEventIDs(Dir + "/" + Name);
    if (IDs.IsEmpty() == false) Groups.push_back(IDs);
  }
  sort(Groups.begin(), Groups.end());
  MString Distribution;
  for (unsigned int g = 0; g < Groups.size(); ++g) {
    if (g > 0) Distribution += " | ";
    Distribution += Groups[g];
  }
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "split every 10 s", "Events at 5 s, 100 + 105 s, and 120 s are in separate sub-files", Distribution, MString("1 | 2 3 | 4"))) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "split every 10 s", "The main file references every sub-file", CountLines(Main, "IN "), SubFiles) && Passed;
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "split every 10 s", "Three sub-files are created (5 s, 100 + 105 s, 120 s)", SubFiles, 3u)) && Passed;
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "split every 10 s", "No event is lost, also not the one before the first split time", Events, 4u)) && Passed;
  Passed = EvaluateTrue("Finalize()", "split every 10 s", "Every sub-file ends with EN", AllEndWithEN) && Passed;

  RemoveTemporaryDirectory("split");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestSplitFileUTCOnly()
{
  bool Passed = true;

  PrepareTemporaryDirectory("splitutc");
  MString Dir = GetTemporaryDirectoryName("splitutc");

  MModuleEventSaver M;
  M.SetMode(MModuleEventSaver::c_RoaFile);
  M.SetSplitFile(true);
  M.SetSplitFileTime(MTime(10));
  M.SetFileName(Dir + "/split.roa");
  Passed = EvaluateTrue("Initialize()", "split every 10 s, UTC-only events", "Initialize succeeds", M.Initialize()) && Passed;

  // The HDF, L0, and FITS loaders only set the UTC or RTS time, not the generic event time
  // All times are after the first split time: 100 and 105 s go into one sub-file, 120 s into the next
  const vector<double> Times = { 100.0, 105.0, 120.0 };
  unsigned long ID = 0;
  for (double T : Times) {
    MReadOutAssembly* E = CreateEvent(++ID, 0.0);
    E->SetTime(MTime(0));
    E->SetTimeUTC(MTime(T));
    M.AnalyzeEvent(E);
    delete E;
  }
  M.Finalize();

  unsigned int Events = 0;
  vector<MString> Groups;
  for (MString& Name : ListDirectory(Dir)) {
    if (Name == "split.roa") continue;
    Events += CountLines(Dir + "/" + Name, "SE");
    MString IDs = ReadEventIDs(Dir + "/" + Name);
    if (IDs.IsEmpty() == false) Groups.push_back(IDs);
  }
  sort(Groups.begin(), Groups.end());
  MString Distribution;
  for (unsigned int g = 0; g < Groups.size(); ++g) {
    if (g > 0) Distribution += " | ";
    Distribution += Groups[g];
  }
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "split every 10 s, UTC-only events", "All three events are written", Events, 3u)) && Passed;
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "split every 10 s, UTC-only events at 100, 105, 120 s", "The events are split by their UTC time", Distribution, MString("1 2 | 3"))) && Passed;

  RemoveTemporaryDirectory("splitutc");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestSplitByDetectorSide()
{
  bool Passed = true;

  PrepareTemporaryDirectory("sides");
  MString Dir = GetTemporaryDirectoryName("sides");

  MModuleEventSaver M;
  M.SetMode(MModuleEventSaver::c_RoaFile);
  M.SetSplitByDetectorSide(true);
  M.SetRoaWithNearestNeighbors(false);
  M.SetFileName(Dir + "/sides.roa");
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Init = M.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("Initialize()", "split by detector side", "Initialize succeeds", Init) && Passed;

  MReadOutAssembly* E1 = CreateEvent(1, 1.0, { 0, 1 });
  MStripHit* NN = new MStripHit();
  NN->SetDetectorID(2);
  NN->SetStripID(3);
  NN->IsLowVoltageStrip(true);
  NN->IsNearestNeighbor(true);
  E1->AddStripHit(NN);
  Passed = EvaluateTrue("AnalyzeEvent()", "detectors 0 and 1", "AnalyzeEvent returns true", M.AnalyzeEvent(E1)) && Passed;
  MReadOutAssembly* E2 = CreateEvent(2, 2.0, { 0 });
  M.AnalyzeEvent(E2);
  M.Finalize();
  delete E1;
  delete E2;

  Passed = EvaluateFalse("Initialize()", "split by detector side", "No main file is created", MFile::Exists(Dir + "/sides.roa")) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "detector 0, LV", "The file sides.det0.lv.roa exists", MFile::Exists(Dir + "/sides.det0.lv.roa")) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "detector 0, HV", "The file sides.det0.hv.roa exists", MFile::Exists(Dir + "/sides.det0.hv.roa")) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "detector 1, LV", "The file sides.det1.lv.roa exists", MFile::Exists(Dir + "/sides.det1.lv.roa")) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "detector 2 nearest neighbor only", "No file is created for a side with only excluded nearest neighbors", MFile::Exists(Dir + "/sides.det2.lv.roa")) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "detector 0, LV", "Both events are in the detector 0 LV file", CountLines(Dir + "/sides.det0.lv.roa", "SE"), 2u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "detector 1, LV", "One event is in the detector 1 LV file", CountLines(Dir + "/sides.det1.lv.roa", "SE"), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "detector 0, LV", "Only LV strip hits are in the LV file", CountLines(Dir + "/sides.det0.lv.roa", "UH "), 2u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "detector 0, LV", "The side files have no crystal unit definition", CountLines(Dir + "/sides.det0.lv.roa", "UF "), 1u) && Passed;
  Passed = Evaluate("Finalize()", "detector 0, LV", "The side file ends with EN", LastLine(Dir + "/sides.det0.lv.roa"), MString("EN")) && Passed;

  RemoveTemporaryDirectory("sides");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventSaver::TestSimulationInfo()
{
  bool Passed = true;

  PrepareTemporaryDirectory("sim");
  MString Dir = GetTemporaryDirectoryName("sim");

  MModuleEventSaver M;
  M.SetMode(MModuleEventSaver::c_TraFile);
  M.SetSplitFile(false);
  M.SetStartAreaFarField(3.5);
  M.SetSimulatedEvents(12345);
  M.SetFileName(Dir + "/sim.tra");
  Passed = EvaluateTrue("Initialize()", "simulation info", "Initialize succeeds", M.Initialize()) && Passed;
  MReadOutAssembly* E = CreateEvent(1, 1.0);
  M.AnalyzeEvent(E);
  delete E;
  M.Finalize();

  MString File = Dir + "/sim.tra";
  Passed = Evaluate("SetStartAreaFarField()", "3.5", "The header contains the start area", CountLines(File, "StartAreaFarField 3.5"), 1u) && Passed;
  Passed = Evaluate("SetStartAreaFarField()", "3.5", "The header contains the TB keyword", CountLines(File, "TB 0"), 1u) && Passed;
  Passed = Evaluate("SetSimulatedEvents()", "12345", "The trailer contains the number of simulated events", CountLines(File, "TS 12345"), 1u) && Passed;
  Passed = Evaluate("SetSimulatedEvents()", "12345", "The file ends with the TS line", LastLine(File), MString("TS 12345")) && Passed;

  RemoveTemporaryDirectory("sim");

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleEventSaver Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleEventSaver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
