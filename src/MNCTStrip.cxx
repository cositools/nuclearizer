/*
 * MNCTStrip.cxx
 *
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Jau-Shian Liang.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTStrip
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTStrip.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNCTStrip)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTStrip::MNCTStrip()
{
  // Construct an instance of MNCTStrip

  Clear();
}


////////////////////////////////////////////////////////////////////////////////

MNCTStrip::MNCTStrip(int DetectorID, bool IsXStrip, int StripID):
m_DetectorID(DetectorID), m_IsXStrip(IsXStrip), m_StripID(StripID)
{

}

////////////////////////////////////////////////////////////////////////////////


MNCTStrip::~MNCTStrip()
{
  // Delete this instance of MNCTStrip
}


////////////////////////////////////////////////////////////////////////////////


void MNCTStrip::Clear()
{
  // Reset all data

  m_DetectorID = g_UnsignedIntNotDefined;
  m_StripID = g_UnsignedIntNotDefined;
  m_IsXStrip = true;
}

////////////////////////////////////////////////////////////////////////////////

bool MNCTStrip::operator==(const MNCTStrip& Strip)
{
  bool tf=false;
  if(m_DetectorID == Strip.GetDetectorID()
     && m_IsXStrip == Strip.IsXStrip()
     && m_StripID == Strip.GetStripID())tf=true;
  return tf;
}

////////////////////////////////////////////////////////////////////////////////
void MNCTStrip::operator=(const MNCTStrip& Strip)
{
  m_DetectorID = Strip.GetDetectorID();
  m_IsXStrip = Strip.IsXStrip();
  m_StripID = Strip.GetStripID();
}

// MNCTStrip.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
