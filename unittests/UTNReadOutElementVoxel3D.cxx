/*
 * UTNReadOutElementVoxel3D.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <sstream>
#include <map>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MTokenizer.h"
#include "MReadOutElement.h"

// Nuclearizer:
#include "MReadOutElementVoxel3D.h"


//! Unit test class for MReadOutElementVoxel3D
class UTNReadOutElementVoxel3D : public MUnitTest
{
 public:
  //! Default constructor
  UTNReadOutElementVoxel3D() : MUnitTest("UTNReadOutElementVoxel3D") {}
  //! Default destructor
  virtual ~UTNReadOutElementVoxel3D() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the default values, the full constructor, and Clear()
  bool TestDefaultConstruction();
  //! Test all setter/getter pairs
  bool TestGettersSetters();
  //! Test type identification
  bool TestType();
  //! Test Clone()
  bool TestClone();
  //! Test operator== and operator<
  bool TestComparison();
  //! Test ToString(), ToParsableString() and operator<<
  bool TestStream();
  //! Test Parse() including round trips and error paths
  bool TestParse();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutElementVoxel3D::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestType() && Passed;
  Passed = TestClone() && Passed;
  Passed = TestComparison() && Passed;
  Passed = TestStream() && Passed;
  Passed = TestParse() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutElementVoxel3D::TestDefaultConstruction()
{
  bool Passed = true;

  MReadOutElementVoxel3D V;
  Passed = Evaluate("MReadOutElementVoxel3D()", "default", "The default detector ID is empty", V.GetDetectorID(), MString("")) && Passed;
  Passed = Evaluate("MReadOutElementVoxel3D()", "default", "The default crystal ID is not defined", V.GetCrystalID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("MReadOutElementVoxel3D()", "default", "The default voxel X ID is not defined", V.GetVoxelXID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("MReadOutElementVoxel3D()", "default", "The default voxel Y ID is not defined", V.GetVoxelYID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("MReadOutElementVoxel3D()", "default", "The default voxel Z ID is not defined", V.GetVoxelZID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("GetNumberOfParsableElements()", "default", "There are 5 parsable elements", V.GetNumberOfParsableElements(), 5u) && Passed;

  MReadOutElementVoxel3D F("ACS_X0_0", 3, 4, 5, 6);
  Passed = Evaluate("MReadOutElementVoxel3D(...)", "full constructor", "The detector ID is ACS_X0_0", F.GetDetectorID(), MString("ACS_X0_0")) && Passed;
  Passed = Evaluate("MReadOutElementVoxel3D(...)", "full constructor", "The crystal ID is 3", F.GetCrystalID(), 3u) && Passed;
  Passed = Evaluate("MReadOutElementVoxel3D(...)", "full constructor", "The voxel X ID is 4", F.GetVoxelXID(), 4u) && Passed;
  Passed = Evaluate("MReadOutElementVoxel3D(...)", "full constructor", "The voxel Y ID is 5", F.GetVoxelYID(), 5u) && Passed;
  Passed = Evaluate("MReadOutElementVoxel3D(...)", "full constructor", "The voxel Z ID is 6", F.GetVoxelZID(), 6u) && Passed;

  F.Clear();
  Passed = Evaluate("Clear()", "after full constructor", "Clear() resets the detector ID to empty", F.GetDetectorID(), MString("")) && Passed;
  Passed = Evaluate("Clear()", "after full constructor", "Clear() resets the crystal ID", F.GetCrystalID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("Clear()", "after full constructor", "Clear() resets the voxel X ID", F.GetVoxelXID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("Clear()", "after full constructor", "Clear() resets the voxel Y ID", F.GetVoxelYID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("Clear()", "after full constructor", "Clear() resets the voxel Z ID", F.GetVoxelZID(), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("Clear()", "after full constructor", "A cleared element equals a default-constructed one", F == V) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutElementVoxel3D::TestGettersSetters()
{
  bool Passed = true;

  MReadOutElementVoxel3D V;
  V.SetDetectorID("ACS_Y1_3");
  V.SetCrystalID(12);
  V.SetVoxelXID(1);
  V.SetVoxelYID(2);
  V.SetVoxelZID(7);
  Passed = Evaluate("SetDetectorID/GetDetectorID", "ACS_Y1_3", "GetDetectorID returns ACS_Y1_3", V.GetDetectorID(), MString("ACS_Y1_3")) && Passed;
  Passed = Evaluate("SetCrystalID/GetCrystalID", "12", "GetCrystalID returns 12", V.GetCrystalID(), 12u) && Passed;
  Passed = Evaluate("SetVoxelXID/GetVoxelXID", "1", "GetVoxelXID returns 1", V.GetVoxelXID(), 1u) && Passed;
  Passed = Evaluate("SetVoxelYID/GetVoxelYID", "2", "GetVoxelYID returns 2", V.GetVoxelYID(), 2u) && Passed;
  Passed = Evaluate("SetVoxelZID/GetVoxelZID", "7", "GetVoxelZID returns 7", V.GetVoxelZID(), 7u) && Passed;

  // Each setter must only touch its own field
  V.SetVoxelYID(9);
  Passed = Evaluate("SetVoxelYID()", "independence", "SetVoxelYID does not change voxel X", V.GetVoxelXID(), 1u) && Passed;
  Passed = Evaluate("SetVoxelYID()", "independence", "SetVoxelYID does not change voxel Z", V.GetVoxelZID(), 7u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutElementVoxel3D::TestType()
{
  bool Passed = true;

  MReadOutElementVoxel3D V;
  Passed = Evaluate("GetType()", "voxel3d", "GetType returns 'voxel3d'", V.GetType(), MString("voxel3d")) && Passed;
  Passed = EvaluateTrue("IsOfType()", "voxel3d", "IsOfType('voxel3d') is true", V.IsOfType("voxel3d")) && Passed;
  Passed = EvaluateFalse("IsOfType()", "single", "IsOfType('single') is false", V.IsOfType("single")) && Passed;
  Passed = EvaluateFalse("IsOfType()", "upper case", "IsOfType('Voxel3D') is case sensitive and false", V.IsOfType("Voxel3D")) && Passed;

  const MReadOutElement& Base = V;
  Passed = Evaluate("GetType()", "through base reference", "GetType is virtual and returns 'voxel3d' through a base reference", Base.GetType(), MString("voxel3d")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutElementVoxel3D::TestClone()
{
  bool Passed = true;

  MReadOutElementVoxel3D V("ACS_X1_2", 8, 0, 1, 2);
  MReadOutElementVoxel3D* C = V.Clone();
  Passed = EvaluateTrue("Clone()", "representative", "Clone() returns a non-null object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "representative", "Clone() returns a distinct object", C != &V) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "representative", "The clone equals the original", *C == V) && Passed;
    Passed = Evaluate("Clone()", "representative", "The clone has the same detector ID", C->GetDetectorID(), MString("ACS_X1_2")) && Passed;
    Passed = Evaluate("Clone()", "representative", "The clone has the same crystal ID", C->GetCrystalID(), 8u) && Passed;
    Passed = Evaluate("Clone()", "representative", "The clone has the same voxel Z ID", C->GetVoxelZID(), 2u) && Passed;
    C->SetVoxelXID(5);
    Passed = Evaluate("Clone()", "independence", "Modifying the clone does not modify the original", V.GetVoxelXID(), 0u) && Passed;
  }
  delete C;

  const MReadOutElement& Base = V;
  MReadOutElement* BaseClone = Base.Clone();
  Passed = EvaluateTrue("Clone()", "through base reference", "Clone() through a base reference creates a voxel element", dynamic_cast<MReadOutElementVoxel3D*>(BaseClone) != nullptr) && Passed;
  delete BaseClone;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutElementVoxel3D::TestComparison()
{
  bool Passed = true;

  MReadOutElementVoxel3D A("ACS_X0_0", 1, 2, 3, 4);
  MReadOutElementVoxel3D B("ACS_X0_0", 1, 2, 3, 4);
  Passed = EvaluateTrue("operator==", "identical", "Identical elements are equal", A == B) && Passed;

  MReadOutElementVoxel3D D1("ACS_X0_1", 1, 2, 3, 4);
  MReadOutElementVoxel3D D2("ACS_X0_0", 9, 2, 3, 4);
  MReadOutElementVoxel3D D3("ACS_X0_0", 1, 9, 3, 4);
  MReadOutElementVoxel3D D4("ACS_X0_0", 1, 2, 9, 4);
  MReadOutElementVoxel3D D5("ACS_X0_0", 1, 2, 3, 9);
  Passed = EvaluateFalse("operator==", "different detector ID", "Elements with different detector IDs are not equal", A == D1) && Passed;
  Passed = EvaluateFalse("operator==", "different crystal ID", "Elements with different crystal IDs are not equal", A == D2) && Passed;
  Passed = EvaluateFalse("operator==", "different voxel X", "Elements with different voxel X IDs are not equal", A == D3) && Passed;
  Passed = EvaluateFalse("operator==", "different voxel Y", "Elements with different voxel Y IDs are not equal", A == D4) && Passed;
  Passed = EvaluateFalse("operator==", "different voxel Z", "Elements with different voxel Z IDs are not equal", A == D5) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MReadOutElement Single(1);
  bool DifferentType = (A == Single);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("operator==", "different element type", "A voxel element is not equal to a single read-out element", DifferentType) && Passed;

  // A strict weak ordering is required when read-out elements are used as sorted keys
  Passed = EvaluateFalse("operator<", "identical", "An element is not smaller than an identical element", A < B) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("operator<", "different crystal ID", "Elements with different crystal IDs are ordered", (A < D2) != (D2 < A))) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("operator<", "different detector ID", "Elements with different detector IDs are ordered", (A < D1) != (D1 < A))) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("operator<", "different voxel X", "Elements with different voxel X IDs are ordered", (A < D3) != (D3 < A))) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("operator<", "different voxel Y", "Elements with different voxel Y IDs are ordered", (A < D4) != (D4 < A))) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("operator<", "different voxel Z", "Elements with different voxel Z IDs are ordered", (A < D5) != (D5 < A))) && Passed;

  // For every pair exactly one of a < b, b < a, a == b holds, and the order is transitive
  vector<MReadOutElementVoxel3D> All = { A, D1, D2, D3, D4, D5 };
  bool Trichotomy = true;
  bool Transitive = true;
  for (unsigned int i = 0; i < All.size(); ++i) {
    for (unsigned int j = 0; j < All.size(); ++j) {
      int Holds = (All[i] < All[j] ? 1 : 0) + (All[j] < All[i] ? 1 : 0) + (All[i] == All[j] ? 1 : 0);
      if (Holds != 1) Trichotomy = false;
      for (unsigned int k = 0; k < All.size(); ++k) {
        if (All[i] < All[j] && All[j] < All[k] && !(All[i] < All[k])) Transitive = false;
      }
    }
  }
  Passed = ExpectedToFail(EvaluateTrue("operator<", "detector, crystal, X, Y, Z variations", "Exactly one of a < b, b < a, a == b holds for every pair", Trichotomy)) && Passed;
  Passed = EvaluateTrue("operator<", "detector, crystal, X, Y, Z variations", "The order is transitive", Transitive) && Passed;

  map<MReadOutElementVoxel3D, int> Map;
  Map[A] = 1;
  Map[D2] = 2;
  Map[D5] = 3;
  Passed = ExpectedToFail(Evaluate("operator<", "std::map keys", "Three distinct voxel elements create three map entries", (unsigned int) Map.size(), 3u)) && Passed;
  map<MReadOutElementVoxel3D, int> AllMap;
  for (unsigned int i = 0; i < All.size(); ++i) AllMap[All[i]] = i;
  Passed = ExpectedToFail(Evaluate("operator<", "std::map keys", "Elements differing in any of detector, crystal, X, Y, Z are distinct map keys", (unsigned int) AllMap.size(), 6u)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutElementVoxel3D::TestStream()
{
  bool Passed = true;

  MReadOutElementVoxel3D V("ACS_X0_0", 3, 4, 5, 6);
  Passed = Evaluate("ToParsableString()", "without descriptor", "ToParsableString(false) writes the five IDs", V.ToParsableString(false), MString("ACS_X0_0 3 4 5 6")) && Passed;
  Passed = Evaluate("ToParsableString()", "with descriptor", "ToParsableString(true) prefixes 'voxel3d'", V.ToParsableString(true), MString("voxel3d ACS_X0_0 3 4 5 6")) && Passed;
  Passed = Evaluate("ToString()", "representative", "ToString writes the human-readable form", V.ToString(), MString("DetectorID: ACS_X0_0, CrystalID: 3, VoxelID: (4, 5, 6)")) && Passed;

  ostringstream Out;
  Out<<V;
  Passed = Evaluate("operator<<", "representative", "operator<< writes ToString", MString(Out.str()), V.ToString()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutElementVoxel3D::TestParse()
{
  bool Passed = true;

  MReadOutElementVoxel3D Writer("ACS_Y1_3", 11, 0, 1, 2);
  MTokenizer T;
  T.AnalyzeFast(Writer.ToParsableString(false));
  MReadOutElementVoxel3D Reader;
  Passed = EvaluateTrue("Parse()", "round trip", "Parse() succeeds on the output of ToParsableString", Reader.Parse(T, 0)) && Passed;
  Passed = EvaluateTrue("Parse()", "round trip", "The parsed element equals the written element", Reader == Writer) && Passed;

  MTokenizer TDescriptor;
  TDescriptor.AnalyzeFast(Writer.ToParsableString(true));
  MReadOutElementVoxel3D ReaderDescriptor;
  Passed = EvaluateTrue("Parse()", "start element 1", "Parse() succeeds after skipping the descriptor", ReaderDescriptor.Parse(TDescriptor, 1)) && Passed;
  Passed = EvaluateTrue("Parse()", "start element 1", "The parsed element equals the written element", ReaderDescriptor == Writer) && Passed;

  // Repeated parse
  MTokenizer TSecond;
  TSecond.AnalyzeFast("ACS_X0_0 0 0 0 0");
  Passed = EvaluateTrue("Parse()", "repeated parse", "A second Parse() succeeds", Reader.Parse(TSecond, 0)) && Passed;
  Passed = Evaluate("Parse()", "repeated parse", "The second Parse() overwrites the detector ID", Reader.GetDetectorID(), MString("ACS_X0_0")) && Passed;
  Passed = Evaluate("Parse()", "repeated parse", "The second Parse() overwrites the crystal ID with 0", Reader.GetCrystalID(), 0u) && Passed;

  // Error path: not enough tokens
  MTokenizer TShort;
  TShort.AnalyzeFast("ACS_X0_0 1 2 3");
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool ShortResult = Reader.Parse(TShort, 0);
  bool OffsetResult = Reader.Parse(T, 1);
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("Parse()", "four tokens", "Parse() returns false with only four tokens", ShortResult) && Passed;
  Passed = EvaluateFalse("Parse()", "start element 1 of 5 tokens", "Parse() returns false if the start offset leaves only four tokens", OffsetResult) && Passed;
  Passed = Evaluate("Parse()", "failed parse", "A failed Parse() preserves the previous detector ID", Reader.GetDetectorID(), MString("ACS_X0_0")) && Passed;
  Passed = Evaluate("Parse()", "failed parse", "A failed Parse() preserves the previous crystal ID", Reader.GetCrystalID(), 0u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNReadOutElementVoxel3D Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNReadOutElementVoxel3D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
