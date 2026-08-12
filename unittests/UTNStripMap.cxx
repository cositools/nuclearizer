/*
 * UTNStripMap.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
#include <fstream>
#include <map>
#include <unistd.h>
#include <utility>
#include <vector>
using namespace std;

// MEGAlib:
#include "MExceptions.h"
#include "MFile.h"
#include "MGlobal.h"
#include "MUnitTest.h"

// Nuclearizer:
#include "MStripMap.h"


//! Unit test class for MStripMap
class UTNStripMap : public MUnitTest
{
public:
  UTNStripMap() : MUnitTest("UTNStripMap") {}
  virtual ~UTNStripMap() {}

  virtual bool Run();

private:
  //! Test the empty map and its missing-value behavior
  bool TestDefaultConstruction();
  //! Test loading the nine-column format and forward/reverse lookups
  bool TestNineColumnFormat();
  //! Test loading the four-column format and reusing an instance
  bool TestFourColumnFormat();
  //! Test the ASIC and primary flags inferred from the read-out ID bits in the four-column format
  bool TestFourColumnReadOutIDInference();
  //! Test dropping the detectors which were not enabled during the run
  bool TestRestrictToEnabledDetectors();
  //! Test replacing LV/HV assignments from ASIC polarities
  bool TestUpdateASICPolarities();
  //! Test invalid ASIC polarity data and atomic failure behavior
  bool TestInvalidASICPolarities();
  //! Test documented boundaries, boolean tokens, and repeated loads
  bool TestBoundariesAndReloads();
  //! Test that out-of-range lookup arguments cannot alias onto a valid entry
  bool TestOutOfRangeLookups();
  //! Test malformed, out-of-range, and duplicate map records
  bool TestCorruptFiles();
  //! Test representative committed detector data when available
  bool TestCommittedMap();

  //! Create a process-unique temporary fixture path
  MString GetFixturePath(const MString& Suffix) const;
};


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestNineColumnFormat() && Passed;
  Passed = TestFourColumnFormat() && Passed;
  Passed = TestFourColumnReadOutIDInference() && Passed;
  Passed = TestRestrictToEnabledDetectors() && Passed;
  Passed = TestUpdateASICPolarities() && Passed;
  Passed = TestInvalidASICPolarities() && Passed;
  Passed = TestBoundariesAndReloads() && Passed;
  Passed = TestOutOfRangeLookups() && Passed;
  Passed = TestCorruptFiles() && Passed;
  Passed = TestCommittedMap() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNStripMap::GetFixturePath(const MString& Suffix) const
{
  return MString("/tmp/UTNStripMap_") + (unsigned int) getpid() + Suffix;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestDefaultConstruction()
{
  bool Passed = true;

  MStripMap Map;

  Passed = EvaluateTrue("UpdateASICPolarities()", "empty map", "Updating an empty map succeeds", Map.UpdateASICPolarities(vector<map<bool, vector<bool>>>())) && Passed;
  Passed = EvaluateFalse("HasReadOutID()", "empty map", "An empty map has no read-out IDs", Map.HasReadOutID(0)) && Passed;
  Passed = EvaluateFalse("HasROIDetSideStrip()", "empty map", "An empty map has no detector/side/strip tuples", Map.HasROIDetSideStrip(0, true, 0)) && Passed;
  Passed = EvaluateException<MExceptionValueNotFound>("GetDetectorID()", "unknown read-out ID", "An unknown read-out ID throws MExceptionValueNotFound", [&Map]() { Map.GetDetectorID(0); }) && Passed;
  Passed = EvaluateException<MExceptionValueNotFound>("GetReadOutID()", "unknown detector/side/strip", "An unknown detector/side/strip tuple throws MExceptionValueNotFound", [&Map]() { Map.GetReadOutID(0, true, 0); }) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestNineColumnFormat()
{
  bool Passed = true;
  const MString Fixture = GetFixturePath("_nine.map");

  ofstream Out(Fixture.Data());
  Passed = EvaluateTrue("Open()", "create nine-column fixture", "The nine-column fixture can be created", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "# comment" << endl;
    Out << "7 0 0 1 2 3 4 0 12" << endl;
    Out << endl;
    Out << "2 0 0 0 1 4 3 1 9" << endl;
    Out.close();
  }

  MStripMap Map;
  Passed = EvaluateTrue("Open()", "nine-column format", "Open() accepts the nine-column strip-map format", Map.Open(Fixture)) && Passed;
  Passed = EvaluateTrue("HasReadOutID()", "ROI 2", "The loaded map contains ROI 2", Map.HasReadOutID(2)) && Passed;
  Passed = EvaluateTrue("HasReadOutID()", "ROI 7", "The loaded map contains ROI 7", Map.HasReadOutID(7)) && Passed;
  Passed = EvaluateFalse("HasReadOutID()", "ROI 3", "The loaded map does not contain ROI 3", Map.HasReadOutID(3)) && Passed;

  // The accessors below throw for an absent read-out ID or tuple, which would abort the executable
  // before Summarize() instead of reporting a failure - guard each block with its presence check
  if (Map.HasReadOutID(2) == true) {
    Passed = Evaluate("GetDetectorID()", "ROI 2", "ROI 2 maps to detector 3", Map.GetDetectorID(2), 3u) && Passed;
    Passed = EvaluateFalse("IsLowVoltage()", "ROI 2", "ROI 2 is HV when the side column is 1", Map.IsLowVoltage(2)) && Passed;
    Passed = Evaluate("GetStripNumber()", "ROI 2", "ROI 2 maps to strip 9", Map.GetStripNumber(2), 9u) && Passed;
  }
  Passed = EvaluateTrue("HasROIDetSideStrip()", "detector 3, HV, strip 9", "The forward lookup recognizes the ROI 2 tuple", Map.HasROIDetSideStrip(3, false, 9)) && Passed;
  if (Map.HasROIDetSideStrip(3, false, 9) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 3, HV, strip 9", "The reverse lookup returns ROI 2", Map.GetReadOutID(3, false, 9), 2u) && Passed;
  }
  if (Map.HasROIDetSideStrip(4, true, 12) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 4, LV, strip 12", "The reverse lookup returns ROI 7", Map.GetReadOutID(4, true, 12), 7u) && Passed;
  }
  Passed = EvaluateException<MExceptionValueNotFound>("GetDetectorID()", "unknown ROI after load", "An unknown loaded-map ROI throws MExceptionValueNotFound", [&Map]() { Map.GetDetectorID(99); }) && Passed;
  Passed = EvaluateException<MExceptionValueNotFound>("GetReadOutID()", "unknown tuple after load", "An unknown loaded-map tuple throws MExceptionValueNotFound", [&Map]() { Map.GetReadOutID(99, true, 99); }) && Passed;

  MFile::Remove(Fixture);
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestFourColumnFormat()
{
  bool Passed = true;
  const MString Fixture = GetFixturePath("_four.map");

  ofstream Out(Fixture.Data());
  Passed = EvaluateTrue("Open()", "create four-column fixture", "The four-column fixture can be created", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "# read-out ID detector side strip" << endl;
    Out << "11 2 0 25" << endl;
    Out << "4 1 1 8" << endl;
    Out.close();
  }

  MStripMap Map;
  Passed = EvaluateTrue("Open()", "four-column format", "Open() accepts the four-column strip-map format", Map.Open(Fixture)) && Passed;
  Passed = EvaluateTrue("HasReadOutID()", "ROI 4", "The loaded map contains ROI 4", Map.HasReadOutID(4)) && Passed;
  if (Map.HasReadOutID(4) == true) {
    Passed = Evaluate("GetDetectorID()", "ROI 4", "ROI 4 maps to detector 1", Map.GetDetectorID(4), 1u) && Passed;
    Passed = EvaluateFalse("IsLowVoltage()", "ROI 4", "ROI 4 is HV when the side column is 1", Map.IsLowVoltage(4)) && Passed;
    Passed = Evaluate("GetStripNumber()", "ROI 4", "ROI 4 maps to strip 8", Map.GetStripNumber(4), 8u) && Passed;
  }
  if (Map.HasROIDetSideStrip(2, true, 25) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 2, LV, strip 25", "The reverse lookup returns ROI 11", Map.GetReadOutID(2, true, 25), 11u) && Passed;
  }

  // MStripMap::Open() reports its own errors via "if (g_Verbosity >= c_Error)", which
  // DisableDefaultStreams() does not silence, while MParser and MFile report through the
  // MEGAlib streams - suppress both, and restore g_Verbosity afterwards.
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool OpenedMissing = Map.Open(GetFixturePath("_does_not_exist.map"));
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("Open()", "missing file after successful load", "Open() returns false for a missing file", OpenedMissing) && Passed;
  Passed = EvaluateFalse("HasReadOutID()", "after failed Open()", "A failed Open() clears the previously loaded map", Map.HasReadOutID(4)) && Passed;

  MFile::Remove(Fixture);
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestFourColumnReadOutIDInference()
{
  bool Passed = true;
  const MString Fixture = GetFixturePath("_inference.map");

  // The four-column format infers the remaining fields from the read-out ID bits:
  // RTB = bit 8, DRM = bit 7, is-primary = bit 6, ASIC = bit 5, channel = bits 0-4.
  // Only the is-primary and ASIC bits are observable through the public API, and only
  // indirectly: UpdateASICPolarities() looks the polarity up by [detector][is-primary][ASIC].
  // RTB, DRM, and the channel have no getter and cannot be exercised here.
  ofstream Out(Fixture.Data());
  Passed = EvaluateTrue("Open()", "create inference fixture", "The read-out ID inference fixture can be created", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "0 3 0 10" << endl;   // is-primary 0, ASIC 0
    Out << "32 3 0 11" << endl;  // is-primary 0, ASIC 1
    Out << "64 3 0 12" << endl;  // is-primary 1, ASIC 0
    Out << "96 3 0 13" << endl;  // is-primary 1, ASIC 1
    Out.close();
  }

  MStripMap Map;
  Passed = EvaluateTrue("Open()", "four-column inference fixture", "The read-out ID inference fixture loads", Map.Open(Fixture)) && Passed;
  // IsLowVoltage() throws for an absent read-out ID, so confirm all four are present before using them
  const bool AllPresent = Map.HasReadOutID(0) && Map.HasReadOutID(32) && Map.HasReadOutID(64) && Map.HasReadOutID(96);
  Passed = EvaluateTrue("HasReadOutID()", "four-column inference fixture", "All four inference records are present", AllPresent) && Passed;
  if (AllPresent == false) {
    MFile::Remove(Fixture);
    return false;
  }

  Passed = EvaluateTrue("IsLowVoltage()", "before polarity update", "All four records start on the LV side", Map.IsLowVoltage(0) && Map.IsLowVoltage(32) && Map.IsLowVoltage(64) && Map.IsLowVoltage(96)) && Passed;

  // Give each (is-primary, ASIC) combination a distinct polarity, so that a wrongly inferred
  // bit necessarily produces a different LV/HV pattern
  vector<map<bool, vector<bool>>> ASICPolarities(4);
  ASICPolarities[3][false] = vector<bool> { true, false };
  ASICPolarities[3][true] = vector<bool> { false, true };
  Passed = EvaluateTrue("UpdateASICPolarities()", "four-column map", "ASIC polarities can be applied to a map loaded from the four-column format", Map.UpdateASICPolarities(ASICPolarities)) && Passed;

  Passed = EvaluateTrue("IsLowVoltage()", "ROI 0", "ROI 0 infers is-primary 0 and ASIC 0, which is LV", Map.IsLowVoltage(0)) && Passed;
  Passed = EvaluateFalse("IsLowVoltage()", "ROI 32", "ROI 32 infers is-primary 0 and ASIC 1, which is HV", Map.IsLowVoltage(32)) && Passed;
  Passed = EvaluateFalse("IsLowVoltage()", "ROI 64", "ROI 64 infers is-primary 1 and ASIC 0, which is HV", Map.IsLowVoltage(64)) && Passed;
  Passed = EvaluateTrue("IsLowVoltage()", "ROI 96", "ROI 96 infers is-primary 1 and ASIC 1, which is LV", Map.IsLowVoltage(96)) && Passed;

  // Guard the reverse lookups with their presence check: GetReadOutID() throws for an absent tuple,
  // which would abort the whole executable before Summarize() instead of reporting a failed test
  Passed = EvaluateTrue("HasROIDetSideStrip()", "detector 3, LV, strip 10", "The updated LV tuple is present", Map.HasROIDetSideStrip(3, true, 10)) && Passed;
  if (Map.HasROIDetSideStrip(3, true, 10) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 3, LV, strip 10", "The reverse lookup follows the updated LV side", Map.GetReadOutID(3, true, 10), 0u) && Passed;
  }
  Passed = EvaluateTrue("HasROIDetSideStrip()", "detector 3, HV, strip 11", "The updated HV tuple is present", Map.HasROIDetSideStrip(3, false, 11)) && Passed;
  if (Map.HasROIDetSideStrip(3, false, 11) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 3, HV, strip 11", "The reverse lookup follows the updated HV side", Map.GetReadOutID(3, false, 11), 32u) && Passed;
  }

  MFile::Remove(Fixture);
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestRestrictToEnabledDetectors()
{
  bool Passed = true;
  const MString Fixture = GetFixturePath("_restrict.map");

  ofstream Out(Fixture.Data());
  Passed = EvaluateTrue("Open()", "create restriction fixture", "The restriction fixture can be created", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "0 0 0 1 0 0 0 0 10" << endl;
    Out << "1 0 0 0 0 1 0 1 10" << endl;
    Out << "2 0 0 1 0 2 1 0 11" << endl;
    Out << "3 0 0 0 0 3 1 1 11" << endl;
    Out << "4 0 0 1 0 4 2 0 12" << endl;
    Out.close();
  }

  // A strip map normally covers every detector, while a run may only enable some of them
  MStripMap Map;
  Passed = EvaluateTrue("Open()", "restriction fixture", "The restriction fixture loads", Map.Open(Fixture)) && Passed;
  Passed = EvaluateTrue("RestrictToEnabledDetectors()", "detectors 0 and 2", "Restricting to a subset of the detectors succeeds", Map.RestrictToEnabledDetectors(vector<unsigned int> { 0, 2 })) && Passed;

  Passed = EvaluateTrue("HasReadOutID()", "detectors 0 and 2", "A read-out ID of a kept detector is still present", Map.HasReadOutID(0)) && Passed;
  Passed = EvaluateTrue("HasReadOutID()", "detectors 0 and 2", "The second read-out ID of a kept detector is still present", Map.HasReadOutID(1)) && Passed;
  Passed = EvaluateTrue("HasReadOutID()", "detectors 0 and 2", "The read-out ID of the other kept detector is still present", Map.HasReadOutID(4)) && Passed;
  Passed = EvaluateFalse("HasReadOutID()", "detectors 0 and 2", "A read-out ID of a dropped detector is gone", Map.HasReadOutID(2)) && Passed;
  Passed = EvaluateFalse("HasReadOutID()", "detectors 0 and 2", "The second read-out ID of a dropped detector is gone", Map.HasReadOutID(3)) && Passed;

  Passed = EvaluateTrue("HasROIDetSideStrip()", "detectors 0 and 2", "The reverse lookup of a kept detector survives", Map.HasROIDetSideStrip(0, true, 10)) && Passed;
  Passed = EvaluateFalse("HasROIDetSideStrip()", "detectors 0 and 2", "The reverse lookup of a dropped detector is gone", Map.HasROIDetSideStrip(1, true, 11)) && Passed;
  if (Map.HasROIDetSideStrip(2, true, 12) == true) {
    Passed = Evaluate("GetReadOutID()", "detectors 0 and 2", "The reverse lookup of a kept detector still returns its read-out ID", Map.GetReadOutID(2, true, 12), 4u) && Passed;
  }

  // The rejections below report via "if (g_Verbosity >= c_Error)", which DisableDefaultStreams() does
  // not silence - lower g_Verbosity for all of them and restore it before returning
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  // Restricting to detectors which are not in the map at all must not silently empty it
  bool RestrictedToUnknown = Map.RestrictToEnabledDetectors(vector<unsigned int> { 7 });
  Passed = EvaluateFalse("RestrictToEnabledDetectors()", "detector 7", "Restricting to a detector which is not in the map returns false", RestrictedToUnknown) && Passed;
  Passed = EvaluateTrue("HasReadOutID()", "detector 7", "A rejected restriction leaves the map unchanged", Map.HasReadOutID(0)) && Passed;

  bool RestrictedToNone = Map.RestrictToEnabledDetectors(vector<unsigned int>());
  Passed = EvaluateFalse("RestrictToEnabledDetectors()", "empty detector list", "Restricting to an empty detector list returns false", RestrictedToNone) && Passed;
  Passed = EvaluateTrue("HasReadOutID()", "empty detector list", "A restriction to an empty list leaves the map unchanged", Map.HasReadOutID(0)) && Passed;

  MStripMap EmptyMap;
  bool RestrictedEmpty = EmptyMap.RestrictToEnabledDetectors(vector<unsigned int> { 0 });
  Passed = EvaluateFalse("RestrictToEnabledDetectors()", "empty map", "Restricting an empty map returns false", RestrictedEmpty) && Passed;

  g_Verbosity = OldVerbosity;

  MFile::Remove(Fixture);
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestUpdateASICPolarities()
{
  bool Passed = true;
  const MString Fixture = GetFixturePath("_polarities.map");

  ofstream Out(Fixture.Data());
  Passed = EvaluateTrue("Open()", "create polarity fixture", "The polarity fixture can be created", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "20 0 0 0 0 0 2 1 40" << endl;
    Out << "21 0 0 1 0 0 2 1 41" << endl;
    Out.close();
  }

  MStripMap Map;
  Passed = EvaluateTrue("Open()", "polarity fixture", "The polarity fixture loads", Map.Open(Fixture)) && Passed;

  // The accessors below throw for an absent read-out ID - stop early rather than abort the executable
  if (Map.HasReadOutID(20) == false || Map.HasReadOutID(21) == false) {
    Passed = EvaluateTrue("HasReadOutID()", "polarity fixture", "The polarity fixture contains ROI 20 and ROI 21", false) && Passed;
    MFile::Remove(Fixture);
    return false;
  }

  Passed = EvaluateFalse("IsLowVoltage()", "before polarity update", "The fixture initially identifies ROI 20 as HV", Map.IsLowVoltage(20)) && Passed;

  vector<map<bool, vector<bool>>> ASICPolarities(3);
  ASICPolarities[2][false] = vector<bool> { true };
  ASICPolarities[2][true] = vector<bool> { true };
  Passed = EvaluateTrue("UpdateASICPolarities()", "detector 2 primary ASIC 0", "ASIC polarity update succeeds", Map.UpdateASICPolarities(ASICPolarities)) && Passed;
  Passed = EvaluateTrue("IsLowVoltage()", "after polarity update", "ASIC polarity update changes ROI 20 to LV", Map.IsLowVoltage(20)) && Passed;
  Passed = EvaluateTrue("IsLowVoltage()", "after primary polarity update", "ASIC polarity update changes ROI 21 to LV", Map.IsLowVoltage(21)) && Passed;
  Passed = EvaluateFalse("HasROIDetSideStrip()", "old HV tuple", "The old HV tuple is no longer present", Map.HasROIDetSideStrip(2, false, 40)) && Passed;
  if (Map.HasROIDetSideStrip(2, true, 40) == true) {
    Passed = Evaluate("GetReadOutID()", "new LV tuple", "The new LV tuple still maps to ROI 20", Map.GetReadOutID(2, true, 40), 20u) && Passed;
  }
  if (Map.HasROIDetSideStrip(2, true, 41) == true) {
    Passed = Evaluate("GetReadOutID()", "new LV tuple", "The new LV tuple still maps to ROI 21", Map.GetReadOutID(2, true, 41), 21u) && Passed;
  }

  MFile::Remove(Fixture);
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestInvalidASICPolarities()
{
  bool Passed = true;
  const MString Fixture = GetFixturePath("_invalid_polarities.map");

  ofstream Out(Fixture.Data());
  Passed = EvaluateTrue("Open()", "create invalid polarity fixture", "The invalid polarity fixture can be created", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "20 0 0 0 0 0 2 1 40" << endl;
    Out << "21 0 0 1 0 0 2 0 40" << endl;
    Out.close();
  }

  MStripMap Map;
  Passed = EvaluateTrue("Open()", "invalid polarity fixture", "The invalid polarity fixture loads", Map.Open(Fixture)) && Passed;

  // IsLowVoltage() below throws for an absent read-out ID - stop early rather than abort the executable
  if (Map.HasReadOutID(20) == false || Map.HasReadOutID(21) == false) {
    Passed = EvaluateTrue("HasReadOutID()", "invalid polarity fixture", "The invalid polarity fixture contains ROI 20 and ROI 21", false) && Passed;
    MFile::Remove(Fixture);
    return false;
  }

  // The failing updates below report via "if (g_Verbosity >= c_Error)", which
  // DisableDefaultStreams() does not silence - lower g_Verbosity for all of them and restore it below.
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  vector<map<bool, vector<bool>>> MissingDetector(2);
  DisableDefaultStreams();
  bool MissingDetectorResult = Map.UpdateASICPolarities(MissingDetector);
  EnableDefaultStreams();
  Passed = EvaluateFalse("UpdateASICPolarities()", "missing detector", "Missing detector polarity data returns false", MissingDetectorResult) && Passed;
  Passed = EvaluateFalse("IsLowVoltage()", "missing detector", "A failed update leaves ROI 20 unchanged", Map.IsLowVoltage(20)) && Passed;
  Passed = EvaluateTrue("IsLowVoltage()", "missing detector", "A failed update leaves ROI 21 unchanged", Map.IsLowVoltage(21)) && Passed;

  vector<map<bool, vector<bool>>> MissingPrimary(3);
  MissingPrimary[2][true] = vector<bool> { true };
  DisableDefaultStreams();
  bool MissingPrimaryResult = Map.UpdateASICPolarities(MissingPrimary);
  EnableDefaultStreams();
  Passed = EvaluateFalse("UpdateASICPolarities()", "missing primary", "Missing primary polarity data returns false", MissingPrimaryResult) && Passed;
  Passed = EvaluateFalse("IsLowVoltage()", "missing primary", "A failed update leaves ROI 20 unchanged", Map.IsLowVoltage(20)) && Passed;
  Passed = EvaluateTrue("IsLowVoltage()", "missing primary", "A failed update leaves ROI 21 unchanged", Map.IsLowVoltage(21)) && Passed;

  vector<map<bool, vector<bool>>> MissingASIC(3);
  MissingASIC[2][false] = vector<bool>();
  DisableDefaultStreams();
  bool MissingASICResult = Map.UpdateASICPolarities(MissingASIC);
  EnableDefaultStreams();
  Passed = EvaluateFalse("UpdateASICPolarities()", "missing ASIC", "Missing ASIC polarity data returns false", MissingASICResult) && Passed;
  Passed = EvaluateFalse("IsLowVoltage()", "missing ASIC", "A failed update leaves ROI 20 unchanged", Map.IsLowVoltage(20)) && Passed;
  Passed = EvaluateTrue("IsLowVoltage()", "missing ASIC", "A failed update leaves ROI 21 unchanged", Map.IsLowVoltage(21)) && Passed;

  // A (detector, side, strip) tuple must map to exactly one read-out channel, so a polarity update that
  // would collide is rejected outright. Note that the GSE writes a default polarity for every ASIC of a
  // detector that was never configured, so a strip map holding detectors the data was not taken with can
  // trigger this legitimately - see Issue #189.
  vector<map<bool, vector<bool>>> CollidingPolarities(3);
  CollidingPolarities[2][false] = vector<bool> { false };
  CollidingPolarities[2][true] = vector<bool> { false };
  DisableDefaultStreams();
  bool CollidingResult = Map.UpdateASICPolarities(CollidingPolarities);
  EnableDefaultStreams();
  Passed = EvaluateFalse("UpdateASICPolarities()", "duplicate resulting tuple", "A polarity update that creates a duplicate tuple returns false", CollidingResult) && Passed;

  // The rejected update leaves the map exactly as it was
  Passed = EvaluateFalse("IsLowVoltage()", "duplicate resulting tuple", "A colliding update leaves ROI 20 unchanged", Map.IsLowVoltage(20)) && Passed;
  Passed = EvaluateTrue("IsLowVoltage()", "duplicate resulting tuple", "A colliding update leaves ROI 21 unchanged", Map.IsLowVoltage(21)) && Passed;
  Passed = EvaluateTrue("HasROIDetSideStrip()", "duplicate resulting tuple", "The original ROI 20 tuple remains mapped", Map.HasROIDetSideStrip(2, false, 40)) && Passed;
  Passed = EvaluateTrue("HasROIDetSideStrip()", "duplicate resulting tuple", "The original ROI 21 tuple remains mapped", Map.HasROIDetSideStrip(2, true, 40)) && Passed;

  g_Verbosity = OldVerbosity;

  MFile::Remove(Fixture);
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestBoundariesAndReloads()
{
  bool Passed = true;
  const MString Fixture = GetFixturePath("_boundaries.map");

  ofstream Out(Fixture.Data());
  Passed = EvaluateTrue("Open()", "create boundary fixture", "The boundary fixture can be created", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "100 0 0 true 0 0 15 1 64" << endl;
    Out << "101 0 0 false 0 0 15 0 63" << endl;
    Out.close();
  }

  MStripMap Map;
  Passed = EvaluateTrue("Open()", "maximum detector and strip values", "Open() accepts detector 15 and strip 64", Map.Open(Fixture)) && Passed;
  if (Map.HasReadOutID(100) == true) {
    Passed = Evaluate("GetDetectorID()", "ROI 100", "ROI 100 maps to detector 15", Map.GetDetectorID(100), 15u) && Passed;
    Passed = EvaluateFalse("IsLowVoltage()", "ROI 100", "ROI 100 is HV for side 1", Map.IsLowVoltage(100)) && Passed;
    Passed = Evaluate("GetStripNumber()", "ROI 100", "ROI 100 maps to strip 64", Map.GetStripNumber(100), 64u) && Passed;
  }
  if (Map.HasReadOutID(101) == true) {
    Passed = EvaluateTrue("IsLowVoltage()", "ROI 101", "ROI 101 is LV for side 0", Map.IsLowVoltage(101)) && Passed;
  }

  Out.open(Fixture.Data(), ios::out | ios::trunc);
  Passed = EvaluateTrue("Open()", "rewrite four-column fixture", "The four-column fixture can be rewritten", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "11 2 0 25" << endl;
    Out << "12 15 1 64" << endl;
    Out.close();
  }
  Passed = EvaluateTrue("Open()", "reuse map with four-column format", "A map object can load the four-column format after the nine-column format", Map.Open(Fixture)) && Passed;
  Passed = EvaluateFalse("HasReadOutID()", "after repeated Open()", "The previous ROI 100 mapping is cleared by a successful reload", Map.HasReadOutID(100)) && Passed;
  if (Map.HasROIDetSideStrip(2, true, 25) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 2, LV, strip 25", "The reloaded map contains the new ROI 11 mapping", Map.GetReadOutID(2, true, 25), 11u) && Passed;
  }
  if (Map.HasROIDetSideStrip(15, false, 64) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 15, HV, strip 64", "The four-column format accepts detector 15 and guard-ring strip 64", Map.GetReadOutID(15, false, 64), 12u) && Passed;
  }

  Out.open(Fixture.Data(), ios::out | ios::trunc);
  Passed = EvaluateTrue("Open()", "rewrite comment-only fixture", "The comment-only fixture can be rewritten", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "# comment-only map" << endl;
    Out << endl;
    Out.close();
  }
  // Open() reports the empty-map rejection via "if (g_Verbosity >= c_Error)", which
  // DisableDefaultStreams() does not silence - lower g_Verbosity and restore it afterwards.
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();
  bool OpenedCommentsOnly = Map.Open(Fixture);
  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("Open()", "comment-only file", "Open() rejects a file with no mapping entries", OpenedCommentsOnly) && Passed;
  Passed = EvaluateFalse("HasReadOutID()", "after comment-only reload", "A failed comment-only reload clears the previous ROI", Map.HasReadOutID(11)) && Passed;
  Passed = EvaluateFalse("HasROIDetSideStrip()", "after comment-only reload", "A failed comment-only reload clears the reverse lookup", Map.HasROIDetSideStrip(2, true, 25)) && Passed;
  Passed = EvaluateException<MExceptionValueNotFound>("GetReadOutID()", "after comment-only reload", "A failed comment-only reload removes the old reverse mapping", [&Map]() { Map.GetReadOutID(2, true, 25); }) && Passed;

  MFile::Remove(Fixture);
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestOutOfRangeLookups()
{
  bool Passed = true;
  const MString Fixture = GetFixturePath("_out_of_range.map");

  ofstream Out(Fixture.Data());
  Passed = EvaluateTrue("Open()", "create out-of-range fixture", "The out-of-range fixture can be created", Out.is_open()) && Passed;
  if (Out.is_open()) {
    Out << "42 0 0 0 0 0 0 1 9" << endl;
    Out << "43 0 0 0 0 1 0 1 64" << endl;
    Out << "44 0 0 0 0 2 0 1 0" << endl;
    Out.close();
  }

  MStripMap Map;
  Passed = EvaluateTrue("Open()", "out-of-range fixture", "The out-of-range fixture loads", Map.Open(Fixture)) && Passed;
  if (Map.HasROIDetSideStrip(0, false, 9) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 0, HV, strip 9", "The in-range tuple maps to ROI 42", Map.GetReadOutID(0, false, 9), 42u) && Passed;
  }
  if (Map.HasROIDetSideStrip(0, false, 64) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 0, HV, strip 64", "The guard-ring strip 64 remains reachable", Map.GetReadOutID(0, false, 64), 43u) && Passed;
  }
  if (Map.HasROIDetSideStrip(0, false, 0) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 0, HV, strip 0", "The in-range tuple maps to ROI 44", Map.GetReadOutID(0, false, 0), 44u) && Passed;
  }

  // Strip numbers above 127 would overflow the 7-bit strip field of the lookup key into the side bit:
  // without an explicit range check, (detector 0, LV, strip 137) packs to the same key as (detector 0, HV, strip 9)
  Passed = EvaluateFalse("HasROIDetSideStrip()", "detector 0, LV, strip 137", "A strip number overflowing into the side bit does not alias onto a valid tuple", Map.HasROIDetSideStrip(0, true, 137)) && Passed;
  Passed = EvaluateException<MExceptionValueNotFound>("GetReadOutID()", "detector 0, LV, strip 137", "A strip number overflowing into the side bit throws MExceptionValueNotFound", [&Map]() { Map.GetReadOutID(0, true, 137); }) && Passed;
  Passed = EvaluateFalse("HasROIDetSideStrip()", "detector 0, LV, strip 128", "A strip number equal to the side bit does not alias onto ROI 44", Map.HasROIDetSideStrip(0, true, 128)) && Passed;
  Passed = EvaluateException<MExceptionValueNotFound>("GetReadOutID()", "detector 0, LV, strip 128", "A strip number equal to the side bit throws MExceptionValueNotFound", [&Map]() { Map.GetReadOutID(0, true, 128); }) && Passed;

  // Out-of-range detector IDs and strip numbers above the guard ring cannot collide with any in-range key, so these only verify argument validation
  Passed = EvaluateFalse("HasROIDetSideStrip()", "detector 0, HV, strip 65", "A strip number one above the guard ring is rejected", Map.HasROIDetSideStrip(0, false, 65)) && Passed;
  Passed = EvaluateFalse("HasROIDetSideStrip()", "detector 16, HV, strip 9", "A detector ID above the maximum is rejected", Map.HasROIDetSideStrip(16, false, 9)) && Passed;
  Passed = EvaluateException<MExceptionValueNotFound>("GetReadOutID()", "detector 16, HV, strip 9", "A detector ID above the maximum throws MExceptionValueNotFound", [&Map]() { Map.GetReadOutID(16, false, 9); }) && Passed;

  MFile::Remove(Fixture);
  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestCorruptFiles()
{
  bool Passed = true;

  // Every rejection below is reported via "if (g_Verbosity >= c_Error)", which
  // DisableDefaultStreams() does not silence - lower g_Verbosity for the whole sub-test
  // and restore it before returning.
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  // Each record is paired with the read-out ID it would create if it were wrongly accepted, so that
  // the HasReadOutID() check below can actually fail - probing a fixed ID 0 is vacuous for records
  // that declare a different one
  const vector<pair<MString, unsigned int>> CorruptLines = {
    { "0 0 0", 0 }, { "0 0 0 0 0", 0 }, { "0 0 0 0 0 0 0 0", 0 }, { "0 0 0 0 0 0 0 0 0 0", 0 },
    { "invalid 0 0 0 0 0 0 0 0", 0 }, { "0 invalid 0 0 0 0 0 0 0", 0 }, { "0 0 invalid 0 0 0 0 0 0", 0 },
    { "0 0 0 invalid 0 0 0 0 0", 0 }, { "0 0 0 0 invalid 0 0 0 0", 0 }, { "0 0 0 0 0 invalid 0 0 0", 0 },
    { "0 0 0 0 0 0 invalid 0 0", 0 }, { "0 0 0 0 0 0 0 invalid 0", 0 }, { "0 0 0 0 0 0 0 0 invalid", 0 },
    { "0 0 0 0 0 0 16 0 0", 0 }, { "0 0 0 0 0 0 0 0 128", 0 }, { "0 0 0 0 0 0 0 -1 0", 0 },
    { "2147483648 0 0 0 0 0 0 0 0", 2147483647u },
    // strtoul() wraps negative values modulo 2^64, so a negative can land back inside the valid range
    { "0 0 0 0 0 0 -18446744073709551613 0 9", 0 }, { "0 0 0 0 0 0 3 -18446744073709551615 9", 0 },
    { "0 0 0 0 0 0 +3 0 9", 0 }, { "0 0 0 0 0 0 -0 0 9", 0 },
    // One past each documented maximum: strip 64 and side 1 are the largest accepted values
    { "0 0 0 0 0 0 0 0 65", 0 }, { "0 0 0 0 0 0 0 2 0", 0 }
  };

  for (unsigned int i = 0; i < CorruptLines.size(); ++i) {
    const MString Fixture = GetFixturePath(MString("_corrupt_") + i + ".map");
    ofstream Out(Fixture.Data());
    Passed = EvaluateTrue("Open()", "create corrupt fixture", "A corrupt fixture can be created", Out.is_open()) && Passed;
    if (Out.is_open()) {
      Out << CorruptLines[i].first << endl;
      Out.close();
    }

    MStripMap Map;
    DisableDefaultStreams();
    bool Opened = Map.Open(Fixture);
    EnableDefaultStreams();
    Passed = EvaluateFalse("Open()", CorruptLines[i].first, "Open() rejects a malformed or out-of-range record", Opened) && Passed;
    Passed = EvaluateFalse("HasReadOutID()", CorruptLines[i].first, "A rejected corrupt file does not create a mapping", Map.HasReadOutID(CorruptLines[i].second)) && Passed;
    MFile::Remove(Fixture);
  }

  const vector<pair<MString, unsigned int>> CorruptFourColumnLines = {
    { "0 0 0", 0 }, { "0 0 0 0 0", 0 }, { "0 invalid 0 0", 0 }, { "0 0 invalid 0", 0 }, { "0 0 0 invalid", 0 },
    { "0 16 0 0", 0 }, { "0 0 2 0", 0 }, { "0 0 0 128", 0 }, { "2147483648 0 0 0", 2147483647u },
    { "0 -18446744073709551613 0 9", 0 }, { "0 +3 0 9", 0 },
    // One past the documented maximum strip number
    { "0 0 0 65", 0 }
  };
  for (unsigned int i = 0; i < CorruptFourColumnLines.size(); ++i) {
    const MString Fixture = GetFixturePath(MString("_corrupt_four_") + i + ".map");
    ofstream Out(Fixture.Data());
    Passed = EvaluateTrue("Open()", "create corrupt four-column fixture", "A corrupt four-column fixture can be created", Out.is_open()) && Passed;
    if (Out.is_open()) {
      Out << CorruptFourColumnLines[i].first << endl;
      Out.close();
    }

    MStripMap Map;
    DisableDefaultStreams();
    bool Opened = Map.Open(Fixture);
    EnableDefaultStreams();
    Passed = EvaluateFalse("Open()", CorruptFourColumnLines[i].first, "Open() rejects a malformed four-column record", Opened) && Passed;
    Passed = EvaluateFalse("HasReadOutID()", CorruptFourColumnLines[i].first, "A rejected four-column file does not create a mapping", Map.HasReadOutID(CorruptFourColumnLines[i].second)) && Passed;
    MFile::Remove(Fixture);
  }

  const MString DuplicateFixture = GetFixturePath("_corrupt_duplicates.map");
  ofstream DuplicateOut(DuplicateFixture.Data());
  Passed = EvaluateTrue("Open()", "create duplicate fixture", "The duplicate fixture can be created", DuplicateOut.is_open()) && Passed;
  if (DuplicateOut.is_open()) {
    DuplicateOut << "4 0 0 0 0 0 1 0 8" << endl;
    DuplicateOut << "4 0 0 0 0 1 1 0 9" << endl;
    DuplicateOut.close();
  }
  MStripMap DuplicateMap;
  DisableDefaultStreams();
  bool DuplicateOpened = DuplicateMap.Open(DuplicateFixture);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Open()", "duplicate ROI", "Open() rejects duplicate read-out IDs", DuplicateOpened) && Passed;
  MFile::Remove(DuplicateFixture);

  const MString DuplicateTupleFixture = GetFixturePath("_corrupt_duplicate_tuples.map");
  ofstream DuplicateTupleOut(DuplicateTupleFixture.Data());
  Passed = EvaluateTrue("Open()", "create duplicate-tuple fixture", "The duplicate-tuple fixture can be created", DuplicateTupleOut.is_open()) && Passed;
  if (DuplicateTupleOut.is_open()) {
    DuplicateTupleOut << "5 0 0 0 0 0 1 0 8" << endl;
    DuplicateTupleOut << "6 0 0 0 0 1 1 0 8" << endl;
    DuplicateTupleOut.close();
  }
  MStripMap DuplicateTupleMap;
  DisableDefaultStreams();
  bool DuplicateTupleOpened = DuplicateTupleMap.Open(DuplicateTupleFixture);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Open()", "duplicate detector/side/strip tuple", "Open() rejects duplicate detector/side/strip tuples", DuplicateTupleOpened) && Passed;
  MFile::Remove(DuplicateTupleFixture);

  const MString MixedFixture = GetFixturePath("_corrupt_mixed.map");
  ofstream MixedOut(MixedFixture.Data());
  Passed = EvaluateTrue("Open()", "create mixed fixture", "The mixed valid/invalid fixture can be created", MixedOut.is_open()) && Passed;
  if (MixedOut.is_open()) {
    MixedOut << "8 0 0 0 0 0 1 0 10" << endl;
    MixedOut << "8 invalid 0 0 0 0 1 0 11" << endl;
    MixedOut.close();
  }
  MStripMap MixedMap;
  DisableDefaultStreams();
  bool MixedOpened = MixedMap.Open(MixedFixture);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Open()", "valid record followed by corrupt record", "Open() rejects a file containing any corrupt record", MixedOpened) && Passed;
  Passed = EvaluateFalse("HasReadOutID()", "valid record followed by corrupt record", "A rejected mixed file does not retain its valid record", MixedMap.HasReadOutID(8)) && Passed;
  MFile::Remove(MixedFixture);

  // A file must not mix the four- and nine-column formats, in either order
  const vector<pair<MString, MString>> MixedFormatFixtures = {
    { "7 0 0 1 2 3 4 0 12", "11 2 0 25" },
    { "11 2 0 25", "7 0 0 1 2 3 4 0 12" }
  };
  for (unsigned int i = 0; i < MixedFormatFixtures.size(); ++i) {
    const MString Fixture = GetFixturePath(MString("_corrupt_mixed_format_") + i + ".map");
    ofstream Out(Fixture.Data());
    Passed = EvaluateTrue("Open()", "create mixed-format fixture", "A mixed-format fixture can be created", Out.is_open()) && Passed;
    if (Out.is_open()) {
      Out << MixedFormatFixtures[i].first << endl;
      Out << MixedFormatFixtures[i].second << endl;
      Out.close();
    }

    MStripMap MixedFormatMap;
    DisableDefaultStreams();
    bool MixedFormatOpened = MixedFormatMap.Open(Fixture);
    EnableDefaultStreams();
    Passed = EvaluateFalse("Open()", MixedFormatFixtures[i].first + " then " + MixedFormatFixtures[i].second, "Open() rejects a file mixing the four- and nine-column formats", MixedFormatOpened) && Passed;
    Passed = EvaluateFalse("HasReadOutID()", "mixed formats", "A rejected mixed-format file does not retain its first record", MixedFormatMap.HasReadOutID(7) || MixedFormatMap.HasReadOutID(11)) && Passed;
    MFile::Remove(Fixture);
  }

  const MString EmptyFixture = GetFixturePath("_corrupt_empty.map");
  ofstream EmptyOut(EmptyFixture.Data());
  Passed = EvaluateTrue("Open()", "create empty fixture", "The empty fixture can be created", EmptyOut.is_open()) && Passed;
  EmptyOut.close();
  MStripMap EmptyMap;
  DisableDefaultStreams();
  bool EmptyOpened = EmptyMap.Open(EmptyFixture);
  EnableDefaultStreams();
  Passed = EvaluateFalse("Open()", "empty file", "Open() rejects an empty strip map", EmptyOpened) && Passed;
  MFile::Remove(EmptyFixture);

  g_Verbosity = OldVerbosity;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripMap::TestCommittedMap()
{
  bool Passed = true;
  const char* NuclearizerEnv = getenv("NUCLEARIZER");
  if (NuclearizerEnv == nullptr || NuclearizerEnv[0] == '\0') {
    mout << "UTNStripMap: NUCLEARIZER not set - skipping committed-map test" << endl;
    return Passed;
  }

  const MString Fixture = MString(NuclearizerEnv) + "/resource/unittestdata/406-1/hp52406-1.stripmap.map";
  Passed = EvaluateTrue("Open()", "406-1 strip map", "The committed 406-1 strip map exists", MFile::Exists(Fixture)) && Passed;
  if (MFile::Exists(Fixture) == false) {
    return Passed;
  }

  MStripMap Map;
  Passed = EvaluateTrue("Open()", "406-1 strip map", "The committed strip map loads", Map.Open(Fixture)) && Passed;
  Passed = EvaluateTrue("HasReadOutID()", "406-1 strip map", "The committed strip map contains ROI 0", Map.HasReadOutID(0)) && Passed;
  if (Map.HasReadOutID(0) == true) {
    Passed = Evaluate("GetDetectorID()", "ROI 0", "ROI 0 belongs to detector 0", Map.GetDetectorID(0), 0u) && Passed;
    Passed = EvaluateFalse("IsLowVoltage()", "ROI 0", "ROI 0 is HV in the committed map", Map.IsLowVoltage(0)) && Passed;
    Passed = Evaluate("GetStripNumber()", "ROI 0", "ROI 0 maps to strip 32", Map.GetStripNumber(0), 32u) && Passed;
  }
  if (Map.HasROIDetSideStrip(0, false, 32) == true) {
    Passed = Evaluate("GetReadOutID()", "detector 0, HV, strip 32", "The committed reverse mapping returns ROI 0", Map.GetReadOutID(0, false, 32), 0u) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNStripMap", "Unit tests for MStripMap") == false) return 1;

  // Depending on debug or not debug mode - the exceptions either abort or are real exceptions
  // To make it alwasys the same here, we have to force the exceptions to not use abort
  MException::UseAbort(false);

  UTNStripMap Test;
  return Test.Run() == true ? 0 : 1;
}
