/*
 * UTNModuleStripPairingMultiRoundChiSquare.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
#include <sstream>
#include <string>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MModuleStripPairingMultiRoundChiSquare.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MHit.h"


//! Unit test class for MModuleStripPairingMultiRoundChiSquare
class UTNModuleStripPairingMultiRoundChiSquare : public MUnitTest
{
public:
  UTNModuleStripPairingMultiRoundChiSquare() : MUnitTest("UTNModuleStripPairingMultiRoundChiSquare") {}
  virtual ~UTNModuleStripPairingMultiRoundChiSquare() {}

  virtual bool Run();

private:
  //! Test default values after construction
  bool TestDefaultConstruction();
  //! Test getter and setter methods
  bool TestGettersSetters();
  //! Test the XML configuration node
  bool TestXmlConfiguration();
  //! Test pairing of a single-detector, single-hit event
  bool TestSingleDetectorPairing();
  //! Test nearest neighbor assignment in events spanning several detectors
  bool TestMultiDetectorNearestNeighbors();
  //! Test that only nearest neighbors adjacent to two hits are flagged as ambiguous
  bool TestAmbiguousNearestNeighbors();
  //! Test that a guard ring hit is reported once with its own detector ID
  bool TestGuardRingQualityFlag();

  //! Run the two-detector nearest-neighbor event with the given order of detectors in the strip-hit list
  bool RunTwoDetectorNearestNeighborCase(bool DetectorFiveFirst);

  //! Create a strip hit and add it to the event (the event owns it)
  MStripHit* AddStripHit(MReadOutAssembly* Event, unsigned int DetectorID, unsigned int StripID, bool LowVoltage, double Energy, bool NearestNeighbor) const;
  //! Return the hit containing the given triggered strip, or nullptr if there is none
  MHit* FindHitWithStrip(MReadOutAssembly* Event, unsigned int DetectorID, unsigned int StripID, bool LowVoltage) const;
  //! Return the number of nearest neighbor strip hits assigned to a hit
  unsigned int CountNearestNeighbors(MHit* Hit) const;
  //! Return true if the nearest neighbor strip hit is assigned to the hit
  bool HasNearestNeighbor(MHit* Hit, MStripHit* StripHit) const;
  //! Return how often Pattern occurs in Text
  unsigned int CountOccurrences(const string& Text, const string& Pattern) const;
};


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleStripPairingMultiRoundChiSquare::AddStripHit(MReadOutAssembly* Event, unsigned int DetectorID, unsigned int StripID, bool LowVoltage, double Energy, bool NearestNeighbor) const
{
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(DetectorID);
  SH->SetStripID(StripID);
  SH->IsLowVoltageStrip(LowVoltage);
  SH->SetEnergy(Energy);
  SH->SetEnergyResolution(1.5);
  SH->IsNearestNeighbor(NearestNeighbor);
  Event->AddStripHit(SH);
  return SH;
}


////////////////////////////////////////////////////////////////////////////////


MHit* UTNModuleStripPairingMultiRoundChiSquare::FindHitWithStrip(MReadOutAssembly* Event, unsigned int DetectorID, unsigned int StripID, bool LowVoltage) const
{
  // Hits are identified by their triggered strips, since the order of hits in the event is not part of the contract
  for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
    MHit* Hit = Event->GetHit(h);
    for (unsigned int sh = 0; sh < Hit->GetNStripHits(); ++sh) {
      MStripHit* SH = Hit->GetStripHit(sh);
      if (SH->GetDetectorID() == DetectorID && SH->GetStripID() == StripID && SH->IsLowVoltageStrip() == LowVoltage) {
        return Hit;
      }
    }
  }
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTNModuleStripPairingMultiRoundChiSquare::CountNearestNeighbors(MHit* Hit) const
{
  // MHit has no count getter for nearest neighbors, and GetNearestNeighborStripHit() prints an error
  // when the index is out of bounds, thus keep the expected out-of-bounds call quiet
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  unsigned int N = 0;
  while (Hit->GetNearestNeighborStripHit(N) != nullptr) {
    ++N;
  }
  g_Verbosity = OldVerbosity;
  return N;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::HasNearestNeighbor(MHit* Hit, MStripHit* StripHit) const
{
  unsigned int N = CountNearestNeighbors(Hit);
  for (unsigned int i = 0; i < N; ++i) {
    if (Hit->GetNearestNeighborStripHit(i) == StripHit) return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int UTNModuleStripPairingMultiRoundChiSquare::CountOccurrences(const string& Text, const string& Pattern) const
{
  unsigned int N = 0;
  size_t Pos = Text.find(Pattern);
  while (Pos != string::npos) {
    ++N;
    Pos = Text.find(Pattern, Pos + Pattern.size());
  }
  return N;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlConfiguration() && Passed;
  Passed = TestSingleDetectorPairing() && Passed;
  Passed = TestMultiDetectorNearestNeighbors() && Passed;
  Passed = TestAmbiguousNearestNeighbors() && Passed;
  Passed = TestGuardRingQualityFlag() && Passed;

  // ShowOptionsGUI() and CreateExpos() need an interactive ROOT GUI; they cannot be exercised in a
  // headless unit test and are intentionally left uncovered.

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleStripPairingMultiRoundChiSquare Module;

  Passed = Evaluate("GetMaximumStrips()", "default", "GetMaximumStrips() returns 6 after construction", Module.GetMaximumStrips(), 6u) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::TestGettersSetters()
{
  bool Passed = true;

  MModuleStripPairingMultiRoundChiSquare Module;

  Module.SetMaximumStrips(8);
  Passed = Evaluate("SetMaximumStrips/GetMaximumStrips", "representative value", "GetMaximumStrips returns the value passed to SetMaximumStrips", Module.GetMaximumStrips(), 8u) && Passed;

  // Clone() returns a new, distinct instance
  MModuleStripPairingMultiRoundChiSquare* Cloned = Module.Clone();
  Passed = EvaluateTrue("Clone()", "new instance", "Clone() returns a non-null pointer", Cloned != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "distinct instance", "Clone() returns an object distinct from the original", Cloned != &Module) && Passed;
  delete Cloned;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::TestXmlConfiguration()
{
  bool Passed = true;

  // The module saves MaximumStrips to the XML config and should survive a round trip
  // Testing this with a representative value of 10, which is different from the default 6
  MModuleStripPairingMultiRoundChiSquare Writer;
  Writer.SetMaximumStrips(10);

  MXmlNode* Node = Writer.CreateXmlConfiguration();
  Passed = EvaluateTrue("CreateXmlConfiguration()", "representative values", "CreateXmlConfiguration() returns a non-null node", Node != nullptr) && Passed;
  if (Node == nullptr) return false;

  Passed = Evaluate("CreateXmlConfiguration()", "node name", "CreateXmlConfiguration() names the node after the module XML tag", 
                    Node->GetName(), MString("XmlTagStripPairingMultiRoundChiSquare")) && Passed;

  MModuleStripPairingMultiRoundChiSquare Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "representative values", "ReadXmlConfiguration() returns true for the node created by CreateXmlConfiguration()", 
                    Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "MaximumStrips", "ReadXmlConfiguration() restores the representative MaximumStrips",
                    Reader.GetMaximumStrips(), 10u) && Passed;

  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::TestSingleDetectorPairing()
{
  bool Passed = true;

  MModuleStripPairingMultiRoundChiSquare Module;
  Passed = EvaluateTrue("Initialize()", "default", "Initialize() succeeds without a geometry", Module.Initialize()) && Passed;

  MReadOutAssembly* Event = new MReadOutAssembly();
  AddStripHit(Event, 0, 10, true, 300.0, false);
  AddStripHit(Event, 0, 30, false, 300.0, false);

  Passed = EvaluateTrue("AnalyzeEvent()", "one LV and one HV strip", "AnalyzeEvent() succeeds for one matching LV/HV strip pair", Module.AnalyzeEvent(Event)) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "one LV and one HV strip", "AnalyzeEvent() sets no strip pairing error for one matching LV/HV strip pair", Event->HasStripPairingError()) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "one LV and one HV strip", "AnalyzeEvent() creates one hit from one matching LV/HV strip pair", Event->GetNHits(), 1u) && Passed;

  if (Event->GetNHits() == 1) {
    MHit* Hit = Event->GetHit(0);
    Passed = Evaluate("AnalyzeEvent()", "one LV and one HV strip", "The hit contains both triggered strips", Hit->GetNStripHits(), 2u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "one LV and one HV strip", "No nearest neighbors are assigned when the event has none", CountNearestNeighbors(Hit), 0u) && Passed;
  }

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::RunTwoDetectorNearestNeighborCase(bool DetectorFiveFirst)
{
  bool Passed = true;

  MString Case = DetectorFiveFirst == true ? "detector 5 strips first" : "detector 0 strips first";

  MModuleStripPairingMultiRoundChiSquare Module;
  Module.Initialize();

  // One hit in detector 0 and one in detector 5; only detector 5 has nearest neighbors
  // Before the fix, the nearest neighbor list had a single entry (detector 5), and the detector 5 hit read entry 5
  MReadOutAssembly* Event = new MReadOutAssembly();
  MStripHit* NNLV = nullptr;
  MStripHit* NNHV = nullptr;
  if (DetectorFiveFirst == true) {
    AddStripHit(Event, 5, 20, true, 200.0, false);
    AddStripHit(Event, 5, 40, false, 200.0, false);
    NNLV = AddStripHit(Event, 5, 21, true, 5.0, true);
    NNHV = AddStripHit(Event, 5, 39, false, 5.0, true);
    AddStripHit(Event, 0, 10, true, 300.0, false);
    AddStripHit(Event, 0, 30, false, 300.0, false);
  } else {
    AddStripHit(Event, 0, 10, true, 300.0, false);
    AddStripHit(Event, 0, 30, false, 300.0, false);
    AddStripHit(Event, 5, 20, true, 200.0, false);
    AddStripHit(Event, 5, 40, false, 200.0, false);
    NNLV = AddStripHit(Event, 5, 21, true, 5.0, true);
    NNHV = AddStripHit(Event, 5, 39, false, 5.0, true);
  }

  Passed = EvaluateTrue("AnalyzeEvent()", Case, "AnalyzeEvent() succeeds for hits in detectors 0 and 5 with nearest neighbors in detector 5", Module.AnalyzeEvent(Event)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", Case, "AnalyzeEvent() creates one hit per detector", Event->GetNHits(), 2u) && Passed;

  MHit* HitZero = FindHitWithStrip(Event, 0, 10, true);
  MHit* HitFive = FindHitWithStrip(Event, 5, 20, true);
  Passed = EvaluateTrue("AnalyzeEvent()", Case, "A hit containing detector 0 LV strip 10 exists", HitZero != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", Case, "A hit containing detector 5 LV strip 20 exists", HitFive != nullptr) && Passed;

  if (HitZero != nullptr && HitFive != nullptr) {
    Passed = Evaluate("AnalyzeEvent()", Case, "The detector 0 hit gets no nearest neighbors from detector 5", CountNearestNeighbors(HitZero), 0u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", Case, "The detector 5 hit gets both of its nearest neighbors", CountNearestNeighbors(HitFive), 2u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", Case, "LV strip 21 is assigned to the detector 5 hit (LV edge 20 + 1)", HasNearestNeighbor(HitFive, NNLV)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", Case, "HV strip 39 is assigned to the detector 5 hit (HV edge 40 - 1)", HasNearestNeighbor(HitFive, NNHV)) && Passed;
  }

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::TestMultiDetectorNearestNeighbors()
{
  bool Passed = true;

  // The order of detectors in the strip-hit list must not matter
  Passed = RunTwoDetectorNearestNeighborCase(true) && Passed;
  Passed = RunTwoDetectorNearestNeighborCase(false) && Passed;

  // A nearest neighbor in another detector must never be assigned, even if its strip ID borders the hit.
  // Before the fix, the detector 0 hit read the first list entry, which held the detector 15 nearest neighbors.
  MModuleStripPairingMultiRoundChiSquare Module;
  Module.Initialize();

  MReadOutAssembly* Event = new MReadOutAssembly();
  AddStripHit(Event, 0, 10, true, 300.0, false);
  AddStripHit(Event, 0, 30, false, 300.0, false);
  MStripHit* OtherDetectorNN = AddStripHit(Event, 15, 11, true, 5.0, true);

  Passed = EvaluateTrue("AnalyzeEvent()", "nearest neighbor only in detector 15", "AnalyzeEvent() succeeds when a detector without hits has a nearest neighbor", Module.AnalyzeEvent(Event)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "nearest neighbor only in detector 15", "AnalyzeEvent() creates the detector 0 hit", Event->GetNHits(), 1u) && Passed;

  if (Event->GetNHits() == 1) {
    Passed = Evaluate("AnalyzeEvent()", "nearest neighbor only in detector 15", "The detector 15 LV strip 11 is not assigned to the detector 0 hit with LV edge 10", CountNearestNeighbors(Event->GetHit(0)), 0u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "nearest neighbor only in detector 15", "The unassigned detector 15 nearest neighbor is not flagged as ambiguous", OtherDetectorNN->IsAmbiguousNearestNeighbor()) && Passed;
  }

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::TestAmbiguousNearestNeighbors()
{
  bool Passed = true;

  MModuleStripPairingMultiRoundChiSquare Module;
  Module.Initialize();

  // Two hits in detector 3: C = LV 10 / HV 30 (300 keV) and D = LV 14 / HV 32 (150 keV)
  // The strips are not adjacent and the energies match exactly, thus the first pairing round finds C and D
  MReadOutAssembly* Event = new MReadOutAssembly();
  AddStripHit(Event, 3, 10, true, 300.0, false);
  AddStripHit(Event, 3, 14, true, 150.0, false);
  AddStripHit(Event, 3, 30, false, 300.0, false);
  AddStripHit(Event, 3, 32, false, 150.0, false);

  MStripHit* LV11 = AddStripHit(Event, 3, 11, true, 5.0, true);  // Next to C only
  MStripHit* LV12 = AddStripHit(Event, 3, 12, true, 5.0, true);  // Next to neither
  MStripHit* LV13 = AddStripHit(Event, 3, 13, true, 5.0, true);  // Next to D only
  MStripHit* HV31 = AddStripHit(Event, 3, 31, false, 5.0, true); // Between C and D
  MStripHit* HV33 = AddStripHit(Event, 3, 33, false, 5.0, true); // Next to D only
  MStripHit* HV50 = AddStripHit(Event, 3, 50, false, 5.0, true); // Next to neither

  Passed = EvaluateTrue("AnalyzeEvent()", "two hits in detector 3", "AnalyzeEvent() succeeds for two hits in one detector with nearest neighbors", Module.AnalyzeEvent(Event)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "two hits in detector 3", "AnalyzeEvent() creates two hits", Event->GetNHits(), 2u) && Passed;

  MHit* HitC = FindHitWithStrip(Event, 3, 10, true);
  MHit* HitD = FindHitWithStrip(Event, 3, 14, true);
  Passed = EvaluateTrue("AnalyzeEvent()", "two hits in detector 3", "A hit containing LV strip 10 exists", HitC != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "two hits in detector 3", "A hit containing LV strip 14 exists", HitD != nullptr) && Passed;

  if (HitC != nullptr && HitD != nullptr) {
    Passed = EvaluateTrue("AnalyzeEvent()", "two hits in detector 3", "The hit containing LV strip 10 is paired with HV strip 30", FindHitWithStrip(Event, 3, 30, false) == HitC) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "two hits in detector 3", "The hit containing LV strip 14 is paired with HV strip 32", FindHitWithStrip(Event, 3, 32, false) == HitD) && Passed;

    // Assignment
    Passed = Evaluate("AnalyzeEvent()", "two hits in detector 3", "Hit C gets LV 11 and HV 31", CountNearestNeighbors(HitC), 2u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "two hits in detector 3", "Hit D gets LV 13, HV 31, and HV 33", CountNearestNeighbors(HitD), 3u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "LV 11", "LV strip 11 is assigned to hit C", HasNearestNeighbor(HitC, LV11)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "LV 13", "LV strip 13 is assigned to hit D", HasNearestNeighbor(HitD, LV13)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "HV 31", "HV strip 31 is assigned to hit C", HasNearestNeighbor(HitC, HV31)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "HV 31", "HV strip 31 is assigned to hit D", HasNearestNeighbor(HitD, HV31)) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "HV 33", "HV strip 33 is assigned to hit D", HasNearestNeighbor(HitD, HV33)) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "LV 12", "LV strip 12 is not assigned to hit C", HasNearestNeighbor(HitC, LV12)) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "LV 12", "LV strip 12 is not assigned to hit D", HasNearestNeighbor(HitD, LV12)) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "HV 50", "HV strip 50 is not assigned to hit C", HasNearestNeighbor(HitC, HV50)) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "HV 50", "HV strip 50 is not assigned to hit D", HasNearestNeighbor(HitD, HV50)) && Passed;
  }

  // Ambiguity: only a nearest neighbor assigned to two hits is ambiguous
  Passed = EvaluateTrue("AnalyzeEvent()", "HV 31", "HV strip 31, adjacent to both hits, is flagged as ambiguous", HV31->IsAmbiguousNearestNeighbor()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "HV 33", "HV strip 33, adjacent to one hit only, is not flagged as ambiguous", HV33->IsAmbiguousNearestNeighbor()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "HV 50", "HV strip 50, adjacent to no hit, is not flagged as ambiguous", HV50->IsAmbiguousNearestNeighbor()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "LV 11", "LV strip 11, adjacent to one hit only, is not flagged as ambiguous", LV11->IsAmbiguousNearestNeighbor()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "LV 12", "LV strip 12, adjacent to no hit, is not flagged as ambiguous", LV12->IsAmbiguousNearestNeighbor()) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "LV 13", "LV strip 13, adjacent to one hit only, is not flagged as ambiguous", LV13->IsAmbiguousNearestNeighbor()) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleStripPairingMultiRoundChiSquare::TestGuardRingQualityFlag()
{
  bool Passed = true;

  MModuleStripPairingMultiRoundChiSquare Module;
  Module.Initialize();

  // Guard ring hit in detector 7 (its strips come first, thus it is at position 0 in the detector loop)
  // and a regular hit in detector 2 (position 1)
  // Before the fix, the QA text used the loop position as detector ID, and the guard ring hit was reported
  // again during every later detector iteration
  MReadOutAssembly* Event = new MReadOutAssembly();
  MStripHit* GuardRing = AddStripHit(Event, 7, 64, true, 200.0, false);
  GuardRing->IsGuardRing(true);
  AddStripHit(Event, 7, 40, false, 200.0, false);
  AddStripHit(Event, 2, 10, true, 300.0, false);
  AddStripHit(Event, 2, 30, false, 300.0, false);

  Passed = EvaluateTrue("AnalyzeEvent()", "guard ring hit in detector 7", "AnalyzeEvent() succeeds for a guard ring hit in detector 7 and a regular hit in detector 2", Module.AnalyzeEvent(Event)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "guard ring hit in detector 7", "AnalyzeEvent() creates one hit per detector", Event->GetNHits(), 2u) && Passed;

  MHit* HitSeven = FindHitWithStrip(Event, 7, 64, true);
  MHit* HitTwo = FindHitWithStrip(Event, 2, 10, true);
  Passed = EvaluateTrue("AnalyzeEvent()", "guard ring hit in detector 7", "A hit containing detector 7 LV strip 64 exists", HitSeven != nullptr) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "guard ring hit in detector 7", "A hit containing detector 2 LV strip 10 exists", HitTwo != nullptr) && Passed;

  if (HitSeven != nullptr && HitTwo != nullptr) {
    Passed = EvaluateTrue("AnalyzeEvent()", "guard ring hit in detector 7", "The detector 7 hit containing strip 64 has the guard ring hit flag", HitSeven->GetGuardRingHitFlag()) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "guard ring hit in detector 7", "The detector 2 hit without guard ring strips has no guard ring hit flag", HitTwo->GetGuardRingHitFlag()) && Passed;
  }

  // The QA text is only accessible through the evta stream
  ostringstream Out;
  Event->StreamEvta(Out);
  Passed = Evaluate("AnalyzeEvent()", "guard ring QA text", "The guard ring hit is reported with its own detector ID 7 exactly once", CountOccurrences(Out.str(), "GR Hit: Detector ID 7 "), 1u) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "guard ring QA text", "Exactly one guard ring hit is reported in total", CountOccurrences(Out.str(), "GR Hit: Detector ID"), 1u) && Passed;

  delete Event;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNModuleStripPairingMultiRoundChiSquare", "Unit tests for MModuleStripPairingMultiRoundChiSquare") == false) return 1;

  UTNModuleStripPairingMultiRoundChiSquare Test;
  return Test.Run() == true ? 0 : 1;
}
