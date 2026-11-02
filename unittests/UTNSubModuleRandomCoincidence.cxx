/*
 * UTNSubModuleRandomCoincidence.cxx
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
#include "MSubModuleRandomCoincidence.h"
#include "MReadOutAssembly.h"
#include "MDEEStripHit.h"
#include "MDEECrystalHit.h"


//! Unit test class for MSubModuleRandomCoincidence
class UTNSubModuleRandomCoincidence : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleRandomCoincidence() : MUnitTest("UTNSubModuleRandomCoincidence") {}
  //! Default destructor
  virtual ~UTNSubModuleRandomCoincidence() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the lifecycle and the (empty) XML configuration
  bool TestLifecycle();
  //! Test that the (not yet implemented) random coincidences leave the event unchanged
  bool TestPassThrough();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleRandomCoincidence::Run()
{
  bool Passed = true;

  Passed = TestLifecycle() && Passed;
  Passed = TestPassThrough() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleRandomCoincidence::TestLifecycle()
{
  bool Passed = true;

  MSubModuleRandomCoincidence M;
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


bool UTNSubModuleRandomCoincidence::TestPassThrough()
{
  bool Passed = true;

  MSubModuleRandomCoincidence M;
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;

  MReadOutAssembly E;
  MDEEStripHit LV;
  LV.m_Energy = 100.0;
  E.AddDEEStripHitLV(LV);
  MDEEStripHit HV;
  HV.m_Energy = 101.0;
  E.AddDEEStripHitHV(HV);
  MDEECrystalHit C;
  C.m_Energy = 50.0;
  E.AddDEECrystalHit(C);

  Passed = EvaluateTrue("AnalyzeEvent()", "one LV, one HV, one crystal hit", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one LV, one HV, one crystal hit", "The LV hits are unchanged", E.GetNDEEStripHitsLV(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one LV, one HV, one crystal hit", "The HV hits are unchanged", E.GetNDEEStripHitsHV(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one LV, one HV, one crystal hit", "The crystal hits are unchanged", E.GetNDEECrystalHits(), 1u) && Passed;
  if (E.GetNDEEStripHitsLV() > 0) {
    Passed = EvaluateNear("AnalyzeEvent()", "one LV hit", "The LV energy is unchanged", E.GetDEEStripHitLVListReference().front().m_Energy, 100.0, 1e-12) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleRandomCoincidence Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleRandomCoincidence.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
