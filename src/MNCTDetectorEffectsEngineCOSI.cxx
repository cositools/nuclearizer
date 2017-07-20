/*
 * MDummy.cxx
 *
 *
 * Copyright (C) by Clio Sleator, Carolyn Kierans, Andreas Zoglauer.
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
// MDummy
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDummy.h"

// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <map>
#include <fstream>
#include <cstring>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <algorithm>
using namespace std;

// ROOT
#include <TApplication.h>
#include <TStyle.h>
#include <TH1.h>
#include <TCanvas.h>
#include <MString.h>
#include <TRandom3.h>

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MReadOutElementDoubleStrip.h"

// Nuclearizer
#include "MNCTDetectorEffectsEngineCOSI.h"
#include "MNCTDepthCalibrator.h"


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MNCTDetectorEffectsEngineCOSI::MNCTDetectorEffectsEngineCOSI()
{
  m_Geometry = nullptr;
  m_OwnGeometry = false;
  m_ShowProgressBar = false;
  m_SaveToFile = false;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MNCTDetectorEffectsEngineCOSI::~MNCTDetectorEffectsEngineCOSI()
{
  // Intentionally left blank
  
  if (m_OwnGeometry == true) delete m_Geometry;
}
    
    
////////////////////////////////////////////////////////////////////////////////



//! Initialize the module
bool MNCTDetectorEffectsEngineCOSI::Initialize()
{
  // Load geometry:
  if (m_Geometry == nullptr) {
    if (m_GeometryFileName == "") {
      cout<<"Error: Need a geometry file name!"<<endl;
      return false;
    }  
  
    m_Geometry = new MDGeometryQuest();
    if (m_Geometry->ScanSetupFile(m_GeometryFileName) == true) {
      m_Geometry->ActivateNoising(false);
      m_Geometry->SetGlobalFailureRate(0.0);
      cout<<"m_Geometry "<<m_Geometry->GetName()<<" loaded!"<<endl;
    } else {
      cout<<"Unable to load geometry "<<m_Geometry->GetName()<<" - Aborting!"<<endl;
      return false;
    }
    m_OwnGeometry = true;
  }
  
  //load energy calibration information
  ParseEnergyCalibrationFile();
  //load dead strip information
  ParseDeadStripFile();
  //load threshold information
  ParseThresholdFile();

  //load charge loss coefficients
  //InitializeChargeLoss();

	//initialize dead time
	for (int i=0; i<12; i++){
		m_CCDeadTime[i] = 1e-5; m_TotalDeadTime[i] = 0.; m_TriggerRates[i]=0;
		for (int j=0; j<16; j++){
			m_DeadTimeBuffer[i][j] = -1;
		}
	}

	//initialize last time to 0 (will this exclude first event?)
	m_LastHitTime = 0;
	for (int i=0; i<12; i++){
		m_LastHitTimeByDet[i] = 0;
	}

	m_MaxBufferFullIndex = 0;

  m_DepthCalibrator = new MNCTDepthCalibrator();
  if( m_DepthCalibrator->LoadCoeffsFile(m_DepthCalibrationCoeffsFileName) == false ){
    cout << "Unable to load depth calibration coefficients file - Aborting!" << endl;
    return false;
  }

  if( m_DepthCalibrator->LoadSplinesFile(m_DepthCalibrationSplinesFileName) == false ){
    cout << "Unable to load depth calibration splines file - Aborting!" << endl;
    return false;
  }


  m_Reader = new MFileEventsSim(m_Geometry);
  if (m_Reader->Open(m_SimulationFileName) == false) {
    cout<<"Unable to open sim file "<<m_SimulationFileName<<" - Aborting!"<<endl; 
    return false;
  }
  if (m_ShowProgressBar == true) {
    m_Reader->ShowProgress();
  }

  
  if (m_SaveToFile == true) {
    cout << "Output File: " << m_RoaFileName << endl;

    m_Roa.open(m_RoaFileName);
    m_Roa<<endl;
    m_Roa<<"TYPE   ROA"<<endl;
    m_Roa<<"UF     doublesidedstrip adc-timing-origins"<<endl;
    m_Roa<<endl;
  }

  //  TCanvas specCanvas("c","c",600,400); 
  //  Threshold1D* spectrum = new Threshold1D("spec","spectrum",40,640,680);

  //count how many events have multiple hits per strip
  m_MultipleHitsCounter = 0;
  m_TotalHitsCounter = 0;
  m_ChargeLossCounter = 0;

  // for shield veto: shield pulse duration and card cage delay: constant for now
  m_ShieldPulseDuration = 1.7e-6;
  m_CCDelay = 700.e-9;
  // for shield veto: gets updated with shield event times
  // start at -10s so that it doesn't veto beginning events by accident
  m_ShieldTime = -10;  
 
	m_IsShieldDead = false;

	m_NumShieldCounts = 0;
 
  return true;
}
    
    
////////////////////////////////////////////////////////////////////////////////


//! Analyze whatever needs to be analyzed...
bool MNCTDetectorEffectsEngineCOSI::GetNextEvent(MReadOutAssembly* Event)
{
  MSimEvent* SimEvent = 0;
  //int RunningID = 0;
  
  while ((SimEvent = m_Reader->GetNextEvent(false)) != nullptr) {
    //cout<<"ID: "<<SimEvent->GetID()<<endl;
    if (SimEvent->GetNHTs() == 0) {
      //cout<<SimEvent->GetID()<<": No hits"<<endl;
      delete SimEvent;
      continue;
    }


/*		for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){
      MSimHT* HT = SimEvent->GetHTAt(h);
      if (HT->GetDetectorType() == 4) { m_NumShieldCounts++; }
		}
*/
    
    // Step (0): Check whether events should be vetoed
    double evt_time = SimEvent->GetTime().GetAsSeconds();
		bool hasDetHits = false;
		bool hasShieldHits = false;
		bool increaseShieldDeadTime = false;

    // first check if there's another shield hit above the threshold
    // if so, veto event
    for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){
      MSimHT* HT = SimEvent->GetHTAt(h);
      if (HT->GetDetectorType() == 4) {
        MDVolumeSequence* VS = HT->GetVolumeSequence();
        MDDetector* Detector = VS->GetDetector();
        MString DetName = Detector->GetName();

        double energy = HT->GetEnergy();
        energy = NoiseShieldEnergy(energy,DetName);
        HT->SetEnergy(energy);

        if (energy > 80.) {
					if (m_ShieldTime + m_ShieldPulseDuration < evt_time){ hasShieldHits = true; }
					increaseShieldDeadTime = true;
         	m_ShieldTime = evt_time;
        }
	    }
			else if (HT->GetDetectorType() == 3){ hasDetHits = true; }
    }

		if (hasShieldHits == true){ m_NumShieldCounts++; }
		if (increaseShieldDeadTime == true){ m_ShieldDeadTime += m_ShieldPulseDuration; }

		if (evt_time + m_CCDelay < m_ShieldTime + m_ShieldPulseDuration){
//		if (evt_time == m_ShieldTime){// && hasDetHits == true){
//		if (abs(evt_time-m_ShieldTime) < 400e-9){
//			if (hasDetHits == true){ m_NumShieldCounts++; }
 		  delete SimEvent;
      //cout<<SimEvent->GetID()<<": Vetoed"<<endl;
      continue;
    }


    //get interactions to look for ionization in hits
    vector<MSimIA*> IAs;
    for (unsigned int i=0; i<SimEvent->GetNIAs(); i++){
      MSimIA* ia = SimEvent->GetIAAt(i);
      IAs.push_back(ia);
    }

		// Step (0.5): Get aspect information
		if (SimEvent->HasGalacticPointing()){
			Event->SetSimAspectInfo(true);

			double phi = SimEvent->GetGalacticPointingXAxis().Phi()*c_Deg;
			if (phi < 0.0){ phi += 360; }
			Event->SetGalacticPointingXAxisPhi(phi);
			Event->SetGalacticPointingXAxisTheta(SimEvent->GetGalacticPointingXAxis().Theta()*c_Deg-90);

			phi = SimEvent->GetGalacticPointingZAxis().Phi()*c_Deg;
			if (phi < 0.0){ phi += 360; }
			Event->SetGalacticPointingZAxisPhi(phi);
			Event->SetGalacticPointingZAxisTheta(SimEvent->GetGalacticPointingZAxis().Theta()*c_Deg-90);
		}

    // Step (1): Convert positions into strip hits
    list<MNCTDEEStripHit> StripHits;

    // (1a) The real strips
    for (unsigned int h = 0; h < SimEvent->GetNHTs(); ++h) {
      MSimHT* HT = SimEvent->GetHTAt(h);

      MDVolumeSequence* VS = HT->GetVolumeSequence();
      MDDetector* Detector = VS->GetDetector();
      MString DetectorName = Detector->GetName();
      if(!DetectorName.BeginsWith("Detector")){
       continue; //probably a shield hit.  this can happen if the veto flag is off for the shields
      }
      DetectorName.RemoveAllInPlace("Detector");
      int DetectorID = DetectorName.ToInt();


      MNCTDEEStripHit pSide;
      MNCTDEEStripHit nSide;

      //should be unique identifiers
      pSide.m_ID = h*10;
      nSide.m_ID = h*10+5;

      pSide.m_OppositeStrip = nSide.m_ID;
      nSide.m_OppositeStrip = pSide.m_ID;

      pSide.m_ROE.IsPositiveStrip(true);
      nSide.m_ROE.IsPositiveStrip(false);

      // Convert detector name in detector ID
      pSide.m_ROE.SetDetectorID(DetectorID);
      nSide.m_ROE.SetDetectorID(DetectorID);

      // Convert position into
      MVector PositionInDetector = VS->GetPositionInSensitiveVolume();
      MDGridPoint GP = Detector->GetGridPoint(PositionInDetector);
      double Depth_ = PositionInDetector.GetZ();
      double Depth = -(Depth_ - (m_DepthCalibrator->GetThickness(DetectorID)/2.0)); // change the depth coordinates so that one side is 0.0 cm and the other side is ~1.5cm

      // Not sure about if p or n-side is up, but we can debug this later
      pSide.m_ROE.SetStripID(38-(GP.GetYGrid()+1));
      nSide.m_ROE.SetStripID(38-(GP.GetXGrid()+1));

//      cout << pSide.m_ROE.GetStripID() << '\t' << nSide.m_ROE.GetStripID() << endl;

      //SetStripID needs to be called before we can look up the depth calibration coefficients
      int PixelCode = DetectorID*10000 + pSide.m_ROE.GetStripID()*100 + nSide.m_ROE.GetStripID();
      std::vector<double>* Coeffs = m_DepthCalibrator->GetPixelCoeffs(PixelCode);
      if( Coeffs == NULL ){
        //pixel is not calibrated! discard this event....
        //cout << "pixel " << PixelCode << " has no depth calibration... discarding event" << endl;
        //delete SimEvent;
        continue;
      }

      pSide.m_Timing = (Coeffs->at(0) * m_DepthCalibrator->GetCathodeSpline(DetectorID)->Eval(Depth)) + (Coeffs->at(1)/2.0);
      nSide.m_Timing = (Coeffs->at(0) * m_DepthCalibrator->GetAnodeSpline(DetectorID)->Eval(Depth)) - (Coeffs->at(1)/2.0);

      pSide.m_Energy = HT->GetEnergy();
      nSide.m_Energy = HT->GetEnergy();

      pSide.m_Position = PositionInDetector;
      nSide.m_Position = PositionInDetector;

      //check for ionization if it's in the sim file
      vector<int> origin = HT->GetOrigins();
      pSide.m_Origins = list<int>(origin.begin(), origin.end());
      nSide.m_Origins = list<int>(origin.begin(), origin.end());

      StripHits.push_back(pSide);
      StripHits.push_back(nSide);
//      int currentSize = StripHits.size();
//      StripHits.at(currentSize-1).m_OppositeStrip = &StripHits.at(currentSize-2);
//      StripHits.at(currentSize-2).m_OppositeStrip = &StripHits.at(currentSize-1);


//      cout << pSide.m_Energy << '\t';
//      cout << pSide.m_OppositeStrip->m_Energy << '\t';
//      cout << nSide.m_Energy << endl;
//      cout << nSide.m_OppositeStrip->m_Energy << endl;

      //set the m_Opposite pointers to each other

      m_TotalHitsCounter++;

    }


    list<MNCTDEEStripHit> GuardRingHits;
    // (1b) The guard ring hits
    for (unsigned int h = 0; h < SimEvent->GetNGRs(); ++h) {
      MSimGR* GR = SimEvent->GetGRAt(h);
      MDVolumeSequence* VS = GR->GetVolumeSequence();
      MDDetector* Detector = VS->GetDetector();
      MString DetectorName = Detector->GetName();
      DetectorName.RemoveAllInPlace("Detector");
      int DetectorID = DetectorName.ToInt();

      MNCTDEEStripHit GuardRingHit;
      GuardRingHit.m_ROE.IsPositiveStrip(true); // <-- not important
      GuardRingHit.m_ROE.SetDetectorID(DetectorID);
      GuardRingHit.m_ROE.SetStripID(38); // ?

      GuardRingHit.m_Energy = GR->GetEnergy();
      GuardRingHit.m_Position = MVector(0, 0, 0); // <-- not important

			GuardRingHits.push_back(GuardRingHit);
    }



    // (1c): Merge strip hits
    list<MNCTDEEStripHit> MergedStripHits;
    while (StripHits.size() > 0) {
      MNCTDEEStripHit Start;
      Start.m_SubStripHits.push_back(StripHits.front());
      StripHits.pop_front();
      Start.m_ROE = Start.m_SubStripHits.front().m_ROE;

//      cout << "------" << endl;
//      cout << Start.m_SubStripHits[0].m_Energy << '\t';
//      cout << Start.m_SubStripHits[0].m_OppositeStrip->m_Energy << endl;

      list<MNCTDEEStripHit>::iterator i = StripHits.begin();
      while (i != StripHits.end()) {
        if ((*i).m_ROE == Start.m_ROE) {
          Start.m_SubStripHits.push_back(*i);
//          cout << (*i).m_Energy << '\t';
//          cout << (*i).m_OppositeStrip->m_Energy << endl;
          i = StripHits.erase(i);
        } else {
          ++i;
        }
      }
//      cout << "-----------" << endl;
      MergedStripHits.push_back(Start);
    }


