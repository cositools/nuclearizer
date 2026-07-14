/*
 * UTNStripHit.cxx
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
#include "MStripHit.h"


//! Unit test class for MStripHit
class UTNStripHit : public MUnitTest
{
public:
  UTNStripHit() : MUnitTest("UTNStripHit") {}
  virtual ~UTNStripHit() {}

  virtual bool Run();

private:
  //! Test default construction and Clear()
  bool TestDefaultConstruction();
  //! Test individual getter/setter pairs
  bool TestGettersSetters();
  //! Test MakeFlags() bit layout and ParseFlags() round-trip
  bool TestMakeParseFlags();
  //! Test AddOrigins() deduplication and sorting
  bool TestAddOrigins();
  //! Test StreamDat() / Parse() round-trip
  bool TestStreamDatParse();
  //! Test StreamRoa() conditional field output
  bool TestStreamRoa();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNStripHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestMakeParseFlags() && Passed;
  Passed = TestAddOrigins() && Passed;
  Passed = TestStreamDatParse() && Passed;
  Passed = TestStreamRoa() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripHit::TestDefaultConstruction()
{
  bool Passed = true;

  MStripHit H;

  Passed = EvaluateTrue("GetReadOutElement()", "non-null", "GetReadOutElement() returns a non-null pointer after construction", H.GetReadOutElement() != nullptr) && Passed;
  Passed = Evaluate("GetDetectorID()", "default", "Default DetectorID is undefined", H.GetDetectorID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("GetStripID()", "default", "Default StripID is undefined", H.GetStripID(), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("IsLowVoltageStrip()", "default", "Default strip type is low-voltage", H.IsLowVoltageStrip()) && Passed;
  Passed = EvaluateFalse("HasTriggered()", "default", "Default HasTriggered is false", H.HasTriggered()) && Passed;
  Passed = EvaluateNear("GetUncorrectedADCUnits()", "default", "Default UncorrectedADCUnits is 0", H.GetUncorrectedADCUnits(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetADCUnits()", "default", "Default ADCUnits is 0", H.GetADCUnits(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergy()", "default", "Default Energy is 0", H.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetEnergyResolution()", "default", "Default EnergyResolution is 0", H.GetEnergyResolution(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTAC()", "default", "Default TAC is 0", H.GetTAC(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTACResolution()", "default", "Default TACResolution is 0", H.GetTACResolution(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTiming()", "default", "Default Timing is 0", H.GetTiming(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetTimingResolution()", "default", "Default TimingResolution is 0", H.GetTimingResolution(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetPreampTemp()", "default", "Default PreampTemp is 0", H.GetPreampTemp(), 0.0, 1e-12) && Passed;
  Passed = EvaluateFalse("IsGuardRing()", "default", "Default IsGuardRing is false", H.IsGuardRing()) && Passed;
  Passed = EvaluateFalse("IsNearestNeighbor()", "default", "Default IsNearestNeighbor is false", H.IsNearestNeighbor()) && Passed;
  Passed = EvaluateFalse("HasFastTiming()", "default", "Default HasFastTiming is false", H.HasFastTiming()) && Passed;
  Passed = EvaluateFalse("HasCalibratedTiming()", "default", "Default HasCalibratedTiming is false", H.HasCalibratedTiming()) && Passed;
  Passed = Evaluate("GetOrigins().size()", "default", "Default origins list is empty", (unsigned int) H.GetOrigins().size(), (unsigned int) 0) && Passed;

  // Verify that Clear() reinstates all defaults
  H.SetDetectorID(5);
  H.SetStripID(12);
  H.IsLowVoltageStrip(true);
  H.HasTriggered(true);
  H.SetADCUnits(1234.0);
  H.SetEnergy(511.0);
  H.SetTAC(9999.0);
  H.SetTiming(42.0);
  H.IsGuardRing(true);
  H.IsNearestNeighbor(true);
  H.HasFastTiming(true);
  H.HasCalibratedTiming(true);
  H.AddOrigins({3, 7});

  H.Clear();

  Passed = Evaluate("Clear() DetectorID", "after clear", "Clear() resets DetectorID to undefined", H.GetDetectorID(), g_UnsignedIntNotDefined) && Passed;
  Passed = Evaluate("Clear() StripID", "after clear", "Clear() resets StripID to undefined", H.GetStripID(), g_UnsignedIntNotDefined) && Passed;
  Passed = EvaluateTrue("Clear() IsLowVoltageStrip", "after clear", "Clear() resets IsLowVoltageStrip to true", H.IsLowVoltageStrip()) && Passed;
  Passed = EvaluateFalse("Clear() HasTriggered", "after clear", "Clear() resets HasTriggered to false", H.HasTriggered()) && Passed;
  Passed = EvaluateNear("Clear() ADCUnits", "after clear", "Clear() resets ADCUnits to 0", H.GetADCUnits(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Clear() Energy", "after clear", "Clear() resets Energy to 0", H.GetEnergy(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Clear() TAC", "after clear", "Clear() resets TAC to 0", H.GetTAC(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Clear() Timing", "after clear", "Clear() resets Timing to 0", H.GetTiming(), 0.0, 1e-12) && Passed;
  Passed = EvaluateFalse("Clear() IsGuardRing", "after clear", "Clear() resets IsGuardRing to false", H.IsGuardRing()) && Passed;
  Passed = EvaluateFalse("Clear() IsNearestNeighbor", "after clear", "Clear() resets IsNearestNeighbor to false", H.IsNearestNeighbor()) && Passed;
  Passed = EvaluateFalse("Clear() HasFastTiming", "after clear", "Clear() resets HasFastTiming to false", H.HasFastTiming()) && Passed;
  Passed = EvaluateFalse("Clear() HasCalibratedTiming", "after clear", "Clear() resets HasCalibratedTiming to false", H.HasCalibratedTiming()) && Passed;
  Passed = Evaluate("Clear() origins", "after clear", "Clear() empties the origins list", (unsigned int) H.GetOrigins().size(), (unsigned int) 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripHit::TestGettersSetters()
{
  bool Passed = true;

  MStripHit H;

  // DetectorID
  H.SetDetectorID(3);
  Passed = Evaluate("SetDetectorID/GetDetectorID", "representative value 3", "GetDetectorID returns the representative value 3", H.GetDetectorID(), 3u) && Passed;

  // StripID
  H.SetStripID(47);
  Passed = Evaluate("SetStripID/GetStripID", "representative value 47", "GetStripID returns the representative value 47", H.GetStripID(), 47u) && Passed;

  // IsLowVoltageStrip
  H.IsLowVoltageStrip(true);
  Passed = EvaluateTrue("IsLowVoltageStrip(bool)/IsLowVoltageStrip()", "representative true", "IsLowVoltageStrip() returns true after IsLowVoltageStrip(true)", H.IsLowVoltageStrip()) && Passed;
  H.IsLowVoltageStrip(false);
  Passed = EvaluateFalse("IsLowVoltageStrip(bool)/IsLowVoltageStrip()", "representative false", "IsLowVoltageStrip() returns false after IsLowVoltageStrip(false)", H.IsLowVoltageStrip()) && Passed;

  // IsXStrip is an alias for IsLowVoltageStrip
  H.IsXStrip(true);
  Passed = EvaluateTrue("IsXStrip(bool)/IsXStrip()", "alias true", "IsXStrip() returns true after IsXStrip(true)", H.IsXStrip()) && Passed;
  Passed = EvaluateTrue("IsXStrip(bool)/IsLowVoltageStrip()", "alias true", "IsLowVoltageStrip() returns true after IsXStrip(true)", H.IsLowVoltageStrip()) && Passed;
  H.IsXStrip(false);
  Passed = EvaluateFalse("IsXStrip(bool)/IsXStrip()", "alias false", "IsXStrip() returns false after IsXStrip(false)", H.IsXStrip()) && Passed;

  // HasTriggered
  H.HasTriggered(true);
  Passed = EvaluateTrue("HasTriggered(bool)/HasTriggered()", "representative true", "HasTriggered() returns true after HasTriggered(true)", H.HasTriggered()) && Passed;
  H.HasTriggered(false);
  Passed = EvaluateFalse("HasTriggered(bool)/HasTriggered()", "representative false", "HasTriggered() returns false after HasTriggered(false)", H.HasTriggered()) && Passed;

  // UncorrectedADCUnits
  H.SetUncorrectedADCUnits(2048.5);
  Passed = EvaluateNear("SetUncorrectedADCUnits/GetUncorrectedADCUnits", "representative value", "GetUncorrectedADCUnits returns the representative value 2048.5", H.GetUncorrectedADCUnits(), 2048.5, 1e-9) && Passed;

  // ADCUnits
  H.SetADCUnits(4095.0);
  Passed = EvaluateNear("SetADCUnits/GetADCUnits", "representative value", "GetADCUnits returns the representative value 4095.0", H.GetADCUnits(), 4095.0, 1e-9) && Passed;

  // Energy
  H.SetEnergy(511.0);
  Passed = EvaluateNear("SetEnergy/GetEnergy", "representative value", "GetEnergy returns the representative value 511.0 keV", H.GetEnergy(), 511.0, 1e-9) && Passed;

  // EnergyResolution
  H.SetEnergyResolution(1.5);
  Passed = EvaluateNear("SetEnergyResolution/GetEnergyResolution", "representative value", "GetEnergyResolution returns the representative value 1.5", H.GetEnergyResolution(), 1.5, 1e-9) && Passed;

  // TAC
  H.SetTAC(10452.0);
  Passed = EvaluateNear("SetTAC/GetTAC", "representative value", "GetTAC returns the representative value 10452.0", H.GetTAC(), 10452.0, 1e-9) && Passed;

  // TACResolution
  H.SetTACResolution(3.0);
  Passed = EvaluateNear("SetTACResolution/GetTACResolution", "representative value", "GetTACResolution returns the representative value 3.0", H.GetTACResolution(), 3.0, 1e-9) && Passed;

  // Timing
  H.SetTiming(123.456);
  Passed = EvaluateNear("SetTiming/GetTiming", "representative value", "GetTiming returns the representative value 123.456 ns", H.GetTiming(), 123.456, 1e-9) && Passed;

  // TimingResolution
  H.SetTimingResolution(0.5);
  Passed = EvaluateNear("SetTimingResolution/GetTimingResolution", "representative value", "GetTimingResolution returns the representative value 0.5 ns", H.GetTimingResolution(), 0.5, 1e-9) && Passed;

  // PreampTemp
  H.SetPreampTemp(25.3);
  Passed = EvaluateNear("SetPreampTemp/GetPreampTemp", "representative value", "GetPreampTemp returns the representative value 25.3 degrees C", H.GetPreampTemp(), 25.3, 1e-9) && Passed;

  // IsGuardRing
  H.IsGuardRing(true);
  Passed = EvaluateTrue("IsGuardRing(bool)/IsGuardRing()", "representative true", "IsGuardRing() returns true after IsGuardRing(true)", H.IsGuardRing()) && Passed;
  H.IsGuardRing(false);
  Passed = EvaluateFalse("IsGuardRing(bool)/IsGuardRing()", "representative false", "IsGuardRing() returns false after IsGuardRing(false)", H.IsGuardRing()) && Passed;

  // IsNearestNeighbor
  H.IsNearestNeighbor(true);
  Passed = EvaluateTrue("IsNearestNeighbor(bool)/IsNearestNeighbor()", "representative true", "IsNearestNeighbor() returns true after IsNearestNeighbor(true)", H.IsNearestNeighbor()) && Passed;
  H.IsNearestNeighbor(false);
  Passed = EvaluateFalse("IsNearestNeighbor(bool)/IsNearestNeighbor()", "representative false", "IsNearestNeighbor() returns false after IsNearestNeighbor(false)", H.IsNearestNeighbor()) && Passed;

  // HasFastTiming
  H.HasFastTiming(true);
  Passed = EvaluateTrue("HasFastTiming(bool)/HasFastTiming()", "representative true", "HasFastTiming() returns true after HasFastTiming(true)", H.HasFastTiming()) && Passed;
  H.HasFastTiming(false);
  Passed = EvaluateFalse("HasFastTiming(bool)/HasFastTiming()", "representative false", "HasFastTiming() returns false after HasFastTiming(false)", H.HasFastTiming()) && Passed;

  // HasCalibratedTiming
  H.HasCalibratedTiming(true);
  Passed = EvaluateTrue("HasCalibratedTiming(bool)/HasCalibratedTiming()", "representative true", "HasCalibratedTiming() returns true after HasCalibratedTiming(true)", H.HasCalibratedTiming()) && Passed;
  H.HasCalibratedTiming(false);
  Passed = EvaluateFalse("HasCalibratedTiming(bool)/HasCalibratedTiming()", "representative false", "HasCalibratedTiming() returns false after HasCalibratedTiming(false)", H.HasCalibratedTiming()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripHit::TestMakeParseFlags()
{
  bool Passed = true;

  // MakeFlags() bit layout (from source):
  //   bit 0 (value 1) = IsGuardRing
  //   bit 1 (value 2) = IsNearestNeighbor
  //   bit 2 (value 4) = HasFastTiming

  MStripHit H;

  // All flags off → 0
  H.IsGuardRing(false);
  H.IsNearestNeighbor(false);
  H.HasFastTiming(false);
  Passed = Evaluate("MakeFlags()", "all off", "MakeFlags() returns 0 when no flag is set",
                    H.MakeFlags(), (unsigned int) 0b000) && Passed;

  // Only IsGuardRing → bit 0 = 1
  H.IsGuardRing(true);
  H.IsNearestNeighbor(false);
  H.HasFastTiming(false);
  Passed = Evaluate("MakeFlags()", "guard ring only", "MakeFlags() returns 1 when only IsGuardRing is set",
                    H.MakeFlags(), (unsigned int) 0b001) && Passed;

  // Only IsNearestNeighbor → bit 1 = 2
  H.IsGuardRing(false);
  H.IsNearestNeighbor(true);
  H.HasFastTiming(false);
  Passed = Evaluate("MakeFlags()", "nearest neighbor only", "MakeFlags() returns 2 when only IsNearestNeighbor is set",
                    H.MakeFlags(), (unsigned int) 0b010) && Passed;

  // Only HasFastTiming → bit 2 = 4
  H.IsGuardRing(false);
  H.IsNearestNeighbor(false);
  H.HasFastTiming(true);
  Passed = Evaluate("MakeFlags()", "fast timing only", "MakeFlags() returns 4 when only HasFastTiming is set",
                    H.MakeFlags(), (unsigned int) 0b100) && Passed;

  // All three flags → 7
  H.IsGuardRing(true);
  H.IsNearestNeighbor(true);
  H.HasFastTiming(true);
  Passed = Evaluate("MakeFlags()", "all flags", "MakeFlags() returns 7 when all three flags are set",
                    H.MakeFlags(), (unsigned int) 0b111) && Passed;

  // ParseFlags() round-trip: flags=5 (guard ring + fast timing, no nearest neighbor)
  H.Clear();
  H.ParseFlags(0b101u);
  Passed = EvaluateTrue("ParseFlags()", "guard ring bit", "ParseFlags(0b101) sets IsGuardRing true",
                        H.IsGuardRing() == true) && Passed;
  Passed = EvaluateFalse("ParseFlags()", "nearest neighbor bit", "ParseFlags(0b101) leaves IsNearestNeighbor false",
                         H.IsNearestNeighbor()) && Passed;
  Passed = EvaluateTrue("ParseFlags()", "fast timing bit", "ParseFlags(0b101) sets HasFastTiming true",
                        H.HasFastTiming() == true) && Passed;
  Passed = Evaluate("ParseFlags()", "round-trip", "MakeFlags() reproduces the representative flags value 5 after ParseFlags(5)",
                    H.MakeFlags(), (unsigned int) 0b101) && Passed;

  // ParseFlags(0) clears all flags
  H.IsGuardRing(true);
  H.IsNearestNeighbor(true);
  H.HasFastTiming(true);
  H.ParseFlags(0);
  Passed = EvaluateFalse("ParseFlags(0)", "guard ring cleared", "ParseFlags(0) clears IsGuardRing", H.IsGuardRing()) && Passed;
  Passed = EvaluateFalse("ParseFlags(0)", "nearest neighbor cleared", "ParseFlags(0) clears IsNearestNeighbor", H.IsNearestNeighbor()) && Passed;
  Passed = EvaluateFalse("ParseFlags(0)", "fast timing cleared", "ParseFlags(0) clears HasFastTiming", H.HasFastTiming()) && Passed;

  // HasCalibratedTiming is intentionally not part of the bit mask
  H.Clear();
  H.HasCalibratedTiming(false);
  Passed = Evaluate("MakeFlags()", "calibrated timing false", "MakeFlags() is unchanged when HasCalibratedTiming is false",
                    H.MakeFlags(), (unsigned int) 0b000) && Passed;
  H.HasCalibratedTiming(true);
  Passed = Evaluate("MakeFlags()", "calibrated timing true", "MakeFlags() is unchanged when HasCalibratedTiming is true",
                    H.MakeFlags(), (unsigned int) 0b000) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripHit::TestAddOrigins()
{
  bool Passed = true;

  MStripHit H;

  // Empty after default construction
  Passed = Evaluate("GetOrigins()", "empty default", "Origins list is empty after construction",
                    (unsigned int) H.GetOrigins().size(), (unsigned int) 0) && Passed;

  // Add a set of origins
  H.AddOrigins({5, 3, 8, 1});
  vector<int> Origins = H.GetOrigins();
  Passed = Evaluate("AddOrigins()", "count after first add", "Origins list has 4 entries after adding {5,3,8,1}",
                    (unsigned int) Origins.size(), (unsigned int) 4) && Passed;

  // Verify sorted order
  if (Origins.size() == 4) {
    Passed = Evaluate("AddOrigins()", "sorted element 0", "Origins are sorted: first element is 1",
                      Origins[0], 1) && Passed;
    Passed = Evaluate("AddOrigins()", "sorted element 1", "Origins are sorted: second element is 3",
                      Origins[1], 3) && Passed;
    Passed = Evaluate("AddOrigins()", "sorted element 2", "Origins are sorted: third element is 5",
                      Origins[2], 5) && Passed;
    Passed = Evaluate("AddOrigins()", "sorted element 3", "Origins are sorted: fourth element is 8",
                      Origins[3], 8) && Passed;
  }

  // Add again with duplicates: {3, 7, 8} — 3 and 8 are already present
  H.AddOrigins({3, 7, 8});
  Origins = H.GetOrigins();
  Passed = Evaluate("AddOrigins()", "deduplicated count", "Origins list has 5 unique entries after adding {3,7,8} to {1,3,5,8}",
                    (unsigned int) Origins.size(), (unsigned int) 5) && Passed;

  if (Origins.size() == 5) {
    Passed = Evaluate("AddOrigins()", "dedup element 0", "Origins after dedup: first element is 1",
                      Origins[0], 1) && Passed;
    Passed = Evaluate("AddOrigins()", "dedup element 1", "Origins after dedup: second element is 3",
                      Origins[1], 3) && Passed;
    Passed = Evaluate("AddOrigins()", "dedup element 2", "Origins after dedup: third element is 5",
                      Origins[2], 5) && Passed;
    Passed = Evaluate("AddOrigins()", "dedup element 3", "Origins after dedup: fourth element is 7",
                      Origins[3], 7) && Passed;
    Passed = Evaluate("AddOrigins()", "dedup element 4", "Origins after dedup: fifth element is 8",
                      Origins[4], 8) && Passed;
  }

  // Add an empty list — count unchanged
  H.AddOrigins({});
  Passed = Evaluate("AddOrigins()", "empty add", "Origins count is unchanged after adding an empty list",
                    (unsigned int) H.GetOrigins().size(), (unsigned int) 5) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripHit::TestStreamDatParse()
{
  bool Passed = true;

  // Set up a representative strip hit with representative values.
  MStripHit Writer;
  Writer.SetDetectorID(2);
  Writer.IsLowVoltageStrip(true);
  Writer.SetStripID(37);
  Writer.HasTriggered(true);
  Writer.SetTiming(500.123);
  Writer.SetUncorrectedADCUnits(3000.25);
  Writer.SetADCUnits(2950.75);
  Writer.SetEnergy(662.0);
  Writer.SetEnergyResolution(2.5);
  Writer.IsGuardRing(false);
  Writer.IsNearestNeighbor(true);
  Writer.HasFastTiming(true);
  // Expected flags: IsNearestNeighbor(bit 1) + HasFastTiming(bit 2) = 0b110 = 6

  // Stream to string
  ostringstream Out;
  Passed = EvaluateTrue("StreamDat()", "representative values", "StreamDat() returns true for a representative strip hit",
                        Writer.StreamDat(Out)) && Passed;

  MString Line(Out.str().c_str());
  Passed = EvaluateTrue("StreamDat()", "non-empty output", "StreamDat() produces non-empty output",
                        Line.IsEmpty() == false) && Passed;

  // Parse back into a fresh instance
  MStripHit Reader;
  Passed = EvaluateTrue("Parse()", "representative line", "Parse() returns true for a representative StreamDat() line",
                        Reader.Parse(Line)) && Passed;

  Passed = Evaluate("Parse()", "DetectorID", "Parse() restores the representative DetectorID 2",
                    Reader.GetDetectorID(), 2u) && Passed;
  Passed = Evaluate("Parse()", "StripID", "Parse() restores the representative StripID 37",
                    Reader.GetStripID(), 37u) && Passed;
  Passed = EvaluateTrue("Parse()", "IsLowVoltageStrip", "Parse() restores the representative low-voltage strip flag",
                        Reader.IsLowVoltageStrip() == true) && Passed;
  Passed = EvaluateTrue("Parse()", "HasTriggered", "Parse() restores the representative HasTriggered flag",
                        Reader.HasTriggered() == true) && Passed;
  Passed = EvaluateNear("Parse()", "Timing", "Parse() restores the representative Timing value 500.123",
                        Reader.GetTiming(), 500.123, 1e-6) && Passed;
  Passed = EvaluateNear("Parse()", "UncorrectedADCUnits", "Parse() restores the representative UncorrectedADCUnits value 3000.25",
                        Reader.GetUncorrectedADCUnits(), 3000.25, 1e-6) && Passed;
  Passed = EvaluateNear("Parse()", "ADCUnits", "Parse() restores the representative ADCUnits value 2950.75",
                        Reader.GetADCUnits(), 2950.75, 1e-6) && Passed;
  Passed = EvaluateNear("Parse()", "Energy", "Parse() restores the representative Energy value 662.0",
                        Reader.GetEnergy(), 662.0, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "EnergyResolution", "Parse() restores the representative EnergyResolution value 2.5",
                        Reader.GetEnergyResolution(), 2.5, 1e-4) && Passed;
  Passed = EvaluateNear("Parse()", "TAC", "Parse() leaves TAC at its default value because StreamDat() does not persist it",
                        Reader.GetTAC(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("Parse()", "PreampTemp", "Parse() leaves PreampTemp at its default value because StreamDat() does not persist it",
                        Reader.GetPreampTemp(), 0.0, 1e-12) && Passed;
  Passed = EvaluateFalse("Parse()", "HasCalibratedTiming", "Parse() leaves HasCalibratedTiming false because StreamDat() does not persist it",
                         Reader.HasCalibratedTiming()) && Passed;
  Passed = EvaluateFalse("Parse()", "IsGuardRing", "Parse() restores IsGuardRing false via flags", Reader.IsGuardRing()) && Passed;
  Passed = EvaluateTrue("Parse()", "IsNearestNeighbor", "Parse() restores IsNearestNeighbor true via flags", Reader.IsNearestNeighbor()) && Passed;
  Passed = EvaluateTrue("Parse()", "HasFastTiming", "Parse() restores HasFastTiming true via flags", Reader.HasFastTiming()) && Passed;

  // High-voltage strip round-trip: verify 'h' marker and HasTriggered(false) survive StreamDat/Parse
  {
    MStripHit WriterHV;
    WriterHV.SetDetectorID(5);
    WriterHV.IsLowVoltageStrip(false);
    WriterHV.SetStripID(18);
    WriterHV.HasTriggered(false);
    WriterHV.SetTiming(99.5);
    WriterHV.SetUncorrectedADCUnits(1024.0);
    WriterHV.SetADCUnits(1000.0);
    WriterHV.SetEnergy(356.0);
    WriterHV.SetEnergyResolution(1.2);
    ostringstream OutHV;
    WriterHV.StreamDat(OutHV);
    MString LineHV(OutHV.str().c_str());
    MStripHit ReaderHV;
    Passed = EvaluateTrue("Parse()", "HV line", "Parse() returns true for a high-voltage strip line",
                          ReaderHV.Parse(LineHV)) && Passed;
    Passed = EvaluateFalse("Parse()", "HV IsLowVoltageStrip", "Parse() restores IsLowVoltageStrip false for a high-voltage strip",
                           ReaderHV.IsLowVoltageStrip()) && Passed;
    Passed = EvaluateFalse("Parse()", "HV HasTriggered false", "Parse() restores HasTriggered false for the high-voltage strip",
                           ReaderHV.HasTriggered()) && Passed;
    Passed = EvaluateNear("Parse()", "HV Timing", "Parse() restores Timing 99.5 for the high-voltage strip",
                          ReaderHV.GetTiming(), 99.5, 1e-6) && Passed;
  }

  // The malformed-line cases below exercise MStripHit::Parse() error paths that
  // print via "if (g_Verbosity >= c_Error)". Lower g_Verbosity to c_Quiet to
  // silence the expected diagnostics, and restore it afterwards.
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  // Parse() returns false for a non-SH line
  MString NonSHLine("UH 0 37 l 2950 10000 4");
  MStripHit Dummy;
  Passed = EvaluateFalse("Parse()", "non-SH line", "Parse() returns false for a line that does not start with 'SH'",
                         Dummy.Parse(NonSHLine)) && Passed;

  // Parse() returns false for an invalid strip face marker
  MString InvalidFaceLine("SH 2 x 37 1 500 3000 2950 662.0 2.5 4");
  Passed = EvaluateFalse("Parse()", "invalid face", "Parse() returns false for a line with an unknown detector face",
                         Dummy.Parse(InvalidFaceLine)) && Passed;

  // Parse() returns false for a line shorter than 3 characters
  MString ShortLine("SH");
  Passed = EvaluateFalse("Parse()", "short line", "Parse() returns false for a line shorter than 3 characters",
                         Dummy.Parse(ShortLine)) && Passed;

  // Parse() returns false for a line with too few fields
  MString TooFewFieldsLine("SH 2 l 37 1 500");
  Passed = EvaluateFalse("Parse()", "too few fields", "Parse() returns false for a line with fewer than 10 fields",
                         Dummy.Parse(TooFewFieldsLine)) && Passed;

  g_Verbosity = OldVerbosity;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNStripHit::TestStreamRoa()
{
  bool Passed = true;

  MStripHit H;
  H.SetDetectorID(0);
  H.SetStripID(41);
  H.IsLowVoltageStrip(true);
  H.SetADCUnits(4053.0);
  H.SetTAC(10452.0);
  H.SetPreampTemp(22.1);
  H.SetEnergy(661.7);
  H.SetTiming(123.0);
  H.IsGuardRing(false);
  H.IsNearestNeighbor(false);
  H.HasFastTiming(true);
  // MakeFlags() = 0b100 = 4
  H.AddOrigins({2, 5});

  // Full output: ADC + TAC + temperature + energy + timing + flags + origins
  {
    ostringstream Out;
    H.StreamRoa(Out, true, true, true, true, true, true, true);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "starts with UH", "Full StreamRoa() output starts with 'UH'",
                          S.BeginsWith("UH")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "contains strip 41", "Full StreamRoa() output contains strip ID 41",
                          S.Contains("41")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "contains lv marker", "Full StreamRoa() output contains low-voltage marker 'l'",
                          S.Contains(" l ")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "contains ADC", "Full StreamRoa() output contains ADC value 4053",
                          S.Contains("4053")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "contains TAC", "Full StreamRoa() output contains TAC value 10452",
                          S.Contains("10452")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "contains origins 2;5", "Full StreamRoa() output contains origin list '2;5'",
                          S.Contains("2;5")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "contains energy", "Full StreamRoa() output contains energy value 661.7",
                          S.Contains("661.7")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "contains timing", "Full StreamRoa() output contains timing value 123",
                          S.Contains("123")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "contains temperature", "Full StreamRoa() output contains temperature value 22.1",
                          S.Contains("22.1")) && Passed;
  }

  // ADC only
  {
    ostringstream Out;
    H.StreamRoa(Out, true, false, false, false, false, false, false);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "ADC only contains 4053", "ADC-only StreamRoa() output contains ADC value 4053",
                          S.Contains("4053")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "ADC only no TAC", "ADC-only StreamRoa() output does not contain TAC value 10452",
                          S.Contains("10452") == false) && Passed;
  }

  // TAC only
  {
    ostringstream Out;
    H.StreamRoa(Out, false, true, false, false, false, false, false);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "TAC only contains 10452", "TAC-only StreamRoa() output contains TAC value 10452",
                          S.Contains("10452")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "TAC only no ADC", "TAC-only StreamRoa() output does not contain ADC value 4053",
                          S.Contains("4053") == false) && Passed;
  }

  // Energy only
  {
    ostringstream Out;
    H.StreamRoa(Out, false, false, true, false, false, false, false);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "energy only contains 661.7", "Energy-only StreamRoa() output contains energy value 661.7",
                          S.Contains("661.7")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "energy only no ADC", "Energy-only StreamRoa() output does not contain ADC value 4053",
                          S.Contains("4053") == false) && Passed;
  }

  // Timing only
  {
    ostringstream Out;
    H.StreamRoa(Out, false, false, false, true, false, false, false);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "timing only contains 123", "Timing-only StreamRoa() output contains timing value 123",
                          S.Contains("123")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "timing only no ADC", "Timing-only StreamRoa() output does not contain ADC value 4053",
                          S.Contains("4053") == false) && Passed;
  }

  // Temperature only
  {
    ostringstream Out;
    H.StreamRoa(Out, false, false, false, false, true, false, false);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "temperature only contains 22.1", "Temperature-only StreamRoa() output contains temperature value 22.1",
                          S.Contains("22.1")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "temperature only no ADC", "Temperature-only StreamRoa() output does not contain ADC value 4053",
                          S.Contains("4053") == false) && Passed;
  }

  // Flags only — MakeFlags() = 4 (HasFastTiming only); check as " 4 " to distinguish from strip ID 41
  {
    ostringstream Out;
    H.StreamRoa(Out, false, false, false, false, false, true, false);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "flags only contains 4", "Flags-only StreamRoa() output contains flags value 4",
                          S.Contains(" 4 ")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "flags only no ADC", "Flags-only StreamRoa() output does not contain ADC value 4053",
                          S.Contains("4053") == false) && Passed;
  }

  // No optional fields: output contains only the fixed UH prefix fields
  {
    ostringstream Out;
    H.StreamRoa(Out, false, false, false, false, false, false, false);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "bare prefix", "Bare StreamRoa() output starts with the UH prefix",
                          S.BeginsWith("UH")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "bare no ADC", "Bare StreamRoa() output does not contain ADC value 4053",
                          S.Contains("4053") == false) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "bare no TAC", "Bare StreamRoa() output does not contain TAC value 10452",
                          S.Contains("10452") == false) && Passed;
  }

  // High-voltage strip → 'h' marker
  {
    MStripHit HV;
    HV.SetDetectorID(1);
    HV.SetStripID(12);
    HV.IsLowVoltageStrip(false);
    ostringstream Out;
    HV.StreamRoa(Out, false, false, false, false, false, false, false);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "HV marker 'h'", "StreamRoa() emits ' h ' for a high-voltage strip",
                          S.Contains(" h ")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "HV no LV marker", "StreamRoa() does not emit ' l ' for a high-voltage strip",
                          S.Contains(" l ") == false) && Passed;
  }

  // Origins: no-origins path emits '-'
  {
    MStripHit NoOrigins;
    NoOrigins.SetDetectorID(0);
    NoOrigins.SetStripID(1);
    NoOrigins.IsLowVoltageStrip(true);
    ostringstream Out;
    NoOrigins.StreamRoa(Out, false, false, false, false, false, false, true);
    MString S(Out.str().c_str());
    Passed = EvaluateTrue("StreamRoa()", "no origins emits dash", "StreamRoa() emits '- ' when the origins list is empty",
                          S.Contains("- ")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNStripHit Test;
  return Test.Run() == true ? 0 : 1;
}
