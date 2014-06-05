/*
 * MNCTEvent.cxx
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
// MNCTEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTEvent.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTEvent::MNCTEvent()
{
  // Construct an instance of MNCTEvent

  m_PhysicalEvent = 0; // Set pointer to zero before delete

  Clear();
//  mout << "create MNCTEvent!!\n" ;//debug
}


////////////////////////////////////////////////////////////////////////////////


MNCTEvent::~MNCTEvent()
{
  // Delete all strip hits
  for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
    delete m_StripHits[h];
  }
  m_StripHits.clear();

  // Delete all hits
  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    delete m_Hits[h];
  }
  m_Hits.clear();

  // Delete all hits from simulation
  for (unsigned int h = 0; h < m_HitsSim.size(); ++h) {
    delete m_HitsSim[h];
  }
  m_HitsSim.clear();


  // Delete all guardring hits
  for (unsigned int h = 0; h < m_GuardringHits.size(); ++h) {
    delete m_GuardringHits[h];
  }
  m_GuardringHits.clear();

  // Delete this instance of MNCTEvent
  delete m_PhysicalEvent;
  
//  mout << "delete MNCTEvent!!\n" ;//debug
}


////////////////////////////////////////////////////////////////////////////////


void MNCTEvent::Clear()
{
  //! Reset all data

  m_ID = 0;
  m_TI = 0;
  m_CL = 0;
  m_FC = 0;
  m_Time = 0.0;
  m_MJD = 0.0;
  m_Latitude = 0.0;
  m_Longitude = 0.0;
  m_Altitude = 0.0;
  m_GX.clear();
  m_GZ.clear();
  m_HX.clear();
  m_HZ.clear();

  m_Veto = false;
  m_Trigger = true;

  for (int DetectorID=0; DetectorID<=9; DetectorID++)
    {
      m_InDetector[DetectorID]=false;
    }

  m_StripHits.clear();
 // Delete all hits
  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    delete m_Hits[h];
  }
  m_Hits.clear();

 // Delete all hits from simulation
  for (unsigned int h = 0; h < m_HitsSim.size(); ++h) {
    delete m_HitsSim[h];
  }
  m_HitsSim.clear();

  // Delete all guardring hits
  for (unsigned int h = 0; h < m_GuardringHits.size(); ++h) {
    delete m_GuardringHits[h];
  }
  m_GuardringHits.clear();

  m_DataRead = false;
  m_EnergyCalibrated = false;
  m_ChargeSharingCorrected = false;
  m_DepthCalibrated = false;
  m_StripsPaired = false;
  m_AspectAdded = false;

  delete m_PhysicalEvent;
  m_PhysicalEvent = 0;
}


////////////////////////////////////////////////////////////////////////////////
void MNCTEvent::DeleteHits()
{
  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    delete m_Hits[h];
  }
  m_Hits.clear();
}


////////////////////////////////////////////////////////////////////////////////

bool MNCTEvent::InDetector(int DetectorID)
{
  //! Find out if the event contains strip hits in a given detector
  if ( (DetectorID>=0) && (DetectorID<=9) )
    {
      return m_InDetector[DetectorID];
    }
  else
    {
      return false;
    }
}


////////////////////////////////////////////////////////////////////////////////


void MNCTEvent::SetPhysicalEvent(MPhysicalEvent* Event)
{
  //! Set the physical event from event reconstruction
  // We make our own local copy here

  m_PhysicalEvent = Event->Duplicate();
}


////////////////////////////////////////////////////////////////////////////////


MNCTStripHit* MNCTEvent::GetStripHit(unsigned int i) 
{ 
  //! Return strip hit i

  if (i < m_StripHits.size()) {
    return m_StripHits[i];
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////

void MNCTEvent::AddStripHit(MNCTStripHit* StripHit)
{
  //! Add a strip hit
  int DetectorID = StripHit->GetDetectorID();
  if ( (DetectorID>=0) && (DetectorID<=9) )
    {
      m_InDetector[DetectorID]=true;
    }
  return m_StripHits.push_back(StripHit);
}


////////////////////////////////////////////////////////////////////////////////

void MNCTEvent::RemoveStripHit(unsigned int i)
{
  //! Remove a strip hit
  if ( (i>=0) && (i<m_StripHits.size()) )
    {
      vector<MNCTStripHit*>::iterator it;
      it = m_StripHits.begin()+i;
      m_StripHits.erase(it);
    }
}


////////////////////////////////////////////////////////////////////////////////


MNCTHit* MNCTEvent::GetHit(unsigned int i) 
{ 
  //! Return hit i
  
  if (i < m_Hits.size()) {
    return m_Hits[i]; 
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
MNCTHit* MNCTEvent::GetHitSim(unsigned int i) 
{ 
  //! Return hit i
  
  if (i < m_HitsSim.size()) {
    return m_HitsSim[i]; 
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}

// MNCTEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
