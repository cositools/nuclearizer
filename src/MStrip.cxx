/*
 * MStrip.cxx
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
// MStrip
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MStrip.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MStrip)
#endif


////////////////////////////////////////////////////////////////////////////////


MStrip::MStrip()
{
  // Construct an instance of MStrip

  Clear();
}


////////////////////////////////////////////////////////////////////////////////

MStrip::MStrip(int DetectorID, bool IsXStrip, int StripID):
m_DetectorID(DetectorID), m_IsXStrip(IsXStrip), m_StripID(StripID)
{

}

////////////////////////////////////////////////////////////////////////////////


MStrip::~MStrip()
{
  // Delete this instance of MStrip
}


////////////////////////////////////////////////////////////////////////////////


void MStrip::Clear()
{
  // Reset all data

  m_DetectorID = g_UnsignedIntNotDefined;
  m_StripID = g_UnsignedIntNotDefined;
  m_IsXStrip = true;
}

////////////////////////////////////////////////////////////////////////////////

bool MStrip::operator==(const MStrip& Strip)
{
  bool tf=false;
  if(m_DetectorID == Strip.GetDetectorID()
     && m_IsXStrip == Strip.IsXStrip()
     && m_StripID == Strip.GetStripID())tf=true;
  return tf;
}

////////////////////////////////////////////////////////////////////////////////
void MStrip::operator=(const MStrip& Strip)
{
  m_DetectorID = Strip.GetDetectorID();
  m_IsXStrip = Strip.IsXStrip();
  m_StripID = Strip.GetStripID();
}

// MStrip.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
