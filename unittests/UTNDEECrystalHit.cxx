/*
 * UTNDEECrystalHit.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <sstream>
using namespace std;

// MEGAlib:
#include "MUnitTest.h"

// Nuclearizer:
#include "MDEECrystalHit.h"


//! Unit test class for MDEECrystalHit
class UTNDEECrystalHit : public MUnitTest
{
 public:
  UTNDEECrystalHit()
      : MUnitTest("UTNDEECrystalHit")
  {
  }
  virtual ~UTNDEECrystalHit()
  {
  }

  virtual bool Run();

 private:
  //! Test default-construction state for reliably defaulted fields
  bool TestDefaultConstruction();
  //! Test Convert() with representative values
  bool TestConvertRepresentativeValues();
  //! Test Convert() false-path booleans
  bool TestConvertFalsePaths();
  //! Test Convert() repeated-allocation lifecycle behavior
  bool TestConvertLifecycleIndependence();
  //! Test that a converted crystal hit is written as a complete roa read-out
  bool TestConvertedRoaLine();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNDEECrystalHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestConvertRepresentativeValues() && Passed;
  Passed = TestConvertFalsePaths() && Passed;
  Passed = TestConvertLifecycleIndependence() && Passed;
  Passed = TestConvertedRoaLine() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNDEECrystalHit::TestDefaultConstruction()
{
  bool Passed = true;

  MDEECrystalHit H;

  Passed = Evaluate("MDEECrystalHit()", "default crystal ID", "The crystal ID is 0", H.m_CrystalID, (unsigned int) 0) && Passed;
  Passed = EvaluateNear("MDEECrystalHit()", "default voxel", "The voxel in the detector is (0, 0, 0)", H.m_VoxelInDetector.Mag(), 0.0, 1e-9) && Passed;
  Passed = Evaluate("MDEECrystalHit()", "default ADC", "The ADC value is 0", H.m_ADC, (unsigned int) 0) && Passed;
  Passed = EvaluateNear("MDEECrystalHit()", "default energy", "The energy is 0", H.m_Energy, 0.0, 1e-9) && Passed;
  Passed = EvaluateFalse("MDEECrystalHit()", "default trigger flag", "The hit has not triggered", H.m_HasTriggered) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNDEECrystalHit::TestConvertRepresentativeValues()
{
  bool Passed = true;

  MDEECrystalHit H;

  H.m_ROE.SetDetectorID("X0");
  H.m_CrystalID = 2;
  H.m_VoxelInDetector = MVector(0, 1, 3);
  H.m_ADC = 812;
  H.m_Energy = 511.5;
  H.m_HasTriggered = true;

  MCrystalHit* Converted = H.Convert();

  Passed = EvaluateTrue("Convert()", "representative allocation", "Convert() returns a non-null pointer for representative values", Converted != nullptr) && Passed;

  if (Converted != nullptr) {
    Passed = Evaluate("Convert()", "representative detector ID", "Convert() transfers detector ID X0", Converted->GetDetectorID(), MString("X0")) && Passed;
    Passed = Evaluate("Convert()", "representative crystal ID", "Convert() transfers crystal ID 2", Converted->GetCrystalID(), (unsigned int) 2) && Passed;
    Passed = Evaluate("Convert()", "representative voxel X ID", "Convert() transfers voxel X ID 0", Converted->GetReadOutElement()->GetVoxelXID(), (unsigned int) 0) && Passed;
    Passed = Evaluate("Convert()", "representative voxel Y ID", "Convert() transfers voxel Y ID 1", Converted->GetReadOutElement()->GetVoxelYID(), (unsigned int) 1) && Passed;
    Passed = Evaluate("Convert()", "representative voxel Z ID", "Convert() transfers voxel Z ID 3", Converted->GetReadOutElement()->GetVoxelZID(), (unsigned int) 3) && Passed;
    Passed = EvaluateNear("Convert()", "representative ADC", "Convert() transfers ADC value 812", Converted->GetADCUnits(), 812.0, 1e-9) && Passed;
    Passed = EvaluateNear("Convert()", "representative energy", "Convert() transfers energy 511.5", Converted->GetEnergy(), 511.5, 1e-9) && Passed;
    Passed = EvaluateTrue("Convert()", "representative trigger flag", "Convert() transfers HasTriggered true", Converted->HasTriggered()) && Passed;
  }

  delete Converted;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNDEECrystalHit::TestConvertFalsePaths()
{
  bool Passed = true;

  MDEECrystalHit H;

  H.m_ROE.SetDetectorID("Y1");
  H.m_CrystalID = 0;
  H.m_VoxelInDetector = MVector(0, 0, 0);
  H.m_ADC = 0;
  H.m_Energy = 0.0;
  H.m_HasTriggered = false;

  MCrystalHit* Converted = H.Convert();

  Passed = EvaluateTrue("Convert()", "false-path allocation", "Convert() returns a non-null pointer for the false paths", Converted != nullptr) && Passed;

  if (Converted != nullptr) {
    Passed = Evaluate("Convert()", "false-path detector ID", "Convert() transfers detector ID Y1", Converted->GetDetectorID(), MString("Y1")) && Passed;
    Passed = EvaluateFalse("Convert()", "false-path trigger flag", "Convert() transfers HasTriggered false", Converted->HasTriggered()) && Passed;
    Passed = Evaluate("Convert()", "false-path crystal ID", "Convert() transfers crystal ID 0 instead of the undefined default", Converted->GetCrystalID(), (unsigned int) 0) && Passed;
    Passed = Evaluate("Convert()", "false-path voxel IDs", "Convert() transfers the voxel IDs 0 instead of the undefined defaults",
                      Converted->GetReadOutElement()->GetVoxelXID() + Converted->GetReadOutElement()->GetVoxelYID() + Converted->GetReadOutElement()->GetVoxelZID(), (unsigned int) 0) && Passed;
  }

  delete Converted;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNDEECrystalHit::TestConvertLifecycleIndependence()
{
  bool Passed = true;

  MDEECrystalHit H;

  H.m_ROE.SetDetectorID("Z0");
  H.m_CrystalID = 1;
  H.m_VoxelInDetector = MVector(1, 2, 3);
  H.m_ADC = 100;
  H.m_Energy = 50.0;
  H.m_HasTriggered = true;

  MCrystalHit* First = H.Convert();
  MCrystalHit* Second = H.Convert();

  Passed = EvaluateTrue("Convert()", "first repeated allocation", "First Convert() call returns a non-null pointer", First != nullptr) && Passed;
  Passed = EvaluateTrue("Convert()", "second repeated allocation", "Second Convert() call returns a non-null pointer", Second != nullptr) && Passed;

  if (First != nullptr && Second != nullptr) {
    Passed = EvaluateTrue("Convert()", "distinct heap objects", "Repeated Convert() calls return distinct heap objects", First != Second) && Passed;

    First->SetADCUnits(9999.0);
    First->SetCrystalID(9);
    Passed = EvaluateNear("Convert()", "state independence after mutation", "Mutating the first converted object does not affect the second converted object", Second->GetADCUnits(), 100.0, 1e-9) && Passed;
    Passed = Evaluate("Convert()", "read-out element independence", "The converted objects do not share their read-out element", Second->GetCrystalID(), (unsigned int) 1) && Passed;
  }

  delete First;
  delete Second;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNDEECrystalHit::TestConvertedRoaLine()
{
  bool Passed = true;

  MDEECrystalHit H;

  H.m_ROE.SetDetectorID("X0");
  H.m_CrystalID = 2;
  H.m_VoxelInDetector = MVector(0, 1, 3);
  H.m_ADC = 812;
  H.m_Energy = 511.5;
  H.m_HasTriggered = true;

  MCrystalHit* Converted = H.Convert();
  Passed = EvaluateTrue("Convert()", "roa line allocation", "Convert() returns a non-null pointer", Converted != nullptr) && Passed;
  if (Converted == nullptr) return Passed;

  // The read-out element of the roa line needs all five IDs, not only the detector ID
  ostringstream Out;
  Converted->StreamRoa(Out, true, true, false, false);
  Passed = Evaluate("StreamRoa()", "converted crystal hit", "The converted crystal hit is written with all five read-out element values",
                    MString(Out.str()), MString("UC X0 2 0 1 3 812 511.5 \n")) && Passed;

  delete Converted;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNDEECrystalHit Test;
  return Test.Run() == true ? 0 : 1;
}
