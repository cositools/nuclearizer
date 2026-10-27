/*
 * UTNModuleSaverMeasurementsL0.cxx
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
#include "MFile.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleSaverMeasurementsL0.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Exposes the protected helpers of MModuleSaverMeasurementsL0 for direct testing
class UTNModuleSaverMeasurementsL0Access : public MModuleSaverMeasurementsL0
{
 public:
  //! Forward to the protected EncodeNormalHit()
  uint64_t CallEncodeNormalHit(int StripID, bool FastTiming, int ADC, int TAC) { return EncodeNormalHit(StripID, FastTiming, ADC, TAC); }
  //! Forward to the protected EncodeNeighborHit()
  uint64_t CallEncodeNeighborHit(int StripID, bool FastTiming, int ADC, int TAC) { return EncodeNeighborHit(StripID, FastTiming, ADC, TAC); }
  //! Forward to the protected EncodeGuardRingHit()
  uint32_t CallEncodeGuardRingHit(int StripID, int ADC) { return EncodeGuardRingHit(StripID, ADC); }
  //! Forward to the protected PackBitsIntoBitstream()
  void CallPackBitsIntoBitstream(vector<uint8_t>& Bitstream, int& BitOffset, uint64_t Bits, int NumBits) { PackBitsIntoBitstream(Bitstream, BitOffset, Bits, NumBits); }
  //! Forward to the protected GetEventType()
  uint8_t CallGetEventType(MReadOutAssembly* Event) { return GetEventType(Event); }
};


////////////////////////////////////////////////////////////////////////////////


//! Unit test class for MModuleSaverMeasurementsL0
class UTNModuleSaverMeasurementsL0 : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleSaverMeasurementsL0() : MUnitTest("UTNModuleSaverMeasurementsL0") {}
  //! Default destructor
  virtual ~UTNModuleSaverMeasurementsL0() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data
  bool TestDefaultConstruction();
  //! Test the setters/getters and the XML round trip
  bool TestConfiguration();
  //! Test the Initialize() error paths
  bool TestInitializeErrors();
  //! Test the hit encoders
  bool TestEncoders();
  //! Test the bit packing
  bool TestPackBits();
  //! Test the event type classification
  bool TestEventType();
  //! Test the written packet bytes
  bool TestPacketLayout();
  //! Test the limits of the packet format
  bool TestFormatLimits();
  //! Test the read-out ID written for a guard ring hit
  bool TestGuardRingReadOutID();

  //! Write the strip map and return its name
  MString WriteStripMap();
  //! Read a binary file
  vector<uint8_t> ReadBinary(const MString& FileName);
  //! Read NBits bits starting at bit position Start of a byte array (MSB first)
  uint64_t ExtractBits(const vector<uint8_t>& Data, size_t Start, unsigned int NBits);
  //! Write one event with a fresh saver into the named temporary file and return the file content
  vector<uint8_t> WriteSingleEvent(const MString& Name, MReadOutAssembly& Event, bool& InitResult, bool& AnalyzeResult);
  //! Return true if the bytes are exactly one CCSDS packet with the HITS/HLEN/ETYPE header
  bool IsSinglePacket(const vector<uint8_t>& Bytes) const;
  //! Add a strip hit to the event
  MStripHit* AddStripHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, int ADC = 1000, int TAC = 2000);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestConfiguration() && Passed;
  Passed = TestInitializeErrors() && Passed;
  Passed = TestEncoders() && Passed;
  Passed = TestPackBits() && Passed;
  Passed = TestEventType() && Passed;
  Passed = TestPacketLayout() && Passed;
  Passed = TestFormatLimits() && Passed;
  Passed = TestGuardRingReadOutID() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleSaverMeasurementsL0::WriteStripMap()
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
  // An empty name makes Initialize() fail, which the callers check
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


vector<uint8_t> UTNModuleSaverMeasurementsL0::ReadBinary(const MString& FileName)
{
  ifstream In(FileName.Data(), ios::binary);
  return vector<uint8_t>((istreambuf_iterator<char>(In)), istreambuf_iterator<char>());
}


////////////////////////////////////////////////////////////////////////////////


vector<uint8_t> UTNModuleSaverMeasurementsL0::WriteSingleEvent(const MString& Name, MReadOutAssembly& Event, bool& InitResult, bool& AnalyzeResult)
{
  MString FileName = GetTemporaryFileName(Name);
  MModuleSaverMeasurementsL0 M;
  M.SetFileName(FileName);
  M.SetFileNameStripMap(WriteStripMap());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  InitResult = M.Initialize();
  AnalyzeResult = false;
  if (InitResult == true) {
    AnalyzeResult = M.AnalyzeEvent(&Event);
    M.Finalize();
  }
  g_Verbosity = OldVerbosity;
  return ReadBinary(FileName);
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::IsSinglePacket(const vector<uint8_t>& Bytes) const
{
  // Primary header (6) + secondary header (8) + TRUNC_TIME (4) + HITS/HLEN/ETYPE (3)
  if (Bytes.size() < 21) return false;
  size_t Length = 6 + ((Bytes[4] << 8) | Bytes[5]) + 1;
  return Bytes.size() == Length;
}


////////////////////////////////////////////////////////////////////////////////


uint64_t UTNModuleSaverMeasurementsL0::ExtractBits(const vector<uint8_t>& Data, size_t Start, unsigned int NBits)
{
  uint64_t Result = 0;
  for (unsigned int i = 0; i < NBits; ++i) {
    size_t Bit = Start + i;
    if (Bit/8 >= Data.size()) return Result;
    Result = (Result << 1) | ((Data[Bit/8] >> (7 - Bit % 8)) & 0x1);
  }
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* UTNModuleSaverMeasurementsL0::AddStripHit(MReadOutAssembly& Event, unsigned int DetID, unsigned int StripID, bool IsLowVoltage, int ADC, int TAC)
{
  MStripHit* SH = new MStripHit();
  SH->SetDetectorID(DetID);
  SH->SetStripID(StripID);
  SH->IsLowVoltageStrip(IsLowVoltage);
  SH->SetADCUnits(ADC);
  SH->SetTAC(TAC);
  SH->HasFastTiming(true);
  Event.AddStripHit(SH);
  return SH;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleSaverMeasurementsL0 M;
  Passed = Evaluate("GetXmlTag()", "default", "The XML tag is 'XmlTagSaverMeasurementsL0'", M.GetXmlTag(), MString("XmlTagSaverMeasurementsL0")) && Passed;
  Passed = EvaluateFalse("GetName()", "default", "The name is not empty", M.GetName().IsEmpty()) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_EventSaver", M.ProvidesModuleType(MAssembly::c_EventSaver)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "An event loader is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_EventLoader)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default", "The detector effects engine is a hard requirement", M.IsHardPreceedingModule(MAssembly::c_DetectorEffectsEngine)) && Passed;
  bool Immediate = false;
  for (unsigned int i = 0; i < M.GetNPreceedingModuleTypes(); ++i) {
    if (M.GetPreceedingModuleType(i) == MAssembly::c_DetectorEffectsEngine) Immediate = M.GetPreceedingModuleImmediateRequirement(i);
  }
  Passed = EvaluateTrue("GetPreceedingModuleImmediateRequirement()", "default", "The detector effects engine must immediately preceed the saver", Immediate) && Passed;
  Passed = EvaluateFalse("IsTypeExclusive()", "default", "Multiple savers are allowed in a sequence", M.IsTypeExclusive()) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", M.HasOptionsGUI()) && Passed;
  Passed = Evaluate("GetFileName()", "default", "The default file name is empty", M.GetFileName(), MString("")) && Passed;
  Passed = Evaluate("GetFileNameStripMap()", "default", "The default strip map name is empty", M.GetFileNameStripMap(), MString("")) && Passed;

  MModuleSaverMeasurementsL0* C = M.Clone();
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a non-null object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "default", "Clone() returns a distinct object", C != &M) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "default", "The clone has the same XML tag", C->GetXmlTag(), M.GetXmlTag()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::TestConfiguration()
{
  bool Passed = true;

  MModuleSaverMeasurementsL0 Writer;
  Writer.SetFileName("/data/out.bin");
  Writer.SetFileNameStripMap("/data/map.map");
  Passed = Evaluate("SetFileName/GetFileName", "/data/out.bin", "GetFileName returns the set name", Writer.GetFileName(), MString("/data/out.bin")) && Passed;
  Passed = Evaluate("SetFileNameStripMap/GetFileNameStripMap", "/data/map.map", "GetFileNameStripMap returns the set name", Writer.GetFileNameStripMap(), MString("/data/map.map")) && Passed;

  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleSaverMeasurementsL0 Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "both names", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "FileName", "The file name survives", Reader.GetFileName(), MString("/data/out.bin")) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "FileNameStripMap", "The strip map name survives", Reader.GetFileNameStripMap(), MString("/data/map.map")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::TestInitializeErrors()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  DisableDefaultStreams();

  MModuleSaverMeasurementsL0 NoName;
  NoName.SetFileNameStripMap(WriteStripMap());
  bool NoNameResult = NoName.Initialize();

  MModuleSaverMeasurementsL0 BadDir;
  BadDir.SetFileName(GetTemporaryFileName("nodir") + "/out.bin");
  BadDir.SetFileNameStripMap(WriteStripMap());
  bool BadDirResult = BadDir.Initialize();

  MModuleSaverMeasurementsL0 NoMap;
  NoMap.SetFileName(GetTemporaryFileName("nomap.bin"));
  bool NoMapResult = NoMap.Initialize();
  NoMap.Finalize();

  MModuleSaverMeasurementsL0 MissingMap;
  MissingMap.SetFileName(GetTemporaryFileName("missingmap.bin"));
  MissingMap.SetFileNameStripMap(GetTemporaryFileName("missing.map"));
  bool MissingMapResult = MissingMap.Initialize();
  MissingMap.Finalize();

  EnableDefaultStreams();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "no file name", "Initialize returns false", NoNameResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "non-existing directory", "Initialize returns false", BadDirResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "no strip map name", "Initialize returns false", NoMapResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing strip map", "Initialize returns false", MissingMapResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::TestEncoders()
{
  bool Passed = true;

  UTNModuleSaverMeasurementsL0Access M;

  uint64_t Normal = M.CallEncodeNormalHit(105, true, 4053, 10452);
  Passed = EvaluateTrue("EncodeNormalHit()", "ROI 105, fast, 4053, 10452", "The encoded hit has the documented layout", Normal == ((uint64_t(105) << 29) | (uint64_t(1) << 28) | (uint64_t(4053) << 14) | uint64_t(10452))) && Passed;
  Passed = EvaluateTrue("EncodeNormalHit()", "ROI 105", "The hit type nibble is 0", (Normal >> 40) == 0) && Passed;
  Passed = EvaluateTrue("EncodeNormalHit()", "slow timing", "The timing bit is 0 for slow timing", ((M.CallEncodeNormalHit(105, false, 1, 1) >> 28) & 0x1) == 0) && Passed;
  Passed = EvaluateTrue("EncodeNormalHit()", "ADC 16384", "The ADC value is masked to 14 bits", ((M.CallEncodeNormalHit(0, false, 16384 + 5, 0) >> 14) & 0x3FFF) == 5) && Passed;

  uint64_t Neighbor = M.CallEncodeNeighborHit(2047, false, 1023, 512);
  Passed = EvaluateTrue("EncodeNeighborHit()", "ROI 2047, slow, 1023, 512", "The encoded hit has the documented layout", Neighbor == ((uint64_t(1) << 32) | (uint64_t(2047) << 21) | (uint64_t(1023) << 10) | uint64_t(512))) && Passed;

  uint32_t GR = M.CallEncodeGuardRingHit(17, 3000);
  Passed = EvaluateTrue("EncodeGuardRingHit()", "ROI 17, 3000", "The encoded hit has the documented layout", GR == ((uint32_t(2) << 20) | (uint32_t(17) << 15) | (uint32_t(3000) << 1))) && Passed;
  Passed = EvaluateTrue("EncodeGuardRingHit()", "ROI 17", "The padding bit is 0", (GR & 0x1) == 0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::TestPackBits()
{
  bool Passed = true;

  UTNModuleSaverMeasurementsL0Access M;
  vector<uint8_t> Stream;
  int Offset = 0;
  M.CallPackBitsIntoBitstream(Stream, Offset, 0x5, 3);   // 101
  M.CallPackBitsIntoBitstream(Stream, Offset, 0xFF, 8);  // 11111111
  Passed = Evaluate("PackBitsIntoBitstream()", "101 + 11111111", "The offset advances by 11 bits", Offset, 11) && Passed;
  Passed = Evaluate("PackBitsIntoBitstream()", "101 + 11111111", "Two bytes are used", (unsigned int) Stream.size(), 2u) && Passed;
  if (Stream.size() == 2) {
    Passed = Evaluate("PackBitsIntoBitstream()", "101 + 11111111", "The first byte is 10111111", (unsigned int) Stream[0], 0xBFu) && Passed;
    Passed = Evaluate("PackBitsIntoBitstream()", "101 + 11111111", "The second byte is 11100000", (unsigned int) Stream[1], 0xE0u) && Passed;
  }

  vector<uint8_t> Wide;
  int WideOffset = 0;
  M.CallPackBitsIntoBitstream(Wide, WideOffset, M.CallEncodeNormalHit(105, true, 4053, 10452), 44);
  Passed = Evaluate("PackBitsIntoBitstream()", "44-bit normal hit", "Six bytes are used", (unsigned int) Wide.size(), 6u) && Passed;
  Passed = EvaluateTrue("PackBitsIntoBitstream()", "44-bit normal hit", "The strip ID can be read back", ExtractBits(Wide, 4, 11) == 105) && Passed;
  Passed = EvaluateTrue("PackBitsIntoBitstream()", "44-bit normal hit", "The TAC can be read back", ExtractBits(Wide, 30, 14) == 10452) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::TestEventType()
{
  bool Passed = true;

  UTNModuleSaverMeasurementsL0Access M;

  {
    MReadOutAssembly E;
    AddStripHit(E, 0, 3, true);
    AddStripHit(E, 0, 4, true);
    AddStripHit(E, 0, 5, false);
    Passed = Evaluate("GetEventType()", "one detector, neighboring strips", "Single site, no veto is 0x02", (unsigned int) M.CallGetEventType(&E), 0x02u) && Passed;
  }
  {
    MReadOutAssembly E;
    AddStripHit(E, 0, 3, true);
    AddStripHit(E, 0, 6, true);
    AddStripHit(E, 0, 5, false);
    Passed = Evaluate("GetEventType()", "one detector, LV strips 3 and 6", "Two strips separated by two strips are a single-detector Compton event (0x01)", (unsigned int) M.CallGetEventType(&E), 0x01u) && Passed;
  }
  {
    // Strips 3 and 5 are separated by only one strip: not neighboring, but below the minimum separation of 2 strips
    MReadOutAssembly E;
    AddStripHit(E, 0, 3, true);
    AddStripHit(E, 0, 5, true);
    AddStripHit(E, 0, 5, false);
    Passed = ExpectedToFail(EvaluateTrue("GetEventType()", "one detector, LV strips 3 and 5", "Strips separated by only one strip are not a single-detector Compton event", M.CallGetEventType(&E) != 0x01)) && Passed;
  }
  {
    MReadOutAssembly E;
    AddStripHit(E, 0, 3, true);
    AddStripHit(E, 1, 3, true);
    Passed = Evaluate("GetEventType()", "two detectors", "Multi-detector, no veto is 0x00", (unsigned int) M.CallGetEventType(&E), 0x00u) && Passed;
    E.SetGuardRingVeto(true);
    Passed = Evaluate("GetEventType()", "two detectors, GR veto", "Multi-detector with GR veto is 0x03", (unsigned int) M.CallGetEventType(&E), 0x03u) && Passed;
  }
  {
    MReadOutAssembly E;
    AddStripHit(E, 0, 3, true);
    AddStripHit(E, 0, 3, false);
    E.SetGuardRingVeto(true);
    Passed = Evaluate("GetEventType()", "single site, GR veto", "Single site with GR veto is 0x05", (unsigned int) M.CallGetEventType(&E), 0x05u) && Passed;
    E.SetGuardRingVeto(false);
    E.SetShieldVeto(true);
    Passed = Evaluate("GetEventType()", "single site, shield veto", "Single site with (soft) shield veto is 0x0B", (unsigned int) M.CallGetEventType(&E), 0x0Bu) && Passed;
  }
  {
    MReadOutAssembly E;
    Passed = Evaluate("GetEventType()", "no strip hits", "An empty event is classified as single site (0x02)", (unsigned int) M.CallGetEventType(&E), 0x02u) && Passed;
  }
  {
    // The guard ring hit (strip 64) is no strip of the single-site interaction
    MReadOutAssembly E;
    AddStripHit(E, 0, 3, true);
    AddStripHit(E, 0, 3, false);
    MStripHit* GR = AddStripHit(E, 0, 64, true);
    GR->IsGuardRing(true);
    E.SetGuardRingVeto(true);
    Passed = ExpectedToFail(Evaluate("GetEventType()", "single site plus LV guard ring hit, GR veto", "Single site with GR veto is 0x05", (unsigned int) M.CallGetEventType(&E), 0x05u)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::TestPacketLayout()
{
  bool Passed = true;

  MString FileName = GetTemporaryFileName("layout.bin");
  MModuleSaverMeasurementsL0 M;
  M.SetFileName(FileName);
  M.SetFileNameStripMap(WriteStripMap());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Init = M.Initialize();
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("Initialize()", "valid configuration", "Initialize succeeds", Init) && Passed;
  if (Init == false) return false;

  MReadOutAssembly E1;
  E1.SetTimeRTS(MTime(1000, 500000000));
  AddStripHit(E1, 0, 5, true, 4053, 10452);
  Passed = EvaluateTrue("AnalyzeEvent()", "one normal hit", "AnalyzeEvent returns true", M.AnalyzeEvent(&E1)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "one normal hit", "The c_EventSaver analysis progress is set", E1.HasAnalysisProgress(MAssembly::c_EventSaver)) && Passed;

  MReadOutAssembly E2;
  E2.SetTimeRTS(MTime(1001, 0));
  AddStripHit(E2, 0, 5, true, 100, 200);
  Passed = EvaluateTrue("AnalyzeEvent()", "second normal hit", "AnalyzeEvent returns true", M.AnalyzeEvent(&E2)) && Passed;
  M.Finalize();

  vector<uint8_t> B = ReadBinary(FileName);
  // Packet 1: 6 primary + 8 secondary + 4 trunc time + 3 header + 6 hit bytes = 27 bytes
  Passed = Evaluate("AnalyzeEvent()", "two packets", "Two 27-byte packets are written", (unsigned int) B.size(), 54u) && Passed;
  if (B.size() >= 54) {
    Passed = Evaluate("AnalyzeEvent()", "packet 1", "Word 0 contains the secondary header flag and APID 0x0DD", (unsigned int) ((B[0] << 8) | B[1]), 0x08DDu) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "packet 1", "Word 1 contains the standalone sequence flags and sequence count 0", (unsigned int) ((B[2] << 8) | B[3]), 0xC000u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "packet 1", "The packet data length is 27 - 6 - 1 = 20", (unsigned int) ((B[4] << 8) | B[5]), 20u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "packet 1", "HITS is 1", (unsigned int) (B[18] >> 2), 1u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "packet 1", "HLEN is 6 bytes", (unsigned int) (((B[18] & 0x3) << 8) | B[19]), 6u) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "packet 1", "ETYPE is single site (0x02)", (unsigned int) B[20], 0x02u) && Passed;
    vector<uint8_t> Hit(B.begin() + 21, B.begin() + 27);
    Passed = EvaluateTrue("AnalyzeEvent()", "packet 1 hit", "The hit type is 0", ExtractBits(Hit, 0, 4) == 0) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "packet 1 hit", "The read-out ID 105 is used", ExtractBits(Hit, 4, 11) == 105) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "packet 1 hit", "The fast timing bit is set", ExtractBits(Hit, 15, 1) == 1) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "packet 1 hit", "The ADC value is 4053", ExtractBits(Hit, 16, 14) == 4053) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "packet 1 hit", "The TAC value is 10452", ExtractBits(Hit, 30, 14) == 10452) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "packet 2", "The sequence count of the second packet is 1", (unsigned int) ((B[29] << 8) | B[30]), 0xC001u) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::TestGuardRingReadOutID()
{
  bool Passed = true;

  // The strip map assigns read-out ID 300 to the LV guard ring (strip 64) of detector 0; strip 0 has read-out ID 100
  MReadOutAssembly E;
  E.SetTimeRTS(MTime(1000, 0));
  MStripHit* GR = AddStripHit(E, 0, 64, true, 3000);
  GR->IsGuardRing(true);
  E.SetGuardRingVeto(true);
  bool Init = false;
  bool Result = false;
  vector<uint8_t> B = WriteSingleEvent("guardring.bin", E, Init, Result);
  Passed = EvaluateTrue("Initialize()", "guard ring hit", "Initialize succeeds", Init) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "guard ring hit", "A complete packet is written", IsSinglePacket(B)) && Passed;
  if (IsSinglePacket(B) == true && B.size() >= 24) {
    vector<uint8_t> Hit(B.begin() + 21, B.begin() + 24);
    Passed = Evaluate("AnalyzeEvent()", "LV guard ring of detector 0", "The hit type is 0x2 (guard ring)", (unsigned int) ExtractBits(Hit, 0, 4), 2u) && Passed;
    Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "LV guard ring of detector 0", "The 5-bit read-out ID is that of the guard ring (300 & 0x1F = 12), not that of strip 0 (100 & 0x1F = 4)", (unsigned int) ExtractBits(Hit, 4, 5), 12u)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsL0::TestFormatLimits()
{
  bool Passed = true;

  // Each scenario uses its own saver and file, so the result of one cannot hide the other

  // 64 strip hits do not fit into the 6-bit HITS field
  MReadOutAssembly Many;
  Many.SetTimeRTS(MTime(1000, 0));
  for (unsigned int i = 0; i < 64; ++i) AddStripHit(Many, 0, i % 10, (i % 2 == 0));
  bool ManyInit = false;
  bool ManyResult = false;
  vector<uint8_t> BM = WriteSingleEvent("limits_many.bin", Many, ManyInit, ManyResult);
  Passed = EvaluateTrue("Initialize()", "64 strip hits", "Initialize succeeds", ManyInit) && Passed;
  if (ManyResult == true) {
    Passed = EvaluateTrue("AnalyzeEvent()", "64 strip hits", "A complete packet is written", IsSinglePacket(BM)) && Passed;
    if (IsSinglePacket(BM) == true) {
      Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "64 strip hits", "A written packet must contain the correct number of hits", (unsigned int) (BM[18] >> 2), 64u)) && Passed;
    }
  } else {
    Passed = Evaluate("AnalyzeEvent()", "64 strip hits", "A rejected event writes nothing", (unsigned int) BM.size(), 0u) && Passed;
  }

  // A strip hit which is not in the strip map
  MReadOutAssembly Unknown;
  Unknown.SetTimeRTS(MTime(1000, 0));
  AddStripHit(Unknown, 7, 3, true);
  bool UnknownInit = false;
  bool UnknownResult = false;
  vector<uint8_t> BU = WriteSingleEvent("limits_unknown.bin", Unknown, UnknownInit, UnknownResult);
  Passed = EvaluateTrue("Initialize()", "strip not in the strip map", "Initialize succeeds", UnknownInit) && Passed;
  if (BU.empty() == false) {
    // If a packet is written, the unknown strip must not appear in it under any read-out ID
    Passed = EvaluateTrue("AnalyzeEvent()", "strip not in the strip map", "A complete packet is written", IsSinglePacket(BU)) && Passed;
    if (IsSinglePacket(BU) == true) {
      Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "strip not in the strip map", "A strip which is not in the strip map is not silently written with another read-out ID", (unsigned int) (BU[18] >> 2), 0u)) && Passed;
    }
  } else {
    Passed = EvaluateFalse("AnalyzeEvent()", "strip not in the strip map", "An event which writes nothing reports a failure", UnknownResult) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleSaverMeasurementsL0 Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleSaverMeasurementsL0.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