//    bool fromSameInteraction = true;
    for (MNCTDEEStripHit& Hit: MergedStripHits){
      int nIndep = 0;
      int nSubHits = Hit.m_SubStripHits.size();
      if (nSubHits > 1){
        for (int i=0; i<nSubHits; i++){
          bool sharedOrigin = false;
          for (int j=0; j<nSubHits; j++){
            if (i != j){
              MNCTDEEStripHit& SubHit1 = Hit.m_SubStripHits.at(i);
              MNCTDEEStripHit& SubHit2 = Hit.m_SubStripHits.at(j);

              for (int o1: SubHit1.m_Origins){
                for (int o2: SubHit2.m_Origins){
                  if (o1 == o2){ sharedOrigin = true; }
                }
              }
            }
          }
          if (!sharedOrigin){ nIndep++; }
        }
        if (nIndep == 1){ nIndep++; }
        //cout << SimEvent->GetID() << '\t' << Hit.m_ROE.GetDetectorID() << '\t'<<  nIndep << '\t' << nSubHits << '\t';
        //cout << Hit.m_SubStripHits.at(0).m_Energy << endl;
        m_MultipleHitsCounter += nIndep;
      }
    }


    // Merge origins
    for (MNCTDEEStripHit& Hit: MergedStripHits) {
      Hit.m_Origins.clear();
      for (MNCTDEEStripHit& SubHit: Hit.m_SubStripHits) {
        for (int& Origin: SubHit.m_Origins) {
          Hit.m_Origins.push_back(Origin);
        }
      }
      Hit.m_Origins.sort();
      Hit.m_Origins.unique();      
    }
    
    // Step (2): Calculate and noise timing
    const double TimingNoise = 3.76; //ns//I have been assuming 12.5 ns FWHM on the CTD... so the 1 sigma error on the timing value should be (12.5/2.35)/sqrt(2)
    for (MNCTDEEStripHit& Hit: MergedStripHits) {
      
      //find lowest timing value 
      double LowestNoisedTiming = Hit.m_SubStripHits.front().m_Timing + gRandom->Gaus(0,TimingNoise);
      for(size_t i = 1; i < Hit.m_SubStripHits.size(); ++i){
        double Timing = Hit.m_SubStripHits.at(i).m_Timing + gRandom->Gaus(0,TimingNoise);
        //SubHit.m_Timing += gRandom->Gaus(0,TimingNoise);
        if( Timing < LowestNoisedTiming ) LowestNoisedTiming = Timing;
      }
      LowestNoisedTiming -= fmod(LowestNoisedTiming,5.0); //round down to nearest multiple of 5
      Hit.m_Timing = LowestNoisedTiming;

    }


 
    // (3c) Noise energy deposit
    for (MNCTDEEStripHit& Hit: MergedStripHits) {
      double Energy = 0;
      for (MNCTDEEStripHit SubHit: Hit.m_SubStripHits) {
        Energy += SubHit.m_Energy;
      }

      Hit.m_Energy = Energy;
      Hit.m_ADC = EnergyToADC(Hit,Energy);
    }

    // (3d) Handle ADC overflow


    // Step (4): Apply thresholds and triggers including guard ring hits
    //           * use the trigger threshold calibration and invert it here 
    //           * take care of guard ring hits with their special thresholds
    //           * take care of hits in dead strips
    //           * throw out hits which did not trigger

    // (4a) Take care of dead strips:
    list<MNCTDEEStripHit>::iterator j = MergedStripHits.begin();
    while (j != MergedStripHits.end()) {
      int det = (*j).m_ROE.GetDetectorID();
      int stripID = (*j).m_ROE.GetStripID();
      bool side_b = (*j).m_ROE.IsPositiveStrip();
      int side = 0;
      if (side_b) {side = 1;}

      //if strip has been flagged as dead, erase strip hit
      if (m_DeadStrips[det][side][stripID-1] == 1){
        j = MergedStripHits.erase(j);
      }
      else {
        ++j;
      }
    }

    // (4b) Handle trigger thresholds make sure we throw out timing too!
    list<MNCTDEEStripHit>::iterator k = MergedStripHits.begin();
    while (k != MergedStripHits.end()) {

      if ((*k).m_Energy < m_LLDThresholds[(*k).m_ROE]) { // Dummy threshold of 0 keV sharp
        k = MergedStripHits.erase(k);
      } else {
//        if(gRandom->Gaus((*k).m_Energy,m_FSTNoise[(*k).m_ROE]) < m_FSTThresholds[(*k).m_ROE]){
				if ((*k).m_Energy < gRandom->Gaus(m_FSTThresholds[(*k).m_ROE],m_FSTNoise[(*k).m_ROE])){
//				if ((*k).m_Energy < m_FSTThresholds[(*k).m_ROE]){
          (*k).m_Timing = 0.0;
        }
        ++k;
      }
    }

