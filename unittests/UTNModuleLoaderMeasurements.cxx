/*
 * UTNModuleLoaderMeasurements.cxx
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
#include "MModuleLoaderMeasurements.h"
#include "MReadOutAssembly.h"


//! Minimal concrete loader to test the abstract base class
class UTNModuleLoaderMeasurementsStub : public MModuleLoaderMeasurements
{
 public:
  //! Default constructor
  UTNModuleLoaderMeasurementsStub() : MModuleLoaderMeasurements() { m_XmlTag = "XmlTagUTNLoaderStub"; }
  //! Create a new object of this class
  virtual UTNModuleLoaderMeasurementsStub* Clone() { return new UTNModuleLoaderMeasurementsStub(); }
  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node) { return true; }
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration() { return new MXmlNode(nullptr, m_XmlTag); }
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test class for MModuleLoaderMeasurements
class UTNModuleLoaderMeasurements : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleLoaderMeasurements() : MUnitTest("UTNModuleLoaderMeasurements") {}
  //! Default destructor
  virtual ~UTNModuleLoaderMeasurements() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module types set by the base class
  bool TestDefaultConstruction();
  //! Test the base Initialize() and AnalyzeEvent()
  bool TestLifecycle();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurements::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestLifecycle() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurements::TestDefaultConstruction()
{
  bool Passed = true;

  UTNModuleLoaderMeasurementsStub M;
  Passed = EvaluateTrue("MModuleLoaderMeasurements()", "default", "The loader provides c_EventLoader", M.ProvidesModuleType(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("MModuleLoaderMeasurements()", "default", "The loader provides c_EventLoaderMeasurement", M.ProvidesModuleType(MAssembly::c_EventLoaderMeasurement)) && Passed;
  Passed = EvaluateFalse("MModuleLoaderMeasurements()", "default", "The loader does not provide c_EventLoaderSimulation", M.ProvidesModuleType(MAssembly::c_EventLoaderSimulation)) && Passed;
  Passed = Evaluate("MModuleLoaderMeasurements()", "default", "A loader has no preceeding module requirements", M.GetNPreceedingModuleTypes(), 0u) && Passed;
  Passed = Evaluate("MModuleLoaderMeasurements()", "default", "There is one succeeding module type", M.GetNSucceedingModuleTypes(), 1u) && Passed;
  if (M.GetNSucceedingModuleTypes() == 1) {
    Passed = EvaluateTrue("MModuleLoaderMeasurements()", "default", "Any module can follow the loader", M.GetSucceedingModuleType(0) == MAssembly::c_NoRestriction) && Passed;
  }
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The loader has an options GUI", M.HasOptionsGUI()) && Passed;

  UTNModuleLoaderMeasurementsStub* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "stub", "Clone() returns an object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "stub", "Clone() returns a distinct object", C != &M) && Passed;
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurements::TestLifecycle()
{
  bool Passed = true;

  UTNModuleLoaderMeasurementsStub M;
  M.Pause(true);
  Passed = EvaluateTrue("Initialize()", "base class", "Initialize returns true", M.Initialize()) && Passed;
  Passed = ExpectedToFail(EvaluateFalse("Initialize()", "paused module", "Initialize calls the base class initialization which resets the paused state", M.IsPaused())) && Passed;

  MReadOutAssembly E;
  Passed = EvaluateTrue("AnalyzeEvent()", "base class", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "base class", "AnalyzeEvent does not create strip hits", E.GetNStripHits(), 0u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleLoaderMeasurements Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleLoaderMeasurements.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
