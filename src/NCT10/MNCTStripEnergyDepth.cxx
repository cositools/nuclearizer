/*
 * MNCTStripEnergyDepth.cxx
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
// MNCTStripEnergyDepth
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTStripEnergyDepth.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNCTStripEnergyDepth)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTStripEnergyDepth::MNCTStripEnergyDepth()
{
  // Construct an instance of MNCTStripEnergyDepth

  m_Energy = g_DoubleNotDefined;
  m_Depth = g_DoubleNotDefined;
}

////////////////////////////////////////////////////////////////////////////////
MNCTStripEnergyDepth::MNCTStripEnergyDepth(MNCTStrip strip, double energy, double depth):
m_Strip(strip), m_Energy(energy), m_Depth(depth)
{


}


////////////////////////////////////////////////////////////////////////////////


MNCTStripEnergyDepth::~MNCTStripEnergyDepth()
{
  // Delete this instance of MNCTStripEnergyDepth
}


////////////////////////////////////////////////////////////////////////////////


void MNCTStripEnergyDepth::Clear()
{
  // Reset all data

  m_Strip.Clear();
  m_Energy = g_DoubleNotDefined;
  m_Depth = g_DoubleNotDefined;
}


// MNCTStripEnergyDepth.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
