/*
 * MReadOutAssembly.cxx
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
// MReadOutAssembly
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutAssembly.h"

// Standard libs:
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly::MReadOutAssembly() : m_Time(0)
{
  // Construct an instance of MReadOutAssembly

  m_PhysicalEvent = 0; // Set pointer to zero before delete
  m_Aspect = 0;
  
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly::~MReadOutAssembly()
{
  // Delete all strip hits
  for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
    delete m_StripHits[h];
  }
  m_StripHits.clear();

  // Delete all TOnly strip hits
  for (unsigned int h = 0; h < m_StripHitsTOnly.size(); ++h) {
    delete m_StripHitsTOnly[h];
  }
  m_StripHitsTOnly.clear();

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

  // Delete this instance of MReadOutAssembly
  delete m_PhysicalEvent;
  
  delete m_Aspect;
//  mout<<"delete MReadOutAssembly!!\n" ;//debug
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::Clear()
{
  //! Reset all data

  m_ID = g_UnsignedIntNotDefined;
  m_TI = 0;
  m_CL = 0;
  m_FC = 0;
  m_Time = 0;
  m_MJD = 0.0;

  m_Veto = false;
  m_Trigger = true;

  for (int DetectorID = 0; DetectorID <= 11; DetectorID++) {
    m_InDetector[DetectorID] = false;
  }

  // Delete all strip hits
  for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
    delete m_StripHits[h];
  }
  m_StripHits.clear();

  for (unsigned int h = 0; h < m_StripHitsTOnly.size(); ++h) {
    delete m_StripHitsTOnly[h];
  }
  m_StripHitsTOnly.clear();

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

  m_AspectIncomplete = false;
  m_AspectIncompleteString = "";
  m_TimeIncomplete = false;
  m_TimeIncompleteString = "";
  m_EnergyCalibrationIncomplete_BadStrip = false;
  m_EnergyCalibrationIncomplete_BadStripString = "";
  m_EnergyCalibrationIncomplete = false;
  m_EnergyCalibrationIncompleteString = "";
  m_EnergyResolutionCalibrationIncomplete = false;
  m_EnergyResolutionCalibrationIncompleteString = "";
  m_StripPairingIncomplete = false;
  m_StripPairingIncompleteString = "";
  m_LLDEvent = false;
  m_LLDEventString = "";
  m_DepthCalibrationIncomplete = false;
  m_DepthCalibrationIncompleteString = "";
  m_DepthCalibration_OutofRange = false;
  m_DepthCalibration_OutofRangeString = ""; 

  
  m_FilteredOut = false;

  delete m_PhysicalEvent;
  m_PhysicalEvent = 0;
  
  delete m_Aspect;
  m_Aspect = 0;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::DeleteHits()
{
  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    delete m_Hits[h];
  }
  m_Hits.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::InDetector(int DetectorID)
{
  //! Find out if the event contains strip hits in a given detector
  if ( (DetectorID>=0) && (DetectorID<=11) )
    {
      return m_InDetector[DetectorID];
    }
  else
    {
      return false;
    }
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::SetPhysicalEvent(MPhysicalEvent* Event)
{
  //! Set the physical event from event reconstruction
  // We make our own local copy here

  m_PhysicalEvent = Event->Duplicate();
}


////////////////////////////////////////////////////////////////////////////////


MNCTStripHit* MReadOutAssembly::GetStripHit(unsigned int i) 
{ 
  //! Return strip hit i

  if (i < m_StripHits.size()) {
    return m_StripHits[i];
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void MReadOutAssembly::AddStripHit(MNCTStripHit* StripHit)
{
  //! Add a strip hit
  int DetectorID = StripHit->GetDetectorID();
  if ( (DetectorID>=0) && (DetectorID<=11) )
    {
      m_InDetector[DetectorID]=true;
    }
  return m_StripHits.push_back(StripHit);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveStripHit(unsigned int i)
{
  //! Remove a strip hit
  if (i < m_StripHits.size()) {
    vector<MNCTStripHit*>::iterator it;
    it = m_StripHits.begin()+i;
    m_StripHits.erase(it);
  }
}


////////////////////////////////////////////////////////////////////////////////


MNCTStripHit* MReadOutAssembly::GetStripHitTOnly(unsigned int i) 
{ 
  //! Return strip hit i

  if (i < m_StripHitsTOnly.size()) {
    return m_StripHitsTOnly[i];
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void MReadOutAssembly::AddStripHitTOnly(MNCTStripHit* StripHit)
{
  //! Add a strip hit
	//comment this out for now since it might mess with other stuff
	/*
  int DetectorID = StripHit->GetDetectorID();
  if ( (DetectorID>=0) && (DetectorID<=11) )
    {
      m_InDetector[DetectorID]=true;
    }*/
  return m_StripHitsTOnly.push_back(StripHit);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveStripHitTOnly(unsigned int i)
{
  //! Remove a strip hit
  if (i < m_StripHitsTOnly.size()) {
    vector<MNCTStripHit*>::iterator it;
    it = m_StripHitsTOnly.begin()+i;
    m_StripHitsTOnly.erase(it);
  }
}


