/*
 * UTNModuleResponseGenerator.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdio>
#include <functional>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// ROOT libs:
#include "TSystem.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MDGeometryQuest.h"
#include "MSettingsRevan.h"
#include "MSettingsMimrec.h"
#include "MResponseMatrixON.h"
#include "MSimIA.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleResponseGenerator.h"
#include "MReadOutAssembly.h"
#include "MHit.h"


//! Unit test class for MModuleResponseGenerator
class UTNModuleResponseGenerator : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleResponseGenerator() : MUnitTest("UTNModuleResponseGenerator") {}
  //! Default destructor
  virtual ~UTNModuleResponseGenerator() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and the defaults
  bool TestDefaultConstruction();
  //! Test the setters/getters and the XML round trip
  bool TestConfiguration();
  //! Test the lifecycle error paths
  bool TestLifecycleErrors();
  //! Test that a configured module accepts an event
  bool TestAcceptEvent();

  //! Write a one-detector test geometry and return its file name
  MString WriteGeometry();
  //! Initialize a spectral response generator in the current working directory; returns false on failure
  bool InitializeSpectral(MModuleResponseGenerator& M, MDGeometryQuest* Geometry, const MString& RevanFile, const MString& MimrecFile);

  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestConfiguration() && Passed;
  Passed = TestLifecycleErrors() && Passed;
  Passed = TestAcceptEvent() && Passed;

  // Not covered: the content of a generated response, which needs many simulated events.
  // ShowOptionsGUI() opens a GUI and is not tested.

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
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


bool UTNModuleResponseGenerator::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleResponseGenerator M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Response creator'", M.GetName(), MString("Response creator")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagResponseGenerator'", M.GetXmlTag(), MString("XmlTagResponseGenerator")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_ResponseGeneration", M.ProvidesModuleType(MAssembly::c_ResponseGeneration)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The position determination is required", M.IsHardPreceedingModule(MAssembly::c_PositionDetermiation)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = Evaluate("GetMode()", "default", "The default mode is c_Spectrum", M.GetMode(), MModuleResponseGenerator::c_Spectrum) && Passed;
  Passed = Evaluate("GetResponseName()", "default", "The default response name is 'Response'", M.GetResponseName(), MString("Response")) && Passed;
  Passed = Evaluate("GetRevanConfigurationFileName()", "default", "The default revan file name is empty", M.GetRevanConfigurationFileName(), MString("")) && Passed;
  Passed = Evaluate("GetMimrecConfigurationFileName()", "default", "The default mimrec file name is empty", M.GetMimrecConfigurationFileName(), MString("")) && Passed;

  Passed = Evaluate("Mode constants", "c_Spectrum", "c_Spectrum is 0", MModuleResponseGenerator::c_Spectrum, 0u) && Passed;
  Passed = Evaluate("Mode constants", "c_Clustering", "c_Clustering is 1", MModuleResponseGenerator::c_Clustering, 1u) && Passed;
  Passed = Evaluate("Mode constants", "c_BayesianER", "c_BayesianER is 2", MModuleResponseGenerator::c_BayesianER, 2u) && Passed;
  Passed = Evaluate("Mode constants", "c_Efficiency", "c_Efficiency is 3", MModuleResponseGenerator::c_Efficiency, 3u) && Passed;
  Passed = Evaluate("Mode constants", "c_Imaging", "c_Imaging is 4", MModuleResponseGenerator::c_Imaging, 4u) && Passed;

  MModuleResponseGenerator* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a non-null object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::TestConfiguration()
{
  bool Passed = true;

  MModuleResponseGenerator Writer;
  Writer.SetMode(MModuleResponseGenerator::c_Imaging);
  Writer.SetResponseName("MyResponse");
  Writer.SetRevanConfigurationFileName("/data/revan.cfg");
  Writer.SetMimrecConfigurationFileName("/data/mimrec.cfg");
  Passed = Evaluate("SetMode/GetMode", "c_Imaging", "Returns c_Imaging", Writer.GetMode(), MModuleResponseGenerator::c_Imaging) && Passed;
  Passed = Evaluate("SetResponseName/GetResponseName", "MyResponse", "Returns the set name", Writer.GetResponseName(), MString("MyResponse")) && Passed;
  Passed = Evaluate("SetRevanConfigurationFileName/GetRevanConfigurationFileName", "/data/revan.cfg", "Returns the set name", Writer.GetRevanConfigurationFileName(), MString("/data/revan.cfg")) && Passed;
  Passed = Evaluate("SetMimrecConfigurationFileName/GetMimrecConfigurationFileName", "/data/mimrec.cfg", "Returns the set name", Writer.GetMimrecConfigurationFileName(), MString("/data/mimrec.cfg")) && Passed;

  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleResponseGenerator Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "all values", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "Mode", "The mode survives", Reader.GetMode(), MModuleResponseGenerator::c_Imaging) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "Name", "The response name survives", Reader.GetResponseName(), MString("MyResponse")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "RevanConfigurationFileName", "The revan file name survives", Reader.GetRevanConfigurationFileName(), MString("/data/revan.cfg")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "MimrecConfigurationFileName", "The mimrec file name survives", Reader.GetMimrecConfigurationFileName(), MString("/data/mimrec.cfg")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::TestLifecycleErrors()
{
  bool Passed = true;

  bool InvalidMode = RunInChild([]() -> bool {
    MModuleResponseGenerator M;
    M.SetMode(7);
    return M.Initialize() == false;
  });
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "invalid mode 7", "Initialize returns false for an invalid mode without crashing", InvalidMode)) && Passed;

  bool NoGeometry = RunInChild([]() -> bool {
    MModuleResponseGenerator M;
    M.SetMode(MModuleResponseGenerator::c_Spectrum);
    return M.Initialize() == false;
  });
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "no geometry", "Initialize returns false without a geometry and without crashing", NoGeometry)) && Passed;

  bool FinalizeOnly = RunInChild([]() -> bool {
    MModuleResponseGenerator M;
    M.Finalize();
    return true;
  });
  Passed = ExpectedToFail(EvaluateTrue("Finalize()", "without Initialize", "Finalize without Initialize does not crash", FinalizeOnly)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleResponseGenerator::WriteGeometry()
{
  // One 8 x 8 x 1.5 cm germanium strip detector
  MString G;
  G += "Name UTNResponseGeneratorGeometry\n";
  G += "SurroundingSphere 20 0.0 0.0 0.0 20\nShowSurroundingSphere False\n\n";
  G += "Material Vacuum\nVacuum.Density 1E-12\nVacuum.Component H 1\n\n";
  G += "Material Germanium\nGermanium.Density 5.323\nGermanium.ComponentByMass Ge 1\n\n";
  G += "Volume WorldVolume\nWorldVolume.Material Vacuum\nWorldVolume.Visibility 0\nWorldVolume.Shape Sphere 0.0 100\nWorldVolume.Mother 0\n\n";
  G += "Volume Wafer\nWafer.Material Germanium\nWafer.Shape BRIK 4.0 4.0 0.75\nWafer.Position 0.0 0.0 0.0\nWafer.Mother WorldVolume\n\n";
  G += "MDStrip3D GeD_0\nGeD_0.DetectorVolume Wafer\nGeD_0.SensitiveVolume Wafer\nGeD_0.Offset 0.0 0.0\nGeD_0.StripNumber 64 64\n";
  G += "GeD_0.NoiseThreshold 15\nGeD_0.TriggerThreshold 30\nGeD_0.DepthResolution 100 0.02\nGeD_0.DepthResolution 1000 0.02\n";
  G += "GeD_0.EnergyResolution Gaus 100 100 1\nGeD_0.EnergyResolution Gaus 1000 1000 2\nGeD_0.FailureRate 0.0\n\n";
  G += "Trigger T1\nT1.Veto false\nT1.TriggerByChannel true\nT1.Detector GeD_0 1\n";

  MString FileName = GetTemporaryFileName("response.geo.setup");
  WriteTextFile(FileName, G);
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::InitializeSpectral(MModuleResponseGenerator& M, MDGeometryQuest* Geometry, const MString& RevanFile, const MString& MimrecFile)
{
  M.SetGeometry(Geometry);
  M.SetMode(MModuleResponseGenerator::c_Spectrum);
  M.SetResponseName("UTNResponse");
  M.SetRevanConfigurationFileName(RevanFile);
  M.SetMimrecConfigurationFileName(MimrecFile);
  return M.Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleResponseGenerator::TestAcceptEvent()
{
  bool Passed = true;

  MDGeometryQuest* Geometry = new MDGeometryQuest();
  DisableDefaultStreams();
  bool Loaded = Geometry->ScanSetupFile(WriteGeometry(), false, false, false);
  EnableDefaultStreams();
  Passed = EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", Loaded) && Passed;
  if (Loaded == false) {
    delete Geometry;
    return false;
  }

  MString RevanFile = GetTemporaryFileName("response.revan.cfg");
  MString MimrecFile = GetTemporaryFileName("response.mimrec.cfg");
  MSettingsRevan RevanSettings;
  MSettingsMimrec MimrecSettings;
  DisableDefaultStreams();
  RevanSettings.Write(RevanFile);
  MimrecSettings.Write(MimrecFile);
  EnableDefaultStreams();

  MString OutputFile = GetTemporaryFileName("response.output.txt");

  bool Init = RunInChild([&]() -> bool {
    MModuleResponseGenerator M;
    return InitializeSpectral(M, Geometry, RevanFile, MimrecFile);
  });
  Passed = EvaluateTrue("Initialize()", "spectral mode with geometry, revan and mimrec configuration", "Initialize succeeds", Init) && Passed;

  if (Init == true) {
    // The module reports a rejected event on stdout ("Unable to set event"); capture stdout of the child process
    bool Accepted = RunInChild([&]() -> bool {
      MModuleResponseGenerator M;
      if (InitializeSpectral(M, Geometry, RevanFile, MimrecFile) == false) return false;
      if (freopen(OutputFile.Data(), "w", stdout) == nullptr) return false;
      g_Verbosity = c_Error;
      MReadOutAssembly E;
      E.SetID(1);
      E.SetTimeUTC(MTime(10.0));
      // The spectral response needs the simulated start of the photon: position (0, 0, 50) cm, direction -z, 662 keV
      MSimIA Start;
      Start.SetProcess("INIT");
      Start.SetID(1);
      Start.SetOriginID(0);
      Start.SetPosition(MVector(0.0, 0.0, 50.0));
      Start.SetSecondaryParticleID(1);
      Start.SetSecondaryDirection(MVector(0.0, 0.0, -1.0));
      Start.SetSecondaryPolarization(MVector(1.0, 0.0, 0.0));
      Start.SetSecondaryEnergy(662.0);
      E.AddSimIA(Start);
      MHit* H = new MHit();
      H->SetPosition(MVector(0.1, 0.2, 0.3));
      H->SetPositionResolution(MVector(0.05, 0.05, 0.05));
      H->SetEnergy(662.0);
      H->SetEnergyResolution(1.0);
      E.AddHit(H);
      bool Analyzed = M.AnalyzeEvent(&E);
      // Saves the response into the working directory
      M.Finalize();
      cout.flush();
      fflush(stdout);
      return Analyzed;
    });
    MString Output = ReadTextFile(OutputFile);
    Passed = EvaluateTrue("AnalyzeEvent()", "one 662 keV hit", "AnalyzeEvent completes and returns true", Accepted) && Passed;
    Passed = ExpectedToFail(EvaluateFalse("AnalyzeEvent()", "one 662 keV hit", "The event is accepted by the response builder (no 'Unable to set event')", Output.Contains("Unable to set event"))) && Passed;

    // The response is saved into the working directory, which MUnitTest points to its private temporary directory
    MString ResponseFile = MString(gSystem->WorkingDirectory()) + "/UTNResponse.energy.beforeeventreconstruction.rsp.gz";
    MResponseMatrixON Response;
    DisableDefaultStreams();
    bool Read = Response.Read(ResponseFile);
    EnableDefaultStreams();
    Passed = EvaluateTrue("Finalize()", "one 662 keV hit", "The energy response before event reconstruction is saved and readable", Read) && Passed;
    if (Read == true) {
      Passed = ExpectedToFail(EvaluateNear("Finalize()", "one 662 keV hit", "The energy response contains the analyzed event", Response.GetSum(), 1.0, 1e-9)) && Passed;
    }
  }

  delete Geometry;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNModuleResponseGenerator", "Unit tests for MModuleResponseGenerator") == false) return 1;

  UTNModuleResponseGenerator Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleResponseGenerator.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
