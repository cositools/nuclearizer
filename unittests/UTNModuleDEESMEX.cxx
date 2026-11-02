/*
 * UTNModuleDEESMEX.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <iomanip>
#include <sstream>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MDGeometryQuest.h"
#include "MFileEventsSim.h"
#include "MSimEvent.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleDEESMEX.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MModuleDEESMEX
class UTNModuleDEESMEX : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleDEESMEX() : MUnitTest("UTNModuleDEESMEX"), m_Geometry(nullptr) {}
  //! Default destructor
  virtual ~UTNModuleDEESMEX() { delete m_Geometry; }

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and the defaults
  bool TestDefaultConstruction();
  //! Test that Clone() creates a distinct module carrying the geometry
  bool TestClone();
  //! Test the setters/getters including the pass-through to the sub modules
  bool TestGettersSetters();
  //! Test the XML round trip of all options
  bool TestXmlRoundTrip();
  //! Test the Initialize() error paths
  bool TestInitialize();
  //! Test the full chain for one germanium hit
  bool TestGermaniumHit();
  //! Test the shield veto
  bool TestShieldVeto();
  //! Test the guard ring veto
  bool TestGuardRingVeto();
  //! Test the GeD dead time
  bool TestDeadTime();

  //! Load the test geometry once, return nullptr on failure
  MDGeometryQuest* GetGeometry();
  //! Write all calibration files, return false if a file cannot be written
  bool WriteFiles();
  //! Configure a module with the geometry and all files
  void Configure(MModuleDEESMEX& M);
  //! Configure a module with all files, but not with the geometry
  void ConfigureFiles(MModuleDEESMEX& M);
  //! Read simulated events from a sim body (the lines after the header); the events have to be deleted by the caller
  vector<MSimEvent*> ReadSimEvents(const MString& Body);
  //! Return the strip hit with the given side and strip ID or nullptr
  MStripHit* FindStripHit(MReadOutAssembly& Event, bool IsLowVoltage, unsigned int StripID);
  //! Return the read-out ID of a strip of the test detector; strip 64 is the guard ring
  unsigned int GetReadOutID(bool IsLowVoltage, unsigned int StripID) const;

  //! The DIB the test detector (detector 0) sits on - deliberately different from the detector ID
  static constexpr unsigned int c_DIB = 5;

  //! The geometry file name
  MString m_GeometryFileName;
  //! The test geometry
  MDGeometryQuest* m_Geometry;
  //! The energy calibration file name
  MString m_EnergyCalibrationFileName;
  //! The hardware threshold file name
  MString m_ThresholdFileName;
  //! The depth splines file name
  MString m_SplinesFileName;
  //! The depth coefficients file name
  MString m_CoeffsFileName;
  //! The TAC calibration file name
  MString m_TACCalFileName;
  //! The strip map file name
  MString m_StripMapFileName;
  //! The shield energy correction file name
  MString m_ShieldCorrectionFileName;
  //! The dead time file name
  MString m_DeadtimeFileName;
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::Run()
{
  bool Passed = true;

  bool Written = WriteFiles();
  Passed = EvaluateTrue("WriteTextFile()", "calibration files", "All calibration files are written", Written) && Passed;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestClone() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestGermaniumHit() && Passed;
  Passed = TestShieldVeto() && Passed;
  Passed = TestGuardRingVeto() && Passed;
  Passed = TestDeadTime() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested
  // SetGeometryFileName() intentionally aborts the program and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest* UTNModuleDEESMEX::GetGeometry()
{
  if (m_Geometry != nullptr) return m_Geometry;

  MString G;
  G += "Name UTNDEESMEXGeometry\n";
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

  m_GeometryFileName = GetTemporaryFileName("deesmex.geo.setup");
  WriteTextFile(m_GeometryFileName, G);

  MDGeometryQuest* Geo = new MDGeometryQuest();
  DisableDefaultStreams();
  bool Loaded = Geo->ScanSetupFile(m_GeometryFileName, false, false, false);
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


bool UTNModuleDEESMEX::WriteFiles()
{
  bool Written = true;

  // Energy calibration: E = 0.5 * ADC for all strips of detector 0 including the guard ring (strip 64)
  MString E;
  for (unsigned int s = 0; s <= 64; ++s) {
    for (int Side = 0; Side < 2; ++Side) {
      MString S = (Side == 0) ? "l" : "h";
      E += MString("CP dss 0 ") + s + " " + S + " pakw 2 200 100 1 400 200 1\n";
      E += MString("CM dss 0 ") + s + " " + S + " poly1 0 0.5\n";
      E += MString("CR dss 0 ") + s + " " + S + " p1 1.0 0.002\n";
    }
  }
  m_EnergyCalibrationFileName = GetTemporaryFileName("deesmex.ecal");
  Written = WriteTextFile(m_EnergyCalibrationFileName, E) && Written;

  // Hardware threshold 40 ADC (20 keV)
  // The real files hold the DIB ID in the second column, but MModuleEnergyCalibration::ReadSlowThresholdCutFile(),
  // which also parses the hardware thresholds of the DEE, uses it as detector ID and ignores the read-out ID.
  // Thus the detector ID (0) is written here, unlike in the TAC calibration file below
  MString T = "STRIP_ID,Detector,side,strip,threshold_adc,threshold_keV\n";
  for (unsigned int s = 0; s <= 64; ++s) {
    T += MString("") + GetReadOutID(true, s) + ",0,l," + s + ",40,20.0\n";
    T += MString("") + GetReadOutID(false, s) + ",0,h," + s + ",40,20.0\n";
  }
  m_ThresholdFileName = GetTemporaryFileName("deesmex.thresholds.csv");
  Written = WriteTextFile(m_ThresholdFileName, T) && Written;

  // Splines: depth, CTD, electron drift 200 * (0.75 - z), hole drift 200 * (z + 0.75)
  ostringstream Sp;
  Sp<<"### 0 1050 87 662\n";
  for (unsigned int i = 0; i <= 30; ++i) {
    double Z = -0.75 + 0.05*i;
    Sp<<setprecision(10)<<Z<<","<<200.0*Z<<","<<200.0*(0.75 - Z)<<","<<200.0*(Z + 0.75)<<"\n";
  }
  m_SplinesFileName = GetTemporaryFileName("deesmex.splines.csv");
  Written = WriteTextFile(m_SplinesFileName, Sp.str()) && Written;

  // Depth coefficients: stretch 1, offset 0
  MString C = "### 800 V 80 K 59.5 keV\n";
  for (unsigned int lv = 0; lv < 64; ++lv) {
    for (unsigned int hv = 0; hv < 64; ++hv) {
      C += MString("") + (100*lv + hv) + ",1.0,0.0,1.0,1.0\n";
    }
  }
  m_CoeffsFileName = GetTemporaryFileName("deesmex.coeffs.csv");
  Written = WriteTextFile(m_CoeffsFileName, C) && Written;

  // Strip map of detector 0, which sits on DIB 5 - the read-out IDs encode the DIB, see GetReadOutID()
  MString Map = "# Columns are strip_id, detector, side (0=LV, 1=HV), strip\n";
  for (unsigned int s = 0; s < 64; ++s) {
    Map += MString("") + GetReadOutID(true, s) + " 0 0 " + s + "\n";
    Map += MString("") + GetReadOutID(false, s) + " 0 1 " + s + "\n";
  }
  m_StripMapFileName = GetTemporaryFileName("deesmex.stripmap.map");
  Written = WriteTextFile(m_StripMapFileName, Map) && Written;

  // TAC calibration: Timing = 0.5 * TAC - 500
  // The second column holds the DIB ID (5), not the detector ID (0): detector, side, and strip are taken from
  // the strip map via the read-out ID (see #239), which TestGermaniumHit() verifies through the detector ID of the hits
  MString TAC = "#STRIP_ID,DIB,Side,Strip,TAC_calibration,TAC_calibration_error,TAC_offset,TAC_offset_error\n";
  for (unsigned int s = 0; s < 64; ++s) {
    TAC += MString("") + GetReadOutID(true, s) + "," + c_DIB + ",0," + s + ",0.5,0.001,-500.0,1.0\n";
    TAC += MString("") + GetReadOutID(false, s) + "," + c_DIB + ",1," + s + ",0.5,0.001,-500.0,1.0\n";
  }
  m_TACCalFileName = GetTemporaryFileName("deesmex.taccal.csv");
  Written = WriteTextFile(m_TACCalFileName, TAC) && Written;

  // Shield energy correction for X0 crystal 3
  m_ShieldCorrectionFileName = GetTemporaryFileName("deesmex.shield.txt");
  Written = WriteTextFile(m_ShieldCorrectionFileName, "X0 3 1 1 1 0 0 1.0 0.0 1.0 0.0 0.0\n") && Written;

  // Dead times
  m_DeadtimeFileName = GetTemporaryFileName("deesmex.deadtime.txt");
  Written = WriteTextFile(m_DeadtimeFileName, "# strip\n1e-6 2e-6 3e-6 4e-6 5e-6\n# shield\n1e-6 5e-6 1e-6 2e-6 0.5e-6\n") && Written;

  return Written;
}


////////////////////////////////////////////////////////////////////////////////


void UTNModuleDEESMEX::Configure(MModuleDEESMEX& M)
{
  M.SetGeometry(GetGeometry());
  ConfigureFiles(M);
}


////////////////////////////////////////////////////////////////////////////////


void UTNModuleDEESMEX::ConfigureFiles(MModuleDEESMEX& M)
{
  M.SetEnergyCalibrationFileName(m_EnergyCalibrationFileName);
  M.SetHardwareThresholdFileName(m_ThresholdFileName);
  M.SetDepthSplinesFileName(m_SplinesFileName);
  M.SetDepthCoefficientsFileName(m_CoeffsFileName);
  M.SetTACCalFileName(m_TACCalFileName);
  M.SetStripMapFileName(m_StripMapFileName);
  M.SetShieldEnergyCorrectionFileName(m_ShieldCorrectionFileName);
  M.SetDeadtimeFileName(m_DeadtimeFileName);
  M.SetApplyResolutionCalibration(false);
  M.SetApplyTimingResolutionCalibration(false);
}


////////////////////////////////////////////////////////////////////////////////


vector<MSimEvent*> UTNModuleDEESMEX::ReadSimEvents(const MString& Body)
{
  vector<MSimEvent*> Events;
  MDGeometryQuest* G = GetGeometry();
  if (G == nullptr) return Events;

  MString S = MString("Version 101\nType SIM\nGeometry ") + m_GeometryFileName + "\n\nTB 0\n\n" + Body + "EN\n\nTE 100.0\nTS 1000\n";
  MString FileName = GetTemporaryFileName("deesmex.sim");
  WriteTextFile(FileName, S);

  MFileEventsSim Reader(G);
  DisableDefaultStreams();
  if (Reader.Open(FileName) == true) {
    MSimEvent* E = nullptr;
    while ((E = Reader.GetNextEvent(false)) != nullptr) Events.push_back(E);
    Reader.Close();
  }
  EnableDefaultStreams();
  return Events;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleDEESMEX::FindStripHit(MReadOutAssembly& Event, bool IsLowVoltage, unsigned int StripID)
{
  for (unsigned int i = 0; i < Event.GetNStripHits(); ++i) {
    MStripHit* SH = Event.GetStripHit(i);
    if (SH != nullptr && SH->IsLowVoltageStrip() == IsLowVoltage && SH->GetStripID() == StripID) return SH;
  }
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTNModuleDEESMEX::GetReadOutID(bool IsLowVoltage, unsigned int StripID) const
{
  // Encoding of MStripMap: strips are DIB << 7 | primary << 6 | ASIC << 5 | channel,
  // guard rings are 2048 + (DIB << 1 | primary), with the HV side as the primary ASIC section
  if (StripID == 64) return 2048 + (c_DIB << 1) + (IsLowVoltage == true ? 0 : 1);
  return (c_DIB << 7) + (IsLowVoltage == true ? 0 : 64) + StripID;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleDEESMEX M;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagDEESMEX'", M.GetXmlTag(), MString("XmlTagDEESMEX")) && Passed;
  Passed = EvaluateFalse("GetName()", "default", "The name is not empty", M.GetName().IsEmpty()) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_DetectorEffectsEngine", M.ProvidesModuleType(MAssembly::c_DetectorEffectsEngine)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "A simulation loader is required", M.IsHardPreceedingModule(MAssembly::c_EventLoaderSimulation)) && Passed;
  Passed = EvaluateFalse("IsStartModule()", "default", "The DEE is not a start module", M.IsStartModule()) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("GetApplyResolutionCalibration()", "default", "The energy resolution is applied by default", M.GetApplyResolutionCalibration()) && Passed;
  Passed = EvaluateTrue("GetApplyTimingResolutionCalibration()", "default", "The timing resolution is applied by default", M.GetApplyTimingResolutionCalibration()) && Passed;
  Passed = EvaluateTrue("GetEnableShieldVeto()", "default", "The shield veto is enabled by default", M.GetEnableShieldVeto()) && Passed;
  Passed = EvaluateTrue("GetEnableGuardRingVeto()", "default", "The guard ring veto is enabled by default", M.GetEnableGuardRingVeto()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::TestClone()
{
  bool Passed = true;

  // MModule has no geometry getter, thus the hand-over of the geometry is verified through
  // Initialize(), which fails without a geometry: the clones get all files, but no geometry

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();

  MModuleDEESMEX M;
  M.SetGeometry(GetGeometry());
  MModuleDEESMEX* C = M.Clone();
  bool CloneInitialized = false;
  if (C != nullptr) {
    ConfigureFiles(*C);
    CloneInitialized = C->Initialize();
    if (CloneInitialized == true) C->Finalize();
  }

  MModuleDEESMEX N;
  N.SetGeometry(nullptr);
  MModuleDEESMEX* CN = N.Clone();
  bool CloneWithoutGeometryInitialized = false;
  if (CN != nullptr) {
    ConfigureFiles(*CN);
    CloneWithoutGeometryInitialized = CN->Initialize();
    if (CloneWithoutGeometryInitialized == true) CN->Finalize();
  }

  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("Clone()", "with geometry", "Clone() returns a non-null object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "with geometry", "Clone() returns a distinct object", C != &M) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "with geometry", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  Passed = EvaluateTrue("Clone()", "with geometry", "The clone carries the geometry: Initialize() succeeds without setting it again", CloneInitialized) && Passed;
  Passed = EvaluateTrue("Clone()", "without geometry", "Clone() returns a non-null object", CN != nullptr) && Passed;
  Passed = EvaluateFalse("Clone()", "without geometry", "The clone has no geometry either: Initialize() fails", CloneWithoutGeometryInitialized) && Passed;

  delete C;
  delete CN;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::TestGettersSetters()
{
  bool Passed = true;

  MModuleDEESMEX M;
  M.SetEnergyCalibrationFileName("e.ecal");
  M.SetHardwareThresholdFileName("t.csv");
  M.SetDepthSplinesFileName("s.csv");
  M.SetDepthCoefficientsFileName("c.csv");
  M.SetTACCalFileName("tac.csv");
  M.SetStripMapFileName("m.map");
  M.SetShieldEnergyCorrectionFileName("sh.txt");
  M.SetDeadtimeFileName("dt.txt");
  Passed = Evaluate("SetEnergyCalibrationFileName/GetEnergyCalibrationFileName", "e.ecal", "Returns the set name", M.GetEnergyCalibrationFileName(), MString("e.ecal")) && Passed;
  Passed = Evaluate("SetHardwareThresholdFileName/GetHardwareThresholdFileName", "t.csv", "Returns the set name", M.GetHardwareThresholdFileName(), MString("t.csv")) && Passed;
  Passed = Evaluate("SetDepthSplinesFileName/GetDepthSplinesFileName", "s.csv", "Returns the set name", M.GetDepthSplinesFileName(), MString("s.csv")) && Passed;
  Passed = Evaluate("SetDepthCoefficientsFileName/GetDepthCoefficientsFileName", "c.csv", "Returns the set name", M.GetDepthCoefficientsFileName(), MString("c.csv")) && Passed;
  Passed = Evaluate("SetTACCalFileName/GetTACCalFileName", "tac.csv", "Returns the set name", M.GetTACCalFileName(), MString("tac.csv")) && Passed;
  Passed = Evaluate("SetStripMapFileName/GetStripMapFileName", "m.map", "Returns the set name", M.GetStripMapFileName(), MString("m.map")) && Passed;
  Passed = EvaluateTrue("GetStripMap()", "before Initialize()", "No strip map is available", M.GetStripMap() == nullptr) && Passed;
  Passed = Evaluate("SetShieldEnergyCorrectionFileName/GetShieldEnergyCorrectionFileName", "sh.txt", "Returns the set name", M.GetShieldEnergyCorrectionFileName(), MString("sh.txt")) && Passed;
  Passed = Evaluate("SetDeadtimeFileName/GetDeadtimeFileName", "dt.txt", "Returns the set name", M.GetDeadtimeFileName(), MString("dt.txt")) && Passed;
  M.SetApplyResolutionCalibration(false);
  M.SetApplyTimingResolutionCalibration(false);
  M.SetEnableShieldVeto(false);
  M.SetEnableGuardRingVeto(false);
  Passed = EvaluateFalse("SetApplyResolutionCalibration/GetApplyResolutionCalibration", "false", "Returns false", M.GetApplyResolutionCalibration()) && Passed;
  Passed = EvaluateFalse("SetApplyTimingResolutionCalibration/GetApplyTimingResolutionCalibration", "false", "Returns false", M.GetApplyTimingResolutionCalibration()) && Passed;
  Passed = EvaluateFalse("SetEnableShieldVeto/GetEnableShieldVeto", "false", "Returns false", M.GetEnableShieldVeto()) && Passed;
  Passed = EvaluateFalse("SetEnableGuardRingVeto/GetEnableGuardRingVeto", "false", "Returns false", M.GetEnableGuardRingVeto()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleDEESMEX Writer;
  Writer.SetEnergyCalibrationFileName("/d/e.ecal");
  Writer.SetHardwareThresholdFileName("/d/t.csv");
  Writer.SetDepthSplinesFileName("/d/s.csv");
  Writer.SetDepthCoefficientsFileName("/d/c.csv");
  Writer.SetTACCalFileName("/d/tac.csv");
  Writer.SetStripMapFileName("/d/m.map");
  Writer.SetShieldEnergyCorrectionFileName("/d/sh.txt");
  Writer.SetDeadtimeFileName("/d/dt.txt");
  Writer.SetApplyResolutionCalibration(false);
  Writer.SetApplyTimingResolutionCalibration(false);
  Writer.SetEnableShieldVeto(false);
  Writer.SetEnableGuardRingVeto(false);
  MXmlNode* Node = Writer.CreateXmlConfiguration();

  MModuleDEESMEX Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "all options", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "EnergyCalibrationFileName", "Survives", Reader.GetEnergyCalibrationFileName(), MString("/d/e.ecal")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "HardwareThresholdFileName", "Survives", Reader.GetHardwareThresholdFileName(), MString("/d/t.csv")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "DepthSplinesFileName", "Survives", Reader.GetDepthSplinesFileName(), MString("/d/s.csv")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "DepthCoefficientsFileName", "Survives", Reader.GetDepthCoefficientsFileName(), MString("/d/c.csv")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "TACCalFileName", "Survives", Reader.GetTACCalFileName(), MString("/d/tac.csv")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "StripMapFileName", "Survives", Reader.GetStripMapFileName(), MString("/d/m.map")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "ShieldEnergyCorrectionFileName", "Survives", Reader.GetShieldEnergyCorrectionFileName(), MString("/d/sh.txt")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "DeadtimeFileName", "Survives", Reader.GetDeadtimeFileName(), MString("/d/dt.txt")) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "ApplyResolutionCalibration", "Survives", Reader.GetApplyResolutionCalibration()) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "ApplyTimingResolutionCalibration", "Survives", Reader.GetApplyTimingResolutionCalibration()) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "EnableShieldVeto", "Survives", Reader.GetEnableShieldVeto()) && Passed;
  Passed = EvaluateFalse("ReadXmlConfiguration()", "EnableGuardRingVeto", "Survives", Reader.GetEnableGuardRingVeto()) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::TestInitialize()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();

  MModuleDEESMEX NoGeometry;
  Configure(NoGeometry);
  NoGeometry.SetGeometry(nullptr);
  bool NoGeometryResult = NoGeometry.Initialize();

  MModuleDEESMEX NoCalibration;
  Configure(NoCalibration);
  NoCalibration.SetEnergyCalibrationFileName(GetTemporaryFileName("missing.ecal"));
  bool NoCalibrationResult = NoCalibration.Initialize();

  MModuleDEESMEX NoShield;
  Configure(NoShield);
  NoShield.SetShieldEnergyCorrectionFileName(GetTemporaryFileName("missing.shield.txt"));
  bool NoShieldResult = NoShield.Initialize();

  MModuleDEESMEX NoStripMap;
  Configure(NoStripMap);
  NoStripMap.SetStripMapFileName("");
  bool NoStripMapResult = NoStripMap.Initialize();

  MModuleDEESMEX Good;
  Configure(Good);
  bool GoodResult = Good.Initialize();
  bool GoodHasStripMap = (Good.GetStripMap() != nullptr);
  if (GoodResult == true) Good.Finalize();

  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "no geometry", "Initialize returns false", NoGeometryResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing energy calibration", "Initialize returns false", NoCalibrationResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing shield correction", "Initialize returns false", NoShieldResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "no strip map", "Initialize returns false", NoStripMapResult) && Passed;
  Passed = EvaluateTrue("Initialize()", "all files", "Initialize succeeds", GoodResult) && Passed;
  Passed = EvaluateTrue("GetStripMap()", "after Initialize()", "The strip map of the depth readout is available", GoodHasStripMap) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::TestGermaniumHit()
{
  bool Passed = true;

  // 500 keV at the center of LV strip 32 / HV strip 33, z = 0
  vector<MSimEvent*> Sim = ReadSimEvents("SE\nID 1 1\nTI 12.5\nHTsim 3;0.0625;0.1875;0.0;500.0;0.0;1\n");
  if (Sim.size() != 1) {
    for (MSimEvent* S : Sim) delete S;
    return EvaluateTrue("MFileEventsSim::GetNextEvent()", "one event", "The simulated event can be read", false);
  }

  MModuleDEESMEX M;
  Configure(M);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool Init = M.Initialize();
  EnableDefaultStreams();
  if (Init == false) {
    g_Verbosity = OldVerbosity;
    delete Sim[0];
    return EvaluateTrue("Initialize()", "all files", "Initialize succeeds", false);
  }

  MReadOutAssembly E;
  E.SetSimulatedEvent(Sim[0]);
  DisableDefaultStreams();
  bool Result = M.AnalyzeEvent(&E);
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("AnalyzeEvent()", "one GeD hit", "AnalyzeEvent returns true", Result) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "one GeD hit", "The c_DetectorEffectsEngine analysis progress is set", E.HasAnalysisProgress(MAssembly::c_DetectorEffectsEngine)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one GeD hit", "The event ID is the simulated ID", E.GetID(), (unsigned long) 1) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "one GeD hit", "No shield veto", E.GetShieldVeto()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "one GeD hit", "No guard ring veto", E.GetGuardRingVeto()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one GeD hit", "Six strip hits are created (main strip and two neighbors per side)", E.GetNStripHits(), 6u) && Passed;

  MStripHit* LV = FindStripHit(E, true, 32);
  MStripHit* HV = FindStripHit(E, false, 33);
  Passed = EvaluateTrue("AnalyzeEvent()", "one GeD hit", "The LV strip 32 is hit", LV != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "one GeD hit", "The HV strip 33 is hit", HV != nullptr) && Passed;
  if (LV != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "LV strip 32", "The strip is in detector 0 from the strip map, not in DIB 5 of the TAC calibration file", LV->GetDetectorID(), 0u) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "LV strip 32", "The ADC value is 500 keV / 0.5 = 1000", LV->GetADCUnits(), 1000.0, 1.5) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "LV strip 32", "The strip is a triggered strip", LV->IsNearestNeighbor()) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "LV strip 32", "The strip has fast timing", LV->HasFastTiming()) && Passed;
    // Hole drift time at z = 0 is 150 ns => timing 4200 - 150 = 4050 ns => TAC (4050 + 500) / 0.5 = 9100
    Passed = EvaluateNear("AnalyzeEvent()", "LV strip 32", "The TAC value follows from the hole drift time", LV->GetTAC(), 9100.0, 2.0) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "LV strip 32", "The simulated origin is kept", (unsigned int) LV->GetOrigins().size(), 1u) && Passed;
  }
  if (HV != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "HV strip 33", "The strip is in detector 0 from the strip map, not in DIB 5 of the TAC calibration file", HV->GetDetectorID(), 0u) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "HV strip 33", "The ADC value is 1000", HV->GetADCUnits(), 1000.0, 1.5) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "HV strip 33", "The strip has fast timing", HV->HasFastTiming()) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::TestShieldVeto()
{
  bool Passed = true;

  vector<MSimEvent*> Sim = ReadSimEvents("SE\nID 1 1\nTI 20.0\nHTsim 3;0.0625;0.1875;0.0;500.0;0.0;1\nHTsim 8;0.5;0.5;10.5;300.0;0.0;1\n");
  if (Sim.size() != 1) {
    for (MSimEvent* S : Sim) delete S;
    return EvaluateTrue("MFileEventsSim::GetNextEvent()", "one event", "The simulated event can be read", false);
  }

  MModuleDEESMEX M;
  Configure(M);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool Init = M.Initialize();
  MReadOutAssembly E;
  E.SetSimulatedEvent(Sim[0]);
  if (Init == true) M.AnalyzeEvent(&E);
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("AnalyzeEvent()", "GeD and shield hit", "The shield veto is set", E.GetShieldVeto()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "GeD and shield hit", "The c_DetectorEffectsEngine analysis progress is set", E.HasAnalysisProgress(MAssembly::c_DetectorEffectsEngine)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "GeD and shield hit", "No strip hits are created for a vetoed event", E.GetNStripHits(), 0u) && Passed;

  // With the shield veto disabled the GeD hit is processed
  vector<MSimEvent*> Sim2 = ReadSimEvents("SE\nID 1 1\nTI 20.0\nHTsim 3;0.0625;0.1875;0.0;500.0;0.0;1\nHTsim 8;0.5;0.5;10.5;300.0;0.0;1\n");
  Passed = EvaluateSize("MFileEventsSim::GetNextEvent()", "GeD and shield hit", "One simulated event is read for the disabled-veto case", Sim2.size(), 1) && Passed;
  if (Sim2.size() == 1) {
    MModuleDEESMEX N;
    Configure(N);
    N.SetEnableShieldVeto(false);
    g_Verbosity = c_Quiet;
    DisableDefaultStreams();
    bool InitN = N.Initialize();
    MReadOutAssembly F;
    F.SetSimulatedEvent(Sim2[0]);
    if (InitN == true) N.AnalyzeEvent(&F);
    EnableDefaultStreams();
    g_Verbosity = OldVerbosity;
    Passed = EvaluateFalse("AnalyzeEvent()", "shield veto disabled", "The shield veto is not set", F.GetShieldVeto()) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "shield veto disabled", "Strip hits are created", F.GetNStripHits() > 0) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "shield veto disabled", "The shield hit is converted into a crystal hit", F.GetNCrystalHits(), 1u) && Passed;
    if (InitN == true) N.Finalize();
  }
  if (Sim2.size() != 1) {
    for (MSimEvent* S : Sim2) delete S;
  }

  if (Init == true) M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::TestGuardRingVeto()
{
  bool Passed = true;

  // Hit on the first LV strip: its left neighbor is the guard ring with a large share of the charge at the very edge
  vector<MSimEvent*> Sim = ReadSimEvents("SE\nID 1 1\nTI 30.0\nHTsim 3;-3.999;0.1875;0.0;500.0;0.0;1\n");
  if (Sim.size() != 1) {
    for (MSimEvent* S : Sim) delete S;
    return EvaluateTrue("MFileEventsSim::GetNextEvent()", "one event", "The simulated event can be read", false);
  }

  MModuleDEESMEX M;
  Configure(M);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool Init = M.Initialize();
  MReadOutAssembly E;
  E.SetSimulatedEvent(Sim[0]);
  if (Init == true) M.AnalyzeEvent(&E);
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("AnalyzeEvent()", "hit at the detector edge", "The guard ring veto is set", E.GetGuardRingVeto()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "hit at the detector edge", "The c_DetectorEffectsEngine analysis progress is set", E.HasAnalysisProgress(MAssembly::c_DetectorEffectsEngine)) && Passed;

  if (Init == true) M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDEESMEX::TestDeadTime()
{
  bool Passed = true;

  // Second event 3 us after the first one: the GeDs are dead (dead time ~ 1 + 2 + 7 us)
  vector<MSimEvent*> Sim = ReadSimEvents("SE\nID 1 1\nTI 40.0\nHTsim 3;0.0625;0.1875;0.0;500.0;0.0;1\nSE\nID 2 2\nTI 40.000003\nHTsim 3;1.0625;0.1875;0.0;500.0;0.0;2\n");
  if (Sim.size() != 2) {
    for (MSimEvent* S : Sim) delete S;
    return EvaluateTrue("MFileEventsSim::GetNextEvent()", "two events", "The simulated events can be read", false);
  }

  MModuleDEESMEX M;
  Configure(M);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool Init = M.Initialize();
  MReadOutAssembly E1;
  E1.SetSimulatedEvent(Sim[0]);
  MReadOutAssembly E2;
  E2.SetSimulatedEvent(Sim[1]);
  if (Init == true) {
    M.AnalyzeEvent(&E1);
    M.AnalyzeEvent(&E2);
  }
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("AnalyzeEvent()", "first event", "The first event has strip hits", E1.GetNStripHits() > 0) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "second event in dead time", "The second event has no strip hits", E2.GetNStripHits(), 0u) && Passed;

  if (Init == true) M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleDEESMEX Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleDEESMEX.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
