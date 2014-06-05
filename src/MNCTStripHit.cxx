/*
 * MNCTStripHit.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
// MNCTStripHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTStripHit.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTStripHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTStripHit::MNCTStripHit()
{
  // Construct an instance of MNCTStripHit

  Clear();
//  mout << "creat StripHit!! \n";
}


////////////////////////////////////////////////////////////////////////////////


MNCTStripHit::~MNCTStripHit()
{
  // Delete this instance of MNCTStripHit
//  mout << "delete StripHit!! \n";
}


////////////////////////////////////////////////////////////////////////////////


void MNCTStripHit::Clear()
{
  // Reset all data

  m_DetectorID = g_UnsignedIntNotDefined;
  m_StripID = g_UnsignedIntNotDefined;
  m_IsXStrip = true;
  m_ADCUnits = g_DoubleNotDefined;
  m_Energy = g_DoubleNotDefined;
  m_Timing = g_DoubleNotDefined;
}


// MNCTStripHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
