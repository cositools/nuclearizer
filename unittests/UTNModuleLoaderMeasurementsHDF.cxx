/*
 * UTNModuleLoaderMeasurementsHDF.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MFile.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MModuleLoaderMeasurementsHDF.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MModuleLoaderMeasurementsHDF
class UTNModuleLoaderMeasurementsHDF : public MUnitTest
{
public:
  UTNModuleLoaderMeasurementsHDF() : MUnitTest("UTNModuleLoaderMeasurementsHDF") {}
  virtual ~UTNModuleLoaderMeasurementsHDF() {}

  virtual bool Run();

private:
  //! Test getter and setter methods
  bool TestGettersSetters();
  //! Test Initialize() fails when the HDF5 file does not exist
  bool TestInitializeMissingHDF5();
  //! Test Initialize() fails when the strip map file does not exist
  bool TestInitializeMissingStripMap();
  //! Test XML configuration round-trip
  bool TestXmlRoundTrip();
  //! End-to-end test using the committed test data
  bool TestEndToEnd();
  //! Test HDF v2 event boundaries using committed data
  bool TestV2EventBoundaries();

  //! Return the path to a nuclearizer test data directory, or "" if NUCLEARIZER is not set
  MString GetTestDataDirectory(const MString& DataSet) const;
};


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleLoaderMeasurementsHDF::GetTestDataDirectory(const MString& DataSet) const
{
  const char* NuclearizerEnv = getenv("NUCLEARIZER");
  if (NuclearizerEnv == nullptr || NuclearizerEnv[0] == '\0') return "";
  return MString(NuclearizerEnv) + "/resource/unittestdata/" + DataSet;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsHDF::Run()
{
  bool Passed = true;

  Passed = TestGettersSetters() && Passed;
  Passed = TestInitializeMissingHDF5() && Passed;
  Passed = TestInitializeMissingStripMap() && Passed;
  Passed = TestXmlRoundTrip() && Passed;
  Passed = TestEndToEnd() && Passed;
  Passed = TestV2EventBoundaries() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI; it cannot be exercised in a
  // headless unit test and is intentionally left uncovered.

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsHDF::TestGettersSetters()
{
  bool Passed = true;

  MModuleLoaderMeasurementsHDF Loader;

  Loader.SetFileName("mydata.hdf5");
  Passed = Evaluate("SetFileName/GetFileName", "representative file name", "GetFileName returns the value passed to SetFileName",
                    Loader.GetFileName(), MString("mydata.hdf5")) && Passed;

  Loader.SetFileNameStripMap("mymap.map");
  Passed = Evaluate("SetFileNameStripMap/GetFileNameStripMap", "representative strip map name", "GetFileNameStripMap returns the value passed to SetFileNameStripMap",
                    Loader.GetFileNameStripMap(), MString("mymap.map")) && Passed;

  Loader.SetLoadContinuationFiles(true);
  Passed = EvaluateTrue("SetLoadContinuationFiles/GetLoadContinuationFiles", "representative true", "GetLoadContinuationFiles returns true after SetLoadContinuationFiles(true)",
                        Loader.GetLoadContinuationFiles() == true) && Passed;

  Loader.SetLoadContinuationFiles(false);
  Passed = EvaluateTrue("SetLoadContinuationFiles/GetLoadContinuationFiles", "representative false", "GetLoadContinuationFiles returns false after SetLoadContinuationFiles(false)",
                        Loader.GetLoadContinuationFiles() == false) && Passed;

  Loader.SetIncludeNearestNeighbor(true);
  Passed = EvaluateTrue("SetIncludeNearestNeighbor/GetIncludeNearestNeighbor", "representative true", "GetIncludeNearestNeighbor returns true after SetIncludeNearestNeighbor(true)",
                        Loader.GetIncludeNearestNeighbor() == true) && Passed;

  Loader.SetIncludeNearestNeighbor(false);
  Passed = EvaluateTrue("SetIncludeNearestNeighbor/GetIncludeNearestNeighbor", "representative false", "GetIncludeNearestNeighbor returns false after SetIncludeNearestNeighbor(false)",
                        Loader.GetIncludeNearestNeighbor() == false) && Passed;

  // Clone() returns a new, distinct instance
  MModuleLoaderMeasurementsHDF* Cloned = Loader.Clone();
  Passed = EvaluateTrue("Clone()", "new instance", "Clone() returns a non-null pointer",
                        Cloned != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "distinct instance", "Clone() returns an object distinct from the original",
                        Cloned != &Loader) && Passed;
  delete Cloned;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsHDF::TestInitializeMissingHDF5()
{
  bool Passed = true;

  const MString TestDataDir = GetTestDataDirectory("542-1");
  if (TestDataDir.IsEmpty()) {
    mout<<"UTNModuleLoaderMeasurementsHDF::TestInitializeMissingHDF5: NUCLEARIZER environment variable is not set - skipping test"<<endl;
    return Passed;
  }

  MModuleLoaderMeasurementsHDF Loader;
  Loader.SetFileName("/this/file/does/not/exist.hdf5");
  Loader.SetFileNameStripMap(TestDataDir + "/hp52542-1.stripmap.map");
  Loader.SetLoadContinuationFiles(false);
  Loader.SetIncludeNearestNeighbor(false);

  DisableDefaultStreams();
  bool Result = Loader.Initialize();
  EnableDefaultStreams();

  Passed = EvaluateFalse("Initialize()", "missing HDF5 file", "Initialize() returns false when the HDF5 file does not exist",
                         Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsHDF::TestInitializeMissingStripMap()
{
  bool Passed = true;

  const MString TestDataDir = GetTestDataDirectory("542-1");
  if (TestDataDir.IsEmpty()) {
    mout<<"UTNModuleLoaderMeasurementsHDF::TestInitializeMissingStripMap: NUCLEARIZER environment variable is not set - skipping test"<<endl;
    return Passed;
  }

  MModuleLoaderMeasurementsHDF Loader;
  Loader.SetFileName(TestDataDir + "/hp52542-1.gse_20251016T191639.hdf5");
  Loader.SetFileNameStripMap("/this/file/does/not/exist.map");
  Loader.SetLoadContinuationFiles(false);
  Loader.SetIncludeNearestNeighbor(false);

  DisableDefaultStreams();
  bool Result = Loader.Initialize();
  EnableDefaultStreams();

  Passed = EvaluateFalse("Initialize()", "missing strip map", "Initialize() returns false when the strip map file does not exist",
                         Result) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsHDF::TestXmlRoundTrip()
{
  bool Passed = true;

  // Set up a loader with representative values
  MModuleLoaderMeasurementsHDF Writer;
  Writer.SetFileName("/path/to/data.hdf5");
  Writer.SetFileNameStripMap("/path/to/strip.map");
  Writer.SetLoadContinuationFiles(true);
  Writer.SetIncludeNearestNeighbor(true);

  // Serialise
  MXmlNode* Node = Writer.CreateXmlConfiguration();
  Passed = EvaluateTrue("CreateXmlConfiguration()", "representative values", "CreateXmlConfiguration() returns a non-null node for representative settings",
                        Node != nullptr) && Passed;
  if (Node == nullptr) return false;

  // Deserialise into a fresh instance
  MModuleLoaderMeasurementsHDF Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "representative values", "ReadXmlConfiguration() returns true for a valid configuration node",
                        Reader.ReadXmlConfiguration(Node)) && Passed;

  Passed = Evaluate("ReadXmlConfiguration()", "FileNameHDF5", "ReadXmlConfiguration() restores the representative HDF5 file name",
                    Reader.GetFileName(), MString("/path/to/data.hdf5")) && Passed;

  Passed = Evaluate("ReadXmlConfiguration()", "FileNameStripMap", "ReadXmlConfiguration() restores the representative strip map file name",
                    Reader.GetFileNameStripMap(), MString("/path/to/strip.map")) && Passed;

  Passed = EvaluateTrue("ReadXmlConfiguration()", "LoadContinuationFiles", "ReadXmlConfiguration() restores the representative LoadContinuationFiles flag",
                        Reader.GetLoadContinuationFiles() == true) && Passed;

  Passed = EvaluateTrue("ReadXmlConfiguration()", "IncludeNearestNeighbor", "ReadXmlConfiguration() restores the representative IncludeNearestNeighbor flag",
                        Reader.GetIncludeNearestNeighbor() == true) && Passed;

  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsHDF::TestEndToEnd()
{
  bool Passed = true;

  const MString TestDataDir = GetTestDataDirectory("542-1");
  if (TestDataDir.IsEmpty()) {
    mout<<"UTNModuleLoaderMeasurementsHDF::TestEndToEnd: NUCLEARIZER environment variable is not set - skipping test"<<endl;
    return Passed;
  }

  const MString HDF5File    = TestDataDir + "/hp52542-1.gse_20251016T191639.hdf5";
  const MString StripMapFile = TestDataDir + "/hp52542-1.stripmap.map";

  Passed = EvaluateTrue("MFile::Exists()", "HDF5 fixture", "The representative HDF5 test fixture exists on disk",
                        MFile::Exists(HDF5File)) && Passed;
  Passed = EvaluateTrue("MFile::Exists()", "strip map fixture", "The representative strip map test fixture exists on disk",
                        MFile::Exists(StripMapFile)) && Passed;
  if (MFile::Exists(HDF5File) == false || MFile::Exists(StripMapFile) == false) return false;

  MModuleLoaderMeasurementsHDF Loader;
  Loader.SetFileName(HDF5File);
  Loader.SetFileNameStripMap(StripMapFile);
  Loader.SetLoadContinuationFiles(false);
  Loader.SetIncludeNearestNeighbor(false);

  Passed = EvaluateTrue("Initialize()", "representative test data", "Initialize() succeeds with the representative HDF5 file and strip map",
                        Loader.Initialize()) && Passed;
  if (Passed == false) return false;

  // ----- Event 1: 4 strip hits -----------------------------------------------
  // Reference ROA:
  //   UH 0 41 l 4053 10452 4
  //   UH 0 42 l 1727 10539 4
  //   UH 0 49 h 1780 10251 4
  //   UH 0 50 h 4203 10105 4

  MReadOutAssembly* Event1 = new MReadOutAssembly();
  Passed = EvaluateTrue("AnalyzeEvent()", "event 1 return value", "AnalyzeEvent() returns true for the first event",
                        Loader.AnalyzeEvent(Event1)) && Passed;

  Passed = Evaluate("AnalyzeEvent()", "event 1 ID", "The first event has the representative ID 1",
                    (unsigned long) Event1->GetID(), (unsigned long) 1) && Passed;

  Passed = Evaluate("AnalyzeEvent()", "event 1 hit count", "The first event has the representative 4 strip hits",
                    (unsigned int) Event1->GetNStripHits(), (unsigned int) 4) && Passed;

  if (Event1->GetNStripHits() == 4) {
    // Hit 0: strip 41 low-voltage, ADC=4053, TAC=10452
    MStripHit* H0 = Event1->GetStripHit(0);
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 0 detector", "Event 1 hit 0 has the representative detector ID 0",
                      H0->GetDetectorID(), 0u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 0 strip", "Event 1 hit 0 has the representative strip ID 41",
                      H0->GetStripID(), 41u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 1 hit 0 is LV", "Event 1 hit 0 is a representative low-voltage strip",
                          H0->IsLowVoltageStrip() == true) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 0 ADC", "Event 1 hit 0 has the representative ADC value 4053",
                          H0->GetADCUnits(), 4053.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 0 TAC", "Event 1 hit 0 has the representative TAC value 10452",
                          H0->GetTAC(), 10452.0, 0.5) && Passed;

    // Hit 1: strip 42 low-voltage, ADC=1727, TAC=10539
    MStripHit* H1 = Event1->GetStripHit(1);
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 1 detector", "Event 1 hit 1 has the representative detector ID 0",
                      H1->GetDetectorID(), 0u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 1 strip", "Event 1 hit 1 has the representative strip ID 42",
                      H1->GetStripID(), 42u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 1 hit 1 is LV", "Event 1 hit 1 is a representative low-voltage strip",
                          H1->IsLowVoltageStrip() == true) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 1 ADC", "Event 1 hit 1 has the representative ADC value 1727",
                          H1->GetADCUnits(), 1727.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 1 TAC", "Event 1 hit 1 has the representative TAC value 10539",
                          H1->GetTAC(), 10539.0, 0.5) && Passed;

    // Hit 2: strip 49 high-voltage, ADC=1780, TAC=10251
    MStripHit* H2 = Event1->GetStripHit(2);
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 2 detector", "Event 1 hit 2 has the representative detector ID 0",
                      H2->GetDetectorID(), 0u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 2 strip", "Event 1 hit 2 has the representative strip ID 49",
                      H2->GetStripID(), 49u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "event 1 hit 2 is HV", "Event 1 hit 2 is a representative high-voltage strip",
                           H2->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 2 ADC", "Event 1 hit 2 has the representative ADC value 1780",
                          H2->GetADCUnits(), 1780.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 2 TAC", "Event 1 hit 2 has the representative TAC value 10251",
                          H2->GetTAC(), 10251.0, 0.5) && Passed;

    // Hit 3: strip 50 high-voltage, ADC=4203, TAC=10105
    MStripHit* H3 = Event1->GetStripHit(3);
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 3 detector", "Event 1 hit 3 has the representative detector ID 0",
                      H3->GetDetectorID(), 0u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1 hit 3 strip", "Event 1 hit 3 has the representative strip ID 50",
                      H3->GetStripID(), 50u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "event 1 hit 3 is HV", "Event 1 hit 3 is a representative high-voltage strip",
                           H3->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 3 ADC", "Event 1 hit 3 has the representative ADC value 4203",
                          H3->GetADCUnits(), 4203.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 3 TAC", "Event 1 hit 3 has the representative TAC value 10105",
                          H3->GetTAC(), 10105.0, 0.5) && Passed;
  }
  delete Event1;

  // ----- Event 2: 2 strip hits -----------------------------------------------
  // Reference ROA:
  //   UH 0 53 l 1685 4653 0
  //   UH 0 12 h 1651 11605 4

  MReadOutAssembly* Event2 = new MReadOutAssembly();
  Passed = EvaluateTrue("AnalyzeEvent()", "event 2 return value", "AnalyzeEvent() returns true for the second event",
                        Loader.AnalyzeEvent(Event2)) && Passed;

  Passed = Evaluate("AnalyzeEvent()", "event 2 ID", "The second event has the representative ID 2",
                    (unsigned long) Event2->GetID(), (unsigned long) 2) && Passed;

  Passed = Evaluate("AnalyzeEvent()", "event 2 hit count", "The second event has the representative 2 strip hits",
                    (unsigned int) Event2->GetNStripHits(), (unsigned int) 2) && Passed;

  if (Event2->GetNStripHits() == 2) {
    // Hit 0: strip 53 low-voltage, ADC=1685, TAC=4653
    MStripHit* H0 = Event2->GetStripHit(0);
    Passed = Evaluate("AnalyzeEvent()", "event 2 hit 0 strip", "Event 2 hit 0 has the representative strip ID 53",
                      H0->GetStripID(), 53u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 2 hit 0 is LV", "Event 2 hit 0 is a representative low-voltage strip",
                          H0->IsLowVoltageStrip() == true) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 2 hit 0 ADC", "Event 2 hit 0 has the representative ADC value 1685",
                          H0->GetADCUnits(), 1685.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 2 hit 0 TAC", "Event 2 hit 0 has the representative TAC value 4653",
                          H0->GetTAC(), 4653.0, 0.5) && Passed;

    // Hit 1: strip 12 high-voltage, ADC=1651, TAC=11605
    MStripHit* H1 = Event2->GetStripHit(1);
    Passed = Evaluate("AnalyzeEvent()", "event 2 hit 1 strip", "Event 2 hit 1 has the representative strip ID 12",
                      H1->GetStripID(), 12u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "event 2 hit 1 is HV", "Event 2 hit 1 is a representative high-voltage strip",
                           H1->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 2 hit 1 ADC", "Event 2 hit 1 has the representative ADC value 1651",
                          H1->GetADCUnits(), 1651.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 2 hit 1 TAC", "Event 2 hit 1 has the representative TAC value 11605",
                          H1->GetTAC(), 11605.0, 0.5) && Passed;
  }
  delete Event2;

  // ----- Event 3: 2 guard-ring strip hits ------------------------------------
  // Reference ROA:
  //   UH 0 64 l 1646 0 5
  //   UH 0 64 h 1829 0 5

  MReadOutAssembly* Event3 = new MReadOutAssembly();
  Passed = EvaluateTrue("AnalyzeEvent()", "event 3 return value", "AnalyzeEvent() returns true for the third event",
                        Loader.AnalyzeEvent(Event3)) && Passed;

  Passed = Evaluate("AnalyzeEvent()", "event 3 ID", "The third AnalyzeEvent() call produces the representative event ID 3",
                    (unsigned long) Event3->GetID(), (unsigned long) 3) && Passed;

  Passed = Evaluate("AnalyzeEvent()", "event 3 hit count", "Event ID 3 has the representative 2 strip hits",
                    (unsigned int) Event3->GetNStripHits(), (unsigned int) 2) && Passed;

  if (Event3->GetNStripHits() == 2) {
    // Hit 0: strip 64 low-voltage guard ring, ADC=1646, TAC=0
    MStripHit* H0 = Event3->GetStripHit(0);
    Passed = Evaluate("AnalyzeEvent()", "event 3 hit 0 strip", "Event 3 hit 0 has the representative strip ID 64",
                      H0->GetStripID(), 64u) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 3 hit 0 is LV", "Event 3 hit 0 is a representative low-voltage strip",
                          H0->IsLowVoltageStrip() == true) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 3 hit 0 is guard ring", "Event 3 hit 0 is a representative guard-ring strip",
                          H0->IsGuardRing() == true) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 3 hit 0 ADC", "Event 3 hit 0 has the representative ADC value 1646",
                          H0->GetADCUnits(), 1646.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 3 hit 0 TAC", "Event 3 hit 0 has the representative TAC value 0",
                          H0->GetTAC(), 0.0, 0.5) && Passed;

    // Hit 1: strip 64 high-voltage guard ring, ADC=1829, TAC=0
    MStripHit* H1 = Event3->GetStripHit(1);
    Passed = Evaluate("AnalyzeEvent()", "event 3 hit 1 strip", "Event 3 hit 1 has the representative strip ID 64",
                      H1->GetStripID(), 64u) && Passed;
    Passed = EvaluateFalse("AnalyzeEvent()", "event 3 hit 1 is HV", "Event 3 hit 1 is a representative high-voltage strip",
                           H1->IsLowVoltageStrip()) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 3 hit 1 is guard ring", "Event 3 hit 1 is a representative guard-ring strip",
                          H1->IsGuardRing() == true) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 3 hit 1 ADC", "Event 3 hit 1 has the representative ADC value 1829",
                          H1->GetADCUnits(), 1829.0, 0.5) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 3 hit 1 TAC", "Event 3 hit 1 has the representative TAC value 0",
                          H1->GetTAC(), 0.0, 0.5) && Passed;
  }
  delete Event3;

  Loader.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsHDF::TestV2EventBoundaries()
{
  bool Passed = true;

  const MString TestDataDir = GetTestDataDirectory("406-1");
  if (TestDataDir.IsEmpty()) {
    mout<<"UTNModuleLoaderMeasurementsHDF::TestV2EventBoundaries: NUCLEARIZER environment variable is not set - skipping test"<<endl;
    return Passed;
  }

  const MString HDF5File = TestDataDir + "/hp52406-1.gse_20260217T124025.hdf5";
  const MString StripMapFile = TestDataDir + "/hp52406-1.stripmap.map";

  Passed = EvaluateTrue("MFile::Exists()", "HDF v2 fixture", "The representative HDF v2 fixture exists on disk",
                        MFile::Exists(HDF5File)) && Passed;
  Passed = EvaluateTrue("MFile::Exists()", "HDF v2 strip map", "The representative HDF v2 strip map exists on disk",
                        MFile::Exists(StripMapFile)) && Passed;
  if (MFile::Exists(HDF5File) == false || MFile::Exists(StripMapFile) == false) return false;

  MModuleLoaderMeasurementsHDF Loader;
  Loader.SetFileName(HDF5File);
  Loader.SetFileNameStripMap(StripMapFile);
  Loader.SetLoadContinuationFiles(false);
  Loader.SetIncludeNearestNeighbor(true);

  Passed = EvaluateTrue("Initialize()", "HDF v2 event-boundary data", "Initialize() succeeds with the representative HDF v2 file and strip map",
                        Loader.Initialize()) && Passed;
  if (Passed == false) return false;

  const unsigned long ExpectedIDs[3] = { 1, 2, 3 };
  const unsigned int ExpectedHitCountsWithNN[3] = { 1, 7, 6 };
  const double ExpectedTimes[3] = {
    1771360826.007614416,
    1771360826.010675750,
    1771360826.011540033
  };

  for (unsigned int e = 0; e < 3; ++e) {
    MReadOutAssembly* Event = new MReadOutAssembly();
    Passed = EvaluateTrue("AnalyzeEvent()", MString("HDF v2 event ") + (e+1),
                          "AnalyzeEvent() returns true for the next HDF v2 event",
                          Loader.AnalyzeEvent(Event)) && Passed;

    Passed = Evaluate("AnalyzeEvent()", MString("HDF v2 event ") + (e+1) + " ID",
                      "AnalyzeEvent() keeps one HDF /Events row as one MReadOutAssembly with the expected event ID",
                      (unsigned long) Event->GetID(), ExpectedIDs[e]) && Passed;

    Passed = Evaluate("AnalyzeEvent()", MString("HDF v2 event ") + (e+1) + " hit count",
                      "AnalyzeEvent() keeps one HDF /Events row as one MReadOutAssembly with the expected strip-hit count",
                      (unsigned int) Event->GetNStripHits(), ExpectedHitCountsWithNN[e]) && Passed;

    Passed = EvaluateNear("AnalyzeEvent()", MString("HDF v2 event ") + (e+1) + " UTC time",
                          "AnalyzeEvent() converts the HDF v2 SPW time code into the expected UTC event time",
                          Event->GetTimeUTC().GetAsSeconds(), ExpectedTimes[e], 1.0e-6) && Passed;

    if (e == 0 && Event->GetNStripHits() == 1) {
      MStripHit* Hit = Event->GetStripHit(0);
      Passed = Evaluate("AnalyzeEvent()", "HDF v2 event 1 guard detector",
                        "The first V2 FEE hit is decoded through the strip map into detector 0",
                        Hit->GetDetectorID(), 0u) && Passed;
      Passed = Evaluate("AnalyzeEvent()", "HDF v2 event 1 guard strip",
                        "The first V2 FEE hit is decoded through the strip map into strip 64",
                        Hit->GetStripID(), 64u) && Passed;
      Passed = EvaluateFalse("AnalyzeEvent()", "HDF v2 event 1 guard side",
                             "The first V2 FEE hit is decoded as a high-voltage strip",
                             Hit->IsLowVoltageStrip()) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "HDF v2 event 1 guard ADC",
                            "The first V2 FEE hit keeps the representative ADC value",
                            Hit->GetADCUnits(), 1738.0, 0.5) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "HDF v2 event 1 guard TAC",
                            "The first V2 FEE hit keeps the representative timing value",
                            Hit->GetTAC(), 0.0, 0.5) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", "HDF v2 event 1 guard flag",
                            "The V2 hit_type 2 value is decoded as a guard-ring hit",
                            Hit->IsGuardRing()) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", "HDF v2 event 1 guard veto",
                            "A V2 guard-ring hit sets the read-out assembly guard-ring veto",
                            Event->GetGuardRingVeto()) && Passed;
      Passed = EvaluateFalse("AnalyzeEvent()", "HDF v2 event 1 nearest-neighbor flag",
                             "A V2 guard-ring hit is not decoded as a nearest-neighbor hit",
                             Hit->IsNearestNeighbor()) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", "HDF v2 event 1 fast timing flag",
                            "The V2 timing_type 1 value is decoded as fast timing",
                            Hit->HasFastTiming()) && Passed;
    }

    if (e == 1 && Event->GetNStripHits() == 7) {
      MStripHit* NearestNeighbor = Event->GetStripHit(0);
      Passed = Evaluate("AnalyzeEvent()", "HDF v2 event 2 nearest-neighbor strip",
                        "The first event-2 V2 FEE hit is decoded through the strip map into strip 16",
                        NearestNeighbor->GetStripID(), 16u) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", "HDF v2 event 2 nearest-neighbor side",
                            "The first event-2 V2 FEE hit is decoded as a low-voltage strip",
                            NearestNeighbor->IsLowVoltageStrip()) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "HDF v2 event 2 nearest-neighbor ADC",
                            "The first event-2 V2 FEE hit keeps the representative ADC value",
                            NearestNeighbor->GetADCUnits(), 1538.0, 0.5) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "HDF v2 event 2 nearest-neighbor TAC",
                            "The first event-2 V2 FEE hit keeps the representative timing value",
                            NearestNeighbor->GetTAC(), 8424.0, 0.5) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", "HDF v2 event 2 nearest-neighbor flag",
                            "The V2 hit_type 1 value is decoded as a nearest-neighbor hit",
                            NearestNeighbor->IsNearestNeighbor()) && Passed;
      Passed = EvaluateFalse("AnalyzeEvent()", "HDF v2 event 2 nearest-neighbor fast timing",
                             "The V2 timing_type 0 value is decoded as non-fast timing",
                             NearestNeighbor->HasFastTiming()) && Passed;

      MStripHit* FastTiming = Event->GetStripHit(5);
      Passed = Evaluate("AnalyzeEvent()", "HDF v2 event 2 fast strip",
                        "The representative event-2 fast-timing hit is decoded through the strip map into strip 60",
                        FastTiming->GetStripID(), 60u) && Passed;
      Passed = EvaluateFalse("AnalyzeEvent()", "HDF v2 event 2 fast side",
                             "The representative event-2 fast-timing hit is decoded as a high-voltage strip",
                             FastTiming->IsLowVoltageStrip()) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "HDF v2 event 2 fast ADC",
                            "The representative event-2 fast-timing hit keeps the ADC value",
                            FastTiming->GetADCUnits(), 2132.0, 0.5) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "HDF v2 event 2 fast TAC",
                            "The representative event-2 fast-timing hit keeps the timing value",
                            FastTiming->GetTAC(), 10993.0, 0.5) && Passed;
      Passed = EvaluateFalse("AnalyzeEvent()", "HDF v2 event 2 fast nearest-neighbor flag",
                             "The representative event-2 fast-timing hit is not a nearest-neighbor hit",
                             FastTiming->IsNearestNeighbor()) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", "HDF v2 event 2 fast timing flag",
                            "The representative event-2 timing_type 1 value is decoded as fast timing",
                            FastTiming->HasFastTiming()) && Passed;
    }

    delete Event;
  }

  Loader.Finalize();

  MModuleLoaderMeasurementsHDF NoNearestNeighbors;
  NoNearestNeighbors.SetFileName(HDF5File);
  NoNearestNeighbors.SetFileNameStripMap(StripMapFile);
  NoNearestNeighbors.SetLoadContinuationFiles(false);
  NoNearestNeighbors.SetIncludeNearestNeighbor(false);

  Passed = EvaluateTrue("Initialize()", "HDF v2 without nearest neighbors",
                        "Initialize() succeeds with the representative HDF v2 file when nearest-neighbor hits are disabled",
                        NoNearestNeighbors.Initialize()) && Passed;
  if (Passed == false) return false;

  const unsigned int ExpectedHitCountsWithoutNN[3] = { 1, 3, 2 };

  for (unsigned int e = 0; e < 3; ++e) {
    MReadOutAssembly* Event = new MReadOutAssembly();
    Passed = EvaluateTrue("AnalyzeEvent()", MString("HDF v2 no-NN event ") + (e+1),
                          "AnalyzeEvent() returns true for the next HDF v2 event with nearest-neighbor hits disabled",
                          NoNearestNeighbors.AnalyzeEvent(Event)) && Passed;

    Passed = Evaluate("AnalyzeEvent()", MString("HDF v2 no-NN event ") + (e+1) + " ID",
                      "Disabling nearest-neighbor hits does not merge or skip the HDF v2 event boundary",
                      (unsigned long) Event->GetID(), ExpectedIDs[e]) && Passed;

    Passed = Evaluate("AnalyzeEvent()", MString("HDF v2 no-NN event ") + (e+1) + " hit count",
                      "Disabling nearest-neighbor hits removes only the V2 hit_type 1 entries from the current event",
                      (unsigned int) Event->GetNStripHits(), ExpectedHitCountsWithoutNN[e]) && Passed;

    for (unsigned int h = 0; h < Event->GetNStripHits(); ++h) {
      Passed = EvaluateFalse("AnalyzeEvent()", MString("HDF v2 no-NN event ") + (e+1) + " hit nearest-neighbor flag",
                             "No retained strip hit is marked as a nearest-neighbor hit when nearest-neighbor hits are disabled",
                             Event->GetStripHit(h)->IsNearestNeighbor()) && Passed;
    }

    delete Event;
  }

  NoNearestNeighbors.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  if (MGlobal::Initialize("UTNModuleLoaderMeasurementsHDF", "Unit tests for MModuleLoaderMeasurementsHDF") == false) return 1;

  UTNModuleLoaderMeasurementsHDF Test;
  return Test.Run() == true ? 0 : 1;
}
