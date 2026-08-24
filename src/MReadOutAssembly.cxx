/*
 * MReadOutAssembly.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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
// MReadOutAssembly
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutAssembly.h"

// Standard libs:
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


atomic<unsigned long> MReadOutAssembly::s_NextAssemblyID(0);


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly::MReadOutAssembly() : MReadOutSequence(), m_AssemblyID(++s_NextAssemblyID), m_EventTimeUTC(0)
{
  // Construct an instance of MReadOutAssembly

  m_PhysicalEvent = nullptr;
  m_SimEvent = nullptr;
  m_HasSimAspectInfo = false;

  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly::~MReadOutAssembly()
{
  // Destruct an instance of MReadOutAssembly

  // Clear() also resets state, not just owned memory, but the overhead is small compared to the code duplication it removes
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::Clear()
{
  // Reset all data

  MReadOutSequence::Clear();

  m_ID = g_UnsignedIntNotDefined;
  m_EventTimeRTS = 0;
  m_EventTimeUTC = 0;

  m_HasSimAspectInfo = false;
  m_GalacticPointingXAxisTheta = 0.0;
  m_GalacticPointingXAxisPhi = 0.0;
  m_GalacticPointingZAxisTheta = 0.0;
  m_GalacticPointingZAxisPhi = 0.0;

  m_ShieldVeto = false;
  m_GuardRingVeto = false;
  m_Trigger = true;
  m_InDetector.fill(false);

  // Delete all strip hits
  for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
    delete m_StripHits[h];
  }
  m_StripHits.clear();

  for (unsigned int h = 0; h < m_CrystalHits.size(); ++h) {
    delete m_CrystalHits[h];
  }
  m_CrystalHits.clear();


  // Delete all hits
  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    delete m_Hits[h];
  }
  m_Hits.clear();

  // Delete all guardring hits
  for (unsigned int h = 0; h < m_GuardringHits.size(); ++h) {
    delete m_GuardringHits[h];
  }
  m_GuardringHits.clear();

  // Delete all event flags and associated variables
  m_EnergyCalibrationError = false;
  m_EnergyCalibrationErrorString.clear();
  m_StripPairingError = false;
  m_StripPairingErrorString.clear();
  m_DepthCalibrationError = false;
  m_DepthCalibrationErrorString.clear();
  m_EventReconstructionError = false;
  m_EventReconstructionErrorString.clear();

  m_StripPairingReducedChiSquare.clear();

  m_StripHitBelowThreshold_QualityFlag = false;
  m_StripHitBelowThresholdString_QualityFlag.clear();

  m_StripPairing_QualityFlag = false;
  m_StripPairingString_QualityFlag.clear();

  m_FilteredOut = false;
  m_AnalysisProgress = 0;

  delete m_PhysicalEvent;
  m_PhysicalEvent = nullptr;

  m_DEEStripHitsLV.clear();
  m_DEEStripHitsHV.clear();
  m_DEECrystalHits.clear();

  delete m_SimEvent;
  m_SimEvent = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::DeleteHits()
{
  // Delete all MHit objects

  for (auto* Hit : m_Hits) {
    delete Hit;
  }
  m_Hits.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::InDetector(int DetectorID) const
{
  // Find out if the event contains strip hits in a given detector

  if (DetectorID >= 0 && DetectorID <= 15) {
    return m_InDetector[DetectorID];
  }

  if (g_Verbosity >= c_Error) {
    cout<<"Error in MReadOutAssembly::InDetector: detector ID "<<DetectorID
        <<" out of bounds (valid range: 0-15)"<<endl;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::SetPhysicalEvent(MPhysicalEvent* Event)
{
  // Set the physical event from event reconstruction
  // We make our own local copy here

  // Guard against self-assignment: deleting m_PhysicalEvent first would leave
  // Event dangling before Duplicate() could be called on it
  if (Event == m_PhysicalEvent) return;

  delete m_PhysicalEvent;
  if (Event != nullptr) {
    m_PhysicalEvent = Event->Duplicate();
  } else {
    m_PhysicalEvent = nullptr;
  }
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* MReadOutAssembly::GetStripHit(unsigned int i)
{
  // Return strip hit i

  if (i < m_StripHits.size()) {
    return m_StripHits[i];
  }

  if (g_Verbosity >= c_Error) cout<<"Error in MReadOutAssembly::GetStripHit: index "<<i<<" out of bounds (size "<<m_StripHits.size()<<")"<<endl;

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::AddStripHit(MStripHit* StripHit)
{
  // Add a strip hit

  if (StripHit == nullptr) return;

  unsigned int DetectorID = StripHit->GetDetectorID();
  if (DetectorID <= 15) {
    m_InDetector[DetectorID] = true;
  }
  m_StripHits.push_back(StripHit);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveStripHit(unsigned int i)
{
  // Remove a strip hit

  if (i < m_StripHits.size()) {
    // BUG: MHit objects retain non-owning references to this strip hit if the caller deletes it
    vector<MStripHit*>::iterator it;
    it = m_StripHits.begin() + i;
    m_StripHits.erase(it);

    // Recompute the per-detector flags from the remaining strip hits
    for (unsigned int DetectorID = 0; DetectorID < 16; ++DetectorID) {
      m_InDetector[DetectorID] = false;
    }
    for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
      unsigned int DetectorID = m_StripHits[h]->GetDetectorID();
      if (DetectorID <= 15) {
        m_InDetector[DetectorID] = true;
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


MCrystalHit* MReadOutAssembly::GetCrystalHit(unsigned int i)
{
  // Return crystal hit i

  if (i < m_CrystalHits.size()) {
    return m_CrystalHits[i];
  }

  if (g_Verbosity >= c_Error) cout<<"Error in MReadOutAssembly::GetCrystalHit: index "<<i<<" out of bounds (size "<<m_CrystalHits.size()<<")"<<endl;

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::AddCrystalHit(MCrystalHit* CrystalHit)
{
  // Add a crystal hit

  if (CrystalHit == nullptr) return;

  // Note: For ACS detectors, DetectorID is a string (e.g., "X0", "X1", "Y0", "Y1", "Z0", "Z1")
  // so we can't use it with m_InDetector array which expects numeric indices 0-15.
  // The m_InDetector tracking is primarily for GeD detectors which have numeric IDs.
  // We skip the m_InDetector tracking for crystal hits.

  m_CrystalHits.push_back(CrystalHit);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveCrystalHit(unsigned int i)
{
  // Remove a crystal hit

  if (i < m_CrystalHits.size()) {
    vector<MCrystalHit*>::iterator it;
    it = m_CrystalHits.begin() + i;
    m_CrystalHits.erase(it);
  }
}


////////////////////////////////////////////////////////////////////////////////


MGuardringHit* MReadOutAssembly::GetGuardringHit(unsigned int i)
{
  // Return guardring hit i

  if (i < m_GuardringHits.size()) {
    return m_GuardringHits[i];
  }

  if (g_Verbosity >= c_Error) cout<<"Error in MReadOutAssembly::GetGuardringHit: index "<<i<<" out of bounds (size "<<m_GuardringHits.size()<<")"<<endl;

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MHit* MReadOutAssembly::GetHit(unsigned int i)
{
  // Return hit i

  if (i < m_Hits.size()) {
    return m_Hits[i];
  }

  if (g_Verbosity >= c_Error) cout<<"Error in MReadOutAssembly::GetHit: index "<<i<<" out of bounds (size "<<m_Hits.size()<<")"<<endl;

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveHit(unsigned int i)
{
  // Remove a hit

  if (i < m_Hits.size()) {
    m_Hits.erase(m_Hits.begin() + i);
  }
}


////////////////////////////////////////////////////////////////////////////////


MTime MReadOutAssembly::ComputeRTSfromUTCTime(MTime UTCTime) const
{
  // Compute the RTS time if the event only has UTC time defined
  // RTS is elapsed time since January 1, 2025 in TT (terrestrial time)
  // TT (terrestrial time) = TAI (international atomic time) + 32.184 seconds, and TAI = UTC + 37 leap seconds

  MTime RTS_Unix = MTime(2025, 1, 1, 0, 0, 0, 0);
  MTime RTS_TT = UTCTime - RTS_Unix + 37 + 32.184;

  return RTS_TT;
}


////////////////////////////////////////////////////////////////////////////////


MTime MReadOutAssembly::ComputeUTCfromRTSTime(MTime RTSTime) const
{
  // Compute the UTC time if the event only has RTS time defined
  // RTS is elapsed time since January 1, 2025 in TT (terrestrial time)
  // TT (terrestrial time) = TAI (international atomic time) + 32.184 seconds, and TAI = UTC + 37 leap seconds

  MTime RTS_Unix = MTime(2025, 1, 1, 0, 0, 0, 0);
  MTime UTCTime = RTSTime + RTS_Unix - 37 - 32.184;

  return UTCTime;
}


////////////////////////////////////////////////////////////////////////////////


MTime MReadOutAssembly::ComputeRTSfromGPSTime(MTime GPSTime) const
{
  // Compute RTS time from GPS by converting to UTC, then call ComputeRTSfromUTCTime
  // UTC = GPS - 18 seconds (GPS time is currently 18 leap seconds ahead of UTC)

  MTime GPS_Unix = MTime(1980, 1, 6, 0, 0, 0, 0);
  MTime UTCTime = GPS_Unix + GPSTime - 18;

  return ComputeRTSfromUTCTime(UTCTime);
}


////////////////////////////////////////////////////////////////////////////////


MTime MReadOutAssembly::ComputeGPSfromRTSTime(MTime RTSTime) const
{
  // Compute GPS time from RTS by calling ComputeUTCfromRTSTime then converting UTC to GPS
  // GPS = UTC + 18

  MTime GPS_Unix = MTime(1980, 1, 6, 0, 0, 0, 0);
  MTime UTCTime = ComputeUTCfromRTSTime(RTSTime);

  return UTCTime - GPS_Unix + 18;
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::Parse(MString& Line, int Version)
{
  // HT, SH, and BD are handled here; malformed HT or SH lines return false

  if (Line.BeginsWith("TI")) {
    MTime T(0);
    if (T.Set(Line) == false) return false;
    SetTime(T);
    SetTimeUTC(T);
    return true;
  }

  if (Line.BeginsWith("HT")) {
    MHit* h = new MHit();
    if (h->Parse(Line, Version) == true) {
      AddHit(h);
      return true;
    } else {
      delete h;
      return false;
    }
  }
  if (Line.BeginsWith("SH")) {
    // Assume that the SH line belongs to the last read hit
    if (m_Hits.empty() == true) return false;
    MHit* h = m_Hits.back();
    MStripHit* SH = new MStripHit();
    if (SH->Parse(Line, 2) == true) {
      AddStripHit(SH);
      h->AddStripHit(SH);
      return true;
    } else {
      delete SH;
      return false;
    }
  }
  if (Line.BeginsWith("BD")) {
    // Mark the event as filtered out
    m_FilteredOut = true;
    return true;
  }

  // Everything else goes to the tolerant base parser, which also consumes unrecognized lines and returns true
  return MReadOutSequence::Parse(Line);
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::GetNextFromDatFile(MFile& F)
{
  // Read data from a .dat file

  MString Line;
  bool EventRead = false;

  int i = 0;
  int MaxLinesToRead = 1000;

  Clear();
  for (i = 0; i < MaxLinesToRead; ++i) {
    // Try up to "MaxLinesToRead" lines to read the next complete event
    if (F.ReadLine(Line) == false) {
      // End of file reached
      break;
    }
    const char* LineData = Line.Data();
    if (Line.BeginsWith("SE")) {
      if (i != 0) {
        // We read the full event in, break now
        break;
      }
    } else if (Line.BeginsWith("ID")) {
      unsigned int ID = 0;
      if (sscanf(&LineData[3], "%u", &ID) == 1) {
        SetID(ID);
        EventRead = true;
      } else {
        if (g_Verbosity >= c_Error) cout<<"Error in MReadOutAssembly::GetNextFromDatFile(): Error parsing ID line"<<endl;
      }
    } else if (Line.BeginsWith("TI")) {
      EventRead = true;
      MTime T = MTime();
      T.Set(Line);
      SetTimeUTC(T);
    } else if (Line.BeginsWith("HT")) {
      MHit* h = new MHit();
      if (h->Parse(Line) == true) {
        AddHit(h);
        EventRead = true;
      } else {
        delete h;
      }
    } else if (Line.BeginsWith("SH")) {
      MStripHit* sh = new MStripHit();
      if (sh->Parse(Line) == true) {
        AddStripHit(sh);
        EventRead = true;
        if (m_Hits.size() > 0) {
          // Add this SH to the last read HT
          MHit* h = m_Hits.back();
          h->AddStripHit(sh);
        }
      } else {
        delete sh;
      }
    } else if (Line.BeginsWith("BD")) {
      EventRead = true;
      SetFilteredOut(true);
    }

  }

  if (i == MaxLinesToRead) {
    if (g_Verbosity >= c_Error) cout<<"Error in MReadOutAssembly::GetNextFromDatFile(): Event not fully read after "<<MaxLinesToRead<<" lines"<<endl;
    return false;
  }

  return EventRead;
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::StreamDat(ostream& S, int Version)
{
  // Stream the read-out assembly to an ASCII `.dat` file

  if (Version >= 1 && Version <= 3) {
    S<<"SE"<<endl;
    S<<"ID "<<m_ID<<endl;

    if (m_EventTimeUTC == 0 && m_EventTimeRTS != 0) {
      S<<"TI "<<ComputeUTCfromRTSTime(m_EventTimeRTS)<<endl;
    } else {
      S<<"TI "<<m_EventTimeUTC<<endl;
    }

    for (MSimIA& IA: m_SimIAs) {
      S<<IA.ToSimString()<<endl;
    }

    if (Version == 1) {
      for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
        m_StripHits[h]->StreamDat(S, Version);
      }

      for (unsigned int h = 0; h < m_Hits.size(); ++h) {
        m_Hits[h]->StreamDat(S, Version);
      }
    } else if (Version == 2) {
      for (auto H : m_Hits) {
        H->StreamDat(S, 2);
      }
    } else if (Version == 3) {
      for (auto H : m_Hits) {
        H->StreamDat(S, 3);
      }
    }

    StreamBDFlags(S);

    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamEvta(ostream& S)
{
  // Stream the read-out assembly in MEGAlib's EVTA format

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;

  if (m_EventTimeUTC == 0 && m_EventTimeRTS != 0) {
    S<<"TI "<<ComputeUTCfromRTSTime(m_EventTimeRTS)<<endl;
  } else {
    S<<"TI "<<m_EventTimeUTC<<endl;
  }

  if (m_HasSimAspectInfo){
    S<<"GX "<<m_GalacticPointingXAxisPhi<<" "<<m_GalacticPointingXAxisTheta<<endl;
    S<<"GZ "<<m_GalacticPointingZAxisPhi<<" "<<m_GalacticPointingZAxisTheta<<endl;
  }

  for (MSimIA& IA: m_SimIAs) {
    S<<IA.ToSimString()<<endl;
  }

  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    m_Hits[h]->StreamEvta(S);
  }

  S<<"CC NStripHits "<<m_StripHits.size()<<endl;

  StreamBDFlags(S);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamRoa(ostream& S, bool WithADCs, bool WithTACs, bool WithEnergies, bool WithTimings, bool WithTemperatures, bool WithFlags, bool WithOrigins, bool WithNearestNeighbors)
{
  // Stream the read-out assembly in MEGAlib's ROA format
  //
  // WithTemperatures is currently not used, since we don't have that housekeeping info at the moment

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;

  if (m_EventTimeUTC == 0 && m_EventTimeRTS != 0) {
    S<<"TI "<<ComputeUTCfromRTSTime(m_EventTimeRTS)<<endl;
  } else {
    S<<"TI "<<m_EventTimeUTC<<endl;
  }

  for (MSimIA& IA: m_SimIAs) {
    S<<IA.ToSimString()<<endl;
  }

  unsigned int Counter = 0;
  for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
    if (WithNearestNeighbors == false && m_StripHits[h]->IsNearestNeighbor() == true) {
      continue;
    }
    m_StripHits[h]->StreamRoa(S, WithADCs, WithTACs, WithEnergies, WithTimings, WithFlags, WithOrigins);
    ++Counter;
  }
  for (unsigned int h = 0; h < m_CrystalHits.size(); ++h) {
    m_CrystalHits[h]->StreamRoa(S, WithADCs, WithEnergies, WithFlags, WithOrigins);
    ++Counter;
  }
  if (Counter == 0) {
    S<<"BD No hits"<<endl;
  }

  StreamBDFlags(S);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamTra(ostream& S)
{
  // Stream the read-out assembly in MEGAlib's TRA format

  S<<"SE"<<endl;

  if (m_PhysicalEvent != nullptr) {
    S<<m_PhysicalEvent->ToTraString();
  } else {
    S<<"ID "<<m_ID<<endl;
    StreamBDFlags(S);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamBDFlags(ostream& S)
{
  // Stream the BD and QA flags for this assembly

  if (m_EnergyCalibrationError == true) {
    S<<"BD EnergyCalibrationError";
    if (m_EnergyCalibrationErrorString.empty() == false) {
      // Append any associated error text
      for (auto i : m_EnergyCalibrationErrorString) {
        S<<" ("<<i<<")";
      }
    }
    S<<endl;
  }
  if (m_StripPairingError == true) {
    S<<"BD StripPairingError";
    if (m_StripPairingErrorString.empty() == false) {
      // Append any associated error text
      for (auto i : m_StripPairingErrorString) {
        S<<" ("<<i<<")";
      }
    }
    S<<endl;
  }
  if (m_DepthCalibrationError == true) {
    S<<"BD DepthCalibrationError";
    if (m_DepthCalibrationErrorString.empty() == false) {
      // Append any associated error text
      for (auto i : m_DepthCalibrationErrorString) {
        S<<" ("<<i<<")";
      }
    }
    S<<endl;
  }
  if (m_EventReconstructionError == true) {
    S<<"BD EventReconstructionError";
    if (m_EventReconstructionErrorString.empty() == false) {
      // Append any associated error text
      for (auto i : m_EventReconstructionErrorString) {
        S<<" ("<<i<<")";
      }
    }
    S<<endl;
  }

  if (m_StripHitBelowThreshold_QualityFlag == true) {
    S<<"QA StripHitBelowThreshold";
    if (m_StripHitBelowThresholdString_QualityFlag.empty() == false) {
      // Append any associated error text
      for (auto i : m_StripHitBelowThresholdString_QualityFlag) {
        S<<" ("<<i<<")";
      }
    }
    S<<endl;
  }

  if (m_StripPairing_QualityFlag == true) {
    S<<"QA StripPairing";
    if (m_StripPairingString_QualityFlag.empty() == false) {
      // Append any associated error text
      for (auto i : m_StripPairingString_QualityFlag) {
        S<<" ("<<i<<")";
      }
    }
    S<<endl;
  }

  if (m_GuardRingVeto == true) {
    S<<"BD GR Veto"<<endl;
  }
  if (m_ShieldVeto == true) {
    S<<"BD Shield Veto"<<endl;
  }

  S<<"PQ";
  // Append the strip pairing reduced chi^2 values
  for (auto i : m_StripPairingReducedChiSquare) {
    S<<" "<<i;
  }
  S<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::IsGood() const
{
  // Return true if no error flag is set and the event has not been filtered out
  // Veto and quality flags do not affect this result

  if (m_EnergyCalibrationError == true) return false;
  if (m_StripPairingError == true) return false;
  if (m_DepthCalibrationError == true) return false;
  if (m_EventReconstructionError == true) return false;

  if (m_FilteredOut == true) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::IsBad() const
{
  // Return true if any error flag is set or the event has been filtered out
  // Veto and quality flags do not affect this result

  if (m_EnergyCalibrationError == true) return true;
  if (m_StripPairingError == true) return true;
  if (m_DepthCalibrationError == true) return true;
  if (m_EventReconstructionError == true) return true;

  if (m_FilteredOut == true) return true;

  return false;
}


//////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::IsVeto() const
{
  // Return true if one of the veto flags has been set

  if (m_ShieldVeto == true) return true;
  if (m_GuardRingVeto == true) return true;

  return false;
}


// MReadOutAssembly.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
