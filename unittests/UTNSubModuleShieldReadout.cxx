/*
 * UTNSubModuleShieldReadout.cxx
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
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MSubModuleShieldReadout.h"
#include "MReadOutAssembly.h"
#include "MDEECrystalHit.h"


//! Unit test class for MSubModuleShieldReadout
class UTNSubModuleShieldReadout : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleShieldReadout() : MUnitTest("UTNSubModuleShieldReadout") {}
  //! Default destructor
  virtual ~UTNSubModuleShieldReadout() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the lifecycle and the (empty) XML configuration
  bool TestLifecycle();
  //! Test the energy to ADC conversion
  bool TestADCConversion();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::Run()
{
  bool Passed = true;

  Passed = TestLifecycle() && Passed;
  Passed = TestADCConversion() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestLifecycle()
{
  bool Passed = true;

  MSubModuleShieldReadout M;
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;
  M.Clear();
  MXmlNode* Node = new MXmlNode(nullptr, "DEE");
  Passed = EvaluateTrue("CreateXmlConfiguration()", "mother node", "The mother node is returned", M.CreateXmlConfiguration(Node) == Node) && Passed;
  Passed = Evaluate("CreateXmlConfiguration()", "mother node", "No configuration is added", Node->GetNNodes(), 0u) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "mother node", "ReadXmlConfiguration returns true", M.ReadXmlConfiguration(Node)) && Passed;
  delete Node;

  MReadOutAssembly Empty;
  Passed = EvaluateTrue("AnalyzeEvent()", "empty event", "AnalyzeEvent returns true for an empty event", M.AnalyzeEvent(&Empty)) && Passed;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldReadout::TestADCConversion()
{
  bool Passed = true;

  MSubModuleShieldReadout M;
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", M.Initialize()) && Passed;

  MReadOutAssembly E;
  vector<double> Energies = { 0.0, 100.0, 511.5, 3595.75, 4000.0 };
  for (double En : Energies) {
    MDEECrystalHit C;
    C.m_Energy = En;
    E.AddDEECrystalHit(C);
  }
  Passed = EvaluateTrue("AnalyzeEvent()", "five crystal hits", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;

  vector<unsigned int> Expected = { 2000, 2400, 4046, 16383, 16383 };
  unsigned int i = 0;
  for (MDEECrystalHit& C : E.GetDEECrystalHitListReference()) {
    if (i >= Energies.size()) break;
    MString Input = MString("energy ") + Energies[i];
    Passed = Evaluate("AnalyzeEvent()", Input, "The ADC value is 2000 + 4 * energy, limited to 16383", C.m_ADC, Expected[i]) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", Input, "The energy is not modified", C.m_Energy, Energies[i], 1e-12) && Passed;
    ++i;
  }
  Passed = Evaluate("AnalyzeEvent()", "five crystal hits", "No crystal hit is added or removed", E.GetNDEECrystalHits(), 5u) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleShieldReadout Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleShieldReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
