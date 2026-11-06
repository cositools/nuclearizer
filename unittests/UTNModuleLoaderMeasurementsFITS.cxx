/*
 * UTNModuleLoaderMeasurementsFITS.cxx
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
#include "MTime.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleLoaderMeasurementsFITS.h"
#include "MModuleSaverMeasurementsFITS.h"
#include "MFITSWriterL1a.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MHit.h"


//! Unit test class for MModuleLoaderMeasurementsFITS
class UTNModuleLoaderMeasurementsFITS : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleLoaderMeasurementsFITS() : MUnitTest("UTNModuleLoaderMeasurementsFITS") {}
  //! Default destructor
  virtual ~UTNModuleLoaderMeasurementsFITS() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data
  bool TestDefaultConstruction();
  //! Test the XML configuration round trip
  bool TestXmlRoundTrip();
  //! Test the Initialize() error paths
  bool TestInitializeErrors();
  //! Test reading an L1a file
  bool TestRead();
  //! Test reading more events than one batch
  bool TestBatches();
  //! Test re-initialization
  bool TestReinitialize();
  //! Test an event with more strip hits than fit into 8 bits
  bool TestManyStripHits();

  //! Write an L1a file with N events, each with (1 + i%3) strip hits; event 2 has a guard ring hit
  MString WriteL1a(const MString& Name, unsigned int N);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsFITS::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestInitializeErrors() && Passed;
  Passed = TestRead() && Passed;
  Passed = TestBatches() && Passed;
  Passed = TestReinitialize() && Passed;
  Passed = TestManyStripHits() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleLoaderMeasurementsFITS::WriteL1a(const MString& Name, unsigned int N)
{
  MString FileName = GetTemporaryFileName(Name);
  MFITSWriterL1a W;
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  if (W.Create(FileName) == true) {
    for (unsigned int e = 1; e <= N; ++e) {
      MReadOutAssembly E;
      E.SetID(e);
      E.SetTimeRTS(MTime(100.0 + 0.5*e));
      for (unsigned int h = 0; h < 1 + e % 3; ++h) {
        MStripHit* SH = new MStripHit();
        SH->SetDetectorID(e % 16);
        SH->SetStripID(10 + h);
        SH->IsLowVoltageStrip(h % 2 == 0);
        SH->HasFastTiming(h == 0);
        SH->SetADCUnits(1000*e % 16000 + h);
        SH->SetTAC(2000 + h);
        if (e == 2 && h == 1) {
          SH->IsGuardRing(true);
          SH->SetStripID(64);
        }
        E.AddStripHit(SH);
      }
      W.Write(&E);
    }
    W.Close();
  }
  g_Verbosity = OldVerbosity;
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsFITS::TestManyStripHits()
{
  bool Passed = true;

  // NUMSTRIPHIT is a 16-bit column and the hit arrays hold up to 2080 entries: 300 strip hits must survive
  MString FileName = GetTemporaryFileName("many.fits");
  MFITSWriterL1a W;
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Created = W.Create(FileName);
  if (Created == true) {
    MReadOutAssembly Big;
    Big.SetID(1);
    Big.SetTimeRTS(MTime(100.0));
    for (unsigned int h = 0; h < 300; ++h) {
      MStripHit* SH = new MStripHit();
      SH->SetDetectorID(h/64);
      SH->SetStripID(h % 64);
      SH->IsLowVoltageStrip(true);
      SH->SetADCUnits(1000 + h);
      Big.AddStripHit(SH);
    }
    W.Write(&Big);
    MReadOutAssembly Small;
    Small.SetID(2);
    Small.SetTimeRTS(MTime(101.0));
    for (unsigned int h = 0; h < 2; ++h) {
      MStripHit* SH = new MStripHit();
      SH->SetDetectorID(7);
      SH->SetStripID(20 + h);
      SH->IsLowVoltageStrip(h == 0);
      SH->SetADCUnits(500);
      Small.AddStripHit(SH);
    }
    W.Write(&Small);
    W.Close();
  }
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("MFITSWriterL1a::Create()", "300 strip hits", "The file can be created", Created) && Passed;
  if (Created == false) return false;

  MModuleLoaderMeasurementsFITS L;
  L.SetFileName(FileName);
  g_Verbosity = c_Quiet;
  bool Init = L.Initialize();
  MReadOutAssembly E1;
  MReadOutAssembly E2;
  bool Read1 = Init == true && L.AnalyzeEvent(&E1) == true;
  bool Read2 = Init == true && L.AnalyzeEvent(&E2) == true;
  if (Init == true) L.Finalize();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("Initialize()", "300 strip hits", "Initialize succeeds", Init) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "event with 300 strip hits", "The event is read", Read1) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event with 300 strip hits", "All 300 strip hits are read", E1.GetNStripHits(), 300u) && Passed;
  if (E1.GetNStripHits() == 300) {
    Passed = Evaluate("AnalyzeEvent()", "event with 300 strip hits, hit 300", "The last hit is in detector 4", E1.GetStripHit(299)->GetDetectorID(), 4u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event with 300 strip hits, hit 300", "The last hit is on strip 43", E1.GetStripHit(299)->GetStripID(), 43u) && Passed;
  }
  Passed = EvaluateTrue("AnalyzeEvent()", "event after the 300-hit event", "The following event is read", Read2) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event after the 300-hit event", "The following event has its two strip hits", E2.GetNStripHits(), 2u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsFITS::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleLoaderMeasurementsFITS M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Measurement loader for FITS files'", M.GetName(), MString("Measurement loader for FITS files")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagMeasurementLoaderFITS'", M.GetXmlTag(), MString("XmlTagMeasurementLoaderFITS")) && Passed;
  Passed = EvaluateTrue("IsStartModule()", "default", "The loader is a start module", M.IsStartModule()) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The loader provides c_EventLoaderMeasurement", M.ProvidesModuleType(MAssembly::c_EventLoaderMeasurement)) && Passed;

  MModuleLoaderMeasurementsFITS* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a non-null object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsFITS::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleLoaderMeasurementsFITS Writer;
  Writer.SetFileName("/data/in.fits");
  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleLoaderMeasurementsFITS Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "file name", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "file name", "The file name survives", Reader.GetFileName(), MString("/data/in.fits")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsFITS::TestInitializeErrors()
{
  bool Passed = true;

  // Text file instead of FITS
  MString Text = GetTemporaryFileName("text.fits");
  WriteTextFile(Text, "This is not a FITS file\n");

  // A L1b file has the wrong extension name
  MString L1b = GetTemporaryFileName("l1b.fits");
  {
    MModuleSaverMeasurementsFITS S;
    S.SetFileName(L1b);
    int OldVerbosity = g_Verbosity;
    g_Verbosity = c_Quiet;
    if (S.Initialize() == true) S.Finalize();
    g_Verbosity = OldVerbosity;
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  MModuleLoaderMeasurementsFITS Missing;
  Missing.SetFileName(GetTemporaryFileName("missing.fits"));
  bool MissingResult = Missing.Initialize();
  MModuleLoaderMeasurementsFITS NotFITS;
  NotFITS.SetFileName(Text);
  bool NotFITSResult = NotFITS.Initialize();
  NotFITS.Finalize();
  MModuleLoaderMeasurementsFITS WrongLevel;
  WrongLevel.SetFileName(L1b);
  bool WrongLevelResult = WrongLevel.Initialize();
  WrongLevel.Finalize();
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "missing file", "Initialize returns false", MissingResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "text file", "Initialize returns false for a non-FITS file", NotFITSResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "L1b file", "Initialize returns false for a file without GED_L1A extension", WrongLevelResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsFITS::TestRead()
{
  bool Passed = true;

  MModuleLoaderMeasurementsFITS L;
  L.SetFileName(WriteL1a("read.fits", 3));
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Init = L.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("Initialize()", "three events", "Initialize succeeds", Init) && Passed;
  if (Init == false) return false;

  // Event 1: 2 strip hits; event 2: 3 strip hits with a guard ring hit; event 3: 1 strip hit
  MReadOutAssembly E1;
  Passed = EvaluateTrue("AnalyzeEvent()", "event 1", "AnalyzeEvent returns true", L.AnalyzeEvent(&E1)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event 1", "The event ID is 1", E1.GetID(), (unsigned long) 1) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "event 1", "The RTS time is 100.5 s", E1.GetTimeRTS().GetAsDouble(), 100.5, 1e-6) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "event 1", "Two strip hits are read", E1.GetNStripHits(), 2u) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "event 1", "The event loader analysis progress is set", E1.HasAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement)) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "event 1", "No guard ring veto", E1.GetGuardRingVeto()) && Passed;
  if (E1.GetNStripHits() == 2) {
    MStripHit* S0 = E1.GetStripHit(0);
    MStripHit* S1 = E1.GetStripHit(1);
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 0", "The detector ID is 1", S0->GetDetectorID(), 1u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 0", "The strip ID is 10", S0->GetStripID(), 10u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 1 hit 0", "The side is LV", S0->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 1 hit 0", "Fast timing is set", S0->HasFastTiming()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 0", "The ADC value is 1000", S0->GetADCUnits(), 1000.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 0", "The TAC value is 2000", S0->GetTAC(), 2000.0, 0.5) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "event 1 hit 1", "The side is HV", S1->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "event 1 hit 1", "Fast timing is not set", S1->HasFastTiming()) && Passed;
  }

  MReadOutAssembly E2;
  L.AnalyzeEvent(&E2);
  Passed = Evaluate("AnalyzeEvent()", "event 2", "Three strip hits are read", E2.GetNStripHits(), 3u) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "event 2", "The guard ring hit sets the guard ring veto", E2.GetGuardRingVeto()) && Passed;
  if (E2.GetNStripHits() == 3) {
    Passed = EvaluateTrue("AnalyzeEvent()", "event 2 hit 1", "The guard ring hit is flagged", E2.GetStripHit(1)->IsGuardRing()) && Passed;
  }

  MReadOutAssembly E3;
  L.AnalyzeEvent(&E3);
  Passed = Evaluate("AnalyzeEvent()", "event 3", "One strip hit is read", E3.GetNStripHits(), 1u) && Passed;

  MReadOutAssembly E4;
  g_Verbosity = c_Quiet;
  bool Fourth = L.AnalyzeEvent(&E4);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("AnalyzeEvent()", "end of file", "AnalyzeEvent returns false at the end of the file", Fourth) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "end of file", "The loader is finished", L.IsFinished()) && Passed;
  L.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsFITS::TestBatches()
{
  bool Passed = true;

  MModuleLoaderMeasurementsFITS L;
  L.SetFileName(WriteL1a("batches.fits", 205));
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Init = L.Initialize();
  unsigned int N = 0;
  bool IDsInOrder = true;
  bool HitsCorrect = true;
  while (Init == true && N < 1000) {
    MReadOutAssembly E;
    if (L.AnalyzeEvent(&E) == false) break;
    ++N;
    if (E.GetID() != N) IDsInOrder = false;
    if (E.GetNStripHits() != 1 + N % 3) HitsCorrect = false;
  }
  L.Finalize();
  g_Verbosity = OldVerbosity;

  Passed = Evaluate("AnalyzeEvent()", "205 events", "All events are read across batch boundaries", N, 205u) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "205 events", "The event IDs are in order", IDsInOrder) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "205 events", "Every event has the right number of strip hits", HitsCorrect) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsFITS::TestReinitialize()
{
  bool Passed = true;

  MModuleLoaderMeasurementsFITS L;
  L.SetFileName(WriteL1a("reinit.fits", 4));
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  vector<unsigned int> Counts = { 0, 0 };
  for (unsigned int Round = 0; Round < Counts.size(); ++Round) {
    if (L.Initialize() == false) continue;
    for (unsigned int i = 0; i < 10; ++i) {
      MReadOutAssembly E;
      if (L.AnalyzeEvent(&E) == false) break;
      ++Counts[Round];
    }
    L.Finalize();
  }
  g_Verbosity = OldVerbosity;

  Passed = Evaluate("AnalyzeEvent()", "first pass", "Four events are read", Counts[0], 4u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "second pass after Finalize", "Four events are read again after re-initialization", Counts[1], 4u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleLoaderMeasurementsFITS Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleLoaderMeasurementsFITS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
