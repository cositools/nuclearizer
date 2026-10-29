/*
 * UTNModuleDiagnostics.cxx
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

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleDiagnostics.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MModuleDiagnostics
class UTNModuleDiagnostics : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleDiagnostics() : MUnitTest("UTNModuleDiagnostics") {}
  //! Default destructor
  virtual ~UTNModuleDiagnostics() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data
  bool TestDefaultConstruction();
  //! Test Initialize(), AnalyzeEvent() without expos, and Finalize()
  bool TestLifecycle();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDiagnostics::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestLifecycle() && Passed;

  // CreateExpos() creates an MGUIExpo GUI element and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDiagnostics::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleDiagnostics M;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagDiagnostics'", M.GetXmlTag(), MString("XmlTagDiagnostics")) && Passed;
  Passed = EvaluateFalse("GetName()", "default", "The name is not empty", M.GetName().IsEmpty()) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_Diagnostics", M.ProvidesModuleType(MAssembly::c_Diagnostics)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "An event loader is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateFalse("HasOptionsGUI()", "default", "The module has no options GUI", M.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "default", "The module allows multi-threading", M.AllowsMultiThreading()) && Passed;
  Passed = EvaluateFalse("AllowsMultipleInstances()", "default", "The module does not allow multiple instances", M.AllowsMultipleInstances()) && Passed;
  Passed = EvaluateFalse("HasExpos()", "default", "No expos exist before CreateExpos()", M.HasExpos()) && Passed;

  MModuleDiagnostics* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDiagnostics::TestLifecycle()
{
  bool Passed = true;

  MModuleDiagnostics M;
  M.Pause(true);
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;
  Passed = EvaluateFalse("Initialize()", "paused module", "Initialize resets the paused state of the base module", M.IsPaused()) && Passed;

  MReadOutAssembly E;
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(1);
  SH->SetStripID(5);
  SH->SetEnergy(511.0);
  E.AddStripHit(SH);

  Passed = EvaluateTrue("AnalyzeEvent()", "without expos", "AnalyzeEvent returns true without expos", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "without expos", "AnalyzeEvent does not remove strip hits", E.GetNStripHits(), 1u) && Passed;
  if (E.GetNStripHits() == 1) {
    Passed = EvaluateNear("AnalyzeEvent()", "without expos", "AnalyzeEvent does not modify the strip hit energy", E.GetStripHit(0)->GetEnergy(), 511.0, 1e-12) && Passed;
  }
  Passed = EvaluateFalse("AnalyzeEvent()", "without expos", "AnalyzeEvent does not filter the event", E.IsFilteredOut()) && Passed;

  MReadOutAssembly Empty;
  Passed = EvaluateTrue("AnalyzeEvent()", "empty event", "AnalyzeEvent returns true for an empty event", M.AnalyzeEvent(&Empty)) && Passed;

  M.Finalize();

  // Repeated initialization
  Passed = EvaluateTrue("Initialize()", "second time", "A second Initialize returns true", M.Initialize()) && Passed;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleDiagnostics Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleDiagnostics.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
