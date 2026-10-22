/*
 * UTNModuleStripPairingChiSquare.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleStripPairingChiSquare.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MHit.h"


//! Unit test class for MModuleStripPairingChiSquare
class UTNModuleStripPairingChiSquare : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleStripPairingChiSquare() : MUnitTest("UTNModuleStripPairingChiSquare") {}
  //! Default destructor
  virtual ~UTNModuleStripPairingChiSquare() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data
  bool TestDefaultConstruction();
  //! Test the (empty) XML configuration
  bool TestXmlConfiguration();
  //! Test the pairing of one interaction
  bool TestSingleInteraction();
  //! Test the pairing of two interactions
  bool TestTwoInteractions();
  //! Test charge sharing between adjacent strips
  bool TestChargeSharing();
  //! Test the energy selection of a hit
  bool TestHitEnergy();
  //! Test multiple detectors
  bool TestMultipleDetectors();
  //! Test the error cases
  bool TestErrors();
  //! Test that nearest-neighbor and guard ring strip hits are not paired as interactions
  bool TestNearestNeighbors();

  //! Add a strip hit to the event
  MStripHit* AddStripHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, double Energy, double Resolution = 2.0);
  //! Return the hit with the energy closest to the given one
  MHit* FindHit(MReadOutAssembly& Event, double Energy);
  //! Return true if the hit contains the strip hit
  bool HasStripHit(MHit* Hit, MStripHit* StripHit);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestXmlConfiguration() && Passed;
  Passed = TestSingleInteraction() && Passed;
  Passed = TestTwoInteractions() && Passed;
  Passed = TestChargeSharing() && Passed;
  Passed = TestHitEnergy() && Passed;
  Passed = TestMultipleDetectors() && Passed;
  Passed = TestErrors() && Passed;
  Passed = TestNearestNeighbors() && Passed;

  // ShowOptionsGUI() and CreateExpos() create GUI elements and are not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleStripPairingChiSquare::AddStripHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, double Energy, double Resolution)
{
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(DetID);
  SH->SetStripID(StripID);
  SH->IsLowVoltageStrip(IsLowVoltage);
  SH->SetEnergy(Energy);
  SH->SetEnergyResolution(Resolution);
  Event.AddStripHit(SH);
  return SH;
}


////////////////////////////////////////////////////////////////////////////////


MHit* UTNModuleStripPairingChiSquare::FindHit(MReadOutAssembly& Event, double Energy)
{
  MHit* Best = nullptr;
  for (unsigned int h = 0; h < Event.GetNHits(); ++h) {
    if (Best == nullptr || fabs(Event.GetHit(h)->GetEnergy() - Energy) < fabs(Best->GetEnergy() - Energy)) {
      Best = Event.GetHit(h);
    }
  }
  return Best;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::HasStripHit(MHit* Hit, MStripHit* StripHit)
{
  if (Hit == nullptr) return false;
  for (unsigned int s = 0; s < Hit->GetNStripHits(); ++s) {
    if (Hit->GetStripHit(s) == StripHit) return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleStripPairingChiSquare M;
  Passed = Evaluate("GetName()", "default", "The module name is 'Strip pairing - chi square approach'", M.GetName(), MString("Strip pairing - chi square approach")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagStripPairingChiSquare'", M.GetXmlTag(), MString("XmlTagStripPairingChiSquare")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_StripPairing", M.ProvidesModuleType(MAssembly::c_StripPairing)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "An event loader is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The energy calibration is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EnergyCalibration)) && Passed;
  Passed = EvaluateFalse("HasOptionsGUI()", "default", "The module has no options GUI", M.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "default", "The module allows multi-threading", M.AllowsMultiThreading()) && Passed;
  Passed = EvaluateTrue("AllowsMultipleInstances()", "default", "The module allows multiple instances", M.AllowsMultipleInstances()) && Passed;

  MModuleStripPairingChiSquare* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::TestXmlConfiguration()
{
  bool Passed = true;

  MModuleStripPairingChiSquare M;
  MXmlNode* Node = M.CreateXmlConfiguration();
  Passed = EvaluateTrue("CreateXmlConfiguration()", "default", "A node is created", Node != nullptr) && Passed;
  if (Node != nullptr) {
    Passed = Evaluate("CreateXmlConfiguration()", "default", "The node name is the XML tag", Node->GetName(), MString("XmlTagStripPairingChiSquare")) && Passed;
    Passed = EvaluateTrue("ReadXmlConfiguration()", "own node", "ReadXmlConfiguration returns true", M.ReadXmlConfiguration(Node)) && Passed;
  }
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::TestSingleInteraction()
{
  bool Passed = true;

  MModuleStripPairingChiSquare M;
  M.Initialize();

  MReadOutAssembly E;
  MStripHit* LV = AddStripHit(E, 0, 10, true, 662.0);
  MStripHit* HV = AddStripHit(E, 0, 20, false, 660.0);
  Passed = EvaluateTrue("AnalyzeEvent()", "1 LV, 1 HV", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "1 LV, 1 HV", "No strip pairing error is set", E.HasStripPairingError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "1 LV, 1 HV", "The c_StripPairing analysis progress is set", E.HasAnalysisProgress(MAssembly::c_StripPairing)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "1 LV, 1 HV", "One hit is created", E.GetNHits(), 1u) && Passed;
  if (E.GetNHits() == 1) {
    MHit* H = E.GetHit(0);
    Passed = Evaluate("AnalyzeEvent()", "1 LV, 1 HV", "The hit has two strip hits", H->GetNStripHits(), 2u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "1 LV, 1 HV", "The hit contains the LV strip hit", HasStripHit(H, LV)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "1 LV, 1 HV", "The hit contains the HV strip hit", HasStripHit(H, HV)) && Passed;
    // Equal resolutions: the energy is the mean of both sides, the resolution 2/sqrt(2)
    Passed = EvaluateNear("AnalyzeEvent()", "1 LV, 1 HV", "The hit energy is the weighted mean 661 keV", H->GetEnergy(), 661.0, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "1 LV, 1 HV", "The hit energy resolution is 2/sqrt(2)", H->GetEnergyResolution(), 2.0/sqrt(2.0), 1e-9) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "1 LV, 1 HV", "The strip hits stay in the event", E.GetNStripHits(), 2u) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::TestTwoInteractions()
{
  bool Passed = true;

  MModuleStripPairingChiSquare M;
  M.Initialize();

  MReadOutAssembly E;
  MStripHit* LV1 = AddStripHit(E, 0, 10, true, 300.0);
  MStripHit* LV2 = AddStripHit(E, 0, 30, true, 500.0);
  MStripHit* HV1 = AddStripHit(E, 0, 5, false, 501.0);
  MStripHit* HV2 = AddStripHit(E, 0, 25, false, 299.0);
  Passed = EvaluateTrue("AnalyzeEvent()", "2 LV, 2 HV", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "2 LV, 2 HV", "Two hits are created", E.GetNHits(), 2u) && Passed;
  if (E.GetNHits() == 2) {
    MHit* H300 = FindHit(E, 300.0);
    MHit* H500 = FindHit(E, 500.0);
    Passed = EvaluateTrue("AnalyzeEvent()", "2 LV, 2 HV", "The 300 keV hit contains LV 10", HasStripHit(H300, LV1)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "2 LV, 2 HV", "The 300 keV hit contains HV 25", HasStripHit(H300, HV2)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "2 LV, 2 HV", "The 500 keV hit contains LV 30", HasStripHit(H500, LV2)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "2 LV, 2 HV", "The 500 keV hit contains HV 5", HasStripHit(H500, HV1)) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "2 LV, 2 HV", "The 300 keV hit energy is 299.5 keV", H300->GetEnergy(), 299.5, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "2 LV, 2 HV", "The 500 keV hit energy is 500.5 keV", H500->GetEnergy(), 500.5, 1e-9) && Passed;
    // Both hits consist of one LV and one HV strip with 2 keV resolution each
    Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "2 LV, 2 HV", "The resolution of the 300 keV hit is 2/sqrt(2)", H300->GetEnergyResolution(), 2.0/sqrt(2.0), 1e-9)) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "2 LV, 2 HV", "The resolution of the 500 keV hit is 2/sqrt(2)", H500->GetEnergyResolution(), 2.0/sqrt(2.0), 1e-9) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::TestChargeSharing()
{
  bool Passed = true;

  MModuleStripPairingChiSquare M;
  M.Initialize();

  MReadOutAssembly E;
  MStripHit* LV1 = AddStripHit(E, 0, 10, true, 200.0);
  MStripHit* LV2 = AddStripHit(E, 0, 11, true, 100.0);
  MStripHit* HV = AddStripHit(E, 0, 5, false, 300.0);
  Passed = EvaluateTrue("AnalyzeEvent()", "LV 10+11, HV 5", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "LV 10+11, HV 5", "One hit is created", E.GetNHits(), 1u) && Passed;
  if (E.GetNHits() == 1) {
    MHit* H = E.GetHit(0);
    Passed = Evaluate("AnalyzeEvent()", "LV 10+11, HV 5", "The hit has three strip hits", H->GetNStripHits(), 3u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "LV 10+11, HV 5", "The hit contains LV 10", HasStripHit(H, LV1)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "LV 10+11, HV 5", "The hit contains LV 11", HasStripHit(H, LV2)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "LV 10+11, HV 5", "The hit contains HV 5", HasStripHit(H, HV)) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "LV 10+11, HV 5", "The hit energy is 300 keV", H->GetEnergy(), 300.0, 1e-9) && Passed;
  }

  // Non-adjacent strips with the same total energy cannot be merged: two LV hits, one HV hit
  MReadOutAssembly N;
  AddStripHit(N, 0, 10, true, 200.0);
  AddStripHit(N, 0, 20, true, 100.0);
  AddStripHit(N, 0, 5, false, 300.0);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  M.AnalyzeEvent(&N);
  g_Verbosity = OldVerbosity;
  bool Merged = false;
  for (unsigned int h = 0; h < N.GetNHits(); ++h) {
    if (N.GetHit(h)->GetNStripHits() == 3) Merged = true;
  }
  Passed = EvaluateFalse("AnalyzeEvent()", "LV 10 and 20, HV 5", "Non-adjacent strips are not merged into one hit", Merged) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::TestHitEnergy()
{
  bool Passed = true;

  MModuleStripPairingChiSquare M;
  M.Initialize();

  // Different resolutions: inverse variance weighting
  {
    MReadOutAssembly E;
    AddStripHit(E, 0, 10, true, 100.0, 1.0);
    AddStripHit(E, 0, 20, false, 102.0, 2.0);
    M.AnalyzeEvent(&E);
    if (E.GetNHits() == 1) {
      double Expected = (100.0/1.0 + 102.0/4.0)/(1.0/1.0 + 1.0/4.0);
      Passed = EvaluateNear("AnalyzeEvent()", "100 +- 1 and 102 +- 2", "The energy is the inverse-variance weighted mean", E.GetHit(0)->GetEnergy(), Expected, 1e-9) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "100 +- 1 and 102 +- 2", "The resolution is the combined resolution", E.GetHit(0)->GetEnergyResolution(), sqrt(1.0/(1.0 + 0.25)), 1e-9) && Passed;
    } else {
      Passed = Evaluate("AnalyzeEvent()", "100 +- 1 and 102 +- 2", "One hit is created", E.GetNHits(), 1u) && Passed;
    }
  }

  // The second hit must have the same resolution as an identical first hit
  {
    MReadOutAssembly E;
    AddStripHit(E, 0, 10, true, 300.0, 2.0);
    AddStripHit(E, 0, 30, true, 300.0, 2.0);
    AddStripHit(E, 0, 5, false, 300.0, 2.0);
    AddStripHit(E, 0, 25, false, 300.0, 2.0);
    M.AnalyzeEvent(&E);
    Passed = Evaluate("AnalyzeEvent()", "two identical interactions", "Two hits are created", E.GetNHits(), 2u) && Passed;
    if (E.GetNHits() == 2) {
      Passed = EvaluateNear("AnalyzeEvent()", "two identical interactions, hit 0", "The resolution is 2/sqrt(2)", E.GetHit(0)->GetEnergyResolution(), 2.0/sqrt(2.0), 1e-9) && Passed;
      Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "two identical interactions, hit 1", "The resolution of the second hit does not include the first hit", E.GetHit(1)->GetEnergyResolution(), 2.0/sqrt(2.0), 1e-9)) && Passed;
    }
  }

  // Strip hits without energy resolution (e.g., not from the energy calibration) must not produce NaNs
  {
    MReadOutAssembly E;
    AddStripHit(E, 0, 10, true, 400.0, 0.0);
    AddStripHit(E, 0, 20, false, 400.0, 0.0);
    int OldVerbosity = g_Verbosity;
    g_Verbosity = c_Quiet;
    M.AnalyzeEvent(&E);
    g_Verbosity = OldVerbosity;
    Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "zero resolutions", "One hit is created", E.GetNHits(), 1u)) && Passed;
    if (E.GetNHits() == 1) {
      Passed = EvaluateNear("AnalyzeEvent()", "zero resolutions", "The energy is finite and 400 keV", E.GetHit(0)->GetEnergy(), 400.0, 1e-9) && Passed;
    }
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::TestMultipleDetectors()
{
  bool Passed = true;

  MModuleStripPairingChiSquare M;
  M.Initialize();

  MReadOutAssembly E;
  MStripHit* LV0 = AddStripHit(E, 0, 10, true, 300.0);
  MStripHit* LV1 = AddStripHit(E, 1, 10, true, 500.0);
  MStripHit* HV0 = AddStripHit(E, 0, 5, false, 300.0);
  MStripHit* HV1 = AddStripHit(E, 1, 5, false, 500.0);
  Passed = EvaluateTrue("AnalyzeEvent()", "two detectors", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "two detectors", "Two hits are created", E.GetNHits(), 2u) && Passed;
  if (E.GetNHits() == 2) {
    MHit* H300 = FindHit(E, 300.0);
    MHit* H500 = FindHit(E, 500.0);
    Passed = EvaluateTrue("AnalyzeEvent()", "two detectors", "The 300 keV hit contains the detector 0 LV strip", HasStripHit(H300, LV0)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "two detectors", "The 300 keV hit contains the detector 0 HV strip", HasStripHit(H300, HV0)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "two detectors", "The 500 keV hit contains the detector 1 LV strip", HasStripHit(H500, LV1)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "two detectors", "The 500 keV hit contains the detector 1 HV strip", HasStripHit(H500, HV1)) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::TestErrors()
{
  bool Passed = true;

  MModuleStripPairingChiSquare M;
  M.Initialize();

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  MReadOutAssembly Empty;
  bool EmptyResult = M.AnalyzeEvent(&Empty);

  MReadOutAssembly OneSide;
  AddStripHit(OneSide, 0, 10, true, 300.0);
  bool OneSideResult = M.AnalyzeEvent(&OneSide);

  MReadOutAssembly Many;
  for (unsigned int s = 0; s < 7; ++s) AddStripHit(Many, 0, 2*s, true, 100.0);
  AddStripHit(Many, 0, 5, false, 700.0);
  bool ManyResult = M.AnalyzeEvent(&Many);

  MReadOutAssembly Mismatch;
  AddStripHit(Mismatch, 0, 10, true, 300.0, 1.0);
  AddStripHit(Mismatch, 0, 5, false, 600.0, 1.0);
  bool MismatchResult = M.AnalyzeEvent(&Mismatch);

  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("AnalyzeEvent()", "no strip hits", "AnalyzeEvent returns false", EmptyResult) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "no strip hits", "A strip pairing error is set", Empty.HasStripPairingError()) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "no strip hits", "The analysis progress is set", Empty.HasAnalysisProgress(MAssembly::c_StripPairing)) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "LV only", "AnalyzeEvent returns false", OneSideResult) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "LV only", "A strip pairing error is set", OneSide.HasStripPairingError()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "7 LV strips", "AnalyzeEvent returns false", ManyResult) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "7 LV strips", "A strip pairing error is set", Many.HasStripPairingError()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "7 LV strips", "No hits are created", Many.GetNHits(), 0u) && Passed;
  // 300 keV vs 600 keV with 1 keV resolution: by design the larger side (here HV) is used
  Passed = EvaluateTrue("AnalyzeEvent()", "300 vs 600 keV", "AnalyzeEvent returns true", MismatchResult) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "300 vs 600 keV", "No strip pairing error is set", Mismatch.HasStripPairingError()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "300 vs 600 keV", "One hit is created", Mismatch.GetNHits(), 1u) && Passed;
  if (Mismatch.GetNHits() == 1) {
    Passed = EvaluateNear("AnalyzeEvent()", "300 vs 600 keV", "The energy of the larger side, 600 keV, is used", Mismatch.GetHit(0)->GetEnergy(), 600.0, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "300 vs 600 keV", "The resolution of the larger side, 1 keV, is used", Mismatch.GetHit(0)->GetEnergyResolution(), 1.0, 1e-9) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingChiSquare::TestNearestNeighbors()
{
  bool Passed = true;

  MModuleStripPairingChiSquare M;
  M.Initialize();

  // One 662 keV interaction (LV 10, HV 20) with its nearest-neighbor strips, as loaded by the HDF loader
  MReadOutAssembly E;
  MStripHit* LV = AddStripHit(E, 0, 10, true, 662.0);
  MStripHit* HV = AddStripHit(E, 0, 20, false, 660.0);
  AddStripHit(E, 0, 9, true, 0.5, 1.0)->IsNearestNeighbor(true);
  AddStripHit(E, 0, 11, true, -0.3, 1.0)->IsNearestNeighbor(true);
  AddStripHit(E, 0, 19, false, 0.2, 1.0)->IsNearestNeighbor(true);
  AddStripHit(E, 0, 21, false, 0.1, 1.0)->IsNearestNeighbor(true);
  M.AnalyzeEvent(&E);
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "1 LV + 1 HV triggered strip, 2 + 2 nearest neighbors", "Only the interaction is paired: one hit", E.GetNHits(), 1u)) && Passed;
  bool InteractionPaired = false;
  for (unsigned int h = 0; h < E.GetNHits(); ++h) {
    if (HasStripHit(E.GetHit(h), LV) == true && HasStripHit(E.GetHit(h), HV) == true) InteractionPaired = true;
  }
  Passed = EvaluateTrue("AnalyzeEvent()", "1 LV + 1 HV triggered strip, 2 + 2 nearest neighbors", "The triggered LV and HV strips form one hit", InteractionPaired) && Passed;

  // Two interactions with nearest neighbors exceed six strips per side only if the neighbors are counted
  MReadOutAssembly T;
  AddStripHit(T, 0, 10, true, 400.0);
  AddStripHit(T, 0, 40, true, 200.0);
  AddStripHit(T, 0, 20, false, 400.0);
  AddStripHit(T, 0, 50, false, 200.0);
  const vector<unsigned int> NN = { 9, 11, 39, 41 };
  for (unsigned int S : NN) {
    AddStripHit(T, 0, S, true, 0.1, 1.0)->IsNearestNeighbor(true);
    AddStripHit(T, 0, S + 10, false, 0.1, 1.0)->IsNearestNeighbor(true);
  }
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  M.AnalyzeEvent(&T);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("AnalyzeEvent()", "2 + 2 triggered strips, 4 + 4 nearest neighbors", "No strip pairing error (the neighbors do not count towards the strip limit)", T.HasStripPairingError()) && Passed;
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "2 + 2 triggered strips, 4 + 4 nearest neighbors", "Two hits are created", T.GetNHits(), 2u)) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleStripPairingChiSquare Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleStripPairingChiSquare.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
