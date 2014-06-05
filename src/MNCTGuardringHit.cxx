/*
 * MNCTGuardringHit.cxx
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
// MNCTGuardringHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTGuardringHit.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTGuardringHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTGuardringHit::MNCTGuardringHit()
{
  // Construct an instance of MNCTGuardringHit

  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MNCTGuardringHit::~MNCTGuardringHit()
{
  // Delete this instance of MNCTGuardringHit
}


////////////////////////////////////////////////////////////////////////////////


void MNCTGuardringHit::Clear()
{
  // Reset all data

  m_DetectorID = g_UnsignedIntNotDefined;
  m_ADCUnits = g_DoubleNotDefined;
}


// MNCTGuardringHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