/*    // (4c) Take care of guard ring vetoes
		list<MNCTDEEStripHit>::iterator gr = GuardRingHits.begin();
		int grHit[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
		while (gr != GuardRingHits.end()) {
			if ((*gr).m_Energy > 25){
				int detID = (*gr).m_ROE.GetDetectorID();
				grHit[detID] = 1;
			}
			++gr;
		}
		list<MNCTDEEStripHit>::iterator grVeto = MergedStripHits.begin();
		while (grVeto != MergedStripHits.end()){
			int detID = (*grVeto).m_ROE.GetDetectorID();
			if (grHit[detID] == 1){ 
				grVeto = MergedStripHits.erase(grVeto);
			}
			else{ ++grVeto; }
		}
*/
		// (4d) Make sure there is at least one strip left on each side of each detector
		//  If not, remove remaining strip(s) from detector because they won't trigger detector
		int xExists[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
		int yExists[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

		//look for (at least) one strip on each side
		list<MNCTDEEStripHit>::iterator tr = MergedStripHits.begin();
		while (tr != MergedStripHits.end()) {
			int DetID = (*tr).m_ROE.GetDetectorID();
			if ((*tr).m_Timing != 0 && (*tr).m_Energy != 0){
				if ((*tr).m_ROE.IsPositiveStrip()){ xExists[DetID] = 1; }
				else{ yExists[DetID] = 1; }
			}
			++tr;
		}

		//remove hits that won't trigger detector
		tr = MergedStripHits.begin();
		while (tr != MergedStripHits.end()) {
			int DetID = (*tr).m_ROE.GetDetectorID();
			if ( xExists[DetID] == 0 || yExists[DetID] == 0){
				tr = MergedStripHits.erase(tr);
			}
			else{ ++tr;}
		}


    // Step (5): Split into card cage events - i.e. split by detector
/*    vector<vector<MNCTDEEStripHit>> CardCagedStripHits;
    for (MNCTDEEStripHit Hit: MergedStripHits) {
      bool Found = false;
      for (vector<MNCTDEEStripHit>& V: CardCagedStripHits) {
        if (V[0].m_ROE.GetDetectorID() == Hit.m_ROE.GetDetectorID()) {
          V.push_back(Hit);
          Found = true;
        }
      }
      if (Found == false) {
        vector<MNCTDEEStripHit> New;
        New.push_back(Hit);
        CardCagedStripHits.push_back(New);
      }
    }


    // Step (6): Determine and noise the global event time
    vector<double> CardCageTiming(CardCagedStripHits.size());
    for (double& T: CardCageTiming) {
      T = SimEvent->GetTime().GetAsSeconds();
    }
*/

		//Step (6.5): Dead time
		//figure out which detectors are currently dead -- 10us dead time per event
		int updateLastHitTime[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
		int detIsDead[12];

		for (int d=0; d<12; d++){
			//second conditional for running multiple sim files when t starts at 0
			if (m_LastHitTimeByDet[d] + m_CCDeadTime[d] > evt_time && m_LastHitTimeByDet[d]<evt_time){ detIsDead[d] = 1; }
			else { detIsDead[d] = 0; }
		}

		//erase strip hits in dead detectors
    list<MNCTDEEStripHit>::iterator DT = MergedStripHits.begin();
    while (DT != MergedStripHits.end()) {
      int DetID = (*DT).m_ROE.GetDetectorID();
//			detHit[DetID] = 1;
      if (detIsDead[DetID] == 1){
        DT = MergedStripHits.erase(DT);
      }
      else {
				updateLastHitTime[DetID] = 1;
        ++DT;
      }
    }

		//update last hit time for live detectors that were hit
		for (int d=0; d<12; d++){
			if (updateLastHitTime[d] == 1){
				m_LastHitTimeByDet[d] = evt_time;
				m_TotalDeadTime[d] += m_CCDeadTime[d];
			}
		}

		// Step (6.75):
		//figure out if dead time buffers are full, and update them accordingly
		double empty_buffer_val = -1;
		double time_buffer_empty = .000625;
		int nBuffSlots = 16;

		//increase buffer times if necessary
		for (int d=0; d<12; d++){
			int indexOfLargest = -1;
			double maxTime = -1;
			for (int s=0; s<nBuffSlots; s++){
				//if buffer slot not empty
				if (m_DeadTimeBuffer[d][s] != -1){
					//if buffer slot has exceeded time to empty, set it to empty
					if (m_DeadTimeBuffer[d][s] >= time_buffer_empty){
						m_DeadTimeBuffer[d][s] = empty_buffer_val;
					}
					//otherwise, find index of largest buffer slot and increase ONLY that slot
					else {
						if (m_DeadTimeBuffer[d][s] > maxTime){
							maxTime = m_DeadTimeBuffer[d][s];
							indexOfLargest = s;
						}
					}
				}
			}
			if (indexOfLargest != -1){ m_DeadTimeBuffer[d][indexOfLargest] += evt_time-m_LastHitTime; }
		}


		//figure out which detectors were hit
		int bufferFull[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
 
		//check if buffer is full for each detector
		for (int d=0; d<12; d++){
			int nextEmptySlot = 16;
			for (int s=0; s<nBuffSlots; s++){
				if (m_DeadTimeBuffer[d][s] == empty_buffer_val){
					nextEmptySlot = s;
					break;
				}
			}
			bufferFull[d] = nextEmptySlot;
		}

		for (int i=0; i<12; i++){
			if (bufferFull[i] > m_MaxBufferFullIndex){ m_MaxBufferFullIndex = bufferFull[i]; m_MaxBufferDetector = i; }
		}


/*		if (bufferFull[0] == 16){
			cout << "************" << endl;
			cout << "evt_time: " << evt_time << '\t' << "last time: " << m_LastHitTime << endl;
			cout << "Buffer values: " << endl;
			for (int i=0; i<16; i++){
				cout << m_DeadTimeBuffer[0][i] << '\t';
			}
			cout << endl;

			cout << "next empty slot: " << bufferFull[0] << endl;
	}
*/
		//erase strip hits in detectors when buffer is full
	  list<MNCTDEEStripHit>::iterator DH = MergedStripHits.begin();
    while (DH != MergedStripHits.end()) {
      int DetID = (*DH).m_ROE.GetDetectorID();
			if (bufferFull[DetID] == 16){
				DH = MergedStripHits.erase(DH);
			}
			else {
				m_DeadTimeBuffer[DetID][bufferFull[DetID]] = 0;
				++DH;
			}
    }

		//update LastHitTime
		m_LastHitTime = evt_time;
 
 	  // Step (7): 
		//check if there are any hits left in the event
		int HitCounter = 0;
		for (MNCTDEEStripHit Hit: MergedStripHits){
			HitCounter++;
		}
		if (HitCounter == 0){
			delete SimEvent;
			continue;
		}
   
		//update trigger rates
		set<int> detectorsHit;
	  list<MNCTDEEStripHit>::iterator TR = MergedStripHits.begin();
    while (TR != MergedStripHits.end()) {
      int DetID = (*TR).m_ROE.GetDetectorID();
			detectorsHit.insert(DetID);
			++TR;
    }

		for (set<int>::iterator s=detectorsHit.begin(); s!=detectorsHit.end(); ++s){
			int detID = *s;
			m_TriggerRates[detID] += 1;
		}
 
    // (1) Move the information to the read-out-assembly
    Event->SetID(SimEvent->GetID());
    Event->SetTimeUTC(SimEvent->GetTime());
    
    for (unsigned int i = 0; i < IAs.size(); ++i) {
      Event->AddSimIA(*IAs[i]);
    }
    for (MNCTDEEStripHit Hit: MergedStripHits){
      MNCTStripHit* SH = new MNCTStripHit();
      SH->SetDetectorID(Hit.m_ROE.GetDetectorID());
      SH->SetStripID(Hit.m_ROE.GetStripID());
      SH->IsXStrip(Hit.m_ROE.IsPositiveStrip());
      SH->SetADCUnits(Hit.m_ADC);
      SH->SetTiming(Hit.m_Timing);
      vector<int> O;
      for (int i: Hit.m_Origins) O.push_back(i);
      SH->AddOrigins(O); 
      Event->AddStripHit(SH); 
    }
        
    // (2) Dump event to file in ROA format
    if (m_SaveToFile == true) {
      m_Roa<<"SE"<<endl;
      m_Roa<<"ID "<<SimEvent->GetID()<<endl;
      //m_Roa<<"ID "<<++RunningID<<endl;
      m_Roa<<"TI "<<SimEvent->GetTime() << endl;
      for (unsigned int i = 0; i < IAs.size(); ++i) {
        m_Roa<<IAs[i]->ToSimString()<<endl;
      }
      for (MNCTDEEStripHit Hit: MergedStripHits){
        m_Roa<<"UH "<<Hit.m_ROE.GetDetectorID()<<" "<<Hit.m_ROE.GetStripID()<<" "<<(Hit.m_ROE.IsPositiveStrip() ? "p" : "n")<<" "<<Hit.m_ADC<<" "<<Hit.m_Timing;
      
        MString Origins;
        for (int Origin: Hit.m_Origins) {
          if (Origins != "") Origins += ";";
          Origins += Origin;
        }
        if (Origins == "") Origins += "-";
        m_Roa<<" "<<Origins<<endl;
      }
    }


    // Never forget to delete the event
    delete SimEvent;
    
    return true;
  }

  //  spectrum->Draw();
  //  specCanvas.Print("spectrum_adc.pdf");

  return false;
}
    
////////////////////////////////////////////////////////////////////////////////


//! Finalize the module
bool MNCTDetectorEffectsEngineCOSI::Finalize()
{
  cout << "total hits: " << m_TotalHitsCounter << endl;
  cout << "number of events with multiple hits per strip: " << m_MultipleHitsCounter << endl;
  cout << "charge loss applies counter: " << m_ChargeLossCounter << endl;
	cout << "Num shield counts: " << m_NumShieldCounts << endl;
	cout << "Dead time " << endl;
	for (int i=0; i<12; i++){
		cout << i << ":\t" << m_TotalDeadTime[i] << endl;
	}
	cout << "Trigger rates" << endl;
	for (int i=0; i<12; i++){
		cout << i << ":\t" << m_TriggerRates[i] << endl;
	}
	cout << "Shield dead time: " << m_ShieldDeadTime << endl;

	cout << "Max buffer full index: " << m_MaxBufferFullIndex << '\t' << "Detector " << m_MaxBufferDetector << endl;

  if (m_SaveToFile == true) {
    m_Roa<<"EN"<<endl<<endl;
    m_Roa.close();
  }
  
  delete m_Reader;

  return true;
}
    


////////////////////////////////////////////////////////////////////////////////


//! Convert energy to ADC value by reversing energy calibration done in 
//! MNCTModuleEnergyCalibrationUniversal.cxx
int MNCTDetectorEffectsEngineCOSI::EnergyToADC(MNCTDEEStripHit& Hit, double mean_energy)
{  
  //first, need to simulate energy spread
  static TRandom3 r(0);
  TF1* FitRes = m_ResolutionCalibration[Hit.m_ROE];
  //resolution is a function of energy
  double EnergyResolutionFWHM = 3; //default to 3keV...does this make sense?
  if (FitRes != 0){
    EnergyResolutionFWHM = FitRes->Eval(mean_energy);
    //cout<<"Energy Res: "<<EnergyResolutionFWHM<<" (FWHM) at "<<mean_energy<<endl;
  }

  //get energy from gaussian around mean_energy with sigma=EnergyResolution
  //TRandom3 r(0);
  double energy = r.Gaus(mean_energy,EnergyResolutionFWHM/2.35);
//	double energy = mean_energy; 
 //  spectrum->Fill(energy);

  //  if (fabs(mean_energy-662.) < 5){
  //    cout << mean_energy << '\t' << EnergyResolution << '\t' << energy << endl;
  //  }

  //then, convert energy to ADC
  double ADC_double = 0;

  //get the fit function
  TF1* Fit = m_EnergyCalibration[Hit.m_ROE];

  if (Fit != 0) {
    //find roots
    ADC_double = Fit->GetX(energy,0.,8000.);
  }


  int ADC = int(ADC_double);
  return ADC;

}


////////////////////////////////////////////////////////////////////////////////


//! Noise shield energy with measured resolution
double MNCTDetectorEffectsEngineCOSI::NoiseShieldEnergy(double energy, MString shield_name)
{ 

  double resolution_consts[6] = {3.75,3.74,18.47,4.23,3.07,3.98};

  shield_name.RemoveAllInPlace("Shield");
  int shield_num = shield_name.ToInt();
  shield_num = shield_num - 1;
  double res_constant = resolution_consts[shield_num];

//  TF1* ShieldRes = new TF1("ShieldRes","[0]*(x^(1/2))",0,1000); //this is from Knoll
//  ShieldRes->SetParameter(0,res_constant);

//  double sigma = ShieldRes->Eval(energy);
	double sigma = res_constant*pow(energy,1./2);

  double noised_energy = gRandom->Gaus(energy,sigma);

  return noised_energy;

}


////////////////////////////////////////////////////////////////////////////////


//! Calculate new summed energy of two strips affected by charge loss
void MNCTDetectorEffectsEngineCOSI::InitializeChargeLoss()
{ 

  vector<string> filenames;
  filenames.push_back("./ChargeLossCorrectionScaled_Ba133.log");
  filenames.push_back("./ChargeLossCorrectionScaled_Cs137.log");

  vector<vector<vector<double> > > coefficients;

  for (unsigned int f=0; f<filenames.size(); f++){
    vector<double> tempOneDet;
    vector<vector<double> > tempOneSource;
    string line;
    int c=0;
    double B;

    ifstream clFile;
    clFile.open(filenames.at(f));

    if (clFile.is_open()){
      while (!clFile.eof()){
        c++;
        getline(clFile,line);

        if (c <= 24){
          B = stod(line);
          if (c%2 != 0){ tempOneDet.push_back(B);}
          else {
            tempOneDet.push_back(B);
            tempOneSource.push_back(tempOneDet);
            tempOneDet.clear();
          }
        }
      }
    }

    clFile.close();
    coefficients.push_back(tempOneSource);
    tempOneSource.clear();
  }

  double energies[2] = {356,662};
  double points[2];
  double A0;
  double A1;

  for (int det=0; det<12; det++){
    for (int side=0; side<2; side++){
      if (coefficients.at(0).size() < 12 || coefficients.at(1).size() < 12) {
        cout<<"Error: The charge loss coefficients do not cover all 12 detectors..."<<endl;
        continue;
      }

      points[0] = coefficients.at(0).at(det).at(side);
      points[1] = coefficients.at(1).at(det).at(side);

      TGraph *g = new TGraph(2,energies,points);
      TF1 *f = new TF1("f","[0]+[1]*x",energies[0],energies[1]);
      g->Fit("f","RQ");

      A0 = f->GetParameter(0);
      A1 = f->GetParameter(1);

      m_ChargeLossCoefficients[det][side][0] = A0;
      m_ChargeLossCoefficients[det][side][1] = A1;

      delete g;
      delete f;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Read in thresholds
void MNCTDetectorEffectsEngineCOSI::ParseThresholdFile()
{
  MParser Parser;
  if (Parser.Open(m_ThresholdFileName, MFile::c_Read) == false) {
    cout << "Unable to open threshold file " << m_ThresholdFileName << endl;
    return;
  }

  for (unsigned int i=0; i<Parser.GetNLines(); i++) {
    unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
		if (NTokens != 4){ continue; } //this shouldn't happen but just in case

		//decode identifier
		int identifier = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(0);
		int det = identifier / 1000;
		int strip = (identifier % 1000) / 10;
		bool isPos = identifier % 10;

		MReadOutElementDoubleStrip R;
		R.SetDetectorID(det);
		R.SetStripID(strip);
		R.IsPositiveStrip(isPos);

		m_LLDThresholds[R] = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1);
		m_FSTThresholds[R] = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2);
		m_FSTNoise[R] = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(3);

  }
}


////////////////////////////////////////////////////////////////////////////////


//! Parse ecal file: should be done once at the beginning to save all the poly3 coefficients
void MNCTDetectorEffectsEngineCOSI::ParseEnergyCalibrationFile()
{
  MParser Parser;
  if (Parser.Open(m_EnergyCalibrationFileName, MFile::c_Read) == false){
    cout << "Unable to open calibration file " << m_EnergyCalibrationFileName << endl;
    return;
  }

  map<MReadOutElementDoubleStrip, unsigned int> CM_ROEToLine; //Energy Calibration Model
  map<MReadOutElementDoubleStrip, unsigned int> CR_ROEToLine; //Energy Resolution Calibration Model
  //used to make sure there are enough data points:
  map<MReadOutElementDoubleStrip, unsigned int> CP_ROEToLine; //peak fits

  for (unsigned int i=0; i<Parser.GetNLines(); i++){
    unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
    if (NTokens < 2) continue;
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0,"CM") == true || 
        Parser.GetTokenizerAt(i)->IsTokenAt(0,"CP") == true ||
        Parser.GetTokenizerAt(i)->IsTokenAt(0,"CR") == true) {

      if (Parser.GetTokenizerAt(i)->IsTokenAt(1,"dss") == true) {
        MReadOutElementDoubleStrip R;
        R.SetDetectorID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2));
        R.SetStripID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3));
        R.IsPositiveStrip(Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "p");

        if (Parser.GetTokenizerAt(i)->IsTokenAt(0,"CM") == true) {
          CM_ROEToLine[R] = i;
        }
        else if (Parser.GetTokenizerAt(i)->IsTokenAt(0,"CP") == true) {
          CP_ROEToLine[R] = i;
        }
        else {
          CR_ROEToLine[R] = i;
        }
      }
    }
  }

  for (auto CM: CM_ROEToLine){

    //only use calibration if we have 3 data points
    if (CP_ROEToLine.find(CM.first) != CP_ROEToLine.end()){
      unsigned int i = CP_ROEToLine[CM.first];
      if (Parser.GetTokenizerAt(i)->IsTokenAt(5,"pakw") == true){
        if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(6) < 3){
          continue;
        }
      }
    }


    //get the fit function from the file
    unsigned int Pos = 5;
    MString CalibratorType = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsString(Pos);
    CalibratorType.ToLower();

    //for now Carolyn just does poly3, so I am only doing that one
    if (CalibratorType == "poly3"){
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a3 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      //from fit parameters, define function
      TF1* melinatorfit = new TF1("poly3","[0]+[1]*x+[2]*x^2+[3]*x^3",0.,8000.);
      melinatorfit->FixParameter(0,a0);
      melinatorfit->FixParameter(1,a1);
      melinatorfit->FixParameter(2,a2);
      melinatorfit->FixParameter(3,a3);

      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_EnergyCalibration[CM.first] = melinatorfit;
    }
  }

  for (auto CR: CR_ROEToLine){

    unsigned int Pos = 5;
    MString CalibratorType = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsString(Pos);
    CalibratorType.ToLower();
    if (CalibratorType == "p1"){
      double f0 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      double f1 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      TF1* resolutionfit = new TF1("P1","sqrt([0]+[1]*x)",0.,2000.);
      resolutionfit->SetParameter(0,f0);
      resolutionfit->SetParameter(1,f1);
      
      m_ResolutionCalibration[CR.first] = resolutionfit;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the dead strip file
void MNCTDetectorEffectsEngineCOSI::ParseDeadStripFile()
{  
  //initialize m_DeadStrips: set all values to 0
  for (int i=0; i<12; i++) {
    for (int j=0; j<2; j++) {
      for (int k=0; k<37; k++) {
        m_DeadStrips[i][j][k] = 0;
      }
    }
  }

  ifstream deadStripFile;
  deadStripFile.open(m_DeadStripFileName);

  if (!deadStripFile.is_open()) {
    cout << "Error opening dead strip file" << endl;
    return;
  }

  string line;
  vector<int> lineVec;
  while (deadStripFile.good() && !deadStripFile.eof()) {
    getline(deadStripFile,line);
    stringstream sLine(line);
    string sub;
    int sub_int;

    while (sLine >> sub){
      sub_int = atoi(sub.c_str());
      lineVec.push_back(sub_int);
    }

    if (lineVec.size() != 3) {
      continue;
    }

    int det = lineVec.at(0);
    int side = lineVec.at(1);
    int strip = lineVec.at(2)-1; //in file, strips go from 1-37; in m_DeadStrips they go from 0-36
    lineVec.clear();

    //any dead strips have their value in m_DeadStrips set to 1 
    m_DeadStrips[det][side][strip] = 1;
  }
}


// MDummy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
