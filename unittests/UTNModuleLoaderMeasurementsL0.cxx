/*
 * UTNModuleLoaderMeasurementsL0.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdint>
#include <fstream>
#include <iterator>
#include <vector>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MTime.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleLoaderMeasurementsL0.h"
#include "MModuleSaverMeasurementsL0.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MModuleLoaderMeasurementsL0
class UTNModuleLoaderMeasurementsL0 : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleLoaderMeasurementsL0() : MUnitTest("UTNModuleLoaderMeasurementsL0") {}
  //! Default destructor
  virtual ~UTNModuleLoaderMeasurementsL0() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data
  bool TestDefaultConstruction();
  //! Test the setters/getters and the XML round trip
  bool TestConfiguration();
  //! Test the Initialize() error paths
  bool TestInitializeErrors();
  //! Test a round trip with the L0 saver
  bool TestRoundTrip();
  //! Test files with the 20-byte L0 file header
  bool TestFileHeader();
  //! Test hand-crafted packets: test pulser hits, unknown read-out IDs, wrong APIDs, truncation
  bool TestHandCraftedPackets();
  //! Test re-initialization
  bool TestReinitialize();

  //! Write the strip map and return its name, or an empty string on failure
  MString WriteStripMap();
  //! Write events with the L0 saver, return the file name
  MString WriteWithSaver(const MString& Name, vector<MReadOutAssembly*>& Events);
  //! Write raw bytes to a file
  bool WriteBinary(const MString& FileName, const vector<uint8_t>& Data);
  //! Read a binary file
  vector<uint8_t> ReadBinary(const MString& FileName);
  //! Create a packet with the given APID and hit data
  vector<uint8_t> CreatePacket(uint16_t APID, unsigned int NHits, const vector<uint8_t>& HitData, uint8_t EventType = 0x02);
  //! Read all events from a file with the loader, the events have to be deleted by the caller
  vector<MReadOutAssembly*> ReadAll(const MString& FileName, const MString& StripMap);
  //! Add a strip hit to the event
  MStripHit* AddStripHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, int ADC, int TAC, bool FastTiming = true);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsL0::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestConfiguration() && Passed;
  Passed = TestInitializeErrors() && Passed;
  Passed = TestRoundTrip() && Passed;
  Passed = TestFileHeader() && Passed;
  Passed = TestHandCraftedPackets() && Passed;
  Passed = TestReinitialize() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleLoaderMeasurementsL0::WriteStripMap()
{
  // Detector 0: LV strips 0-9 = ROI 100-109, HV strips 0-9 = ROI 200-209, LV guard ring strip 64 = ROI 300
  // Detector 1: LV strips 0-9 = ROI 400-409, HV strips 0-9 = ROI 500-509
  MString C = "# Columns are strip_id, detector, side (0=LV, 1=HV), strip\n";
  for (unsigned int s = 0; s < 10; ++s) {
    C += MString("") + (100 + s) + " 0 0 " + s + "\n";
    C += MString("") + (200 + s) + " 0 1 " + s + "\n";
    C += MString("") + (400 + s) + " 1 0 " + s + "\n";
    C += MString("") + (500 + s) + " 1 1 " + s + "\n";
  }
  C += "300 0 0 64\n";

  MString FileName = GetTemporaryFileName("l0.stripmap.map");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleLoaderMeasurementsL0::WriteWithSaver(const MString& Name, vector<MReadOutAssembly*>& Events)
{
  MString FileName = GetTemporaryFileName(Name);
  MModuleSaverMeasurementsL0 S;
  S.SetFileName(FileName);
  S.SetFileNameStripMap(WriteStripMap());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  if (S.Initialize() == true) {
    for (MReadOutAssembly* E : Events) S.AnalyzeEvent(E);
    S.Finalize();
  }
  g_Verbosity = OldVerbosity;
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsL0::WriteBinary(const MString& FileName, const vector<uint8_t>& Data)
{
  ofstream Out(FileName.Data(), ios::binary);
  if (Out.is_open() == false) return false;
  Out.write(reinterpret_cast<const char*>(Data.data()), Data.size());
  return Out.good();
}


////////////////////////////////////////////////////////////////////////////////


vector<uint8_t> UTNModuleLoaderMeasurementsL0::ReadBinary(const MString& FileName)
{
  ifstream In(FileName.Data(), ios::binary);
  return vector<uint8_t>((istreambuf_iterator<char>(In)), istreambuf_iterator<char>());
}


////////////////////////////////////////////////////////////////////////////////


vector<uint8_t> UTNModuleLoaderMeasurementsL0::CreatePacket(uint16_t APID, unsigned int NHits, const vector<uint8_t>& HitData, uint8_t EventType)
{
  vector<uint8_t> P;
  uint16_t Word0 = 0x0800 | (APID & 0x7FF);
  uint16_t Length = 8 + 4 + 3 + HitData.size() - 1;
  P.push_back(Word0 >> 8);
  P.push_back(Word0 & 0xFF);
  P.push_back(0xC0);
  P.push_back(0x00);
  P.push_back(Length >> 8);
  P.push_back(Length & 0xFF);
  // Secondary header: 1000 s, 0 ticks
  P.push_back(0x00); P.push_back(0x00); P.push_back(0x03); P.push_back(0xE8);
  P.push_back(0x00); P.push_back(0x00); P.push_back(0x00); P.push_back(0x00);
  // Trunc time
  P.push_back(0x00); P.push_back(0x00); P.push_back(0x00); P.push_back(0x00);
  // HITS, HLEN, ETYPE
  P.push_back(((NHits & 0x3F) << 2) | ((HitData.size() >> 8) & 0x3));
  P.push_back(HitData.size() & 0xFF);
  P.push_back(EventType);
  P.insert(P.end(), HitData.begin(), HitData.end());
  return P;
}


////////////////////////////////////////////////////////////////////////////////


vector<MReadOutAssembly*> UTNModuleLoaderMeasurementsL0::ReadAll(const MString& FileName, const MString& StripMap)
{
  vector<MReadOutAssembly*> Events;
  MModuleLoaderMeasurementsL0 L;
  L.SetFileName(FileName);
  L.SetFileNameStripMap(StripMap);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  if (L.Initialize() == true) {
    for (unsigned int i = 0; i < 1000; ++i) {
      MReadOutAssembly* E = new MReadOutAssembly();
      if (L.AnalyzeEvent(E) == false) {
        delete E;
        break;
      }
      Events.push_back(E);
    }
    L.Finalize();
  }
  g_Verbosity = OldVerbosity;
  return Events;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleLoaderMeasurementsL0::AddStripHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, int ADC, int TAC, bool FastTiming)
{
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(DetID);
  SH->SetStripID(StripID);
  SH->IsLowVoltageStrip(IsLowVoltage);
  SH->SetADCUnits(ADC);
  SH->SetTAC(TAC);
  SH->HasFastTiming(FastTiming);
  Event.AddStripHit(SH);
  return SH;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsL0::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleLoaderMeasurementsL0 M;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagMeasurementLoaderL0'", M.GetXmlTag(), MString("XmlTagMeasurementLoaderL0")) && Passed;
  Passed = EvaluateFalse("GetName()", "default", "The name is not empty", M.GetName().IsEmpty()) && Passed;
  Passed = EvaluateTrue("IsStartModule()", "default", "The loader is a start module", M.IsStartModule()) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The loader provides c_EventLoader", M.ProvidesModuleType(MAssembly::c_EventLoader)) && Passed;
  Passed = Evaluate("GetFileName()", "default", "The default file name is empty", M.GetFileName(), MString("")) && Passed;
  Passed = Evaluate("GetFileNameStripMap()", "default", "The default strip map name is empty", M.GetFileNameStripMap(), MString("")) && Passed;

  MModuleLoaderMeasurementsL0* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns an object", C != nullptr) && Passed;
  if (C != nullptr) {
    Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsL0::TestConfiguration()
{
  bool Passed = true;

  MModuleLoaderMeasurementsL0 Writer;
  Writer.SetFileName("/data/in.bin");
  Writer.SetFileNameStripMap("/data/map.map");
  Passed = Evaluate("SetFileName/GetFileName", "/data/in.bin", "GetFileName returns the set name", Writer.GetFileName(), MString("/data/in.bin")) && Passed;
  Passed = Evaluate("SetFileNameStripMap/GetFileNameStripMap", "/data/map.map", "GetFileNameStripMap returns the set name", Writer.GetFileNameStripMap(), MString("/data/map.map")) && Passed;

  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleLoaderMeasurementsL0 Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "both names", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "FileName", "The file name survives", Reader.GetFileName(), MString("/data/in.bin")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "FileNameStripMap", "The strip map name survives", Reader.GetFileNameStripMap(), MString("/data/map.map")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsL0::TestInitializeErrors()
{
  bool Passed = true;

  MString Data = GetTemporaryFileName("short.bin");
  Passed = EvaluateTrue("WriteBinary()", "short.bin", "The fixture file is written", WriteBinary(Data, { 0x08, 0xDD })) && Passed;
  MString Valid = GetTemporaryFileName("valid.bin");
  Passed = EvaluateTrue("WriteBinary()", "valid.bin", "The fixture file is written", WriteBinary(Valid, CreatePacket(0x0DD, 0, {}))) && Passed;
  MString StripMap = WriteStripMap();
  Passed = EvaluateFalse("WriteStripMap()", "strip map", "The fixture file is written", StripMap.IsEmpty()) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();

  MModuleLoaderMeasurementsL0 NoName;
  NoName.SetFileNameStripMap(StripMap);
  bool NoNameResult = NoName.Initialize();

  MModuleLoaderMeasurementsL0 Missing;
  Missing.SetFileName(GetTemporaryFileName("missing.bin"));
  Missing.SetFileNameStripMap(StripMap);
  bool MissingResult = Missing.Initialize();

  MModuleLoaderMeasurementsL0 NoMap;
  NoMap.SetFileName(Valid);
  bool NoMapResult = NoMap.Initialize();

  MModuleLoaderMeasurementsL0 MissingMap;
  MissingMap.SetFileName(Valid);
  MissingMap.SetFileNameStripMap(GetTemporaryFileName("missing.map"));
  bool MissingMapResult = MissingMap.Initialize();

  MModuleLoaderMeasurementsL0 Short;
  Short.SetFileName(Data);
  Short.SetFileNameStripMap(StripMap);
  bool ShortResult = Short.Initialize();
  Short.Finalize();

  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "no file name", "Initialize returns false", NoNameResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing file", "Initialize returns false", MissingResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "no strip map name", "Initialize returns false", NoMapResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing strip map", "Initialize returns false", MissingMapResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "2-byte file", "Initialize returns false for a file shorter than 4 bytes", ShortResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsL0::TestRoundTrip()
{
  bool Passed = true;

  vector<MReadOutAssembly*> Written;
  MReadOutAssembly* E1 = new MReadOutAssembly();
  E1->SetTimeRTS(MTime(1000, 500000000));
  AddStripHit(*E1, 0, 5, true, 4053, 10452, true);
  AddStripHit(*E1, 0, 7, false, 3900, 10400, false);
  MStripHit* NN = AddStripHit(*E1, 0, 6, true, 500, 700, true);
  NN->IsNearestNeighbor(true);
  Written.push_back(E1);

  MReadOutAssembly* E2 = new MReadOutAssembly();
  E2->SetTimeRTS(MTime(1002, 250));
  AddStripHit(*E2, 1, 2, true, 100, 200);
  AddStripHit(*E2, 1, 3, false, 110, 210);
  E2->SetShieldVeto(true);
  Written.push_back(E2);

  MReadOutAssembly* E3 = new MReadOutAssembly();
  E3->SetTimeRTS(MTime(1003, 0));
  AddStripHit(*E3, 0, 1, true, 1000, 2000);
  MStripHit* GR = AddStripHit(*E3, 0, 64, true, 3000, 0, false);
  GR->IsGuardRing(true);
  E3->SetGuardRingVeto(true);
  Written.push_back(E3);

  MString File = WriteWithSaver("roundtrip.bin", Written);
  vector<MReadOutAssembly*> Read = ReadAll(File, WriteStripMap());

  Passed = Evaluate("AnalyzeEvent()", "three packets", "Three events are read", (unsigned int) Read.size(), 3u) && Passed;
  if (Read.size() >= 1) {
    MReadOutAssembly* R = Read[0];
    Passed = Evaluate("AnalyzeEvent()", "event 1", "The event ID is the packet number 1", R->GetID(), (unsigned long) 1) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "event 1", "The event loader analysis progress is set", R->HasAnalysisProgress(MAssembly::c_EventLoader)) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "event 1", "The RTS time survives within 25 ns", R->GetTimeRTS().GetAsDouble(), 1000.5, 3e-8) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 1", "Three strip hits are read", R->GetNStripHits(), 3u) && Passed;
    if (R->GetNStripHits() == 3) {
      MStripHit* S0 = R->GetStripHit(0);
      Passed = Evaluate("AnalyzeEvent()", "event 1 hit 0", "The detector ID is 0", S0->GetDetectorID(), 0u) && Passed;
      Passed = Evaluate("AnalyzeEvent()", "event 1 hit 0", "The strip ID is 5", S0->GetStripID(), 5u) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", "event 1 hit 0", "The side is LV", S0->IsLowVoltageStrip()) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 0", "The ADC value is 4053", S0->GetADCUnits(), 4053.0, 0.5) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 0", "The TAC value is 10452", S0->GetTAC(), 10452.0, 0.5) && Passed;
      Passed = EvaluateTrue("AnalyzeEvent()", "event 1 hit 0", "Fast timing is set", S0->HasFastTiming()) && Passed;
      MStripHit* S1 = R->GetStripHit(1);
      Passed = EvaluateFalse("AnalyzeEvent()", "event 1 hit 1", "The side is HV", S1->IsLowVoltageStrip()) && Passed;
      Passed = EvaluateFalse("AnalyzeEvent()", "event 1 hit 1", "Slow timing is kept", S1->HasFastTiming()) && Passed;
      MStripHit* S2 = R->GetStripHit(2);
      Passed = EvaluateTrue("AnalyzeEvent()", "event 1 hit 2", "The nearest neighbor flag survives", S2->IsNearestNeighbor()) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 2", "The 10-bit nearest-neighbor ADC value 500 survives", S2->GetADCUnits(), 500.0, 0.5) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "event 1 hit 2", "The 10-bit nearest-neighbor TAC value 700 survives", S2->GetTAC(), 700.0, 0.5) && Passed;
    }
  }
  if (Read.size() >= 2) {
    MReadOutAssembly* R = Read[1];
    Passed = Evaluate("AnalyzeEvent()", "event 2", "The event ID is the packet number 2", R->GetID(), (unsigned long) 2) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "event 2", "Two strip hits are read", R->GetNStripHits(), 2u) && Passed;
    if (R->GetNStripHits() == 2) {
      Passed = Evaluate("AnalyzeEvent()", "event 2", "The strip hits are in detector 1", R->GetStripHit(0)->GetDetectorID(), 1u) && Passed;
    }
    Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "event 2", "The shield veto encoded in the event type survives", R->GetShieldVeto())) && Passed;
  }
  if (Read.size() >= 3) {
    MReadOutAssembly* R = Read[2];
    Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "event 3", "The guard ring veto survives", R->GetGuardRingVeto())) && Passed;
    Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "event 3", "Both strip hits survive including the guard ring hit", R->GetNStripHits(), 2u)) && Passed;
    bool FoundGR = false;
    for (unsigned int i = 0; i < R->GetNStripHits(); ++i) {
      MStripHit* S = R->GetStripHit(i);
      if (S->IsGuardRing() == true && S->GetDetectorID() == 0 && S->GetStripID() == 64) FoundGR = true;
    }
    Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "event 3", "The guard ring hit keeps its detector and strip ID", FoundGR)) && Passed;
  }

  for (MReadOutAssembly* E : Written) delete E;
  for (MReadOutAssembly* E : Read) delete E;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsL0::TestFileHeader()
{
  bool Passed = true;

  vector<MReadOutAssembly*> Written;
  MReadOutAssembly* E1 = new MReadOutAssembly();
  E1->SetTimeRTS(MTime(1000, 0));
  AddStripHit(*E1, 0, 5, true, 4053, 10452);
  Written.push_back(E1);
  MString Raw = WriteWithSaver("header_raw.bin", Written);

  vector<uint8_t> Data = ReadBinary(Raw);
  vector<uint8_t> WithHeader = { 'C', 'O', 'S', 'I', 0, 1, 0, 0xDD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
  WithHeader.insert(WithHeader.end(), Data.begin(), Data.end());
  MString HeaderFile = GetTemporaryFileName("header.bin");
  Passed = EvaluateTrue("WriteBinary()", "header.bin", "The fixture file is written", WriteBinary(HeaderFile, WithHeader)) && Passed;

  vector<MReadOutAssembly*> Read = ReadAll(HeaderFile, WriteStripMap());
  Passed = Evaluate("Initialize()", "20-byte COSI header", "One event is read after skipping the header", (unsigned int) Read.size(), 1u) && Passed;
  if (Read.size() == 1) {
    Passed = Evaluate("AnalyzeEvent()", "20-byte COSI header", "The strip hit is read", Read[0]->GetNStripHits(), 1u) && Passed;
  }

  for (MReadOutAssembly* E : Written) delete E;
  for (MReadOutAssembly* E : Read) delete E;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsL0::TestHandCraftedPackets()
{
  bool Passed = true;

  MString StripMap = WriteStripMap();
  Passed = EvaluateFalse("WriteStripMap()", "strip map", "The fixture file is written", StripMap.IsEmpty()) && Passed;

  // Test pulser hit (type 3) with ROI 101, fast, ADC 1234, TAC 4321, followed by a normal hit with unknown ROI 999
  uint64_t Pulser = (uint64_t(0x3) << 40) | (uint64_t(101) << 29) | (uint64_t(1) << 28) | (uint64_t(1234) << 14) | uint64_t(4321);
  uint64_t Unknown = (uint64_t(0x0) << 40) | (uint64_t(999) << 29) | (uint64_t(0) << 28) | (uint64_t(10) << 14) | uint64_t(20);
  vector<uint8_t> HitData(11, 0);
  for (int i = 0; i < 44; ++i) {
    if ((Pulser >> (43 - i)) & 0x1) HitData[i/8] |= (1 << (7 - i % 8));
    if ((Unknown >> (43 - i)) & 0x1) HitData[(44 + i)/8] |= (1 << (7 - (44 + i) % 8));
  }
  MString File = GetTemporaryFileName("handcrafted.bin");
  Passed = EvaluateTrue("WriteBinary()", "handcrafted.bin", "The fixture file is written", WriteBinary(File, CreatePacket(0x0E2, 2, HitData))) && Passed;
  vector<MReadOutAssembly*> Read = ReadAll(File, StripMap);
  Passed = Evaluate("AnalyzeEvent()", "APID 0x0E2", "One event is read", (unsigned int) Read.size(), 1u) && Passed;
  if (Read.size() == 1) {
    Passed = Evaluate("AnalyzeEvent()", "test pulser + unknown ROI", "Only the known strip hit is kept", Read[0]->GetNStripHits(), 1u) && Passed;
    if (Read[0]->GetNStripHits() >= 1) {
      MStripHit* S = Read[0]->GetStripHit(0);
      Passed = Evaluate("AnalyzeEvent()", "test pulser hit", "ROI 101 is LV strip 1 of detector 0", S->GetStripID(), 1u) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "test pulser hit", "The ADC value is 1234", S->GetADCUnits(), 1234.0, 0.5) && Passed;
      Passed = EvaluateNear("AnalyzeEvent()", "test pulser hit", "The TAC value is 4321", S->GetTAC(), 4321.0, 0.5) && Passed;
    }
  }
  for (MReadOutAssembly* E : Read) delete E;

  // Unexpected APID: stop reading
  vector<uint8_t> Good = CreatePacket(0x0DD, 0, {});
  vector<uint8_t> Bad = CreatePacket(0x123, 0, {});
  vector<uint8_t> Mixed = Good;
  Mixed.insert(Mixed.end(), Bad.begin(), Bad.end());
  Mixed.insert(Mixed.end(), Good.begin(), Good.end());
  MString MixedFile = GetTemporaryFileName("mixed.bin");
  Passed = EvaluateTrue("WriteBinary()", "mixed.bin", "The fixture file is written", WriteBinary(MixedFile, Mixed)) && Passed;
  vector<MReadOutAssembly*> MixedRead = ReadAll(MixedFile, StripMap);
  Passed = Evaluate("AnalyzeEvent()", "good, wrong APID, good", "Reading stops at the packet with the unexpected APID", (unsigned int) MixedRead.size(), 1u) && Passed;
  for (MReadOutAssembly* E : MixedRead) delete E;

  // Truncated packet at the end
  vector<uint8_t> Truncated = Good;
  vector<uint8_t> Second = CreatePacket(0x0DD, 1, vector<uint8_t>(6, 0xFF));
  Truncated.insert(Truncated.end(), Second.begin(), Second.end() - 3);
  MString TruncatedFile = GetTemporaryFileName("truncated.bin");
  Passed = EvaluateTrue("WriteBinary()", "truncated.bin", "The fixture file is written", WriteBinary(TruncatedFile, Truncated)) && Passed;
  vector<MReadOutAssembly*> TruncatedRead = ReadAll(TruncatedFile, StripMap);
  Passed = Evaluate("AnalyzeEvent()", "good, truncated", "Only the complete packet is read", (unsigned int) TruncatedRead.size(), 1u) && Passed;
  for (MReadOutAssembly* E : TruncatedRead) delete E;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleLoaderMeasurementsL0::TestReinitialize()
{
  bool Passed = true;

  vector<MReadOutAssembly*> Written;
  for (unsigned int e = 0; e < 2; ++e) {
    MReadOutAssembly* E = new MReadOutAssembly();
    E->SetTimeRTS(MTime((int) (1000 + e), 0));
    AddStripHit(*E, 0, 5, true, 1000, 2000);
    Written.push_back(E);
  }
  MString File = WriteWithSaver("reinit.bin", Written);
  for (MReadOutAssembly* E : Written) delete E;

  MModuleLoaderMeasurementsL0 L;
  L.SetFileName(File);
  L.SetFileNameStripMap(WriteStripMap());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  vector<unsigned int> Counts = { 0, 0 };
  vector<unsigned long> LastID = { 0, 0 };
  vector<bool> Init = { false, false };
  for (unsigned int Round = 0; Round < Init.size(); ++Round) {
    Init[Round] = L.Initialize();
    if (Init[Round] == false) continue;
    for (unsigned int i = 0; i < 10; ++i) {
      MReadOutAssembly E;
      if (L.AnalyzeEvent(&E) == false) break;
      ++Counts[Round];
      LastID[Round] = E.GetID();
    }
    // Deliberately no Finalize() after the first round in order to test a repeated Initialize()
  }
  L.Finalize();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("Initialize()", "first time", "Initialize succeeds", Init[0]) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "first pass", "Two events are read", Counts[0], 2u) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("Initialize()", "second time without Finalize", "Initialize succeeds again", Init[1])) && Passed;
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "second pass", "Two events are read again", Counts[1], 2u)) && Passed;
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "second pass", "The event IDs restart at 1", LastID[1], (unsigned long) 2)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleLoaderMeasurementsL0 Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleLoaderMeasurementsL0.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
