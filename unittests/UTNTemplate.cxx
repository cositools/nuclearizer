/*
 * UTNTemplate.cxx
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

// Nuclearizer:
#include "MTemplate.h"


//! Unit test class for MTemplate
class UTNTemplate : public MUnitTest
{
 public:
  //! Default constructor
  UTNTemplate() : MUnitTest("UTNTemplate") {}
  //! Default destructor
  virtual ~UTNTemplate() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the class properties new classes copied from the template rely on
  bool TestClassProperties();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNTemplate::Run()
{
  bool Passed = true;

  // MTemplate has no public interface besides construction and destruction
  Passed = TestClassProperties() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNTemplate::TestClassProperties()
{
  bool Passed = true;

  Passed = EvaluateTrue("MTemplate", "type traits", "The template is default constructible", is_default_constructible<MTemplate>::value) && Passed;
  Passed = EvaluateTrue("MTemplate", "type traits", "The template has a virtual destructor", has_virtual_destructor<MTemplate>::value) && Passed;

  MTemplate* T = new MTemplate();
  delete T;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNTemplate Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNTemplate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
