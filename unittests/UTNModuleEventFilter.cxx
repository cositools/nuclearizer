/*
 * UTNModuleEventFilter.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <limits>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MPhotoEvent.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleEventFilter.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MHit.h"


//! Unit test class for MModuleEventFilter
class UTNModuleEventFilter : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleEventFilter() : MUnitTest("UTNModuleEventFilter") {}
  //! Default destructor
  virtual ~UTNModuleEventFilter() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data and the default configuration
  bool TestDefaultConstruction();
  //! Test all setter/getter pairs
  bool TestGettersSetters();
  //! Test the XML configuration round trip
  bool TestXmlRoundTrip();
  //! Test ReadXmlConfiguration() with a partial node
  bool TestXmlPartial();
  //! Test the total-energy filter for physical events, hits, and strip hits
  bool TestAnalyzeEventEnergy();
  //! Test the LV/HV strip count filter
  bool TestAnalyzeEventStripCounts();
  //! Test the hit count filter
  bool TestAnalyzeEventHitCount();
  //! Test the strip pairing reduced chi-square filter
  bool TestAnalyzeEventChiSquare();
  //! Test the detector list filter
  bool TestAnalyzeEventDetectorList();
  //! Test Initialize(), Finalize() and the analysis progress
  bool TestLifecycle();

  //! Create a strip hit
  MStripHit* CreateStripHit(unsigned int DetectorID, bool IsLowVoltage, double Energy);
  //! Create a hit with the given energy
  MHit* CreateHit(double Energy);
  //! Run the filter on the event and return true if the event was filtered out
  bool IsFilteredOut(MModuleEventFilter& F, MReadOutAssembly& Event);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestGettersSetters() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestXmlPartial() && Passed;
  Passed = TestAnalyzeEventEnergy() && Passed;
  Passed = TestAnalyzeEventStripCounts() && Passed;
  Passed = TestAnalyzeEventHitCount() && Passed;
  Passed = TestAnalyzeEventChiSquare() && Passed;
  Passed = TestAnalyzeEventDetectorList() && Passed;
  Passed = TestLifecycle() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleEventFilter::CreateStripHit(unsigned int DetectorID, bool IsLowVoltage, double Energy)
{
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(DetectorID);
  SH->IsLowVoltageStrip(IsLowVoltage);
  SH->SetStripID(10);
  SH->SetEnergy(Energy);
  return SH;
}


////////////////////////////////////////////////////////////////////////////////


MHit* UTNModuleEventFilter::CreateHit(double Energy)
{
  MHit* H = new MHit();
  H->SetEnergy(Energy);
  return H;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::IsFilteredOut(MModuleEventFilter& F, MReadOutAssembly& Event)
{
  Event.SetFilteredOut(false);
  F.AnalyzeEvent(&Event);
  return Event.IsFilteredOut();
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleEventFilter F;
  Passed = Evaluate("GetName()", "default", "The module name is 'Event Filter'", F.GetName(), MString("Event Filter")) && Passed;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagEventFilter'", F.GetXmlTag(), MString("XmlTagEventFilter")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_EventFilter", F.ProvidesModuleType(MAssembly::c_EventFilter)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "An event loader is a hard requirement", F.IsHardPreceedingModule(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", F.HasOptionsGUI()) && Passed;

  Passed = Evaluate("GetDetectorList()", "default", "The default detector list is empty", (unsigned int) F.GetDetectorList().size(), 0u) && Passed;
  Passed = EvaluateNear("GetMinimumTotalEnergy()", "default", "The default minimum total energy is 0", F.GetMinimumTotalEnergy(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaximumTotalEnergy()", "default", "The default maximum total energy is 10000", F.GetMaximumTotalEnergy(), 10000.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinimumHVStrips()", "default", "The default minimum number of HV strips is 0", F.GetMinimumHVStrips(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaximumHVStrips()", "default", "The default maximum number of HV strips is 20", F.GetMaximumHVStrips(), 20.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinimumLVStrips()", "default", "The default minimum number of LV strips is 0", F.GetMinimumLVStrips(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaximumLVStrips()", "default", "The default maximum number of LV strips is 20", F.GetMaximumLVStrips(), 20.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinimumHits()", "default", "The default minimum number of hits is 0", F.GetMinimumHits(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMaximumHits()", "default", "The default maximum number of hits is 100", F.GetMaximumHits(), 100.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinimumStripPairingReducedChiSquare()", "default", "The default minimum reduced chi square is -1", F.GetMinimumStripPairingReducedChiSquare(), -1.0, 1e-12) && Passed;
  Passed = EvaluateTrue("GetMaximumStripPairingReducedChiSquare()", "default", "The default maximum reduced chi square is the largest double", F.GetMaximumStripPairingReducedChiSquare() == numeric_limits<double>::max()) && Passed;

  MModuleEventFilter* C = F.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a non-null object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &F) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), F.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestGettersSetters()
{
  bool Passed = true;

  MModuleEventFilter F;
  F.SetDetectorList({ 3, 7 });
  F.SetMinimumTotalEnergy(50.0);
  F.SetMaximumTotalEnergy(2000.0);
  F.SetMinimumHVStrips(1);
  F.SetMaximumHVStrips(5);
  F.SetMinimumLVStrips(2);
  F.SetMaximumLVStrips(6);
  F.SetMinimumHits(1);
  F.SetMaximumHits(8);
  F.SetMinimumStripPairingReducedChiSquare(0.5);
  F.SetMaximumStripPairingReducedChiSquare(25.0);

  vector<int> D = F.GetDetectorList();
  Passed = Evaluate("SetDetectorList/GetDetectorList", "3, 7", "The detector list has two entries", (unsigned int) D.size(), 2u) && Passed;
  if (D.size() == 2) {
    Passed = Evaluate("SetDetectorList/GetDetectorList", "3, 7", "The first detector is 3", D[0], 3) && Passed;
    Passed = Evaluate("SetDetectorList/GetDetectorList", "3, 7", "The second detector is 7", D[1], 7) && Passed;
  }
  Passed = EvaluateNear("SetMinimumTotalEnergy/GetMinimumTotalEnergy", "50", "GetMinimumTotalEnergy returns 50", F.GetMinimumTotalEnergy(), 50.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMaximumTotalEnergy/GetMaximumTotalEnergy", "2000", "GetMaximumTotalEnergy returns 2000", F.GetMaximumTotalEnergy(), 2000.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinimumHVStrips/GetMinimumHVStrips", "1", "GetMinimumHVStrips returns 1", F.GetMinimumHVStrips(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMaximumHVStrips/GetMaximumHVStrips", "5", "GetMaximumHVStrips returns 5", F.GetMaximumHVStrips(), 5.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinimumLVStrips/GetMinimumLVStrips", "2", "GetMinimumLVStrips returns 2", F.GetMinimumLVStrips(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMaximumLVStrips/GetMaximumLVStrips", "6", "GetMaximumLVStrips returns 6", F.GetMaximumLVStrips(), 6.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinimumHits/GetMinimumHits", "1", "GetMinimumHits returns 1", F.GetMinimumHits(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMaximumHits/GetMaximumHits", "8", "GetMaximumHits returns 8", F.GetMaximumHits(), 8.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetMinimumStripPairingReducedChiSquare/GetMinimumStripPairingReducedChiSquare", "0.5", "GetMinimumStripPairingReducedChiSquare returns 0.5", F.GetMinimumStripPairingReducedChiSquare(), 0.5, 1e-12) && Passed;
  Passed = EvaluateNear("SetMaximumStripPairingReducedChiSquare/GetMaximumStripPairingReducedChiSquare", "25", "GetMaximumStripPairingReducedChiSquare returns 25", F.GetMaximumStripPairingReducedChiSquare(), 25.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestXmlRoundTrip()
{
  bool Passed = true;

  MModuleEventFilter Writer;
  Writer.SetDetectorList({ 2, 11 });
  Writer.SetMinimumTotalEnergy(55.5);
  Writer.SetMaximumTotalEnergy(1999.5);
  Writer.SetMinimumHVStrips(1);
  Writer.SetMaximumHVStrips(4);
  Writer.SetMinimumLVStrips(2);
  Writer.SetMaximumLVStrips(7);
  Writer.SetMinimumHits(3);
  Writer.SetMaximumHits(9);
  Writer.SetMinimumStripPairingReducedChiSquare(0.25);
  Writer.SetMaximumStripPairingReducedChiSquare(12.5);

  MXmlNode* Node = Writer.CreateXmlConfiguration();
  Passed = EvaluateTrue("CreateXmlConfiguration()", "configured", "CreateXmlConfiguration returns a node", Node != nullptr) && Passed;
  if (Node == nullptr) return false;
  Passed = Evaluate("CreateXmlConfiguration()", "configured", "The node name is the XML tag", Node->GetName(), MString("XmlTagEventFilter")) && Passed;

  MModuleEventFilter Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "configured", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MinimumTotalEnergy", "The minimum total energy survives", Reader.GetMinimumTotalEnergy(), 55.5, 1e-9) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MaximumTotalEnergy", "The maximum total energy survives", Reader.GetMaximumTotalEnergy(), 1999.5, 1e-9) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MinimumHVStrips", "The minimum HV strips survive", Reader.GetMinimumHVStrips(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MaximumHVStrips", "The maximum HV strips survive", Reader.GetMaximumHVStrips(), 4.0, 1e-12) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MinimumLVStrips", "The minimum LV strips survive", Reader.GetMinimumLVStrips(), 2.0, 1e-12) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MaximumLVStrips", "The maximum LV strips survive", Reader.GetMaximumLVStrips(), 7.0, 1e-12) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MinimumHits", "The minimum hits survive", Reader.GetMinimumHits(), 3.0, 1e-12) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MaximumHits", "The maximum hits survive", Reader.GetMaximumHits(), 9.0, 1e-12) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MinimumStripPairingReducedChiSquare", "The minimum reduced chi square survives", Reader.GetMinimumStripPairingReducedChiSquare(), 0.25, 1e-9) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "MaximumStripPairingReducedChiSquare", "The maximum reduced chi square survives", Reader.GetMaximumStripPairingReducedChiSquare(), 12.5, 1e-9) && Passed;
  Passed = ExpectedToFail(Evaluate("ReadXmlConfiguration()", "DetectorList", "The detector list survives the round trip", (unsigned int) Reader.GetDetectorList().size(), 2u)) && Passed;
  delete Node;

  // The default maximum reduced chi square is the largest double, it must stay a usable finite upper limit
  MModuleEventFilter DefaultWriter;
  MXmlNode* DefaultNode = DefaultWriter.CreateXmlConfiguration();
  MModuleEventFilter DefaultReader;
  DefaultReader.SetMaximumStripPairingReducedChiSquare(1.0);
  DefaultReader.ReadXmlConfiguration(DefaultNode);
  double Max = DefaultReader.GetMaximumStripPairingReducedChiSquare();
  Passed = ExpectedToFail(EvaluateTrue("ReadXmlConfiguration()", "default maximum reduced chi square", "The default maximum reduced chi square survives as a finite value", std::isfinite(Max))) && Passed;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "default maximum reduced chi square", "The default maximum reduced chi square survives as a huge value", Max > 1e300) && Passed;
  delete DefaultNode;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestXmlPartial()
{
  bool Passed = true;

  MXmlNode* Node = new MXmlNode(nullptr, "XmlTagEventFilter");
  new MXmlNode(Node, "MaximumHits", 42u);

  MModuleEventFilter Reader;
  Reader.SetMinimumTotalEnergy(33.0);
  Passed = EvaluateTrue("ReadXmlConfiguration()", "partial node", "ReadXmlConfiguration returns true for a partial node", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "partial node", "The given maximum hits value is read", Reader.GetMaximumHits(), 42.0, 1e-12) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "partial node", "Missing values keep their previous value", Reader.GetMinimumTotalEnergy(), 33.0, 1e-12) && Passed;
  Passed = EvaluateNear("ReadXmlConfiguration()", "partial node", "Missing values keep their default", Reader.GetMaximumHVStrips(), 20.0, 1e-12) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestAnalyzeEventEnergy()
{
  bool Passed = true;

  MModuleEventFilter F;
  F.SetMinimumTotalEnergy(400.0);
  F.SetMaximumTotalEnergy(600.0);

  // Hits: the sum of the hit energies is used
  {
    MReadOutAssembly E;
    E.AddHit(CreateHit(200.0));
    E.AddHit(CreateHit(311.0));
    Passed = EvaluateFalse("AnalyzeEvent()", "hits sum 511", "An event with 511 keV in hits passes the 400-600 keV window", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddHit(CreateHit(350.0));
    Passed = EvaluateTrue("AnalyzeEvent()", "hits sum 350", "An event with 350 keV in hits is filtered out", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddHit(CreateHit(700.0));
    Passed = EvaluateTrue("AnalyzeEvent()", "hits sum 700", "An event with 700 keV in hits is filtered out", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddHit(CreateHit(400.0));
    Passed = EvaluateFalse("AnalyzeEvent()", "hits sum 400", "The minimum energy is inclusive", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddHit(CreateHit(600.0));
    Passed = EvaluateFalse("AnalyzeEvent()", "hits sum 600", "The maximum energy is inclusive", IsFilteredOut(F, E)) && Passed;
  }

  // Physical event takes precedence over the hits
  {
    MReadOutAssembly E;
    E.AddHit(CreateHit(511.0));
    MPhotoEvent P;
    P.SetEnergy(1000.0);
    E.SetPhysicalEvent(&P);
    Passed = EvaluateTrue("AnalyzeEvent()", "physical event 1000, hits 511", "The energy of the physical event is used when available", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddHit(CreateHit(511.0));
    MPhotoEvent P;
    P.SetEnergy(500.0);
    E.SetPhysicalEvent(&P);
    Passed = EvaluateFalse("AnalyzeEvent()", "physical event 500", "A physical event with 500 keV passes", IsFilteredOut(F, E)) && Passed;
  }

  // Strip hits only: one 511 keV interaction is measured on both detector sides
  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, true, 511.0));
    E.AddStripHit(CreateStripHit(0, false, 511.0));
    Passed = ExpectedToFail(EvaluateFalse("AnalyzeEvent()", "strip hits 511 keV on LV and HV", "A 511 keV interaction seen on both sides is not counted twice", IsFilteredOut(F, E))) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, true, 100.0));
    E.AddStripHit(CreateStripHit(0, false, 100.0));
    Passed = EvaluateTrue("AnalyzeEvent()", "strip hits 100 keV on LV and HV", "A 100 keV interaction is filtered out", IsFilteredOut(F, E)) && Passed;
  }

  // Empty event: zero energy is below the minimum
  {
    MReadOutAssembly E;
    Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "empty event", "An empty event has zero energy and is filtered out by a 400 keV minimum", IsFilteredOut(F, E))) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestAnalyzeEventStripCounts()
{
  bool Passed = true;

  MModuleEventFilter F;
  F.SetMinimumLVStrips(1);
  F.SetMaximumLVStrips(2);
  F.SetMinimumHVStrips(1);
  F.SetMaximumHVStrips(2);

  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, true, 100.0));
    E.AddStripHit(CreateStripHit(0, false, 100.0));
    Passed = EvaluateFalse("AnalyzeEvent()", "1 LV, 1 HV", "One LV and one HV strip pass the 1-2 strip window", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, true, 100.0));
    E.AddStripHit(CreateStripHit(0, true, 100.0));
    E.AddStripHit(CreateStripHit(0, false, 100.0));
    E.AddStripHit(CreateStripHit(0, false, 100.0));
    Passed = EvaluateFalse("AnalyzeEvent()", "2 LV, 2 HV", "The maximum strip counts are inclusive", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, true, 100.0));
    Passed = EvaluateTrue("AnalyzeEvent()", "1 LV, 0 HV", "Missing HV strips filter the event", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, false, 100.0));
    Passed = EvaluateTrue("AnalyzeEvent()", "0 LV, 1 HV", "Missing LV strips filter the event", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    for (unsigned int i = 0; i < 3; ++i) E.AddStripHit(CreateStripHit(0, true, 100.0));
    E.AddStripHit(CreateStripHit(0, false, 100.0));
    Passed = EvaluateTrue("AnalyzeEvent()", "3 LV, 1 HV", "Too many LV strips filter the event", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(0, true, 100.0));
    for (unsigned int i = 0; i < 3; ++i) E.AddStripHit(CreateStripHit(0, false, 100.0));
    Passed = EvaluateTrue("AnalyzeEvent()", "1 LV, 3 HV", "Too many HV strips filter the event", IsFilteredOut(F, E)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestAnalyzeEventHitCount()
{
  bool Passed = true;

  MModuleEventFilter F;
  F.SetMinimumHits(2);
  F.SetMaximumHits(3);

  {
    MReadOutAssembly E;
    E.AddHit(CreateHit(100.0));
    Passed = EvaluateTrue("AnalyzeEvent()", "1 hit", "One hit is below the minimum of 2", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddHit(CreateHit(100.0));
    E.AddHit(CreateHit(100.0));
    Passed = EvaluateFalse("AnalyzeEvent()", "2 hits", "Two hits pass (inclusive minimum)", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    for (unsigned int i = 0; i < 3; ++i) E.AddHit(CreateHit(100.0));
    Passed = EvaluateFalse("AnalyzeEvent()", "3 hits", "Three hits pass (inclusive maximum)", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    for (unsigned int i = 0; i < 4; ++i) E.AddHit(CreateHit(100.0));
    Passed = EvaluateTrue("AnalyzeEvent()", "4 hits", "Four hits are above the maximum of 3", IsFilteredOut(F, E)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestAnalyzeEventChiSquare()
{
  bool Passed = true;

  MModuleEventFilter F;
  F.SetMinimumStripPairingReducedChiSquare(0.5);
  F.SetMaximumStripPairingReducedChiSquare(10.0);

  {
    MReadOutAssembly E;
    Passed = EvaluateFalse("AnalyzeEvent()", "no chi square", "Events without reduced chi squares are not filtered by the chi square cut", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.SetStripPairingReducedChiSquare(1.0);
    E.SetStripPairingReducedChiSquare(9.0);
    Passed = EvaluateFalse("AnalyzeEvent()", "1 and 9", "All chi squares within the window pass", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.SetStripPairingReducedChiSquare(1.0);
    E.SetStripPairingReducedChiSquare(11.0);
    Passed = EvaluateTrue("AnalyzeEvent()", "1 and 11", "One chi square above the maximum filters the event", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.SetStripPairingReducedChiSquare(0.25);
    E.SetStripPairingReducedChiSquare(5.0);
    Passed = EvaluateTrue("AnalyzeEvent()", "0.25 and 5", "One chi square below the minimum filters the event", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.SetStripPairingReducedChiSquare(10.0);
    Passed = EvaluateFalse("AnalyzeEvent()", "10", "The maximum chi square is inclusive", IsFilteredOut(F, E)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestAnalyzeEventDetectorList()
{
  bool Passed = true;

  MModuleEventFilter F;
  F.SetDetectorList({ 1, 2 });

  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(1, true, 100.0));
    E.AddStripHit(CreateStripHit(1, false, 100.0));
    Passed = EvaluateFalse("AnalyzeEvent()", "detector 1 with list 1, 2", "An event in an accepted detector passes", IsFilteredOut(F, E)) && Passed;
  }
  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(5, true, 100.0));
    E.AddStripHit(CreateStripHit(5, false, 100.0));
    Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "detector 5 with list 1, 2", "An event only in a detector not on the list is filtered out", IsFilteredOut(F, E))) && Passed;
  }

  MModuleEventFilter All;
  {
    MReadOutAssembly E;
    E.AddStripHit(CreateStripHit(5, true, 100.0));
    E.AddStripHit(CreateStripHit(5, false, 100.0));
    Passed = EvaluateFalse("AnalyzeEvent()", "detector 5 with empty list", "An empty detector list accepts all detectors", IsFilteredOut(All, E)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleEventFilter::TestLifecycle()
{
  bool Passed = true;

  MModuleEventFilter F;
  Passed = EvaluateTrue("Initialize()", "default", "Initialize returns true", F.Initialize()) && Passed;

  MReadOutAssembly E;
  E.AddStripHit(CreateStripHit(0, true, 100.0));
  E.AddStripHit(CreateStripHit(0, false, 100.0));
  Passed = EvaluateTrue("AnalyzeEvent()", "good event", "AnalyzeEvent returns true", F.AnalyzeEvent(&E)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "good event", "AnalyzeEvent sets the c_EventFilter analysis progress", E.HasAnalysisProgress(MAssembly::c_EventFilter)) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "good event", "A good event is not filtered out", E.IsFilteredOut()) && Passed;

  // An already filtered-out event is not reset by the filter
  MModuleEventFilter G;
  MReadOutAssembly Filtered;
  Filtered.AddStripHit(CreateStripHit(0, true, 100.0));
  Filtered.AddStripHit(CreateStripHit(0, false, 100.0));
  Filtered.SetFilteredOut(true);
  Passed = EvaluateTrue("AnalyzeEvent()", "previously filtered event", "AnalyzeEvent returns true", G.AnalyzeEvent(&Filtered)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "previously filtered event", "AnalyzeEvent does not clear a previous filtered-out flag", Filtered.IsFilteredOut()) && Passed;

  F.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleEventFilter Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleEventFilter.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
