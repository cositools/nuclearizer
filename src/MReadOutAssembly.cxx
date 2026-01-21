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


#ifdef ___CLING___
ClassImp(MReadOutAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly::MReadOutAssembly() : MReadOutSequence(), m_EventTimeUTC(0)
{
  // Construct an instance of MReadOutAssembly

  m_PhysicalEvent = nullptr;
  m_SimEvent = nullptr;
  m_Aspect = nullptr;
  m_HasSimAspectInfo = false;
 
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

  // Delete all DEE Strip hits
  m_DEEStripHitsLV.clear();
  m_DEEStripHitsHV.clear();
  m_DEECrystalHits.clear();

  // Delete all crystal hits
  for (unsigned int h = 0; h < m_CrystalHits.size(); ++h) {
    delete m_CrystalHits[h];
  }
  m_CrystalHits.clear();

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

  // Delete all Events
  delete m_SimEvent;
  delete m_PhysicalEvent;
  delete m_Aspect;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::Clear()
{
  //! Reset all data

  MReadOutSequence::Clear();
  
  m_ID = g_UnsignedIntNotDefined;
  m_TI = 0;
  m_CL = 0;
  m_FC = 0;
  m_Time = 0;
  m_EventTimeUTC = 0;
  m_MJD = 0.0;

  m_ShieldVeto = false;
  m_GuardRingVeto = false;
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

  for (unsigned int h = 0; h < m_CrystalHits.size(); ++h) {
    delete m_CrystalHits[h];
  }
  m_CrystalHits.clear();


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

  // Delete all event flags and associated variables
  m_EnergyCalibrationError = false;
  m_EnergyCalibrationErrorString = "";
  m_StripPairingError = false;
  m_StripPairingErrorString = "";
  m_DepthCalibrationError = false;
  m_DepthCalibrationErrorString = "";
  m_EventReconstructionError = false;
  m_EventReconstructionErrorString = "";
  
  m_ReducedChiSquare = -1; 
 
  m_StripHitBelowThreshold_QualityFlag = false;
  m_StripHitBelowThresholdString_QualityFlag = "";
  m_StripHitBelowThreshold_Energy = 0;
  m_StripHitBelowThreshold_Number = 0;

  m_FilteredOut = false;

  delete m_PhysicalEvent;
  m_PhysicalEvent = nullptr;

  m_DEEStripHitsLV.clear();
  m_DEEStripHitsHV.clear();
  m_DEECrystalHits.clear();

  delete m_SimEvent;
  m_SimEvent = nullptr;

  delete m_Aspect;
  m_Aspect = nullptr;
}//


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


MStripHit* MReadOutAssembly::GetStripHit(unsigned int i) 
{ 
  //! Return strip hit i

  if (i < m_StripHits.size()) {
    return m_StripHits[i];
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::AddStripHit(MStripHit* StripHit)
{
  //! Add a strip hit
  int DetectorID = StripHit->GetDetectorID();
  if ( (DetectorID>=0) && (DetectorID<=11) ) {
    m_InDetector[DetectorID]=true;
  }
  m_StripHits.push_back(StripHit);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveStripHit(unsigned int i)
{
  //! Remove a strip hit
  if (i < m_StripHits.size()) {
    vector<MStripHit*>::iterator it;
    it = m_StripHits.begin()+i;
    m_StripHits.erase(it);
  }
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* MReadOutAssembly::GetStripHitTOnly(unsigned int i) 
{ 
  //! Return strip hit i

  if (i < m_StripHitsTOnly.size()) {
    return m_StripHitsTOnly[i];
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::AddStripHitTOnly(MStripHit* StripHit)
{
  //! Add a strip hit
	//comment this out for now since it might mess with other stuff
	/*
  int DetectorID = StripHit->GetDetectorID();
  if ( (DetectorID>=0) && (DetectorID<=11) )
    {
      m_InDetector[DetectorID]=true;
    }*/
  m_StripHitsTOnly.push_back(StripHit);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveStripHitTOnly(unsigned int i)
{
  //! Remove a strip hit
  if (i < m_StripHitsTOnly.size()) {
    vector<MStripHit*>::iterator it;
    it = m_StripHitsTOnly.begin()+i;
    m_StripHitsTOnly.erase(it);
  }
}


////////////////////////////////////////////////////////////////////////////////


MCrystalHit* MReadOutAssembly::GetCrystalHit(unsigned int i)
{
  //! Return strip hit i

  if (i < m_CrystalHits.size()) {
    return m_CrystalHits[i];
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::AddCrystalHit(MCrystalHit* CrystalHit)
{
  //! Add a strip hit
  int DetectorID = CrystalHit->GetDetectorID();
  if ( (DetectorID>=0) && (DetectorID<=11) ) {
    m_InDetector[DetectorID]=true;
  }
  m_CrystalHits.push_back(CrystalHit);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::RemoveCrystalHit(unsigned int i)
{
  //! Remove a strip hit
  if (i < m_CrystalHits.size()) {
    vector<MCrystalHit*>::iterator it;
    it = m_CrystalHits.begin()+i;
    m_CrystalHits.erase(it);
  }
}


////////////////////////////////////////////////////////////////////////////////


MHit* MReadOutAssembly::GetHit(unsigned int i) 
{ 
  //! Return hit i
  
  if (i < m_Hits.size()) {
    return m_Hits[i]; 
  }

  merr<<"Index out of bounds!"<<show;

  return nullptr;
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


bool MReadOutAssembly::Parse(MString& Line, int Version)
{  
  // Handles SE, TI, RO, IA
  if (MReadOutSequence::Parse(Line) == true) return true;
  
  /* In base class
  if (Line.BeginsWith("SE")) return true;
  if (Line.BeginsWith("TI")) {
    m_Time.Set(Line);
    return true;
  }
  */
  // skipping aspect for now
  if (Line.BeginsWith("HT")) {
    MHit* h = new MHit();
    if( h->Parse(Line,1) ){
      AddHit(h);
      return true;
    } else {
      return false;
    }
  }
  if (Line.BeginsWith("SH")) {
    // assuming that the SHs belong to the last read hit
    MHit* h = m_Hits.back();
    if (h != nullptr) {
      MStripHit* SH = new MStripHit();
      if (SH->Parse(Line, 2)) {
        h->AddStripHit(SH);
        return true;
      } else {
        delete SH;
        return false;
      }
    } else {
      return false;
    }
  }
  if (Line.BeginsWith("BD")) {
    // set a bad flag
    // too lazy RN to go thru each flag.  the following should do::
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
			MHit* h = new MHit();
			h->Parse(Line);
			AddHit(h);
		} else if( Line.BeginsWith("SH") ){
			MStripHit* sh = new MStripHit();
			sh->Parse(Line);
			AddStripHit(sh);
			if( m_Hits.size() > 0 ){
				//add this SH to the last read in HT
				MHit* h = m_Hits.back();
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
  S<<"CL "<<m_Time<<endl;
  S<<"TI "<<m_EventTimeUTC<<endl;
  S<<"QP "<<m_ReducedChiSquare<<endl; // Read out strip pairing qualiy factor
    
  for (MSimIA& IA: m_SimIAs) {
    S<<IA.ToSimString()<<endl; 
  }
  
  if (m_Aspect != 0) {
    m_Aspect->StreamDat(S, Version);
  }
  
  if (Version == 1) {
    for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
      m_StripHits[h]->StreamDat(S, Version);  
    }

    for (unsigned int h = 0; h < m_Hits.size(); ++h) {
      m_Hits[h]->StreamDat(S, Version);  
    }
  } else if (Version == 2) {
    for (auto H : m_Hits) {
      H->StreamDat(S, 2);
    }
  } else if (Version == 3) {
     for (auto H : m_Hits) {
       H->StreamDat(S, 3);
    }
  }

  StreamBDFlags(S);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamEvta(ostream& S)
{
  //! Stream the content in MEGAlib's evta format 

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;
  S<<"CL "<<m_Time<<endl;
  S<<"TI "<<m_EventTimeUTC<<endl;

  if (m_Aspect != 0) {
    m_Aspect->StreamEvta(S);
  }

	if (m_HasSimAspectInfo){
		S<<"GX "<<m_GalacticPointingXAxisPhi<<" "<<m_GalacticPointingXAxisTheta<<endl;
		S<<"GZ "<<m_GalacticPointingZAxisPhi<<" "<<m_GalacticPointingZAxisTheta<<endl;
	}

  for (MSimIA& IA: m_SimIAs) {
    S<<IA.ToSimString()<<endl; 
  }
  
  for (unsigned int h = 0; h < m_Hits.size(); ++h) {
    m_Hits[h]->StreamEvta(S);  
  }
  
  S<<"CC NStripHits "<<m_StripHits.size()<<endl;
  
  StreamBDFlags(S);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamRoa(ostream& S, bool WithADCs, bool WithTACs, bool WithEnergies, bool WithTimings, bool WithTemperatures, bool WithFlags, bool WithOrigins, bool WithNearestNeighbors)
{
  //! Stream the content in MEGAlib's evta format 

  S<<"SE"<<endl;
  S<<"ID "<<m_ID<<endl;
  S<<"CL "<<m_Time<<endl;
  S<<"TI "<<m_EventTimeUTC<<endl;

  if (m_Aspect != nullptr) {
    m_Aspect->StreamEvta(S);
  }

  for (MSimIA& IA: m_SimIAs) {
    S<<IA.ToSimString()<<endl; 
  }

  unsigned int Counter = 0;
  for (unsigned int h = 0; h < m_StripHits.size(); ++h) {
    if (WithNearestNeighbors == false && m_StripHits[h]->IsNearestNeighbor() == true) {
      continue;
    }
    m_StripHits[h]->StreamRoa(S, WithADCs, WithTACs, WithEnergies, WithTimings, WithTemperatures, WithFlags);
    ++Counter;
  }
  for (unsigned int h = 0; h < m_CrystalHits.size(); ++h) {
    m_CrystalHits[h]->StreamRoa(S, WithADCs, WithEnergies, WithTemperatures, WithFlags);
    ++Counter;
  }
  if (Counter == 0) {
    S<<"BD No hits"<<endl;;
  }
  
  StreamBDFlags(S);
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamTra(ostream& S)
{
  //! Stream the content in MEGAlib's evta format

  S<<"SE"<<endl;

  if (m_PhysicalEvent != nullptr) {
    S<<m_PhysicalEvent->ToTraString();
  } else {
    S<<"ID "<<m_ID<<endl;
    StreamBDFlags(S);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MReadOutAssembly::StreamBDFlags(ostream& S)
{
  // Stream the BD and QA flags

  if (m_EnergyCalibrationError == true) {
    S<<"BD EnergyCalibrationError";
    if (m_EnergyCalibrationErrorString != "") S<<" ("<<m_EnergyCalibrationErrorString<<")";
    S<<endl;
  }
   if (m_StripPairingError == true) {
    S<<"BD StripPairingError";
    if (m_StripPairingErrorString != "") S<<" ("<<m_StripPairingErrorString<<")";
    S<<endl;
  }
  if (m_DepthCalibrationError == true) {
    S<<"BD DepthCalibrationError";
    if (m_DepthCalibrationErrorString != "") S<<" ("<<m_DepthCalibrationErrorString<<")";
    S<<endl;
  }
  if (m_EventReconstructionError == true) {
    S<<"BD EventReconstructionError";
    if (m_EventReconstructionErrorString != "") S<<" ("<<m_EventReconstructionErrorString<<")";
    S<<endl;
  }

  if (m_StripHitBelowThreshold_QualityFlag == true) {
    S<<"QA StripHitBelowThreshold";
    if (m_StripHitBelowThresholdString_QualityFlag != "") S<<" ("<<m_StripHitBelowThresholdString_QualityFlag<<")";
    S<<endl;
  }

  if (m_GuardRingVeto == true) {
    S<<"BD GR Veto"<<endl;
  }
  if (m_ShieldVeto == true) {
    S<<"BD Shield Veto"<<endl;
  }
  for (auto H : m_Hits) {
    if (H->GetStripHitMultipleTimesX()) {
      S<<"BD Multiple Hits on LV Strip"<<endl;
      break;
    }
  }
  for (auto H : m_Hits) {
    if (H->GetStripHitMultipleTimesY()) {
      S<<"BD Multiple Hits on HV Strip"<<endl;
      break;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::IsGood() const
{
  //! Returns true if none of the "bad" or "Error" falgs has been set

  if (m_EnergyCalibrationError == true) return false;
  if (m_StripPairingError == true) return false;
  if (m_DepthCalibrationError == true) return false;
  if (m_EventReconstructionError == true) return false;

  if (m_FilteredOut == true) return false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::IsBad() const
{
  //! Returns true if none of the "bad" or "Error" flag has been set

  if (m_EnergyCalibrationError == true) return true;
  if (m_StripPairingError == true) return true;
  if (m_DepthCalibrationError == true) return true;
  if (m_EventReconstructionError == true) return true;

  if (m_FilteredOut == true) return true;

  return false;
}
 

//////////////////////////////////////////////////////////////////////////////

bool MReadOutAssembly::IsVeto() const
{
  //! Returns true if none of the "bad" or "Error" falgs has been set

  if (m_ShieldVeto == true) return true;
  if (m_GuardRingVeto == true) return true;

  return false;
}


//////////////////////////////////////////////////////////////////////////////


bool MReadOutAssembly::ComputeAbsoluteTime()
{

	//the following code assumes that the clock board oscillator is exactly 10 MHz.
	//in reality there is a +/- 25 ppm tolerance on the frequency, so worst case this
	//would give an absolute timing error of 25 us.  This can be corrected for by 
	//comparing the difference between PPS values from sample to sample. The GPS
	//PPS timing error is much smaller (it is specd at 200 ns )

	/*

	int64_t dt = m_CL - PPS;
	MTime dT;
	dT.Set((int)(dt/10000000),(int)((dt % 10000000)*100));
	MTime UTC(UTCSecond,(long int)0);
	UTC += dT; //dT can be positive or negative, += operator calls Normalize()
	m_EventTimeUTC.Set(UTC);
	return true;

	*/

	if(m_Aspect != 0){
		int64_t dt = m_CL - m_Aspect->GetPPS();
		MTime dT;
		dT.Set((int)(dt/10000000),(int)((dt % 10000000)*100));
		MTime UTCTimeTrunc = m_Aspect->GetUTCTime();
		UTCTimeTrunc.Set(UTCTimeTrunc.GetAsSystemSeconds(), (long int)0);
		UTCTimeTrunc += dT; //dT can be positive or negative, += operator calls Normalize()
		m_EventTimeUTC.Set(UTCTimeTrunc);
		//cout << "m_Time = " << m_Time << ", m_EventTimeUTC = " << m_EventTimeUTC << ", dT = " << dT << endl;
		return true;
	} else {
		return false;
	}

}


// MReadOutAssembly.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
