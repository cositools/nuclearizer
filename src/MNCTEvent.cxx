/*
 * MNCTEvent.cxx
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
// MNCTEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTEvent.h"

// Standard libs:
#include <iomanip>
using namespace std;

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
//  mout<<"create MNCTEvent!!\n" ;//debug
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
  
//  mout<<"delete MNCTEvent!!\n" ;//debug
}


////////////////////////////////////////////////////////////////////////////////


void MNCTEvent::Clear()
{
  //! Reset all data

  m_ID = g_UnsignedIntNotDefined;
  m_TI = 0;
  m_CL = 0;
  m_FC = 0;
  m_Time = 0;
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
  m_AspectGood = true;

  for (int DetectorID = 0; DetectorID <= 9; DetectorID++) {
    m_InDetector[DetectorID] = false;
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


////////////////////////////////////////////////////////////////////////////////


bool MNCTEvent::Parse(MString& Line, int Version)
{
  //! Stream the content from a line of an ASCII file  

  /*
  
  bool Error = false;

  const char* Data = Line.Data();
  if (Data[0] == 'T' && Data[1] == 'I') {
    m_Time.Set(Data);
  } else if (Data[0] == 'I' && Data[1] == 'D') {
    if (sscanf(Data, "ID %lu", &m_ID) != 1) {
      Error = true;
    } 
  } else if (Data[0] == 'S' && Data[1] == 'H') {
    MNCTStripHit* H = new MNCTStripHit();
    if (SH->Parse(Line, Version) == true) {
      m_StripHits.push_back(SH);
    } else {
      delete H;
      Error = true;
    }
  } else {
    Error = true;
  }
  
  if (Error == true) {
    merr<<"Unable to parse line:"<<endl;
    merr<<Data<<endl;
    return false;
  }

  m_Empty = false;

  */
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTEvent::Stream(ofstream& S, int Version, int Mode)
{
  //! Stream the content to an ASCII file 

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;
  S<<"TI "<<m_Time<<endl;

  if (m_AspectAdded == true && m_AspectGood == true) {
    S<<"LT "<<setprecision(8)<<m_Latitude<<endl;
    S<<"LN "<<setprecision(8)<<m_Longitude<<endl;
    S<<"AL "<<setprecision(8)<<m_Altitude<<endl;
    S<<"GX "<<setprecision(8)<<m_GX[0]<<" "<<m_GX[1]<<endl;
    S<<"GZ "<<setprecision(8)<<m_GZ[0]<<" "<<m_GZ[1]<<endl;
    S<<"HX "<<setprecision(8)<<m_HX[0]<<" "<<m_HX[1]<<endl;
    S<<"HZ "<<setprecision(8)<<m_HZ[0]<<" "<<m_HZ[1]<<endl;
  }
  
  if (Mode == 0) { // Dat mode
    for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
      m_StripHits[h]->Stream(S, Version);  
    }
  } else { // evta mode
    for (unsigned int h = 0; h < m_Hits.size(); ++h) {
      m_Hits[h]->Stream(S, Version);  
    }
  }
  
  return true;
}


// MNCTEvent.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
