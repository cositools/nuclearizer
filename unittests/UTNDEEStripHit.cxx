/*
 * UTNDEEStripHit.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <vector>
using namespace std;

// MEGAlib:
#include "MUnitTest.h"

// Nuclearizer:
#include "MDEEStripHit.h"


//! Unit test class for MDEEStripHit
class UTNDEEStripHit : public MUnitTest
{
 public:
  UTNDEEStripHit()
      : MUnitTest("UTNDEEStripHit")
  {
  }
  virtual ~UTNDEEStripHit()
  {
  }

  virtual bool Run();

 private:
  //! Test default-construction state for reliably defaulted fields
  bool TestDefaultConstruction();
  //! Test Convert() with representative values
  bool TestConvertRepresentativeValues();
  //! Test Convert() false-path booleans
  bool TestConvertFalsePaths();
  //! Test Convert() repeated-allocation lifecycle behavior
  bool TestConvertLifecycleIndependence();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNDEEStripHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestConvertRepresentativeValues() && Passed;
  Passed = TestConvertFalsePaths() && Passed;
  Passed = TestConvertLifecycleIndependence() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNDEEStripHit::TestDefaultConstruction()
{
  bool Passed = true;

  MDEEStripHit H;

  Passed = Evaluate("MDEEStripHit()", "default simulated event ID", "Default simulated event ID is 0", H.m_SimulatedEventID, (unsigned long) 0) && Passed;
  Passed = EvaluateNear("MDEEStripHit()", "default simulated position X", "Default simulated position X is 0", H.m_SimulatedPosition.X(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDEEStripHit()", "default simulated position Y", "Default simulated position Y is 0", H.m_SimulatedPosition.Y(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDEEStripHit()", "default simulated position Z", "Default simulated position Z is 0", H.m_SimulatedPosition.Z(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDEEStripHit()", "default simulated detector position X", "Default simulated detector position X is 0", H.m_SimulatedPositionInDetector.X(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDEEStripHit()", "default simulated detector position Y", "Default simulated detector position Y is 0", H.m_SimulatedPositionInDetector.Y(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDEEStripHit()", "default simulated detector position Z", "Default simulated detector position Z is 0", H.m_SimulatedPositionInDetector.Z(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDEEStripHit()", "default simulated relative depth", "Default simulated relative depth is 0", H.m_SimulatedRelativeDepth, 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MDEEStripHit()", "default simulated energy", "Default simulated energy is 0", H.m_SimulatedEnergy, 0.0, 1e-12) && Passed;
  Passed = Evaluate("MDEEStripHit()", "default simulated origins", "Default simulated origins list is empty", (unsigned int) H.m_SimulatedOrigins.size(), (unsigned int) 0) && Passed;
  Passed = EvaluateFalse("MDEEStripHit()", "default simulated guard ring", "Default simulated guard ring flag is false", H.m_SimulatedIsGuardRing) && Passed;
  Passed = Evaluate("MDEEStripHit()", "default simulated hit index", "Default simulated hit index is 0", H.m_SimulatedHitIndex, (unsigned int) 0) && Passed;

  Passed = EvaluateFalse("MDEEStripHit()", "default guard ring", "Default guard ring flag is false", H.m_IsGuardRing) && Passed;
  Passed = EvaluateNear("MDEEStripHit()", "default measured energy", "Default measured energy is 0", H.m_Energy, 0.0, 1e-12) && Passed;
  Passed = Evaluate("MDEEStripHit()", "default ADC", "Default ADC value is 0", H.m_ADC, (unsigned int) 0) && Passed;
  Passed = EvaluateFalse("MDEEStripHit()", "default trigger", "Default trigger flag is false", H.m_HasTriggered) && Passed;
  Passed = EvaluateFalse("MDEEStripHit()", "default fast timing", "Default fast timing is false", H.m_HasFastTiming) && Passed;
  Passed = Evaluate("MDEEStripHit()", "default TAC", "Default TAC value is 0", H.m_TAC, (unsigned int) 0) && Passed;
  Passed = Evaluate("MDEEStripHit()", "default sub strip hits", "Default sub strip hit list is empty", (unsigned int) H.m_SubStripHits.size(), (unsigned int) 0) && Passed;
  Passed = Evaluate("MDEEStripHit()", "default shared origin", "Default shared origin list is empty", (unsigned int) H.m_SharedOrigin.size(), (unsigned int) 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNDEEStripHit::TestConvertRepresentativeValues()
{
  bool Passed = true;

  MDEEStripHit H;

  H.m_ROE.SetDetectorID(7);
  H.m_ROE.SetStripID(41);
  H.m_ROE.IsLowVoltageStrip(false);
  H.m_HasTriggered = true;
  H.m_HasFastTiming = true;
  H.m_ADC = 4053;
  H.m_TAC = 10452;
  H.m_IsGuardRing = true;

  MStripHit* Converted = H.Convert();

  Passed = EvaluateTrue("Convert()", "representative allocation", "Convert() returns a non-null pointer for representative values", Converted != nullptr) && Passed;

  if (Converted != nullptr) {
    Passed = Evaluate("Convert()", "representative detector ID", "Convert() transfers detector ID 7", Converted->GetDetectorID(), (unsigned int) 7) && Passed;
    Passed = Evaluate("Convert()", "representative strip ID", "Convert() transfers strip ID 41", Converted->GetStripID(), (unsigned int) 41) && Passed;
    Passed = EvaluateFalse("Convert()", "representative strip side high voltage", "Convert() transfers IsLowVoltageStrip(false) as high-voltage strip", Converted->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateTrue("Convert()", "representative trigger", "Convert() transfers HasTriggered true", Converted->HasTriggered()) && Passed;
    Passed = EvaluateTrue("Convert()", "representative fast timing", "Convert() transfers HasFastTiming true", Converted->HasFastTiming()) && Passed;
    Passed = EvaluateNear("Convert()", "representative ADC", "Convert() transfers ADC value 4053", Converted->GetADCUnits(), 4053.0, 1e-9) && Passed;
    Passed = EvaluateNear("Convert()", "representative TAC", "Convert() transfers TAC value 10452", Converted->GetTAC(), 10452.0, 1e-9) && Passed;
    Passed = EvaluateTrue("Convert()", "representative guard ring", "Convert() transfers IsGuardRing true", Converted->IsGuardRing()) && Passed;
  }

  delete Converted;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNDEEStripHit::TestConvertFalsePaths()
{
  bool Passed = true;

  MDEEStripHit H;

  H.m_ROE.SetDetectorID(1);
  H.m_ROE.SetStripID(2);
  H.m_ROE.IsLowVoltageStrip(true);
  H.m_HasTriggered = false;
  H.m_HasFastTiming = false;
  H.m_ADC = 0;
  H.m_TAC = 0;
  H.m_IsGuardRing = false;

  MStripHit* Converted = H.Convert();

  Passed = EvaluateTrue("Convert()", "false-path allocation", "Convert() returns a non-null pointer for false-path values", Converted != nullptr) && Passed;

  if (Converted != nullptr) {
    Passed = EvaluateTrue("Convert()", "low-voltage strip side", "Convert() transfers IsLowVoltageStrip(true) for low-voltage strips", Converted->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateFalse("Convert()", "false-path trigger", "Convert() transfers HasTriggered false", Converted->HasTriggered()) && Passed;
    Passed = EvaluateFalse("Convert()", "false-path fast timing", "Convert() transfers HasFastTiming false", Converted->HasFastTiming()) && Passed;
    Passed = EvaluateFalse("Convert()", "false-path guard ring", "Convert() transfers IsGuardRing false", Converted->IsGuardRing()) && Passed;
    Passed = EvaluateNear("Convert()", "false-path ADC", "Convert() transfers ADC value 0", Converted->GetADCUnits(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("Convert()", "false-path TAC", "Convert() transfers TAC value 0", Converted->GetTAC(), 0.0, 1e-12) && Passed;
  }

  delete Converted;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNDEEStripHit::TestConvertLifecycleIndependence()
{
  bool Passed = true;

  MDEEStripHit H;

  H.m_ROE.SetDetectorID(3);
  H.m_ROE.SetStripID(9);
  H.m_ROE.IsLowVoltageStrip(true);
  H.m_HasTriggered = true;
  H.m_HasFastTiming = true;
  H.m_ADC = 100;
  H.m_TAC = 200;
  H.m_IsGuardRing = false;

  MStripHit* First = H.Convert();
  MStripHit* Second = H.Convert();

  Passed = EvaluateTrue("Convert()", "first repeated allocation", "First Convert() call returns a non-null pointer", First != nullptr) && Passed;
  Passed = EvaluateTrue("Convert()", "second repeated allocation", "Second Convert() call returns a non-null pointer", Second != nullptr) && Passed;

  if (First != nullptr && Second != nullptr) {
    Passed = EvaluateTrue("Convert()", "distinct heap objects", "Repeated Convert() calls return distinct heap objects", First != Second) && Passed;

    First->SetADCUnits(9999.0);
    Passed = EvaluateNear("Convert()", "state independence after mutation", "Mutating the first converted object does not affect the second converted object", Second->GetADCUnits(), 100.0, 1e-9) && Passed;
  }

  delete First;
  delete Second;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNDEEStripHit Test;
  return Test.Run() == true ? 0 : 1;
}