////////////////////////////////////////////////////////////////////////////////


MNCTHit* MReadOutAssembly::GetHit(unsigned int i) 
{ 
  //! Return hit i
  
  if (i < m_Hits.size()) {
    return m_Hits[i]; 
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveHit(unsigned int i)
{
  //! Remove a strip hit
  if (i < m_Hits.size()) {
    m_Hits.erase(m_Hits.begin()+i);
  }
}


////////////////////////////////////////////////////////////////////////////////


MNCTHit* MReadOutAssembly::GetHitSim(unsigned int i) 
{ 
  //! Return hit i
  
  if (i < m_HitsSim.size()) {
    return m_HitsSim[i]; 
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::Parse(MString& Line, int Version)
{

  if( Line.BeginsWith("SE") ) return true;
  if( Line.BeginsWith("TI") ){
    m_Time.Set(Line);
    return true;
  }
  //skipping aspect for now
  if( Line.BeginsWith("HT") ){
    MNCTHit* h = new MNCTHit();
    if( h->Parse(Line,1) ){
      AddHit(h);
      return true;
    } else {
      return false;
    }
  }
  if( Line.BeginsWith("SH") ){
    //assuming that the SHs belong to the last read hit
    MNCTHit* h = m_Hits.back();
    if( h != NULL ){
      MNCTStripHit* SH = new MNCTStripHit();
      if( SH->Parse(Line,2) ){
        h->AddStripHit(SH);
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }
  if( Line.BeginsWith("BD") ){
    //set a bad flag
    //too lazy RN to go thru each flag.  the following should do::
    m_FilteredOut = true;
    return true;
  }

  return false;

}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::GetNextFromDatFile(MFile &F){

  MString Line;
  int i;
  int MaxIter = 1000;

	Clear();
	for( i = 0; i < MaxIter; i++ ){
		//try 1000 times to get the complete event
		F.ReadLine(Line);
		const char* line = Line.Data();
		//vector<MString> tokens = Line.Tokenize(" ");
		if( Line.BeginsWith("SE") ){
			if( i != 0 ){
				//we read the full event in, break now
				break;
			}
		}else if( Line.BeginsWith("ID") ){
			unsigned int ID;
			sscanf(&line[3],"%u",&ID);
			SetID( ID );
		} else if( Line.BeginsWith("TI") ){
			MTime T = MTime();
			T.Set(Line);
			SetTime( T );
		} else if( Line.BeginsWith("HT") ){
			MNCTHit* h = new MNCTHit();
			h->Parse(Line);
			AddHit(h);
		} else if( Line.BeginsWith("SH") ){
			MNCTStripHit* sh = new MNCTStripHit();
			sh->Parse(Line);
			AddStripHit(sh);
			if( m_Hits.size() > 0 ){
				//add this SH to the last read in HT
				MNCTHit* h = m_Hits.back();
				h->AddStripHit(sh);
			}
		} else if( Line.BeginsWith("BD") ){
			SetFilteredOut(true);
		}
		//ignoring ASPECT info for right now

  }

  if( i == MaxIter ){
    cout<<"MReadoutAssembly::GetNextFromFile(): reached MaxIter"<<endl;
    return false;
  } else {
    return true;
  }

}
      









////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::StreamDat(ostream& S, int Version)
{
  //! Stream the content to an ASCII file 

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;
  S<<"TI "<<m_Time<<endl;

  if (m_Aspect != 0) {
    m_Aspect->StreamDat(S, Version);
  }
  
  if( Version == 1 ){
    for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
      m_StripHits[h]->StreamDat(S, Version);  
    }

    for (unsigned int h = 0; h < m_Hits.size(); ++h) {
      m_Hits[h]->StreamDat(S, Version);  
    }
  } else if( Version == 2 ){
    for( auto H : m_Hits ){
      H->StreamDat(S,2);
    }
  }

  if (m_AspectIncomplete == true) {
    S<<"BD AspectIncomplete";
    if (m_AspectIncompleteString != "") S<<" ("<<m_AspectIncompleteString<<")";
    S<<endl;
  }
  if (m_TimeIncomplete == true) {
    S<<"BD TimeIncomplete";
    if (m_TimeIncompleteString != "") S<<" ("<<m_TimeIncompleteString<<")";
    S<<endl;
  }
  if (m_EnergyCalibrationIncomplete_BadStrip == true) {
    S<<"BD EnergyCalibrationIncomplete_BadStrip";
    if (m_EnergyCalibrationIncomplete_BadStripString != "") S<<" ("<<m_EnergyCalibrationIncomplete_BadStripString<<")";
    S<<endl;
  }
  if (m_EnergyCalibrationIncomplete == true) {
  S<<"BD EnergyCalibrationIncomplete";
  if (m_EnergyCalibrationIncompleteString != "") S<<" ("<<m_EnergyCalibrationIncompleteString<<")";
  S<<endl;
  }
  if (m_EnergyResolutionCalibrationIncomplete == true) {
    S<<"BD EnergyResolutionCalibrationIncomplete";
    if (m_EnergyResolutionCalibrationIncompleteString != "") S<<" ("<<m_EnergyResolutionCalibrationIncompleteString<<")";
    S<<endl;
  }
  if (m_StripPairingIncomplete == true) {
    S<<"BD StripPairingIncomplete";
    if (m_StripPairingIncompleteString != "") S<<" ("<<m_StripPairingIncompleteString<<")";
    S<<endl;
  }
  if (m_LLDEvent == true) {
  S<<"BD LLDEvent";
  if (m_LLDEventString != "") S<<" ("<<m_LLDEventString<<")";
    S<<endl;
  }
  if (m_DepthCalibrationIncomplete == true) {
    S<<"BD DepthCalibrationIncomplete";
    if (m_DepthCalibrationIncompleteString != "") S<<" ("<<m_DepthCalibrationIncompleteString<<")";
    S<<endl;
  }
  if (m_DepthCalibration_OutofRange == true) {
  S<<"BD DepthCalibration_OutofRange";
  if (m_DepthCalibration_OutofRangeString != "") S<<" ("<<m_DepthCalibration_OutofRangeString<<")";
  S<<endl;
  }


  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamEvta(ostream& S)
{
  //! Stream the content in MEGAlib's evta format 

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;
  S<<"TI "<<m_Time<<endl;

  if (m_Aspect != 0) {
    m_Aspect->StreamEvta(S);
  }
  
  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    m_Hits[h]->StreamEvta(S);  
  }
  
  if (m_AspectIncomplete == true) {
    S<<"BD AspectIncomplete";
    if (m_AspectIncompleteString != "") S<<" ("<<m_AspectIncompleteString<<")";
    S<<endl;
  }
  if (m_TimeIncomplete == true) {
    S<<"BD TimeIncomplete";
    if (m_TimeIncompleteString != "") S<<" ("<<m_TimeIncompleteString<<")";
    S<<endl;
  }
  if (m_EnergyCalibrationIncomplete_BadStrip == true) {
    S<<"BD EnergyCalibrationIncomplete_BadStrip";
    if (m_EnergyCalibrationIncomplete_BadStripString != "") S<<" ("<<m_EnergyCalibrationIncomplete_BadStripString<<")";
    S<<endl;
  }
  if (m_EnergyCalibrationIncomplete == true) {
    S<<"BD EnergyCalibrationIncomplete";
    if (m_EnergyCalibrationIncompleteString != "") S<<" ("<<m_EnergyCalibrationIncompleteString<<")";
    S<<endl;
  }
  if (m_EnergyResolutionCalibrationIncomplete == true) {
    S<<"BD EnergyResolutionCalibrationIncomplete";
    if (m_EnergyResolutionCalibrationIncompleteString != "") S<<" ("<<m_EnergyResolutionCalibrationIncompleteString<<")";
    S<<endl;
  }
  if (m_StripPairingIncomplete == true) {
    S<<"BD StripPairingIncomplete";
    if (m_StripPairingIncompleteString != "") S<<" ("<<m_StripPairingIncompleteString<<")";
    S<<endl;
  }
  if (m_LLDEvent == true) {
    S<<"BD LLDEvent";
    if (m_LLDEventString != "") S<<" ("<<m_LLDEventString<<")";
    S<<endl;
  }
  if (m_DepthCalibrationIncomplete == true) {
    S<<"BD DepthCalibrationIncomplete";
    if (m_DepthCalibrationIncompleteString != "") S<<" ("<<m_DepthCalibrationIncompleteString<<")";
    S<<endl;
  }
  if (m_DepthCalibration_OutofRange == true) { 
    S<<"BD DepthCalibration_OutofRange";
    if (m_DepthCalibration_OutofRangeString != "") S<<" ("<<m_DepthCalibration_OutofRangeString<<")";
    S<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamRoa(ostream& S, bool)
{
  //! Stream the content in MEGAlib's evta format 

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;
  S<<"TI "<<m_Time<<endl;

  if (m_Aspect != 0) {
    m_Aspect->StreamEvta(S);
  }

  for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
    m_StripHits[h]->StreamRoa(S);  
  }
  
  // Those are the only BD's relevant for the roa format
  if (m_AspectIncomplete == true) {
    S<<"BD AspectIncomplete";
    if (m_AspectIncompleteString != "") S<<" ("<<m_AspectIncompleteString<<")";
    S<<endl;
  }
  if (m_TimeIncomplete == true) {
    S<<"BD TimeIncomplete";
    if (m_TimeIncompleteString != "") S<<" ("<<m_TimeIncompleteString<<")";
    S<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::IsGood() const
{
  //! Returns true if none of the "bad" or "incomplete" falgs has been set

  if (m_AspectIncomplete == true) return false;
  if (m_TimeIncomplete == true) return false;
  if (m_EnergyCalibrationIncomplete_BadStrip == true) return false;
  if (m_EnergyCalibrationIncomplete == true) return false;
  if (m_EnergyResolutionCalibrationIncomplete == true) return false;
  if (m_StripPairingIncomplete == true) return false;
  if (m_LLDEvent == true) return false;
  if (m_DepthCalibrationIncomplete == true) return false;
  if (m_DepthCalibration_OutofRange == true) return false;


  if (m_FilteredOut == true) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::IsBad() const
{
  //! Returns true if none of the "bad" or "incomplete" falgs has been set

  if (m_AspectIncomplete == true) return true;
  if (m_TimeIncomplete == true) return true;
  if (m_EnergyCalibrationIncomplete_BadStrip == true) return true;
  if (m_EnergyCalibrationIncomplete == true) return true;
  if (m_EnergyResolutionCalibrationIncomplete == true) return true;
  if (m_StripPairingIncomplete == true) return true;
  if (m_LLDEvent == true) return true;
  if (m_DepthCalibrationIncomplete == true) return true;
  if (m_DepthCalibration_OutofRange == true) return true;

  return false;
}
  

// MReadOutAssembly.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
