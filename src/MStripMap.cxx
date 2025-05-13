/*
 * MStripMap.cxx
 *
 *
 * Copyright (C) by YOUR NAME HERE.
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
// MStripMap
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MStripMap.h"

// Standard libs:

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


MStripMap::MStripMap()
{
  // Construct an instance of MStripMap
}


////////////////////////////////////////////////////////////////////////////////


MStripMap::~MStripMap()
{
  // Delete this instance of MStripMap
}


////////////////////////////////////////////////////////////////////////////////


//! Load a strip map
bool MStripMap::Open(MString FileName)
{
  m_ReadOutID.clear();
  m_RTB.clear();
  m_DRM.clear();
  m_IsPrimary.clear();
  m_ASICID.clear();
  m_ChannelID.clear();
  m_DetectorID.clear();
  m_IsLowVoltage.clear();
  m_StripNumber.clear();

  MParser Parser;
  if (Parser.Open(FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<"MStripMap: Unable to load file: "<<endl<<FileName<<error;
    return false;
  }

  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (Parser.GetTokenizerAt(i)->GetTokenAtAsString(0).BeginsWith("#") == true) continue;
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 9) {
      m_ReadOutID.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(0));
      m_RTB.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(1));
      m_DRM.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2));
      m_IsPrimary.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsBoolean(3));
      m_ASICID.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(4));
      m_ChannelID.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(5));
      m_DetectorID.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(6));
      m_IsLowVoltage.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(7) == 0 ? true : false);
      m_StripNumber.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(8));
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Get number of channels
unsigned int MStripMap::GetNumberOfReadOutIDs() const
{
  return m_ReadOutID.size();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the given read-out ID is on file
bool MStripMap::HasReadOutID(unsigned int ROI) const
{
  return binary_search(m_ReadOutID.begin(), m_ReadOutID.end(), ROI);
}

////////////////////////////////////////////////////////////////////////////////


//! Get detector by read out ID
unsigned int MStripMap::GetDetectorID(unsigned int ROI) const
{
  auto Iter = std::lower_bound(m_ReadOutID.begin(), m_ReadOutID.end(), ROI);
  if (Iter != m_ReadOutID.end() && *Iter == ROI) {
    size_t Index = std::distance(m_ReadOutID.begin(), Iter);
    return m_DetectorID[Index];
  } else {
    throw MExceptionValueNotFound(ROI, "vector of read-out IDs");
    return g_UnsignedIntNotDefined;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get detector side by read out ID
bool MStripMap::IsLowVoltage(unsigned int ROI) const
{
  auto Iter = std::lower_bound(m_ReadOutID.begin(), m_ReadOutID.end(), ROI);
  if (Iter != m_ReadOutID.end() && *Iter == ROI) {
    size_t Index = std::distance(m_ReadOutID.begin(), Iter);
    return m_IsLowVoltage[Index];
  } else {
    throw MExceptionValueNotFound(ROI, "vector of read-out IDs");
    return g_UnsignedIntNotDefined;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get strip ID by read out ID
unsigned int MStripMap::GetStripNumber(unsigned int ROI) const
{
  auto Iter = std::lower_bound(m_ReadOutID.begin(), m_ReadOutID.end(), ROI);
  if (Iter != m_ReadOutID.end() && *Iter == ROI) {
    size_t Index = std::distance(m_ReadOutID.begin(), Iter);
    return m_StripNumber[Index];
  } else {
    throw MExceptionValueNotFound(ROI, "vector of read-out IDs");
    return g_UnsignedIntNotDefined;
  }
}


// MStripMap.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
