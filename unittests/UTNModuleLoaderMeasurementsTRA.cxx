/*
 * UTNModuleLoaderMeasurementsTRA.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
#include <fstream>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MComptonEvent.h"
#include "MFile.h"
#include "MPhysicalEvent.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MHit.h"
#include "MModuleLoaderMeasurementsTRA.h"
#include "MModuleSaverMeasurementsFITS.h"
#include "MReadOutAssembly.h"


//! Unit test class for MModuleLoaderMeasurementsTRA
class UTNModuleLoaderMeasurementsTRA : public MUnitTest
{
public:
  UTNModuleLoaderMeasurementsTRA() : MUnitTest("UTNModuleLoaderMeasurementsTRA") {}
  virtual ~UTNModuleLoaderMeasurementsTRA() {}

  virtual bool Run();

private:
  //! Test getter and setter methods, XML tag, clone, and module types
  bool TestGettersSetters();
  //! Test Initialize() fails when the tra file does not exist
  bool TestInitializeMissingFile();
  //! Test XML configuration round-trip
  bool TestXmlRoundTrip();
  //! Test event boundaries, IDs, times, and physical events of the fixture
  bool TestEvents();
  //! Test hits created from Compton and photo events
  bool TestHits();
  //! Test the error and veto flags derived from the bad strings
  bool TestFlags();
  //! Test that a re-initialized loader starts again at the first event
  bool TestReinitialize();
  //! Test the saver with the strictest requirements accepts the loaded events
  bool TestSaverRequirements();
  //! Test the committed 542-1 tra reference file
  bool TestCommittedData();

  //! Write the fixture tra file and return its name, or "" on error
  MString WriteFixture();
  //! Create a loader for this file, initialize it, and return it, or nullptr on error
  MModuleLoaderMeasurementsTRA* CreateLoader(const MString& FileName);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::Run()
{
  bool Passed = true;

  Passed = TestGettersSetters() && Passed;
  Passed = TestInitializeMissingFile() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestEvents() && Passed;
  Passed = TestHits() && Passed;
  Passed = TestFlags() && Passed;
  Passed = TestReinitialize() && Passed;
  Passed = TestSaverRequirements() && Passed;
  Passed = TestCommittedData() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI; it cannot be exercised in a
  // headless unit test and is intentionally left uncovered.

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleLoaderMeasurementsTRA::WriteFixture()
{
  // Events in file order:
  //   ID 1: photo event
  //   ID 2: unidentifiable event with a depth calibration error
  //   ID 3: no ET line -- MFileEventsTra skips it
  //   ID 4: Compton event with its hit sequence
  //   ID 5: unidentifiable event with a guard ring veto
  //   ID 6: unidentifiable event with a revan bad string, which is no nuclearizer flag
  //   ID 7: unidentifiable event with a strip pairing error with two reasons
  //   ID 8-11: unidentifiable events with energy calibration, TAC calibration, event reconstruction error, shield veto
  //   ID 12: Compton event without hit sequence
  //   ID 13: unidentifiable event with a flag repeated for several strips
  //   ID 14: unidentifiable event whose reason text contains parentheses
  //   ID 15: unidentifiable event with several BD lines, each one carrying its own flag

  MString FileName = GetTemporaryFileName("Fixture.tra");
  if (FileName == "") return "";

  ofstream Out(FileName.Data());
  if (Out.is_open() == false) return "";

  Out<<endl;
  Out<<"Version 1"<<endl;
  Out<<"TYPE TRA"<<endl;
  Out<<endl;
  Out<<"SE"<<endl;
  Out<<"ET PH"<<endl;
  Out<<"ID 1"<<endl;
  Out<<"TI 1760667399.582988500"<<endl;
  Out<<"CC NStripHits 4"<<endl;
  Out<<"PE 460.202"<<endl;
  Out<<"PP 1.79236 -0.517545 -3.14452"<<endl;
  Out<<"PW 0"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 2"<<endl;
  Out<<"TI 1760667399.583013296"<<endl;
  Out<<"BD DepthCalibrationError (Out of Range)"<<endl;
  Out<<"SE"<<endl;
  Out<<"ID 3"<<endl;
  Out<<"BD StripPairingError (One detector side has no strip hits)"<<endl;
  Out<<"PQ"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET CO"<<endl;
  Out<<"ID 4"<<endl;
  Out<<"TI 1760667399.583075284"<<endl;
  Out<<"PQ 25.7"<<endl;
  Out<<"SQ 2"<<endl;
  Out<<"CT 0 1"<<endl;
  Out<<"TL 1"<<endl;
  Out<<"TE 55.6932"<<endl;
  Out<<"CE 136.669 1.37268   55.6932 1.34271"<<endl;
  Out<<"CD 2.84002 -0.356284 -0.350766   0.0336036 0.0432359 0.0336036   1.79236 -0.615525 0.464078   0.0336036 0.0158803 0.0336036   0 0 0   0 0 0"<<endl;
  Out<<"LA 1.35232"<<endl;
  Out<<"CH 0 2.84002 -0.356284 -0.350766 55.6932 0 0.0336036 0.0432359 0.0336036 1.34271 0"<<endl;
  Out<<"CH 1 1.79236 -0.615525 0.464078 136.669 0 0.0336036 0.0158803 0.0336036 1.37268 0"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 5"<<endl;
  Out<<"TI 1760667399.583100000"<<endl;
  Out<<"BD GR Veto"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 6"<<endl;
  Out<<"TI 1760667399.583200000"<<endl;
  Out<<"BD ComptelTypeWithIncompatibleKinematics"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 7"<<endl;
  Out<<"TI 1760667399.583300000"<<endl;
  Out<<"BD StripPairingError (First reason) (Second reason)"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 8"<<endl;
  Out<<"TI 1760667399.583400000"<<endl;
  Out<<"BD EnergyCalibrationError"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 9"<<endl;
  Out<<"TI 1760667399.583500000"<<endl;
  Out<<"BD TACCalibrationError (No calibration)"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 10"<<endl;
  Out<<"TI 1760667399.583600000"<<endl;
  Out<<"BD EventReconstructionError"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 11"<<endl;
  Out<<"TI 1760667399.583700000"<<endl;
  Out<<"BD Shield Veto"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET CO"<<endl;
  Out<<"ID 12"<<endl;
  Out<<"TI 1760667399.583800000"<<endl;
  Out<<"SQ 2"<<endl;
  Out<<"CT 0 1"<<endl;
  Out<<"TL 1"<<endl;
  Out<<"TE 55.6932"<<endl;
  Out<<"CE 136.669 1.37268   55.6932 1.34271"<<endl;
  Out<<"CD 2.84002 -0.356284 -0.350766   0.0336036 0.0432359 0.0336036   1.79236 -0.615525 0.464078   0.0336036 0.0158803 0.0336036   0 0 0   0 0 0"<<endl;
  Out<<"LA 1.35232"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 13"<<endl;
  Out<<"TI 1760667399.583900000"<<endl;
  Out<<"BD DepthCalibrationError (Multiple hits on single strip) (Multiple hits on single strip)"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 14"<<endl;
  Out<<"TI 1760667399.584000000"<<endl;
  Out<<"BD StripPairingError (More than maximum number of strip hits allowed on one side (7))"<<endl;
  Out<<"SE"<<endl;
  Out<<"ET UN"<<endl;
  Out<<"ID 15"<<endl;
  Out<<"TI 1760667399.584100000"<<endl;
  Out<<"BD StripPairingError (No strip hits)"<<endl;
  Out<<"BD DepthCalibrationError (GR Veto)"<<endl;
  Out<<"BD GR Veto"<<endl;
  Out<<"EN"<<endl;
  Out.close();

  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsTRA* UTNModuleLoaderMeasurementsTRA::CreateLoader(const MString& FileName)
{
  MModuleLoaderMeasurementsTRA* Loader = new MModuleLoaderMeasurementsTRA();
  Loader->SetFileName(FileName);
  if (Loader->Initialize() == false) {
    delete Loader;
    return nullptr;
  }

  return Loader;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::TestGettersSetters()
{
  bool Passed = true;

  MModuleLoaderMeasurementsTRA Loader;

  Loader.SetFileName("mydata.tra");
  Passed = Evaluate("SetFileName/GetFileName", "representative file name", "GetFileName returns the value passed to SetFileName", Loader.GetFileName(), MString("mydata.tra")) && Passed;

  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is XmlTagMeasurementLoaderTRA", Loader.GetXmlTag(), MString("XmlTagMeasurementLoaderTRA")) && Passed;

  MModuleLoaderMeasurementsTRA* Clone = Loader.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone returns a new object", Clone != nullptr && Clone != &Loader) && Passed;
  delete Clone;

  // The tra file contains strip-paired, depth-calibrated, and reconstructed events
  uint64_t Types = 0;
  for (unsigned int t = 0; t < Loader.GetNModuleTypes(); ++t) {
    Types |= Loader.GetModuleType(t);
  }
  Passed = EvaluateTrue("GetModuleType()", "loader types", "The module is an event loader", (Types & MAssembly::c_EventLoader) != 0) && Passed;
  Passed = EvaluateTrue("GetModuleType()", "loader types", "The module is a loader of measurements", (Types & MAssembly::c_EventLoaderMeasurement) != 0) && Passed;
  Passed = EvaluateTrue("GetModuleType()", "energy calibration", "The module provides energy calibration", (Types & MAssembly::c_EnergyCalibration) != 0) && Passed;
  Passed = EvaluateTrue("GetModuleType()", "TAC calibration", "The module provides TAC calibration", (Types & MAssembly::c_TACCalibration) != 0) && Passed;
  Passed = EvaluateTrue("GetModuleType()", "strip pairing", "The module provides strip pairing", (Types & MAssembly::c_StripPairing) != 0) && Passed;
  Passed = EvaluateTrue("GetModuleType()", "depth correction", "The module provides depth correction", (Types & MAssembly::c_DepthCorrection) != 0) && Passed;
  Passed = EvaluateTrue("GetModuleType()", "position determination", "The module provides position determination", (Types & MAssembly::c_PositionDetermiation) != 0) && Passed;
  Passed = EvaluateTrue("GetModuleType()", "event reconstruction", "The module provides event reconstruction", (Types & MAssembly::c_EventReconstruction) != 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::TestInitializeMissingFile()
{
  bool Passed = true;

  MModuleLoaderMeasurementsTRA Loader;
  Loader.SetFileName(GetTemporaryFileName("DoesNotExist.tra"));

  // MFile reports the missing file through the MEGAlib streams, the loader through a guarded cout
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool Result = Loader.Initialize();
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "missing tra file", "Initialize() returns false when the tra file does not exist", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleLoaderMeasurementsTRA Writer;
  Writer.SetFileName("/some/path/events.tra");
  MXmlNode* Node = Writer.CreateXmlConfiguration();

  Passed = Evaluate("CreateXmlConfiguration()", "node name", "The XML node carries the module XML tag", Node->GetName(), MString("XmlTagMeasurementLoaderTRA")) && Passed;

  MModuleLoaderMeasurementsTRA Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "node from CreateXmlConfiguration", "ReadXmlConfiguration() returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "FileName", "ReadXmlConfiguration() restores the file name", Reader.GetFileName(), MString("/some/path/events.tra")) && Passed;

  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::TestEvents()
{
  bool Passed = true;

  MString FileName = WriteFixture();
  Passed = EvaluateTrue("AnalyzeEvent()", "write fixture", "The fixture tra file can be created", FileName != "") && Passed;
  if (Passed == false) return Passed;

  MModuleLoaderMeasurementsTRA* Loader = CreateLoader(FileName);
  Passed = EvaluateTrue("Initialize()", "fixture tra file", "Initialize() succeeds for an existing tra file", Loader != nullptr) && Passed;
  if (Passed == false) return Passed;

  // Event without ET line (ID 3) is skipped by MFileEventsTra
  const vector<unsigned long> IDs = { 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
  const vector<int> Types = { MPhysicalEvent::c_Photo, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Compton, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Compton, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Unidentifiable, MPhysicalEvent::c_Unidentifiable };
  const uint64_t Progress = MAssembly::c_EventLoader | MAssembly::c_EnergyCalibration | MAssembly::c_TACCalibration | MAssembly::c_StripPairing | MAssembly::c_DepthCorrection | MAssembly::c_PositionDetermiation | MAssembly::c_EventReconstruction;

  // Use the same assembly for all events to verify nothing is left over from the previous one
  MReadOutAssembly Event;
  for (unsigned int e = 0; e < IDs.size(); ++e) {
    MString Input = MString("event ") + e;
    bool Result = Loader->AnalyzeEvent(&Event);
    Passed = EvaluateTrue("AnalyzeEvent()", Input, "AnalyzeEvent() returns true while events are left", Result) && Passed;
    if (Result == false) break;

    Passed = Evaluate("AnalyzeEvent()", Input, "The assembly has the ID of the event on file", Event.GetID(), IDs[e]) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", Input, "The assembly has a physical event", Event.GetPhysicalEvent() != nullptr) && Passed;
    if (Event.GetPhysicalEvent() != nullptr) {
      Passed = Evaluate("AnalyzeEvent()", Input, "The physical event has the type on file", Event.GetPhysicalEvent()->GetType(), Types[e]) && Passed;
      Passed = Evaluate("AnalyzeEvent()", Input, "The physical event has the ID on file", (unsigned long) Event.GetPhysicalEvent()->GetId(), IDs[e]) && Passed;
    }
    Passed = EvaluateTrue("AnalyzeEvent()", Input, "The progress flags of the whole analysis up to event reconstruction are set", Event.HasAnalysisProgress(Progress)) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", Input, "The saver progress flag is not set", Event.HasAnalysisProgress(MAssembly::c_EventSaver)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", Input, "The loader creates no strip hits", Event.GetNStripHits(), 0u) && Passed;

    if (e == 0) {
      Passed = Evaluate("AnalyzeEvent()", "event 0 time", "The UTC time is the TI value", Event.GetTimeUTC().GetLongIntsString(), MString("1760667399.582988500")) && Passed;
      Passed = Evaluate("AnalyzeEvent()", "event 0 time", "The sequence time is the TI value", Event.GetTime().GetLongIntsString(), MString("1760667399.582988500")) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", "event 0 time", "The RTS time is left for the savers to compute", Event.GetTimeRTS() == MTime(0)) && Passed;
    }
  }

  Passed = EvaluateFalse("IsFinished()", "before end of file", "The loader is not finished before reading past the last event", Loader->IsFinished()) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Result = Loader->AnalyzeEvent(&Event);
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("AnalyzeEvent()", "end of file", "AnalyzeEvent() returns false after the last event", Result) && Passed;
  Passed = EvaluateTrue("IsFinished()", "end of file", "The loader is finished after reading past the last event", Loader->IsFinished()) && Passed;

  g_Verbosity = c_Quiet;
  Loader->Finalize();
  g_Verbosity = OldVerbosity;

  delete Loader;
  RemoveTemporaryFile(FileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::TestHits()
{
  bool Passed = true;

  MString FileName = WriteFixture();
  MModuleLoaderMeasurementsTRA* Loader = CreateLoader(FileName);
  Passed = EvaluateTrue("Initialize()", "fixture tra file", "Initialize() succeeds for an existing tra file", Loader != nullptr) && Passed;
  if (Passed == false) return Passed;

  // ID 1: photo event -> one hit from PP and PE without uncertainties
  MReadOutAssembly* Event = new MReadOutAssembly();
  Loader->AnalyzeEvent(Event);
  Passed = Evaluate("AnalyzeEvent()", "photo event", "A photo event has one hit", Event->GetNHits(), 1u) && Passed;
  if (Event->GetNHits() == 1) {
    MHit* Hit = Event->GetHit(0);
    Passed = EvaluateNear("AnalyzeEvent()", "photo event", "The hit x position is PP x", Hit->GetPosition().X(), 1.79236, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "photo event", "The hit y position is PP y", Hit->GetPosition().Y(), -0.517545, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "photo event", "The hit z position is PP z", Hit->GetPosition().Z(), -3.14452, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "photo event", "The hit energy is PE", Hit->GetEnergy(), 460.202, 1e-9) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "photo event", "The hit position resolution is zero", Hit->GetPositionResolution() == MVector(0.0, 0.0, 0.0)) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "photo event", "The hit energy resolution is zero", Hit->GetEnergyResolution(), 0.0, 1e-9) && Passed;
  }
  delete Event;

  // ID 2: unidentifiable event -> no hits
  Event = new MReadOutAssembly();
  Loader->AnalyzeEvent(Event);
  Passed = Evaluate("AnalyzeEvent()", "unidentifiable event", "An unidentifiable event has no hits", Event->GetNHits(), 0u) && Passed;
  delete Event;

  // ID 4: Compton event -> one hit per CH line, in sequence order
  Event = new MReadOutAssembly();
  Loader->AnalyzeEvent(Event);
  Passed = Evaluate("AnalyzeEvent()", "Compton event", "A Compton event has one hit per CH line", Event->GetNHits(), 2u) && Passed;
  if (Event->GetNHits() == 2) {
    MHit* First = Event->GetHit(0);
    MHit* Second = Event->GetHit(1);
    Passed = EvaluateNear("AnalyzeEvent()", "Compton event", "The first hit x position is CH 0 x", First->GetPosition().X(), 2.84002, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "Compton event", "The first hit z position is CH 0 z", First->GetPosition().Z(), -0.350766, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "Compton event", "The first hit energy is CH 0 energy", First->GetEnergy(), 55.6932, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "Compton event", "The first hit y position resolution is CH 0 y uncertainty", First->GetPositionResolution().Y(), 0.0432359, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "Compton event", "The first hit energy resolution is CH 0 energy uncertainty", First->GetEnergyResolution(), 1.34271, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "Compton event", "The second hit y position is CH 1 y", Second->GetPosition().Y(), -0.615525, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "Compton event", "The second hit energy is CH 1 energy", Second->GetEnergy(), 136.669, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "Compton event", "The second hit energy resolution is CH 1 energy uncertainty", Second->GetEnergyResolution(), 1.37268, 1e-9) && Passed;
  }
  // The stored copy of the physical event keeps its hits, so the savers can use the hit sequence
  Passed = EvaluateTrue("AnalyzeEvent()", "Compton event", "The stored physical event has as many hits as the assembly", Event->GetPhysicalEvent() != nullptr && Event->GetPhysicalEvent()->GetNHits() == Event->GetNHits()) && Passed;
  delete Event;

  // Skip IDs 5 to 11
  for (unsigned int e = 0; e < 7; ++e) {
    Event = new MReadOutAssembly();
    Loader->AnalyzeEvent(Event);
    delete Event;
  }

  // ID 12: Compton event without CH lines (e.g. an older tra file) -> no hits
  Event = new MReadOutAssembly();
  Loader->AnalyzeEvent(Event);
  Passed = Evaluate("AnalyzeEvent()", "Compton event without CH lines", "The event has the ID 12", Event->GetID(), (unsigned long) 12) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "Compton event without CH lines", "A Compton event without hit sequence has no hits", Event->GetNHits(), 0u) && Passed;
  delete Event;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  Loader->Finalize();
  g_Verbosity = OldVerbosity;

  delete Loader;
  RemoveTemporaryFile(FileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::TestFlags()
{
  bool Passed = true;

  MString FileName = WriteFixture();
  MModuleLoaderMeasurementsTRA* Loader = CreateLoader(FileName);
  Passed = EvaluateTrue("Initialize()", "fixture tra file", "Initialize() succeeds for an existing tra file", Loader != nullptr) && Passed;
  if (Passed == false) return Passed;

  vector<MReadOutAssembly*> Events;
  for (unsigned int e = 0; e < 14; ++e) {
    MReadOutAssembly* Event = new MReadOutAssembly();
    Loader->AnalyzeEvent(Event);
    Events.push_back(Event);
  }

  // ID 1: photo event without bad string
  Passed = EvaluateTrue("AnalyzeEvent()", "photo event", "An event without bad string is good", Events[0]->IsGood()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "photo event", "An event without bad string has no veto", Events[0]->IsVeto()) && Passed;

  // ID 2: depth calibration error
  Passed = EvaluateTrue("AnalyzeEvent()", "BD DepthCalibrationError (Out of Range)", "The depth calibration error is set", Events[1]->HasDepthCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "BD DepthCalibrationError (Out of Range)", "The event is bad", Events[1]->IsBad()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "BD DepthCalibrationError (Out of Range)", "No other error flag is set", Events[1]->HasStripPairingError() || Events[1]->HasEnergyCalibrationError() || Events[1]->HasTACCalibrationError() || Events[1]->HasEventReconstructionError()) && Passed;
  ostringstream DepthOut;
  Events[1]->StreamBDFlags(DepthOut);
  Passed = EvaluateTrue("AnalyzeEvent()", "BD DepthCalibrationError (Out of Range)", "The reason is kept", MString(DepthOut.str()).Contains("BD DepthCalibrationError (Out of Range)\n")) && Passed;

  // ID 4: Compton event without bad string
  Passed = EvaluateTrue("AnalyzeEvent()", "Compton event", "An event without bad string is good", Events[2]->IsGood()) && Passed;

  // ID 5: guard ring veto
  Passed = EvaluateTrue("AnalyzeEvent()", "BD GR Veto", "The guard ring veto is set", Events[3]->GetGuardRingVeto()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "BD GR Veto", "The shield veto is not set", Events[3]->GetShieldVeto()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "BD GR Veto", "A vetoed event is still good", Events[3]->IsGood()) && Passed;

  // ID 6: bad string from revan -> no nuclearizer flag, but the physical event keeps it
  Passed = EvaluateTrue("AnalyzeEvent()", "BD ComptelTypeWithIncompatibleKinematics", "An unknown bad string does not make the assembly bad", Events[4]->IsGood()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "BD ComptelTypeWithIncompatibleKinematics", "An unknown bad string sets no veto", Events[4]->IsVeto()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "BD ComptelTypeWithIncompatibleKinematics", "The physical event keeps its bad string", Events[4]->GetPhysicalEvent() != nullptr && Events[4]->GetPhysicalEvent()->IsBad() == true && Events[4]->GetPhysicalEvent()->GetNBadFlags() == 1 && Events[4]->GetPhysicalEvent()->GetBadFlag(0) == "ComptelTypeWithIncompatibleKinematics") && Passed;

  // ID 7: strip pairing error with two reasons
  Passed = EvaluateTrue("AnalyzeEvent()", "BD StripPairingError with two reasons", "The strip pairing error is set", Events[5]->HasStripPairingError()) && Passed;
  ostringstream PairingOut;
  Events[5]->StreamBDFlags(PairingOut);
  Passed = EvaluateTrue("AnalyzeEvent()", "BD StripPairingError with two reasons", "Both reasons are kept", MString(PairingOut.str()).Contains("BD StripPairingError (First reason) (Second reason)\n")) && Passed;

  // ID 8: energy calibration error without reason
  Passed = EvaluateTrue("AnalyzeEvent()", "BD EnergyCalibrationError", "The energy calibration error is set", Events[6]->HasEnergyCalibrationError()) && Passed;
  ostringstream EnergyOut;
  Events[6]->StreamBDFlags(EnergyOut);
  Passed = EvaluateTrue("AnalyzeEvent()", "BD EnergyCalibrationError", "No empty reason is added", MString(EnergyOut.str()).Contains("BD EnergyCalibrationError\n")) && Passed;

  // ID 9-11
  Passed = EvaluateTrue("AnalyzeEvent()", "BD TACCalibrationError (No calibration)", "The TAC calibration error is set", Events[7]->HasTACCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "BD EventReconstructionError", "The event reconstruction error is set", Events[8]->HasEventReconstructionError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "BD Shield Veto", "The shield veto is set", Events[9]->GetShieldVeto()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "BD Shield Veto", "The guard ring veto is not set", Events[9]->GetGuardRingVeto()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "BD Shield Veto", "A vetoed event is still good", Events[9]->IsGood()) && Passed;

  // ID 13 is list index 11: one flag with the same reason once per strip, as the depth calibration writes it
  Passed = EvaluateTrue("AnalyzeEvent()", "BD DepthCalibrationError with a repeated reason", "The depth calibration error is set", Events[11]->HasDepthCalibrationError()) && Passed;
  ostringstream RepeatedOut;
  Events[11]->StreamBDFlags(RepeatedOut);
  Passed = EvaluateTrue("AnalyzeEvent()", "BD DepthCalibrationError with a repeated reason", "Every occurrence of the reason is kept", MString(RepeatedOut.str()).Contains("BD DepthCalibrationError (Multiple hits on single strip) (Multiple hits on single strip)\n")) && Passed;

  // ID 14: the reason text itself contains parentheses
  Passed = EvaluateTrue("AnalyzeEvent()", "BD StripPairingError with parentheses in the reason", "The strip pairing error is set", Events[12]->HasStripPairingError()) && Passed;
  ostringstream NestedOut;
  Events[12]->StreamBDFlags(NestedOut);
  Passed = EvaluateTrue("AnalyzeEvent()", "BD StripPairingError with parentheses in the reason", "The parentheses inside the reason are kept", MString(NestedOut.str()).Contains("BD StripPairingError (More than maximum number of strip hits allowed on one side (7))\n")) && Passed;

  // ID 15: several BD lines. Each line is one reason with its own flag, and all of them arrive
  // at the assembly, so an event whose error flags come with a veto stays bad.
  Passed = EvaluateTrue("AnalyzeEvent()", "several BD lines", "The strip pairing error of the first BD line is set", Events[13]->HasStripPairingError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "several BD lines", "The depth calibration error of the second BD line is set", Events[13]->HasDepthCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "several BD lines", "The guard ring veto of the third BD line is set", Events[13]->GetGuardRingVeto()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "several BD lines", "The event is bad, so the L2 saver screens it out", Events[13]->IsBad()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "several BD lines", "The physical event keeps every bad flag", Events[13]->GetPhysicalEvent()->GetNBadFlags(), 3u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "several BD lines", "The bad flags are kept in file order", Events[13]->GetPhysicalEvent()->GetBadFlag(0), MString("StripPairingError (No strip hits)")) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "several BD lines", "The last bad flag is the veto", Events[13]->GetPhysicalEvent()->GetBadFlag(2), MString("GR Veto")) && Passed;
  ostringstream SeveralOut;
  Events[13]->StreamBDFlags(SeveralOut);
  Passed = EvaluateTrue("AnalyzeEvent()", "several BD lines", "All three flags are streamed again", MString(SeveralOut.str()).Contains("BD StripPairingError (No strip hits)\n") == true && MString(SeveralOut.str()).Contains("BD DepthCalibrationError (GR Veto)\n") == true && MString(SeveralOut.str()).Contains("BD GR Veto\n") == true) && Passed;

  for (MReadOutAssembly* Event : Events) {
    delete Event;
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  Loader->Finalize();
  g_Verbosity = OldVerbosity;

  delete Loader;
  RemoveTemporaryFile(FileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::TestReinitialize()
{
  bool Passed = true;

  MString FileName = WriteFixture();
  MModuleLoaderMeasurementsTRA* Loader = CreateLoader(FileName);
  Passed = EvaluateTrue("Initialize()", "fixture tra file", "Initialize() succeeds for an existing tra file", Loader != nullptr) && Passed;
  if (Passed == false) return Passed;

  MReadOutAssembly Event;
  Loader->AnalyzeEvent(&Event);
  Loader->AnalyzeEvent(&Event);
  Passed = Evaluate("AnalyzeEvent()", "second event", "The second event has ID 2", Event.GetID(), (unsigned long) 2) && Passed;

  // Initialize again without Finalize
  Passed = EvaluateTrue("Initialize()", "open loader", "Initialize() succeeds on a loader with an open file", Loader->Initialize()) && Passed;
  Loader->AnalyzeEvent(&Event);
  Passed = Evaluate("AnalyzeEvent()", "after second Initialize", "Reading restarts at the first event", Event.GetID(), (unsigned long) 1) && Passed;

  // Initialize again after Finalize
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  Loader->Finalize();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("Initialize()", "finalized loader", "Initialize() succeeds on a finalized loader", Loader->Initialize()) && Passed;
  Loader->AnalyzeEvent(&Event);
  Passed = Evaluate("AnalyzeEvent()", "after Finalize and Initialize", "Reading restarts at the first event", Event.GetID(), (unsigned long) 1) && Passed;

  g_Verbosity = c_Quiet;
  Loader->Finalize();
  g_Verbosity = OldVerbosity;

  delete Loader;
  RemoveTemporaryFile(FileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::TestSaverRequirements()
{
  bool Passed = true;

  MString FileName = WriteFixture();
  MModuleLoaderMeasurementsTRA* Loader = CreateLoader(FileName);
  Passed = EvaluateTrue("Initialize()", "fixture tra file", "Initialize() succeeds for an existing tra file", Loader != nullptr) && Passed;
  if (Passed == false) return Passed;

  // L1b and L2 need the analysis up to event reconstruction, L1a needs the measured read-outs,
  // which a tra file does not contain
  MModuleSaverMeasurementsFITS SaverL2("XmlTagSaverMeasurementsFITSL2", 2, "Save events to L2 FITS");
  MModuleSaverMeasurementsFITS SaverL1b("XmlTagSaverMeasurementsFITSL1b", 1, "Save events to L1b FITS");
  MModuleSaverMeasurementsFITS SaverL1a("XmlTagSaverMeasurementsFITSL1a", 0, "Save events to L1a FITS");

  MReadOutAssembly Untouched;
  Passed = EvaluateFalse("FullfillsRequirements()", "assembly without progress", "The L2 FITS saver rejects an assembly which has not been through the analysis", SaverL2.FullfillsRequirements(&Untouched)) && Passed;

  unsigned int NEvents = 0;
  bool AllAcceptedL2 = true;
  bool AllAcceptedL1b = true;
  bool AnyAcceptedL1a = false;
  MReadOutAssembly Event;
  while (Loader->AnalyzeEvent(&Event) == true) {
    if (SaverL2.FullfillsRequirements(&Event) == false) {
      AllAcceptedL2 = false;
    }
    if (SaverL1b.FullfillsRequirements(&Event) == false) {
      AllAcceptedL1b = false;
    }
    if (SaverL1a.FullfillsRequirements(&Event) == true) {
      AnyAcceptedL1a = true;
    }
    ++NEvents;
  }

  Passed = Evaluate("AnalyzeEvent()", "fixture tra file", "All events with ET line are loaded", NEvents, 14u) && Passed;
  Passed = EvaluateTrue("FullfillsRequirements()", "loaded events", "The L2 FITS saver accepts every loaded event", AllAcceptedL2) && Passed;
  Passed = EvaluateTrue("FullfillsRequirements()", "loaded events", "The L1b FITS saver accepts every loaded event", AllAcceptedL1b) && Passed;
  Passed = EvaluateFalse("FullfillsRequirements()", "loaded events", "The L1a FITS saver accepts no loaded event, since the measurement progress flag is not set", AnyAcceptedL1a) && Passed;

  // The type comes from MModuleLoaderMeasurements, the progress flag is not set by this loader
  Passed = EvaluateTrue("ProvidesModuleType()", "measurement loader", "The loader provides the measurement loader type", Loader->ProvidesModuleType(MAssembly::c_EventLoaderMeasurement)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "event loader", "The loader provides the event loader type", Loader->ProvidesModuleType(MAssembly::c_EventLoader)) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  Loader->Finalize();
  g_Verbosity = OldVerbosity;

  delete Loader;
  RemoveTemporaryFile(FileName);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsTRA::TestCommittedData()
{
  bool Passed = true;

  const char* NuclearizerEnv = getenv("NUCLEARIZER");
  if (NuclearizerEnv == nullptr || NuclearizerEnv[0] == '\0') {
    mout<<"TestCommittedData: NUCLEARIZER not set - skipping test"<<endl;
    return Passed;
  }
  MString FileName = MString(NuclearizerEnv) + "/resource/unittestdata/542-1/hdf5-to-tra.reference.tra";

  Passed = EvaluateTrue("Initialize()", "542-1 reference tra file", "The reference tra file exists", MFile::Exists(FileName)) && Passed;
  if (Passed == false) return Passed;

  MModuleLoaderMeasurementsTRA* Loader = CreateLoader(FileName);
  Passed = EvaluateTrue("Initialize()", "542-1 reference tra file", "Initialize() succeeds for the reference tra file", Loader != nullptr) && Passed;
  if (Passed == false) return Passed;

  // Reference values from the file: 6803 SE blocks, 5206 of them with an ET line (833 CO, 3227 PH, 1146 UN),
  // 1118 UN events carry nuclearizer error flags; the file IDs begin 1, 2, 4, 5, 6 since ID 3 has no ET line
  unsigned int NEvents = 0;
  unsigned int NGood = 0;
  unsigned int NCompton = 0;
  unsigned int NPhoto = 0;
  unsigned int NComptonWithSequence = 0;
  unsigned int NPhotoWithOneHit = 0;
  unsigned int NOtherWithoutHits = 0;
  vector<unsigned long> FirstIDs;
  unsigned long LastID = 0;

  MReadOutAssembly* Event = new MReadOutAssembly();
  while (Loader->AnalyzeEvent(Event) == true) {
    ++NEvents;
    if (FirstIDs.size() < 5) FirstIDs.push_back(Event->GetID());
    LastID = Event->GetID();
    if (Event->IsGood() == true) ++NGood;

    MPhysicalEvent* PhysicalEvent = Event->GetPhysicalEvent();
    if (PhysicalEvent != nullptr && PhysicalEvent->GetType() == MPhysicalEvent::c_Compton) {
      ++NCompton;
      MComptonEvent* Compton = dynamic_cast<MComptonEvent*>(PhysicalEvent);
      if (Compton != nullptr && Event->GetNHits() == Compton->SequenceLength() && Event->GetNHits() >= 2) ++NComptonWithSequence;
    } else if (PhysicalEvent != nullptr && PhysicalEvent->GetType() == MPhysicalEvent::c_Photo) {
      ++NPhoto;
      if (Event->GetNHits() == 1) ++NPhotoWithOneHit;
    } else if (Event->GetNHits() == 0) {
      ++NOtherWithoutHits;
    }

    delete Event;
    Event = new MReadOutAssembly();
  }
  delete Event;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  Loader->Finalize();
  g_Verbosity = OldVerbosity;
  delete Loader;

  Passed = Evaluate("AnalyzeEvent()", "542-1 reference tra file", "All 5206 events with ET line are loaded", NEvents, 5206u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "542-1 reference tra file", "4088 events are good", NGood, 4088u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "542-1 reference tra file", "833 events are Compton events", NCompton, 833u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "542-1 reference tra file", "3227 events are photo events", NPhoto, 3227u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "542-1 reference tra file", "Every Compton event has one hit per sequence element", NComptonWithSequence, 833u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "542-1 reference tra file", "Every photo event has one hit", NPhotoWithOneHit, 3227u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "542-1 reference tra file", "No other event has hits", NOtherWithoutHits, 1146u) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "542-1 reference tra file", "The first IDs skip the event without ET line", FirstIDs == vector<unsigned long>({ 1, 2, 4, 5, 6 })) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "542-1 reference tra file", "The last ID is 6803", LastID, (unsigned long) 6803) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNModuleLoaderMeasurementsTRA", "Unit tests for MModuleLoaderMeasurementsTRA") == false) return 1;

  UTNModuleLoaderMeasurementsTRA Test;
  return Test.Run() == true ? 0 : 1;
}
