/*
 * UTNModuleLoaderMeasurementsROA.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MFretalonRegistry.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleLoaderMeasurementsROA.h"
#include "MModuleEventSaver.h"
#include "MReadOutAssembly.h"
#include "MReadOutDataTAC.h"
#include "MReadOutDataEnergy.h"
#include "MReadOutDataFlags.h"
#include "MReadOutElementVoxel3D.h"
#include "MStripHit.h"
#include "MCrystalHit.h"


//! Unit test class for MModuleLoaderMeasurementsROA
class UTNModuleLoaderMeasurementsROA : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleLoaderMeasurementsROA() : MUnitTest("UTNModuleLoaderMeasurementsROA") {}
  //! Default destructor
  virtual ~UTNModuleLoaderMeasurementsROA() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data
  bool TestDefaultConstruction();
  //! Test the XML configuration round trip
  bool TestXmlRoundTrip();
  //! Test the Initialize() error paths
  bool TestInitializeErrors();
  //! Test Open() directly
  bool TestOpen();
  //! Test reading a hand-written roa file
  bool TestReadHandWritten();
  //! Test a write/read round trip with the event saver
  bool TestRoundTrip();
  //! Test re-initialization after reading the whole file
  bool TestReinitialize();

  //! Register the nuclearizer read-out types the same way MAssembly does
  void RegisterReadOutTypes();
  //! Write a hand-written roa file and return its name
  MString WriteHandWritten();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsROA::Run()
{
  bool Passed = true;

  RegisterReadOutTypes();

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestInitializeErrors() && Passed;
  Passed = TestOpen() && Passed;
  Passed = TestReadHandWritten() && Passed;
  Passed = TestRoundTrip() && Passed;
  Passed = TestReinitialize() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


void UTNModuleLoaderMeasurementsROA::RegisterReadOutTypes()
{
  MFretalonRegistry::Instance().Register(MReadOutDataTAC());
  MFretalonRegistry::Instance().Register(MReadOutDataEnergy());
  MFretalonRegistry::Instance().Register(MReadOutDataFlags());
  MFretalonRegistry::Instance().Register(MReadOutElementVoxel3D());
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleLoaderMeasurementsROA::WriteHandWritten()
{
  MString C;
  C += "\nTYPE ROA\n";
  C += "UF UH doublesidedstrip adc-tac\n";
  C += "UF UC voxel3d adc-energy\n\n";
  C += "SE\nID 11\nTI 100.5\nUH 0 41 l 4053 10452\nUH 0 12 h 3900 10400\nUC ACS_X0_0 3 4 5 6 2000 150.5\n";
  C += "SE\nID 12\nTI 101.25\nUH 1 5 h 1000 9000\n";
  C += "EN\n";

  MString FileName = GetTemporaryFileName("handwritten.roa");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsROA::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleLoaderMeasurementsROA M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Measurement loader for ROA files'", M.GetName(), MString("Measurement loader for ROA files")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagMeasurementLoaderROA'", M.GetXmlTag(), MString("XmlTagMeasurementLoaderROA")) && Passed;
  Passed = EvaluateTrue("IsStartModule()", "default", "The loader is a start module", M.IsStartModule()) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The loader provides c_EventLoader", M.ProvidesModuleType(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The loader provides c_EventLoaderMeasurement", M.ProvidesModuleType(MAssembly::c_EventLoaderMeasurement)) && Passed;
  Passed = EvaluateFalse("AllowsMultipleInstances()", "default", "The loader does not allow multiple instances", M.AllowsMultipleInstances()) && Passed;

  MModuleLoaderMeasurementsROA* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a non-null object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsROA::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleLoaderMeasurementsROA Writer;
  Writer.SetFileName("/data/in.roa");
  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleLoaderMeasurementsROA Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "file name", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "file name", "The file name survives", Reader.GetFileName(), MString("/data/in.roa")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsROA::TestInitializeErrors()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  MModuleLoaderMeasurementsROA M;
  M.SetFileName(GetTemporaryFileName("missing.roa"));
  bool Result = M.Initialize();
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("Initialize()", "missing file", "Initialize returns false for a missing file", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsROA::TestOpen()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MModuleLoaderMeasurementsROA Existing;
  bool ExistingResult = Existing.Open(WriteHandWritten(), MFile::c_Read);
  MModuleLoaderMeasurementsROA Missing;
  bool MissingResult = Missing.Open(GetTemporaryFileName("missing.roa"), MFile::c_Read);
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("Open()", "hand-written file", "Open succeeds for an existing roa file", ExistingResult) && Passed;
  Passed = EvaluateFalse("Open()", "missing file", "Open returns false for a missing file", MissingResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsROA::TestReadHandWritten()
{
  bool Passed = true;

  MModuleLoaderMeasurementsROA M;
  M.SetFileName(WriteHandWritten());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Init = M.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("Initialize()", "hand-written file", "Initialize succeeds", Init) && Passed;
  if (Init == false) return false;

  MReadOutAssembly* E1 = new MReadOutAssembly();
  Passed = EvaluateTrue("AnalyzeEvent()", "event 1", "AnalyzeEvent returns true", M.AnalyzeEvent(E1)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event 1", "The event ID is 11", E1->GetID(), (unsigned long) 11) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "event 1", "The event time is 100.5 s", E1->GetTime().GetAsDouble(), 100.5, 1e-9) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "event 1", "The UTC time is set to the event time", E1->GetTimeUTC().GetAsDouble(), 100.5, 1e-9) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "event 1", "The event loader analysis progress is set", E1->HasAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event 1", "Two strip hits are read", E1->GetNStripHits(), 2u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event 1", "One crystal hit is read", E1->GetNCrystalHits(), 1u) && Passed;
  if (E1->GetNStripHits() == 2) {
    MStripHit* S0 = E1->GetStripHit(0);
    Passed = Evaluate("AnalyzeEvent()", "event 1 strip hit 0", "The detector ID is 0", S0->GetDetectorID(), 0u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1 strip hit 0", "The strip ID is 41", S0->GetStripID(), 41u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 1 strip hit 0", "The strip is on the LV side", S0->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 strip hit 0", "The ADC value is 4053", S0->GetADCUnits(), 4053.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 strip hit 0", "The TAC value is 10452", S0->GetTAC(), 10452.0, 0.5) && Passed;
    MStripHit* S1 = E1->GetStripHit(1);
    Passed = EvaluateFalse("AnalyzeEvent()", "event 1 strip hit 1", "The strip is on the HV side", S1->IsLowVoltageStrip()) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1 strip hit 1", "The strip ID is 12", S1->GetStripID(), 12u) && Passed;
  }
  if (E1->GetNCrystalHits() == 1) {
    MCrystalHit* CH = E1->GetCrystalHit(0);
    Passed = Evaluate("AnalyzeEvent()", "event 1 crystal hit", "The detector ID is ACS_X0_0", CH->GetDetectorID(), MString("ACS_X0_0")) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1 crystal hit", "The crystal ID is 3", CH->GetCrystalID(), 3u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1 crystal hit", "The voxel Z ID is 6", CH->GetReadOutElement()->GetVoxelZID(), 6u) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 crystal hit", "The ADC value is 2000", CH->GetADCUnits(), 2000.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 crystal hit", "The energy is 150.5 keV", CH->GetEnergy(), 150.5, 1e-4) && Passed;
  }
  delete E1;

  MReadOutAssembly* E2 = new MReadOutAssembly();
  Passed = EvaluateTrue("AnalyzeEvent()", "event 2", "AnalyzeEvent returns true", M.AnalyzeEvent(E2)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event 2", "The event ID is 12", E2->GetID(), (unsigned long) 12) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event 2", "One strip hit is read", E2->GetNStripHits(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event 2", "No crystal hit is read", E2->GetNCrystalHits(), 0u) && Passed;
  if (E2->GetNStripHits() == 1) {
    Passed = Evaluate("AnalyzeEvent()", "event 2", "The detector ID is 1", E2->GetStripHit(0)->GetDetectorID(), 1u) && Passed;
  }
  delete E2;

  MReadOutAssembly* E3 = new MReadOutAssembly();
  g_Verbosity = c_Quiet;
  bool Third = M.AnalyzeEvent(E3);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("AnalyzeEvent()", "end of file", "AnalyzeEvent returns false at the end of the file", Third) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "end of file", "The loader is finished", M.IsFinished()) && Passed;
  delete E3;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsROA::TestRoundTrip()
{
  bool Passed = true;

  MString Base = GetTemporaryFileName("roundtrip");

  // Write
  MModuleEventSaver Saver;
  Saver.SetMode(MModuleEventSaver::c_RoaFile);
  Saver.SetSplitFile(false);
  Saver.SetRoaWithADCs(true);
  Saver.SetRoaWithTACs(true);
  Saver.SetRoaWithEnergies(true);
  Saver.SetRoaWithTimings(true);
  Saver.SetRoaWithFlags(true);
  Saver.SetRoaWithOrigins(true);
  Saver.SetFileName(Base + ".roa");
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool SaverInit = Saver.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("MModuleEventSaver::Initialize()", "round trip", "The saver initializes", SaverInit) && Passed;
  if (SaverInit == false) return false;

  MReadOutAssembly* W = new MReadOutAssembly();
  W->SetID(77);
  W->SetTime(MTime(1234.5));
  W->SetTimeUTC(MTime(1234.5));
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(5);
  SH->SetStripID(33);
  SH->IsLowVoltageStrip(false);
  SH->SetADCUnits(4053);
  SH->SetTAC(10452);
  SH->SetEnergy(662.5);
  SH->SetTiming(4321.25);
  SH->HasFastTiming(true);
  SH->AddOrigins({ 3, 8 });
  W->AddStripHit(SH);
  MCrystalHit* CH = new MCrystalHit();
  CH->SetDetectorID("ACS_Y1_2");
  CH->SetCrystalID(7);
  CH->SetVoxelXID(1);
  CH->SetVoxelYID(2);
  CH->SetVoxelZID(3);
  CH->SetADCUnits(3000);
  CH->SetEnergy(250.5);
  CH->AddOrigins({ 4 });
  W->AddCrystalHit(CH);
  Saver.AnalyzeEvent(W);
  Saver.Finalize();
  delete W;

  // Read
  MModuleLoaderMeasurementsROA M;
  M.SetFileName(Base + ".roa");
  g_Verbosity = c_Quiet;
  bool Init = M.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("Initialize()", "round trip", "The loader initializes", Init) && Passed;
  if (Init == false) return false;

  MReadOutAssembly* R = new MReadOutAssembly();
  Passed = EvaluateTrue("AnalyzeEvent()", "round trip", "The event is read", M.AnalyzeEvent(R)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "round trip", "The event ID survives", R->GetID(), (unsigned long) 77) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "round trip", "The event time survives", R->GetTime().GetAsDouble(), 1234.5, 1e-6) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "round trip", "One strip hit survives", R->GetNStripHits(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "round trip", "One crystal hit survives", R->GetNCrystalHits(), 1u) && Passed;
  if (R->GetNStripHits() == 1) {
    MStripHit* S = R->GetStripHit(0);
    Passed = Evaluate("AnalyzeEvent()", "round trip strip hit", "The detector ID survives", S->GetDetectorID(), 5u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "round trip strip hit", "The strip ID survives", S->GetStripID(), 33u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "round trip strip hit", "The HV side survives", S->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "round trip strip hit", "The ADC value survives", S->GetADCUnits(), 4053.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "round trip strip hit", "The TAC value survives", S->GetTAC(), 10452.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "round trip strip hit", "The energy survives", S->GetEnergy(), 662.5, 1e-4) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "round trip strip hit", "The timing survives", S->GetTiming(), 4321.25, 1e-2) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "round trip strip hit", "The fast timing flag survives", S->HasFastTiming()) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "round trip strip hit", "The origins survive", (unsigned int) S->GetOrigins().size(), 2u) && Passed;
  }
  if (R->GetNCrystalHits() == 1) {
    MCrystalHit* C = R->GetCrystalHit(0);
    Passed = Evaluate("AnalyzeEvent()", "round trip crystal hit", "The detector ID survives", C->GetDetectorID(), MString("ACS_Y1_2")) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "round trip crystal hit", "The crystal ID survives", C->GetCrystalID(), 7u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "round trip crystal hit", "The voxel Y ID survives", C->GetReadOutElement()->GetVoxelYID(), 2u) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "round trip crystal hit", "The ADC value survives", C->GetADCUnits(), 3000.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "round trip crystal hit", "The energy survives", C->GetEnergy(), 250.5, 1e-4) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "round trip crystal hit", "The origins survive", (unsigned int) C->GetOrigins().size(), 1u) && Passed;
  }
  delete R;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsROA::TestReinitialize()
{
  bool Passed = true;

  MModuleLoaderMeasurementsROA M;
  M.SetFileName(WriteHandWritten());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  vector<unsigned int> Counts = { 0, 0 };
  for (unsigned int Round = 0; Round < Counts.size(); ++Round) {
    if (M.Initialize() == false) break;
    while (true) {
      MReadOutAssembly E;
      if (M.AnalyzeEvent(&E) == false) break;
      ++Counts[Round];
      if (Counts[Round] > 10) break;
    }
    M.Finalize();
  }
  g_Verbosity = OldVerbosity;

  Passed = Evaluate("Initialize()", "first pass", "Two events are read in the first pass", Counts[0], 2u) && Passed;
  Passed = Evaluate("Initialize()", "second pass after Finalize", "Two events are read again after re-initialization", Counts[1], 2u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleLoaderMeasurementsROA Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleLoaderMeasurementsROA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
