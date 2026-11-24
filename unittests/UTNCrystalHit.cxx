/*
 * UTNCrystalHit.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <sstream>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"

// Nuclearizer:
#include "MCrystalHit.h"


//! Unit test class for MCrystalHit
class UTNCrystalHit : public MUnitTest
{
 public:
  //! Default constructor
  UTNCrystalHit() : MUnitTest("UTNCrystalHit") {}
  //! Default destructor
  virtual ~UTNCrystalHit() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test default values and Clear()
  bool TestDefaultConstruction();
  //! Test every setter/getter pair
  bool TestGettersSetters();
  //! Test AddOrigins() sorting and deduplication
  bool TestOrigins();
  //! Test MakeFlags()/ParseFlags()
  bool TestFlags();
  //! Test StreamDat() and Parse()
  bool TestStreamDatParse();
  //! Test StreamRoa() with all option combinations
  bool TestStreamRoa();
  //! Test copy construction and assignment for independent ownership of the read-out element
  bool TestCopy();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNCrystalHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestOrigins() && Passed;
  Passed = TestFlags() && Passed;
  Passed = TestStreamDatParse() && Passed;
  Passed = TestStreamRoa() && Passed;
  Passed = TestCopy() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNCrystalHit::TestDefaultConstruction()
{
  bool Passed = true;

  MCrystalHit H;
  Passed = EvaluateTrue("MCrystalHit()", "default", "The read-out element exists", H.GetReadOutElement() != nullptr) && Passed;
  Passed = Evaluate("MCrystalHit()", "default", "The default detector ID is empty", H.GetDetectorID(), MString("")) && Passed;
  Passed = Evaluate("MCrystalHit()", "default", "The default crystal ID is not defined", H.GetCrystalID(), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateFalse("MCrystalHit()", "default", "The default HasTriggered is false", H.HasTriggered()) && Passed;
  Passed = EvaluateFalse("MCrystalHit()", "default", "The default HasVetoed is false", H.HasVetoed()) && Passed;
  Passed = EvaluateNear("MCrystalHit()", "default", "The default ADC units are 0", H.GetADCUnits(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCrystalHit()", "default", "The default energy is 0", H.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MCrystalHit()", "default", "The default energy resolution is 0", H.GetEnergyResolution(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("MCrystalHit()", "default", "The default origin list is empty", (unsigned int) H.GetOrigins().size(), 0u) && Passed;

  H.SetDetectorID("ACS_X0_0");
  H.SetCrystalID(4);
  H.SetVoxelXID(1);
  H.SetVoxelYID(2);
  H.SetVoxelZID(3);
  H.HasTriggered(true);
  H.HasVetoed(true);
  H.SetADCUnits(1000);
  H.SetEnergy(511);
  H.SetEnergyResolution(3);
  H.AddOrigins({ 1, 2 });
  MReadOutElementVoxel3D* ROE = H.GetReadOutElement();
  H.Clear();

  Passed = EvaluateTrue("Clear()", "all fields set", "Clear() keeps the same read-out element object", H.GetReadOutElement() == ROE) && Passed;
  Passed = Evaluate("Clear()", "all fields set", "Clear() resets the detector ID", H.GetDetectorID(), MString("")) && Passed;
  Passed = Evaluate("Clear()", "all fields set", "Clear() resets the crystal ID", H.GetCrystalID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("Clear()", "all fields set", "Clear() resets the voxel X ID", H.GetReadOutElement()->GetVoxelXID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("Clear()", "all fields set", "Clear() resets the voxel Y ID", H.GetReadOutElement()->GetVoxelYID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("Clear()", "all fields set", "Clear() resets the voxel Z ID", H.GetReadOutElement()->GetVoxelZID(), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateFalse("Clear()", "all fields set", "Clear() resets HasTriggered", H.HasTriggered()) && Passed;
  Passed = EvaluateFalse("Clear()", "all fields set", "Clear() resets HasVetoed", H.HasVetoed()) && Passed;
  Passed = EvaluateNear("Clear()", "all fields set", "Clear() resets the ADC units", H.GetADCUnits(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Clear()", "all fields set", "Clear() resets the energy", H.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Clear()", "all fields set", "Clear() resets the energy resolution", H.GetEnergyResolution(), 0.0, 1e-12) && Passed;
  Passed = Evaluate("Clear()", "all fields set", "Clear() empties the origins", (unsigned int) H.GetOrigins().size(), 0u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNCrystalHit::TestGettersSetters()
{
  bool Passed = true;

  MCrystalHit H;
  H.SetDetectorID("ACS_Y1_3");
  H.SetCrystalID(17);
  H.SetVoxelXID(2);
  H.SetVoxelYID(5);
  H.SetVoxelZID(9);
  H.SetADCUnits(2345.5);
  H.SetEnergy(662.0);
  H.SetEnergyResolution(4.5);

  Passed = Evaluate("SetDetectorID/GetDetectorID", "ACS_Y1_3", "GetDetectorID returns ACS_Y1_3", H.GetDetectorID(), MString("ACS_Y1_3")) && Passed;
  Passed = Evaluate("SetCrystalID/GetCrystalID", "17", "GetCrystalID returns 17", H.GetCrystalID(), 17u) && Passed;
  Passed = Evaluate("SetVoxelXID()", "2", "The read-out element voxel X ID is 2", H.GetReadOutElement()->GetVoxelXID(), 2u) && Passed;
  Passed = Evaluate("SetVoxelYID()", "5", "The read-out element voxel Y ID is 5", H.GetReadOutElement()->GetVoxelYID(), 5u) && Passed;
  Passed = Evaluate("SetVoxelZID()", "9", "The read-out element voxel Z ID is 9", H.GetReadOutElement()->GetVoxelZID(), 9u) && Passed;
  Passed = Evaluate("SetDetectorID()", "read-out element", "The detector ID is stored in the read-out element", H.GetReadOutElement()->GetDetectorID(), MString("ACS_Y1_3")) && Passed;
  Passed = EvaluateNear("SetADCUnits/GetADCUnits", "2345.5", "GetADCUnits returns 2345.5", H.GetADCUnits(), 2345.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetEnergy/GetEnergy", "662 keV", "GetEnergy returns 662 keV", H.GetEnergy(), 662.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetEnergyResolution/GetEnergyResolution", "4.5 keV", "GetEnergyResolution returns 4.5 keV", H.GetEnergyResolution(), 4.5, 1e-12) && Passed;

  H.HasTriggered(true);
  Passed = EvaluateTrue("HasTriggered()", "true", "HasTriggered returns true after HasTriggered(true)", H.HasTriggered()) && Passed;
  H.HasTriggered(false);
  Passed = EvaluateFalse("HasTriggered()", "false", "HasTriggered returns false after HasTriggered(false)", H.HasTriggered()) && Passed;
  H.HasVetoed(true);
  Passed = EvaluateTrue("HasVetoed()", "true", "HasVetoed returns true after HasVetoed(true)", H.HasVetoed()) && Passed;
  Passed = EvaluateFalse("HasVetoed()", "independence", "HasVetoed(true) does not set HasTriggered", H.HasTriggered()) && Passed;
  H.HasVetoed(false);
  Passed = EvaluateFalse("HasVetoed()", "false", "HasVetoed returns false after HasVetoed(false)", H.HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNCrystalHit::TestOrigins()
{
  bool Passed = true;

  MCrystalHit H;
  H.AddOrigins({ 7, 3, 7, 5 });
  vector<int> O = H.GetOrigins();
  Passed = Evaluate("AddOrigins()", "7 3 7 5", "Duplicates are removed", (unsigned int) O.size(), 3u) && Passed;
  if (O.size() == 3) {
    Passed = Evaluate("AddOrigins()", "7 3 7 5", "The first origin is 3 (sorted)", O[0], 3) && Passed;
    Passed = Evaluate("AddOrigins()", "7 3 7 5", "The second origin is 5 (sorted)", O[1], 5) && Passed;
    Passed = Evaluate("AddOrigins()", "7 3 7 5", "The third origin is 7 (sorted)", O[2], 7) && Passed;
  }

  H.AddOrigins({ 1, 5 });
  O = H.GetOrigins();
  Passed = Evaluate("AddOrigins()", "second call 1 5", "A second call merges and deduplicates with existing origins", (unsigned int) O.size(), 4u) && Passed;
  if (O.size() == 4) {
    Passed = Evaluate("AddOrigins()", "second call 1 5", "The merged origins start with 1", O[0], 1) && Passed;
    Passed = Evaluate("AddOrigins()", "second call 1 5", "The merged origins end with 7", O[3], 7) && Passed;
  }

  H.AddOrigins({});
  Passed = Evaluate("AddOrigins()", "empty", "Adding an empty list keeps the origins", (unsigned int) H.GetOrigins().size(), 4u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNCrystalHit::TestFlags()
{
  bool Passed = true;

  // The crystal hit currently defines no flag bits
  MCrystalHit H;
  H.HasTriggered(true);
  H.HasVetoed(true);
  Passed = Evaluate("MakeFlags()", "triggered and vetoed", "MakeFlags returns 0 since no flags are defined", H.MakeFlags(), 0u) && Passed;

  H.ParseFlags(0xFFFFFFFFu);
  Passed = EvaluateTrue("ParseFlags()", "all bits set", "ParseFlags does not modify HasTriggered", H.HasTriggered()) && Passed;
  Passed = EvaluateTrue("ParseFlags()", "all bits set", "ParseFlags does not modify HasVetoed", H.HasVetoed()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNCrystalHit::TestStreamDatParse()
{
  bool Passed = true;

  MCrystalHit Writer;
  Writer.SetDetectorID("ACS_X0_0");
  Writer.SetCrystalID(3);
  Writer.HasTriggered(true);
  Writer.HasVetoed(false);
  Writer.SetADCUnits(1234);
  Writer.SetEnergy(511.5);
  Writer.SetEnergyResolution(2.5);

  ostringstream Out;
  Passed = EvaluateTrue("StreamDat()", "representative", "StreamDat returns true", Writer.StreamDat(Out)) && Passed;
  MString S(Out.str());
  Passed = EvaluateTrue("StreamDat()", "representative", "The output contains the detector ID", S.Contains("ACS_X0_0")) && Passed;
  Passed = EvaluateTrue("StreamDat()", "representative", "The output contains the ADC value", S.Contains("1234")) && Passed;
  Passed = EvaluateTrue("StreamDat()", "representative", "The output contains the energy", S.Contains("511.5")) && Passed;
  Passed = EvaluateTrue("StreamDat()", "representative", "The output ends with a new line", S.EndsWith("\n")) && Passed;
  // UH is the keyword of strip hits in the roa format, a crystal hit needs its own keyword
  Passed = ExpectedToFail(EvaluateFalse("StreamDat()", "representative", "The crystal hit line does not use the strip-hit keyword 'UH'", S.BeginsWith("UH"))) && Passed;
  vector<MString> Tokens = MString(S).StripBack('\n').Tokenize(" ");
  bool HasCrystalID = false;
  for (const MString& T : Tokens) {
    if (T == "3") HasCrystalID = true;
  }
  Passed = ExpectedToFail(EvaluateTrue("StreamDat()", "representative", "The output contains the crystal ID 3 as a separate value", HasCrystalID)) && Passed;

  // Round trip
  MString Line = S;
  Line.StripBackInPlace('\n');
  MCrystalHit Reader;
  Passed = ExpectedToFail(EvaluateTrue("Parse()", "StreamDat output", "Parse() accepts the line written by StreamDat()", Reader.Parse(Line))) && Passed;
  Passed = ExpectedToFail(Evaluate("Parse()", "StreamDat output", "Parse() restores the detector ID", Reader.GetDetectorID(), MString("ACS_X0_0"))) && Passed;
  Passed = ExpectedToFail(EvaluateNear("Parse()", "StreamDat output", "Parse() restores the energy", Reader.GetEnergy(), 511.5, 1e-4)) && Passed;

  MString Bad("SH 0 l 41 1 0 4053 0 0 0");
  MCrystalHit Dummy;
  Passed = EvaluateFalse("Parse()", "strip-hit line", "Parse() returns false for a strip-hit line", Dummy.Parse(Bad)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNCrystalHit::TestStreamRoa()
{
  bool Passed = true;

  MCrystalHit H;
  H.SetDetectorID("ACS_X0_0");
  H.SetCrystalID(3);
  H.SetVoxelXID(4);
  H.SetVoxelYID(5);
  H.SetVoxelZID(6);
  H.SetADCUnits(4053);
  H.SetEnergy(511);

  ostringstream Default;
  H.StreamRoa(Default);
  Passed = Evaluate("StreamRoa()", "default options", "StreamRoa writes the read-out element and the ADC value", MString(Default.str()), MString("UC ACS_X0_0 3 4 5 6 4053 \n")) && Passed;

  ostringstream All;
  H.AddOrigins({ 9, 2 });
  H.StreamRoa(All, true, true, true, true);
  Passed = Evaluate("StreamRoa()", "all options", "StreamRoa writes ADC, energy, flags and origins", MString(All.str()), MString("UC ACS_X0_0 3 4 5 6 4053 511 0 2;9\n")) && Passed;

  ostringstream EnergyOnly;
  H.StreamRoa(EnergyOnly, false, true, false, false);
  Passed = Evaluate("StreamRoa()", "energy only", "StreamRoa writes only the energy", MString(EnergyOnly.str()), MString("UC ACS_X0_0 3 4 5 6 511 \n")) && Passed;

  ostringstream NoOrigins;
  MCrystalHit Empty;
  Empty.SetDetectorID("ACS_X0_0");
  Empty.SetCrystalID(0);
  Empty.SetVoxelXID(0);
  Empty.SetVoxelYID(0);
  Empty.SetVoxelZID(0);
  Empty.StreamRoa(NoOrigins, false, false, false, true);
  Passed = Evaluate("StreamRoa()", "empty origins", "StreamRoa writes '-' for empty origins", MString(NoOrigins.str()), MString("UC ACS_X0_0 0 0 0 0 - \n")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNCrystalHit::TestCopy()
{
  bool Passed = true;

  // MCrystalHit owns its read-out element, thus a copy must own a separate one.
  // On failure the copy is intentionally leaked to avoid a double delete.
  MCrystalHit* Original = new MCrystalHit();
  Original->SetDetectorID("ACS_X0_0");
  Original->SetCrystalID(2);
  MCrystalHit* Copy = new MCrystalHit(*Original);
  bool Independent = (Copy->GetReadOutElement() != Original->GetReadOutElement());
  Passed = ExpectedToFail(EvaluateTrue("MCrystalHit(const MCrystalHit&)", "copy", "A copy owns its own read-out element", Independent)) && Passed;
  if (Independent == true) {
    Passed = Evaluate("MCrystalHit(const MCrystalHit&)", "copy", "The copy has the same crystal ID", Copy->GetCrystalID(), 2u) && Passed;
    Copy->SetCrystalID(8);
    Passed = Evaluate("MCrystalHit(const MCrystalHit&)", "copy", "Modifying the copy does not modify the original", Original->GetCrystalID(), 2u) && Passed;
    delete Copy;
  }
  delete Original;

  MCrystalHit* Source = new MCrystalHit();
  Source->SetCrystalID(5);
  MCrystalHit* Target = new MCrystalHit();
  *Target = *Source;
  bool AssignIndependent = (Target->GetReadOutElement() != Source->GetReadOutElement());
  Passed = ExpectedToFail(EvaluateTrue("operator=", "assignment", "An assigned hit owns its own read-out element", AssignIndependent)) && Passed;
  if (AssignIndependent == true) {
    Passed = Evaluate("operator=", "assignment", "The assigned hit has the same crystal ID", Target->GetCrystalID(), 5u) && Passed;
    delete Target;
  }
  delete Source;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNCrystalHit Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNCrystalHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
