/*
 * UTNSubModuleStripReadoutNoise.cxx
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
#include "MXmlNode.h"

// Nuclearizer:
#include "MSubModuleStripReadoutNoise.h"
#include "MReadOutAssembly.h"
#include "MDEEStripHit.h"


//! Unit test class for MSubModuleStripReadoutNoise
class UTNSubModuleStripReadoutNoise : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleStripReadoutNoise() : MUnitTest("UTNSubModuleStripReadoutNoise") {}
  //! Default destructor
  virtual ~UTNSubModuleStripReadoutNoise() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the lifecycle and the (empty) XML configuration
  bool TestLifecycle();
  //! Test that the (not yet implemented) noise leaves the strip hits unchanged
  bool TestPassThrough();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadoutNoise::Run()
{
  bool Passed = true;

  Passed = TestLifecycle() && Passed;
  Passed = TestPassThrough() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleStripReadoutNoise::TestLifecycle()
{
  bool Passed = true;

  MSubModuleStripReadoutNoise M;
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


bool UTNSubModuleStripReadoutNoise::TestPassThrough()
{
  bool Passed = true;

  MSubModuleStripReadoutNoise M;
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;

  MReadOutAssembly E;
  MDEEStripHit LV;
  LV.m_Energy = 100.0;
  LV.m_ADC = 2400;
  E.AddDEEStripHitLV(LV);
  MDEEStripHit HV;
  HV.m_Energy = 101.0;
  HV.m_ADC = 2404;
  E.AddDEEStripHitHV(HV);

  Passed = EvaluateTrue("AnalyzeEvent()", "one LV, one HV", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one LV, one HV", "The number of LV hits is unchanged", E.GetNDEEStripHitsLV(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one LV, one HV", "The number of HV hits is unchanged", E.GetNDEEStripHitsHV(), 1u) && Passed;
  if (E.GetNDEEStripHitsLV() > 0) {
    Passed = Evaluate("AnalyzeEvent()", "LV hit", "The LV ADC value is unchanged", E.GetDEEStripHitLVListReference().front().m_ADC, 2400u) && Passed;
  }
  if (E.GetNDEEStripHitsHV() > 0) {
    Passed = Evaluate("AnalyzeEvent()", "HV hit", "The HV ADC value is unchanged", E.GetDEEStripHitHVListReference().front().m_ADC, 2404u) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleStripReadoutNoise Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleStripReadoutNoise.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
