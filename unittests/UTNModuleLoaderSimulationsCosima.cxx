/*
 * UTNModuleLoaderSimulationsCosima.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstring>
#include <new>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MSupervisor.h"
#include "MDGeometryQuest.h"
#include "MSimEvent.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleLoaderSimulationsCosima.h"
#include "MModuleEventSaver.h"
#include "MReadOutAssembly.h"


//! Unit test class for MModuleLoaderSimulationsCosima
class UTNModuleLoaderSimulationsCosima : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleLoaderSimulationsCosima() : MUnitTest("UTNModuleLoaderSimulationsCosima"), m_Geometry(nullptr) {}
  //! Default destructor
  virtual ~UTNModuleLoaderSimulationsCosima() { delete m_Geometry; }

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and the defaults
  bool TestDefaultConstruction();
  //! Test the setters/getters and the XML round trip
  bool TestConfiguration();
  //! Test the Initialize() error paths
  bool TestInitializeErrors();
  //! Test reading the simulated events
  bool TestRead();
  //! Test the stop-after option
  bool TestStopAfter();
  //! Test the hand-over of the simulation meta data to the event saver
  bool TestSaverHandOver();

  //! Load the test geometry once, return nullptr on failure
  MDGeometryQuest* GetGeometry();
  //! Write the sim file and return its name
  MString WriteSimFile();

  //! The geometry file name
  MString m_GeometryFileName;
  //! The test geometry
  MDGeometryQuest* m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderSimulationsCosima::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestConfiguration() && Passed;
  Passed = TestInitializeErrors() && Passed;
  Passed = TestRead() && Passed;
  Passed = TestStopAfter() && Passed;
  Passed = TestSaverHandOver() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest* UTNModuleLoaderSimulationsCosima::GetGeometry()
{
  if (m_Geometry != nullptr) return m_Geometry;

  MString G;
  G += "Name UTNSimGeometry\n";
  G += "SurroundingSphere 20 0.0 0.0 0.0 20\nShowSurroundingSphere False\n\n";
  G += "Material Vacuum\nVacuum.Density 1E-12\nVacuum.Component H 1\n\n";
  G += "Material Germanium\nGermanium.Density 5.323\nGermanium.ComponentByMass Ge 1\n\n";
  G += "Material BGO\nBGO.Density 7.13\nBGO.ComponentByAtoms Bi 4\nBGO.ComponentByAtoms Ge 3\nBGO.ComponentByAtoms O 12\n\n";
  G += "Volume WorldVolume\nWorldVolume.Material Vacuum\nWorldVolume.Visibility 0\nWorldVolume.Shape Sphere 0.0 100\nWorldVolume.Mother 0\n\n";
  G += "Volume Wafer\nWafer.Material Germanium\nWafer.Shape BRIK 4.0 4.0 0.75\nWafer.Position 0.0 0.0 0.0\nWafer.Mother WorldVolume\n\n";
  G += "Volume Shield\nShield.Material BGO\nShield.Shape BRIK 2.0 2.0 2.0\nShield.Position 0.0 0.0 10.0\nShield.Mother WorldVolume\n\n";
  G += "MDStrip3D GeD_0\nGeD_0.DetectorVolume Wafer\nGeD_0.SensitiveVolume Wafer\nGeD_0.Offset 0.0 0.0\nGeD_0.StripNumber 64 64\n";
  G += "GeD_0.NoiseThreshold 15\nGeD_0.TriggerThreshold 30\nGeD_0.DepthResolution 100 0.02\nGeD_0.DepthResolution 1000 0.02\n";
  G += "GeD_0.EnergyResolution Gaus 100 100 1\nGeD_0.EnergyResolution Gaus 1000 1000 2\nGeD_0.FailureRate 0.0\n\n";
  G += "Voxel3D ACS_X0_3\nACS_X0_3.DetectorVolume Shield\nACS_X0_3.SensitiveVolume Shield\nACS_X0_3.Offset 0.0 0.0 0.0\nACS_X0_3.VoxelNumber 2 2 2\n";
  G += "ACS_X0_3.NoiseThreshold 10\nACS_X0_3.TriggerThreshold 20\nACS_X0_3.EnergyResolution Gaus 100 100 5\nACS_X0_3.EnergyResolution Gaus 1000 1000 10\nACS_X0_3.FailureRate 0.0\n\n";
  G += "Trigger T1\nT1.Veto false\nT1.TriggerByChannel true\nT1.Detector GeD_0 1\n\n";
  G += "Trigger T2\nT2.Veto false\nT2.TriggerByChannel true\nT2.Detector ACS_X0_3 1\n";

  m_GeometryFileName = GetTemporaryFileName("sim.geo.setup");
  WriteTextFile(m_GeometryFileName, G);

  MDGeometryQuest* Geo = new MDGeometryQuest();
  DisableDefaultStreams();
  bool Loaded = Geo->ScanSetupFile(m_GeometryFileName, false, false, false);
  EnableDefaultStreams();
  if (Loaded == false) {
    delete Geo;
    return nullptr;
  }
  m_Geometry = Geo;
  return m_Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleLoaderSimulationsCosima::WriteSimFile()
{
  GetGeometry();

  MString S;
  S += "# Simulation file for UTNModuleLoaderSimulationsCosima\n\n";
  S += "Version 101\nType SIM\n";
  S += MString("Geometry ") + m_GeometryFileName + "\n\n";
  S += "StartAreaFarField 3.5\n\n";
  S += "TB 0\n\n";
  S += "SE\nID 1 1\nTI 12.5\nHTsim 3;0.1;0.2;0.5;500.0;0.0;1\n";
  S += "SE\nID 2 2\nTI 13.5\nHTsim 3;-1.0;1.0;-0.5;600.0;0.0;2\nHTsim 8;0.5;0.5;10.5;300.0;0.0;2;3\n";
  S += "SE\nID 3 7\nTI 14.0\n";
  S += "EN\n\nTE 20.0\nTS 1000\n";

  MString FileName = GetTemporaryFileName("events.sim");
  WriteTextFile(FileName, S);
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderSimulationsCosima::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleLoaderSimulationsCosima M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Cosima simulation loader'", M.GetName(), MString("Cosima simulation loader")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagLoaderSimulations'", M.GetXmlTag(), MString("XmlTagLoaderSimulations")) && Passed;
  Passed = EvaluateTrue("IsStartModule()", "default", "The loader is a start module", M.IsStartModule()) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The loader provides c_EventLoader", M.ProvidesModuleType(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The loader provides c_EventLoaderSimulation", M.ProvidesModuleType(MAssembly::c_EventLoaderSimulation)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The loader has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = EvaluateFalse("UseStopAfter()", "default", "Stop-after is off by default", M.UseStopAfter()) && Passed;
  Passed = Evaluate("GetMaximumAcceptedEvents()", "default", "The default maximum is 10000000", M.GetMaximumAcceptedEvents(), 10000000ul) && Passed;
  Passed = Evaluate("GetSimulationFileName()", "default", "The default file name is empty", M.GetSimulationFileName(), MString("")) && Passed;

  MModuleLoaderSimulationsCosima* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a non-null object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderSimulationsCosima::TestConfiguration()
{
  bool Passed = true;

  MModuleLoaderSimulationsCosima Writer;
  Writer.SetSimulationFileName("/data/in.sim");
  Writer.SetUseStopAfter(true);
  Writer.SetMaximumAcceptedEvents(1234);
  Passed = Evaluate("SetSimulationFileName/GetSimulationFileName", "/data/in.sim", "Returns the set name", Writer.GetSimulationFileName(), MString("/data/in.sim")) && Passed;
  Passed = EvaluateTrue("SetUseStopAfter/UseStopAfter", "true", "Returns true", Writer.UseStopAfter()) && Passed;
  Passed = Evaluate("SetMaximumAcceptedEvents/GetMaximumAcceptedEvents", "1234", "Returns 1234", Writer.GetMaximumAcceptedEvents(), 1234ul) && Passed;

  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleLoaderSimulationsCosima Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "configured", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "SimulationFileName", "The file name survives", Reader.GetSimulationFileName(), MString("/data/in.sim")) && Passed;
  // The stop-after options are set in the options GUI and must be part of the configuration
  Passed = ExpectedToFail(EvaluateTrue("ReadXmlConfiguration()", "UseStopAfter", "The stop-after flag survives", Reader.UseStopAfter())) && Passed;
  Passed = ExpectedToFail(Evaluate("ReadXmlConfiguration()", "MaximumAcceptedEvents", "The maximum number of accepted events survives", Reader.GetMaximumAcceptedEvents(), 1234ul)) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderSimulationsCosima::TestInitializeErrors()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  Passed = EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", G != nullptr) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  MModuleLoaderSimulationsCosima M;
  M.SetGeometry(G);
  M.SetSimulationFileName(GetTemporaryFileName("missing.sim"));
  bool Result = M.Initialize();
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("Initialize()", "missing file", "Initialize returns false", Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderSimulationsCosima::TestRead()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  if (G == nullptr) return EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", false);

  MModuleLoaderSimulationsCosima M;
  M.SetGeometry(G);
  M.SetSimulationFileName(WriteSimFile());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool Init = M.Initialize();
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("Initialize()", "sim file", "Initialize succeeds", Init) && Passed;
  if (Init == false) return false;

  vector<unsigned int> ExpectedHTs = { 1, 2, 0 };
  vector<long> ExpectedIDs = { 1, 2, 3 };
  vector<double> ExpectedTimes = { 12.5, 13.5, 14.0 };
  for (unsigned int e = 0; e < ExpectedIDs.size(); ++e) {
    MReadOutAssembly E;
    MString Input = MString("event ") + (e + 1);
    DisableDefaultStreams();
    bool Result = M.AnalyzeEvent(&E);
    EnableDefaultStreams();
    Passed = EvaluateTrue("AnalyzeEvent()", Input, "AnalyzeEvent returns true", Result) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", Input, "The simulated event is attached", E.GetSimulatedEvent() != nullptr) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", Input, "The event loader analysis progress is set", E.HasAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderSimulation)) && Passed;
    if (E.GetSimulatedEvent() != nullptr) {
      Passed = Evaluate("AnalyzeEvent()", Input, "The simulated event ID is read", E.GetSimulatedEvent()->GetID(), ExpectedIDs[e]) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", Input, "The simulated event time is read", E.GetSimulatedEvent()->GetTime().GetAsDouble(), ExpectedTimes[e], 1e-9) && Passed;
      Passed = Evaluate("AnalyzeEvent()", Input, "All simulated hits are read", E.GetSimulatedEvent()->GetNHTs(), ExpectedHTs[e]) && Passed;
    }
  }

  MReadOutAssembly End;
  DisableDefaultStreams();
  bool EndResult = M.AnalyzeEvent(&End);
  EnableDefaultStreams();
  Passed = EvaluateFalse("AnalyzeEvent()", "end of file", "AnalyzeEvent returns false", EndResult) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "end of file", "The loader is finished", M.IsFinished()) && Passed;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderSimulationsCosima::TestStopAfter()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  if (G == nullptr) return EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", false);

  MModuleLoaderSimulationsCosima M;
  M.SetGeometry(G);
  M.SetSimulationFileName(WriteSimFile());
  M.SetUseStopAfter(true);
  M.SetMaximumAcceptedEvents(2);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  unsigned int N = 0;
  if (M.Initialize() == true) {
    for (unsigned int i = 0; i < 10; ++i) {
      MReadOutAssembly E;
      if (M.AnalyzeEvent(&E) == false) break;
      ++N;
    }
    M.Finalize();
  }
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;
  Passed = Evaluate("AnalyzeEvent()", "stop after 2", "Only two events are accepted", N, 2u) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "stop after 2", "The loader is finished after the maximum", M.IsFinished()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderSimulationsCosima::TestSaverHandOver()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  if (G == nullptr) return EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", false);

  // The loader passes the start area and the number of simulated events to the available event saver
  MSupervisor* S = MSupervisor::GetSupervisor();
  if (S->GetAvailableModuleByXmlTag("XmlTagEventSaver") == nullptr) {
    S->AddAvailableModule(new MModuleEventSaver());
  }
  MModuleEventSaver* Saver = dynamic_cast<MModuleEventSaver*>(S->GetAvailableModuleByXmlTag("XmlTagEventSaver"));
  if (Saver == nullptr) {
    return EvaluateTrue("Initialize()", "event saver", "An event saver is available", false);
  }
  MString OutFile = GetTemporaryFileName("handover.tra");
  Saver->SetMode(MModuleEventSaver::c_TraFile);
  Saver->SetSplitFile(false);
  Saver->SetFileName(OutFile);

  // Construct the loader in memory filled with 0xFF to make uninitialized members visible
  alignas(MModuleLoaderSimulationsCosima) unsigned char Buffer[sizeof(MModuleLoaderSimulationsCosima)];
  memset(Buffer, 0xFF, sizeof(Buffer));
  MModuleLoaderSimulationsCosima* M = new (Buffer) MModuleLoaderSimulationsCosima;
  M->SetGeometry(G);
  M->SetSimulationFileName(WriteSimFile());

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool Init = M->Initialize();
  bool SaverInit = Saver->Initialize();
  for (unsigned int i = 0; i < 10 && Init == true; ++i) {
    MReadOutAssembly E;
    if (M->AnalyzeEvent(&E) == false) break;
  }
  if (Init == true) M->Finalize();
  if (SaverInit == true) Saver->Finalize();
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;
  M->~MModuleLoaderSimulationsCosima();

  Passed = EvaluateTrue("Initialize()", "hand-over", "The loader initializes", Init) && Passed;
  Passed = EvaluateTrue("MModuleEventSaver::Initialize()", "hand-over", "The saver initializes", SaverInit) && Passed;

  istringstream In(ReadTextFile(OutFile).GetString());
  string Line;
  bool HasStartArea = false;
  MString TSLine;
  while (getline(In, Line).fail() == false) {
    MString L(Line);
    if (L.BeginsWith("StartAreaFarField 3.5") == true) HasStartArea = true;
    if (L.BeginsWith("TS ") == true) TSLine = L;
  }
  Passed = EvaluateTrue("Initialize()", "hand-over", "The far-field start area of the sim file is written by the saver", HasStartArea) && Passed;
  Passed = ExpectedToFail(Evaluate("Finalize()", "hand-over", "The number of simulated events of the sim file (TS 1000) is written by the saver", TSLine, MString("TS 1000"))) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleLoaderSimulationsCosima Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleLoaderSimulationsCosima.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
