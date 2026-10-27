/*
 * UTNGuardringHit.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MVector.h"

// Nuclearizer:
#include "MGuardringHit.h"


//! Unit test class for MGuardringHit
class UTNGuardringHit : public MUnitTest
{
 public:
  //! Default constructor
  UTNGuardringHit() : MUnitTest("UTNGuardringHit") {}
  //! Default destructor
  virtual ~UTNGuardringHit() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test default values and Clear()
  bool TestDefaultConstruction();
  //! Test every setter/getter pair
  bool TestGettersSetters();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNGuardringHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNGuardringHit::TestDefaultConstruction()
{
  bool Passed = true;

  MGuardringHit H;
  // The detector ID is a signed int, thus its "not defined" sentinel is g_IntNotDefined
  Passed = ExpectedToFail(Evaluate("MGuardringHit()", "default", "The default detector ID is g_IntNotDefined", H.GetDetectorID(), g_IntNotDefined)) && Passed;
  Passed = EvaluateTrue("MGuardringHit()", "default", "The default ADC units are g_DoubleNotDefined", H.GetADCUnits() == g_DoubleNotDefined) && Passed;
  MVector DefaultPosition = H.GetPosition();

  H.SetDetectorID(5);
  H.SetADCUnits(1234.0);
  H.SetPosition(MVector(1.0, 2.0, 3.0));
  H.Clear();

  Passed = ExpectedToFail(Evaluate("Clear()", "all fields set", "Clear() resets the detector ID", H.GetDetectorID(), g_IntNotDefined)) && Passed;
  Passed = EvaluateTrue("Clear()", "all fields set", "Clear() resets the ADC units", H.GetADCUnits() == g_DoubleNotDefined) && Passed;
  Passed = ExpectedToFail(EvaluateVectorNear("Clear()", "all fields set", "Clear() resets the position to its default", H.GetPosition(), DefaultPosition, 1e-12)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNGuardringHit::TestGettersSetters()
{
  bool Passed = true;

  MGuardringHit H;
  H.SetDetectorID(11);
  Passed = Evaluate("SetDetectorID/GetDetectorID", "11", "GetDetectorID returns 11", H.GetDetectorID(), 11) && Passed;
  H.SetDetectorID(0);
  Passed = Evaluate("SetDetectorID/GetDetectorID", "0", "GetDetectorID returns 0", H.GetDetectorID(), 0) && Passed;

  H.SetADCUnits(4053.5);
  Passed = EvaluateNear("SetADCUnits/GetADCUnits", "4053.5", "GetADCUnits returns 4053.5", H.GetADCUnits(), 4053.5, 1e-12) && Passed;

  H.SetPosition(MVector(-1.5, 2.25, 0.75));
  Passed = EvaluateVectorNear("SetPosition/GetPosition", "(-1.5, 2.25, 0.75)", "GetPosition returns the set position", H.GetPosition(), MVector(-1.5, 2.25, 0.75), 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNGuardringHit Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNGuardringHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
