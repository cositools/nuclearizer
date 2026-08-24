/*
 * MStripMap.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Felix Hagemann.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer & Felix Hagemann.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MStripMap
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MStripMap.h"

// Standard libs:
#include <cerrno>
#include <cstdlib>
#include <limits>

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MParser.h"
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MStripMap)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Construct an instance of a strip map
MStripMap::MStripMap()
{
  // Nothing to do
}


////////////////////////////////////////////////////////////////////////////////


//! // Delete this instance of a strip map
MStripMap::~MStripMap()
{
  // Nothing to do
}


////////////////////////////////////////////////////////////////////////////////


//! Compute the packed (detector, side, strip) lookup key
bool MStripMap::ComputeDetSideStripKey(unsigned int DetectorID, bool IsLowVoltage, unsigned int StripNumber, unsigned int& Key) const
{
  // Out-of-range values overflow their bit field and would alias onto a different, valid entry
  if (DetectorID > c_MaxDetectorID || StripNumber > c_MaxStripNumber) return false;

  Key = (DetectorID << c_DetectorIDBitPosition) | ((IsLowVoltage ? 0u : 1u) << c_SideBitPosition) | StripNumber;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Load a strip map
bool MStripMap::Open(const MString& FileName)
{
  m_StripMappings.clear();
  m_DetSideStripToROI.clear();

  auto IsUnsignedInteger = [](const MString& Token) {
    if (Token.IsEmpty() == true) return false;

    // strtoul() accepts a leading sign and wraps negative values modulo 2^64, so require digits only
    for (size_t c = 0; c < Token.Length(); ++c) {
      if (Token[c] < '0' || Token[c] > '9') return false;
    }

    char* End = nullptr;
    errno = 0;
    unsigned long Value = strtoul(Token.Data(), &End, 10);
    return errno == 0 && *End == '\0' && Value <= (unsigned long) numeric_limits<int>::max();
  };

  auto IsBoolean = [](const MString& Token) {
    MString Lower = Token;
    Lower.ToLower();
    return Lower == "true" || Lower == "false" || Lower == "0" || Lower == "1";
  };

  MParser Parser;
  if (Parser.Open(FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<"MStripMap: Unable to load file: "<<endl<<FileName<<error;
    return false;
  }

  // The column count of the first record fixes the format for the whole file
  unsigned int ExpectedNTokens = 0;

  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (Parser.GetTokenizerAt(i)->GetTokenAtAsString(0).BeginsWith("#") == true) continue;
    const unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
    MTokenizer* T = Parser.GetTokenizerAt(i);

    if (ExpectedNTokens == 0) {
      ExpectedNTokens = NTokens;
    } else if (NTokens != ExpectedNTokens) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: Mixed strip map formats in " << FileName << " at line " << i + 1 << " (expected " << ExpectedNTokens << " columns, got " << NTokens << ")" << endl;
      m_StripMappings.clear();
      return false;
    }

    // Strip map format 1 (with 9 columns)
    if (NTokens == 9) {
      if (IsUnsignedInteger(T->GetTokenAtAsString(0)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(1)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(2)) == false ||
          IsBoolean(T->GetTokenAtAsString(3)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(4)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(5)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(6)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(7)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(8)) == false) {
        if (g_Verbosity >= c_Error) cout << "MStripMap: Invalid value in " << FileName << " at line " << i + 1 << endl;
        m_StripMappings.clear();
        return false;
      }

      MSingleStripMapping SM;
      SM.m_ReadOutID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(0);
      SM.m_RTB = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(1);
      SM.m_DRM = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2);
      SM.m_IsPrimary = Parser.GetTokenizerAt(i)->GetTokenAtAsBoolean(3);
      SM.m_ASICID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(4);
      SM.m_ChannelID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(5);
      SM.m_DetectorID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(6);
      const unsigned int Side = T->GetTokenAtAsUnsignedInt(7);
      SM.m_IsLowVoltage = (Side == 0 ? true : false);
      SM.m_StripNumber = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(8);
      if (SM.m_DetectorID > c_MaxDetectorID) {
        if (g_Verbosity >= c_Error) cout << "MStripMap: Detector ID value " << SM.m_DetectorID << " is out of range [0, " << c_MaxDetectorID << "] in " << FileName << " at line " << i + 1 << endl;
        m_StripMappings.clear();
        return false;
      }
      if (Side > 1) {
        if (g_Verbosity >= c_Error) cout << "MStripMap: Side value " << Side << " is out of range [0, 1] in " << FileName << " at line " << i + 1 << endl;
        m_StripMappings.clear();
        return false;
      }
      if (SM.m_StripNumber > c_MaxStripNumber) {
        if (g_Verbosity >= c_Error) cout << "MStripMap: Strip number value " << SM.m_StripNumber << " is out of range [0, " << c_MaxStripNumber << "] in " << FileName << " at line " << i + 1 << endl;
        m_StripMappings.clear();
        return false;
      }
      m_StripMappings.push_back(SM);
    }
    // Strip map format 2 (with 4 columns)
    else if (NTokens == 4) {
      if (IsUnsignedInteger(T->GetTokenAtAsString(0)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(1)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(2)) == false ||
          IsUnsignedInteger(T->GetTokenAtAsString(3)) == false) {
        if (g_Verbosity >= c_Error) cout << "MStripMap: Invalid value in " << FileName << " at line " << i + 1 << endl;
        m_StripMappings.clear();
        return false;
      }

      MSingleStripMapping SM;
      SM.m_ReadOutID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(0);
      SM.m_DetectorID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(1);
      const unsigned int Side = T->GetTokenAtAsUnsignedInt(2);
      SM.m_IsLowVoltage = (Side == 0 ? true : false);
      SM.m_StripNumber = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3);
      if (SM.m_DetectorID > c_MaxDetectorID) {
        if (g_Verbosity >= c_Error) cout << "MStripMap: Detector ID value " << SM.m_DetectorID << " is out of range [0, " << c_MaxDetectorID << "] in " << FileName << " at line " << i + 1 << endl;
        m_StripMappings.clear();
        return false;
      }
      if (Side > 1) {
        if (g_Verbosity >= c_Error) cout << "MStripMap: Side value " << Side << " is out of range [0, 1] in " << FileName << " at line " << i + 1 << endl;
        m_StripMappings.clear();
        return false;
      }
      if (SM.m_StripNumber > c_MaxStripNumber) {
        if (g_Verbosity >= c_Error) cout << "MStripMap: Strip number value " << SM.m_StripNumber << " is out of range [0, " << c_MaxStripNumber << "] in " << FileName << " at line " << i + 1 << endl;
        m_StripMappings.clear();
        return false;
      }
      
      // Infer the rest of the information from the ReadOutID
      SM.m_RTB = (SM.m_ReadOutID >> 8) & 0x01;
      SM.m_DRM = (SM.m_ReadOutID >> 7) & 0x01;
      SM.m_IsPrimary = (SM.m_ReadOutID >> 6) & 0x01;
      SM.m_ASICID = (SM.m_ReadOutID >> 5) & 0x01;
      SM.m_ChannelID = SM.m_ReadOutID & 0x1F;

      m_StripMappings.push_back(SM);
    } else {
      if (g_Verbosity >= c_Error) cout << "MStripMap: Invalid number of columns in " << FileName << " at line " << i + 1 << " (expected 4 or 9, got " << NTokens << ")" << endl;
      m_StripMappings.clear();
      return false;
    }
  }

  if (m_StripMappings.empty()) {
    if (g_Verbosity >= c_Error) cout << "MStripMap: No mapping entries found in " << FileName << endl;
    return false;
  }

  // Sort by m_ReadOutID:
  sort(m_StripMappings.begin(), m_StripMappings.end(), [](const MSingleStripMapping& A, const MSingleStripMapping& B) { return A.m_ReadOutID < B.m_ReadOutID; });

  for (unsigned int i = 1; i < m_StripMappings.size(); ++i) {
    if (m_StripMappings[i - 1].m_ReadOutID == m_StripMappings[i].m_ReadOutID) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: Duplicate read-out ID " << m_StripMappings[i].m_ReadOutID << " in " << FileName << endl;
      m_StripMappings.clear();
      return false;
    }
  }

  // build the map - it was already cleared on entry and nothing has filled it since
  m_DetSideStripToROI.reserve(m_StripMappings.size());
  for (const MSingleStripMapping& SM : m_StripMappings) {
    unsigned int Key = 0;
    if (ComputeDetSideStripKey(SM.m_DetectorID, SM.m_IsLowVoltage, SM.m_StripNumber, Key) == false) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: Unable to build a lookup key for detector " << SM.m_DetectorID << " strip " << SM.m_StripNumber << " in " << FileName << endl;
      m_StripMappings.clear();
      m_DetSideStripToROI.clear();
      return false;
    }
    if (m_DetSideStripToROI.find(Key) != m_DetSideStripToROI.end()) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: Duplicate detector/side/strip tuple in " << FileName << endl;
      m_StripMappings.clear();
      m_DetSideStripToROI.clear();
      return false;
    }
    m_DetSideStripToROI[Key] = SM.m_ReadOutID;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Keep only the mappings belonging to the given detectors
bool MStripMap::RestrictToEnabledDetectors(const vector<unsigned int>& DetectorIDs)
{
  vector<MSingleStripMapping> Kept;
  Kept.reserve(m_StripMappings.size());
  for (const MSingleStripMapping& SM : m_StripMappings) {
    if (find(DetectorIDs.begin(), DetectorIDs.end(), SM.m_DetectorID) != DetectorIDs.end()) {
      Kept.push_back(SM);
    }
  }

  if (Kept.empty() == true) {
    if (g_Verbosity >= c_Error) cout << "MStripMap: No strip map entries remain after restricting to the given detectors" << endl;
    return false;
  }

  // Dropping entries preserves both the read-out ID ordering and the uniqueness Open() established
  m_StripMappings = Kept;

  m_DetSideStripToROI.clear();
  m_DetSideStripToROI.reserve(m_StripMappings.size());
  for (const MSingleStripMapping& SM : m_StripMappings) {
    unsigned int Key = 0;
    if (ComputeDetSideStripKey(SM.m_DetectorID, SM.m_IsLowVoltage, SM.m_StripNumber, Key) == false) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: Unable to build a lookup key for detector " << SM.m_DetectorID << " strip " << SM.m_StripNumber << endl;
      m_StripMappings.clear();
      m_DetSideStripToROI.clear();
      return false;
    }
    m_DetSideStripToROI[Key] = SM.m_ReadOutID;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Update which ASICs are LV/HV depending on their polarities
bool MStripMap::UpdateASICPolarities(const vector<map<bool, vector<bool>>>& ASICPolarities)
{
  if (m_StripMappings.empty()) return true;

  vector<bool> IsLowVoltageSide(m_StripMappings.size());
  unordered_map<unsigned int, unsigned int> UpdatedDetSideStripToROI;
  UpdatedDetSideStripToROI.reserve(m_StripMappings.size());

  for (unsigned int i = 0; i < m_StripMappings.size(); ++i) {
    const MSingleStripMapping& S = m_StripMappings[i];

    if (S.m_DetectorID >= ASICPolarities.size()) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: No ASIC polarity data for detector " << S.m_DetectorID << endl;
      return false;
    }

    map<bool, vector<bool>>::const_iterator Primary = ASICPolarities[S.m_DetectorID].find(S.m_IsPrimary);
    if (Primary == ASICPolarities[S.m_DetectorID].end()) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: No ASIC polarity data for detector " << S.m_DetectorID << " primary " << S.m_IsPrimary << endl;
      return false;
    }

    if (S.m_ASICID >= Primary->second.size()) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: No ASIC polarity data for detector " << S.m_DetectorID << " primary " << S.m_IsPrimary << " ASIC " << S.m_ASICID << endl;
      return false;
    }

    IsLowVoltageSide[i] = Primary->second[S.m_ASICID];
    unsigned int Key = 0;
    if (ComputeDetSideStripKey(S.m_DetectorID, IsLowVoltageSide[i], S.m_StripNumber, Key) == false) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: Unable to build a lookup key for detector " << S.m_DetectorID << " strip " << S.m_StripNumber << endl;
      return false;
    }
    // A (detector, side, strip) tuple must map to exactly one read-out channel. Note that the GSE writes
    // a default polarity for every ASIC of a detector that was never configured, so the polarity data can
    // mark both sides of an unused detector as low voltage - the strip map must therefore only contain
    // the detectors the data was actually taken with, or this fires on detectors that were not part of
    // the run. See Issue #189.
    if (UpdatedDetSideStripToROI.find(Key) != UpdatedDetSideStripToROI.end()) {
      if (g_Verbosity >= c_Error) cout << "MStripMap: ASIC polarity update creates a duplicate detector/side/strip tuple for detector " << S.m_DetectorID << " strip " << S.m_StripNumber << endl;
      return false;
    }
    UpdatedDetSideStripToROI[Key] = S.m_ReadOutID;
  }

  for (unsigned int i = 0; i < m_StripMappings.size(); ++i) {
    m_StripMappings[i].m_IsLowVoltage = IsLowVoltageSide[i];
  }
  m_DetSideStripToROI = UpdatedDetSideStripToROI;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the given read-out ID is on file
bool MStripMap::HasReadOutID(unsigned int ROI) const
{
  auto Iter = lower_bound(m_StripMappings.begin(), m_StripMappings.end(), ROI, [](const MSingleStripMapping& SSM, unsigned int ID) { return SSM.m_ReadOutID < ID; });
  return Iter != m_StripMappings.end() && Iter->m_ReadOutID == ROI;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the index of the ROI, throw an exception otherwise
unsigned int MStripMap::GetReadOutIDIndex(unsigned int ROI) const
{
  auto Iter = lower_bound(m_StripMappings.begin(), m_StripMappings.end(), ROI, [](const MSingleStripMapping& SSM, unsigned int ID) { return SSM.m_ReadOutID < ID; });

  if (Iter != m_StripMappings.end() && Iter->m_ReadOutID == ROI) {
    return distance(m_StripMappings.begin(), Iter);
  } else {
    throw MExceptionValueNotFound(ROI, "vector of read-out IDs");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get detector by read-out ID
unsigned int MStripMap::GetDetectorID(unsigned int ROI) const
{
  return m_StripMappings[GetReadOutIDIndex(ROI)].m_DetectorID;
}


////////////////////////////////////////////////////////////////////////////////


//! Get detector side by read-out ID
bool MStripMap::IsLowVoltage(unsigned int ROI) const
{
  return m_StripMappings[GetReadOutIDIndex(ROI)].m_IsLowVoltage;
}


////////////////////////////////////////////////////////////////////////////////


//! Get strip ID by read-out ID
unsigned int MStripMap::GetStripNumber(unsigned int ROI) const
{
  return m_StripMappings[GetReadOutIDIndex(ROI)].m_StripNumber;
}


////////////////////////////////////////////////////////////////////////////////


//! Check whether (detector, side, strip) maps to a known read-out ID
bool MStripMap::HasROIDetSideStrip(unsigned int DetectorID, bool IsLowVoltage, unsigned int StripNumber) const
{
  unsigned int Key = 0;
  if (ComputeDetSideStripKey(DetectorID, IsLowVoltage, StripNumber, Key) == false) return false;

  return m_DetSideStripToROI.find(Key) != m_DetSideStripToROI.end();
}


////////////////////////////////////////////////////////////////////////////////


//! Reverse lookup: Ret read-out ID for a (detector, side, strip) tuple
unsigned int MStripMap::GetReadOutID(unsigned int DetectorID, bool IsLowVoltage, unsigned int StripNumber) const
{
  unsigned int Key = 0;
  if (ComputeDetSideStripKey(DetectorID, IsLowVoltage, StripNumber, Key) == false) {
    throw MExceptionValueNotFound(StripNumber, "(det/side/strip) tuple in strip map - detector or strip out of range");
  }

  auto Iter = m_DetSideStripToROI.find(Key);
  if (Iter == m_DetSideStripToROI.end()) {
    throw MExceptionValueNotFound(Key, "(det/side/strip) tuple in strip map");
  }
  return Iter->second;
}


// MStripMap.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
