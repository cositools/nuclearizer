/*
 * UTNSubModuleChargeTransport.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <algorithm>
#include <functional>
#include <iostream>
#include <list>
#include <sstream>
#include <iomanip>
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

// Nuclearizer:
#include "MSubModuleChargeTransport.h"
#include "MReadOutAssembly.h"
#include "MDEEStripHit.h"


//! Unit test class for MSubModuleChargeTransport
class UTNSubModuleChargeTransport : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleChargeTransport() : MUnitTest("UTNSubModuleChargeTransport"), m_Geometry(nullptr) {}
  //! Default destructor
  virtual ~UTNSubModuleChargeTransport() { delete m_Geometry; }

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the setters/getters and the XML configuration
  bool TestConfiguration();
  //! Test the Initialize() error paths
  bool TestInitializeErrors();
  //! Test a hit in the center of a strip
  bool TestStripCenter();
  //! Test a hit exactly between two strips
  bool TestStripGap();
  //! Test hits at the detector edge and outside of the strips
  bool TestEdgeAndGuardRing();
  //! Test the drift time / fast peak time
  bool TestFastPeakTime();
  //! Test the merging of hits on the same strip
  bool TestMerging();

  //! Load the test geometry once, return nullptr on failure
  MDGeometryQuest* GetGeometry();
  //! Load a geometry with the two detectors GeD_0 and GeD_1; the caller owns it, nullptr on failure
  MDGeometryQuest* GetTwoDetectorGeometry();
  //! Write the splines file with 4 columns (or less) and return its name
  MString WriteSplines(unsigned int Columns = 4);
  //! Write the coefficients file and return its name
  MString WriteCoeffs();
  //! Initialize a module with the standard files
  bool InitializeStandard(MSubModuleChargeTransport& M);
  //! Create a DEE strip hit at the given position in the detector
  MDEEStripHit CreateHit(bool IsLowVoltage, MVector Position, double Energy, int Origin = 1);
  //! Return the hit on the given strip or nullptr
  MDEEStripHit* FindStrip(list<MDEEStripHit>& Hits, unsigned int StripID);
  //! Sum all energies in a list
  double SumEnergy(list<MDEEStripHit>& Hits);
  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);

  //! The test geometry
  MDGeometryQuest* m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleChargeTransport::Run()
{
  bool Passed = true;

  Passed = TestConfiguration() && Passed;
  Passed = TestInitializeErrors() && Passed;
  Passed = TestStripCenter() && Passed;
  Passed = TestStripGap() && Passed;
  Passed = TestEdgeAndGuardRing() && Passed;
  Passed = TestFastPeakTime() && Passed;
  Passed = TestMerging() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest* UTNSubModuleChargeTransport::GetGeometry()
{
  if (m_Geometry != nullptr) return m_Geometry;

  // One 8 x 8 x 1.5 cm germanium detector with 64 strips per side: pitch 0.125 cm
  MString G;
  G += "Name UTNChargeTransportGeometry\n";
  G += "SurroundingSphere 20 0.0 0.0 0.0 20\nShowSurroundingSphere False\n\n";
  G += "Material Vacuum\nVacuum.Density 1E-12\nVacuum.Component H 1\n\n";
  G += "Material Germanium\nGermanium.Density 5.323\nGermanium.ComponentByMass Ge 1\n\n";
  G += "Volume WorldVolume\nWorldVolume.Material Vacuum\nWorldVolume.Visibility 0\nWorldVolume.Shape Sphere 0.0 100\nWorldVolume.Mother 0\n\n";
  G += "Volume Wafer\nWafer.Material Germanium\nWafer.Shape BRIK 4.0 4.0 0.75\nWafer.Position 0.0 0.0 0.0\nWafer.Mother WorldVolume\n\n";
  G += "MDStrip3D GeD_0\nGeD_0.DetectorVolume Wafer\nGeD_0.SensitiveVolume Wafer\nGeD_0.Offset 0.0 0.0\nGeD_0.StripNumber 64 64\n";
  G += "GeD_0.NoiseThreshold 15\nGeD_0.TriggerThreshold 30\nGeD_0.DepthResolution 100 0.02\nGeD_0.DepthResolution 1000 0.02\n";
  G += "GeD_0.EnergyResolution Gaus 100 100 1\nGeD_0.EnergyResolution Gaus 1000 1000 2\nGeD_0.FailureRate 0.0\n\n";
  G += "Trigger T1\nT1.Veto false\nT1.TriggerByChannel true\nT1.Detector GeD_0 1\n";

  MString FileName = GetTemporaryFileName("ct.geo.setup");
  WriteTextFile(FileName, G);

  MDGeometryQuest* Geo = new MDGeometryQuest();
  DisableDefaultStreams();
  bool Loaded = Geo->ScanSetupFile(FileName, false, false, false);
  EnableDefaultStreams();
  if (Loaded == false) {
    delete Geo;
    return nullptr;
  }
  Geo->ActivateNoising(false);
  m_Geometry = Geo;
  return m_Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest* UTNSubModuleChargeTransport::GetTwoDetectorGeometry()
{
  // Two of the standard detectors, GeD_0 and GeD_1, stacked in z
  MString G;
  G += "Name UTNChargeTransportGeometry2\n";
  G += "SurroundingSphere 20 0.0 0.0 0.0 20\nShowSurroundingSphere False\n\n";
  G += "Material Vacuum\nVacuum.Density 1E-12\nVacuum.Component H 1\n\n";
  G += "Material Germanium\nGermanium.Density 5.323\nGermanium.ComponentByMass Ge 1\n\n";
  G += "Volume WorldVolume\nWorldVolume.Material Vacuum\nWorldVolume.Visibility 0\nWorldVolume.Shape Sphere 0.0 100\nWorldVolume.Mother 0\n\n";
  G += "Volume Wafer0\nWafer0.Material Germanium\nWafer0.Shape BRIK 4.0 4.0 0.75\nWafer0.Position 0.0 0.0 -2.0\nWafer0.Mother WorldVolume\n\n";
  G += "Volume Wafer1\nWafer1.Material Germanium\nWafer1.Shape BRIK 4.0 4.0 0.75\nWafer1.Position 0.0 0.0 2.0\nWafer1.Mother WorldVolume\n\n";
  for (unsigned int d = 0; d < 2; ++d) {
    MString N = MString("GeD_") + d;
    G += MString("MDStrip3D ") + N + "\n" + N + ".DetectorVolume Wafer" + d + "\n" + N + ".SensitiveVolume Wafer" + d + "\n";
    G += N + ".Offset 0.0 0.0\n" + N + ".StripNumber 64 64\n" + N + ".NoiseThreshold 15\n" + N + ".TriggerThreshold 30\n";
    G += N + ".DepthResolution 100 0.02\n" + N + ".DepthResolution 1000 0.02\n";
    G += N + ".EnergyResolution Gaus 100 100 1\n" + N + ".EnergyResolution Gaus 1000 1000 2\n" + N + ".FailureRate 0.0\n\n";
    G += MString("Trigger T") + d + "\nT" + d + ".Veto false\nT" + d + ".TriggerByChannel true\nT" + d + ".Detector " + N + " 1\n\n";
  }

  MString FileName = GetTemporaryFileName("ct.2det.geo.setup");
  WriteTextFile(FileName, G);

  MDGeometryQuest* Geo = new MDGeometryQuest();
  DisableDefaultStreams();
  bool Loaded = Geo->ScanSetupFile(FileName, false, false, false);
  EnableDefaultStreams();
  if (Loaded == false) {
    delete Geo;
    return nullptr;
  }
  Geo->ActivateNoising(false);
  return Geo;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleChargeTransport::WriteSplines(unsigned int Columns)
{
  // depth, CTD, electron drift time = 200 * (0.75 - z), hole drift time = 200 * (z + 0.75)
  ostringstream S;
  S<<"### 0 1050 87 662\n";
  for (unsigned int i = 0; i <= 30; ++i) {
    double Z = -0.75 + 0.05*i;
    S<<setprecision(10)<<Z<<","<<200.0*Z;
    if (Columns >= 3) S<<","<<200.0*(0.75 - Z);
    if (Columns >= 4) S<<","<<200.0*(Z + 0.75);
    S<<"\n";
  }
  MString FileName = GetTemporaryFileName(MString("ct.splines.") + Columns + ".csv");
  // An empty name makes Initialize() fail, which the callers check
  if (WriteTextFile(FileName, S.str()) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleChargeTransport::WriteCoeffs()
{
  // Stretch 1 and offset 0 for all pixels
  MString C = "### 800 V 80 K 59.5 keV\n";
  for (unsigned int lv = 0; lv < 64; ++lv) {
    for (unsigned int hv = 0; hv < 64; ++hv) {
      C += MString("") + (100*lv + hv) + ",1.0,0.0,1.0,1.0\n";
    }
  }
  MString FileName = GetTemporaryFileName("ct.coeffs.csv");
  // An empty name makes Initialize() fail, which the callers check
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleChargeTransport::InitializeStandard(MSubModuleChargeTransport& M)
{
  MDGeometryQuest* G = GetGeometry();
  if (G == nullptr) return false;
  M.SetGeometry(G);
  M.SetDepthSplinesFileName(WriteSplines());
  M.SetDepthCoefficientsFileName(WriteCoeffs());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Result = M.Initialize();
  g_Verbosity = OldVerbosity;
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


MDEEStripHit UTNSubModuleChargeTransport::CreateHit(bool IsLowVoltage, MVector Position, double Energy, int Origin)
{
  MDEEStripHit H;
  H.m_ROE.SetDetectorID(0);
  H.m_ROE.IsLowVoltageStrip(IsLowVoltage);
  H.m_SimulatedPositionInDetector = Position;
  H.m_SimulatedPosition = Position;
  H.m_SimulatedEnergy = Energy;
  H.m_SimulatedOrigins = { Origin };
  return H;
}


////////////////////////////////////////////////////////////////////////////////


MDEEStripHit* UTNSubModuleChargeTransport::FindStrip(list<MDEEStripHit>& Hits, unsigned int StripID)
{
  for (MDEEStripHit& H : Hits) {
    if (H.m_ROE.GetStripID() == StripID) return &H;
  }
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


double UTNSubModuleChargeTransport::SumEnergy(list<MDEEStripHit>& Hits)
{
  double Sum = 0;
  for (MDEEStripHit& H : Hits) Sum += H.m_Energy;
  return Sum;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleChargeTransport::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // A crash is a possible outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Kill a hanging child process after this time
    const unsigned int ChildTimeoutSeconds = 60;
    alarm(ChildTimeoutSeconds);
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


bool UTNSubModuleChargeTransport::TestConfiguration()
{
  bool Passed = true;

  MSubModuleChargeTransport M;
  M.SetDepthSplinesFileName("s.csv");
  M.SetDepthCoefficientsFileName("c.csv");
  Passed = Evaluate("SetDepthSplinesFileName/GetDepthSplinesFileName", "s.csv", "Returns the set name", M.GetDepthSplinesFileName(), MString("s.csv")) && Passed;
  Passed = Evaluate("SetDepthCoefficientsFileName/GetDepthCoefficientsFileName", "c.csv", "Returns the set name", M.GetDepthCoefficientsFileName(), MString("c.csv")) && Passed;

  // The file names are stored by MModuleDEESMEX, the sub module adds nothing
  MXmlNode* Node = new MXmlNode(nullptr, "DEE");
  Passed = EvaluateTrue("CreateXmlConfiguration()", "mother node", "The mother node is returned", M.CreateXmlConfiguration(Node) == Node) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "mother node", "ReadXmlConfiguration returns true", M.ReadXmlConfiguration(Node)) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleChargeTransport::TestInitializeErrors()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  Passed = EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", G != nullptr) && Passed;
  if (G == nullptr) return false;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MSubModuleChargeTransport MissingSplines;
  MissingSplines.SetGeometry(G);
  MissingSplines.SetDepthSplinesFileName(GetTemporaryFileName("missing.csv"));
  MissingSplines.SetDepthCoefficientsFileName(WriteCoeffs());
  bool MissingSplinesResult = MissingSplines.Initialize();

  MSubModuleChargeTransport TwoColumns;
  TwoColumns.SetGeometry(G);
  TwoColumns.SetDepthSplinesFileName(WriteSplines(2));
  TwoColumns.SetDepthCoefficientsFileName(WriteCoeffs());
  bool TwoColumnsResult = TwoColumns.Initialize();

  MSubModuleChargeTransport MissingCoeffs;
  MissingCoeffs.SetGeometry(G);
  MissingCoeffs.SetDepthSplinesFileName(WriteSplines());
  MissingCoeffs.SetDepthCoefficientsFileName(GetTemporaryFileName("missing.coeffs.csv"));
  bool MissingCoeffsResult = MissingCoeffs.Initialize();

  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "missing splines file", "Initialize returns false", MissingSplinesResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "splines without drift time columns", "Initialize returns false", TwoColumnsResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing coefficients file", "Initialize returns false", MissingCoeffsResult) && Passed;

  MSubModuleChargeTransport Good;
  Passed = EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", InitializeStandard(Good)) && Passed;
  Good.Finalize();

  // Without geometry, Initialize must fail gracefully
  bool NoGeometry = RunInChild([this]() -> bool {
    MSubModuleChargeTransport M;
    M.SetDepthSplinesFileName(WriteSplines());
    M.SetDepthCoefficientsFileName(WriteCoeffs());
    return M.Initialize() == false;
  });
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "no geometry", "Initialize returns false without a geometry and without crashing", NoGeometry)) && Passed;

  // Two detectors with depth coefficients, but drift splines only for detector 0
  MDGeometryQuest* TwoDetectors = GetTwoDetectorGeometry();
  Passed = EvaluateTrue("ScanSetupFile()", "two detectors", "The two-detector test geometry can be loaded", TwoDetectors != nullptr) && Passed;
  if (TwoDetectors != nullptr) {
    MString Splines = WriteSplines();
    MString Coeffs = GetTemporaryFileName("ct.coeffs.2det.csv");
    MString C = ReadTextFile(WriteCoeffs());
    for (unsigned int lv = 0; lv < 64; ++lv) {
      for (unsigned int hv = 0; hv < 64; ++hv) {
        C += MString("") + (10000 + 100*lv + hv) + ",1.0,0.0,1.0,1.0\n";
      }
    }
    Passed = EvaluateTrue("WriteTextFile()", "two-detector coefficients", "The coefficients file for two detectors is written", WriteTextFile(Coeffs, C)) && Passed;
    bool MissingSpline = RunInChild([this, TwoDetectors, Splines, Coeffs]() -> bool {
      MSubModuleChargeTransport M;
      M.SetGeometry(TwoDetectors);
      M.SetDepthSplinesFileName(Splines);
      M.SetDepthCoefficientsFileName(Coeffs);
      if (M.Initialize() == false) return true;
      MReadOutAssembly E;
      MDEEStripHit LV = CreateHit(true, MVector(0.0625, 0.1875, 0.0), 500.0);
      MDEEStripHit HV = CreateHit(false, MVector(0.0625, 0.1875, 0.0), 500.0);
      LV.m_ROE.SetDetectorID(1);
      HV.m_ROE.SetDetectorID(1);
      E.AddDEEStripHitLV(LV);
      E.AddDEEStripHitHV(HV);
      M.AnalyzeEvent(&E);
      return true;
    });
    Passed = ExpectedToFail(EvaluateTrue("Initialize()/AnalyzeEvent()", "hit in detector 1 without drift splines", "A detector without drift splines is rejected or skipped, without crashing", MissingSpline)) && Passed;
    delete TwoDetectors;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleChargeTransport::TestStripCenter()
{
  bool Passed = true;

  MSubModuleChargeTransport M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  // Strip 32 spans [0, 0.125] cm, its center is at 0.0625 cm; HV strip 33 spans [0.125, 0.25] cm
  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(true, MVector(0.0625, 0.1875, 0.0), 500.0));
  E.AddDEEStripHitHV(CreateHit(false, MVector(0.0625, 0.1875, 0.0), 500.0));
  Passed = EvaluateTrue("AnalyzeEvent()", "strip center", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;

  list<MDEEStripHit>& LV = E.GetDEEStripHitLVListReference();
  list<MDEEStripHit>& HV = E.GetDEEStripHitHVListReference();
  Passed = Evaluate("AnalyzeEvent()", "strip center LV", "Three LV hits are created (main + 2 neighbors)", (unsigned int) LV.size(), 3u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "strip center HV", "Three HV hits are created (main + 2 neighbors)", (unsigned int) HV.size(), 3u) && Passed;
  MDEEStripHit* Main = FindStrip(LV, 32);
  Passed = EvaluateTrue("AnalyzeEvent()", "strip center LV", "The main LV strip is 32", Main != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "strip center LV", "The neighbor strip 31 exists", FindStrip(LV, 31) != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "strip center LV", "The neighbor strip 33 exists", FindStrip(LV, 33) != nullptr) && Passed;
  if (Main != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "strip center LV", "Nearly all energy is on the main strip", Main->m_Energy, 500.0, 0.5) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "strip center LV", "The opposite strip is HV strip 33", Main->m_OppositeStripID, 33) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "strip center LV", "The main strip is not a guard ring", Main->m_IsGuardRing) && Passed;
  }
  Passed = EvaluateNear("AnalyzeEvent()", "strip center LV", "The energy is conserved", SumEnergy(LV), 500.0, 1e-3) && Passed;
  MDEEStripHit* MainHV = FindStrip(HV, 33);
  Passed = EvaluateTrue("AnalyzeEvent()", "strip center HV", "The main HV strip is 33", MainHV != nullptr) && Passed;
  if (MainHV != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "strip center HV", "Nearly all energy is on the main HV strip", MainHV->m_Energy, 500.0, 0.5) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "strip center HV", "The opposite strip is LV strip 32", MainHV->m_OppositeStripID, 32) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleChargeTransport::TestStripGap()
{
  bool Passed = true;

  MSubModuleChargeTransport M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  // x = 0 is exactly the gap between LV strips 31 and 32: the charge is shared equally
  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(true, MVector(0.0, 0.1875, 0.2), 600.0));
  Passed = EvaluateTrue("AnalyzeEvent()", "strip gap", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  list<MDEEStripHit>& LV = E.GetDEEStripHitLVListReference();
  MDEEStripHit* S31 = FindStrip(LV, 31);
  MDEEStripHit* S32 = FindStrip(LV, 32);
  Passed = EvaluateTrue("AnalyzeEvent()", "strip gap", "Strip 31 has a hit", S31 != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "strip gap", "Strip 32 has a hit", S32 != nullptr) && Passed;
  if (S31 != nullptr && S32 != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "strip gap", "Strip 31 gets half of the energy", S31->m_Energy, 300.0, 1.0) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "strip gap", "Strip 32 gets half of the energy", S32->m_Energy, 300.0, 1.0) && Passed;
  }
  Passed = EvaluateNear("AnalyzeEvent()", "strip gap", "The energy is conserved", SumEnergy(LV), 600.0, 1e-3) && Passed;
  for (MDEEStripHit& H : LV) {
    Passed = EvaluateTrue("AnalyzeEvent()", "strip gap", "No strip has negative energy", H.m_Energy >= 0.0) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleChargeTransport::TestEdgeAndGuardRing()
{
  bool Passed = true;

  MSubModuleChargeTransport M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  // First strip: the left neighbor is the guard ring
  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(true, MVector(-3.9375, 0.1875, 0.0), 500.0));
  Passed = EvaluateTrue("AnalyzeEvent()", "strip 0", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  list<MDEEStripHit>& LV = E.GetDEEStripHitLVListReference();
  Passed = EvaluateTrue("AnalyzeEvent()", "strip 0", "The main strip is 0", FindStrip(LV, 0) != nullptr) && Passed;
  MDEEStripHit* GR = FindStrip(LV, 64);
  Passed = EvaluateTrue("AnalyzeEvent()", "strip 0", "The left neighbor strip 64 exists", GR != nullptr) && Passed;
  if (GR != nullptr) {
    Passed = EvaluateTrue("AnalyzeEvent()", "strip 0", "The left neighbor strip 64 is the guard ring", GR->m_IsGuardRing) && Passed;
  }

  // Outside of the strip area along the strip: the full energy goes into the guard ring
  MReadOutAssembly O;
  O.AddDEEStripHitLV(CreateHit(true, MVector(0.0625, 4.05, 0.0), 400.0));
  Passed = EvaluateTrue("AnalyzeEvent()", "outside the strip area", "AnalyzeEvent returns true", M.AnalyzeEvent(&O)) && Passed;
  list<MDEEStripHit>& OLV = O.GetDEEStripHitLVListReference();
  Passed = Evaluate("AnalyzeEvent()", "outside the strip area", "One hit is created", (unsigned int) OLV.size(), 1u) && Passed;
  if (OLV.size() == 1) {
    Passed = EvaluateTrue("AnalyzeEvent()", "outside the strip area", "The hit is a guard ring hit", OLV.front().m_IsGuardRing) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "outside the strip area", "The guard ring has strip ID 64", OLV.front().m_ROE.GetStripID(), 64u) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "outside the strip area", "The guard ring gets the full energy", OLV.front().m_Energy, 400.0, 1e-9) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleChargeTransport::TestFastPeakTime()
{
  bool Passed = true;

  MSubModuleChargeTransport M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  // LV signals use the hole drift time 200 * (z + 0.75), HV signals the electron drift time 200 * (0.75 - z)
  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(true, MVector(0.0625, 0.1875, 0.5), 500.0));
  E.AddDEEStripHitHV(CreateHit(false, MVector(0.0625, 0.1875, 0.5), 500.0));
  Passed = EvaluateTrue("AnalyzeEvent()", "z = 0.5 cm", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  MDEEStripHit* LV = FindStrip(E.GetDEEStripHitLVListReference(), 32);
  MDEEStripHit* HV = FindStrip(E.GetDEEStripHitHVListReference(), 33);
  Passed = EvaluateTrue("AnalyzeEvent()", "z = 0.5 cm", "The LV strip 32 is hit", LV != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "z = 0.5 cm", "The HV strip 33 is hit", HV != nullptr) && Passed;
  if (LV != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "LV at z = 0.5 cm", "The LV fast peak time is the hole drift time 250 ns", LV->m_FastPeakTime, 250.0, 0.5) && Passed;
  }
  if (HV != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "HV at z = 0.5 cm", "The HV fast peak time is the electron drift time 50 ns", HV->m_FastPeakTime, 50.0, 0.5) && Passed;
  }

  // Clear() between events keeps the run configuration: the same event gives the same fast peak time
  M.Clear();
  MReadOutAssembly Next;
  Next.AddDEEStripHitLV(CreateHit(true, MVector(0.0625, 0.1875, 0.5), 500.0));
  Passed = EvaluateTrue("AnalyzeEvent()", "next event", "AnalyzeEvent returns true", M.AnalyzeEvent(&Next)) && Passed;
  MDEEStripHit* NextLV = FindStrip(Next.GetDEEStripHitLVListReference(), 32);
  Passed = EvaluateTrue("Clear()", "next event, LV at z = 0.5 cm", "The LV strip 32 is hit", NextLV != nullptr) && Passed;
  if (NextLV != nullptr) {
    Passed = EvaluateNear("Clear()", "next event, LV at z = 0.5 cm", "The LV fast peak time is still 250 ns", NextLV->m_FastPeakTime, 250.0, 0.5) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleChargeTransport::TestMerging()
{
  bool Passed = true;

  MSubModuleChargeTransport M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  // Two interactions in LV strip 32 at different depths: 100 keV at z = -0.5 cm (50 ns) and 400 keV at z = 0.5 cm (250 ns)
  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(true, MVector(0.0625, 0.1875, -0.5), 100.0, 3));
  E.AddDEEStripHitLV(CreateHit(true, MVector(0.0625, 1.1875, 0.5), 400.0, 7));
  Passed = EvaluateTrue("AnalyzeEvent()", "two interactions in strip 32", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  list<MDEEStripHit>& LV = E.GetDEEStripHitLVListReference();
  unsigned int N32 = 0;
  for (MDEEStripHit& H : LV) {
    if (H.m_ROE.GetStripID() == 32) ++N32;
  }
  Passed = Evaluate("AnalyzeEvent()", "two interactions in strip 32", "The hits on the same strip are merged", N32, 1u) && Passed;
  MDEEStripHit* S = FindStrip(LV, 32);
  if (S != nullptr) {
    Passed = EvaluateNear("AnalyzeEvent()", "two interactions in strip 32", "The merged energy is the sum", S->m_Energy, 500.0, 0.5) && Passed;
    Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "two interactions in strip 32", "The merged hit has the fast peak time of the more energetic interaction", S->m_FastPeakTime, 250.0, 0.5)) && Passed;
    bool HasBothOrigins = (find(S->m_SimulatedOrigins.begin(), S->m_SimulatedOrigins.end(), 3) != S->m_SimulatedOrigins.end()) && (find(S->m_SimulatedOrigins.begin(), S->m_SimulatedOrigins.end(), 7) != S->m_SimulatedOrigins.end());
    Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "two interactions in strip 32", "The merged hit keeps the origins of both interactions", HasBothOrigins)) && Passed;
  }
  Passed = EvaluateNear("AnalyzeEvent()", "two interactions in strip 32", "The total energy is conserved", SumEnergy(LV), 500.0, 1e-3) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleChargeTransport Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleChargeTransport.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
