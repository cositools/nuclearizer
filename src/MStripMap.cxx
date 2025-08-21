/*
 * MStripMap.cxx
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
  m_StripMappings.clear();

  MParser Parser;
  if (Parser.Open(FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<"MStripMap: Unable to load file: "<<endl<<FileName<<error;
    return false;
  }

  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (Parser.GetTokenizerAt(i)->GetTokenAtAsString(0).BeginsWith("#") == true) continue;
    if (Parser.GetTokenizerAt(i)->GetNTokens() == 9) {
      MSingleStripMapping SM;
      SM.m_ReadOutID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(0);
      SM.m_RTB = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(1);
      SM.m_DRM = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2);
      SM.m_IsPrimary = Parser.GetTokenizerAt(i)->GetTokenAtAsBoolean(3);
      SM.m_ASICID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(4);
      SM.m_ChannelID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(5);
      SM.m_DetectorID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(6);
      SM.m_IsLowVoltage = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(7) == 0 ? true : false;
      SM.m_StripNumber = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(8);
      m_StripMappings.push_back(SM);
    }
  }

  // Sort by m_ReadOutID:
  sort(m_StripMappings.begin(), m_StripMappings.end(), [](const MSingleStripMapping& A, const MSingleStripMapping& B) { return A.m_ReadOutID < B.m_ReadOutID; });

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


// MStripMap.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
