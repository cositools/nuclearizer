/*
 * UTNModuleNearestNeighbor.cxx
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
#include "MAssembly.h"
#include "MModuleNearestNeighbor.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MModuleNearestNeighbor
class UTNModuleNearestNeighbor : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleNearestNeighbor() : MUnitTest("UTNModuleNearestNeighbor") {}
  //! Default destructor
  virtual ~UTNModuleNearestNeighbor() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data
  bool TestDefaultConstruction();
  //! Test the (empty) XML configuration
  bool TestXmlConfiguration();
  //! Test the pass-through AnalyzeEvent()
  bool TestAnalyzeEvent();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleNearestNeighbor::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestXmlConfiguration() && Passed;
  Passed = TestAnalyzeEvent() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleNearestNeighbor::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleNearestNeighbor M;
  Passed = Evaluate("GetName()", "default", "The module name is 'NearestNeighbor'", M.GetName(), MString("NearestNeighbor")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagNearestNeighbor'", M.GetXmlTag(), MString("XmlTagNearestNeighbor")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_NearestNeighbor", M.ProvidesModuleType(MAssembly::c_NearestNeighbor)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "An event loader is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateFalse("HasOptionsGUI()", "default", "The module has no options GUI", M.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "default", "The module allows multi-threading", M.AllowsMultiThreading()) && Passed;
  Passed = EvaluateTrue("AllowsMultipleInstances()", "default", "The module allows multiple instances", M.AllowsMultipleInstances()) && Passed;

  MModuleNearestNeighbor* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  // The options GUI is empty, thus calling it is safe without a display
  M.ShowOptionsGUI();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleNearestNeighbor::TestXmlConfiguration()
{
  bool Passed = true;

  MModuleNearestNeighbor M;
  MXmlNode* Node = M.CreateXmlConfiguration();
  Passed = EvaluateTrue("CreateXmlConfiguration()", "default", "A node is created", Node != nullptr) && Passed;
  if (Node != nullptr) {
    Passed = Evaluate("CreateXmlConfiguration()", "default", "The node name is the XML tag", Node->GetName(), MString("XmlTagNearestNeighbor")) && Passed;
    Passed = Evaluate("CreateXmlConfiguration()", "default", "The node has no children", Node->GetNNodes(), 0u) && Passed;
    Passed = EvaluateTrue("ReadXmlConfiguration()", "own node", "ReadXmlConfiguration returns true", M.ReadXmlConfiguration(Node)) && Passed;
  }
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleNearestNeighbor::TestAnalyzeEvent()
{
  bool Passed = true;

  MModuleNearestNeighbor M;
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;

  MReadOutAssembly E;
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(2);
  SH->SetStripID(17);
  SH->IsNearestNeighbor(true);
  SH->SetEnergy(12.5);
  E.AddStripHit(SH);

  Passed = EvaluateTrue("AnalyzeEvent()", "one nearest-neighbor strip hit", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one nearest-neighbor strip hit", "The strip hit is kept", E.GetNStripHits(), 1u) && Passed;
  if (E.GetNStripHits() == 1) {
    Passed = EvaluateTrue("AnalyzeEvent()", "one nearest-neighbor strip hit", "The strip hit is still a nearest neighbor", E.GetStripHit(0)->IsNearestNeighbor()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "one nearest-neighbor strip hit", "The energy is unchanged", E.GetStripHit(0)->GetEnergy(), 12.5, 1e-12) && Passed;
  }
  Passed = EvaluateFalse("AnalyzeEvent()", "one nearest-neighbor strip hit", "The event is not marked bad", E.IsBad()) && Passed;

  MReadOutAssembly Empty;
  Passed = EvaluateTrue("AnalyzeEvent()", "empty event", "AnalyzeEvent returns true for an empty event", M.AnalyzeEvent(&Empty)) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleNearestNeighbor Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleNearestNeighbor.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
