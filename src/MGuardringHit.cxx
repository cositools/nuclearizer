/*
 * MGuardringHit.cxx
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
// MGuardringHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGuardringHit.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGuardringHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MGuardringHit::MGuardringHit()
{
  // Construct an instance of MGuardringHit

  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MGuardringHit::~MGuardringHit()
{
  // Delete this instance of MGuardringHit
}


////////////////////////////////////////////////////////////////////////////////


void MGuardringHit::Clear()
{
  // Reset all data

  m_DetectorID = g_UnsignedIntNotDefined;
  m_ADCUnits = g_DoubleNotDefined;
}


// MGuardringHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
