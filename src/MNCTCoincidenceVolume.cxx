/*
 * MNCTCoincidenceVolume.cxx
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
// MNCTCoincidenceVolume
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTCoincidenceVolume.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTStrip)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTCoincidenceVolume::MNCTCoincidenceVolume()
{
  // Construct an instance of MNCTStrip

  Clear();
}

////////////////////////////////////////////////////////////////////////////////
MNCTCoincidenceVolume::MNCTCoincidenceVolume(MDVolume* Coin_Vol):
m_Volume(Coin_Vol)
{

}

////////////////////////////////////////////////////////////////////////////////

MNCTCoincidenceVolume::MNCTCoincidenceVolume(MDVolume* Coin_Vol, double Threshold):
m_Volume(Coin_Vol), m_Threshold(Threshold)
{

}

////////////////////////////////////////////////////////////////////////////////


MNCTCoincidenceVolume::~MNCTCoincidenceVolume()
{
  // Delete this instance of MNCTStrip
}


////////////////////////////////////////////////////////////////////////////////


void MNCTCoincidenceVolume::Clear()
{
  // Reset all data
  m_Volume=NULL;
  m_Threshold=0;
}

// MNCTCoincidenceVolume.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
