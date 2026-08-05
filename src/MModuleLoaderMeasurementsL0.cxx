/*
 * MModuleLoaderMeasurementsL0.cxx
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
// MModuleLoaderMeasurementsL0
//
// Reads CCSDS science packets from an L0 binary file and converts them into
// MReadOutAssembly events with strip hits.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleLoaderMeasurementsL0.h"

// Standard libs:
#include <stdexcept>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MFile.h"
#include "MStripHit.h"
#include "MTime.h"

// Nuclearizer libs:
#include "MGUIOptionsLoaderMeasurementsL0.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleLoaderMeasurementsL0)
#endif


////////////////////////////////////////////////////////////////////////////////


// CCSDS / L0 file constants
static const uint32_t COSI_IDENTIFIER = 0x434F5349;  // "COSI" in ASCII
static const size_t L0_FILE_HEADER_SIZE = 20;         // 20 byte: 4 bytes ID, 2 bytes SC ID, 2 bytes PKT APID, 4 bytes UTC first pkt, 4 bytes UTC time last pkt, 4 bytes pkt count
static const size_t L0_CRC_TRAILER_SIZE = 2;          //16-bit CRC-CCITT checksum of the file
static const size_t CCSDS_PRIMARY_HEADER_SIZE = 6;
static const size_t CCSDS_SECONDARY_HEADER_SIZE = 8;
static const uint16_t APID_GRB_COMPTON = 0x0DD;
static const uint16_t APID_PRIMARY_SCIENCE = 0x0E2;
static const uint16_t APID_ANCILLARY_SCIENCE = 0x0E3;


////////////////////////////////////////////////////////////////////////////////


// bit-level reader for the variable-length HIT_DATA payload.
namespace {
class BitReader {
public:
  BitReader(const std::vector<uint8_t>& data) : m_Data(data), m_BytePos(0), m_BitPos(0) {}

  // Read up to 32 bits and return them as an unsigned integer
  uint32_t ReadBits(unsigned int n) {
    uint32_t result = 0;
    for (unsigned int i = 0; i < n; ++i) {
      if (m_BytePos >= m_Data.size()) {
        throw std::out_of_range("BitReader: ran out of bits");
      }
      // read each bit and accumulate into result
      uint8_t bit = (m_Data[m_BytePos] >> (7 - m_BitPos)) & 0x1;
      result = (result << 1) | bit;

      // Advance the cursor. Move to the next bit
      ++m_BitPos;
      if (m_BitPos == 8) {
        ++m_BytePos;
        m_BitPos = 0;
      }
    }
    return result;
  }

  size_t BitsRemaining() const {
    if (m_BytePos >= m_Data.size()) return 0;
    return (m_Data.size() - m_BytePos) * 8 - m_BitPos;
  }

private:
  const std::vector<uint8_t>& m_Data;
  size_t m_BytePos;
  unsigned int m_BitPos;
};
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsL0::MModuleLoaderMeasurementsL0() : MModuleLoaderMeasurements()
{
  // Construct an instance of MModuleLoaderMeasurementsL0

  m_Name = "Measurement loader for L0 science binary files";
  m_XmlTag = "XmlTagMeasurementLoaderL0";

  m_IsStartModule = true;

  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;

  m_StripMapLoaded = false;
  m_PacketsRead = 0;
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsL0::~MModuleLoaderMeasurementsL0()
{
  // Delete this instance of MModuleLoaderMeasurementsL0
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsL0::Initialize()
{
  m_PacketsRead = 0;

  if (m_FileName == "") {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": No L0 file name specified."<<endl;
    return false;
  }

  if (MFile::Exists(m_FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": L0 file does not exist: "<<m_FileName<<endl;
    return false;
  }

  // Load the strip map
  if (m_FileNameStripMap == "") {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": No strip map file name specified."<<endl;
    return false;
  }

  if (m_StripMap.Open(m_FileNameStripMap) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Failed to open strip map file: "<<m_FileNameStripMap<<endl;
    return false;
  }
  m_StripMapLoaded = true;

  if (OpenL0File(m_FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Failed to open L0 file."<<endl;
    return false;
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsL0::OpenL0File(MString FileName)
{
  m_InFile.open(string(FileName), ios::in | ios::binary);
  if (!m_InFile.is_open()) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open file: "<<FileName<<endl;
    return false;
  }

  // Auto-detect file format: peek at first 4 bytes.
  //   MOC-simulator output: starts with 0x434F5349 ("COSI") magic
  //   Raw nuclearizer output: starts with a CCSDS primary header
  uint8_t magicBuf[4];
  m_InFile.read(reinterpret_cast<char*>(magicBuf), 4);
  if (m_InFile.gcount() != 4) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": File too short to read magic bytes."<<endl;
    return false;
  }

  uint32_t magic = (uint32_t(magicBuf[0]) << 24) | (uint32_t(magicBuf[1]) << 16) |
                   (uint32_t(magicBuf[2]) << 8)  |  uint32_t(magicBuf[3]);

  if (magic == COSI_IDENTIFIER) {
    // L0 file with header — skip the rest of the 20-byte header (16 more bytes).
    m_InFile.seekg(L0_FILE_HEADER_SIZE, ios::beg);
    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Detected L0 file format (with header)."<<endl;
  } else {
    // Raw packets — rewind to the start
    m_InFile.seekg(0, ios::beg);
    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Detected raw CCSDS packet format (no header)."<<endl;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsL0::AnalyzeEvent(MReadOutAssembly* Event)
{
  if (ReadNextPacket(Event) == false) {
    if (g_Verbosity >= c_Info) cout<<m_Name<<": No more packets!"<<endl;
    m_IsFinished = true;
    return false;
  }

  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement);

  ++m_PacketsRead;

  // set the current packet # as the event ID
  Event->SetID(m_PacketsRead);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsL0::ReadNextPacket(MReadOutAssembly* Event)
{
  // Read the 6-byte CCSDS primary header
  uint8_t priHdr[CCSDS_PRIMARY_HEADER_SIZE];
  m_InFile.read(reinterpret_cast<char*>(priHdr), CCSDS_PRIMARY_HEADER_SIZE);
  if (m_InFile.gcount() < (streamsize)CCSDS_PRIMARY_HEADER_SIZE) {
    return false; // EOF or truncated
  }

  uint16_t word0 = (uint16_t(priHdr[0]) << 8) | priHdr[1];
  uint16_t pktDataLen = (uint16_t(priHdr[4]) << 8) | priHdr[5];
  uint16_t apid = word0 & 0x07FF;
  bool secHdrFlag = (word0 >> 11) & 0x1;

  // Total bytes after primary header = pktDataLen + 1
  size_t payloadSize = pktDataLen + 1;

  if (apid != APID_GRB_COMPTON &&
      apid != APID_PRIMARY_SCIENCE &&
      apid != APID_ANCILLARY_SCIENCE) {
    if (g_Verbosity >= c_Error) {
      cout << m_XmlTag << ": Unexpected APID 0x" << hex << apid << dec
           << " in file (expected 0x0DD, 0x0E2, or 0x0E3). Stopping." << endl;
    }
    m_IsFinished = true;
    return false;
  }

  // Read the rest of the packet (secondary header + payload)
  std::vector<uint8_t> rest(payloadSize);
  m_InFile.read(reinterpret_cast<char*>(rest.data()), payloadSize);
  if (m_InFile.gcount() < (streamsize)payloadSize) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Truncated packet"<<endl;
    return false;
  }

  size_t cursor = 0;

  // Secondary header (8 bytes): 4 bytes seconds + 4 bytes subseconds.
  // subseconds are 40 MHz DCB clock ticks (25 ns each)
  uint32_t pktSeconds = 0;
  uint32_t pktSubseconds = 0;
  if (secHdrFlag) {
    if (rest.size() < CCSDS_SECONDARY_HEADER_SIZE) return false;
    pktSeconds = (uint32_t(rest[0]) << 24) | (uint32_t(rest[1]) << 16) |
                 (uint32_t(rest[2]) <<  8) |  uint32_t(rest[3]);
    pktSubseconds = (uint32_t(rest[4]) << 24) | (uint32_t(rest[5]) << 16) |
                    (uint32_t(rest[6]) <<  8) |  uint32_t(rest[7]);
    cursor += CCSDS_SECONDARY_HEADER_SIZE;
  }

  // Science payload: TRUNC_TIME (4) + HITS/HLEN/ETYPE (3) + HIT_DATA (HLEN bytes)
  if (rest.size() < cursor + 7) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Science payload too short"<<endl;
    return false;
  }

  uint32_t truncTime = (uint32_t(rest[cursor]) << 24) | (uint32_t(rest[cursor+1]) << 16) |
                       (uint32_t(rest[cursor+2]) <<  8) |  uint32_t(rest[cursor+3]);
  cursor += 4;

  uint8_t  byte0 = rest[cursor];
  uint8_t  byte1 = rest[cursor+1];
  uint8_t  etype = rest[cursor+2];
  unsigned int hits = byte0 >> 2;
  unsigned int hlen = ((uint16_t(byte0 & 0x3) << 8) | byte1);
  cursor += 3;

  if (rest.size() < cursor + hlen) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": HIT_DATA truncated (have "<<rest.size()-cursor<<", want "<<hlen<<")"<<endl;
    return false;
  }
  std::vector<uint8_t> hitData(rest.begin() + cursor, rest.begin() + cursor + hlen);

  // Build GPS MTime from seconds and subseconds, then convert and set to RTS
  // Subseconds are 40 MHz DCB ticks → ns = ticks * 25
  long int pktNanoseconds = (long int)pktSubseconds * 25;
  MTime gpsTime((long int)pktSeconds, pktNanoseconds);
  Event->SetTimeRTS(Event->ComputeRTSfromGPSTime(gpsTime));

  // Decode the bit-packed HIT_DATA into individual MStripHit objects on the event
  if (DecodeHitData(hitData, hits, Event) == false) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsL0::DecodeHitData(const std::vector<uint8_t>& hitData,
                                                unsigned int expectedHits,
                                                MReadOutAssembly* Event)
{
  BitReader br(hitData);
  unsigned int parsed = 0;

  while (br.BitsRemaining() >= 4 && parsed < expectedHits) {
    uint32_t hitType = 0;
    try {
      hitType = br.ReadBits(4);
    } catch (const std::out_of_range&) {
      break;
    }

    unsigned int stripID = 0;
    int adc = 0;
    int tac = 0;
    bool fastTiming = false;
    bool isGuardRing = false;
    bool isNeighbor = false;

    try {
      if (hitType == 0x0) {
        // Normal: 11(strip) + 1(fastTimingFlag) + 14(adc) + 14(tac) = 40 more bits
        if (br.BitsRemaining() < 40) break;
        stripID = br.ReadBits(11);
        fastTiming = br.ReadBits(1) == 1;
        adc = br.ReadBits(14);
        tac = br.ReadBits(14);
      } else if (hitType == 0x1) {
        // Neighbor: 11 + 1 + 10 + 10 = 32 more bits
        if (br.BitsRemaining() < 32) break;
        stripID = br.ReadBits(11);
        fastTiming = br.ReadBits(1) == 1;
        adc = br.ReadBits(10);
        tac = br.ReadBits(10);
        isNeighbor = true;
      } else if (hitType == 0x2) {
        // Guard ring: 5 + 14 + 1(pad) = 20 more bits
        if (br.BitsRemaining() < 20) break;
        stripID = br.ReadBits(5);
        adc = br.ReadBits(14);
        (void)br.ReadBits(1); // pad
        isGuardRing = true;
        tac = 0;
        fastTiming = false;
      } else if (hitType == 0x3) {
        // Test pulser: 11(strip) + 1(fastTimingFlag) + 14(adc) + 14(tac) = 40 more bits
        if (br.BitsRemaining() < 40) break;
        stripID = br.ReadBits(11);
        fastTiming = br.ReadBits(1) == 1;
        adc = br.ReadBits(14);
        tac = br.ReadBits(14);
      } else {
        // Unknown — abort this packet
        if (g_Verbosity >= c_Warning) {
          cout<<m_XmlTag<<": Unknown hit type 0x"<<hex<<hitType<<dec<<", aborting packet"<<endl;
        }
        break;
      }
    } catch (const std::out_of_range&) {
      break;
    }

    // Resolve the 11-bit strip ID into (detector, side, strip) via the strip map
    unsigned int detectorID = 0;
    bool isLowVoltage = false;
    unsigned int stripNumber = 0;

    if (m_StripMap.HasReadOutID(stripID)) {
      detectorID = m_StripMap.GetDetectorID(stripID);
      isLowVoltage = m_StripMap.IsLowVoltage(stripID);
      stripNumber = m_StripMap.GetStripNumber(stripID);
    } else {
      if (g_Verbosity >= c_Warning) {
        cout<<m_XmlTag<<": Read-out ID "<<stripID<<" not found in strip map; skipping hit"<<endl;
      }
      ++parsed;
      continue;
    }

    // Build the MStripHit and add it to the event
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(detectorID);
    SH->SetStripID(stripNumber);
    SH->IsLowVoltageStrip(isLowVoltage);
    SH->SetADCUnits(adc);
    SH->SetTAC(tac);
    SH->HasFastTiming(fastTiming);
    SH->IsNearestNeighbor(isNeighbor);
    SH->IsGuardRing(isGuardRing);

    if (isGuardRing) {
      Event->SetGuardRingVeto(true);
    }

    Event->AddStripHit(SH);
    ++parsed;
  }

  if (parsed != expectedHits && g_Verbosity >= c_Warning) {
    cout<<m_XmlTag<<": Expected "<<expectedHits<<" hits but parsed "<<parsed<<endl;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsL0::Finalize()
{
  if (m_InFile.is_open()) {
    m_InFile.close();
  }

  MModule::Finalize();

  if (g_Verbosity >= c_Info) {
    cout<<m_XmlTag<<": MModuleLoaderMeasurementsL0"<<endl;
    cout<<m_XmlTag<<":   * total packets read: "<<m_PacketsRead<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsL0::ReadXmlConfiguration(MXmlNode* Node)
{
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != nullptr) {
    m_FileName = FileNameNode->GetValue();
  }

  MXmlNode* StripMapNode = Node->GetNode("FileNameStripMap");
  if (StripMapNode != nullptr) {
    m_FileNameStripMap = StripMapNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleLoaderMeasurementsL0::CreateXmlConfiguration()
{
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);
  new MXmlNode(Node, "FileNameStripMap", m_FileNameStripMap);
  return Node;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsL0::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsLoaderMeasurementsL0* Options = new MGUIOptionsLoaderMeasurementsL0(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


// MModuleLoaderMeasurementsL0.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
