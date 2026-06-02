/*
 * UTNHit.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MVector.h"
#include "MUnitTest.h"

// Nuclearizer:
#include "MHit.h"
#include "MStripHit.h"


//! Unit test class for MHit
class UTNHit : public MUnitTest
{
public:
  UTNHit() : MUnitTest("UTNHit") {}
  virtual ~UTNHit() {}

  virtual bool Run();

private:
  //! Test default construction and Clear()
  bool TestDefaultConstruction();
  //! Test individual getter/setter pairs
  bool TestGettersSetters();
  //! Test strip hit list management
  bool TestStripHitManagement();
  //! Test AddOrigins() deduplication and sorting
  bool TestAddOrigins();
  //! Test StreamDat() versions and Parse() round-trip
  bool TestStreamDatParse();
  //! Test StreamEvta() origin intersection and format
  bool TestStreamEvta();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNHit::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestStripHitManagement() && Passed;
  Passed = TestAddOrigins() && Passed;
  Passed = TestStreamDatParse() && Passed;
  Passed = TestStreamEvta() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNHit::TestDefaultConstruction()
{
  bool Passed = true;

  MHit H;

  // Construction calls Clear(), which initialises all fields: numeric fields to sentinels,
  // collections to empty, bool flags to false, and HitQuality to 0.0.

  Passed = EvaluateTrue("GetPosition()", "sentinel after construction", "Position is initialised to g_VectorNotDefined after construction",
                        H.GetPosition() == g_VectorNotDefined) && Passed;

  Passed = EvaluateTrue("GetPositionResolution()", "sentinel after construction", "PositionResolution is initialised to g_VectorNotDefined after construction",
                        H.GetPositionResolution() == g_VectorNotDefined) && Passed;

  Passed = EvaluateTrue("GetEnergy()", "sentinel after construction", "Energy is initialised to g_DoubleNotDefined after construction",
                        H.GetEnergy() == g_DoubleNotDefined) && Passed;

  Passed = EvaluateTrue("GetLVEnergy()", "sentinel after construction", "LVEnergy is initialised to g_DoubleNotDefined after construction",
                        H.GetLVEnergy() == g_DoubleNotDefined) && Passed;

  Passed = EvaluateTrue("GetHVEnergy()", "sentinel after construction", "HVEnergy is initialised to g_DoubleNotDefined after construction",
                        H.GetHVEnergy() == g_DoubleNotDefined) && Passed;

  Passed = EvaluateTrue("GetEnergyResolution()", "sentinel after construction", "EnergyResolution is initialised to g_DoubleNotDefined after construction",
                        H.GetEnergyResolution() == g_DoubleNotDefined) && Passed;

  Passed = Evaluate("GetNStripHits()", "empty after construction", "Strip hit list is empty after construction",
                    H.GetNStripHits(), (unsigned int) 0) && Passed;

  Passed = Evaluate("GetOrigins()", "empty after construction", "Origins list is empty after construction",
                    (unsigned int) H.GetOrigins().size(), (unsigned int) 0) && Passed;

  Passed = EvaluateNear("GetHitQuality()", "default after construction", "HitQuality is initialised to 0.0 after construction",
                        H.GetHitQuality(), 0.0, 1e-9) && Passed;

  Passed = EvaluateFalse("GetStripHitMultipleTimesX()", "default after construction", "StripHitMultipleTimesX is false after construction",
                         H.GetStripHitMultipleTimesX()) && Passed;
  Passed = EvaluateFalse("GetStripHitMultipleTimesY()", "default after construction", "StripHitMultipleTimesY is false after construction",
                         H.GetStripHitMultipleTimesY()) && Passed;
  Passed = EvaluateFalse("GetCrossTalkFlag()", "default after construction", "CrossTalk is false after construction",
                         H.GetCrossTalkFlag()) && Passed;
  Passed = EvaluateFalse("GetChargeLossFlag()", "default after construction", "ChargeLoss is false after construction",
                         H.GetChargeLossFlag()) && Passed;
  Passed = EvaluateFalse("GetGuardRingHitFlag()", "default after construction", "GuardRingHit is false after construction",
                         H.GetGuardRingHitFlag()) && Passed;
  Passed = EvaluateFalse("GetChargeSharing()", "default after construction", "ChargeSharing is false after construction",
                         H.GetChargeSharing()) && Passed;
  Passed = EvaluateFalse("GetChargeSharingLV()", "default after construction", "ChargeSharingLV is false after construction",
                         H.GetChargeSharingLV()) && Passed;
  Passed = EvaluateFalse("GetChargeSharingHV()", "default after construction", "ChargeSharingHV is false after construction",
                         H.GetChargeSharingHV()) && Passed;
  Passed = EvaluateFalse("GetNoDepth()", "default after construction", "NoDepth is false after construction",
                         H.GetNoDepth()) && Passed;
  Passed = EvaluateFalse("GetIsNondominantNeighborStrip()", "default after construction", "IsNonDominantNeighborStrip is false after construction",
                         H.GetIsNondominantNeighborStrip()) && Passed;

  // Verify Clear() reinstates sentinels on a populated instance
  H.SetPosition(MVector(1.0, 2.0, 3.0));
  H.SetEnergy(511.0);
  H.SetLVEnergy(200.0);
  H.SetHVEnergy(311.0);
  H.SetPositionResolution(MVector(0.1, 0.1, 0.1));
  H.SetEnergyResolution(1.5);
  H.SetHitQuality(0.75);
  H.AddOrigins({4, 7});
  H.SetCrossTalkFlag(true);
  H.SetChargeLossFlag(true);
  H.SetGuardRingHitFlag(true);
  H.SetChargeSharing(true);
  H.SetChargeSharingLV(true);
  H.SetChargeSharingHV(true);
  H.SetNoDepth(true);
  H.SetIsNondominantNeighborStrip(true);
  H.SetStripHitMultipleTimesX(true);
  H.SetStripHitMultipleTimesY(true);
  MStripHit SH;
  H.AddStripHit(&SH);

  H.Clear();

  Passed = EvaluateTrue("Clear() Position", "sentinel restored", "Clear() restores Position to g_VectorNotDefined",
                        H.GetPosition() == g_VectorNotDefined) && Passed;
  Passed = EvaluateTrue("Clear() Energy", "sentinel restored", "Clear() restores Energy to g_DoubleNotDefined",
                        H.GetEnergy() == g_DoubleNotDefined) && Passed;
  Passed = EvaluateTrue("Clear() LVEnergy", "sentinel restored", "Clear() restores LVEnergy to g_DoubleNotDefined",
                        H.GetLVEnergy() == g_DoubleNotDefined) && Passed;
  Passed = EvaluateTrue("Clear() HVEnergy", "sentinel restored", "Clear() restores HVEnergy to g_DoubleNotDefined",
                        H.GetHVEnergy() == g_DoubleNotDefined) && Passed;
  Passed = EvaluateTrue("Clear() EnergyResolution", "sentinel restored", "Clear() restores EnergyResolution to g_DoubleNotDefined",
                        H.GetEnergyResolution() == g_DoubleNotDefined) && Passed;
  Passed = EvaluateTrue("Clear() PositionResolution", "sentinel restored", "Clear() restores PositionResolution to g_VectorNotDefined",
                        H.GetPositionResolution() == g_VectorNotDefined) && Passed;
  Passed = Evaluate("Clear() StripHits", "empty restored", "Clear() empties the strip hit list",
                    H.GetNStripHits(), (unsigned int) 0) && Passed;
  Passed = Evaluate("Clear() Origins", "empty restored", "Clear() empties the origins list",
                    (unsigned int) H.GetOrigins().size(), (unsigned int) 0) && Passed;
  Passed = EvaluateNear("Clear() HitQuality", "zero restored", "Clear() restores HitQuality to 0.0",
                        H.GetHitQuality(), 0.0, 1e-9) && Passed;
  Passed = EvaluateFalse("Clear() CrossTalk", "false restored", "Clear() restores CrossTalk to false",
                         H.GetCrossTalkFlag()) && Passed;
  Passed = EvaluateFalse("Clear() ChargeLoss", "false restored", "Clear() restores ChargeLoss to false",
                         H.GetChargeLossFlag()) && Passed;
  Passed = EvaluateFalse("Clear() GuardRingHit", "false restored", "Clear() restores GuardRingHit to false",
                         H.GetGuardRingHitFlag()) && Passed;
  Passed = EvaluateFalse("Clear() ChargeSharing", "false restored", "Clear() restores ChargeSharing to false",
                         H.GetChargeSharing()) && Passed;
  Passed = EvaluateFalse("Clear() ChargeSharingLV", "false restored", "Clear() restores ChargeSharingLV to false",
                         H.GetChargeSharingLV()) && Passed;
  Passed = EvaluateFalse("Clear() ChargeSharingHV", "false restored", "Clear() restores ChargeSharingHV to false",
                         H.GetChargeSharingHV()) && Passed;
  Passed = EvaluateFalse("Clear() NoDepth", "false restored", "Clear() restores NoDepth to false",
                         H.GetNoDepth()) && Passed;
  Passed = EvaluateFalse("Clear() IsNonDominantNeighborStrip", "false restored", "Clear() restores IsNonDominantNeighborStrip to false",
                         H.GetIsNondominantNeighborStrip()) && Passed;
  Passed = EvaluateFalse("Clear() StripHitMultipleTimesX", "false restored", "Clear() restores StripHitMultipleTimesX to false",
                         H.GetStripHitMultipleTimesX()) && Passed;
  Passed = EvaluateFalse("Clear() StripHitMultipleTimesY", "false restored", "Clear() restores StripHitMultipleTimesY to false",
                         H.GetStripHitMultipleTimesY()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNHit::TestGettersSetters()
{
  bool Passed = true;

  MHit H;

  // Position
  MVector Pos(3.5, -1.2, 7.8);
  H.SetPosition(Pos);
  Passed = EvaluateNear("SetPosition/GetPosition X", "representative value", "GetPosition().X() returns the representative value 3.5",
                        H.GetPosition().GetX(), 3.5, 1e-9) && Passed;
  Passed = EvaluateNear("SetPosition/GetPosition Y", "representative value", "GetPosition().Y() returns the representative value -1.2",
                        H.GetPosition().GetY(), -1.2, 1e-9) && Passed;
  Passed = EvaluateNear("SetPosition/GetPosition Z", "representative value", "GetPosition().Z() returns the representative value 7.8",
                        H.GetPosition().GetZ(), 7.8, 1e-9) && Passed;

  // PositionResolution
  MVector PosRes(0.1, 0.2, 0.3);
  H.SetPositionResolution(PosRes);
  Passed = EvaluateNear("SetPositionResolution/GetPositionResolution X", "representative value", "GetPositionResolution().X() returns the representative value 0.1",
                        H.GetPositionResolution().GetX(), 0.1, 1e-9) && Passed;
  Passed = EvaluateNear("SetPositionResolution/GetPositionResolution Y", "representative value", "GetPositionResolution().Y() returns the representative value 0.2",
                        H.GetPositionResolution().GetY(), 0.2, 1e-9) && Passed;
  Passed = EvaluateNear("SetPositionResolution/GetPositionResolution Z", "representative value", "GetPositionResolution().Z() returns the representative value 0.3",
                        H.GetPositionResolution().GetZ(), 0.3, 1e-9) && Passed;

  // Energy
  H.SetEnergy(661.7);
  Passed = EvaluateNear("SetEnergy/GetEnergy", "representative value", "GetEnergy returns the representative value 661.7 keV",
                        H.GetEnergy(), 661.7, 1e-9) && Passed;

  // LVEnergy
  H.SetLVEnergy(330.0);
  Passed = EvaluateNear("SetLVEnergy/GetLVEnergy", "representative value", "GetLVEnergy returns the representative value 330.0 keV",
                        H.GetLVEnergy(), 330.0, 1e-9) && Passed;

  // HVEnergy
  H.SetHVEnergy(331.7);
  Passed = EvaluateNear("SetHVEnergy/GetHVEnergy", "representative value", "GetHVEnergy returns the representative value 331.7 keV",
                        H.GetHVEnergy(), 331.7, 1e-9) && Passed;

  // EnergyResolution
  H.SetEnergyResolution(2.5);
  Passed = EvaluateNear("SetEnergyResolution/GetEnergyResolution", "representative value", "GetEnergyResolution returns the representative value 2.5 keV",
                        H.GetEnergyResolution(), 2.5, 1e-9) && Passed;

  // HitQuality
  H.SetHitQuality(0.95);
  Passed = EvaluateNear("SetHitQuality/GetHitQuality", "representative value", "GetHitQuality returns the representative value 0.95",
                        H.GetHitQuality(), 0.95, 1e-9) && Passed;

  // CrossTalk flag
  H.SetCrossTalkFlag(true);
  Passed = EvaluateTrue("SetCrossTalkFlag/GetCrossTalkFlag", "representative true", "GetCrossTalkFlag returns true after SetCrossTalkFlag(true)",
                        H.GetCrossTalkFlag() == true) && Passed;
  H.SetCrossTalkFlag(false);
  Passed = EvaluateFalse("SetCrossTalkFlag/GetCrossTalkFlag", "representative false", "GetCrossTalkFlag returns false after SetCrossTalkFlag(false)",
                         H.GetCrossTalkFlag()) && Passed;

  // GuardRingHit flag
  H.SetGuardRingHitFlag(true);
  Passed = EvaluateTrue("SetGuardRingHitFlag/GetGuardRingHitFlag", "representative true", "GetGuardRingHitFlag returns true after SetGuardRingHitFlag(true)",
                        H.GetGuardRingHitFlag() == true) && Passed;
  H.SetGuardRingHitFlag(false);
  Passed = EvaluateFalse("SetGuardRingHitFlag/GetGuardRingHitFlag", "representative false", "GetGuardRingHitFlag returns false after SetGuardRingHitFlag(false)",
                         H.GetGuardRingHitFlag()) && Passed;

  // ChargeLoss flag
  H.SetChargeLossFlag(true);
  Passed = EvaluateTrue("SetChargeLossFlag/GetChargeLossFlag", "representative true", "GetChargeLossFlag returns true after SetChargeLossFlag(true)",
                        H.GetChargeLossFlag() == true) && Passed;
  H.SetChargeLossFlag(false);
  Passed = EvaluateFalse("SetChargeLossFlag/GetChargeLossFlag", "representative false", "GetChargeLossFlag returns false after SetChargeLossFlag(false)",
                         H.GetChargeLossFlag()) && Passed;

  // StripHitMultipleTimesX
  H.SetStripHitMultipleTimesX(true);
  Passed = EvaluateTrue("SetStripHitMultipleTimesX/GetStripHitMultipleTimesX", "representative true", "GetStripHitMultipleTimesX returns true after setting true",
                        H.GetStripHitMultipleTimesX() == true) && Passed;
  H.SetStripHitMultipleTimesX(false);
  Passed = EvaluateFalse("SetStripHitMultipleTimesX/GetStripHitMultipleTimesX", "representative false", "GetStripHitMultipleTimesX returns false after setting false",
                         H.GetStripHitMultipleTimesX()) && Passed;

  // StripHitMultipleTimesY
  H.SetStripHitMultipleTimesY(true);
  Passed = EvaluateTrue("SetStripHitMultipleTimesY/GetStripHitMultipleTimesY", "representative true", "GetStripHitMultipleTimesY returns true after setting true",
                        H.GetStripHitMultipleTimesY() == true) && Passed;
  H.SetStripHitMultipleTimesY(false);
  Passed = EvaluateFalse("SetStripHitMultipleTimesY/GetStripHitMultipleTimesY", "representative false", "GetStripHitMultipleTimesY returns false after setting false",
                         H.GetStripHitMultipleTimesY()) && Passed;

  // ChargeSharingLV
  H.SetChargeSharingLV(true);
  Passed = EvaluateTrue("SetChargeSharingLV/GetChargeSharingLV", "representative true", "GetChargeSharingLV returns true after SetChargeSharingLV(true)",
                        H.GetChargeSharingLV() == true) && Passed;
  H.SetChargeSharingLV(false);
  Passed = EvaluateFalse("SetChargeSharingLV/GetChargeSharingLV", "representative false", "GetChargeSharingLV returns false after SetChargeSharingLV(false)",
                         H.GetChargeSharingLV()) && Passed;

  // ChargeSharingHV
  H.SetChargeSharingHV(true);
  Passed = EvaluateTrue("SetChargeSharingHV/GetChargeSharingHV", "representative true", "GetChargeSharingHV returns true after SetChargeSharingHV(true)",
                        H.GetChargeSharingHV() == true) && Passed;
  H.SetChargeSharingHV(false);
  Passed = EvaluateFalse("SetChargeSharingHV/GetChargeSharingHV", "representative false", "GetChargeSharingHV returns false after SetChargeSharingHV(false)",
                         H.GetChargeSharingHV()) && Passed;

  // ChargeSharing (general)
  H.SetChargeSharing(true);
  Passed = EvaluateTrue("SetChargeSharing/GetChargeSharing", "representative true", "GetChargeSharing returns true after SetChargeSharing(true)",
                        H.GetChargeSharing() == true) && Passed;
  H.SetChargeSharing(false);
  Passed = EvaluateFalse("SetChargeSharing/GetChargeSharing", "representative false", "GetChargeSharing returns false after SetChargeSharing(false)",
                         H.GetChargeSharing()) && Passed;

  // NoDepth
  H.SetNoDepth(true);
  Passed = EvaluateTrue("SetNoDepth/GetNoDepth", "representative true", "GetNoDepth returns true after SetNoDepth(true)",
                        H.GetNoDepth() == true) && Passed;
  H.SetNoDepth(false);
  Passed = EvaluateFalse("SetNoDepth/GetNoDepth", "representative false", "GetNoDepth returns false after SetNoDepth(false)",
                         H.GetNoDepth()) && Passed;

  // IsNondominantNeighborStrip
  H.SetIsNondominantNeighborStrip(true);
  Passed = EvaluateTrue("SetIsNondominantNeighborStrip/GetIsNondominantNeighborStrip", "representative true", "GetIsNondominantNeighborStrip returns true after setting true",
                        H.GetIsNondominantNeighborStrip() == true) && Passed;
  H.SetIsNondominantNeighborStrip(false);
  Passed = EvaluateFalse("SetIsNondominantNeighborStrip/GetIsNondominantNeighborStrip", "representative false", "GetIsNondominantNeighborStrip returns false after setting false",
                         H.GetIsNondominantNeighborStrip()) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNHit::TestStripHitManagement()
{
  bool Passed = true;

  // MHit does NOT own the strip hits — ownership and lifetime remain with the caller.
  MStripHit SH0, SH1, SH2;

  MHit H;
  Passed = Evaluate("GetNStripHits()", "empty", "GetNStripHits() returns 0 on a fresh MHit",
                    H.GetNStripHits(), (unsigned int) 0) && Passed;

  // AddStripHit
  H.AddStripHit(&SH0);
  Passed = Evaluate("AddStripHit()", "count after one add", "GetNStripHits() returns 1 after adding one strip hit",
                    H.GetNStripHits(), (unsigned int) 1) && Passed;

  H.AddStripHit(&SH1);
  H.AddStripHit(&SH2);
  Passed = Evaluate("AddStripHit()", "count after three adds", "GetNStripHits() returns 3 after adding three strip hits",
                    H.GetNStripHits(), (unsigned int) 3) && Passed;

  // GetStripHit — pointer identity
  Passed = EvaluateTrue("GetStripHit()", "pointer identity 0", "GetStripHit(0) returns the pointer passed in the first AddStripHit()",
                        H.GetStripHit(0) == &SH0) && Passed;
  Passed = EvaluateTrue("GetStripHit()", "pointer identity 1", "GetStripHit(1) returns the pointer passed in the second AddStripHit()",
                        H.GetStripHit(1) == &SH1) && Passed;
  Passed = EvaluateTrue("GetStripHit()", "pointer identity 2", "GetStripHit(2) returns the pointer passed in the third AddStripHit()",
                        H.GetStripHit(2) == &SH2) && Passed;

  // GetStripHit out of bounds returns null (emits merr)
  DisableDefaultStreams();
  MStripHit* OutOfBounds = H.GetStripHit(99);
  EnableDefaultStreams();
  Passed = EvaluateTrue("GetStripHit()", "out of bounds returns null", "GetStripHit(99) returns nullptr when index is out of bounds",
                        OutOfBounds == nullptr) && Passed;

  // RemoveStripHit by index — removes SH1, leaving {SH0, SH2}
  H.RemoveStripHit((unsigned int) 1);
  Passed = Evaluate("RemoveStripHit(unsigned int)", "count after remove", "GetNStripHits() returns 2 after removing index 1",
                    H.GetNStripHits(), (unsigned int) 2) && Passed;
  Passed = EvaluateTrue("RemoveStripHit(unsigned int)", "remaining pointer 0", "After removing index 1, GetStripHit(0) still returns the first strip hit",
                        H.GetStripHit(0) == &SH0) && Passed;
  Passed = EvaluateTrue("RemoveStripHit(unsigned int)", "remaining pointer 1", "After removing index 1, GetStripHit(1) returns the third strip hit",
                        H.GetStripHit(1) == &SH2) && Passed;

  // RemoveStripHit by pointer — removes SH2, leaving {SH0}
  H.RemoveStripHit(&SH2);
  Passed = Evaluate("RemoveStripHit(MStripHit*)", "count after pointer remove", "GetNStripHits() returns 1 after removing by pointer",
                    H.GetNStripHits(), (unsigned int) 1) && Passed;
  Passed = EvaluateTrue("RemoveStripHit(MStripHit*)", "remaining pointer", "After removing by pointer, GetStripHit(0) still returns the first strip hit",
                        H.GetStripHit(0) == &SH0) && Passed;

  // RemoveStripHit of a pointer not in the list is a no-op
  MStripHit NotPresent;
  H.RemoveStripHit(&NotPresent);
  Passed = Evaluate("RemoveStripHit(MStripHit*)", "no-op for absent pointer", "Removing a pointer not in the list leaves the count unchanged",
                    H.GetNStripHits(), (unsigned int) 1) && Passed;

  // RemoveStripHit by index out of range is a no-op
  H.RemoveStripHit((unsigned int) 99);
  Passed = Evaluate("RemoveStripHit(unsigned int)", "no-op for out-of-range index", "Removing an out-of-range index leaves the count unchanged",
                    H.GetNStripHits(), (unsigned int) 1) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNHit::TestAddOrigins()
{
  bool Passed = true;

  MHit H;

  Passed = Evaluate("GetOrigins()", "empty default", "Origins list is empty after construction",
                    (unsigned int) H.GetOrigins().size(), (unsigned int) 0) && Passed;

  // Add {4, 1, 7, 2} — should be sorted to {1, 2, 4, 7}
  H.AddOrigins({4, 1, 7, 2});
  vector<int> Origins = H.GetOrigins();
  Passed = Evaluate("AddOrigins()", "count after first add", "Origins list has 4 entries after adding {4,1,7,2}",
                    (unsigned int) Origins.size(), (unsigned int) 4) && Passed;
  if (Origins.size() == 4) {
    Passed = Evaluate("AddOrigins()", "sorted element 0", "Origins are sorted: first element is 1",
                      Origins[0], 1) && Passed;
    Passed = Evaluate("AddOrigins()", "sorted element 1", "Origins are sorted: second element is 2",
                      Origins[1], 2) && Passed;
    Passed = Evaluate("AddOrigins()", "sorted element 2", "Origins are sorted: third element is 4",
                      Origins[2], 4) && Passed;
    Passed = Evaluate("AddOrigins()", "sorted element 3", "Origins are sorted: fourth element is 7",
                      Origins[3], 7) && Passed;
  }

  // Add {2, 5, 7} — 2 and 7 are duplicates; result should be {1, 2, 4, 5, 7}
  H.AddOrigins({2, 5, 7});
  Origins = H.GetOrigins();
  Passed = Evaluate("AddOrigins()", "deduplicated count", "Origins list has 5 unique entries after adding {2,5,7} to {1,2,4,7}",
                    (unsigned int) Origins.size(), (unsigned int) 5) && Passed;
  if (Origins.size() == 5) {
    Passed = Evaluate("AddOrigins()", "dedup element 0", "Deduped origins: element 0 is 1", Origins[0], 1) && Passed;
    Passed = Evaluate("AddOrigins()", "dedup element 1", "Deduped origins: element 1 is 2", Origins[1], 2) && Passed;
    Passed = Evaluate("AddOrigins()", "dedup element 2", "Deduped origins: element 2 is 4", Origins[2], 4) && Passed;
    Passed = Evaluate("AddOrigins()", "dedup element 3", "Deduped origins: element 3 is 5", Origins[3], 5) && Passed;
    Passed = Evaluate("AddOrigins()", "dedup element 4", "Deduped origins: element 4 is 7", Origins[4], 7) && Passed;
  }

  // Adding an empty list is a no-op
  H.AddOrigins({});
  Passed = Evaluate("AddOrigins()", "empty add no-op", "Origins count is unchanged after adding an empty list",
                    (unsigned int) H.GetOrigins().size(), (unsigned int) 5) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNHit::TestStreamDatParse()
{
  bool Passed = true;

  // StreamDat writes with default ostream precision (6 significant digits), so use values
  // that survive that round-trip exactly.
  const double X = 2.0, Y = -3.0, Z = 5.0, E = 511.0;
  const double LVE = 250.0, HVE = 261.0;
  const double Tolerance = 1e-4;

  // --- Version 1: HT x y z energy -------------------------------------------
  {
    MHit Writer;
    Writer.SetPosition(MVector(X, Y, Z));
    Writer.SetEnergy(E);

    ostringstream Out;
    Passed = EvaluateTrue("StreamDat() V1", "return value", "StreamDat(V1) returns true",
                          Writer.StreamDat(Out, 1)) && Passed;

    MString Line(Out.str().c_str());
    Passed = EvaluateTrue("StreamDat() V1", "starts with HT", "StreamDat(V1) output starts with 'HT'",
                          Line.BeginsWith("HT")) && Passed;

    MHit Reader;
    Passed = EvaluateTrue("Parse() V1", "return value", "Parse() returns true for a V1 StreamDat line",
                          Reader.Parse(Line, 1)) && Passed;

    Passed = EvaluateNear("Parse() V1", "X coordinate", "Parse() restores the representative X coordinate",
                          Reader.GetPosition().GetX(), X, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V1", "Y coordinate", "Parse() restores the representative Y coordinate",
                          Reader.GetPosition().GetY(), Y, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V1", "Z coordinate", "Parse() restores the representative Z coordinate",
                          Reader.GetPosition().GetZ(), Z, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V1", "energy", "Parse() restores the representative energy",
                          Reader.GetEnergy(), E, Tolerance) && Passed;
  }

  // --- Version 2: HT x y z energy + SH lines for each strip hit -------------
  {
    MStripHit SH;
    SH.SetDetectorID(0);
    SH.SetStripID(41);
    SH.IsLowVoltageStrip(true);
    SH.SetADCUnits(4053.0);
    SH.SetTAC(10452.0);

    MHit Writer;
    Writer.SetPosition(MVector(X, Y, Z));
    Writer.SetEnergy(E);
    Writer.AddStripHit(&SH);

    ostringstream Out;
    Passed = EvaluateTrue("StreamDat() V2", "return value", "StreamDat(V2) returns true",
                          Writer.StreamDat(Out, 2)) && Passed;

    MString Output(Out.str().c_str());
    Passed = EvaluateTrue("StreamDat() V2", "starts with HT", "StreamDat(V2) output starts with 'HT'",
                          Output.BeginsWith("HT")) && Passed;
    // V2 appends SH lines for each strip hit
    Passed = EvaluateTrue("StreamDat() V2", "contains SH line", "StreamDat(V2) output contains an SH sub-line for the strip hit",
                          Output.Contains("SH")) && Passed;

    // Verify line structure: exactly 2 lines (HT then SH)
    {
      istringstream SS(Out.str());
      string L0, L1, Extra;
      bool GotL0 = (bool) getline(SS, L0);
      bool GotL1 = (bool) getline(SS, L1);
      bool GotExtra = (bool) getline(SS, Extra);
      Passed = EvaluateTrue("StreamDat() V2", "line count", "StreamDat(V2) produces exactly 2 lines for one strip hit",
                            GotL0 && GotL1 && !GotExtra) && Passed;
      Passed = EvaluateTrue("StreamDat() V2", "SH line after HT", "StreamDat(V2) second line starts with 'SH'",
                            GotL1 && MString(L1.c_str()).BeginsWith("SH")) && Passed;
    }

    // Round-trip: extract the HT line and verify Parse() restores the fields
    string OutputStr2 = Out.str();
    size_t NL2 = OutputStr2.find('\n');
    MString HTLine2((NL2 != string::npos ? OutputStr2.substr(0, NL2) : OutputStr2).c_str());
    MHit Reader2;
    Passed = EvaluateTrue("Parse() V2", "return value", "Parse() returns true for the HT line from StreamDat(V2)",
                          Reader2.Parse(HTLine2, 2)) && Passed;
    Passed = EvaluateNear("Parse() V2", "X coordinate", "Parse() restores the representative X coordinate",
                          Reader2.GetPosition().GetX(), X, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V2", "Y coordinate", "Parse() restores the representative Y coordinate",
                          Reader2.GetPosition().GetY(), Y, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V2", "Z coordinate", "Parse() restores the representative Z coordinate",
                          Reader2.GetPosition().GetZ(), Z, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V2", "energy", "Parse() restores the representative energy",
                          Reader2.GetEnergy(), E, Tolerance) && Passed;
  }

  // --- Version 3: HT x y z energy lvEnergy hvEnergy + SH lines --------------
  {
    MStripHit SH;
    SH.SetDetectorID(0);
    SH.SetStripID(49);
    SH.IsLowVoltageStrip(false);
    SH.SetADCUnits(1780.0);

    MHit Writer;
    Writer.SetPosition(MVector(X, Y, Z));
    Writer.SetEnergy(E);
    Writer.SetLVEnergy(LVE);
    Writer.SetHVEnergy(HVE);
    Writer.AddStripHit(&SH);

    ostringstream Out;
    Passed = EvaluateTrue("StreamDat() V3", "return value", "StreamDat(V3) returns true",
                          Writer.StreamDat(Out, 3)) && Passed;

    MString Output(Out.str().c_str());
    Passed = EvaluateTrue("StreamDat() V3", "starts with HT", "StreamDat(V3) output starts with 'HT'",
                          Output.BeginsWith("HT")) && Passed;
    Passed = EvaluateTrue("StreamDat() V3", "contains LV energy", "StreamDat(V3) output contains the representative LV energy 250",
                          Output.Contains("250")) && Passed;
    Passed = EvaluateTrue("StreamDat() V3", "contains HV energy", "StreamDat(V3) output contains the representative HV energy 261",
                          Output.Contains("261")) && Passed;
    Passed = EvaluateTrue("StreamDat() V3", "contains SH line", "StreamDat(V3) output contains an SH sub-line for the strip hit",
                          Output.Contains("SH")) && Passed;

    // Verify line structure: exactly 2 lines (HT then SH)
    {
      istringstream SS(Out.str());
      string L0, L1, Extra;
      bool GotL0 = (bool) getline(SS, L0);
      bool GotL1 = (bool) getline(SS, L1);
      bool GotExtra = (bool) getline(SS, Extra);
      Passed = EvaluateTrue("StreamDat() V3", "line count", "StreamDat(V3) produces exactly 2 lines for one strip hit",
                            GotL0 && GotL1 && !GotExtra) && Passed;
      Passed = EvaluateTrue("StreamDat() V3", "SH line after HT", "StreamDat(V3) second line starts with 'SH'",
                            GotL1 && MString(L1.c_str()).BeginsWith("SH")) && Passed;
    }

    // Round-trip: extract the HT line and verify Parse() restores all six fields
    string OutputStr3 = Out.str();
    size_t NL3 = OutputStr3.find('\n');
    MString HTLine3((NL3 != string::npos ? OutputStr3.substr(0, NL3) : OutputStr3).c_str());
    MHit Reader3;
    Passed = EvaluateTrue("Parse() V3", "return value", "Parse() returns true for the HT line from StreamDat(V3)",
                          Reader3.Parse(HTLine3, 3)) && Passed;
    Passed = EvaluateNear("Parse() V3", "X coordinate", "Parse() restores the representative X coordinate",
                          Reader3.GetPosition().GetX(), X, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V3", "Y coordinate", "Parse() restores the representative Y coordinate",
                          Reader3.GetPosition().GetY(), Y, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V3", "Z coordinate", "Parse() restores the representative Z coordinate",
                          Reader3.GetPosition().GetZ(), Z, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V3", "energy", "Parse() restores the representative energy",
                          Reader3.GetEnergy(), E, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V3", "LV energy", "Parse() restores the representative LV energy",
                          Reader3.GetLVEnergy(), LVE, Tolerance) && Passed;
    Passed = EvaluateNear("Parse() V3", "HV energy", "Parse() restores the representative HV energy",
                          Reader3.GetHVEnergy(), HVE, Tolerance) && Passed;
  }

  // --- Parse() returns false for a non-HT line ------------------------------
  {
    MHit Dummy;
    MString NonHTLine("UH 0 41 l 4053 10452 4");
    Passed = EvaluateFalse("Parse()", "non-HT line", "Parse() returns false for a line that does not start with 'HT'",
                           Dummy.Parse(NonHTLine)) && Passed;
  }

  // --- Parse() returns false for an unsupported version number --------------
  {
    MHit Dummy;
    MString HTLine("HT 2.0 -3.0 5.0 511.0");
    Passed = EvaluateFalse("Parse()", "unsupported version 99", "Parse() returns false for an unsupported version number",
                           Dummy.Parse(HTLine, 99)) && Passed;
  }

  // --- Parse() V1 on a reused object resets stale LV/HV energy -------------
  {
    MHit H;
    H.SetLVEnergy(250.0);
    H.SetHVEnergy(261.0);
    MString HTLine("HT 2.0 -3.0 5.0 511.0");
    Passed = EvaluateTrue("Parse() V1 clears stale LV/HV", "return value", "Parse() V1 returns true on a valid HT line",
                          H.Parse(HTLine, 1)) && Passed;
    Passed = EvaluateTrue("Parse() V1 clears stale LV/HV", "LVEnergy reset", "Parse() V1 resets LVEnergy to g_DoubleNotDefined",
                          H.GetLVEnergy() == g_DoubleNotDefined) && Passed;
    Passed = EvaluateTrue("Parse() V1 clears stale LV/HV", "HVEnergy reset", "Parse() V1 resets HVEnergy to g_DoubleNotDefined",
                          H.GetHVEnergy() == g_DoubleNotDefined) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNHit::TestStreamEvta()
{
  bool Passed = true;

  // StreamEvta format:  HT 3;x;y;z;energy;posresX;posresY;posresZ;energyRes[;origin...]
  // Origins are the intersection of LV-strip origins and HV-strip origins.
  // If the intersection is empty but either side has origins, the union is used instead.

  // --- Case 1: LV origins {1,2} ∩ HV origins {2,3} = {2} -------------------
  // Coordinates chosen so that ";1" and ";3" do not appear as substrings of
  // any serialized field value, making the Contains() checks unambiguous.
  {
    MStripHit LV, HV;
    LV.IsLowVoltageStrip(true);
    LV.AddOrigins({1, 2});
    HV.IsLowVoltageStrip(false);
    HV.AddOrigins({2, 3});

    MHit H;
    H.SetPosition(MVector(5.0, 6.0, 7.0));
    H.SetEnergy(511.0);
    H.SetPositionResolution(MVector(0.5, 0.5, 0.5));
    H.SetEnergyResolution(4.0);
    H.AddStripHit(&LV);
    H.AddStripHit(&HV);

    ostringstream Out;
    H.StreamEvta(Out);
    MString S(Out.str().c_str());

    Passed = EvaluateTrue("StreamEvta()", "starts with HT 3", "StreamEvta() output starts with 'HT 3'",
                          S.BeginsWith("HT 3")) && Passed;
    // Origin 2 is in the intersection and must appear
    Passed = EvaluateTrue("StreamEvta()", "intersection origin 2", "StreamEvta() output contains the intersecting origin 2",
                          S.Contains(";2")) && Passed;
    // Origins 1 and 3 are not in the intersection and must not appear as origins
    // (They may appear as part of position/energy values, so search for ;1 and ;3 specifically)
    Passed = EvaluateTrue("StreamEvta()", "non-intersection origin 1 absent", "StreamEvta() output does not contain LV-only origin ;1",
                          S.Contains(";1") == false) && Passed;
    Passed = EvaluateTrue("StreamEvta()", "non-intersection origin 3 absent", "StreamEvta() output does not contain HV-only origin ;3",
                          S.Contains(";3") == false) && Passed;
  }

  // --- Case 2: LV origins {1} ∩ HV origins {2} = {} → fallback to union {1,2} ---
  {
    MStripHit LV, HV;
    LV.IsLowVoltageStrip(true);
    LV.AddOrigins({1});
    HV.IsLowVoltageStrip(false);
    HV.AddOrigins({2});

    MHit H;
    H.SetPosition(MVector(0.0, 0.0, 0.0));
    H.SetEnergy(300.0);
    H.SetPositionResolution(MVector(0.0, 0.0, 0.0));
    H.SetEnergyResolution(0.0);
    H.AddStripHit(&LV);
    H.AddStripHit(&HV);

    ostringstream Out;
    H.StreamEvta(Out);
    MString S(Out.str().c_str());

    // Both 1 and 2 must appear because the intersection is empty → union fallback
    Passed = EvaluateTrue("StreamEvta()", "union fallback origin 1", "StreamEvta() output contains union-fallback origin ;1 when LV/HV intersection is empty",
                          S.Contains(";1")) && Passed;
    Passed = EvaluateTrue("StreamEvta()", "union fallback origin 2", "StreamEvta() output contains union-fallback origin ;2 when LV/HV intersection is empty",
                          S.Contains(";2")) && Passed;
  }

  // --- Case 3: No strip hits → no origins appended --------------------------
  {
    MHit H;
    H.SetPosition(MVector(0.0, 0.0, 0.0));
    H.SetEnergy(0.0);
    H.SetPositionResolution(MVector(0.0, 0.0, 0.0));
    H.SetEnergyResolution(0.0);

    ostringstream Out;
    H.StreamEvta(Out);
    MString S(Out.str().c_str());

    // With no strip hits there are no origins: output ends with energyRes + newline
    // Count semicolons: HT 3;x;y;z;e;prx;pry;prz;er → exactly 8
    unsigned int Semicolons = 0;
    for (size_t i = 0; i < S.Length(); ++i) {
      if (S[i] == ';') ++Semicolons;
    }
    Passed = Evaluate("StreamEvta()", "semicolons with no origins", "StreamEvta() emits exactly 8 semicolons when there are no strip hits",
                      Semicolons, (unsigned int) 8) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNHit Test;
  return Test.Run() == true ? 0 : 1;
}
