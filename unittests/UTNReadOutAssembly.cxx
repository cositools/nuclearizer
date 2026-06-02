/*
 * UTNReadOutAssembly.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MTime.h"
#include "MSimEvent.h"
#include "MPhysicalEvent.h"

// Nuclearizer:
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MHit.h"
#include "MDEEStripHit.h"
#include "MDEECrystalHit.h"


//! Unit test class for MReadOutAssembly
class UTNReadOutAssembly : public MUnitTest
{
public:
  UTNReadOutAssembly() : MUnitTest("UTNReadOutAssembly") {}
  virtual ~UTNReadOutAssembly() {}

  virtual bool Run();

private:
  //! Test default construction and Clear() state
  bool TestDefaultConstruction();
  //! Test individual getter/setter pairs
  bool TestGettersSetters();
  //! Test analysis-progress OR accumulation
  bool TestAnalysisProgress();
  //! Test strip hit list management and InDetector tracking
  bool TestStripHitManagement();
  //! Test hit list management
  bool TestHitManagement();
  //! Test T-only, crystal, and guardring hit list management
  bool TestOtherHitCollections();
  //! Test error/quality flags and IsGood/IsBad/IsVeto
  bool TestEventFlags();
  //! Test Clear() resets collections and ownership
  bool TestClearOwnership();
  //! Test Parse() of HT/SH/BD lines and version forwarding
  bool TestParse();
  //! Test GetNextFromDatFile() with well-formed and malformed events
  bool TestGetNextFromDatFile();
  //! Test StreamDat() V1/V2/V3 output format
  bool TestStreamDat();
  //! Test StreamEvta() and StreamTra() output format
  bool TestStreamEvta();
  //! Test StreamRoa() output: UH lines, BD flags, PQ terminator
  bool TestStreamRoa();
  //! Test unique assembly IDs across instances and across Clear()
  bool TestAssemblyIDUniqueness();
};


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestAnalysisProgress() && Passed;
  Passed = TestStripHitManagement() && Passed;
  Passed = TestHitManagement() && Passed;
  Passed = TestOtherHitCollections() && Passed;
  Passed = TestEventFlags() && Passed;
  Passed = TestClearOwnership() && Passed;
  Passed = TestParse() && Passed;
  Passed = TestGetNextFromDatFile() && Passed;
  Passed = TestStreamDat() && Passed;
  Passed = TestStreamEvta() && Passed;
  Passed = TestStreamRoa() && Passed;
  Passed = TestAssemblyIDUniqueness() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestDefaultConstruction()
{
  bool Passed = true;

  MReadOutAssembly R;

  // ID is reset by Clear() in constructor
  Passed = EvaluateTrue("GetID()", "default after construction", "GetID() returns g_UnsignedIntNotDefined after construction",
                        R.GetID() == g_UnsignedIntNotDefined) && Passed;

  // Time fields
  Passed = EvaluateTrue("GetTimeRTS()", "default after construction", "GetTimeRTS() returns 0 after construction",
                        R.GetTimeRTS() == MTime(0)) && Passed;
  Passed = EvaluateTrue("GetTimeUTC()", "default after construction", "GetTimeUTC() returns 0 after construction",
                        R.GetTimeUTC() == MTime(0)) && Passed;

  // Veto / trigger flags after Clear()
  // Note: Clear() explicitly sets m_Trigger = true (not false)
  Passed = EvaluateTrue("GetTrigger()", "true after Clear", "GetTrigger() returns true after Clear()",
                        R.GetTrigger() == true) && Passed;
  Passed = EvaluateFalse("GetGuardRingVeto()", "default false", "GetGuardRingVeto() returns false after construction",
                         R.GetGuardRingVeto()) && Passed;
  Passed = EvaluateFalse("GetShieldVeto()", "default false", "GetShieldVeto() returns false after construction",
                         R.GetShieldVeto()) && Passed;

  // Error / quality flags
  Passed = EvaluateFalse("HasEnergyCalibrationError()", "default false", "HasEnergyCalibrationError() returns false after construction",
                         R.HasEnergyCalibrationError()) && Passed;
  Passed = EvaluateFalse("HasStripPairingError()", "default false", "HasStripPairingError() returns false after construction",
                         R.HasStripPairingError()) && Passed;
  Passed = EvaluateFalse("HasDepthCalibrationError()", "default false", "HasDepthCalibrationError() returns false after construction",
                         R.HasDepthCalibrationError()) && Passed;
  Passed = EvaluateFalse("HasEventReconstructionError()", "default false", "HasEventReconstructionError() returns false after construction",
                         R.HasEventReconstructionError()) && Passed;
  Passed = EvaluateFalse("HasStripHitBelowThreshold_QualityFlag()", "default false", "HasStripHitBelowThreshold_QualityFlag() returns false after construction",
                         R.HasStripHitBelowThreshold_QualityFlag()) && Passed;
  Passed = EvaluateFalse("HasStripPairing_QualityFlag()", "default false", "HasStripPairing_QualityFlag() returns false after construction",
                         R.HasStripPairing_QualityFlag()) && Passed;
  Passed = EvaluateFalse("IsFilteredOut()", "default false", "IsFilteredOut() returns false after construction",
                         R.IsFilteredOut()) && Passed;

  // IsGood / IsBad / IsVeto
  Passed = EvaluateTrue("IsGood()", "default true", "IsGood() returns true after construction",
                        R.IsGood() == true) && Passed;
  Passed = EvaluateFalse("IsBad()", "default false", "IsBad() returns false after construction",
                         R.IsBad()) && Passed;
  Passed = EvaluateFalse("IsVeto()", "default false", "IsVeto() returns false after construction",
                         R.IsVeto()) && Passed;

  // Collections empty
  Passed = Evaluate("GetNStripHits()", "empty after construction", "GetNStripHits() returns 0 after construction",
                    R.GetNStripHits(), (unsigned int) 0) && Passed;
  Passed = Evaluate("GetNHits()", "empty after construction", "GetNHits() returns 0 after construction",
                    R.GetNHits(), (unsigned int) 0) && Passed;
  Passed = Evaluate("GetNCrystalHits()", "empty after construction", "GetNCrystalHits() returns 0 after construction",
                    R.GetNCrystalHits(), (unsigned int) 0) && Passed;
  Passed = Evaluate("GetNGuardringHits()", "empty after construction", "GetNGuardringHits() returns 0 after construction",
                    R.GetNGuardringHits(), (unsigned int) 0) && Passed;

  // AssemblyID is set in the constructor (atomic counter) and is > 0
  Passed = EvaluateTrue("GetAssemblyID()", "greater than zero", "GetAssemblyID() returns a value > 0 after construction",
                        R.GetAssemblyID() > 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestGettersSetters()
{
  bool Passed = true;

  MReadOutAssembly R;

  // TimeRTS
  MTime RTS(12345678);
  R.SetTimeRTS(RTS);
  Passed = EvaluateTrue("SetTimeRTS/GetTimeRTS", "representative value 12345678", "GetTimeRTS() returns the time set by SetTimeRTS()",
                        R.GetTimeRTS() == RTS) && Passed;

  // TimeUTC
  MTime UTC(9876);
  R.SetTimeUTC(UTC);
  Passed = EvaluateTrue("SetTimeUTC/GetTimeUTC", "representative value 9876", "GetTimeUTC() returns the time set by SetTimeUTC()",
                        R.GetTimeUTC() == UTC) && Passed;

  // GuardRingVeto
  R.SetGuardRingVeto(true);
  Passed = EvaluateTrue("SetGuardRingVeto/GetGuardRingVeto", "representative true", "GetGuardRingVeto() returns true after SetGuardRingVeto(true)",
                        R.GetGuardRingVeto() == true) && Passed;
  R.SetGuardRingVeto(false);
  Passed = EvaluateFalse("SetGuardRingVeto/GetGuardRingVeto", "representative false", "GetGuardRingVeto() returns false after SetGuardRingVeto(false)",
                         R.GetGuardRingVeto()) && Passed;

  // ShieldVeto
  R.SetShieldVeto(true);
  Passed = EvaluateTrue("SetShieldVeto/GetShieldVeto", "representative true", "GetShieldVeto() returns true after SetShieldVeto(true)",
                        R.GetShieldVeto() == true) && Passed;
  R.SetShieldVeto(false);
  Passed = EvaluateFalse("SetShieldVeto/GetShieldVeto", "representative false", "GetShieldVeto() returns false after SetShieldVeto(false)",
                         R.GetShieldVeto()) && Passed;

  // Trigger
  R.SetTrigger(false);
  Passed = EvaluateFalse("SetTrigger/GetTrigger", "representative false", "GetTrigger() returns false after SetTrigger(false)",
                         R.GetTrigger()) && Passed;
  R.SetTrigger(true);
  Passed = EvaluateTrue("SetTrigger/GetTrigger", "representative true", "GetTrigger() returns true after SetTrigger(true)",
                        R.GetTrigger() == true) && Passed;

  // FilteredOut
  R.SetFilteredOut(true);
  Passed = EvaluateTrue("SetFilteredOut/IsFilteredOut", "representative true", "IsFilteredOut() returns true after SetFilteredOut(true)",
                        R.IsFilteredOut() == true) && Passed;
  R.SetFilteredOut(false);
  Passed = EvaluateFalse("SetFilteredOut/IsFilteredOut", "representative false", "IsFilteredOut() returns false after SetFilteredOut(false)",
                         R.IsFilteredOut()) && Passed;

  // Sim aspect info
  R.SetSimAspectInfo(true);
  Passed = EvaluateTrue("SetSimAspectInfo/HasSimAspectInfo", "representative true", "HasSimAspectInfo() returns true after SetSimAspectInfo(true)",
                        R.HasSimAspectInfo() == true) && Passed;

  // Galactic pointing getters return values only when HasSimAspectInfo == true
  R.SetGalacticPointingXAxisTheta(1.1);
  R.SetGalacticPointingXAxisPhi(2.2);
  R.SetGalacticPointingZAxisTheta(3.3);
  R.SetGalacticPointingZAxisPhi(4.4);
  Passed = EvaluateNear("SetGalacticPointingXAxisTheta/GetGalacticPointingXAxisTheta", "representative value 1.1",
                        "GetGalacticPointingXAxisTheta() returns the representative value 1.1",
                        R.GetGalacticPointingXAxisTheta(), 1.1, 1e-10) && Passed;
  Passed = EvaluateNear("SetGalacticPointingXAxisPhi/GetGalacticPointingXAxisPhi", "representative value 2.2",
                        "GetGalacticPointingXAxisPhi() returns the representative value 2.2",
                        R.GetGalacticPointingXAxisPhi(), 2.2, 1e-10) && Passed;
  Passed = EvaluateNear("SetGalacticPointingZAxisTheta/GetGalacticPointingZAxisTheta", "representative value 3.3",
                        "GetGalacticPointingZAxisTheta() returns the representative value 3.3",
                        R.GetGalacticPointingZAxisTheta(), 3.3, 1e-10) && Passed;
  Passed = EvaluateNear("SetGalacticPointingZAxisPhi/GetGalacticPointingZAxisPhi", "representative value 4.4",
                        "GetGalacticPointingZAxisPhi() returns the representative value 4.4",
                        R.GetGalacticPointingZAxisPhi(), 4.4, 1e-10) && Passed;

  // When HasSimAspectInfo == false the getters return 0
  R.SetSimAspectInfo(false);
  Passed = EvaluateNear("GetGalacticPointingXAxisTheta()", "no aspect info", "GetGalacticPointingXAxisTheta() returns 0 when HasSimAspectInfo is false",
                        R.GetGalacticPointingXAxisTheta(), 0.0, 1e-10) && Passed;

  // SetSimulatedEvent takes ownership; a repeated call replaces (and frees) the previous event
  MSimEvent* Sim1 = new MSimEvent();
  R.SetSimulatedEvent(Sim1);
  Passed = EvaluateTrue("SetSimulatedEvent/GetSimulatedEvent", "first event", "GetSimulatedEvent() returns the event passed to SetSimulatedEvent()",
                        R.GetSimulatedEvent() == Sim1) && Passed;
  MSimEvent* Sim2 = new MSimEvent();
  R.SetSimulatedEvent(Sim2);
  Passed = EvaluateTrue("SetSimulatedEvent/GetSimulatedEvent", "replaced event", "A repeated SetSimulatedEvent() replaces the previous event with the new one",
                        R.GetSimulatedEvent() == Sim2) && Passed;

  // Self-assignment must not free the event and leave a dangling pointer
  R.SetSimulatedEvent(R.GetSimulatedEvent());
  Passed = EvaluateTrue("SetSimulatedEvent/GetSimulatedEvent", "self-assignment", "SetSimulatedEvent(GetSimulatedEvent()) leaves the event pointer unchanged",
                        R.GetSimulatedEvent() == Sim2) && Passed;

  // RTS/UTC conversion helpers
  MTime ReferenceUTC(2025, 1, 1, 0, 0, 0, 0);
  MTime ReferenceRTS = R.ComputeRTSfromUTCTime(ReferenceUTC);
  Passed = EvaluateNear("ComputeRTSfromUTCTime()", "UTC epoch", "The COSI RTS starts 69.184 seconds ahead of UTC on 2025-01-01",
                        ReferenceRTS.GetAsDouble(), 69.184, 1e-6) && Passed;
  MTime RoundTripUTC = R.ComputeUTCfromRTSTime(ReferenceRTS);
  Passed = EvaluateTrue("ComputeUTCfromRTSTime()", "round trip", "Converting UTC -> RTS -> UTC restores the representative UTC time",
                        RoundTripUTC == ReferenceUTC) && Passed;

  // StripPairingReducedChiSquare accumulates appended values into a vector
  R.SetStripPairingReducedChiSquare(1.5);
  R.SetStripPairingReducedChiSquare(2.5);
  vector<double> ChiSquares = R.GetStripPairingReducedChiSquare();
  Passed = Evaluate("SetStripPairingReducedChiSquare/GetStripPairingReducedChiSquare", "count after two adds", "GetStripPairingReducedChiSquare() returns both appended values",
                    (unsigned int) ChiSquares.size(), (unsigned int) 2) && Passed;
  if (ChiSquares.size() == 2) {
    Passed = EvaluateNear("SetStripPairingReducedChiSquare", "first value 1.5", "The first appended reduced chi-square is 1.5",
                          ChiSquares[0], 1.5, 1e-9) && Passed;
    Passed = EvaluateNear("SetStripPairingReducedChiSquare", "second value 2.5", "The second appended reduced chi-square is 2.5",
                          ChiSquares[1], 2.5, 1e-9) && Passed;
  }

  // SetPhysicalEvent stores a duplicate; the ROA owns it
  MPhysicalEvent* PE = new MPhysicalEvent();
  R.SetPhysicalEvent(PE);
  Passed = EvaluateTrue("SetPhysicalEvent/GetPhysicalEvent", "stores a duplicate", "GetPhysicalEvent() returns a non-null event after SetPhysicalEvent()",
                        R.GetPhysicalEvent() != nullptr) && Passed;
  delete PE;   // SetPhysicalEvent duplicated the event, so the caller still owns PE

  // Self-assignment must not free the stored event
  MPhysicalEvent* Stored = R.GetPhysicalEvent();
  R.SetPhysicalEvent(Stored);
  Passed = EvaluateTrue("SetPhysicalEvent/GetPhysicalEvent", "self-assignment", "SetPhysicalEvent(GetPhysicalEvent()) leaves the stored event unchanged",
                        R.GetPhysicalEvent() == Stored) && Passed;

  // Passing nullptr clears the stored event
  R.SetPhysicalEvent(nullptr);
  Passed = EvaluateTrue("SetPhysicalEvent/GetPhysicalEvent", "nullptr clears", "GetPhysicalEvent() returns nullptr after SetPhysicalEvent(nullptr)",
                        R.GetPhysicalEvent() == nullptr) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestAnalysisProgress()
{
  bool Passed = true;

  MReadOutAssembly R;

  // Clear() (called by the constructor) resets m_AnalysisProgress to 0
  Passed = EvaluateTrue("GetAnalysisProgress()", "zero after construction", "GetAnalysisProgress() returns 0 after construction",
                        R.GetAnalysisProgress() == 0) && Passed;

  R.SetAnalysisProgress(0x01ULL);
  Passed = EvaluateTrue("HasAnalysisProgress()", "bit 0x01 set", "HasAnalysisProgress(0x01) returns true after SetAnalysisProgress(0x01)",
                        R.HasAnalysisProgress(0x01ULL) == true) && Passed;
  Passed = EvaluateTrue("GetAnalysisProgress()", "bit 0x01 set", "GetAnalysisProgress() has bit 0x01 set after SetAnalysisProgress(0x01)",
                        (R.GetAnalysisProgress() & 0x01ULL) != 0) && Passed;

  // OR accumulation: setting a second bit must not clear the first
  R.SetAnalysisProgress(0x02ULL);
  Passed = EvaluateTrue("HasAnalysisProgress()", "bit 0x01 still set after OR", "HasAnalysisProgress(0x01) still returns true after SetAnalysisProgress(0x02)",
                        R.HasAnalysisProgress(0x01ULL) == true) && Passed;
  Passed = EvaluateTrue("HasAnalysisProgress()", "bit 0x02 set", "HasAnalysisProgress(0x02) returns true after SetAnalysisProgress(0x02)",
                        R.HasAnalysisProgress(0x02ULL) == true) && Passed;
  Passed = EvaluateTrue("HasAnalysisProgress()", "both bits 0x03", "HasAnalysisProgress(0x03) returns true when both bits are set",
                        R.HasAnalysisProgress(0x03ULL) == true) && Passed;

  // A bit that was never set must return false
  Passed = EvaluateFalse("HasAnalysisProgress()", "bit 0x04 not set", "HasAnalysisProgress(0x04) returns false when bit was never set",
                         R.HasAnalysisProgress(0x04ULL)) && Passed;

  // HasAnalysisProgress returns false when not all requested bits are present
  Passed = EvaluateFalse("HasAnalysisProgress()", "partial match 0x07", "HasAnalysisProgress(0x07) returns false when only 0x03 is set",
                         R.HasAnalysisProgress(0x07ULL)) && Passed;

  // Clear() resets the accumulated progress back to 0
  R.Clear();
  Passed = EvaluateTrue("GetAnalysisProgress()", "zero after Clear", "GetAnalysisProgress() returns 0 after Clear()",
                        R.GetAnalysisProgress() == 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestStripHitManagement()
{
  bool Passed = true;

  MReadOutAssembly R;

  // Add two strip hits with different detector IDs
  MStripHit* SH0 = new MStripHit();
  SH0->SetDetectorID(3);
  SH0->SetStripID(10);
  R.AddStripHit(SH0);

  MStripHit* SH1 = new MStripHit();
  SH1->SetDetectorID(14);
  SH1->SetStripID(20);
  R.AddStripHit(SH1);

  Passed = Evaluate("GetNStripHits()", "count after two adds", "GetNStripHits() returns 2 after adding two strip hits",
                    R.GetNStripHits(), (unsigned int) 2) && Passed;

  // Pointer identity
  Passed = EvaluateTrue("GetStripHit()", "pointer identity index 0", "GetStripHit(0) returns the pointer passed in the first AddStripHit()",
                        R.GetStripHit(0) == SH0) && Passed;
  Passed = EvaluateTrue("GetStripHit()", "pointer identity index 1", "GetStripHit(1) returns the pointer passed in the second AddStripHit()",
                        R.GetStripHit(1) == SH1) && Passed;

  // InDetector tracking: AddStripHit updates m_InDetector for IDs 0-15
  Passed = EvaluateTrue("InDetector()", "detector 3 hit", "InDetector(3) returns true after adding a strip hit with DetectorID 3",
                        R.InDetector(3) == true) && Passed;
  Passed = EvaluateTrue("InDetector()", "detector 14 hit", "InDetector(14) returns true after adding a strip hit with DetectorID 14",
                        R.InDetector(14) == true) && Passed;
  Passed = EvaluateFalse("InDetector()", "detector 4 not hit", "InDetector(4) returns false when no strip hit has DetectorID 4",
                         R.InDetector(4)) && Passed;

  // Out-of-range InDetector returns false
  Passed = EvaluateFalse("InDetector()", "out of range 16", "InDetector(16) returns false for an out-of-range detector ID",
                         R.InDetector(16)) && Passed;
  Passed = EvaluateFalse("InDetector()", "out of range -1", "InDetector(-1) returns false for a negative detector ID",
                         R.InDetector(-1)) && Passed;

  // Out-of-bounds access emits merr and returns nullptr
  DisableDefaultStreams();
  Passed = EvaluateTrue("GetStripHit()", "out of bounds returns nullptr", "GetStripHit(99) returns nullptr when index is out of bounds",
                        R.GetStripHit(99) == nullptr) && Passed;
  EnableDefaultStreams();

  // RemoveStripHit removes by index and shifts remaining elements
  R.RemoveStripHit(0);
  Passed = Evaluate("RemoveStripHit()", "count after remove", "GetNStripHits() returns 1 after removing index 0",
                    R.GetNStripHits(), (unsigned int) 1) && Passed;
  Passed = EvaluateTrue("RemoveStripHit()", "remaining pointer after remove", "After removing index 0, GetStripHit(0) returns the second strip hit",
                        R.GetStripHit(0) == SH1) && Passed;

  // RemoveStripHit must clear the per-detector flag when the last hit in a detector is removed
  Passed = EvaluateFalse("RemoveStripHit()", "InDetector cleared", "InDetector(3) returns false after removing the only strip hit in detector 3",
                         R.InDetector(3)) && Passed;
  Passed = EvaluateTrue("RemoveStripHit()", "InDetector retained", "InDetector(14) still returns true since detector 14 still has a strip hit",
                        R.InDetector(14) == true) && Passed;

  // RemoveStripHit out-of-range is a no-op
  R.RemoveStripHit(99);
  Passed = Evaluate("RemoveStripHit()", "no-op for out-of-range index", "Removing an out-of-range index leaves the count unchanged",
                    R.GetNStripHits(), (unsigned int) 1) && Passed;

  // RemoveStripHit erased SH0 from the vector but did not free it, so delete it manually
  delete SH0;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestHitManagement()
{
  bool Passed = true;

  MReadOutAssembly R;

  MHit* H0 = new MHit();
  MHit* H1 = new MHit();
  R.AddHit(H0);
  R.AddHit(H1);

  Passed = Evaluate("GetNHits()", "count after two adds", "GetNHits() returns 2 after adding two hits",
                    R.GetNHits(), (unsigned int) 2) && Passed;

  // Pointer identity
  Passed = EvaluateTrue("GetHit()", "pointer identity index 0", "GetHit(0) returns the pointer passed in the first AddHit()",
                        R.GetHit(0) == H0) && Passed;
  Passed = EvaluateTrue("GetHit()", "pointer identity index 1", "GetHit(1) returns the pointer passed in the second AddHit()",
                        R.GetHit(1) == H1) && Passed;

  // Out-of-bounds returns nullptr
  DisableDefaultStreams();
  Passed = EvaluateTrue("GetHit()", "out of bounds returns nullptr", "GetHit(99) returns nullptr when index is out of bounds",
                        R.GetHit(99) == nullptr) && Passed;
  EnableDefaultStreams();

  // RemoveHit shifts remaining elements
  R.RemoveHit(0);
  Passed = Evaluate("RemoveHit()", "count after remove", "GetNHits() returns 1 after removing index 0",
                    R.GetNHits(), (unsigned int) 1) && Passed;
  Passed = EvaluateTrue("RemoveHit()", "remaining pointer after remove", "After removing index 0, GetHit(0) returns the second hit",
                        R.GetHit(0) == H1) && Passed;

  // RemoveHit does not delete: H0 was removed from the vector but not freed, so delete manually
  delete H0;

  // DeleteHits() frees and clears the remaining hits
  R.DeleteHits();
  Passed = Evaluate("DeleteHits()", "count after DeleteHits", "DeleteHits() empties the hit list",
                    R.GetNHits(), (unsigned int) 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestOtherHitCollections()
{
  bool Passed = true;

  MReadOutAssembly R;

  // --- T-only strip hits ---------------------------------------------------
  MStripHit* TOnly0 = new MStripHit();
  MStripHit* TOnly1 = new MStripHit();
  R.AddStripHitTOnly(TOnly0);
  R.AddStripHitTOnly(TOnly1);
  Passed = Evaluate("GetNStripHitsTOnly()", "count after two adds", "GetNStripHitsTOnly() returns 2 after adding two T-only strip hits",
                    R.GetNStripHitsTOnly(), (unsigned int) 2) && Passed;
  Passed = EvaluateTrue("GetStripHitTOnly()", "pointer identity index 0", "GetStripHitTOnly(0) returns the first T-only strip hit pointer",
                        R.GetStripHitTOnly(0) == TOnly0) && Passed;
  Passed = EvaluateTrue("GetStripHitTOnly()", "pointer identity index 1", "GetStripHitTOnly(1) returns the second T-only strip hit pointer",
                        R.GetStripHitTOnly(1) == TOnly1) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateTrue("GetStripHitTOnly()", "out of bounds returns nullptr", "GetStripHitTOnly(99) returns nullptr when index is out of bounds",
                        R.GetStripHitTOnly(99) == nullptr) && Passed;
  EnableDefaultStreams();
  R.RemoveStripHitTOnly(0);
  Passed = Evaluate("RemoveStripHitTOnly()", "count after remove", "GetNStripHitsTOnly() returns 1 after removing index 0",
                    R.GetNStripHitsTOnly(), (unsigned int) 1) && Passed;
  Passed = EvaluateTrue("RemoveStripHitTOnly()", "remaining pointer", "After removing index 0, GetStripHitTOnly(0) returns the second T-only strip hit",
                        R.GetStripHitTOnly(0) == TOnly1) && Passed;
  // RemoveStripHitTOnly erased TOnly0 from the vector without freeing it
  delete TOnly0;

  // --- Crystal hits --------------------------------------------------------
  MCrystalHit* CH0 = new MCrystalHit();
  MCrystalHit* CH1 = new MCrystalHit();
  R.AddCrystalHit(CH0);
  R.AddCrystalHit(CH1);
  Passed = Evaluate("GetNCrystalHits()", "count after two adds", "GetNCrystalHits() returns 2 after adding two crystal hits",
                    R.GetNCrystalHits(), (unsigned int) 2) && Passed;
  Passed = EvaluateTrue("GetCrystalHit()", "pointer identity index 0", "GetCrystalHit(0) returns the first crystal hit pointer",
                        R.GetCrystalHit(0) == CH0) && Passed;
  Passed = EvaluateTrue("GetCrystalHit()", "pointer identity index 1", "GetCrystalHit(1) returns the second crystal hit pointer",
                        R.GetCrystalHit(1) == CH1) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateTrue("GetCrystalHit()", "out of bounds returns nullptr", "GetCrystalHit(99) returns nullptr when index is out of bounds",
                        R.GetCrystalHit(99) == nullptr) && Passed;
  EnableDefaultStreams();
  R.RemoveCrystalHit(0);
  Passed = Evaluate("RemoveCrystalHit()", "count after remove", "GetNCrystalHits() returns 1 after removing index 0",
                    R.GetNCrystalHits(), (unsigned int) 1) && Passed;
  Passed = EvaluateTrue("RemoveCrystalHit()", "remaining pointer", "After removing index 0, GetCrystalHit(0) returns the second crystal hit",
                        R.GetCrystalHit(0) == CH1) && Passed;
  // RemoveCrystalHit erased CH0 from the vector without freeing it
  delete CH0;

  // --- Guardring hits ------------------------------------------------------
  MGuardringHit* GH0 = new MGuardringHit();
  R.AddGuardringHit(GH0);
  Passed = Evaluate("GetNGuardringHits()", "count after one add", "GetNGuardringHits() returns 1 after adding one guardring hit",
                    R.GetNGuardringHits(), (unsigned int) 1) && Passed;
  Passed = EvaluateTrue("GetGuardringHit()", "pointer identity index 0", "GetGuardringHit(0) returns the guardring hit pointer",
                        R.GetGuardringHit(0) == GH0) && Passed;
  DisableDefaultStreams();
  Passed = EvaluateTrue("GetGuardringHit()", "out of bounds returns nullptr", "GetGuardringHit(99) returns nullptr when index is out of bounds",
                        R.GetGuardringHit(99) == nullptr) && Passed;
  EnableDefaultStreams();

  // --- DEE strip hits (LV/HV) and DEE crystal hits -------------------------
  MDEEStripHit DEELV;
  R.AddDEEStripHitLV(DEELV);
  R.AddDEEStripHitLV(DEELV);
  Passed = Evaluate("AddDEEStripHitLV/GetNDEEStripHitsLV", "count after two adds", "GetNDEEStripHitsLV() returns 2 after adding two LV DEE strip hits",
                    R.GetNDEEStripHitsLV(), (unsigned int) 2) && Passed;

  MDEEStripHit DEEHV;
  R.AddDEEStripHitHV(DEEHV);
  Passed = Evaluate("AddDEEStripHitHV/GetNDEEStripHitsHV", "count after one add", "GetNDEEStripHitsHV() returns 1 after adding one HV DEE strip hit",
                    R.GetNDEEStripHitsHV(), (unsigned int) 1) && Passed;

  MDEECrystalHit DEECrystal;
  R.AddDEECrystalHit(DEECrystal);
  Passed = Evaluate("AddDEECrystalHit/GetNDEECrystalHits", "count after one add", "GetNDEECrystalHits() returns 1 after adding one DEE crystal hit",
                    R.GetNDEECrystalHits(), (unsigned int) 1) && Passed;

  // The list-reference accessor exposes the same underlying LV list
  Passed = Evaluate("GetDEEStripHitLVListReference()", "size matches", "GetDEEStripHitLVListReference() exposes the 2 LV DEE strip hits",
                    (unsigned int) R.GetDEEStripHitLVListReference().size(), (unsigned int) 2) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestEventFlags()
{
  bool Passed = true;

  MReadOutAssembly R;

  // Fresh state: good, not bad, not veto
  Passed = EvaluateTrue("IsGood()", "initial state", "IsGood() returns true on a freshly constructed ROA",
                        R.IsGood() == true) && Passed;
  Passed = EvaluateFalse("IsBad()", "initial state", "IsBad() returns false on a freshly constructed ROA",
                         R.IsBad()) && Passed;
  Passed = EvaluateFalse("IsVeto()", "initial state", "IsVeto() returns false on a freshly constructed ROA",
                         R.IsVeto()) && Passed;

  // EnergyCalibrationError
  R.SetEnergyCalibrationError();
  Passed = EvaluateTrue("HasEnergyCalibrationError()", "after set", "HasEnergyCalibrationError() returns true after SetEnergyCalibrationError()",
                        R.HasEnergyCalibrationError() == true) && Passed;
  Passed = EvaluateFalse("IsGood()", "after EnergyCalibrationError", "IsGood() returns false when EnergyCalibrationError is set",
                         R.IsGood()) && Passed;
  Passed = EvaluateTrue("IsBad()", "after EnergyCalibrationError", "IsBad() returns true when EnergyCalibrationError is set",
                        R.IsBad() == true) && Passed;
  Passed = EvaluateFalse("IsVeto()", "EnergyCalibrationError is not a veto", "IsVeto() returns false when only EnergyCalibrationError is set",
                         R.IsVeto()) && Passed;
  R.Clear();

  // StripPairingError
  R.SetStripPairingError();
  Passed = EvaluateTrue("HasStripPairingError()", "after set", "HasStripPairingError() returns true after SetStripPairingError()",
                        R.HasStripPairingError() == true) && Passed;
  Passed = EvaluateFalse("IsGood()", "after StripPairingError", "IsGood() returns false when StripPairingError is set",
                         R.IsGood()) && Passed;
  Passed = EvaluateTrue("IsBad()", "after StripPairingError", "IsBad() returns true when StripPairingError is set",
                        R.IsBad() == true) && Passed;
  R.Clear();

  // DepthCalibrationError
  R.SetDepthCalibrationError();
  Passed = EvaluateTrue("HasDepthCalibrationError()", "after set", "HasDepthCalibrationError() returns true after SetDepthCalibrationError()",
                        R.HasDepthCalibrationError() == true) && Passed;
  Passed = EvaluateFalse("IsGood()", "after DepthCalibrationError", "IsGood() returns false when DepthCalibrationError is set",
                         R.IsGood()) && Passed;
  Passed = EvaluateTrue("IsBad()", "after DepthCalibrationError", "IsBad() returns true when DepthCalibrationError is set",
                        R.IsBad() == true) && Passed;
  R.Clear();

  // EventReconstructionError
  R.SetEventReconstructionError();
  Passed = EvaluateTrue("HasEventReconstructionError()", "after set", "HasEventReconstructionError() returns true after SetEventReconstructionError()",
                        R.HasEventReconstructionError() == true) && Passed;
  Passed = EvaluateFalse("IsGood()", "after EventReconstructionError", "IsGood() returns false when EventReconstructionError is set",
                         R.IsGood()) && Passed;
  Passed = EvaluateTrue("IsBad()", "after EventReconstructionError", "IsBad() returns true when EventReconstructionError is set",
                        R.IsBad() == true) && Passed;
  R.Clear();

  // FilteredOut
  R.SetFilteredOut(true);
  Passed = EvaluateTrue("IsFilteredOut()", "after set", "IsFilteredOut() returns true after SetFilteredOut(true)",
                        R.IsFilteredOut() == true) && Passed;
  Passed = EvaluateFalse("IsGood()", "when FilteredOut", "IsGood() returns false when FilteredOut is set",
                         R.IsGood()) && Passed;
  Passed = EvaluateTrue("IsBad()", "when FilteredOut", "IsBad() returns true when FilteredOut is set",
                        R.IsBad() == true) && Passed;
  R.Clear();

  // Veto flags are separate from IsGood/IsBad
  R.SetGuardRingVeto(true);
  Passed = EvaluateTrue("IsVeto()", "after GuardRingVeto", "IsVeto() returns true after SetGuardRingVeto(true)",
                        R.IsVeto() == true) && Passed;
  Passed = EvaluateTrue("IsGood()", "GuardRingVeto does not affect IsGood", "IsGood() returns true when only GuardRingVeto is set",
                        R.IsGood() == true) && Passed;
  R.Clear();

  R.SetShieldVeto(true);
  Passed = EvaluateTrue("IsVeto()", "after ShieldVeto", "IsVeto() returns true after SetShieldVeto(true)",
                        R.IsVeto() == true) && Passed;
  Passed = EvaluateTrue("IsGood()", "ShieldVeto does not affect IsGood", "IsGood() returns true when only ShieldVeto is set",
                        R.IsGood() == true) && Passed;
  R.Clear();

  // Quality flags do not affect IsGood
  R.SetStripHitBelowThreshold_QualityFlag("below_thresh");
  Passed = EvaluateTrue("HasStripHitBelowThreshold_QualityFlag()", "after set", "HasStripHitBelowThreshold_QualityFlag() returns true after setting it",
                        R.HasStripHitBelowThreshold_QualityFlag() == true) && Passed;
  Passed = EvaluateTrue("IsGood()", "quality flag does not affect IsGood", "IsGood() returns true when only a quality flag is set",
                        R.IsGood() == true) && Passed;
  R.Clear();

  R.SetStripPairing_QualityFlag();
  Passed = EvaluateTrue("HasStripPairing_QualityFlag()", "after set", "HasStripPairing_QualityFlag() returns true after setting it",
                        R.HasStripPairing_QualityFlag() == true) && Passed;
  Passed = EvaluateTrue("IsGood()", "StripPairing quality flag does not affect IsGood", "IsGood() returns true when only StripPairing quality flag is set",
                        R.IsGood() == true) && Passed;
  R.Clear();

  // All flags cleared after Clear()
  Passed = EvaluateFalse("HasEnergyCalibrationError()", "cleared by Clear", "HasEnergyCalibrationError() returns false after Clear()",
                         R.HasEnergyCalibrationError()) && Passed;
  Passed = EvaluateTrue("IsGood()", "restored by Clear", "IsGood() returns true after Clear()",
                        R.IsGood() == true) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestClearOwnership()
{
  bool Passed = true;

  MReadOutAssembly R;

  // Add strip hits and MHit objects that are owned by the ROA
  MStripHit* SH0 = new MStripHit();
  MStripHit* SH1 = new MStripHit();
  R.AddStripHit(SH0);
  R.AddStripHit(SH1);

  MHit* H0 = new MHit();
  R.AddHit(H0);

  Passed = Evaluate("GetNStripHits()", "two strip hits before Clear", "GetNStripHits() returns 2 before Clear()",
                    R.GetNStripHits(), (unsigned int) 2) && Passed;
  Passed = Evaluate("GetNHits()", "one hit before Clear", "GetNHits() returns 1 before Clear()",
                    R.GetNHits(), (unsigned int) 1) && Passed;

  // Clear() must delete and reset all owned collections
  R.Clear();

  Passed = Evaluate("GetNStripHits()", "empty after Clear", "GetNStripHits() returns 0 after Clear()",
                    R.GetNStripHits(), (unsigned int) 0) && Passed;
  Passed = Evaluate("GetNHits()", "empty after Clear", "GetNHits() returns 0 after Clear()",
                    R.GetNHits(), (unsigned int) 0) && Passed;

  // AssemblyID must survive Clear() — it is assigned once in the constructor
  unsigned long ID = R.GetAssemblyID();
  R.Clear();
  Passed = EvaluateTrue("GetAssemblyID()", "unchanged by Clear", "GetAssemblyID() returns the same value after Clear()",
                        R.GetAssemblyID() == ID) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestParse()
{
  bool Passed = true;

  // --- HT line creates a hit -----------------------------------------------
  {
    MReadOutAssembly R;
    MString HTLine("HT 1.0 2.0 3.0 511.0");
    Passed = EvaluateTrue("Parse()", "HT line", "Parse() returns true for an HT line",
                          R.Parse(HTLine)) && Passed;
    Passed = Evaluate("Parse()", "hit count after HT", "Parse() of an HT line adds exactly one hit",
                      R.GetNHits(), (unsigned int) 1) && Passed;
  }

  // --- TI line sets the read-out assembly UTC event time -------------------
  {
    MReadOutAssembly R;
    MString TILine("TI 123456789.123456789");
    Passed = EvaluateTrue("Parse()", "TI line", "Parse() returns true for a TI line",
                          R.Parse(TILine)) && Passed;
    Passed = EvaluateTrue("Parse()", "TI sets UTC event time", "Parse() of a TI line updates GetTimeUTC()",
                          R.GetTimeUTC() == MTime(123456789, 123456789)) && Passed;
  }

  // --- SH line attaches a strip hit to the last hit and is owned by the ROA -
  {
    MReadOutAssembly R;
    MString HTLine("HT 1.0 2.0 3.0 511.0");
    R.Parse(HTLine);
    MString SHLine("SH 0 l 41 1 0 100 100 50 1 0");
    Passed = EvaluateTrue("Parse()", "SH after HT", "Parse() returns true for an SH line following an HT line",
                          R.Parse(SHLine)) && Passed;
    Passed = Evaluate("Parse()", "strip hit owned by ROA", "Parse() of an SH line adds the strip hit to the ROA so it owns it",
                      R.GetNStripHits(), (unsigned int) 1) && Passed;
    if (R.GetNHits() == 1) {
      Passed = Evaluate("Parse()", "strip hit attached to hit", "Parse() of an SH line attaches the strip hit to the last hit",
                        R.GetHit(0)->GetNStripHits(), (unsigned int) 1) && Passed;
    }
  }

  // --- SH line before any HT line is rejected (was undefined behavior) -----
  {
    MReadOutAssembly R;
    MString SHLine("SH 0 l 41 1 0 100 100 50 1 0");
    Passed = EvaluateFalse("Parse()", "SH before any HT", "Parse() returns false for an SH line when no hit has been parsed yet",
                           R.Parse(SHLine)) && Passed;
  }

  // --- BD line sets the filtered-out flag ----------------------------------
  {
    MReadOutAssembly R;
    MString BDLine("BD some reason");
    Passed = EvaluateTrue("Parse()", "BD line", "Parse() returns true for a BD line",
                          R.Parse(BDLine)) && Passed;
    Passed = EvaluateTrue("Parse()", "BD sets FilteredOut", "Parse() of a BD line sets the filtered-out flag",
                          R.IsFilteredOut()) && Passed;
  }

  // --- An unrecognized line is tolerantly consumed -------------------------
  {
    MReadOutAssembly R;
    MString BadLine("XY 1 2 3");
    Passed = EvaluateTrue("Parse()", "unrecognized line", "Parse() returns true for an unrecognized line, which is tolerantly consumed",
                          R.Parse(BadLine)) && Passed;
  }

  // --- Version is forwarded to MHit::Parse: V3 restores LV/HV energy -------
  {
    MReadOutAssembly R;
    MString HTV3("HT 1.0 2.0 3.0 511.0 250.0 261.0");
    bool ParsedV3 = R.Parse(HTV3, 3);
    Passed = EvaluateTrue("Parse() V3", "HT V3 line", "Parse(V3) returns true for a six-field HT line",
                          ParsedV3) && Passed;
    if (ParsedV3 == true && R.GetNHits() == 1) {
      Passed = EvaluateNear("Parse() V3", "LV energy forwarded", "Parse(V3) forwards the version so the hit's LV energy is restored",
                            R.GetHit(0)->GetLVEnergy(), 250.0, 1e-4) && Passed;
      Passed = EvaluateNear("Parse() V3", "HV energy forwarded", "Parse(V3) forwards the version so the hit's HV energy is restored",
                            R.GetHit(0)->GetHVEnergy(), 261.0, 1e-4) && Passed;
    }
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestGetNextFromDatFile()
{
  bool Passed = true;

  // --- A well-formed event is read correctly -------------------------------
  {
    MString DatFile("/tmp/UTNReadOutAssembly_valid.dat");
    ofstream Out(DatFile.Data());
    Passed = EvaluateTrue("GetNextFromDatFile()", "write valid .dat file", "The valid .dat fixture file can be created",
                          Out.is_open()) && Passed;
    Out << "SE" << endl;
    Out << "ID 1" << endl;
    Out << "HT 1.0 2.0 3.0 511.0" << endl;
    Out << "SH 0 l 41 1 0 100 100 50 1 0" << endl;
    Out << "SE" << endl;
    Out.close();

    MReadOutAssembly R;
    MFile F;
    bool Opened = F.Open(DatFile);
    Passed = EvaluateTrue("GetNextFromDatFile()", "open valid .dat file", "The valid .dat test file can be opened",
                          Opened) && Passed;
    if (Opened == true) {
      bool Result = R.GetNextFromDatFile(F);
      F.Close();
      Passed = EvaluateTrue("GetNextFromDatFile()", "well-formed event", "GetNextFromDatFile() returns true for a well-formed event",
                            Result) && Passed;
      Passed = Evaluate("GetNextFromDatFile()", "hit count", "GetNextFromDatFile() reads one hit from a well-formed event",
                        R.GetNHits(), (unsigned int) 1) && Passed;
      Passed = Evaluate("GetNextFromDatFile()", "strip hit count", "GetNextFromDatFile() reads one strip hit from a well-formed event",
                        R.GetNStripHits(), (unsigned int) 1) && Passed;
      Passed = Evaluate("GetNextFromDatFile()", "ID parsed", "GetNextFromDatFile() reads the event ID 1",
                        R.GetID(), (unsigned long) 1) && Passed;
    }
    MFile::Remove(DatFile);
  }

  // --- A malformed HT line is rejected, not added as an invalid hit --------
  {
    MString DatFile("/tmp/UTNReadOutAssembly_badht.dat");
    ofstream Out(DatFile.Data());
    Passed = EvaluateTrue("GetNextFromDatFile()", "write bad-HT .dat file", "The bad-HT .dat fixture file can be created",
                          Out.is_open()) && Passed;
    Out << "SE" << endl;
    Out << "HT garbage" << endl;
    Out << "SE" << endl;
    Out.close();

    MReadOutAssembly R;
    MFile F;
    bool Opened = F.Open(DatFile);
    Passed = EvaluateTrue("GetNextFromDatFile()", "open bad-HT .dat file", "The bad-HT .dat test file can be opened",
                          Opened) && Passed;
    if (Opened == true) {
      bool Result = R.GetNextFromDatFile(F);
      F.Close();
      Passed = Evaluate("GetNextFromDatFile()", "malformed HT line", "GetNextFromDatFile() does not add a hit for a malformed HT line",
                        R.GetNHits(), (unsigned int) 0) && Passed;
      Passed = EvaluateFalse("GetNextFromDatFile()", "malformed HT return value", "GetNextFromDatFile() returns false when the only HT line is malformed",
                             Result) && Passed;
    }
    MFile::Remove(DatFile);
  }

  // --- A malformed SH line is rejected, the preceding hit is kept ----------
  {
    MString DatFile("/tmp/UTNReadOutAssembly_badsh.dat");
    ofstream Out(DatFile.Data());
    Passed = EvaluateTrue("GetNextFromDatFile()", "write bad-SH .dat file", "The bad-SH .dat fixture file can be created",
                          Out.is_open()) && Passed;
    Out << "SE" << endl;
    Out << "HT 1.0 2.0 3.0 511.0" << endl;
    Out << "SH garbage" << endl;
    Out << "SE" << endl;
    Out.close();

    MReadOutAssembly R;
    MFile F;
    bool Opened = F.Open(DatFile);
    Passed = EvaluateTrue("GetNextFromDatFile()", "open bad-SH .dat file", "The bad-SH .dat test file can be opened",
                          Opened) && Passed;
    if (Opened == true) {
      DisableDefaultStreams();
      bool Result = R.GetNextFromDatFile(F);
      EnableDefaultStreams();
      F.Close();
      Passed = Evaluate("GetNextFromDatFile()", "valid HT kept", "GetNextFromDatFile() keeps the valid HT line preceding a malformed SH line",
                        R.GetNHits(), (unsigned int) 1) && Passed;
      Passed = Evaluate("GetNextFromDatFile()", "malformed SH line", "GetNextFromDatFile() does not add a strip hit for a malformed SH line",
                        R.GetNStripHits(), (unsigned int) 0) && Passed;
      Passed = EvaluateTrue("GetNextFromDatFile()", "malformed SH return value", "GetNextFromDatFile() returns true when a valid HT precedes a malformed SH line",
                            Result) && Passed;
    }
    MFile::Remove(DatFile);
  }

  // --- A malformed ID line leaves the ID at its sentinel -------------------
  {
    MString DatFile("/tmp/UTNReadOutAssembly_badid.dat");
    ofstream Out(DatFile.Data());
    Passed = EvaluateTrue("GetNextFromDatFile()", "write bad-ID .dat file", "The bad-ID .dat fixture file can be created",
                          Out.is_open()) && Passed;
    Out << "SE" << endl;
    Out << "ID garbage" << endl;
    Out << "HT 1.0 2.0 3.0 511.0" << endl;
    Out << "SE" << endl;
    Out.close();

    MReadOutAssembly R;
    MFile F;
    bool Opened = F.Open(DatFile);
    Passed = EvaluateTrue("GetNextFromDatFile()", "open bad-ID .dat file", "The bad-ID .dat test file can be opened",
                          Opened) && Passed;
    if (Opened == true) {
      DisableDefaultStreams();
      R.GetNextFromDatFile(F);
      EnableDefaultStreams();
      F.Close();
      Passed = EvaluateTrue("GetNextFromDatFile()", "malformed ID line", "GetNextFromDatFile() leaves the ID at g_UnsignedIntNotDefined for a malformed ID line",
                            R.GetID() == g_UnsignedIntNotDefined) && Passed;
    }
    MFile::Remove(DatFile);
  }

  // --- A malformed ID with no other payload yields no event ----------------
  {
    MString DatFile("/tmp/UTNReadOutAssembly_badidonly.dat");
    ofstream Out(DatFile.Data());
    Passed = EvaluateTrue("GetNextFromDatFile()", "write bad-ID-only .dat file", "The bad-ID-only .dat fixture file can be created",
                          Out.is_open()) && Passed;
    Out << "SE" << endl;
    Out << "ID garbage" << endl;
    Out << "SE" << endl;
    Out.close();

    MReadOutAssembly R;
    MFile F;
    bool Opened = F.Open(DatFile);
    Passed = EvaluateTrue("GetNextFromDatFile()", "open bad-ID-only .dat file", "The bad-ID-only .dat test file can be opened",
                          Opened) && Passed;
    if (Opened == true) {
      DisableDefaultStreams();
      bool Result = R.GetNextFromDatFile(F);
      EnableDefaultStreams();
      F.Close();
      Passed = EvaluateFalse("GetNextFromDatFile()", "malformed ID only", "GetNextFromDatFile() returns false when the only content is a malformed ID line",
                             Result) && Passed;
      Passed = Evaluate("GetNextFromDatFile()", "malformed ID only hit count", "GetNextFromDatFile() reads no hits when the only content is a malformed ID line",
                        R.GetNHits(), (unsigned int) 0) && Passed;
    }
    MFile::Remove(DatFile);
  }

  // --- An event with no trailing SE is still read at end of file -----------
  {
    MString DatFile("/tmp/UTNReadOutAssembly_noterm.dat");
    ofstream Out(DatFile.Data());
    Passed = EvaluateTrue("GetNextFromDatFile()", "write unterminated .dat file", "The unterminated .dat fixture file can be created",
                          Out.is_open()) && Passed;
    Out << "SE" << endl;
    Out << "ID 5" << endl;
    Out << "HT 1.0 2.0 3.0 511.0" << endl;
    Out.close();   // deliberately no trailing SE

    MReadOutAssembly R;
    MFile F;
    bool Opened = F.Open(DatFile);
    Passed = EvaluateTrue("GetNextFromDatFile()", "open unterminated .dat file", "The unterminated .dat test file can be opened",
                          Opened) && Passed;
    if (Opened == true) {
      bool Result = R.GetNextFromDatFile(F);
      F.Close();
      Passed = EvaluateTrue("GetNextFromDatFile()", "event without trailing SE", "GetNextFromDatFile() returns true for a final event with no trailing SE",
                            Result) && Passed;
      Passed = Evaluate("GetNextFromDatFile()", "unterminated event hit count", "GetNextFromDatFile() reads the hit from an event with no trailing SE",
                        R.GetNHits(), (unsigned int) 1) && Passed;
    }
    MFile::Remove(DatFile);
  }

  // --- An empty event (only SE markers) yields no payload ------------------
  {
    MString DatFile("/tmp/UTNReadOutAssembly_empty.dat");
    ofstream Out(DatFile.Data());
    Passed = EvaluateTrue("GetNextFromDatFile()", "write empty .dat file", "The empty-event .dat fixture file can be created",
                          Out.is_open()) && Passed;
    Out << "SE" << endl;
    Out << "SE" << endl;
    Out.close();

    MReadOutAssembly R;
    MFile F;
    bool Opened = F.Open(DatFile);
    Passed = EvaluateTrue("GetNextFromDatFile()", "open empty .dat file", "The empty-event .dat test file can be opened",
                          Opened) && Passed;
    if (Opened == true) {
      bool Result = R.GetNextFromDatFile(F);
      F.Close();
      Passed = EvaluateFalse("GetNextFromDatFile()", "empty event", "GetNextFromDatFile() returns false for an event with only SE markers and no payload",
                             Result) && Passed;
      Passed = Evaluate("GetNextFromDatFile()", "empty event hit count", "GetNextFromDatFile() reads no hits from an empty event",
                        R.GetNHits(), (unsigned int) 0) && Passed;
    }
    MFile::Remove(DatFile);
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestStreamDat()
{
  bool Passed = true;

  // --- V1: strip hits and hits streamed separately -------------------------
  {
    MReadOutAssembly R;
    R.SetID(7);
    MHit* H = new MHit();
    H->SetPosition(MVector(1.0, 2.0, 3.0));
    H->SetEnergy(511.0);
    R.AddHit(H);

    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(2);
    SH->SetStripID(8);
    SH->IsLowVoltageStrip(true);
    R.AddStripHit(SH);

    ostringstream SS;
    Passed = EvaluateTrue("StreamDat() V1", "return value", "StreamDat(V1) returns true",
                          R.StreamDat(SS, 1)) && Passed;
    MString S = SS.str();
    Passed = EvaluateTrue("StreamDat() V1", "SE line", "StreamDat(V1) output starts with an 'SE' line",
                          S.BeginsWith("SE")) && Passed;
    Passed = EvaluateTrue("StreamDat() V1", "ID line", "StreamDat(V1) output contains the 'ID 7' line",
                          S.Contains("ID 7")) && Passed;
    Passed = EvaluateTrue("StreamDat() V1", "HT line", "StreamDat(V1) output contains the 'HT' line for the hit",
                          S.Contains("HT ")) && Passed;
    Passed = EvaluateTrue("StreamDat() V1", "SH line", "StreamDat(V1) output contains the standalone 'SH' line for the strip hit",
                          S.Contains("SH ")) && Passed;
    Passed = EvaluateTrue("StreamDat() V1", "PQ terminator", "StreamDat(V1) output contains the 'PQ' line from StreamBDFlags()",
                          S.Contains("PQ")) && Passed;
  }

  // --- V2: HT + SH lines, verified by parsing the output back --------------
  {
    MReadOutAssembly R;
    R.SetID(7);
    MHit* H = new MHit();
    H->SetPosition(MVector(1.0, 2.0, 3.0));
    H->SetEnergy(511.0);
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(0);
    SH->SetStripID(41);
    SH->IsLowVoltageStrip(true);
    R.AddStripHit(SH);    // the ROA owns the strip hit
    H->AddStripHit(SH);   // the hit references it
    R.AddHit(H);

    ostringstream SS;
    Passed = EvaluateTrue("StreamDat() V2", "return value", "StreamDat(V2) returns true",
                          R.StreamDat(SS, 2)) && Passed;
    Passed = EvaluateTrue("StreamDat() V2", "SE line", "StreamDat(V2) output starts with an 'SE' line",
                          MString(SS.str().c_str()).BeginsWith("SE")) && Passed;

    // Round-trip: parse the StreamDat output back into a fresh ROA
    MReadOutAssembly Reader;
    istringstream In(SS.str());
    string RawLine;
    while (getline(In, RawLine)) {
      MString PLine(RawLine.c_str());
      Reader.Parse(PLine, 2);
    }
    Passed = Evaluate("StreamDat() V2", "round-trip hit count", "Parsing StreamDat(V2) output back reconstructs one hit",
                      Reader.GetNHits(), (unsigned int) 1) && Passed;
    if (Reader.GetNHits() == 1) {
      Passed = EvaluateNear("StreamDat() V2", "round-trip energy", "Parsing StreamDat(V2) output back restores the hit energy 511",
                            Reader.GetHit(0)->GetEnergy(), 511.0, 1e-4) && Passed;
      Passed = Evaluate("StreamDat() V2", "round-trip strip hit", "Parsing StreamDat(V2) output back reattaches the strip hit to the hit",
                        Reader.GetHit(0)->GetNStripHits(), (unsigned int) 1) && Passed;
    }
  }

  // --- V3: HT (with LV/HV energy) + SH lines, verified by parsing back -----
  {
    MReadOutAssembly R;
    R.SetID(7);
    MHit* H = new MHit();
    H->SetPosition(MVector(1.0, 2.0, 3.0));
    H->SetEnergy(511.0);
    H->SetLVEnergy(250.0);
    H->SetHVEnergy(261.0);
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(0);
    SH->SetStripID(41);
    SH->IsLowVoltageStrip(true);
    R.AddStripHit(SH);
    H->AddStripHit(SH);
    R.AddHit(H);

    ostringstream SS;
    Passed = EvaluateTrue("StreamDat() V3", "return value", "StreamDat(V3) returns true",
                          R.StreamDat(SS, 3)) && Passed;

    // Round-trip: parse the StreamDat output back into a fresh ROA
    MReadOutAssembly Reader;
    istringstream In(SS.str());
    string RawLine;
    while (getline(In, RawLine)) {
      MString PLine(RawLine.c_str());
      Reader.Parse(PLine, 3);
    }
    Passed = Evaluate("StreamDat() V3", "round-trip hit count", "Parsing StreamDat(V3) output back reconstructs one hit",
                      Reader.GetNHits(), (unsigned int) 1) && Passed;
    if (Reader.GetNHits() == 1) {
      Passed = EvaluateNear("StreamDat() V3", "round-trip energy", "Parsing StreamDat(V3) output back restores the hit energy 511",
                            Reader.GetHit(0)->GetEnergy(), 511.0, 1e-4) && Passed;
      Passed = EvaluateNear("StreamDat() V3", "round-trip LV energy", "Parsing StreamDat(V3) output back restores the hit LV energy 250",
                            Reader.GetHit(0)->GetLVEnergy(), 250.0, 1e-4) && Passed;
      Passed = EvaluateNear("StreamDat() V3", "round-trip HV energy", "Parsing StreamDat(V3) output back restores the hit HV energy 261",
                            Reader.GetHit(0)->GetHVEnergy(), 261.0, 1e-4) && Passed;
      Passed = Evaluate("StreamDat() V3", "round-trip strip hit", "Parsing StreamDat(V3) output back reattaches the strip hit to the hit",
                        Reader.GetHit(0)->GetNStripHits(), (unsigned int) 1) && Passed;
    }
  }

  // --- Unsupported versions are rejected with no output --------------------
  {
    MReadOutAssembly R;
    R.SetID(7);

    ostringstream SS0;
    Passed = EvaluateFalse("StreamDat()", "unsupported version 0", "StreamDat(0) returns false for an unsupported version",
                           R.StreamDat(SS0, 0)) && Passed;
    Passed = EvaluateTrue("StreamDat()", "no output for version 0", "StreamDat(0) writes nothing when the version is unsupported",
                          SS0.str().empty()) && Passed;

    ostringstream SS99;
    Passed = EvaluateFalse("StreamDat()", "unsupported version 99", "StreamDat(99) returns false for an unsupported version",
                           R.StreamDat(SS99, 99)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestStreamEvta()
{
  bool Passed = true;

  // --- StreamEvta() format -------------------------------------------------
  {
    MReadOutAssembly R;
    R.SetID(8);
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(0);
    SH->SetStripID(12);
    R.AddStripHit(SH);

    ostringstream SS;
    R.StreamEvta(SS);
    MString S = SS.str();

    Passed = EvaluateTrue("StreamEvta()", "SE line", "StreamEvta() output starts with an 'SE' line",
                          S.BeginsWith("SE")) && Passed;
    Passed = EvaluateTrue("StreamEvta()", "ID line", "StreamEvta() output contains the 'ID 8' line",
                          S.Contains("ID 8")) && Passed;
    Passed = EvaluateTrue("StreamEvta()", "CC NStripHits line", "StreamEvta() output contains 'CC NStripHits 1' with the strip hit count",
                          S.Contains("CC NStripHits 1")) && Passed;
    Passed = EvaluateTrue("StreamEvta()", "PQ terminator", "StreamEvta() output contains the 'PQ' line from StreamBDFlags()",
                          S.Contains("PQ")) && Passed;
  }

  // --- StreamTra() without a physical event --------------------------------
  {
    MReadOutAssembly R;
    R.SetID(9);

    ostringstream SS;
    R.StreamTra(SS);
    MString S = SS.str();

    Passed = EvaluateTrue("StreamTra()", "SE line", "StreamTra() output starts with an 'SE' line",
                          S.BeginsWith("SE")) && Passed;
    Passed = EvaluateTrue("StreamTra()", "ID line without physical event", "StreamTra() output contains the 'ID 9' line when there is no physical event",
                          S.Contains("ID 9")) && Passed;
    Passed = EvaluateTrue("StreamTra()", "PQ terminator", "StreamTra() output contains the 'PQ' line from StreamBDFlags()",
                          S.Contains("PQ")) && Passed;
  }

  // --- StreamTra() with a physical event uses the event's tra string -------
  {
    MReadOutAssembly R;
    R.SetID(10);
    MPhysicalEvent* PE = new MPhysicalEvent();
    R.SetPhysicalEvent(PE);
    delete PE;   // SetPhysicalEvent duplicated it, so the caller still owns PE

    ostringstream SS;
    R.StreamTra(SS);
    MString S = SS.str();

    Passed = EvaluateTrue("StreamTra()", "SE line with physical event", "StreamTra() output starts with an 'SE' line when a physical event is set",
                          S.BeginsWith("SE")) && Passed;
    Passed = EvaluateTrue("StreamTra()", "physical-event branch taken", "StreamTra() with a physical event does not emit the no-event branch's 'PQ' line",
                          S.Contains("PQ") == false) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestStreamRoa()
{
  bool Passed = true;

  // --- Empty ROA: must emit "BD No hits" and end with "PQ" ---
  {
    MReadOutAssembly R;
    R.SetID(1);
    ostringstream SS;
    R.StreamRoa(SS);
    MString S = SS.str();

    Passed = EvaluateTrue("StreamRoa()", "SE line present", "StreamRoa() output contains 'SE'",
                          S.Contains("SE")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "ID line present", "StreamRoa() output contains 'ID '",
                          S.Contains("ID ")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "BD No hits when empty", "StreamRoa() emits 'BD No hits' when there are no strip hits",
                          S.Contains("BD No hits")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "PQ terminator present", "StreamRoa() output ends with a 'PQ' line",
                          S.Contains("PQ")) && Passed;
  }

  // --- ROA with one strip hit: UH line present, no "BD No hits" ---
  {
    MReadOutAssembly R;
    R.SetID(2);
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(0);
    SH->SetStripID(10);
    SH->IsXStrip(true);
    SH->SetADCUnits(1000.0);
    R.AddStripHit(SH);

    ostringstream SS;
    R.StreamRoa(SS);
    MString S = SS.str();

    Passed = EvaluateTrue("StreamRoa()", "UH line present with strip hit", "StreamRoa() output contains 'UH' when there is a strip hit",
                          S.Contains("UH")) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "BD No hits absent with strip hit", "StreamRoa() does not emit 'BD No hits' when there is a strip hit",
                          S.Contains("BD No hits") == false) && Passed;
    Passed = EvaluateTrue("StreamRoa()", "PQ terminator present", "StreamRoa() output ends with a 'PQ' line",
                          S.Contains("PQ")) && Passed;
  }

  // --- Nearest-neighbor filtering ---
  // WithNearestNeighbors=false (default): NN strips skipped; non-NN strips emitted
  {
    MReadOutAssembly R;
    R.SetID(3);

    MStripHit* SH_NN = new MStripHit();
    SH_NN->SetDetectorID(0);
    SH_NN->SetStripID(5);
    SH_NN->IsXStrip(true);
    SH_NN->IsNearestNeighbor(true);
    SH_NN->SetADCUnits(500.0);
    R.AddStripHit(SH_NN);

    MStripHit* SH_Good = new MStripHit();
    SH_Good->SetDetectorID(0);
    SH_Good->SetStripID(10);
    SH_Good->IsXStrip(true);
    SH_Good->IsNearestNeighbor(false);
    SH_Good->SetADCUnits(1200.0);
    R.AddStripHit(SH_Good);

    // Without nearest neighbors: only SH_Good should appear (one UH line)
    ostringstream SS_noNN;
    R.StreamRoa(SS_noNN, true, true, false, false, false, false, false, false);
    MString S_noNN = SS_noNN.str();
    size_t count = 0;
    size_t pos = 0;
    while ((pos = S_noNN.Index("UH", pos)) != string::npos) {
      ++count;
      pos += 2;
    }
    Passed = Evaluate("StreamRoa()", "WithNearestNeighbors=false: one UH line",
                      "StreamRoa() with WithNearestNeighbors=false emits exactly one UH line",
                      count, (size_t) 1) && Passed;

    // With nearest neighbors: both strips should appear (two UH lines)
    ostringstream SS_withNN;
    R.StreamRoa(SS_withNN, true, true, false, false, false, false, false, true);
    MString S_withNN = SS_withNN.str();
    count = 0;
    pos = 0;
    while ((pos = S_withNN.Index("UH", pos)) != string::npos) {
      ++count;
      pos += 2;
    }
    Passed = Evaluate("StreamRoa()", "WithNearestNeighbors=true: two UH lines",
                      "StreamRoa() with WithNearestNeighbors=true emits two UH lines",
                      count, (size_t) 2) && Passed;
  }

  // --- WithOrigins forwarding: origins appear only when WithOrigins=true ---
  {
    MReadOutAssembly R;
    R.SetID(5);
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(0);
    SH->SetStripID(7);
    SH->IsXStrip(true);
    SH->SetADCUnits(900.0);
    SH->AddOrigins({42});
    R.AddStripHit(SH);

    // WithOrigins=false: the origin value 42 must not appear
    ostringstream SS_no;
    R.StreamRoa(SS_no, true, true, false, false, false, false, false, false);
    MString S_no = SS_no.str();
    Passed = EvaluateTrue("StreamRoa()", "WithOrigins=false omits origins", "StreamRoa() with WithOrigins=false does not emit the origin value 42",
                          S_no.Contains("42") == false) && Passed;

    // WithOrigins=true: the origin value 42 must appear
    ostringstream SS_yes;
    R.StreamRoa(SS_yes, true, true, false, false, false, false, true, false);
    MString S_yes = SS_yes.str();
    Passed = EvaluateTrue("StreamRoa()", "WithOrigins=true emits origins", "StreamRoa() with WithOrigins=true emits the origin value 42",
                          S_yes.Contains("42")) && Passed;
  }

  // --- WithOrigins forwarding for crystal hits -----------------------------
  {
    MReadOutAssembly R;
    R.SetID(6);
    MCrystalHit* CH = new MCrystalHit();
    CH->SetCrystalID(2);
    CH->AddOrigins({77});
    R.AddCrystalHit(CH);

    // WithOrigins=false: the crystal hit origin 77 must not appear
    ostringstream SS_no;
    R.StreamRoa(SS_no, true, true, false, false, false, false, false, false);
    MString S_no = SS_no.str();
    Passed = EvaluateTrue("StreamRoa()", "crystal WithOrigins=false omits origins", "StreamRoa() with WithOrigins=false does not emit the crystal hit origin 77",
                          S_no.Contains("77") == false) && Passed;

    // WithOrigins=true: the crystal hit origin 77 must appear
    ostringstream SS_yes;
    R.StreamRoa(SS_yes, true, true, false, false, false, false, true, false);
    MString S_yes = SS_yes.str();
    Passed = EvaluateTrue("StreamRoa()", "crystal WithOrigins=true emits origins", "StreamRoa() with WithOrigins=true emits the crystal hit origin 77",
                          S_yes.Contains("77")) && Passed;
  }

  // --- BD flag and quality flag lines in StreamBDFlags ---
  {
    MReadOutAssembly R;
    R.SetID(4);
    R.SetEnergyCalibrationError("ecal_test");
    R.SetStripPairingError();
    R.SetDepthCalibrationError();
    R.SetEventReconstructionError();
    R.SetGuardRingVeto(true);
    R.SetShieldVeto(true);
    R.SetStripHitBelowThreshold_QualityFlag();
    R.SetStripPairing_QualityFlag();

    ostringstream SS;
    R.StreamBDFlags(SS);
    MString S = SS.str();

    Passed = EvaluateTrue("StreamBDFlags()", "BD EnergyCalibrationError line", "StreamBDFlags() emits 'BD EnergyCalibrationError' when that flag is set",
                          S.Contains("BD EnergyCalibrationError")) && Passed;
    Passed = EvaluateTrue("StreamBDFlags()", "error text appended", "StreamBDFlags() appends the representative error text 'ecal_test'",
                          S.Contains("ecal_test")) && Passed;
    Passed = EvaluateTrue("StreamBDFlags()", "BD StripPairingError line", "StreamBDFlags() emits 'BD StripPairingError' when that flag is set",
                          S.Contains("BD StripPairingError")) && Passed;
    Passed = EvaluateTrue("StreamBDFlags()", "BD DepthCalibrationError line", "StreamBDFlags() emits 'BD DepthCalibrationError' when that flag is set",
                          S.Contains("BD DepthCalibrationError")) && Passed;
    Passed = EvaluateTrue("StreamBDFlags()", "BD EventReconstructionError line", "StreamBDFlags() emits 'BD EventReconstructionError' when that flag is set",
                          S.Contains("BD EventReconstructionError")) && Passed;
    Passed = EvaluateTrue("StreamBDFlags()", "BD GR Veto line", "StreamBDFlags() emits 'BD GR Veto' when GuardRingVeto is set",
                          S.Contains("BD GR Veto")) && Passed;
    Passed = EvaluateTrue("StreamBDFlags()", "BD Shield Veto line", "StreamBDFlags() emits 'BD Shield Veto' when ShieldVeto is set",
                          S.Contains("BD Shield Veto")) && Passed;
    Passed = EvaluateTrue("StreamBDFlags()", "QA StripHitBelowThreshold line", "StreamBDFlags() emits 'QA StripHitBelowThreshold' when that flag is set",
                          S.Contains("QA StripHitBelowThreshold")) && Passed;
    Passed = EvaluateTrue("StreamBDFlags()", "QA StripPairing line", "StreamBDFlags() emits 'QA StripPairing' when that flag is set",
                          S.Contains("QA StripPairing")) && Passed;
    Passed = EvaluateTrue("StreamBDFlags()", "PQ terminator always present", "StreamBDFlags() always ends with a 'PQ' line",
                          S.Contains("PQ")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNReadOutAssembly::TestAssemblyIDUniqueness()
{
  bool Passed = true;

  MReadOutAssembly R0;
  MReadOutAssembly R1;
  MReadOutAssembly R2;

  unsigned long ID0 = R0.GetAssemblyID();
  unsigned long ID1 = R1.GetAssemblyID();
  unsigned long ID2 = R2.GetAssemblyID();

  // Each instance must have a distinct ID
  Passed = EvaluateTrue("GetAssemblyID()", "R0 and R1 distinct", "Two separately constructed ROAs have different assembly IDs",
                        ID0 != ID1) && Passed;
  Passed = EvaluateTrue("GetAssemblyID()", "R1 and R2 distinct", "Three separately constructed ROAs all have different assembly IDs",
                        ID1 != ID2) && Passed;

  // IDs are assigned by incrementing counter so they are strictly increasing
  Passed = EvaluateTrue("GetAssemblyID()", "strictly increasing", "Assembly IDs are strictly increasing across consecutive constructions",
                        ID0 < ID1 && ID1 < ID2) && Passed;

  // AssemblyID must survive Clear()
  R0.Clear();
  Passed = EvaluateTrue("GetAssemblyID()", "unchanged by Clear", "GetAssemblyID() returns the same value before and after Clear()",
                        R0.GetAssemblyID() == ID0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNReadOutAssembly", "Unit tests for MReadOutAssembly") == false) return 1;

  UTNReadOutAssembly Test;
  return Test.Run() == true ? 0 : 1;
}
