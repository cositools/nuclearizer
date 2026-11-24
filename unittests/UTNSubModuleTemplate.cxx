/*
 * UTNSubModuleTemplate.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <type_traits>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MSubModuleTemplate.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MSubModuleTemplate
class UTNSubModuleTemplate : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleTemplate() : MUnitTest("UTNSubModuleTemplate") {}
  //! Default destructor
  virtual ~UTNSubModuleTemplate() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the class properties every derived sub module relies on
  bool TestClassProperties();
  //! Test the lifecycle and the (empty) XML configuration
  bool TestLifecycle();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleTemplate::Run()
{
  bool Passed = true;

  Passed = TestClassProperties() && Passed;
  Passed = TestLifecycle() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleTemplate::TestClassProperties()
{
  bool Passed = true;

  Passed = EvaluateTrue("MSubModuleTemplate", "type traits", "The template has a virtual destructor", has_virtual_destructor<MSubModuleTemplate>::value) && Passed;
  Passed = EvaluateFalse("MSubModuleTemplate", "type traits", "The template cannot be copied", is_copy_constructible<MSubModuleTemplate>::value) && Passed;
  Passed = EvaluateFalse("MSubModuleTemplate", "type traits", "The template cannot be moved", is_move_constructible<MSubModuleTemplate>::value) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleTemplate::TestLifecycle()
{
  bool Passed = true;

  MSubModuleTemplate M;
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;
  M.Clear();

  MReadOutAssembly E;
  MStripHit* SH = new MStripHit();
  SH->SetEnergy(42.0);
  E.AddStripHit(SH);
  Passed = EvaluateTrue("AnalyzeEvent()", "one strip hit", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one strip hit", "The event is not modified", E.GetNStripHits(), 1u) && Passed;

  MXmlNode* Node = new MXmlNode(nullptr, "Template");
  Passed = EvaluateTrue("CreateXmlConfiguration()", "mother node", "The mother node is returned", M.CreateXmlConfiguration(Node) == Node) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "mother node", "No configuration is added", Node->GetNNodes(), 0u) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "mother node", "ReadXmlConfiguration returns true", M.ReadXmlConfiguration(Node)) && Passed;
  delete Node;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleTemplate Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleTemplate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
