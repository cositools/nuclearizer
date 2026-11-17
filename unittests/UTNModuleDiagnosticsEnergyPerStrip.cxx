/*
 * UTNModuleDiagnosticsEnergyPerStrip.cxx
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
#include "MModuleDiagnosticsEnergyPerStrip.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MHit.h"


//! Unit test class for MModuleDiagnosticsEnergyPerStrip
class UTNModuleDiagnosticsEnergyPerStrip : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleDiagnosticsEnergyPerStrip() : MUnitTest("UTNModuleDiagnosticsEnergyPerStrip") {}
  //! Default destructor
  virtual ~UTNModuleDiagnosticsEnergyPerStrip() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data
  bool TestDefaultConstruction();
  //! Test Initialize(), AnalyzeEvent() without expos, and Finalize()
  bool TestLifecycle();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDiagnosticsEnergyPerStrip::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestLifecycle() && Passed;

  // CreateExpos() creates an MGUIExpo GUI element and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDiagnosticsEnergyPerStrip::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleDiagnosticsEnergyPerStrip M;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagDiagnosticsEnergyPerStrip'", M.GetXmlTag(), MString("XmlTagDiagnosticsEnergyPerStrip")) && Passed;
  Passed = Evaluate("GetName()", "default", "The name is 'Diagnostics: Energy Per Strip'", M.GetName(), MString("Diagnostics: Energy Per Strip")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_Diagnostics", M.ProvidesModuleType(MAssembly::c_Diagnostics)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "An event loader is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateFalse("HasOptionsGUI()", "default", "The module has no options GUI", M.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("AllowsMultiThreading()", "default", "The module allows multi-threading", M.AllowsMultiThreading()) && Passed;
  Passed = EvaluateFalse("AllowsMultipleInstances()", "default", "The module does not allow multiple instances", M.AllowsMultipleInstances()) && Passed;
  Passed = EvaluateFalse("HasExpos()", "default", "No expos exist before CreateExpos()", M.HasExpos()) && Passed;

  MModuleDiagnosticsEnergyPerStrip* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleDiagnosticsEnergyPerStrip::TestLifecycle()
{
  bool Passed = true;

  MModuleDiagnosticsEnergyPerStrip M;
  M.Pause(true);
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;
  // MModule::Initialize() resets the paused state, the thread, the timers, and the event counter
  Passed = ExpectedToFail(EvaluateFalse("Initialize()", "paused module", "Initialize calls the base class initialization which resets the paused state", M.IsPaused())) && Passed;

  // Without expos, the events must pass unchanged, also beyond the 10 setup events
  for (unsigned int e = 0; e < 12; ++e) {
    MReadOutAssembly E;
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(1);
    SH->SetStripID(5);
    SH->SetEnergy(662.0);
    E.AddStripHit(SH);
    MHit* H = new MHit();
    H->SetEnergy(662.0);
    E.AddHit(H);

    MString Input = MString("event ") + e;
    Passed = EvaluateTrue("AnalyzeEvent()", Input, "AnalyzeEvent returns true without expos", M.AnalyzeEvent(&E)) && Passed;
    Passed = Evaluate("AnalyzeEvent()", Input, "AnalyzeEvent does not remove strip hits", E.GetNStripHits(), 1u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", Input, "AnalyzeEvent does not remove hits", E.GetNHits(), 1u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", Input, "AnalyzeEvent does not filter the event", E.IsFilteredOut()) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleDiagnosticsEnergyPerStrip Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleDiagnosticsEnergyPerStrip.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
