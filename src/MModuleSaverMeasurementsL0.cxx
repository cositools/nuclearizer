/*
 * MModuleSaverMeasurementsL0.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MModuleSaverMeasurementsL0
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleSaverMeasurementsL0.h"

// Standard libs:
#include <algorithm>
#include <cstring>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MStripHit.h"

// Nuclearizer libs:
#include "MGUIOptionsSaverMeasurementsL0.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleSaverMeasurementsL0)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleSaverMeasurementsL0::MModuleSaverMeasurementsL0() : MModule()
{
  // Construct an instance of MModuleSaverMeasurementsL0

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Save events to L0 binary (DD packets)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagSaverMeasurementsL0";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventSaver);

  // Set if this module has an options GUI
  m_HasOptionsGUI = true;

  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;

  m_SequenceCount = 0;
  m_TotalEventsWritten = 0;
}


////////////////////////////////////////////////////////////////////////////////


MModuleSaverMeasurementsL0::~MModuleSaverMeasurementsL0()
{
  // Delete this instance of MModuleSaverMeasurementsL0
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaverMeasurementsL0::Initialize()
{
  // Initialize the module

  if (m_FileName == "") {
    if (g_Verbosity >= c_Error) cout << m_XmlTag << ": No output file name specified." << endl;
    return false;
  }

  // Open the output file in binary mode
  m_OutFile.open(string(m_FileName), ios::out | ios::binary);
  if (!m_OutFile.is_open()) {
    if (g_Verbosity >= c_Error) cout << m_XmlTag << ": Unable to open output file: " << m_FileName << endl;
    return false;
  }

  m_SequenceCount = 0;
  m_TotalEventsWritten = 0;

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaverMeasurementsL0::WriteUInt16BE(uint16_t value)
{
  // initialize a 2 bytes array
  uint8_t bytes[2];
  bytes[0] = (value >> 8) & 0xFF;
  bytes[1] = value & 0xFF;
  m_OutFile.write(reinterpret_cast<char*>(bytes), 2);
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaverMeasurementsL0::WriteUInt32BE(uint32_t value)
{
  // initialize a 4 bytes array
  uint8_t bytes[4];
  bytes[0] = (value >> 24) & 0xFF;
  bytes[1] = (value >> 16) & 0xFF;
  bytes[2] = (value >> 8) & 0xFF;
  bytes[3] = value & 0xFF;

  // write the 4 bytes into the file
  m_OutFile.write(reinterpret_cast<char*>(bytes), 4);
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaverMeasurementsL0::WriteCCSDSPrimaryHeader(uint16_t apid, uint16_t seqCount, uint16_t dataLength)
{
  // Write 6-byte CCSDS Primary Header
  // Byte 0-1: Pkt Version 3 bits | Packet Type 1 bit | SecHdrFlag 1 bit | APID 11 bits
  // Byte 2-3: SeqFlags 2 bits | SeqCount 14 bits
  // Byte 4-5: PacketDataLength 2 bytes

  // Version: 0, Packet type: 0, secHdrFlag: 1, and APID
  uint16_t word0 = 0x0800 | (apid & 0x7FF);
  WriteUInt16BE(word0);

  // SeqFlags: 11 (standalone Pkt), and Sequence Count
  uint16_t word1 = 0xC000 | (seqCount & 0x3FFF);
  WriteUInt16BE(word1);

  // Word 2: Packet Data Length = len(bytes after primary header) - 1
  WriteUInt16BE(dataLength);
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaverMeasurementsL0::WriteSecondaryHeader(uint32_t seconds, uint32_t subseconds)
{
  // Write 8-byte Secondary Header (packet time: sec (4 bytes), and subsecond (4 bytes))
  WriteUInt32BE(seconds);
  WriteUInt32BE(subseconds);
}


////////////////////////////////////////////////////////////////////////////////

// bitstream: Hit Data Array
// bitOffset: Track where am I in the bitStream as I pack hits one after another. Ensure no padding.
// bits: it is the actaully bits that needed to insert into the bit array, Hit Type 0: 44 bits, ...
// numBits: Number of bits
void MModuleSaverMeasurementsL0::PackBitsIntoBitstream(std::vector<uint8_t>& bitstream, int& bitOffset, uint64_t bits, int numBits)
{
  for (int i = numBits - 1; i >= 0; --i) {
    // Find which byte am I at
    int byteIndex = bitOffset / 8;
    // Find which bite am I at
    int bitIndex = 7 - (bitOffset % 8);

    // Ensure we have enough bytes
    while (bitstream.size() <= (size_t)byteIndex) {
      bitstream.push_back(0);
    }

    // Extract the bit and set it
    uint8_t bit = (bits >> i) & 1;
    bitstream[byteIndex] |= (bit << bitIndex);

    bitOffset++;
  }
}


////////////////////////////////////////////////////////////////////////////////


uint64_t MModuleSaverMeasurementsL0::EncodeNormalHit(int stripID, bool fastTiming, int energy, int timing)
{
  // A normal strip hit (Type 0x0) - 44 bits
  // Format: HitType: 4 bits + StripID 11 bits + Timing Type: 1bit + Energy Data: 14 bits + Timing data: 14 bits
  // Returns 44 bits packed into the lower bits of a uint64_t

  uint64_t hitType = 0x0;
  uint64_t strip = stripID & 0x7FF;      // 11 bits
  uint64_t dt = fastTiming ? 1 : 0;      // 1 bit
  uint64_t eng = energy & 0x3FFF;        // 14 bits
  uint64_t tim = timing & 0x3FFF;        // 14 bits

  // Pack bits: [HitType:4][StripID:11][TimingType:1][Energy:14][TimingData:14] = 44 bits
  uint64_t encoded = (hitType << 40) | (strip << 29) | (dt << 28) | (eng << 14) | tim;

  return encoded;
}


////////////////////////////////////////////////////////////////////////////////


uint64_t MModuleSaverMeasurementsL0::EncodeNeighborHit(int stripID, bool fastTiming, int energy, int timing)
{
  // A neighboring strip hit (Type 0x1) - 36 bits
  // Format: HitType: 4 bits + StripID: 11 bits + Timing Type: 1 bit + Energy Data: 10 bits + Timing Data: 10 bits
  // Returns 36 bits packed into the lower bits of a uint64_t

  uint64_t hitType = 0x1;
  uint64_t strip = stripID & 0x7FF;      // 11 bits
  uint64_t dt = fastTiming ? 1 : 0;      // 1 bit
  uint64_t eng = energy & 0x3FF;         // 10 bits
  uint64_t tim = timing & 0x3FF;         // 10 bits

  // Pack bits: [HitType:4][StripID:11][TimingType:1][Energy:10][TimingData:10] = 36 bits
  uint64_t encoded = (hitType << 32) | (strip << 21) | (dt << 20) | (eng << 10) | tim;

  return encoded;
}


////////////////////////////////////////////////////////////////////////////////


uint32_t MModuleSaverMeasurementsL0::EncodeGuardRingHit(int stripID, int energy)
{
  // A guard ring hit (Type 0x2) - 24 bits
  // Format: HitType: 4 bits + StripID: 5 bits + Energy Data: 14 bits + Pad: 1 bit
  // Returns 24 bits packed into the lower bits of a uint32_t

  uint32_t hitType = 0x2;
  uint32_t strip = stripID & 0x1F;        // 5 bits
  uint32_t eng = energy & 0x3FFF;         // 14 bits

  // Pack bits: [HitType:4][StripID:5][Energy:14][Pad:1] = 24 bits
  uint32_t encoded = (hitType << 20) | (strip << 15) | (eng << 1) | 0;

  return encoded;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaverMeasurementsL0::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Write this event as a DD packet

  // Get event timing
  uint64_t clockTick = Event->GetCL(); //TODO: What does GetCL actaully return?

  // Convert clock tick to TRUNC_TIME format: 10 bits seconds + 22 bits 4MHz subseconds
  uint32_t seconds10bit = clockTick & 0x3FF;  // 10-bit seconds
  uint32_t truncTime = seconds10bit << 22;    // TODO: Need to confirm with Andreas, not sure is this the right way to do it.

  // Collect all strip hits and pack them into a continuous bit stream
  unsigned int numHits = Event->GetNStripHits();
  std::vector<uint8_t> hitData;
  int bitOffset = 0;
  int totalBits = 0;

  for (unsigned int i = 0; i < numHits; ++i) {
    MStripHit* hit = Event->GetStripHit(i);

    // Get strip info
    int stripID = hit->GetStripID();
    bool fastTiming = hit->HasFastTiming();
    int energy = (int)hit->GetADCUnits();
    int timing = (int)hit->GetTAC();

    if (hit->IsGuardRing()) {
      uint32_t encoded = EncodeGuardRingHit(stripID, energy);
      PackBitsIntoBitstream(hitData, bitOffset, encoded, 24);
      totalBits += 24;
    } else if (hit->IsNearestNeighbor()) {
      uint64_t encoded = EncodeNeighborHit(stripID, fastTiming, energy, timing);
      PackBitsIntoBitstream(hitData, bitOffset, encoded, 36);
      totalBits += 36;
    } else {
      uint64_t encoded = EncodeNormalHit(stripID, fastTiming, energy, timing);
      PackBitsIntoBitstream(hitData, bitOffset, encoded, 44);
      totalBits += 44;
    }
  }

  // get the length of hitData in bytes
  uint16_t hlen = (totalBits + 7) / 8;

  // Build packet data: TRUNC_TIME: 4 bytes + NumHits: 6 bits, HLEN(Lenght of the hit data): 10 bits, ETYPE: 8 bits = 4 + 3 = 7 bytes + HLEN bytes
  uint8_t hits6 = numHits & 0x3F;  // 6 bits
  uint16_t hlen10 = hlen & 0x3FF;  // 10 bits
  uint8_t etype = 0x00;            // Event type, TODO: placeholder for now, dont know how to get event type

  // Calculate total packet data length (after primary header)
  // = Secondary Header (8) + TRUNC_TIME (4) + HITS + HLEN + ETYPE (3) + HIT_DATA (hlen)
  uint16_t packetDataLength = 8 + 4 + 3 + hlen - 1;

  // Get packet time for secondary header
  uint32_t pktSeconds = (uint32_t)clockTick;  // Whole seconds
  uint32_t pktSubseconds = 0;                  // TODO: Is there subsecond

  // Write CCSDS Primary Header
  WriteCCSDSPrimaryHeader(APID_DD, m_SequenceCount, packetDataLength);
  m_SequenceCount = (m_SequenceCount + 1) & 0x3FFF;

  // Write Secondary Header
  WriteSecondaryHeader(pktSeconds, pktSubseconds);

  // Write TRUNC_TIME (4 bytes)
  WriteUInt32BE(truncTime);

  // Write HITS (6 bits) + HLEN (10 bits) + ETYPE(8 bits) = 3 bytes
  // Byte 0: HITS[5:0] HLEN[9:8]
  uint8_t byte0 = (hits6 << 2) | ((hlen10 >> 8) & 0x03);
  // Byte 1: HLEN[7:0]
  uint8_t byte1 = hlen10 & 0xFF;
  // Byte 2: ETYPE[7:0]
  uint8_t byte2 = etype;

  m_OutFile.write(reinterpret_cast<char*>(&byte0), 1);
  m_OutFile.write(reinterpret_cast<char*>(&byte1), 1);
  m_OutFile.write(reinterpret_cast<char*>(&byte2), 1);

  // Write HIT_DATA
  if (!hitData.empty()) {
    m_OutFile.write(reinterpret_cast<char*>(hitData.data()), hitData.size());
  }

  Event->SetAnalysisProgress(MAssembly::c_EventSaver);
  m_TotalEventsWritten++;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaverMeasurementsL0::Finalize()
{
  // Finalize the module

  MModule::Finalize();

  if (g_Verbosity >= c_Info) {
    cout << m_XmlTag << ": MModuleSaverMeasurementsL0" << endl;
    cout << m_XmlTag << ":   * total events written: " << m_TotalEventsWritten << endl;
  }

  // Close the output file
  if (m_OutFile.is_open()) {
    m_OutFile.close();
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaverMeasurementsL0::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != nullptr) {
    m_FileName = FileNameNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleSaverMeasurementsL0::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);

  return Node;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaverMeasurementsL0::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsSaverMeasurementsL0* Options = new MGUIOptionsSaverMeasurementsL0(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


// MModuleSaverMeasurementsL0.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
