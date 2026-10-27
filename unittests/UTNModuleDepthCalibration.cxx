/*
 * UTNModuleDepthCalibration.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <cstring>
#include <functional>
#include <iomanip>
#include <map>
#include <new>
#include <sstream>
#include <unordered_map>
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
#include "MSupervisor.h"
#include "MDGeometryQuest.h"
#include "MReadOutElementDoubleStrip.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleDepthCalibration.h"
#include "MModuleEnergyCalibration.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MHit.h"


//! Exposes the protected members of MModuleDepthCalibration for direct testing
class UTNModuleDepthCalibrationAccess : public MModuleDepthCalibration
{
 public:
  //! Forward to the protected GetDepth()
  vector<double> CallGetDepth(int DetID) { return GetDepth(DetID); }
  //! Forward to the protected GetCTD()
  vector<double> CallGetCTD(int DetID, int Grade) { return GetCTD(DetID, Grade); }
  //! Forward to the protected GetSpline()
  TSpline3* CallGetSpline(int DetID, int Grade) { return GetSpline(DetID, Grade); }
  //! Forward to the protected norm_pdf()
  vector<double> CallNormPdf(vector<double> X, double Mu, double Sigma) { return norm_pdf(X, Mu, Sigma); }
  //! Forward to the protected GetPixelCoeffs()
  vector<double>* CallGetPixelCoeffs(int PixelCode) { return GetPixelCoeffs(PixelCode); }
  //! Forward to the protected LoadMaskMetrologyFile()
  bool CallLoadMaskMetrologyFile(MString FileName) { return LoadMaskMetrologyFile(FileName); }
  //! Forward to the protected GetStripIntersection()
  vector<double> CallGetStripIntersection(MReadOutElementDoubleStrip LV, MReadOutElementDoubleStrip HV) { return GetStripIntersection(LV, HV); }
  //! Forward to the protected GetTimingNoiseFWHM()
  double CallGetTimingNoiseFWHM(int PixelCode, double Energy) { return GetTimingNoiseFWHM(PixelCode, Energy); }
  //! Return the number of disabled strips
  unsigned int GetNDisabledStrips() const { return m_DisabledStrips.size(); }
  //! Return the number of shorted strips
  unsigned int GetNShortedStrips() const { return m_ShortedStrips.size(); }
  //! Return the number of known detectors
  unsigned int GetNDetectorIDs() const { return m_DetectorIDs.size(); }
  //! Return the raw byte of a bool member; reading an uninitialized bool as bool is undefined behavior, its byte is not
  unsigned int GetRawByte(const bool& Member) const { unsigned char Byte = 0; memcpy(&Byte, &Member, 1); return Byte; }
  //! Return the raw byte of m_AccountForDisabledStrips
  unsigned int GetRawAccountForDisabledStrips() const { return GetRawByte(m_AccountForDisabledStrips); }
  //! Return the raw byte of m_MaskMetrologyEnabled
  unsigned int GetRawMaskMetrologyEnabled() const { return GetRawByte(m_MaskMetrologyEnabled); }
  //! Return the raw byte of m_UCSDOverride
  unsigned int GetRawUCSDOverride() const { return GetRawByte(m_UCSDOverride); }
  //! Return the raw byte of m_CoeffsFileIsLoaded
  unsigned int GetRawCoeffsFileIsLoaded() const { return GetRawByte(m_CoeffsFileIsLoaded); }
  //! Return the raw byte of m_SplinesFileIsLoaded
  unsigned int GetRawSplinesFileIsLoaded() const { return GetRawByte(m_SplinesFileIsLoaded); }
  //! Return the raw byte of m_MaskMetrologyFileIsLoaded
  unsigned int GetRawMaskMetrologyFileIsLoaded() const { return GetRawByte(m_MaskMetrologyFileIsLoaded); }
  //! Return the thickness of a detector
  double GetThickness(int DetID) { return m_Thicknesses[DetID]; }
  //! Return the X pitch of a detector
  double GetXPitch(int DetID) { return m_XPitches[DetID]; }
  //! Return the Y pitch of a detector
  double GetYPitch(int DetID) { return m_YPitches[DetID]; }
  //! Return the number of X strips of a detector
  int GetNXStrips(int DetID) { return m_NXStrips[DetID]; }
  //! Return the number of Y strips of a detector
  int GetNYStrips(int DetID) { return m_NYStrips[DetID]; }
  //! Return whether the coefficients file is flagged as loaded
  bool IsCoeffsFileLoaded() const { return m_CoeffsFileIsLoaded; }
  //! Return whether the splines file is flagged as loaded
  bool IsSplinesFileLoaded() const { return m_SplinesFileIsLoaded; }
  //! Return whether the mask metrology file is flagged as loaded
  bool IsMaskMetrologyFileLoaded() const { return m_MaskMetrologyFileIsLoaded; }
  //! Flag the coefficients file as loaded
  void SetCoeffsFileLoaded(bool Flag) { m_CoeffsFileIsLoaded = Flag; }
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test class for MModuleDepthCalibration
class UTNModuleDepthCalibration : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleDepthCalibration() : MUnitTest("UTNModuleDepthCalibration"), m_Geometry(nullptr) {}
  //! Default destructor
  virtual ~UTNModuleDepthCalibration() { delete m_Geometry; }

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and the default values (including uninitialized members)
  bool TestDefaultConstruction();
  //! Test all setter/getter pairs
  bool TestGettersSetters();
  //! Test the XML configuration round trip
  bool TestXmlRoundTrip();
  //! Test GetDominantStrip()
  bool TestGetDominantStrip();
  //! Test GetHitGrade()
  bool TestGetHitGrade();
  //! Test norm_pdf()
  bool TestNormPdf();
  //! Test LoadDetectorDimensions()
  bool TestLoadDetectorDimensions();
  //! Test LoadCoeffsFile(), GetPixelCoeffs(), and GetTimingNoiseFWHM()
  bool TestLoadCoeffsFile();
  //! Test LoadSplinesFile(), GetDepthGrid(), GetCTDMap(), GetDepth(), GetCTD(), and GetSpline()
  bool TestLoadSplinesFile();
  //! Test LoadDisabledStripsFile()
  bool TestLoadDisabledStripsFile();
  //! Test LoadMaskMetrologyFile() and GetStripIntersection()
  bool TestMaskMetrology();
  //! Test that Initialize() fails without an energy calibration module in the supervisor
  bool TestInitializeWithoutEnergyCalibration();
  //! Test the Initialize() error paths and re-initialization
  bool TestInitialize();
  //! Test the depth and position reconstruction in AnalyzeEvent()
  bool TestAnalyzeEventDepth();
  //! Test the error handling in AnalyzeEvent()
  bool TestAnalyzeEventErrors();
  //! Test the shorted and disabled strips in AnalyzeEvent()
  bool TestAnalyzeEventDisabledStrips();
  //! Test the cases which might crash in a child process
  bool TestCrashCandidates();

  //! Load the test geometry once, return nullptr on failure
  MDGeometryQuest* GetGeometry();
  //! Write the geometry file and return its name
  MString WriteGeometry();
  //! Write the coefficients file and return its name
  MString WriteCoeffs();
  //! Write the splines file and return its name
  MString WriteSplines(double Thickness = 1.5, unsigned int DetID = 0);
  //! Write the disabled strips file and return its name
  MString WriteDisabledStrips();
  //! Make sure the energy calibration module is available in the supervisor
  void EnsureEnergyCalibrationAvailable();
  //! Initialize a module with the standard files, returns true on success
  bool InitializeStandard(UTNModuleDepthCalibrationAccess& M, bool WithDisabledStrips = false);
  //! Create a strip hit and add it to the event
  MStripHit* AddStripHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, double Energy, double Timing, bool FastTiming = true);
  //! Create a hit with one LV and one HV strip hit with the given CTD
  MHit* AddSimpleHit(MReadOutAssembly& Event, unsigned int LVStrip, unsigned int HVStrip, double CTD, unsigned int DetID = 0);
  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);

  //! The test geometry
  MDGeometryQuest* m_Geometry;
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::Run()
{
  bool Passed = true;

  // Must run first: later subtests add an energy calibration module to the MSupervisor singleton, which cannot remove it again
  Passed = TestInitializeWithoutEnergyCalibration() && Passed;
  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestGetDominantStrip() && Passed;
  Passed = TestGetHitGrade() && Passed;
  Passed = TestNormPdf() && Passed;
  Passed = TestLoadDetectorDimensions() && Passed;
  Passed = TestLoadCoeffsFile() && Passed;
  Passed = TestLoadSplinesFile() && Passed;
  Passed = TestLoadDisabledStripsFile() && Passed;
  Passed = TestMaskMetrology() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestAnalyzeEventDepth() && Passed;
  Passed = TestAnalyzeEventErrors() && Passed;
  Passed = TestAnalyzeEventDisabledStrips() && Passed;
  Passed = TestCrashCandidates() && Passed;

  // ShowOptionsGUI() and CreateExpos() create GUI elements and are not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleDepthCalibration::WriteGeometry()
{
  // One Strip3D detector, 4.0 x 2.5 x 1.5 cm, 40 x 10 strips: pitch X = 0.1 cm, pitch Y = 0.25 cm
  MString G;
  G += "Name UTNDepthCalibrationGeometry\n";
  G += "SurroundingSphere 20 0.0 0.0 0.0 20\n";
  G += "ShowSurroundingSphere False\n\n";
  G += "Material Vacuum\nVacuum.Density 1E-12\nVacuum.Component H 1\n\n";
  G += "Material Germanium\nGermanium.Density 5.323\nGermanium.ComponentByMass Ge 1\n\n";
  G += "Volume WorldVolume\nWorldVolume.Material Vacuum\nWorldVolume.Visibility 0\nWorldVolume.Shape Sphere 0.0 100\nWorldVolume.Mother 0\n\n";
  G += "Volume Wafer\nWafer.Material Germanium\nWafer.Visibility 1\nWafer.Shape BRIK 2.0 1.25 0.75\nWafer.Position 0.0 0.0 0.0\nWafer.Mother WorldVolume\n\n";
  G += "MDStrip3D GeD_0\nGeD_0.DetectorVolume Wafer\nGeD_0.SensitiveVolume Wafer\nGeD_0.Offset 0.0 0.0\nGeD_0.StripNumber 40 10\n";
  G += "GeD_0.NoiseThreshold 15\nGeD_0.TriggerThreshold 30\nGeD_0.DepthResolution 100 0.02\nGeD_0.DepthResolution 1000 0.02\nGeD_0.EnergyResolution Gaus 100 100 1\nGeD_0.EnergyResolution Gaus 1000 1000 2\nGeD_0.FailureRate 0.0\n\n";
  G += "Trigger GeD_0Trigger\nGeD_0Trigger.Veto false\nGeD_0Trigger.TriggerByChannel true\nGeD_0Trigger.Detector GeD_0 1\n";

  MString FileName = GetTemporaryFileName("depth.geo.setup");
  if (WriteTextFile(FileName, G) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MDGeometryQuest* UTNModuleDepthCalibration::GetGeometry()
{
  if (m_Geometry != nullptr) return m_Geometry;

  MDGeometryQuest* G = new MDGeometryQuest();
  DisableDefaultStreams();
  bool Loaded = G->ScanSetupFile(WriteGeometry(), false, false, false);
  EnableDefaultStreams();
  if (Loaded == false) {
    delete G;
    return nullptr;
  }
  m_Geometry = G;
  return m_Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleDepthCalibration::WriteCoeffs()
{
  // Stretch 1, offset 0, CTD sigma 1 ns for all pixels of detector 0, except:
  // pixel 101 (LV 1, HV 1) is missing, pixel 202 has stretch 2 and offset 10, pixel 909 has a CTD sigma of 10 ns
  MString C;
  C += "### 800 V 80 K 59.5 keV\n";
  C += "# Pixel code, stretch, offset, CTD sigma, chi2\n";
  for (unsigned int lv = 0; lv < 10; ++lv) {
    for (unsigned int hv = 0; hv < 40; ++hv) {
      unsigned int Pixel = 100*lv + hv;
      if (Pixel == 101) continue;
      if (Pixel == 202) {
        C += "202,2.0,10.0,1.0,1.0\n";
      } else if (Pixel == 909) {
        C += "909,1.0,0.0,10.0,1.0\n";
      } else {
        C += MString("") + Pixel + ",1.0,0.0,1.0,1.0\n";
      }
    }
  }

  MString FileName = GetTemporaryFileName("depth.coeffs.csv");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleDepthCalibration::WriteSplines(double Thickness, unsigned int DetID)
{
  // Linear relation: CTD = 200 ns/cm * depth
  MString C;
  C += MString("### ") + DetID + " 800 80 662\n";
  C += "# depth, ctd\n";
  unsigned int N = 31;
  for (unsigned int i = 0; i < N; ++i) {
    double Depth = -Thickness/2 + i*Thickness/(N - 1);
    ostringstream Line;
    Line<<setprecision(10)<<Depth<<","<<200.0*Depth<<"\n";
    C += Line.str();
  }

  MString FileName = GetTemporaryFileName(MString("depth.splines.") + DetID + "." + (int) (Thickness*1000) + ".csv");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleDepthCalibration::WriteDisabledStrips()
{
  MString C;
  C += "# Det ID, Side, Disabled Strip, [Read-out via]\n";
  C += "0 l 6 5\n";
  C += "0 h 4 3\n";
  C += "0 l 7\n";

  MString FileName = GetTemporaryFileName("depth.disabled.csv");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


void UTNModuleDepthCalibration::EnsureEnergyCalibrationAvailable()
{
  MSupervisor* S = MSupervisor::GetSupervisor();
  if (S->GetAvailableModuleByXmlTag("XmlTagEnergyCalibration") == nullptr) {
    S->AddAvailableModule(new MModuleEnergyCalibration());
  }
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::InitializeStandard(UTNModuleDepthCalibrationAccess& M, bool WithDisabledStrips)
{
  MDGeometryQuest* G = GetGeometry();
  if (G == nullptr) return false;
  EnsureEnergyCalibrationAvailable();

  M.SetGeometry(G);
  M.SetCoeffsFileName(WriteCoeffs());
  M.SetSplinesFileName(WriteSplines());
  M.SetMaskMetrologyCorrectionEnable(false);
  M.SetUCSDOverride(false);
  M.SetAccountForDisabledStrips(WithDisabledStrips);
  M.SetDisabledStripsFileName(WithDisabledStrips == true ? WriteDisabledStrips() : MString(""));

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Result = M.Initialize();
  g_Verbosity = OldVerbosity;
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleDepthCalibration::AddStripHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, double Energy, double Timing, bool FastTiming)
{
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(DetID);
  SH->SetStripID(StripID);
  SH->IsLowVoltageStrip(IsLowVoltage);
  SH->SetEnergy(Energy);
  SH->SetTiming(Timing);
  SH->HasFastTiming(FastTiming);
  Event.AddStripHit(SH);
  return SH;
}


////////////////////////////////////////////////////////////////////////////////


MHit* UTNModuleDepthCalibration::AddSimpleHit(MReadOutAssembly& Event, unsigned int LVStrip, unsigned int HVStrip, double CTD, unsigned int DetID)
{
  // CTD = HV timing - LV timing
  MHit* H = new MHit();
  H->AddStripHit(AddStripHit(Event, DetID, LVStrip, true, 100.0, 4000.0));
  H->AddStripHit(AddStripHit(Event, DetID, HVStrip, false, 100.0, 4000.0 + CTD));
  H->SetEnergy(100.0);
  Event.AddHit(H);
  return H;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // A crash is the expected outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Seconds after which a hanging child is killed
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


bool UTNModuleDepthCalibration::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleDepthCalibration M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Depth Calibration'", M.GetName(), MString("Depth Calibration")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagDepthCalibration'", M.GetXmlTag(), MString("XmlTagDepthCalibration")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_DepthCorrection", M.ProvidesModuleType(MAssembly::c_DepthCorrection)) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_PositionDetermiation", M.ProvidesModuleType(MAssembly::c_PositionDetermiation)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The energy calibration is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EnergyCalibration)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The strip pairing is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_StripPairing)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The TAC calibration is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_TACCalibration)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = EvaluateNear("GetCoeffsEnergy()", "default", "The default coefficients energy is 59.5 keV", M.GetCoeffsEnergy(), 59.5, 1e-12) && Passed;
  Passed = Evaluate("GetCoeffsFileName()", "default", "The default coefficients file name is empty", M.GetCoeffsFileName(), MString("")) && Passed;
  Passed = Evaluate("GetSplinesFileName()", "default", "The default splines file name is empty", M.GetSplinesFileName(), MString("")) && Passed;
  Passed = Evaluate("GetCoeffs()", "default", "The default coefficients are empty", (unsigned int) M.GetCoeffs().size(), 0u) && Passed;
  Passed = Evaluate("GetDepthGrid()", "default", "The default depth grid is empty", (unsigned int) M.GetDepthGrid().size(), 0u) && Passed;
  Passed = Evaluate("GetCTDMap()", "default", "The default CTD map is empty", (unsigned int) M.GetCTDMap().size(), 0u) && Passed;

  MModuleDepthCalibration* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  // The boolean options must be initialized by the constructor; construct into memory filled with 0xFF to make uninitialized members visible
  // The raw byte is compared, since reading an uninitialized bool as bool is undefined behavior and differs between compilers
  // GCC removes the fill before the constructor as a dead store (-flifetime-dse), which leaves random bytes: not checked with GCC
#if !defined(__GNUC__) || defined(__clang__)
  alignas(UTNModuleDepthCalibrationAccess) unsigned char Buffer[sizeof(UTNModuleDepthCalibrationAccess)];
  memset(Buffer, 0xFF, sizeof(Buffer));
  // Default-initialization (no parentheses): value-initialization would zero the memory first
  UTNModuleDepthCalibrationAccess* D = new (Buffer) UTNModuleDepthCalibrationAccess;
  Passed = ExpectedToFail(Evaluate("MModuleDepthCalibration()", "dirty memory", "AccountForDisabledStrips is initialized to false", D->GetRawAccountForDisabledStrips(), 0u)) && Passed;
  Passed = ExpectedToFail(Evaluate("MModuleDepthCalibration()", "dirty memory", "MaskMetrologyCorrectionEnable is initialized to false", D->GetRawMaskMetrologyEnabled(), 0u)) && Passed;
  Passed = ExpectedToFail(Evaluate("MModuleDepthCalibration()", "dirty memory", "UCSDOverride is initialized to false", D->GetRawUCSDOverride(), 0u)) && Passed;
  Passed = ExpectedToFail(Evaluate("MModuleDepthCalibration()", "dirty memory", "The coefficients file is initially not loaded", D->GetRawCoeffsFileIsLoaded(), 0u)) && Passed;
  Passed = ExpectedToFail(Evaluate("MModuleDepthCalibration()", "dirty memory", "The splines file is initially not loaded", D->GetRawSplinesFileIsLoaded(), 0u)) && Passed;
  Passed = ExpectedToFail(Evaluate("MModuleDepthCalibration()", "dirty memory", "The mask metrology file is initially not loaded", D->GetRawMaskMetrologyFileIsLoaded(), 0u)) && Passed;
  D->~UTNModuleDepthCalibrationAccess();
#endif

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestGettersSetters()
{
  bool Passed = true;

  MModuleDepthCalibration M;
  M.SetCoeffsFileName("c.csv");
  Passed = Evaluate("SetCoeffsFileName/GetCoeffsFileName", "c.csv", "GetCoeffsFileName returns the set name", M.GetCoeffsFileName(), MString("c.csv")) && Passed;
  M.SetSplinesFileName("s.csv");
  Passed = Evaluate("SetSplinesFileName/GetSplinesFileName", "s.csv", "GetSplinesFileName returns the set name", M.GetSplinesFileName(), MString("s.csv")) && Passed;
  M.SetMaskMetrologyFileName("m.csv");
  Passed = Evaluate("SetMaskMetrologyFileName/GetMaskMetrologyFileName", "m.csv", "GetMaskMetrologyFileName returns the set name", M.GetMaskMetrologyFileName(), MString("m.csv")) && Passed;
  M.SetDisabledStripsFileName("d.csv");
  Passed = Evaluate("SetDisabledStripsFileName/GetDisabledStripsFileName", "d.csv", "GetDisabledStripsFileName returns the set name", M.GetDisabledStripsFileName(), MString("d.csv")) && Passed;

  M.SetMaskMetrologyCorrectionEnable(true);
  Passed = EvaluateTrue("SetMaskMetrologyCorrectionEnable/GetMaskMetrologyCorrectionEnable", "true", "Returns true", M.GetMaskMetrologyCorrectionEnable()) && Passed;
  M.SetMaskMetrologyCorrectionEnable(false);
  Passed = EvaluateFalse("SetMaskMetrologyCorrectionEnable/GetMaskMetrologyCorrectionEnable", "false", "Returns false", M.GetMaskMetrologyCorrectionEnable()) && Passed;
  M.SetAccountForDisabledStrips(true);
  Passed = EvaluateTrue("SetAccountForDisabledStrips/GetAccountForDisabledStrips", "true", "Returns true", M.GetAccountForDisabledStrips()) && Passed;
  M.SetAccountForDisabledStrips(false);
  Passed = EvaluateFalse("SetAccountForDisabledStrips/GetAccountForDisabledStrips", "false", "Returns false", M.GetAccountForDisabledStrips()) && Passed;
  M.SetUCSDOverride(true);
  Passed = EvaluateTrue("SetUCSDOverride/GetUCSDOverride", "true", "Returns true", M.GetUCSDOverride()) && Passed;
  M.SetUCSDOverride(false);
  Passed = EvaluateFalse("SetUCSDOverride/GetUCSDOverride", "false", "Returns false", M.GetUCSDOverride()) && Passed;

  M.SetCoeffsEnergy(662.0);
  Passed = EvaluateNear("SetCoeffsEnergy/GetCoeffsEnergy", "662", "GetCoeffsEnergy returns 662", M.GetCoeffsEnergy(), 662.0, 1e-12) && Passed;

  unordered_map<int, vector<double>> Coeffs;
  Coeffs[503] = { 1.1, 2.2, 3.3, 4.4 };
  M.SetCoeffs(Coeffs);
  auto Got = M.GetCoeffs();
  Passed = Evaluate("SetCoeffs/GetCoeffs", "one pixel", "GetCoeffs has one pixel", (unsigned int) Got.size(), 1u) && Passed;
  Passed = Evaluate("SetCoeffs/GetCoeffs", "pixel 503", "Pixel 503 exists", (unsigned int) Got.count(503), 1u) && Passed;
  if (Got.count(503) == 1) {
    Passed = Evaluate("SetCoeffs/GetCoeffs", "pixel 503", "Pixel 503 has four coefficients", (unsigned int) Got[503].size(), 4u) && Passed;
  }
  if (Got.count(503) == 1 && Got[503].size() == 4) {
    Passed = EvaluateNear("SetCoeffs/GetCoeffs", "pixel 503", "The offset is 2.2", Got[503][1], 2.2, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestXmlRoundTrip()
{
  bool Passed = true;

  for (int Flags = 0; Flags < 2; ++Flags) {
    bool Flag = (Flags == 1);
    MModuleDepthCalibration Writer;
    Writer.SetCoeffsFileName("/data/c.csv");
    Writer.SetSplinesFileName("/data/s.csv");
    Writer.SetAccountForDisabledStrips(Flag);
    Writer.SetDisabledStripsFileName("/data/d.csv");
    Writer.SetMaskMetrologyCorrectionEnable(!Flag);
    Writer.SetMaskMetrologyFileName("/data/m.csv");
    Writer.SetUCSDOverride(Flag);
    MXmlNode* Node = Writer.CreateXmlConfiguration();

    MModuleDepthCalibration Reader;
    Reader.SetAccountForDisabledStrips(!Flag);
    Reader.SetMaskMetrologyCorrectionEnable(Flag);
    Reader.SetUCSDOverride(!Flag);
    MString Input = MString("flags ") + Flags;
    Passed = EvaluateTrue("ReadXmlConfiguration()", Input, "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
    Passed = Evaluate("ReadXmlConfiguration()", Input, "The coefficients file name survives", Reader.GetCoeffsFileName(), MString("/data/c.csv")) && Passed;
    Passed = Evaluate("ReadXmlConfiguration()", Input, "The splines file name survives", Reader.GetSplinesFileName(), MString("/data/s.csv")) && Passed;
    Passed = Evaluate("ReadXmlConfiguration()", Input, "The disabled strips file name survives", Reader.GetDisabledStripsFileName(), MString("/data/d.csv")) && Passed;
    Passed = Evaluate("ReadXmlConfiguration()", Input, "The mask metrology file name survives", Reader.GetMaskMetrologyFileName(), MString("/data/m.csv")) && Passed;
    Passed = EvaluateTrue("ReadXmlConfiguration()", Input, "AccountForDisabledStrips survives", Reader.GetAccountForDisabledStrips() == Flag) && Passed;
    Passed = EvaluateTrue("ReadXmlConfiguration()", Input, "MaskMetrology survives", Reader.GetMaskMetrologyCorrectionEnable() == !Flag) && Passed;
    Passed = EvaluateTrue("ReadXmlConfiguration()", Input, "UCSDOverride survives", Reader.GetUCSDOverride() == Flag) && Passed;
    delete Node;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestGetDominantStrip()
{
  bool Passed = true;

  MModuleDepthCalibration M;
  MStripHit A, B, C;
  A.SetEnergy(100.0);
  B.SetEnergy(300.0);
  C.SetEnergy(100.0);
  vector<MStripHit*> Strips = { &A, &B, &C };
  double Fraction = -1;
  Passed = EvaluateTrue("GetDominantStrip()", "100, 300, 100", "The strip with the highest energy is returned", M.GetDominantStrip(Strips, Fraction) == &B) && Passed;
  Passed = EvaluateNear("GetDominantStrip()", "100, 300, 100", "The energy fraction is 300/500", Fraction, 0.6, 1e-12) && Passed;

  vector<MStripHit*> One = { &A };
  Passed = EvaluateTrue("GetDominantStrip()", "single strip", "The single strip is returned", M.GetDominantStrip(One, Fraction) == &A) && Passed;
  Passed = EvaluateNear("GetDominantStrip()", "single strip", "The energy fraction is 1", Fraction, 1.0, 1e-12) && Passed;

  vector<MStripHit*> Empty;
  Passed = EvaluateTrue("GetDominantStrip()", "no strips", "nullptr is returned for an empty list", M.GetDominantStrip(Empty, Fraction) == nullptr) && Passed;
  Passed = EvaluateNear("GetDominantStrip()", "no strips", "The energy fraction is 0", Fraction, 0.0, 1e-12) && Passed;

  MStripHit Z1, Z2;
  Z1.SetEnergy(0.0);
  Z2.SetEnergy(0.0);
  vector<MStripHit*> Zero = { &Z1, &Z2 };
  Passed = EvaluateTrue("GetDominantStrip()", "zero energies", "A strip is returned even if all energies are zero", M.GetDominantStrip(Zero, Fraction) != nullptr) && Passed;
  Passed = EvaluateNear("GetDominantStrip()", "zero energies", "The energy fraction is 0", Fraction, 0.0, 1e-12) && Passed;

  vector<MStripHit*> Equal = { &A, &C };
  Passed = EvaluateTrue("GetDominantStrip()", "equal energies", "The first strip wins a tie", M.GetDominantStrip(Equal, Fraction) == &A) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestGetHitGrade()
{
  bool Passed = true;

  MModuleDepthCalibration M;
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  // Owner of all strip hits
  MReadOutAssembly Owner;
  auto Grade = [&](vector<unsigned int> LV, vector<unsigned int> HV) -> int {
    MHit H;
    for (unsigned int s : LV) H.AddStripHit(AddStripHit(Owner, 0, s, true, 50.0, 0.0));
    for (unsigned int s : HV) H.AddStripHit(AddStripHit(Owner, 0, s, false, 50.0, 0.0));
    return M.GetHitGrade(&H);
  };

  Passed = Evaluate("GetHitGrade()", "1 LV, 1 HV", "Grade 0", Grade({ 5 }, { 3 }), 0) && Passed;
  Passed = Evaluate("GetHitGrade()", "1 LV, 2 HV", "Grade 1", Grade({ 5 }, { 3, 4 }), 1) && Passed;
  Passed = Evaluate("GetHitGrade()", "2 LV, 1 HV", "Grade 2", Grade({ 5, 6 }, { 3 }), 2) && Passed;
  Passed = Evaluate("GetHitGrade()", "2 LV, 2 HV", "Grade 3", Grade({ 5, 6 }, { 3, 4 }), 3) && Passed;
  Passed = Evaluate("GetHitGrade()", "4 LV, 1 HV", "Complicated geometries are grade 4", Grade({ 4, 5, 6, 7 }, { 3 }), 4) && Passed;
  Passed = Evaluate("GetHitGrade()", "non-adjacent LV", "Non-adjacent strips are grade 6", Grade({ 2, 5 }, { 3 }), 6) && Passed;
  Passed = Evaluate("GetHitGrade()", "non-adjacent HV", "Non-adjacent strips are grade 6", Grade({ 5 }, { 3, 9 }), 6) && Passed;
  Passed = Evaluate("GetHitGrade()", "LV only", "A hit without HV strips is an error (-1)", Grade({ 5 }, {}), -1) && Passed;
  Passed = Evaluate("GetHitGrade()", "HV only", "A hit without LV strips is an error (-1)", Grade({}, { 3 }), -1) && Passed;
  Passed = Evaluate("GetHitGrade()", "no strips", "A hit without strips is an error (-1)", Grade({}, {}), -1) && Passed;
  Passed = Evaluate("GetHitGrade()", "nullptr", "A null hit is an error (-1)", M.GetHitGrade(nullptr), -1) && Passed;

  {
    MHit H;
    H.AddStripHit(AddStripHit(Owner, 0, 5, true, 50.0, 0.0));
    H.AddStripHit(AddStripHit(Owner, 0, 3, false, 0.0, 0.0));
    Passed = Evaluate("GetHitGrade()", "zero-energy strip", "A strip without energy is an error (-1)", M.GetHitGrade(&H), -1) && Passed;
  }
  {
    MHit H;
    H.AddStripHit(AddStripHit(Owner, 0, 5, true, 50.0, 0.0));
    H.AddStripHit(AddStripHit(Owner, 0, 3, false, 50.0, 0.0));
    H.SetStripHitMultipleTimesLV(true);
    Passed = Evaluate("GetHitGrade()", "LV strip hit multiple times", "Grade 5", M.GetHitGrade(&H), 5) && Passed;
  }
  {
    MHit H;
    H.AddStripHit(AddStripHit(Owner, 0, 5, true, 50.0, 0.0));
    H.AddStripHit(AddStripHit(Owner, 0, 3, false, 50.0, 0.0));
    H.SetStripHitMultipleTimesHV(true);
    Passed = Evaluate("GetHitGrade()", "HV strip hit multiple times", "Grade 5", M.GetHitGrade(&H), 5) && Passed;
  }

  g_Verbosity = OldVerbosity;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestNormPdf()
{
  bool Passed = true;

  UTNModuleDepthCalibrationAccess M;
  vector<double> P = M.CallNormPdf({ 0.0, 1.0, -2.0 }, 0.0, 1.0);
  Passed = Evaluate("norm_pdf()", "three values", "One value per input", (unsigned int) P.size(), 3u) && Passed;
  if (P.size() == 3) {
    Passed = EvaluateNear("norm_pdf()", "x = 0", "The peak is 1/sqrt(2 pi)", P[0], 1.0/sqrt(2*M_PI), 1e-12) && Passed;
    Passed = EvaluateNear("norm_pdf()", "x = 1", "One sigma off is exp(-1/2)/sqrt(2 pi)", P[1], exp(-0.5)/sqrt(2*M_PI), 1e-12) && Passed;
    Passed = EvaluateNear("norm_pdf()", "x = -2", "Two sigma off is exp(-2)/sqrt(2 pi)", P[2], exp(-2.0)/sqrt(2*M_PI), 1e-12) && Passed;
  }
  vector<double> Q = M.CallNormPdf({ 7.0 }, 5.0, 2.0);
  Passed = EvaluateSize("norm_pdf()", "mu 5, sigma 2, x 7", "One value per input", Q.size(), 1) && Passed;
  if (Q.size() == 1) {
    Passed = EvaluateNear("norm_pdf()", "mu 5, sigma 2, x 7", "The normalization includes sigma", Q[0], exp(-0.5)/(2.0*sqrt(2*M_PI)), 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestLoadDetectorDimensions()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  Passed = EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", G != nullptr) && Passed;
  if (G == nullptr) return Passed;

  UTNModuleDepthCalibrationAccess M;
  M.SetUCSDOverride(false);
  Passed = EvaluateTrue("LoadDetectorDimensions()", "test geometry", "LoadDetectorDimensions succeeds", M.LoadDetectorDimensions(G)) && Passed;
  Passed = Evaluate("LoadDetectorDimensions()", "test geometry", "One detector is found", M.GetNDetectorIDs(), 1u) && Passed;
  Passed = EvaluateNear("LoadDetectorDimensions()", "test geometry", "The thickness is 1.5 cm", M.GetThickness(0), 1.5, 1e-9) && Passed;
  Passed = EvaluateNear("LoadDetectorDimensions()", "test geometry", "The X pitch is 0.1 cm", M.GetXPitch(0), 0.1, 1e-9) && Passed;
  Passed = EvaluateNear("LoadDetectorDimensions()", "test geometry", "The Y pitch is 0.25 cm", M.GetYPitch(0), 0.25, 1e-9) && Passed;
  Passed = Evaluate("LoadDetectorDimensions()", "test geometry", "There are 40 X strips", M.GetNXStrips(0), 40) && Passed;
  Passed = Evaluate("LoadDetectorDimensions()", "test geometry", "There are 10 Y strips", M.GetNYStrips(0), 10) && Passed;

  // Loading the same geometry again must not duplicate the detector
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  M.LoadDetectorDimensions(G);
  g_Verbosity = OldVerbosity;
  Passed = ExpectedToFail(Evaluate("LoadDetectorDimensions()", "loaded twice", "Loading the geometry twice still gives one detector", M.GetNDetectorIDs(), 1u)) && Passed;

  // UCSD override maps the detector to ID 11
  UTNModuleDepthCalibrationAccess U;
  U.SetUCSDOverride(true);
  g_Verbosity = c_Quiet;
  U.LoadDetectorDimensions(G);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateNear("LoadDetectorDimensions()", "UCSD override", "With the UCSD override the detector has ID 11", U.GetThickness(11), 1.5, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestLoadCoeffsFile()
{
  bool Passed = true;

  UTNModuleDepthCalibrationAccess M;
  M.SetCoeffsEnergy(1.0);
  Passed = EvaluateTrue("LoadCoeffsFile()", "standard file", "Loading the coefficients succeeds", M.LoadCoeffsFile(WriteCoeffs())) && Passed;
  Passed = EvaluateNear("LoadCoeffsFile()", "### header", "The coefficients energy is read from the ### header", M.GetCoeffsEnergy(), 59.5, 1e-12) && Passed;
  auto C = M.GetCoeffs();
  Passed = Evaluate("LoadCoeffsFile()", "standard file", "399 pixels are loaded", (unsigned int) C.size(), 399u) && Passed;
  if (C.count(202) == 1 && C[202].size() == 4) {
    Passed = EvaluateNear("LoadCoeffsFile()", "pixel 202", "The stretch is 2", C[202][0], 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("LoadCoeffsFile()", "pixel 202", "The offset is 10", C[202][1], 10.0, 1e-12) && Passed;
    Passed = EvaluateNear("LoadCoeffsFile()", "pixel 202", "The CTD sigma is converted to FWHM", C[202][2], 2.355, 1e-12) && Passed;
    Passed = EvaluateNear("LoadCoeffsFile()", "pixel 202", "The chi square is stored", C[202][3], 1.0, 1e-12) && Passed;
  } else {
    Passed = EvaluateTrue("LoadCoeffsFile()", "pixel 202", "Pixel 202 is loaded with four values", false) && Passed;
  }

  // GetPixelCoeffs() and GetTimingNoiseFWHM() need the loaded flag which is set by Initialize()
  M.SetCoeffsFileLoaded(true);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  Passed = EvaluateTrue("GetPixelCoeffs()", "pixel 503", "The coefficients of pixel 503 exist", M.CallGetPixelCoeffs(503) != nullptr) && Passed;
  Passed = EvaluateTrue("GetPixelCoeffs()", "pixel 101", "The missing pixel 101 returns nullptr", M.CallGetPixelCoeffs(101) == nullptr) && Passed;
  g_Verbosity = OldVerbosity;
  // FWHM = sigma * 2.355 * 59.5 / E, at least 3 * 2.355
  Passed = EvaluateNear("GetTimingNoiseFWHM()", "pixel 909, 59.5 keV", "The noise is the CTD FWHM at the reference energy", M.CallGetTimingNoiseFWHM(909, 59.5), 23.55, 1e-9) && Passed;
  Passed = EvaluateNear("GetTimingNoiseFWHM()", "pixel 909, 119 keV", "The noise scales with 1/E", M.CallGetTimingNoiseFWHM(909, 119.0), 11.775, 1e-9) && Passed;
  Passed = EvaluateNear("GetTimingNoiseFWHM()", "pixel 503, 59.5 keV", "The noise has a minimum of 3 * 2.355", M.CallGetTimingNoiseFWHM(503, 59.5), 3.0*2.355, 1e-9) && Passed;
  M.SetCoeffsFileLoaded(false);
  Passed = EvaluateNear("GetTimingNoiseFWHM()", "not loaded", "Without coefficients the noise is 6 * 2.355", M.CallGetTimingNoiseFWHM(503, 59.5), 6.0*2.355, 1e-9) && Passed;
  g_Verbosity = c_Quiet;
  Passed = EvaluateTrue("GetPixelCoeffs()", "not loaded", "Without coefficients nullptr is returned", M.CallGetPixelCoeffs(503) == nullptr) && Passed;
  g_Verbosity = OldVerbosity;

  // Missing file
  g_Verbosity = c_Quiet;
  UTNModuleDepthCalibrationAccess Missing;
  bool MissingResult = Missing.LoadCoeffsFile(GetTemporaryFileName("missing.csv"));
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("LoadCoeffsFile()", "missing file", "A missing file is rejected", MissingResult) && Passed;

  // Loading a second file replaces the first one
  MString Small = GetTemporaryFileName("small.coeffs.csv");
  Passed = EvaluateTrue("WriteTextFile()", "Small", "The fixture file can be written", WriteTextFile(Small, "1234,1.0,0.0,1.0,1.0\n")) && Passed;
  M.LoadCoeffsFile(Small);
  Passed = ExpectedToFail(Evaluate("LoadCoeffsFile()", "second file", "Loading a second file replaces the coefficients of the first file", (unsigned int) M.GetCoeffs().size(), 1u)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestLoadSplinesFile()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  Passed = EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", G != nullptr) && Passed;
  if (G == nullptr) return Passed;

  int OldVerbosity = g_Verbosity;

  UTNModuleDepthCalibrationAccess M;
  M.SetUCSDOverride(false);
  M.LoadDetectorDimensions(G);
  Passed = EvaluateTrue("LoadSplinesFile()", "standard file", "Loading the splines succeeds", M.LoadSplinesFile(WriteSplines())) && Passed;
  auto Grid = M.GetDepthGrid();
  auto CTD = M.GetCTDMap();
  Passed = Evaluate("GetDepthGrid()", "standard file", "There is a depth grid for one detector", (unsigned int) Grid.size(), 1u) && Passed;
  Passed = Evaluate("GetCTDMap()", "standard file", "There is a CTD map for one detector", (unsigned int) CTD.size(), 1u) && Passed;
  if (Grid.count(0) == 1 && CTD.count(0) == 1 && CTD[0].size() >= 1) {
    vector<double>& D = Grid[0];
    vector<double>& C = CTD[0][0];
    Passed = Evaluate("GetDepthGrid()", "standard file", "The depth grid has 1000 points", (unsigned int) D.size(), 1000u) && Passed;
    Passed = Evaluate("GetCTDMap()", "standard file", "The CTD grid has as many points as the depth grid", (unsigned int) C.size(), (unsigned int) D.size()) && Passed;
    if (D.size() == 1000 && C.size() == 1000) {
      Passed = EvaluateNear("GetDepthGrid()", "standard file", "The depth grid starts at -thickness/2", D.front(), -0.75, 1e-9) && Passed;
      Passed = EvaluateNear("GetDepthGrid()", "standard file", "The depth grid ends at +thickness/2", D.back(), 0.75, 1e-9) && Passed;
      Passed = EvaluateNear("GetCTDMap()", "standard file", "The CTD at index 500 follows the linear relation", C[500], 200.0*D[500], 1e-6) && Passed;
      Passed = EvaluateNear("GetCTDMap()", "standard file", "The CTD at index 0 follows the linear relation", C[0], 200.0*D[0], 1e-6) && Passed;
    }
  } else {
    Passed = EvaluateTrue("LoadSplinesFile()", "standard file", "Detector 0 has a depth grid and CTD map", false) && Passed;
  }

  // The getters which need the loaded flag
  UTNModuleDepthCalibrationAccess I;
  if (InitializeStandard(I) == true) {
    Passed = Evaluate("GetDepth()", "detector 0", "GetDepth returns the depth grid", (unsigned int) I.CallGetDepth(0).size(), 1000u) && Passed;
    Passed = Evaluate("GetCTD()", "detector 0, grade 0", "GetCTD returns the CTD grid", (unsigned int) I.CallGetCTD(0, 0).size(), 1000u) && Passed;
    Passed = Evaluate("GetCTD()", "detector 0, grade 3", "GetCTD returns the grade-0 relation for all grades", (unsigned int) I.CallGetCTD(0, 3).size(), 1000u) && Passed;
    TSpline3* S = I.CallGetSpline(0, 0);
    Passed = EvaluateTrue("GetSpline()", "detector 0", "GetSpline returns a spline", S != nullptr) && Passed;
    if (S != nullptr) {
      Passed = EvaluateNear("GetSpline()", "detector 0, depth 0.3", "The spline evaluates to 200 * depth", S->Eval(0.3), 60.0, 1e-6) && Passed;
    }
    g_Verbosity = c_Quiet;
    Passed = Evaluate("GetDepth()", "unknown detector 4", "An empty depth grid is returned for an unknown detector", (unsigned int) I.CallGetDepth(4).size(), 0u) && Passed;
    Passed = Evaluate("GetCTD()", "unknown detector 4", "An empty CTD grid is returned for an unknown detector", (unsigned int) I.CallGetCTD(4, 0).size(), 0u) && Passed;
    Passed = EvaluateTrue("GetSpline()", "unknown detector 4", "nullptr is returned for an unknown detector", I.CallGetSpline(4, 0) == nullptr) && Passed;
    g_Verbosity = OldVerbosity;
    I.Finalize();
  } else {
    Passed = EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false) && Passed;
  }

  // Error paths
  // UCSDOverride is set explicitly: the constructor leaves it uninitialized (see TestDefaultConstruction), and a garbage
  // value maps all detectors to ID 11, which makes LoadSplinesFile() skip detector 0
  g_Verbosity = c_Quiet;
  UTNModuleDepthCalibrationAccess Thick;
  Thick.SetUCSDOverride(false);
  Thick.LoadDetectorDimensions(G);
  bool ThickResult = Thick.LoadSplinesFile(WriteSplines(1.2));

  UTNModuleDepthCalibrationAccess Dup;
  Dup.SetUCSDOverride(false);
  Dup.LoadDetectorDimensions(G);
  MString DupFile = GetTemporaryFileName("dup.splines.csv");
  MString Twice = ReadTextFile(WriteSplines());
  Passed = EvaluateTrue("WriteTextFile()", "DupFile", "The fixture file can be written", WriteTextFile(DupFile, Twice + Twice)) && Passed;
  bool DupResult = Dup.LoadSplinesFile(DupFile);

  UTNModuleDepthCalibrationAccess Columns;
  Columns.SetUCSDOverride(false);
  Columns.LoadDetectorDimensions(G);
  MString ColFile = GetTemporaryFileName("columns.splines.csv");
  Passed = EvaluateTrue("WriteTextFile()", "ColFile", "The fixture file can be written", WriteTextFile(ColFile, "### 0\n-0.75,-150,1\n0.0,0\n0.75,150,1\n")) && Passed;
  bool ColumnsResult = Columns.LoadSplinesFile(ColFile);

  UTNModuleDepthCalibrationAccess Missing;
  bool MissingResult = Missing.LoadSplinesFile(GetTemporaryFileName("missing.csv"));
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("LoadSplinesFile()", "thickness 1.2 cm vs 1.5 cm", "A thickness mismatch is rejected", ThickResult) && Passed;
  Passed = EvaluateFalse("LoadSplinesFile()", "detector 0 twice", "Two splines for the same detector are rejected", DupResult) && Passed;
  Passed = EvaluateFalse("LoadSplinesFile()", "inconsistent column count", "CTD columns with a different length than the depth column are rejected", ColumnsResult) && Passed;
  Passed = EvaluateFalse("LoadSplinesFile()", "missing file", "A missing file is rejected", MissingResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestLoadDisabledStripsFile()
{
  bool Passed = true;

  UTNModuleDepthCalibrationAccess M;
  Passed = EvaluateTrue("LoadDisabledStripsFile()", "standard file", "Loading succeeds", M.LoadDisabledStripsFile(WriteDisabledStrips())) && Passed;
  Passed = Evaluate("LoadDisabledStripsFile()", "standard file", "Three strips are disabled", M.GetNDisabledStrips(), 3u) && Passed;
  Passed = Evaluate("LoadDisabledStripsFile()", "standard file", "Two strips are shorted", M.GetNShortedStrips(), 2u) && Passed;

  // Loading again replaces the content
  Passed = EvaluateTrue("LoadDisabledStripsFile()", "same file twice", "Loading the same file twice succeeds", M.LoadDisabledStripsFile(WriteDisabledStrips())) && Passed;
  Passed = Evaluate("LoadDisabledStripsFile()", "same file twice", "Still three strips are disabled", M.GetNDisabledStrips(), 3u) && Passed;

  // Empty lines and comments
  MString Comments = GetTemporaryFileName("comments.csv");
  Passed = EvaluateTrue("WriteTextFile()", "Comments", "The fixture file can be written", WriteTextFile(Comments, "# no disabled strips\n\n")) && Passed;
  UTNModuleDepthCalibrationAccess MC;
  Passed = EvaluateTrue("LoadDisabledStripsFile()", "comments only", "A file with only comments is accepted", MC.LoadDisabledStripsFile(Comments)) && Passed;
  Passed = Evaluate("LoadDisabledStripsFile()", "comments only", "No strips are disabled", MC.GetNDisabledStrips(), 0u) && Passed;

  // Error paths
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MString NonAdjacent = GetTemporaryFileName("nonadjacent.csv");
  Passed = EvaluateTrue("WriteTextFile()", "NonAdjacent", "The fixture file can be written", WriteTextFile(NonAdjacent, "0 l 6 8\n")) && Passed;
  UTNModuleDepthCalibrationAccess MN;
  bool NonAdjacentResult = MN.LoadDisabledStripsFile(NonAdjacent);
  MString Duplicate = GetTemporaryFileName("duplicate.csv");
  Passed = EvaluateTrue("WriteTextFile()", "Duplicate", "The fixture file can be written", WriteTextFile(Duplicate, "0 l 6\n0 l 6\n")) && Passed;
  UTNModuleDepthCalibrationAccess MD;
  bool DuplicateResult = MD.LoadDisabledStripsFile(Duplicate);
  MString DoubleShort = GetTemporaryFileName("doubleshort.csv");
  Passed = EvaluateTrue("WriteTextFile()", "DoubleShort", "The fixture file can be written", WriteTextFile(DoubleShort, "0 l 6 5\n0 l 4 5\n")) && Passed;
  UTNModuleDepthCalibrationAccess MS;
  bool DoubleShortResult = MS.LoadDisabledStripsFile(DoubleShort);
  MString Short = GetTemporaryFileName("short.csv");
  Passed = EvaluateTrue("WriteTextFile()", "Short", "The fixture file can be written", WriteTextFile(Short, "0 l\n")) && Passed;
  UTNModuleDepthCalibrationAccess MT;
  bool ShortResult = MT.LoadDisabledStripsFile(Short);
  UTNModuleDepthCalibrationAccess MM;
  bool MissingResult = MM.LoadDisabledStripsFile(GetTemporaryFileName("missing.csv"));
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("LoadDisabledStripsFile()", "non-adjacent shorted strips", "Non-adjacent shorted strips are rejected", NonAdjacentResult) && Passed;
  Passed = EvaluateFalse("LoadDisabledStripsFile()", "duplicate entry", "A duplicate entry is rejected", DuplicateResult) && Passed;
  Passed = EvaluateFalse("LoadDisabledStripsFile()", "two strips read out via the same strip", "Two shorted strips read out via the same strip are rejected", DoubleShortResult) && Passed;
  Passed = EvaluateFalse("LoadDisabledStripsFile()", "two tokens", "A line with two tokens is rejected", ShortResult) && Passed;
  Passed = EvaluateFalse("LoadDisabledStripsFile()", "missing file", "A missing file is rejected", MissingResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestMaskMetrology()
{
  bool Passed = true;

  UTNModuleDepthCalibrationAccess M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds for the metrology test", false);
  }

  // Values in mm: LV strip 5 through (10 mm, 5 mm) with yaw 1 deg; HV strip 3 through (3 mm, 20 mm) with yaw 90 deg (parallel to x)
  // LV strip 6 with yaw 0 deg triggers the division-by-zero fallback
  MString File = GetTemporaryFileName("metrology.csv");
  MString Content = "# Det, Side, Strip, x, y, z, roll, pitch, yaw\n0,l,5,10,5,0,0,0,1\n0,h,3,3,20,0,0,0,90\n0,p,6,10,5,0,0,0,0";
  Passed = EvaluateTrue("WriteTextFile()", "File", "The fixture file can be written", WriteTextFile(File, Content)) && Passed;
  Passed = EvaluateTrue("LoadMaskMetrologyFile()", "file without final new line", "Loading the metrology succeeds", M.CallLoadMaskMetrologyFile(File)) && Passed;

  // Text files usually end with a new line
  MString FileNewLine = GetTemporaryFileName("metrology_newline.csv");
  Passed = EvaluateTrue("WriteTextFile()", "FileNewLine", "The fixture file can be written", WriteTextFile(FileNewLine, Content + "\n")) && Passed;
  UTNModuleDepthCalibrationAccess MNewLine;
  int OldVerbosityNewLine = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool NewLineResult = MNewLine.CallLoadMaskMetrologyFile(FileNewLine);
  g_Verbosity = OldVerbosityNewLine;
  Passed = ExpectedToFail(EvaluateTrue("LoadMaskMetrologyFile()", "file with final new line", "A metrology file ending with a new line is accepted", NewLineResult)) && Passed;

  MReadOutElementDoubleStrip LV5;
  LV5.SetDetectorID(0);
  LV5.SetStripID(5);
  LV5.IsLowVoltageStrip(true);
  MReadOutElementDoubleStrip HV3;
  HV3.SetDetectorID(0);
  HV3.SetStripID(3);
  HV3.IsLowVoltageStrip(false);
  MReadOutElementDoubleStrip LV6;
  LV6.SetDetectorID(0);
  LV6.SetStripID(6);
  LV6.IsLowVoltageStrip(true);

  // The LV line has the slope 1/tan(yaw_LV), the HV line the slope tan(yaw_HV - 90)
  // HV line: y = 2.0 cm; LV line: y = 0.5 + (x - 1.0) / tan(1 deg) => x = 1.0 + 1.5 * tan(1 deg)
  vector<double> I = M.CallGetStripIntersection(LV5, HV3);
  Passed = Evaluate("GetStripIntersection()", "LV yaw 1, HV yaw 90", "Two coordinates are returned", (unsigned int) I.size(), 2u) && Passed;
  if (I.size() == 2) {
    Passed = EvaluateNear("GetStripIntersection()", "LV yaw 1, HV yaw 90", "The intersection is on the HV strip line (y = 2 cm)", I[1], 2.0, 1e-9) && Passed;
    Passed = ExpectedToFail(EvaluateNear("GetStripIntersection()", "LV yaw 1, HV yaw 90", "The intersection is on the LV strip line", I[0], 1.0 + 1.5*tan(1.0*M_PI/180.0), 1e-9)) && Passed;
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  vector<double> F = M.CallGetStripIntersection(LV6, HV3);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateSize("GetStripIntersection()", "LV yaw 0", "Two coordinates are returned", F.size(), 2) && Passed;
  if (F.size() == 2) {
    // Unrotated position: X = pitchY * (6 - 4.5) for LV 6, Y = pitchX * (3 - 19.5) for HV 3
    Passed = EvaluateNear("GetStripIntersection()", "LV yaw 0", "The fallback X position is the unrotated LV strip position", F[0], 0.25*1.5, 1e-9) && Passed;
    Passed = EvaluateNear("GetStripIntersection()", "LV yaw 0", "The fallback Y position is the unrotated HV strip position", F[1], 0.1*(3 - 19.5), 1e-9) && Passed;
  }

  // Error paths
  g_Verbosity = c_Quiet;
  MString Bad = GetTemporaryFileName("bad_metrology.csv");
  Passed = EvaluateTrue("WriteTextFile()", "Bad", "The fixture file can be written", WriteTextFile(Bad, "0,l,5,10,5,0\n")) && Passed;
  UTNModuleDepthCalibrationAccess MB;
  bool BadResult = MB.CallLoadMaskMetrologyFile(Bad);
  UTNModuleDepthCalibrationAccess MM;
  bool MissingResult = MM.CallLoadMaskMetrologyFile(GetTemporaryFileName("missing.csv"));
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("LoadMaskMetrologyFile()", "six tokens", "A line with a wrong number of tokens is rejected", BadResult) && Passed;
  Passed = EvaluateFalse("LoadMaskMetrologyFile()", "missing file", "A missing file is rejected", MissingResult) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestInitializeWithoutEnergyCalibration()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  Passed = EvaluateTrue("Initialize()", "no energy calibration module", "The test geometry can be loaded", G != nullptr) && Passed;
  if (G == nullptr) return Passed;

  MSupervisor* S = MSupervisor::GetSupervisor();
  bool NoModule = (S->GetAvailableModuleByXmlTag("XmlTagEnergyCalibration") == nullptr);
  Passed = EvaluateTrue("Initialize()", "no energy calibration module", "Precondition: the supervisor has no energy calibration module", NoModule) && Passed;
  if (NoModule == false) return Passed;

  MString Coeffs = WriteCoeffs();
  MString Splines = WriteSplines();

  // In a child process, so a crash is caught and the supervisor of the parent stays unchanged
  bool Rejected = RunInChild([G, Coeffs, Splines]() -> bool {
    UTNModuleDepthCalibrationAccess NoEnergy;
    NoEnergy.SetGeometry(G);
    NoEnergy.SetCoeffsFileName(Coeffs);
    NoEnergy.SetSplinesFileName(Splines);
    NoEnergy.SetAccountForDisabledStrips(false);
    NoEnergy.SetMaskMetrologyCorrectionEnable(false);
    NoEnergy.SetUCSDOverride(false);
    return NoEnergy.Initialize() == false;
  });
  Passed = EvaluateTrue("Initialize()", "no energy calibration module", "Initialize returns false without an available energy calibration module", Rejected) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestInitialize()
{
  bool Passed = true;

  MDGeometryQuest* G = GetGeometry();
  Passed = EvaluateTrue("ScanSetupFile()", "test geometry", "The test geometry can be loaded", G != nullptr) && Passed;
  if (G == nullptr) return Passed;

  int OldVerbosity = g_Verbosity;

  EnsureEnergyCalibrationAvailable();

  auto Configure = [&](UTNModuleDepthCalibrationAccess& M) {
    M.SetGeometry(G);
    M.SetCoeffsFileName(WriteCoeffs());
    M.SetSplinesFileName(WriteSplines());
    M.SetAccountForDisabledStrips(false);
    M.SetMaskMetrologyCorrectionEnable(false);
    M.SetUCSDOverride(false);
  };

  g_Verbosity = c_Quiet;
  UTNModuleDepthCalibrationAccess MissingCoeffs;
  Configure(MissingCoeffs);
  MissingCoeffs.SetCoeffsFileName(GetTemporaryFileName("missing.coeffs.csv"));
  bool MissingCoeffsResult = MissingCoeffs.Initialize();

  UTNModuleDepthCalibrationAccess MissingSplines;
  Configure(MissingSplines);
  MissingSplines.SetSplinesFileName(GetTemporaryFileName("missing.splines.csv"));
  bool MissingSplinesResult = MissingSplines.Initialize();

  UTNModuleDepthCalibrationAccess MissingDisabled;
  Configure(MissingDisabled);
  MissingDisabled.SetAccountForDisabledStrips(true);
  MissingDisabled.SetDisabledStripsFileName(GetTemporaryFileName("missing.disabled.csv"));
  bool MissingDisabledResult = MissingDisabled.Initialize();

  UTNModuleDepthCalibrationAccess IgnoredDisabled;
  Configure(IgnoredDisabled);
  IgnoredDisabled.SetAccountForDisabledStrips(false);
  IgnoredDisabled.SetDisabledStripsFileName(GetTemporaryFileName("missing.disabled.csv"));
  bool IgnoredDisabledResult = IgnoredDisabled.Initialize();

  UTNModuleDepthCalibrationAccess MissingMetrology;
  Configure(MissingMetrology);
  MissingMetrology.SetMaskMetrologyCorrectionEnable(true);
  MissingMetrology.SetMaskMetrologyFileName(GetTemporaryFileName("missing.metrology.csv"));
  bool MissingMetrologyResult = MissingMetrology.Initialize();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "missing coefficients file", "Initialize returns false", MissingCoeffsResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing splines file", "Initialize returns false", MissingSplinesResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing disabled strips file", "Initialize returns false when disabled strips are accounted for", MissingDisabledResult) && Passed;
  Passed = EvaluateTrue("Initialize()", "missing disabled strips file, not used", "Initialize ignores the disabled strips file when not accounting for disabled strips", IgnoredDisabledResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing metrology file", "Initialize returns false when the mask metrology is enabled", MissingMetrologyResult) && Passed;
  IgnoredDisabled.Finalize();

  // Re-initialization
  UTNModuleDepthCalibrationAccess M;
  Passed = EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", InitializeStandard(M)) && Passed;
  g_Verbosity = c_Quiet;
  bool Second = M.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "second time without Finalize", "A second Initialize without Finalize succeeds", Second)) && Passed;
  Passed = ExpectedToFail(Evaluate("Initialize()", "second time without Finalize", "The detector is not duplicated", M.GetNDetectorIDs(), 1u)) && Passed;
  M.Finalize();
  Passed = Evaluate("Finalize()", "after Initialize", "Finalize clears the coefficients", (unsigned int) M.GetCoeffs().size(), 0u) && Passed;
  Passed = Evaluate("Finalize()", "after Initialize", "Finalize clears the CTD map", (unsigned int) M.GetCTDMap().size(), 0u) && Passed;
  Passed = Evaluate("Finalize()", "after Initialize", "Finalize clears the detector list", M.GetNDetectorIDs(), 0u) && Passed;
  g_Verbosity = c_Quiet;
  bool Third = M.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("Initialize()", "after Finalize", "Initialize after Finalize succeeds", Third) && Passed;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestAnalyzeEventDepth()
{
  bool Passed = true;

  UTNModuleDepthCalibrationAccess M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds for the depth test", false);
  }

  // CTD 40 ns => depth 0.2 cm; position from LV 5 and HV 3
  const vector<double> CTDs = { 40.0, -100.0, 0.0, 130.0 };
  for (double CTD : CTDs) {
    MReadOutAssembly E;
    MHit* H = AddSimpleHit(E, 5, 3, CTD);
    MString Input = MString("CTD ") + CTD + " ns";
    Passed = EvaluateTrue("AnalyzeEvent()", Input, "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", Input, "No depth calibration error is set", E.HasDepthCalibrationError()) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", Input, "The hit has a depth", H->GetNoDepth()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", Input, "The depth is CTD / 200 ns/cm", H->GetLocalPosition().Z(), CTD/200.0, 2e-3) && Passed;
    if (CTD == 40.0) {
      Passed = EvaluateTrue("AnalyzeEvent()", Input, "The c_DepthCorrection analysis progress is set", E.HasAnalysisProgress(MAssembly::c_DepthCorrection)) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", Input, "The c_PositionDetermiation analysis progress is set", E.HasAnalysisProgress(MAssembly::c_PositionDetermiation)) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", Input, "X = pitchY * (5 - 4.5)", H->GetLocalPosition().X(), 0.125, 1e-9) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", Input, "Y = pitchX * (3 - 19.5)", H->GetLocalPosition().Y(), -1.65, 1e-9) && Passed;
      Passed = EvaluateVectorNear("AnalyzeEvent()", Input, "The global position equals the local position for an unrotated detector at the origin", H->GetPosition(), H->GetLocalPosition(), 1e-9) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", Input, "The X resolution is pitchY / sqrt(12)", H->GetPositionResolution().X(), 0.25/sqrt(12.0), 1e-9) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", Input, "The Y resolution is pitchX / sqrt(12)", H->GetPositionResolution().Y(), 0.1/sqrt(12.0), 1e-9) && Passed;
      // Timing noise FWHM 3 * 2.355 ns => sigma 3 ns => 0.015 cm
      Passed = EvaluateNear("AnalyzeEvent()", Input, "The Z resolution is the CTD noise translated into depth", H->GetPositionResolution().Z(), 0.015, 1e-3) && Passed;
    }
  }

  // Stretch and offset: pixel 202 has CTD_measured = 2 * CTD + 10
  {
    MReadOutAssembly E;
    MHit* H = AddSimpleHit(E, 2, 2, 2.0*60.0 + 10.0);
    M.AnalyzeEvent(&E);
    Passed = EvaluateNear("AnalyzeEvent()", "pixel 202 with stretch 2, offset 10", "The stretch and offset are inverted before the depth lookup", H->GetLocalPosition().Z(), 0.3, 2e-3) && Passed;
  }

  // The dominant strips define the pixel
  {
    MReadOutAssembly E;
    MHit* H = new MHit();
    H->AddStripHit(AddStripHit(E, 0, 5, true, 80.0, 4000.0));
    H->AddStripHit(AddStripHit(E, 0, 6, true, 20.0, 4000.0));
    H->AddStripHit(AddStripHit(E, 0, 3, false, 30.0, 4040.0));
    H->AddStripHit(AddStripHit(E, 0, 4, false, 70.0, 4040.0));
    H->SetEnergy(100.0);
    E.AddHit(H);
    M.AnalyzeEvent(&E);
    Passed = EvaluateNear("AnalyzeEvent()", "grade 3, dominant LV 5 and HV 4", "X is defined by the dominant LV strip 5", H->GetLocalPosition().X(), 0.125, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "grade 3, dominant LV 5 and HV 4", "Y is defined by the dominant HV strip 4", H->GetLocalPosition().Y(), 0.1*(4 - 19.5), 1e-9) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestAnalyzeEventErrors()
{
  bool Passed = true;

  UTNModuleDepthCalibrationAccess M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds for the error test", false);
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  // Missing coefficients for pixel 101
  MReadOutAssembly E1;
  MHit* H1 = AddSimpleHit(E1, 1, 1, 40.0);
  M.AnalyzeEvent(&E1);

  // CTD far out of range
  MReadOutAssembly E2;
  MHit* H2 = AddSimpleHit(E2, 5, 3, 1000.0);
  M.AnalyzeEvent(&E2);

  // No fast timing
  MReadOutAssembly E3;
  MHit* H3 = AddSimpleHit(E3, 5, 3, 40.0);
  E3.GetStripHit(1)->HasFastTiming(false);
  M.AnalyzeEvent(&E3);

  // No timing
  MReadOutAssembly E4;
  MHit* H4 = new MHit();
  H4->AddStripHit(AddStripHit(E4, 0, 5, true, 100.0, 0.0));
  H4->AddStripHit(AddStripHit(E4, 0, 3, false, 100.0, 40.0));
  H4->SetEnergy(100.0);
  E4.AddHit(H4);
  M.AnalyzeEvent(&E4);

  // Grade 6: non-adjacent strips
  MReadOutAssembly E5;
  MHit* H5 = new MHit();
  H5->AddStripHit(AddStripHit(E5, 0, 2, true, 50.0, 4000.0));
  H5->AddStripHit(AddStripHit(E5, 0, 5, true, 50.0, 4000.0));
  H5->AddStripHit(AddStripHit(E5, 0, 3, false, 100.0, 4040.0));
  H5->SetEnergy(100.0);
  E5.AddHit(H5);
  M.AnalyzeEvent(&E5);

  // Grade -1: no HV strip
  MReadOutAssembly E6;
  MHit* H6 = new MHit();
  H6->AddStripHit(AddStripHit(E6, 0, 2, true, 50.0, 4000.0));
  H6->SetEnergy(50.0);
  E6.AddHit(H6);
  M.AnalyzeEvent(&E6);

  // Guard ring veto
  MReadOutAssembly E7;
  MHit* H7 = AddSimpleHit(E7, 5, 3, 40.0);
  E7.SetGuardRingVeto(true);
  bool R7 = M.AnalyzeEvent(&E7);

  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("AnalyzeEvent()", "missing coefficients", "The hit has no depth", H1->GetNoDepth()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "missing coefficients", "A depth calibration error is set", E1.HasDepthCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "CTD 1000 ns", "The hit has no depth", H2->GetNoDepth()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "CTD 1000 ns", "A depth calibration error is set", E2.HasDepthCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "no fast timing", "The hit has no depth", H3->GetNoDepth()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "no fast timing", "A depth calibration error is set", E3.HasDepthCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "no LV timing", "The hit has no depth", H4->GetNoDepth()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "no LV timing", "A depth calibration error is set", E4.HasDepthCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "grade 6", "The hit has no depth", H5->GetNoDepth()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "grade 6", "A depth calibration error is set", E5.HasDepthCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "grade -1", "The hit has no depth", H6->GetNoDepth()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "grade -1", "A depth calibration error is set", E6.HasDepthCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "grade -1", "The analysis progress is set even for hits with errors", E6.HasAnalysisProgress(MAssembly::c_DepthCorrection)) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "missing coefficients", "Hits without depth still get the strip position", H1->GetLocalPosition().X(), 0.25*(1 - 4.5), 1e-9) && Passed;

  Passed = EvaluateTrue("AnalyzeEvent()", "guard ring veto", "A depth calibration error is set for vetoed events", E7.HasDepthCalibrationError()) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "guard ring veto", "AnalyzeEvent returns true for vetoed events", R7)) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "guard ring veto", "The analysis progress is set for vetoed events", E7.HasAnalysisProgress(MAssembly::c_DepthCorrection))) && Passed;
  (void) H7;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestAnalyzeEventDisabledStrips()
{
  bool Passed = true;

  UTNModuleDepthCalibrationAccess M;
  if (InitializeStandard(M, true) == false) {
    return EvaluateTrue("Initialize()", "with disabled strips", "Initialize succeeds for the disabled strip test", false);
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  // LV strip 5 is read out together with LV strip 6: X at the center of strips 5 and 6, sigma for a strip of double width
  MReadOutAssembly E1;
  MHit* H1 = AddSimpleHit(E1, 5, 10, 40.0);
  M.AnalyzeEvent(&E1);

  // HV strip 3 is read out together with HV strip 4
  MReadOutAssembly E2;
  MHit* H2 = AddSimpleHit(E2, 2, 3, 40.0);
  M.AnalyzeEvent(&E2);

  // LV strip 7 is dead
  MReadOutAssembly E3;
  MHit* H3 = AddSimpleHit(E3, 7, 10, 40.0);
  M.AnalyzeEvent(&E3);

  // HV strip 4 is shorted (not read out itself)
  MReadOutAssembly E4;
  MHit* H4 = AddSimpleHit(E4, 2, 4, 40.0);
  M.AnalyzeEvent(&E4);

  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("AnalyzeEvent()", "shorted LV 5+6", "The hit has a depth", H1->GetNoDepth()) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "shorted LV 5+6", "X is the center of LV strips 5 and 6", H1->GetLocalPosition().X(), 0.25*(5.5 - 4.5), 1e-9) && Passed;
  Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "shorted LV 5+6", "The X resolution is that of a double-wide strip", H1->GetPositionResolution().X(), 2*0.25/sqrt(12.0), 1e-9)) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "shorted HV 3+4", "The hit has a depth", H2->GetNoDepth()) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "shorted HV 3+4", "Y is the center of HV strips 3 and 4", H2->GetLocalPosition().Y(), 0.1*(3.5 - 19.5), 1e-9) && Passed;
  Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "shorted HV 3+4", "The Y resolution is that of a double-wide strip with the X pitch", H2->GetPositionResolution().Y(), 2*0.1/sqrt(12.0), 1e-9)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "dead LV 7", "The hit has no depth", H3->GetNoDepth()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "dead LV 7", "A depth calibration error is set", E3.HasDepthCalibrationError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "disabled HV 4", "The hit has no depth", H4->GetNoDepth()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "disabled HV 4", "A depth calibration error is set", E4.HasDepthCalibrationError()) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDepthCalibration::TestCrashCandidates()
{
  bool Passed = true;

  // Initialize() without a geometry must fail gracefully
  bool NoGeometry = RunInChild([this]() -> bool {
    EnsureEnergyCalibrationAvailable();
    UTNModuleDepthCalibrationAccess M;
    M.SetCoeffsFileName(WriteCoeffs());
    M.SetSplinesFileName(WriteSplines());
    M.SetAccountForDisabledStrips(false);
    M.SetMaskMetrologyCorrectionEnable(false);
    M.SetUCSDOverride(false);
    return M.Initialize() == false;
  });
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "no geometry", "Initialize without geometry returns false without crashing", NoGeometry)) && Passed;

  // A hit in a detector which is not in the geometry must not crash the analysis
  bool UnknownDetector = RunInChild([this]() -> bool {
    UTNModuleDepthCalibrationAccess M;
    if (InitializeStandard(M) == false) return false;
    MReadOutAssembly E;
    MHit* H = AddSimpleHit(E, 5, 3, 40.0, 3);
    M.AnalyzeEvent(&E);
    return H->GetNoDepth() == true && E.HasDepthCalibrationError() == true;
  });
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "hit in unknown detector 3", "A hit in an unknown detector is flagged without crashing", UnknownDetector)) && Passed;

  // A ### header line with fewer than six tokens must not crash the coefficients reader
  // The reader reads Tokens[5] out of bounds: whether that crashes is platform dependent (it does on Linux, not on macOS),
  // thus there is no separate "does not crash" check; the energy check below fails on all platforms until the fix
  MString ShortHeader = GetTemporaryFileName("shortheader.coeffs.csv");
  Passed = EvaluateTrue("WriteTextFile()", "ShortHeader", "The fixture file can be written", WriteTextFile(ShortHeader, "### 800 V\n503,1.0,0.0,1.0,1.0\n")) && Passed;

  // The energy is the sixth token of the header: a short header must not read past the tokens
  bool EnergyKept = RunInChild([&]() -> bool {
    UTNModuleDepthCalibrationAccess M;
    M.LoadCoeffsFile(ShortHeader);
    return M.GetCoeffsEnergy() == 59.5;
  });
  Passed = ExpectedToFail(EvaluateTrue("LoadCoeffsFile()", "short ### header", "A short ### header keeps the default energy of 59.5 keV", EnergyKept)) && Passed;

  // A bare ### line in the splines file is not tested: LoadSplinesFile() reads tokens[1] out of bounds,
  // so the result is random until that is fixed. Add the check "a ### line without detector ID is rejected" with the fix.

  // A splines file with only a depth column has no CTD relation
  MString DepthOnly = GetTemporaryFileName("depthonly.splines.csv");
  Passed = EvaluateTrue("WriteTextFile()", "DepthOnly", "The fixture file can be written", WriteTextFile(DepthOnly, "### 0 800 80 662\n-0.75\n0.0\n0.75\n")) && Passed;
  MDGeometryQuest* Geometry = GetGeometry();
  MString Coeffs = WriteCoeffs();
  EnsureEnergyCalibrationAvailable();
  bool DepthOnlyOK = RunInChild([&]() -> bool {
    UTNModuleDepthCalibrationAccess M;
    M.SetGeometry(Geometry);
    M.SetCoeffsFileName(Coeffs);
    M.SetSplinesFileName(DepthOnly);
    M.SetAccountForDisabledStrips(false);
    M.SetMaskMetrologyCorrectionEnable(false);
    M.SetUCSDOverride(false);
    if (M.Initialize() == false) return true;
    MReadOutAssembly E;
    MHit* H = AddSimpleHit(E, 5, 3, 40.0);
    M.AnalyzeEvent(&E);
    return H->GetNoDepth() == true;
  });
  Passed = ExpectedToFail(EvaluateTrue("Initialize()/AnalyzeEvent()", "splines file with depth column only", "A splines file without CTD column is rejected, or the hit gets no depth, without crashing", DepthOnlyOK)) && Passed;

  // With the mask metrology enabled, a hit on a strip missing in the metrology file must not crash the analysis
  MString PartialMetrology = GetTemporaryFileName("partial_metrology.csv");
  Passed = EvaluateTrue("WriteTextFile()", "PartialMetrology", "The fixture file can be written", WriteTextFile(PartialMetrology, "0,l,5,10,5,0,0,0,1\n0,h,3,3,20,0,0,0,90\n")) && Passed;
  bool MissingStripOK = RunInChild([&]() -> bool {
    UTNModuleDepthCalibrationAccess M;
    if (InitializeStandard(M) == false) return false;
    if (M.CallLoadMaskMetrologyFile(PartialMetrology) == false) return false;
    M.SetMaskMetrologyCorrectionEnable(true);
    MReadOutAssembly E;
    MHit* H = AddSimpleHit(E, 7, 3, 40.0);
    M.AnalyzeEvent(&E);
    MVector P = H->GetPosition();
    return std::isfinite(P.X()) == true && std::isfinite(P.Y()) == true && std::isfinite(P.Z()) == true;
  });
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "mask metrology without LV strip 7", "A hit on a strip missing in the metrology file is handled without crashing and with a finite position", MissingStripOK)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleDepthCalibration Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleDepthCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
