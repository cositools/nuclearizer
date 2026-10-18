/*
 * UTNModuleRevan.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <functional>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
using namespace std;

// ROOT:
#include "TSystem.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MDGeometryQuest.h"
#include "MSettingsRevan.h"
#include "MPhysicalEvent.h"
#include "MComptonEvent.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleRevan.h"
#include "MReadOutAssembly.h"
#include "MHit.h"


//! Unit test class for MModuleRevan
class UTNModuleRevan : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleRevan() : MUnitTest("UTNModuleRevan"), m_Geometry(nullptr) {}
  //! Default destructor
  virtual ~UTNModuleRevan() { delete m_Geometry; }

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data
  bool TestDefaultConstruction();
  //! Test the setters/getters and the XML round trip
  bool TestConfiguration();
  //! Test the Initialize() error paths
  bool TestInitializeErrors();
  //! Test the reconstruction of simple events
  bool TestReconstruction();

  //! Load the test geometry once, return nullptr on failure
  MDGeometryQuest* GetGeometry();
  //! Write a default revan configuration file and return its name
  MString WriteRevanConfiguration();
  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);

  //! The test geometry
  MDGeometryQuest* m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleRevan::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestConfiguration() && Passed;
  Passed = TestInitializeErrors() && Passed;
  Passed = TestReconstruction() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest* UTNModuleRevan::GetGeometry()
{
  if (m_Geometry != nullptr) return m_Geometry;

  MString G;
  G += "Name UTNRevanGeometry\n";
  G += "SurroundingSphere 20 0.0 0.0 0.0 20\nShowSurroundingSphere False\n\n";
  G += "Material Vacuum\nVacuum.Density 1E-12\nVacuum.Component H 1\n\n";
  G += "Material Germanium\nGermanium.Density 5.323\nGermanium.ComponentByMass Ge 1\n\n";
  G += "Volume WorldVolume\nWorldVolume.Material Vacuum\nWorldVolume.Visibility 0\nWorldVolume.Shape Sphere 0.0 100\nWorldVolume.Mother 0\n\n";
  G += "Volume Wafer\nWafer.Material Germanium\nWafer.Shape BRIK 4.0 4.0 0.75\nWafer.Position 0.0 0.0 0.0\nWafer.Mother WorldVolume\n\n";
  G += "MDStrip3D GeD_0\nGeD_0.DetectorVolume Wafer\nGeD_0.SensitiveVolume Wafer\nGeD_0.Offset 0.0 0.0\nGeD_0.StripNumber 64 64\n";
  G += "GeD_0.NoiseThreshold 15\nGeD_0.TriggerThreshold 30\nGeD_0.DepthResolution 100 0.02\nGeD_0.DepthResolution 1000 0.02\n";
  G += "GeD_0.EnergyResolution Gaus 100 100 1\nGeD_0.EnergyResolution Gaus 1000 1000 2\nGeD_0.FailureRate 0.0\n\n";
  G += "Trigger T1\nT1.Veto false\nT1.TriggerByChannel true\nT1.Detector GeD_0 1\n";

  MString FileName = GetTemporaryFileName("revan.geo.setup");
  WriteTextFile(FileName, G);

  MDGeometryQuest* Geo = new MDGeometryQuest();
  DisableDefaultStreams();
  bool Loaded = Geo->ScanSetupFile(FileName, false, false, false);
  EnableDefaultStreams();
  if (Loaded == false) {
    delete Geo;
    return nullptr;
  }
  m_Geometry = Geo;
  return m_Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleRevan::WriteRevanConfiguration()
{
  MString FileName = GetTemporaryFileName("revan.cfg");
  MSettingsRevan S;
  DisableDefaultStreams();
  S.Write(FileName);
  EnableDefaultStreams();
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleRevan::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // A crash is a possible outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Time-out of the child process in seconds
    const unsigned int ChildTimeout = 120;
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


bool UTNModuleRevan::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleRevan M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Event reconstruction (Revan)'", M.GetName(), MString("Event reconstruction (Revan)")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagRevan'", M.GetXmlTag(), MString("XmlTagRevan")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_EventReconstruction", M.ProvidesModuleType(MAssembly::c_EventReconstruction)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The strip pairing is required", M.IsHardPreceedingModule(MAssembly::c_StripPairing)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The depth correction is required", M.IsHardPreceedingModule(MAssembly::c_DepthCorrection)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The position determination is required", M.IsHardPreceedingModule(MAssembly::c_PositionDetermiation)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = Evaluate("GetRevanConfigurationFileName()", "default", "The default file name is empty", M.GetRevanConfigurationFileName(), MString("")) && Passed;

  MModuleRevan* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a non-null object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  // Finalize without Initialize is harmless
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleRevan::TestConfiguration()
{
  bool Passed = true;

  MModuleRevan Writer;
  Writer.SetRevanConfigurationFileName("/data/revan.cfg");
  Passed = Evaluate("SetRevanConfigurationFileName/GetRevanConfigurationFileName", "/data/revan.cfg", "Returns the set name", Writer.GetRevanConfigurationFileName(), MString("/data/revan.cfg")) && Passed;
  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleRevan Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "file name", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "RevanConfigurationFileName", "The file name survives", Reader.GetRevanConfigurationFileName(), MString("/data/revan.cfg")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleRevan::TestInitializeErrors()
{
  bool Passed = true;

  // Without geometry, Initialize must fail gracefully
  MString Config = WriteRevanConfiguration();
  bool NoGeometry = RunInChild([Config]() -> bool {
    MModuleRevan M;
    M.SetRevanConfigurationFileName(Config);
    return M.Initialize() == false;
  });
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "no geometry", "Initialize returns false without a geometry and without crashing", NoGeometry)) && Passed;

  // A missing revan configuration must not be silently replaced by the defaults
  MDGeometryQuest* G = GetGeometry();
  Passed = EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", G != nullptr) && Passed;
  if (G != nullptr) {
    MModuleRevan M;
    M.SetGeometry(G);
    M.SetRevanConfigurationFileName(GetTemporaryFileName("missing.revan.cfg"));
    int OldVerbosity = g_Verbosity;
    g_Verbosity = c_Quiet;
    DisableDefaultStreams();
    bool Result = M.Initialize();
    EnableDefaultStreams();
    g_Verbosity = OldVerbosity;
    Passed = ExpectedToFail(EvaluateFalse("Initialize()", "missing revan configuration file", "Initialize returns false", Result)) && Passed;
    M.Finalize();
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleRevan::TestReconstruction()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  if (G == nullptr) return EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", false);

  MModuleRevan M;
  M.SetGeometry(G);
  M.SetRevanConfigurationFileName(WriteRevanConfiguration());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool Init = M.Initialize();
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("Initialize()", "default revan configuration", "Initialize succeeds", Init) && Passed;
  if (Init == false) return false;

  // One hit: a photo-absorption event
  MReadOutAssembly E;
  E.SetID(5);
  MHit* H = new MHit();
  H->SetPosition(MVector(0.1, 0.2, 0.3));
  H->SetPositionResolution(MVector(0.05, 0.05, 0.05));
  H->SetEnergy(662.0);
  H->SetEnergyResolution(1.0);
  E.AddHit(H);
  DisableDefaultStreams();
  bool Result = M.AnalyzeEvent(&E);
  EnableDefaultStreams();
  Passed = EvaluateTrue("AnalyzeEvent()", "one hit", "AnalyzeEvent returns true", Result) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "one hit", "The c_EventReconstruction analysis progress is set", E.HasAnalysisProgress(MAssembly::c_EventReconstruction)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "one hit", "A physical event is created", E.GetPhysicalEvent() != nullptr) && Passed;
  if (E.GetPhysicalEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "one hit", "The physical event is a photo-absorption event", E.GetPhysicalEvent()->GetType(), MPhysicalEvent::c_Photo) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "one hit", "The physical event has the hit energy", E.GetPhysicalEvent()->Ei(), 662.0, 1e-3) && Passed;
  }
  Passed = EvaluateFalse("AnalyzeEvent()", "one hit", "No reconstruction error is set", E.HasEventReconstructionError()) && Passed;

  // Two separated hits of 100 and 562 keV: a Compton event with only one kinematically allowed sequence
  // (scattering first in the 562 keV hit would require a scattered photon above the 184 keV minimum for 662 keV)
  MReadOutAssembly C;
  C.SetID(7);
  // Simulated start of the photon (as streamed for cosima input): position (0, 0, 50) cm, direction -z, 662 keV
  MSimIA Start;
  Start.SetProcess("INIT");
  Start.SetID(1);
  Start.SetOriginID(0);
  Start.SetPosition(MVector(0.0, 0.0, 50.0));
  Start.SetSecondaryParticleID(1);
  Start.SetSecondaryDirection(MVector(0.0, 0.0, -1.0));
  Start.SetSecondaryPolarization(MVector(1.0, 0.0, 0.0));
  Start.SetSecondaryEnergy(662.0);
  C.AddSimIA(Start);
  vector<double> Deposits = { 100.0, 562.0 };
  vector<MVector> Positions = { MVector(-1.5, 0.0, 0.3), MVector(1.5, 0.0, -0.3) };
  for (unsigned int h = 0; h < Deposits.size(); ++h) {
    MHit* CH = new MHit();
    CH->SetPosition(Positions[h]);
    CH->SetPositionResolution(MVector(0.05, 0.05, 0.05));
    CH->SetEnergy(Deposits[h]);
    CH->SetEnergyResolution(1.0);
    C.AddHit(CH);
  }
  DisableDefaultStreams();
  bool CResult = M.AnalyzeEvent(&C);
  EnableDefaultStreams();
  Passed = EvaluateTrue("AnalyzeEvent()", "two hits", "AnalyzeEvent returns true", CResult) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "two hits", "No reconstruction error is set", C.HasEventReconstructionError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "two hits", "A physical event is created", C.GetPhysicalEvent() != nullptr) && Passed;
  if (C.GetPhysicalEvent() != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "two hits", "The physical event is a Compton event", C.GetPhysicalEvent()->GetType(), MPhysicalEvent::c_Compton) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "two hits", "The physical event has the total energy 662 keV", C.GetPhysicalEvent()->Ei(), 662.0, 1e-3) && Passed;
    MComptonEvent* CE = dynamic_cast<MComptonEvent*>(C.GetPhysicalEvent());
    if (CE != nullptr) {
      Passed = EvaluateNear("AnalyzeEvent()", "two hits", "The electron energy is the 100 keV deposit (the only allowed sequence)", CE->Ee(), 100.0, 1e-3) && Passed;
    }
    // MModuleRevan enables SetSaveOI(true): the simulated origin must reach the physical event
    Passed = ExpectedToFail(EvaluateVectorNear("AnalyzeEvent()", "two hits with simulated INIT interaction", "The origin position of the simulation is kept (OI)", C.GetPhysicalEvent()->GetOIPosition(), MVector(0.0, 0.0, 50.0), 1e-6)) && Passed;
    Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "two hits with simulated INIT interaction", "The origin energy of the simulation is kept (OI)", C.GetPhysicalEvent()->GetOIEnergy(), 662.0, 1e-6)) && Passed;
  }

  // No hits: a reconstruction error
  MReadOutAssembly N;
  N.SetID(6);
  DisableDefaultStreams();
  M.AnalyzeEvent(&N);
  EnableDefaultStreams();
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "no hits", "A reconstruction error is set", N.HasEventReconstructionError())) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "no hits", "No physical event is created", N.GetPhysicalEvent() == nullptr)) && Passed;
  if (N.GetPhysicalEvent() != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "no hits after a 662 keV event", "The physical event is not the one of the previous event", N.GetPhysicalEvent()->Ei(), 0.0, 1e-9) && Passed;
  }

  M.Finalize();

  // Re-initialization works
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool Again = M.Initialize();
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("Initialize()", "after Finalize", "A second Initialize succeeds", Again) && Passed;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNModuleRevan", "Unit tests for MModuleRevan") == false) return 1;

  UTNModuleRevan Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleRevan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
