/*
 * UTNSubModuleDEEOutput.cxx
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
#include "MSubModuleDEEOutput.h"
#include "MReadOutAssembly.h"
#include "MDEEStripHit.h"
#include "MDEECrystalHit.h"
#include "MStripHit.h"
#include "MCrystalHit.h"


//! Unit test class for MSubModuleDEEOutput
class UTNSubModuleDEEOutput : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleDEEOutput() : MUnitTest("UTNSubModuleDEEOutput") {}
  //! Default destructor
  virtual ~UTNSubModuleDEEOutput() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the lifecycle and the (empty) XML configuration
  bool TestLifecycle();
  //! Test the conversion and sorting of DEE strip hits
  bool TestStripHits();
  //! Test the conversion of DEE crystal hits
  bool TestCrystalHits();
  //! Test an empty event
  bool TestEmptyEvent();

  //! Create a DEE strip hit
  MDEEStripHit CreateDEEStripHit(unsigned int DetID, unsigned int StripID, bool IsLowVoltage, unsigned int ADC);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEOutput::Run()
{
  bool Passed = true;

  Passed = TestLifecycle() && Passed;
  Passed = TestStripHits() && Passed;
  Passed = TestCrystalHits() && Passed;
  Passed = TestEmptyEvent() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MDEEStripHit UTNSubModuleDEEOutput::CreateDEEStripHit(unsigned int DetID, unsigned int StripID, bool IsLowVoltage, unsigned int ADC)
{
  MDEEStripHit H;
  H.m_ROE.SetDetectorID(DetID);
  H.m_ROE.SetStripID(StripID);
  H.m_ROE.IsLowVoltageStrip(IsLowVoltage);
  H.m_ADC = ADC;
  H.m_TAC = 2*ADC;
  H.m_IsNearestNeighbor = false;
  H.m_HasFastTiming = true;
  return H;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEOutput::TestLifecycle()
{
  bool Passed = true;

  MSubModuleDEEOutput M;
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


bool UTNSubModuleDEEOutput::TestStripHits()
{
  bool Passed = true;

  MSubModuleDEEOutput M;
  Passed = EvaluateTrue("Initialize()", "strip hits", "Initialize returns true", M.Initialize()) && Passed;

  MReadOutAssembly E;
  E.AddDEEStripHitLV(CreateDEEStripHit(1, 5, true, 100));
  E.AddDEEStripHitLV(CreateDEEStripHit(0, 9, true, 200));
  E.AddDEEStripHitLV(CreateDEEStripHit(0, 2, true, 300));
  E.AddDEEStripHitHV(CreateDEEStripHit(0, 3, false, 400));

  Passed = EvaluateTrue("AnalyzeEvent()", "3 LV, 1 HV", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "3 LV, 1 HV", "Four strip hits are created", E.GetNStripHits(), 4u) && Passed;
  if (E.GetNStripHits() == 4) {
    // LV first, sorted by detector and strip, then HV
    const vector<unsigned int> ExpectedDetectorIDs = { 0, 0, 1, 0 };
    const vector<unsigned int> ExpectedStripIDs = { 2, 9, 5, 3 };
    const vector<bool> ExpectedIsLowVoltage = { true, true, true, false };
    for (unsigned int h = 0; h < ExpectedDetectorIDs.size(); ++h) {
      MString Input = MString("strip hit ") + h;
      Passed = Evaluate("AnalyzeEvent()", Input, "The strip hit has the expected detector ID", E.GetStripHit(h)->GetDetectorID(), ExpectedDetectorIDs[h]) && Passed;
      Passed = Evaluate("AnalyzeEvent()", Input, "The strip hit has the expected strip ID", E.GetStripHit(h)->GetStripID(), ExpectedStripIDs[h]) && Passed;
      Passed = Evaluate("AnalyzeEvent()", Input, "The strip hit is on the expected side", E.GetStripHit(h)->IsLowVoltageStrip(), static_cast<bool>(ExpectedIsLowVoltage[h])) && Passed;
    }
    Passed = EvaluateNear("AnalyzeEvent()", "strip hit 0", "The ADC value is converted", E.GetStripHit(0)->GetADCUnits(), 300.0, 1e-9) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "strip hit 0", "The TAC value is converted", E.GetStripHit(0)->GetTAC(), 600.0, 1e-9) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "strip hit 0", "The fast timing flag is converted", E.GetStripHit(0)->HasFastTiming()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "strip hit 3", "The HV ADC value is converted", E.GetStripHit(3)->GetADCUnits(), 400.0, 1e-9) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEOutput::TestCrystalHits()
{
  bool Passed = true;

  MSubModuleDEEOutput M;
  Passed = EvaluateTrue("Initialize()", "crystal hits", "Initialize returns true", M.Initialize()) && Passed;

  MReadOutAssembly E;
  // Filled the same way as MSubModuleDEEIntake does
  MDEECrystalHit C;
  C.m_DetectorID = "X0";
  C.m_CrystalID = 3;
  C.m_VoxelInDetector = MVector(1, 0, 1);
  C.m_ROE.SetDetectorID("X0");
  C.m_Energy = 250.0;
  C.m_ADC = 1234;
  C.m_HasTriggered = true;
  C.m_HasVetoed = true;
  C.m_SimulatedOrigins = { 9, 4 };
  E.AddDEECrystalHit(C);

  Passed = EvaluateTrue("AnalyzeEvent()", "one crystal hit", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one crystal hit", "One crystal hit is created", E.GetNCrystalHits(), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one crystal hit", "No strip hits are created", E.GetNStripHits(), 0u) && Passed;
  if (E.GetNCrystalHits() == 1) {
    Passed = Evaluate("AnalyzeEvent()", "one crystal hit", "The detector ID is converted", E.GetCrystalHit(0)->GetDetectorID(), MString("X0")) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "one crystal hit", "The crystal ID is converted", E.GetCrystalHit(0)->GetCrystalID(), 3u) && Passed;
    MCrystalHit* CH = E.GetCrystalHit(0);
    Passed = Evaluate("AnalyzeEvent()", "one crystal hit", "The voxel X ID is converted", CH->GetReadOutElement()->GetVoxelXID(), 1u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "one crystal hit", "The voxel Y ID is converted", CH->GetReadOutElement()->GetVoxelYID(), 0u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "one crystal hit", "The voxel Z ID is converted", CH->GetReadOutElement()->GetVoxelZID(), 1u) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "one crystal hit", "The energy is converted", CH->GetEnergy(), 250.0, 1e-12) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "one crystal hit", "The ADC value is converted", CH->GetADCUnits(), 1234.0, 1e-12) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "one crystal hit", "The trigger flag is converted", CH->HasTriggered()) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "one crystal hit", "The veto flag is converted", CH->HasVetoed()) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "one crystal hit", "The simulated origins are converted (sorted)", CH->GetOrigins() == vector<int>({ 4, 9 })) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleDEEOutput::TestEmptyEvent()
{
  bool Passed = true;

  MSubModuleDEEOutput M;
  Passed = EvaluateTrue("Initialize()", "empty event", "Initialize returns true", M.Initialize()) && Passed;
  MReadOutAssembly E;
  Passed = EvaluateTrue("AnalyzeEvent()", "empty event", "AnalyzeEvent returns true", M.AnalyzeEvent(&E)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "empty event", "No strip hits are created", E.GetNStripHits(), 0u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "empty event", "No crystal hits are created", E.GetNCrystalHits(), 0u) && Passed;
  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleDEEOutput Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleDEEOutput.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
