/*
 * UTNSubModuleDEEIntake.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <functional>
#include <list>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT:
#include "TSystem.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MDGeometryQuest.h"
#include "MFileEventsSim.h"
#include "MSimEvent.h"

// Nuclearizer:
#include "MSubModuleDEEIntake.h"
#include "MReadOutAssembly.h"
#include "MDEEStripHit.h"
#include "MDEECrystalHit.h"


//! Unit test class for MSubModuleDEEIntake
class UTNSubModuleDEEIntake : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleDEEIntake() : MUnitTest("UTNSubModuleDEEIntake"), m_Geometry(nullptr) {}
  //! Default destructor
  virtual ~UTNSubModuleDEEIntake() { delete m_Geometry; }

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the lifecycle and the (empty) XML configuration
  bool TestLifecycle();
  //! Test the intake of a germanium detector hit
  bool TestGermaniumHit();
  //! Test the intake of a germanium and a shield hit
  bool TestShieldHit();
  //! Test an event without hits
  bool TestEmptyEvent();
  //! Test an event without simulated event
  bool TestNoSimulatedEvent();

  //! Load the test geometry once, return nullptr on failure
  MDGeometryQuest* GetGeometry();
  //! Write the sim file and return its name
  MString WriteSimFile();
  //! Read the simulated events from the sim file; the events have to be deleted by the caller
  vector<MSimEvent*> ReadSimEvents();
  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);

  //! The geometry file name
  MString m_GeometryFileName;
  //! The test geometry
  MDGeometryQuest* m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEIntake::Run()
{
  bool Passed = true;

  Passed = TestLifecycle() && Passed;
  Passed = TestGermaniumHit() && Passed;
  Passed = TestShieldHit() && Passed;
  Passed = TestEmptyEvent() && Passed;
  Passed = TestNoSimulatedEvent() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest* UTNSubModuleDEEIntake::GetGeometry()
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
  G += "MDStrip3D GeD_2\nGeD_2.DetectorVolume Wafer\nGeD_2.SensitiveVolume Wafer\nGeD_2.Offset 0.0 0.0\nGeD_2.StripNumber 64 64\n";
  G += "GeD_2.NoiseThreshold 15\nGeD_2.TriggerThreshold 30\nGeD_2.DepthResolution 100 0.02\nGeD_2.DepthResolution 1000 0.02\n";
  G += "GeD_2.EnergyResolution Gaus 100 100 1\nGeD_2.EnergyResolution Gaus 1000 1000 2\nGeD_2.FailureRate 0.0\n\n";
  G += "Voxel3D ACS_X0_3\nACS_X0_3.DetectorVolume Shield\nACS_X0_3.SensitiveVolume Shield\nACS_X0_3.Offset 0.0 0.0 0.0\nACS_X0_3.VoxelNumber 2 2 2\n";
  G += "ACS_X0_3.NoiseThreshold 10\nACS_X0_3.TriggerThreshold 20\nACS_X0_3.EnergyResolution Gaus 100 100 5\nACS_X0_3.EnergyResolution Gaus 1000 1000 10\nACS_X0_3.FailureRate 0.0\n\n";
  G += "Trigger T1\nT1.Veto false\nT1.TriggerByChannel true\nT1.Detector GeD_2 1\n\n";
  G += "Trigger T2\nT2.Veto false\nT2.TriggerByChannel true\nT2.Detector ACS_X0_3 1\n";

  m_GeometryFileName = GetTemporaryFileName("dee.geo.setup");
  WriteTextFile(m_GeometryFileName, G);

  MDGeometryQuest* Geo = new MDGeometryQuest();
  DisableDefaultStreams();
  bool Loaded = Geo->ScanSetupFile(m_GeometryFileName, false, false, false);
  EnableDefaultStreams();
  if (Loaded == false) {
    delete Geo;
    return nullptr;
  }
  // The supervisor loads the geometry without noising, the DEE does its own
  Geo->ActivateNoising(false);
  m_Geometry = Geo;
  return m_Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleDEEIntake::WriteSimFile()
{
  GetGeometry();

  MString S;
  S += "Version 101\nType SIM\n";
  S += MString("Geometry ") + m_GeometryFileName + "\n\n";
  S += "TB 0\n\n";
  // Event 1: one hit in GeD_2 at depth z = 0.5 cm (relative depth 0.8333)
  S += "SE\nID 1 1\nTI 12.5\nHTsim 3;0.1;0.2;0.5;500.0;0.0;1\n";
  // Event 2: one hit in GeD_2 at z = -0.5 cm (relative depth 0.1667) and one in the shield voxel (1, 1, 1)
  S += "SE\nID 2 2\nTI 13.5\nHTsim 3;-1.0;1.0;-0.5;600.0;0.0;2\nHTsim 8;0.5;0.5;10.5;300.0;0.0;2;3\n";
  // Event 3: no hits
  S += "SE\nID 3 7\nTI 14.0\n";
  S += "EN\n\nTE 20.0\nTS 1000\n";

  MString FileName = GetTemporaryFileName("dee.sim");
  WriteTextFile(FileName, S);
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


vector<MSimEvent*> UTNSubModuleDEEIntake::ReadSimEvents()
{
  vector<MSimEvent*> Events;
  MDGeometryQuest* G = GetGeometry();
  if (G == nullptr) return Events;

  MFileEventsSim Reader(G);
  DisableDefaultStreams();
  if (Reader.Open(WriteSimFile()) == true) {
    MSimEvent* E = nullptr;
    while ((E = Reader.GetNextEvent(false)) != nullptr) {
      Events.push_back(E);
    }
    Reader.Close();
  }
  EnableDefaultStreams();
  return Events;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEIntake::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // A crash is a possible outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Time-out of the child process in seconds
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


bool UTNSubModuleDEEIntake::TestLifecycle()
{
  bool Passed = true;

  MSubModuleDEEIntake M;
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;
  M.Clear();
  MXmlNode* Node = new MXmlNode(nullptr, "DEE");
  Passed = EvaluateTrue("CreateXmlConfiguration()", "mother node", "The mother node is returned", M.CreateXmlConfiguration(Node) == Node) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "mother node", "No configuration is added", Node->GetNNodes(), 0u) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "mother node", "ReadXmlConfiguration returns true", M.ReadXmlConfiguration(Node)) && Passed;
  delete Node;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEIntake::TestGermaniumHit()
{
  bool Passed = true;

  vector<MSimEvent*> SimEvents = ReadSimEvents();
  Passed = Evaluate("MFileEventsSim::GetNextEvent()", "sim file", "Three simulated events are available", (unsigned int) SimEvents.size(), 3u) && Passed;
  if (SimEvents.size() != 3) {
    for (MSimEvent* S : SimEvents) delete S;
    return false;
  }

  MSubModuleDEEIntake M;
  M.Initialize();
  MReadOutAssembly E;
  E.SetSimulatedEvent(SimEvents[0]);
  Passed = EvaluateTrue("AnalyzeEvent()", "one GeD hit", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one GeD hit", "The event ID is taken from the simulation", E.GetID(), (unsigned long) 1) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "one GeD hit", "The UTC time is the simulated time", E.GetTimeUTC().GetAsDouble(), 12.5, 1e-9) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "one GeD hit", "The time is the simulated time", E.GetTime().GetAsDouble(), 12.5, 1e-9) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one GeD hit", "One LV DEE strip hit is created", E.GetNDEEStripHitsLV(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one GeD hit", "One HV DEE strip hit is created", E.GetNDEEStripHitsHV(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one GeD hit", "No DEE crystal hit is created", E.GetNDEECrystalHits(), 0u) && Passed;
  if (E.GetNDEEStripHitsLV() == 1 && E.GetNDEEStripHitsHV() == 1) {
    MDEEStripHit& LV = E.GetDEEStripHitLVListReference().front();
    MDEEStripHit& HV = E.GetDEEStripHitHVListReference().front();
    Passed = EvaluateTrue("AnalyzeEvent()", "LV hit", "The LV hit is on the LV side", LV.m_ROE.IsLowVoltageStrip()) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "HV hit", "The HV hit is on the HV side", HV.m_ROE.IsLowVoltageStrip()) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV hit", "The detector ID is 2 (from GeD_2)", LV.m_ROE.GetDetectorID(), 2u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "HV hit", "The detector ID is 2 (from GeD_2)", HV.m_ROE.GetDetectorID(), 2u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV hit", "The simulated event ID is 1", LV.m_SimulatedEventID, (unsigned long) 1) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV hit", "The simulated hit index is 0", LV.m_SimulatedHitIndex, 0u) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "LV hit", "The simulated energy is 500 keV", LV.m_SimulatedEnergy, 500.0, 1e-9) && Passed;
    Passed = EvaluateVectorNear("AnalyzeEvent()", "LV hit", "The simulated position is the HT position", LV.m_SimulatedPosition, MVector(0.1, 0.2, 0.5), 1e-9) && Passed;
    Passed = EvaluateVectorNear("AnalyzeEvent()", "LV hit", "The position in the detector is the local position", LV.m_SimulatedPositionInDetector, MVector(0.1, 0.2, 0.5), 1e-9) && Passed;
    // The wafer is 1.5 cm thick: z = 0.5 cm is at (0.5 + 0.75) / 1.5 of the thickness
    Passed = EvaluateNear("AnalyzeEvent()", "LV hit at z = 0.5 cm", "The relative depth is 0.8333 (within [0, 1])", LV.m_SimulatedRelativeDepth, 1.25/1.5, 1e-6) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "HV hit at z = 0.5 cm", "The relative depth is identical on both sides", HV.m_SimulatedRelativeDepth, LV.m_SimulatedRelativeDepth, 1e-12) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "LV hit", "The hit is not a simulated guard ring hit", LV.m_SimulatedIsGuardRing) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV hit", "The LV ID is 1000", LV.m_ID, 1000u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "HV hit", "The HV ID is 1001", HV.m_ID, 1001u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV hit", "The opposite side of LV is HV", LV.m_OppositeSideID, HV.m_ID) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "HV hit", "The opposite side of HV is LV", HV.m_OppositeSideID, LV.m_ID) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV hit", "The origins are copied", (unsigned int) LV.m_SimulatedOrigins.size(), 1u) && Passed;
    if (LV.m_SimulatedOrigins.size() == 1) {
      Passed = Evaluate("AnalyzeEvent()", "LV hit", "The origin is 1", LV.m_SimulatedOrigins.front(), 1) && Passed;
    }
  }

  for (unsigned int i = 1; i < SimEvents.size(); ++i) delete SimEvents[i];
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEIntake::TestShieldHit()
{
  bool Passed = true;

  vector<MSimEvent*> SimEvents = ReadSimEvents();
  if (SimEvents.size() != 3) {
    for (MSimEvent* S : SimEvents) delete S;
    return EvaluateTrue("MFileEventsSim::GetNextEvent()", "sim file", "Three simulated events are available", false);
  }

  MSubModuleDEEIntake M;
  M.Initialize();
  MReadOutAssembly E;
  E.SetSimulatedEvent(SimEvents[1]);
  Passed = EvaluateTrue("AnalyzeEvent()", "GeD and shield hit", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "GeD and shield hit", "One LV DEE strip hit is created", E.GetNDEEStripHitsLV(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "GeD and shield hit", "One DEE crystal hit is created", E.GetNDEECrystalHits(), 1u) && Passed;
  if (E.GetNDEEStripHitsLV() == 1) {
    MDEEStripHit& LV = E.GetDEEStripHitLVListReference().front();
    Passed = EvaluateNear("AnalyzeEvent()", "LV hit at z = -0.5 cm", "The relative depth is 0.1667 (within [0, 1])", LV.m_SimulatedRelativeDepth, 0.25/1.5, 1e-6) && Passed;
  }
  if (E.GetNDEECrystalHits() == 1) {
    MDEECrystalHit& C = E.GetDEECrystalHitListReference().front();
    Passed = Evaluate("AnalyzeEvent()", "shield hit", "The detector ID is X0 (from ACS_X0_3)", C.m_DetectorID, MString("X0")) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "shield hit", "The crystal ID is 3 (from ACS_X0_3)", C.m_CrystalID, 3u) && Passed;
    Passed = EvaluateVectorNear("AnalyzeEvent()", "shield hit", "The voxel is (1, 1, 1)", C.m_VoxelInDetector, MVector(1, 1, 1), 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "shield hit", "The simulated energy is 300 keV", C.m_SimulatedEnergy, 300.0, 1e-9) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "shield hit", "The simulated hit index is 1", C.m_SimulatedHitIndex, 1u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "shield hit", "Both origins are copied", (unsigned int) C.m_SimulatedOrigins.size(), 2u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "shield hit", "The read-out element has the detector ID", C.m_ROE.GetDetectorID(), MString("X0")) && Passed;
  }

  delete SimEvents[0];
  delete SimEvents[2];
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEIntake::TestEmptyEvent()
{
  bool Passed = true;

  vector<MSimEvent*> SimEvents = ReadSimEvents();
  if (SimEvents.size() != 3) {
    for (MSimEvent* S : SimEvents) delete S;
    return EvaluateTrue("MFileEventsSim::GetNextEvent()", "sim file", "Three simulated events are available", false);
  }

  MSubModuleDEEIntake M;
  M.Initialize();
  MReadOutAssembly E;
  E.SetSimulatedEvent(SimEvents[2]);
  Passed = EvaluateTrue("AnalyzeEvent()", "no hits", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "no hits", "The event ID is 3", E.GetID(), (unsigned long) 3) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "no hits", "No DEE strip hits are created", E.GetNDEEStripHitsLV() + E.GetNDEEStripHitsHV(), 0u) && Passed;

  delete SimEvents[0];
  delete SimEvents[1];
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEIntake::TestNoSimulatedEvent()
{
  bool Passed = true;

  bool Graceful = RunInChild([]() -> bool {
    MSubModuleDEEIntake M;
    M.Initialize();
    MReadOutAssembly E;
    M.AnalyzeEvent(&E);
    return true;
  });
  Passed = EvaluateTrue("AnalyzeEvent()", "no simulated event", "An event without simulated event does not crash the intake", Graceful) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleDEEIntake Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleDEEIntake.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
