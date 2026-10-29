/*
 * UTNSubModuleStripReadout.cxx
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
#include <list>
#include <new>
using namespace std;

// ROOT libs:
#include "TRandom.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MSubModuleStripReadout.h"
#include "MReadOutAssembly.h"
#include "MDEEStripHit.h"


//! Unit test class for MSubModuleStripReadout
class UTNSubModuleStripReadout : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleStripReadout() : MUnitTest("UTNSubModuleStripReadout") {}
  //! Default destructor
  virtual ~UTNSubModuleStripReadout() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the defaults and the setters/getters
  bool TestDefaults();
  //! Test the XML configuration round trip
  bool TestXmlRoundTrip();
  //! Test the Initialize() error paths
  bool TestInitialize();
  //! Test the energy to ADC conversion and the thresholds
  bool TestADCAndThresholds();
  //! Test the nearest neighbor handling
  bool TestNearestNeighbors();
  //! Test strips without calibration
  bool TestUncalibratedStrips();
  //! Test that created nearest-neighbor hits carry the identifiers of their triggering strip
  bool TestNearestNeighborIdentifiers();
  //! Test the guard ring neighbor of a triggered strip 63
  bool TestGuardRingNeighbor();
  //! Test the energy resolution smearing
  bool TestResolution();

  //! Write the energy calibration and return its name, or an empty string on failure
  MString WriteCalibration(unsigned int NCalibratedStrips = 60);
  //! Write the hardware thresholds and return the name, or an empty string on failure
  MString WriteThresholds(unsigned int NStrips = 64);
  //! Initialize a module with the standard files
  bool InitializeStandard(MSubModuleStripReadout& M, bool ApplyResolution);
  //! Create a DEE strip hit
  MDEEStripHit CreateHit(unsigned int StripID, bool IsLowVoltage, double Energy, bool IsGuardRing = false);
  //! Return the hit on the given strip or nullptr
  MDEEStripHit* FindStrip(list<MDEEStripHit>& Hits, unsigned int StripID);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::Run()
{
  bool Passed = true;

  Passed = TestDefaults() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestADCAndThresholds() && Passed;
  Passed = TestNearestNeighbors() && Passed;
  Passed = TestUncalibratedStrips() && Passed;
  Passed = TestNearestNeighborIdentifiers() && Passed;
  Passed = TestGuardRingNeighbor() && Passed;
  Passed = TestResolution() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleStripReadout::WriteCalibration(unsigned int NCalibratedStrips)
{
  // Detector 0, LV and HV strips 0 to NCalibratedStrips-1: E = 0.5 * ADC, FWHM = 1 + 0.002 * E; by default strips 60-63 have no calibration
  MString C;
  for (unsigned int s = 0; s < NCalibratedStrips; ++s) {
    for (int Side = 0; Side < 2; ++Side) {
      MString S = (Side == 0) ? "l" : "h";
      C += MString("CP dss 0 ") + s + " " + S + " pakw 2 200 100 1 400 200 1\n";
      C += MString("CM dss 0 ") + s + " " + S + " poly1 0 0.5\n";
      C += MString("CR dss 0 ") + s + " " + S + " p1 1.0 0.002\n";
    }
  }
  MString FileName = GetTemporaryFileName("sr.ecal");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleStripReadout::WriteThresholds(unsigned int NStrips)
{
  // Hardware threshold 1600 ADC (800 keV) for all strips
  MString C = "STRIP_ID,Detector,side,strip,threshold_adc,threshold_keV\n";
  for (unsigned int s = 0; s < NStrips; ++s) {
    C += MString("") + s + ",0,l," + s + ",1600,800.0\n";
    C += MString("") + (100 + s) + ",0,h," + s + ",1600,800.0\n";
  }
  MString FileName = GetTemporaryFileName("sr.thresholds.csv");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::InitializeStandard(MSubModuleStripReadout& M, bool ApplyResolution)
{
  M.SetEnergyCalibrationFileName(WriteCalibration());
  M.SetHardwareThresholdFileName(WriteThresholds());
  M.SetApplyResolutionCalibration(ApplyResolution);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Result = M.Initialize();
  g_Verbosity = OldVerbosity;
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


MDEEStripHit UTNSubModuleStripReadout::CreateHit(unsigned int StripID, bool IsLowVoltage, double Energy, bool IsGuardRing)
{
  MDEEStripHit H;
  H.m_ROE.SetDetectorID(0);
  H.m_ROE.SetStripID(StripID);
  H.m_ROE.IsLowVoltageStrip(IsLowVoltage);
  H.m_Energy = Energy;
  H.m_IsGuardRing = IsGuardRing;
  H.m_IsNearestNeighbor = false;
  return H;
}


////////////////////////////////////////////////////////////////////////////////


MDEEStripHit* UTNSubModuleStripReadout::FindStrip(list<MDEEStripHit>& Hits, unsigned int StripID)
{
  for (MDEEStripHit& H : Hits) {
    if (H.m_ROE.GetStripID() == StripID) return &H;
  }
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::TestDefaults()
{
  bool Passed = true;

  MSubModuleStripReadout M;
  Passed = Evaluate("GetEnergyCalibrationFileName()", "default", "The default file name is empty", M.GetEnergyCalibrationFileName(), MString("")) && Passed;
  Passed = Evaluate("GetHardwareThresholdFileName()", "default", "The default file name is empty", M.GetHardwareThresholdFileName(), MString("")) && Passed;

  M.SetEnergyCalibrationFileName("e.ecal");
  M.SetHardwareThresholdFileName("t.csv");
  Passed = Evaluate("SetEnergyCalibrationFileName/GetEnergyCalibrationFileName", "e.ecal", "Returns the set name", M.GetEnergyCalibrationFileName(), MString("e.ecal")) && Passed;
  Passed = Evaluate("SetHardwareThresholdFileName/GetHardwareThresholdFileName", "t.csv", "Returns the set name", M.GetHardwareThresholdFileName(), MString("t.csv")) && Passed;
  M.SetApplyResolutionCalibration(true);
  Passed = EvaluateTrue("SetApplyResolutionCalibration/GetApplyResolutionCalibration", "true", "Returns true", M.GetApplyResolutionCalibration()) && Passed;
  M.SetApplyResolutionCalibration(false);
  Passed = EvaluateFalse("SetApplyResolutionCalibration/GetApplyResolutionCalibration", "false", "Returns false", M.GetApplyResolutionCalibration()) && Passed;

  // GCC removes the memory fills before the constructor as dead stores (-flifetime-dse), which leaves random bytes: not checked with GCC
#if !defined(__GNUC__) || defined(__clang__)
  // The resolution flag must be initialized by the constructor: construct into memory filled with 0x00 and with 0xFF
  alignas(MSubModuleStripReadout) unsigned char Zero[sizeof(MSubModuleStripReadout)];
  alignas(MSubModuleStripReadout) unsigned char Ones[sizeof(MSubModuleStripReadout)];
  memset(Zero, 0x00, sizeof(Zero));
  memset(Ones, 0xFF, sizeof(Ones));
  MSubModuleStripReadout* A = new (Zero) MSubModuleStripReadout;
  MSubModuleStripReadout* B = new (Ones) MSubModuleStripReadout;
  bool ValueA = A->GetApplyResolutionCalibration();
  bool ValueB = B->GetApplyResolutionCalibration();
  Passed = ExpectedToFail(EvaluateTrue("MSubModuleStripReadout()", "uninitialized memory", "The resolution flag has a defined default independent of the memory content", ValueA == ValueB)) && Passed;
  A->~MSubModuleStripReadout();
  B->~MSubModuleStripReadout();
#endif

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::TestXmlRoundTrip()
{
  bool Passed = true;

  MSubModuleStripReadout Writer;
  Writer.SetEnergyCalibrationFileName("/data/e.ecal");
  Writer.SetHardwareThresholdFileName("/data/t.csv");
  MXmlNode* Node = new MXmlNode(nullptr, "DEE");
  Writer.CreateXmlConfiguration(Node);
  MSubModuleStripReadout Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "file names", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "EnergyCalibrationFileName", "The energy calibration file name survives", Reader.GetEnergyCalibrationFileName(), MString("/data/e.ecal")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "HardwareThresholdFileName", "The threshold file name survives", Reader.GetHardwareThresholdFileName(), MString("/data/t.csv")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::TestInitialize()
{
  bool Passed = true;

  MString Thresholds = WriteThresholds();
  Passed = EvaluateFalse("WriteThresholds()", "threshold file", "The fixture file is written", Thresholds.IsEmpty()) && Passed;
  MString Calibration = WriteCalibration();
  Passed = EvaluateFalse("WriteCalibration()", "calibration file", "The fixture file is written", Calibration.IsEmpty()) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MSubModuleStripReadout NoName;
  NoName.SetHardwareThresholdFileName(Thresholds);
  bool NoNameResult = NoName.Initialize();

  MSubModuleStripReadout Missing;
  Missing.SetEnergyCalibrationFileName(GetTemporaryFileName("missing.ecal"));
  Missing.SetHardwareThresholdFileName(Thresholds);
  bool MissingResult = Missing.Initialize();

  MSubModuleStripReadout MissingThresholds;
  MissingThresholds.SetEnergyCalibrationFileName(Calibration);
  MissingThresholds.SetHardwareThresholdFileName(GetTemporaryFileName("missing.csv"));
  bool MissingThresholdsResult = MissingThresholds.Initialize();

  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "no calibration file name", "Initialize returns false", NoNameResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing calibration file", "Initialize returns false", MissingResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing threshold file", "Initialize returns false", MissingThresholdsResult) && Passed;

  MSubModuleStripReadout Good;
  Passed = EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", InitializeStandard(Good, false)) && Passed;
  Good.Finalize();
  Passed = EvaluateTrue("Initialize()", "after Finalize", "A second Initialize succeeds", InitializeStandard(Good, false)) && Passed;
  Good.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::TestADCAndThresholds()
{
  bool Passed = true;

  MSubModuleStripReadout M;
  if (InitializeStandard(M, false) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(10, true, 1000.0));   // ADC 2000: triggered
  E.AddDEEStripHitLV(CreateHit(30, true, 9000.0));   // ADC 18000: saturated
  E.AddDEEStripHitHV(CreateHit(20, false, 2000.0));  // ADC 4000: triggered
  Passed = EvaluateTrue("AnalyzeEvent()", "three triggered strips", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;

  list<MDEEStripHit>& LV = E.GetDEEStripHitLVListReference();
  list<MDEEStripHit>& HV = E.GetDEEStripHitHVListReference();
  MDEEStripHit* S10 = FindStrip(LV, 10);
  MDEEStripHit* S30 = FindStrip(LV, 30);
  MDEEStripHit* H20 = FindStrip(HV, 20);
  if (S10 != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "LV strip 10, 1000 keV", "The ADC value is 1000 / 0.5 = 2000", S10->m_ADC, 2000u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "LV strip 10, 1000 keV", "The strip is above the hardware threshold", S10->m_IsNearestNeighbor) && Passed;
  } else {
    Passed = EvaluateTrue("AnalyzeEvent()", "LV strip 10", "The strip hit exists", false) && Passed;
  }
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "LV strip 30, 9000 keV", "The saturated strip hit exists", S30 != nullptr)) && Passed;
  if (S30 != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "LV strip 30, 9000 keV", "The ADC value saturates at 16383", S30->m_ADC, 16383u) && Passed;
  }
  if (H20 != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", "HV strip 20, 2000 keV", "The ADC value is 4000", H20->m_ADC, 4000u) && Passed;
  } else {
    Passed = EvaluateTrue("AnalyzeEvent()", "HV strip 20", "The strip hit exists", false) && Passed;
  }

  // Clear() between events keeps the run configuration: the same strip gives the same ADC value
  M.Clear();
  MReadOutAssembly Next;
  Next.AddDEEStripHitLV(CreateHit(10, true, 1000.0));
  Passed = EvaluateTrue("AnalyzeEvent()", "after Clear()", "AnalyzeEvent returns true", M.AnalyzeEvent(&Next)) && Passed;
  MDEEStripHit* NextS10 = FindStrip(Next.GetDEEStripHitLVListReference(), 10);
  Passed = EvaluateTrue("Clear()", "next event, LV strip 10", "The strip hit exists", NextS10 != nullptr) && Passed;
  if (NextS10 != nullptr) {
    Passed = Evaluate("Clear()", "next event, LV strip 10, 1000 keV", "The ADC value is still 2000", NextS10->m_ADC, 2000u) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::TestNearestNeighborIdentifiers()
{
  bool Passed = true;

  // MSubModuleDepthReadout builds the pixel code of every hit from m_OppositeStripID, also for added nearest neighbors
  // The defaults of MDEEStripHit are tested in UTNDEEStripHit
  MSubModuleStripReadout M;
  if (InitializeStandard(M, false) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  MReadOutAssembly E;
  MDEEStripHit Triggered = CreateHit(10, true, 1000.0);
  Triggered.m_OppositeStripID = 33;
  E.AddDEEStripHitLV(Triggered);
  M.AnalyzeEvent(&E);
  list<MDEEStripHit>& LV = E.GetDEEStripHitLVListReference();
  MDEEStripHit* S9 = FindStrip(LV, 9);
  MDEEStripHit* S11 = FindStrip(LV, 11);
  Passed = EvaluateTrue("AnalyzeEvent()", "triggered LV strip 10", "An empty neighbor hit is added for strip 9", S9 != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "triggered LV strip 10", "An empty neighbor hit is added for strip 11", S11 != nullptr) && Passed;
  if (S9 != nullptr) {
    Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "added neighbor strip 9", "The added hit has the opposite strip of its triggering strip 10", S9->m_OppositeStripID, 33)) && Passed;
  }
  if (S11 != nullptr) {
    Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "added neighbor strip 11", "The added hit has the opposite strip of its triggering strip 10", S11->m_OppositeStripID, 33)) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::TestGuardRingNeighbor()
{
  bool Passed = true;

  // All strips including 63 and the guard ring 64 are calibrated, as in the dummy DEE calibration files
  MSubModuleStripReadout M;
  M.SetEnergyCalibrationFileName(WriteCalibration(65));
  M.SetHardwareThresholdFileName(WriteThresholds(65));
  M.SetApplyResolutionCalibration(false);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Init = M.Initialize();
  g_Verbosity = OldVerbosity;
  if (Init == false) {
    return EvaluateTrue("Initialize()", "strips 0-64 calibrated", "Initialize succeeds", false);
  }

  // MSubModuleChargeTransport creates the right neighbor of a strip 63 hit as a guard ring hit on strip 64
  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(63, true, 1000.0));
  E.AddDEEStripHitLV(CreateHit(64, true, 0.0, true));
  M.AnalyzeEvent(&E);
  list<MDEEStripHit>& LV = E.GetDEEStripHitLVListReference();
  Passed = EvaluateTrue("AnalyzeEvent()", "triggered LV strip 63", "The triggered strip 63 is kept", FindStrip(LV, 63) != nullptr) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "sub-threshold guard ring next to strip 63", "A sub-threshold guard ring hit is removed (a kept strip-64 hit raises the guard ring veto)", FindStrip(LV, 64) == nullptr)) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::TestNearestNeighbors()
{
  bool Passed = true;

  MSubModuleStripReadout M;
  if (InitializeStandard(M, false) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(10, true, 1000.0));  // triggered
  E.AddDEEStripHitLV(CreateHit(11, true, 100.0));   // ADC 200: sub-threshold neighbor of 10 -> kept as nearest neighbor
  E.AddDEEStripHitLV(CreateHit(40, true, 100.0));   // ADC 200: sub-threshold without triggered neighbor -> removed
  M.AnalyzeEvent(&E);
  list<MDEEStripHit>& LV = E.GetDEEStripHitLVListReference();

  MDEEStripHit* S11 = FindStrip(LV, 11);
  MDEEStripHit* S9 = FindStrip(LV, 9);
  Passed = EvaluateTrue("AnalyzeEvent()", "sub-threshold strip 11 next to 10", "The neighbor strip is kept", S11 != nullptr) && Passed;
  if (S11 != nullptr) {
    Passed = EvaluateTrue("AnalyzeEvent()", "sub-threshold strip 11 next to 10", "The neighbor strip is flagged as nearest neighbor", S11->m_IsNearestNeighbor) && Passed;
    // TF1::GetX() returns 199.99999 and the cast truncates it (MSubModuleStripReadout.cxx:176); depends on the root finder precision
    Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "sub-threshold strip 11 next to 10", "The neighbor ADC is 200", S11->m_ADC, 200u)) && Passed;
  }
  Passed = EvaluateTrue("AnalyzeEvent()", "isolated sub-threshold strip 40", "An isolated sub-threshold strip is removed", FindStrip(LV, 40) == nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "strip 9 next to 10", "An empty neighbor hit is added for strip 9", S9 != nullptr) && Passed;
  if (S9 != nullptr) {
    Passed = EvaluateTrue("AnalyzeEvent()", "empty neighbor strip 9", "The added hit is a nearest neighbor", S9->m_IsNearestNeighbor) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "empty neighbor strip 9", "The added hit has zero energy", S9->m_Energy, 0.0, 1e-12) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "empty neighbor strip 9", "The added hit has ADC 0", S9->m_ADC, 0u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "empty neighbor strip 9", "The added hit has no fast timing", S9->m_HasFastTiming) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "strips 10 and 11", "Exactly three LV hits remain: 9, 10, 11", (unsigned int) LV.size(), 3u) && Passed;

  // Guard ring hits do not get nearest neighbors
  MReadOutAssembly G;
  G.AddDEEStripHitHV(CreateHit(5, false, 1000.0, true));
  M.AnalyzeEvent(&G);
  Passed = Evaluate("AnalyzeEvent()", "triggered guard ring", "No nearest neighbors are added for a guard ring hit", (unsigned int) G.GetDEEStripHitHVListReference().size(), 1u) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::TestUncalibratedStrips()
{
  bool Passed = true;

  MSubModuleStripReadout M;
  if (InitializeStandard(M, false) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateHit(61, true, 1000.0));  // no calibration: removed
  E.AddDEEStripHitLV(CreateHit(59, true, 1000.0));  // calibrated, its right neighbor 60 has no calibration
  M.AnalyzeEvent(&E);
  g_Verbosity = OldVerbosity;
  list<MDEEStripHit>& LV = E.GetDEEStripHitLVListReference();
  Passed = EvaluateTrue("AnalyzeEvent()", "uncalibrated strip 61", "A strip without calibration is removed", FindStrip(LV, 61) == nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "strip 59", "The calibrated strip is kept", FindStrip(LV, 59) != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "neighbor strip 60 without calibration", "No neighbor hit is added for an uncalibrated strip", FindStrip(LV, 60) == nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "neighbor strip 58", "The calibrated neighbor 58 is added", FindStrip(LV, 58) != nullptr) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadout::TestResolution()
{
  bool Passed = true;

  MSubModuleStripReadout M;
  if (InitializeStandard(M, true) == false) {
    return EvaluateTrue("Initialize()", "standard files", "Initialize succeeds", false);
  }

  // FWHM at 1000 keV is 3 keV => sigma = 3 / 2.355
  gRandom->SetSeed(4711);
  const unsigned int N = 4000;
  double Sum = 0;
  double Sum2 = 0;
  for (unsigned int i = 0; i < N; ++i) {
    MReadOutAssembly E;
    E.AddDEEStripHitLV(CreateHit(10, true, 1000.0));
    M.AnalyzeEvent(&E);
    MDEEStripHit* S = FindStrip(E.GetDEEStripHitLVListReference(), 10);
    double En = (S != nullptr) ? S->m_Energy : 0.0;
    Sum += En;
    Sum2 += En*En;
  }
  double Mean = Sum/N;
  double Sigma = sqrt(Sum2/N - Mean*Mean);
  Passed = EvaluateNear("AnalyzeEvent()", "resolution on, 1000 keV", "The smeared energies are centered on 1000 keV", Mean, 1000.0, 0.1) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "resolution on, 1000 keV", "The width of the smeared energies is FWHM / 2.355", Sigma, 3.0/2.355, 0.1) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleStripReadout Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleStripReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
