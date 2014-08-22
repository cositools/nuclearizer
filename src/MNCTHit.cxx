/*
 * MNCTHit.cxx
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
// MNCTHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTHit.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTHit::MNCTHit()
{
  // Construct an instance of MNCTHit

  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MNCTHit::~MNCTHit()
{
  // Delete this instance of MNCTHit
  
  // This strip hits are not deleted since they where not generated here
}


////////////////////////////////////////////////////////////////////////////////


void MNCTHit::Clear()
{
  // Reset all data

  m_Position = g_VectorNotDefined;
  m_Energy = g_DoubleNotDefined;
  m_PositionResolution = g_VectorNotDefined;
  m_EnergyResolution = g_DoubleNotDefined;

  m_StripHits.clear();
}


////////////////////////////////////////////////////////////////////////////////


MNCTStripHit* MNCTHit::GetStripHit(unsigned int i) 
{ 
  //! Return strip hit i

  if (i < m_StripHits.size()) {
    return m_StripHits[i];
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTHit::StreamDat(ostream& S, int Version)
{
  //! Stream the content to an ASCII file 
  
  S<<"HT "<<m_Position.GetX()<<" "<<m_Position.GetY()<<" "<<m_Position.GetZ()<<" "<<m_Energy<<endl;
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTHit::StreamEvta(ostream& S)
{
  //! Stream the content to an ASCII file 
  
  S<<"HT 3;"<<m_Position.GetX()<<";"<<m_Position.GetY()<<";"<<m_Position.GetZ()<<";"<<m_Energy<<endl;
}


// MNCTHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
