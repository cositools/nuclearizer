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
#include <numeric>
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


#ifdef ___CLING___
ClassImp(MNCTDetectorEffectsEngineCOSI)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MNCTDetectorEffectsEngineCOSI::MNCTDetectorEffectsEngineCOSI()
{
  m_Geometry = nullptr;
  m_OwnGeometry = false;
  m_ShowProgressBar = false;
  m_SaveToFile = false;
	m_ApplyFudgeFactor = true;
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
	gRandom->SetSeed(0);

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
  if (ParseEnergyCalibrationFile() == false) return false;
  //load dead strip information
  if (ParseDeadStripFile() == false) return false;
  //load threshold information
  if (ParseThresholdFile() == false) return false;

	//load charge sharing factors
	if (ParseChargeSharingFile() == false) return false;
  //load charge loss coefficients
  if (InitializeChargeLoss() == false) return false;
	//load crosstalk coefficients
	if (ParseCrosstalkFile() == false) return false;

	//initialize dead time and trigger rates
	for (int i=0; i<nDets; i++){
		m_CCDeadTime[i] = 0; m_TotalDeadTime[i] = 0.; m_TriggerRates[i]=0;
		for (int j=0; j<nDTBuffSlots; j++){
			m_DeadTimeBuffer[i][j] = -1;
		}
	}

	//initialize last time to 0 (will this exclude first event?)
	m_LastHitTime = 0;
	for (int i=0; i<nDets; i++){
		m_LastHitTimeByDet[i] = 0;
	}

	//initialize m_FirstTime to max double and m_LastTime to 0
	m_FirstTime = std::numeric_limits<double>::max();
	m_LastTime = 0;

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
  m_StartAreaFarField = m_Reader->GetSimulationStartAreaFarField();

  
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
	m_ShieldDelay = 900.e-9; //this is just a guess based on when veto window occurs!
	m_ShieldVetoWindowSize = 0.4e-6;
  // for shield veto: gets updated with shield event times
  // start at -10s so that it doesn't veto beginning events by accident
  m_ShieldTime = -10;  
 
	m_IsShieldDead = false;

	m_NumShieldCounts = 0;
 
	// initialize constants for charge sharing due to diffusion
	double k = 1.38e-16; //Boltzmann's constant
	double T = 84; //detector temperature in Kelvin
	double e = 4.8e-10; //electron charge in cgs
	double d = 1.5; //thickness in centimeters
	double driftConstant = sqrt(2*k*T*d/e);
	//need to divide each voltage by 299.79 to get statvolts (using cgs units for some reason)
	m_DriftConstant.reserve(nDets);
	m_DriftConstant[0] = driftConstant/sqrt(1000./299.79);
	m_DriftConstant[1] = driftConstant/sqrt(1200/299.79);
	m_DriftConstant[2] = driftConstant/sqrt(1500/299.79);
	m_DriftConstant[3] = driftConstant/sqrt(1500/299.79);
	m_DriftConstant[4] = driftConstant/sqrt(1000/299.79);
	m_DriftConstant[5] = driftConstant/sqrt(1500/299.79);
	m_DriftConstant[6] = driftConstant/sqrt(1000/299.79);
	m_DriftConstant[7] = driftConstant/sqrt(1500/299.79);
	m_DriftConstant[8] = driftConstant/sqrt(1500/299.79);
	m_DriftConstant[9] = driftConstant/sqrt(1200/299.79);
	m_DriftConstant[10] = driftConstant/sqrt(1000/299.79);
	m_DriftConstant[11] = driftConstant/sqrt(1000/299.79);

	//for debugging charge loss
	m_ChargeLossHist = new TH2D("CL","",100,632,667,100,0,50);

  // The statistics:
  m_NumberOfEventsWithADCOverflows = 0;
  m_NumberOfEventsWithNoADCOverflows = 0;
  m_NumberOfFailedIASearches = 0;
  m_NumberOfSuccessfulIASearches = 0;

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


    bool HasOverflow = false;

/*		double initialEnergy = 0.;
		for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){
      MSimHT* HT = SimEvent->GetHTAt(h);
			initialEnergy += HT->GetEnergy();
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
					//this is handling paralyzable dead time
         	m_ShieldTime = evt_time;
        }
	    }
			else if (HT->GetDetectorType() == 3){ hasDetHits = true; }
    }

		if (hasShieldHits == true){ m_NumShieldCounts++; }
		if (increaseShieldDeadTime == true){ m_ShieldDeadTime += m_ShieldPulseDuration; }

		//3 cases to veto events:
		//(1) shield active starts in veto window
		//(2) shield active ends in veto window
		//(3) shield active during the entire veto window
		//this if statement could perhaps be condensed but I'm less confused this way
		if ((m_ShieldTime + m_ShieldDelay > evt_time + m_CCDelay && m_ShieldTime + m_ShieldDelay < evt_time + m_CCDelay + m_ShieldVetoWindowSize) || 
			(m_ShieldTime + m_ShieldDelay + m_ShieldPulseDuration > evt_time + m_CCDelay && m_ShieldTime + m_ShieldDelay + m_ShieldPulseDuration > evt_time + m_CCDelay + m_ShieldVetoWindowSize) || 
			(m_ShieldTime + m_ShieldDelay < evt_time + m_CCDelay && m_ShieldTime + m_ShieldDelay + m_ShieldPulseDuration > evt_time + m_CCDelay + m_ShieldVetoWindowSize)){
// 		  delete SimEvent;
//      continue;
			//don't delete the event yet: need to apply dead time to the card cage first!
			m_ShieldVeto = true;
    }
		else { m_ShieldVeto = false; }

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
		vector<MNCTDEEStripHit> GuardRingHitsFromChargeSharing;
		vector<int> detectorsHitForShieldVeto(nDets,0);

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
			detectorsHitForShieldVeto[DetectorID] = 1;

      // Convert position into
      MVector PositionInDetector = VS->GetPositionInSensitiveVolume();
      MDGridPoint GP = Detector->GetGridPoint(PositionInDetector);
      double Depth_ = PositionInDetector.GetZ();
      double Depth = -(Depth_ - (m_DepthCalibrator->GetThickness(DetectorID)/2.0)); // change the depth coordinates so that one side is 0.0 cm and the other side is ~1.5cm
			pSide.m_Depth = Depth;
			nSide.m_Depth = Depth;

      // Not sure about if p or n-side is up, but we can debug this later
			// Confirmed by Clio on 11/14/18: this is right
      pSide.m_ROE.SetStripID(38-(GP.GetYGrid()+1));
      nSide.m_ROE.SetStripID(38-(GP.GetXGrid()+1));


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

			//m_EnergyOrig will be unchanged: to see if event is incompletely absorbed or not
			//(m_Energy is changed due to crosstalk and charge loss, etc)
      pSide.m_EnergyOrig = HT->GetEnergy();
      nSide.m_EnergyOrig = HT->GetEnergy();
			//hit index to keep track of which SimHT this strip hit came from
			pSide.m_HitIndex = h;
			nSide.m_HitIndex = h;

			MVector PosFromGP = GP.GetPosition();

      pSide.m_Position = PositionInDetector;
      nSide.m_Position = PositionInDetector;


			// (1aa): charge sharing due to diffusion

			//get origins: these are the IA indices
	    vector<int> Origins = HT->GetOrigins();
      pSide.m_Origins = list<int>(Origins.begin(), Origins.end());
      nSide.m_Origins = list<int>(Origins.begin(), Origins.end());

			//group origins for this HT by position
			//  and figure out each energy is deposited at each position
			MVector PrevPos(0,0,0);
			vector<vector<int> > OriginsGroupedByPosition;
			vector<MVector> IAPositions;
			vector<double> EnergyDepositedByPosition;
			vector<int> temp_vec;
			double energyDeposited = 0.;
			double totalEnergyFromIAs = 0.;

			for (unsigned int o=0; o<Origins.size(); o++){
				MSimIA* ia = SimEvent->GetIAById(Origins[o]);
				MVector iaPosition = ia->GetPosition();
				if (PrevPos == iaPosition){
					temp_vec.push_back(Origins[o]);
					energyDeposited += ia->GetSecondaryEnergy();
				}
				else {
					if (temp_vec.size() != 0){
						OriginsGroupedByPosition.push_back(temp_vec);
						EnergyDepositedByPosition.push_back(energyDeposited);
						totalEnergyFromIAs += energyDeposited;
					}
					temp_vec.clear();
					temp_vec.push_back(Origins[o]);
					IAPositions.push_back(iaPosition);
					energyDeposited = ia->GetSecondaryEnergy();
				}
				PrevPos = iaPosition;
			}
			if (temp_vec.size() != 0){
				OriginsGroupedByPosition.push_back(temp_vec);
				EnergyDepositedByPosition.push_back(energyDeposited);
				totalEnergyFromIAs += energyDeposited;
			}

			//scale energy deposited so that the sum of the energy deposits equals the HT energy
			if (totalEnergyFromIAs != 0){
				for (unsigned int pos=0; pos<EnergyDepositedByPosition.size(); pos++){
					EnergyDepositedByPosition[pos] = EnergyDepositedByPosition[pos]*(HT->GetEnergy()/totalEnergyFromIAs);
				}
			}

			//initialize these variables before the for loop
			map<unsigned int,double> pStripsEnergies;
			map<unsigned int,double> nStripsEnergies;

			double totalEnergyDeposited = 0.;

			//then apply charge sharing for each POSITION
			//the position could have one or more IAs
			for (unsigned int pos=0; pos<OriginsGroupedByPosition.size(); pos++){
				//figure out the energy deposited for all the IAs
				double energyDeposited = EnergyDepositedByPosition[pos];

				totalEnergyDeposited += energyDeposited;

				//get IA position in detector
				MVector IAPosition = IAPositions[pos];

				MDVolumeSequence* IAVolSeq = m_Geometry->GetVolumeSequencePointer(IAPosition, false, false);
				MVector IAPositionInDetector = IAVolSeq->GetPositionInSensitiveVolume();
				if (IAVolSeq->GetDetector() == 0 || IAVolSeq->GetSensitiveVolume() == 0){
					//if IA not in the detector, just use the HT position
					IAPositionInDetector = PositionInDetector;
					m_NumberOfFailedIASearches += 1;
				} else {
					m_NumberOfSuccessfulIASearches += 1;
				}

				//now we have the energy deposited at this position
				//that is enough information to do the diffusion
				//do half-keV steps to avoid iterating over hundreds of thousands of charge carriers
				// so this isn't really the number of charge carriers, but the number of steps
				double EnergyPerChargeCarrier = 0.5;
				int NChargeCarriers = (int)(energyDeposited/EnergyPerChargeCarrier);
				//unless the deposited energy is perfectly divisible by 0.5, there will be some extra energy
				// need to account for it or else there is extra charge loss
				double ExtraEnergy = energyDeposited - NChargeCarriers*EnergyPerChargeCarrier;

				//figured out by printing out IAPositionInDetector.Z() for Am241 source:
				// IAPositionInDetector.Z() < 0: closer to n side for *all* detector stacks
				double DriftLengthN = IAPositionInDetector.Z() + Detector->GetStructuralSize().Z();
				double DriftLengthP = Detector->GetStructuralSize().Z()*2 - DriftLengthN;
				if (DriftLengthN < 0){ DriftLengthN = 0; }
				if (DriftLengthP < 0){ DriftLengthP = 0; }

				double factorN = m_ChargeSharingFactors[DetectorID][0]->Eval(energyDeposited);
				double factorP = m_ChargeSharingFactors[DetectorID][1]->Eval(energyDeposited);
//				double factorN = 1;
//				double factorP = 1;

				double DriftRadiusSigmaN = m_DriftConstant[DetectorID]*sqrt(DriftLengthN)*factorN;
				double DriftRadiusSigmaP = m_DriftConstant[DetectorID]*sqrt(DriftLengthP)*factorP;

				double DriftX = 0;
				double DriftY = 0;

				for (int i=0; i<NChargeCarriers+1; i++){
					//last iteration is for extra energy -- change EnergyPerChargeCarrier just for last iteration
					if (i == NChargeCarriers){ EnergyPerChargeCarrier = ExtraEnergy; }

					//first n side
					//Rannor draws random x and y from 2D gaussian with mean = 0, sigma = 1
					gRandom->Rannor(DriftX,DriftY);
					DriftX *= DriftRadiusSigmaN;
					DriftY *= DriftRadiusSigmaN;

					MVector DriftPositionN = IAPositionInDetector + MVector(DriftX, DriftY, 0);

					MDGridPoint GPDriftN = Detector->GetGridPoint(DriftPositionN);
					int nStripID;
					//if position isn't in detector (0) or is guard ring (7) assign as guard ring
					if (GPDriftN.GetType() == 7 || GPDriftN.GetType() == 0){
						nStripID = 38;
					}
					else { nStripID = 38-(GPDriftN.GetXGrid()+1); }

					//then p side
					gRandom->Rannor(DriftX,DriftY);
					DriftX *= DriftRadiusSigmaP;
					DriftY *= DriftRadiusSigmaP;

					MVector DriftPositionP = IAPositionInDetector + MVector(DriftX, DriftY, 0);

					MDGridPoint GPDriftP = Detector->GetGridPoint(DriftPositionP);
					int pStripID;
					if (GPDriftP.GetType() == 7 || GPDriftP.GetType() == 0){
						pStripID = 38;
					}
					else { pStripID = 38-(GPDriftP.GetYGrid()+1); }

					//save which strips have been hit
					if (pStripsEnergies.count(pStripID) == 0){
						pStripsEnergies[pStripID] = 0.;
					}
					pStripsEnergies[pStripID] += EnergyPerChargeCarrier;

					if (nStripsEnergies.count(nStripID) == 0){
						nStripsEnergies[nStripID] = 0.;
					}
					nStripsEnergies[nStripID] += EnergyPerChargeCarrier;
				}

			}


			//for debugging
			double pEnergySum = 0.;
			double nEnergySum = 0.;

			//lists of strips that charge cloud hit: at least one must be original strip
			double energyAddToOrigP = 0;
			for (auto P: pStripsEnergies){
				pEnergySum += P.second;
				//change the energy of original strip
				if (pSide.m_ROE.GetStripID() == P.first){
					pSide.m_Energy = P.second;
					pSide.m_EnergyOrig = P.second;
				}
				//make new strip hit if needed
				//guard ring hit
				else if (P.first == 38){
					MNCTDEEStripHit chargeShareGRHit;
					chargeShareGRHit.m_ROE.IsPositiveStrip(true);
					chargeShareGRHit.m_ROE.SetDetectorID(pSide.m_ROE.GetDetectorID());
					chargeShareGRHit.m_ROE.SetStripID(38);
					chargeShareGRHit.m_Energy = P.second;
					chargeShareGRHit.m_Position = MVector(0,0,0); // apparently not important
					GuardRingHitsFromChargeSharing.push_back(chargeShareGRHit);
				}
				//normal strip hit
				else {
					MNCTDEEStripHit chargeShareStrip;
					chargeShareStrip.m_ROE.IsPositiveStrip(true);
					chargeShareStrip.m_ROE.SetStripID(P.first);
					chargeShareStrip.m_ROE.SetDetectorID(pSide.m_ROE.GetDetectorID());
					chargeShareStrip.m_Timing = pSide.m_Timing;
					chargeShareStrip.m_Energy = P.second;
					chargeShareStrip.m_EnergyOrig = P.second;
					chargeShareStrip.m_Depth = pSide.m_Depth;
					chargeShareStrip.m_Position = pSide.m_Position;
					chargeShareStrip.m_Origins = pSide.m_Origins;
					chargeShareStrip.m_HitIndex = pSide.m_HitIndex;
					StripHits.push_back(chargeShareStrip);
				}
			}

			double energyAddToOrigN = 0;
			for (auto N: nStripsEnergies){
				nEnergySum += N.second;
				if (nSide.m_ROE.GetStripID() == N.first){
					nSide.m_Energy = N.second;
					nSide.m_EnergyOrig = N.second;
				}
				else if (N.first == 38){
					MNCTDEEStripHit chargeShareGRHit;
					chargeShareGRHit.m_ROE.IsPositiveStrip(true);
					chargeShareGRHit.m_ROE.SetDetectorID(nSide.m_ROE.GetDetectorID());
					chargeShareGRHit.m_ROE.SetStripID(38);
					chargeShareGRHit.m_Energy = N.second;
					chargeShareGRHit.m_Position = MVector(0,0,0);
					GuardRingHitsFromChargeSharing.push_back(chargeShareGRHit);
				}
				else {
					MNCTDEEStripHit chargeShareStrip;
					chargeShareStrip.m_ROE.IsPositiveStrip(false);
					chargeShareStrip.m_ROE.SetStripID(N.first);
					chargeShareStrip.m_ROE.SetDetectorID(nSide.m_ROE.GetDetectorID());
					chargeShareStrip.m_Timing = nSide.m_Timing;
					chargeShareStrip.m_Energy = N.second;
					chargeShareStrip.m_EnergyOrig = N.second;
					chargeShareStrip.m_Depth = nSide.m_Depth;
					chargeShareStrip.m_Position = nSide.m_Position;
					chargeShareStrip.m_Origins = nSide.m_Origins;
					chargeShareStrip.m_HitIndex = nSide.m_HitIndex;
					StripHits.push_back(chargeShareStrip);
				}
			}

			pSide.m_Energy += energyAddToOrigP;
			nSide.m_Energy += energyAddToOrigN;
			pSide.m_EnergyOrig += energyAddToOrigP;
			nSide.m_EnergyOrig += energyAddToOrigN;

      StripHits.push_back(pSide);
      StripHits.push_back(nSide);

      m_TotalHitsCounter++;

    }

		//delete event and update deadtime if the event was vetoed by the shields
		//can't do this earlier because need to know which detectors got hit
		if (m_ShieldVeto){
			for (int det=0; det<nDets; det++){
				if (detectorsHitForShieldVeto[det] == 1){
					//make sure CC not already dead
					if (evt_time > m_LastHitTimeByDet[det] + m_CCDeadTime[det]){
						m_CCDeadTime[det] = 1e-5;
						m_LastHitTimeByDet[det] = evt_time;
						m_TotalDeadTime[det] += m_CCDeadTime[det];
					}
				}
			}
			delete SimEvent;
			continue;
		}


    list<MNCTDEEStripHit> GuardRingHits;
    // (1b) The guard ring hits
		vector<int> GRIndices;
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

			//add extra energy from charge sharing to guard ring hits already present
			for (unsigned int gr=0; gr<GuardRingHitsFromChargeSharing.size(); gr++){
				if (GuardRingHit.m_ROE == GuardRingHitsFromChargeSharing[gr].m_ROE){
					GuardRingHit.m_Energy += GuardRingHitsFromChargeSharing[gr].m_Energy;
					GRIndices.push_back(gr);
				}
			}

			GuardRingHits.push_back(GuardRingHit);
    }

		//add guard ring hits from charge sharing that aren't already present
		for (unsigned int h=0; h<GuardRingHitsFromChargeSharing.size(); h++){
			//did we already count this hit?
			if (find(GRIndices.begin(), GRIndices.end(), h) == GRIndices.end()){
				GuardRingHits.push_back(GuardRingHitsFromChargeSharing[h]);
			}
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

		// Step (3): Calculate and noise ADC values including cross talk, charge loss, charge sharing, ADC overflow!

    // (3a) Add energy of all subhits to get energy of each striphit
    for (MNCTDEEStripHit& Hit: MergedStripHits) { 
     	double Energy = 0;
			double EnergyOrig = 0;
      for (MNCTDEEStripHit SubHit: Hit.m_SubStripHits) {
        Energy += SubHit.m_Energy;
				EnergyOrig += SubHit.m_EnergyOrig;
      }

      Hit.m_Energy = Energy;
			Hit.m_EnergyOrig = EnergyOrig;
    }

		// (3b) Charge loss
		list<MNCTDEEStripHit>::iterator sh1, sh2;
		for (sh1 = MergedStripHits.begin(); sh1 != MergedStripHits.end(); ++sh1){
			for (sh2 = sh1; sh2 != MergedStripHits.end(); ++sh2){
				if (sh1 == sh2){ continue; }

				//check if strip hits are adjacent
				bool adjacent = false;
				int stripID1 = (*sh1).m_ROE.GetStripID();
				int stripID2 = (*sh2).m_ROE.GetStripID();
				int detID1 = (*sh1).m_ROE.GetDetectorID();
				int detID2 = (*sh2).m_ROE.GetDetectorID();
				bool side1 = (*sh1).m_ROE.IsPositiveStrip();
				bool side2 = (*sh2).m_ROE.IsPositiveStrip();
				if (abs(stripID1-stripID2) == 1 && side1 == side2 && detID1 == detID2){
					adjacent = true;
				}

				//if adjacent, check if strip hits share origins
				bool sharedOrigin = false;
				if (adjacent){
					for (int o1: (*sh1).m_Origins){
						for (int o2: (*sh2).m_Origins){
							if (o1 == o2){
								sharedOrigin = true;
								break;
							}
						}
					}
				}

				//if shared origin and adjacent, apply charge loss effect -- only on p side
				if (adjacent && sharedOrigin){
					double energy1 = (*sh1).m_Energy;
					double energy2 = (*sh2).m_Energy;
					double depth1 = (*sh1).m_Depth;
					double depth2 = (*sh2).m_Depth;
					if (side1 && depth1 == depth2){
						vector<double> newEnergies = ApplyChargeLoss(energy1,energy2,detID1,0,depth1,depth2);
						(*sh1).m_Energy = newEnergies.at(0);
						(*sh2).m_Energy = newEnergies.at(1);
					}
				}

			}
		}
 

		// (3c) Cross talk

    //Identify hits that need crosstalk
    double sim_arr[MergedStripHits.size()][5];
    list<MNCTDEEStripHit>::iterator i = MergedStripHits.begin();
    int i2 = 0;
    while (i != MergedStripHits.end()) {
      int sdet = (*i).m_ROE.GetDetectorID();
      bool bside = (*i).m_ROE.IsPositiveStrip();
      int sside = 0;
      if (bside == true) {sside = 1;}
      int sstrip = (*i).m_ROE.GetStripID();
      double senergy = (*i).m_Energy;

      sim_arr[i2][0] = i2;
      sim_arr[i2][1] = sdet;
      sim_arr[i2][2] = sside;
      sim_arr[i2][3] = sstrip;
      sim_arr[i2][4] = senergy;

      ++i;
      ++i2;
    }

    //Add cross talk energy to chosen strips
    //E_sim = M^-1(E_real+C) <- cross talk correction
    //E_real = (E_sim*M)-C <- adding cross talk
		//CCS 190408: changing sim_energies, matrix, and constant from arrays to vectors
		//  old way didn't compile on mac os
    vector<double> sim_energies = vector<double>(MergedStripHits.size());
		vector<vector<double> > matrix = vector<vector<double> >(MergedStripHits.size(), vector<double> (MergedStripHits.size()));
    vector<double> constant = vector<double>(MergedStripHits.size());

    for (unsigned int i=0; i<MergedStripHits.size(); i++) {
      sim_energies[i] = sim_arr[i][4];
    }
    
    for (unsigned int i=0; i<MergedStripHits.size(); i++) {
      for (unsigned int j=0; j<MergedStripHits.size(); j++) {
        int mdet = sim_arr[i][1];
        int mside = sim_arr[i][2];
        int mstrip = sim_arr[i][3];

        double a0 = m_CrosstalkCoefficients[mdet][mside][0][0];
        double b0 = m_CrosstalkCoefficients[mdet][mside][0][1];
        double a1 = m_CrosstalkCoefficients[mdet][mside][1][0];
        double b1 = m_CrosstalkCoefficients[mdet][mside][1][1];

        if (i == j) {
          matrix[i][j] += 1.0;
        }
        if (sim_arr[j][1] == mdet && sim_arr[j][2] == mside && sim_arr[j][3] == mstrip+1) {
          constant[i] += a0/2.;
          constant[j] += a0/2.;
          matrix[i][j] += b0;
          matrix[j][i] += b0;
        }
        if (sim_arr[j][1] == mdet && sim_arr[j][2] == mside && sim_arr[j][3] == mstrip+2) {
          constant[i] += a1/2.;
          constant[j] += a1/2.;
          matrix[i][j] += b1;
          matrix[j][i] += b1;
        }
      }
    }
    
    vector<double> real_energies = vector<double>(MergedStripHits.size());
    for (unsigned int i=0; i<MergedStripHits.size(); i++) {
      for (unsigned int j=0; j<MergedStripHits.size(); j++) {
        real_energies[i] += matrix[j][i]*sim_energies[j];
      }
    }
    for (unsigned int i=0; i<MergedStripHits.size(); i++) {
      real_energies[i] -= constant[i];
    }

    list<MNCTDEEStripHit>::iterator l = MergedStripHits.begin();
    int l2 = 0;
    while (l != MergedStripHits.end()) {
      (*l).m_Energy = real_energies[l2];

      ++l;
      ++l2;
    }




		// (3d) Give each striphit an noised ADC value; handle ADC overflow
		list<MNCTDEEStripHit>::iterator A = MergedStripHits.begin();
		while (A != MergedStripHits.end()) {
			double Energy = (*A).m_Energy;
			(*A).m_ADC = EnergyToADC((*A),Energy);
			if ((*A).m_ADC > 8192){
				A = MergedStripHits.erase(A);
				HasOverflow = true;
			}
			else {
				++A;
			}
		}



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

			//so that we can use default value if necessary
			MReadOutElementDoubleStrip ROE_map_key = (*k).m_ROE;
			if (m_LLDThresholds.count((*k).m_ROE) == 0){
				ROE_map_key.SetDetectorID(12);
				ROE_map_key.SetStripID(0);
				ROE_map_key.IsPositiveStrip(0);
			}

      if ((*k).m_ADC < m_LLDThresholds[ROE_map_key]) {
        k = MergedStripHits.erase(k);
      } else {
				double prob = gRandom->Rndm();
				if (prob > m_FSTThresholds[ROE_map_key]->Eval((*k).m_ADC)){
          (*k).m_Timing = 0.0;
        }
        ++k;
      }
    }

    // (4c) Take care of guard ring vetoes
		list<MNCTDEEStripHit>::iterator gr = GuardRingHits.begin();
		vector<int> grHit = vector<int>(nDets,0);
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
		//update dead time stuff if the hit is vetoed by the guard ring
		for (int det=0; det<nDets; det++){
			if (grHit[det] == 1){
				//make sure CC not already dead
				if (evt_time > m_LastHitTimeByDet[det] + m_CCDeadTime[det]){
					m_CCDeadTime[det] = 1e-5;
					m_LastHitTimeByDet[det] = evt_time;
					m_TotalDeadTime[det] += m_CCDeadTime[det];
				}
			}
		}


		// (4d) Make sure there is at least one strip left on each side of each detector
		//  If not, remove remaining strip(s) from detector because they won't trigger detector
		vector<int> xExists = vector<int>(nDets,0);
		vector<int> yExists = vector<int>(nDets,0);

		//look for (at least) one strip on each side
		list<MNCTDEEStripHit>::iterator tr = MergedStripHits.begin();
		while (tr != MergedStripHits.end()) {
			int DetID = (*tr).m_ROE.GetDetectorID();
			if ((*tr).m_Timing != 0){
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

		//apply dead time for hits that don't have coincidence
		//i.e. hits with strip hits on only x OR y, not both
		for (int det=0; det<nDets; det++){
			if ((xExists[det] == 0 && yExists[det] == 1) || (xExists[det] == 1 && yExists[det] == 0)){
				//make sure CC not already dead
				if (evt_time > m_LastHitTimeByDet[det] + m_CCDeadTime[det]){
					m_CCDeadTime[det] = 1e-5;
					m_LastHitTimeByDet[det] = evt_time;
					m_TotalDeadTime[det] += m_CCDeadTime[det];
				}
			}
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
		vector<int> updateLastHitTime = vector<int>(nDets,0);
		vector<int> detIsDead = vector<int>(nDets,0);

		for (int d=0; d<nDets; d++){
			//second conditional for running multiple sim files when t starts at 0
			if (m_LastHitTimeByDet[d] + m_CCDeadTime[d] > evt_time && m_LastHitTimeByDet[d]<evt_time){ detIsDead[d] = 1; }
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
		for (int d=0; d<nDets; d++){
			if (updateLastHitTime[d] == 1){
				m_LastHitTimeByDet[d] = evt_time;
				m_CCDeadTime[d] = 1e-5;
				m_TotalDeadTime[d] += m_CCDeadTime[d];
			}
		}

		// Step (6.75):
		//figure out if dead time buffers are full, and update them accordingly
		double empty_buffer_val = -1;
		double time_buffer_empty = .000625;

		//increase buffer times if necessary
		for (int d=0; d<nDets; d++){
			int indexOfLargest = -1;
			double maxTime = -1;
			for (int s=0; s<nDTBuffSlots; s++){
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
		vector<int> bufferFull = vector<int>(nDets,0);
 
		//check if buffer is full for each detector
		for (int d=0; d<nDets; d++){
			int nextEmptySlot = 16;
			for (int s=0; s<nDTBuffSlots; s++){
				if (m_DeadTimeBuffer[d][s] == empty_buffer_val){
					nextEmptySlot = s;
					break;
				}
			}
			bufferFull[d] = nextEmptySlot;
		}

		for (int i=0; i<nDets; i++){
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

		//update last time (and first time for first event)
		if (SimEvent->GetTime().GetAsSeconds() < m_FirstTime){
			m_FirstTime = SimEvent->GetTime().GetAsSeconds();
		}
		m_LastTime = SimEvent->GetTime().GetAsSeconds();
		

		// Step (8): Apply fudge factor to completely absorbed events (photopeak)
		//to deal with successor stuff, need to do this for each SimHT
		//but same origin can make multiple SimHTs, so have to add them back together
		if (m_ApplyFudgeFactor){
			map<int,double> initialEnergyByIA;
			map<int,double> finalEnergyByIA;
			map<int,vector<unsigned int> > HitIndexByIA;

			for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){
				MSimHT* Hit = SimEvent->GetHTAt(h);
				int initIA = Hit->GetSmallestOrigin();
				MString IAprocess = SimEvent->GetIAById(initIA)->GetProcess();
				while (IAprocess != "INIT"){
					initIA = SimEvent->GetIAById(initIA)->GetOriginID();
					IAprocess = SimEvent->GetIAById(initIA)->GetProcess();
				}

				double initialEnergy = SimEvent->GetIAById(initIA)->GetSecondaryEnergy();
				double finalEnergy = 0.0;
				for (list<MNCTDEEStripHit>::iterator p=MergedStripHits.begin(); p!=MergedStripHits.end(); ++p){
					if ((*p).m_ROE.IsPositiveStrip() == false && (*p).m_HitIndex == h){
						finalEnergy += (*p).m_EnergyOrig;
					}
				}

				initialEnergyByIA[initIA] = initialEnergy;
				finalEnergyByIA[initIA] += finalEnergy;
				HitIndexByIA[initIA].push_back(h);
			}

			//now that we have initial and final energy for each INIT IA,
			// figure out if IA was completely absorbed or not
			map<int,bool> eraseHit;
			for (auto i: initialEnergyByIA){
				double initialEnergy = i.second;
				double finalEnergy = finalEnergyByIA[i.first];

				double sigma = 8.35e-4*initialEnergy+1.69;
				double windowSize = 1.5*sigma;
				double threshold = 7.04e-5*initialEnergy+0.79;

				if (finalEnergy > initialEnergy-windowSize && finalEnergy < initialEnergy+windowSize){
					double prob = gRandom->Rndm();
					if (prob > threshold){
						eraseHit[i.first] = true;
					}
					else { eraseHit[i.first] = false; }
				}
			}
			//erase strip hits from IAs where probability was above the threshold
			for (auto i: eraseHit){
				if (i.second == true){
					list<MNCTDEEStripHit>::iterator p = MergedStripHits.begin();
					while (p != MergedStripHits.end()){
						bool eraseP = false;
						for (unsigned int j=0; j<HitIndexByIA[i.first].size(); j++){
							if ((*p).m_HitIndex == HitIndexByIA[i.first][j]){
								eraseP = true;
								break;
							}
						}
						if (eraseP){
							p = MergedStripHits.erase(p);
						}
						else { ++p; }
					}
				}
			}
		}
		//check if there are any strip hits left...
 		HitCounter = 0;
		for (MNCTDEEStripHit Hit: MergedStripHits){
			HitCounter++;
		}
		if (HitCounter == 0){
			delete SimEvent;
			continue;
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

    // (3) Take care of some final statistics
    if (HasOverflow == true) {
      m_NumberOfEventsWithADCOverflows += 1;
    } else {
      m_NumberOfEventsWithNoADCOverflows += 1;
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
//	cout << "Num shield counts: " << m_NumShieldCounts << endl;
	cout << "Shield rate (cps): " << m_NumShieldCounts/(m_LastTime-m_FirstTime) << endl;
	cout << "Dead time " << endl;
	for (int i=0; i<nDets; i++){
		cout << i << ":\t" << m_TotalDeadTime[i] << endl;
	}
	cout << "Trigger rates (events per second)" << endl;
	for (int i=0; i<nDets; i++){
		cout << i << ":\t" << m_TriggerRates[i]/(m_LastTime-m_FirstTime) << endl;
	}
	cout << "Shield dead time: " << m_ShieldDeadTime << endl;

	cout << "Max buffer full index: " << m_MaxBufferFullIndex << '\t' << "Detector " << m_MaxBufferDetector << endl;

  cout<<endl;
  cout<<"Ratio of events with ADC overflows: "<<(m_NumberOfEventsWithADCOverflows > 0 ? double(m_NumberOfEventsWithADCOverflows) / (m_NumberOfEventsWithADCOverflows + m_NumberOfEventsWithNoADCOverflows): 0)<<endl;
  cout<<"Ratio of failed IA searches for charge sharing: "<<(m_NumberOfFailedIASearches > 0 ? double(m_NumberOfFailedIASearches) / (m_NumberOfFailedIASearches + m_NumberOfSuccessfulIASearches): 0)<<endl;

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
    ADC_double = Fit->GetX(energy,0.,10000.);
  }


  int ADC = int(ADC_double);
  return ADC;

}


////////////////////////////////////////////////////////////////////////////////


//! Noise shield energy with measured resolution
double MNCTDetectorEffectsEngineCOSI::NoiseShieldEnergy(double energy, MString shield_name)
{ 

  vector<double> resolution_consts{3.75,3.74,18.47,4.23,3.07,3.98};

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
vector<double> MNCTDetectorEffectsEngineCOSI::ApplyChargeLoss(double energy1, double energy2, int detID, int side, double depth1, double depth2){

	double trueSum = energy1+energy2;
	double diff = abs(energy1-energy2);

	//deal with depth
	//use average depth? or don't do charge loss if hits dont have the same depth?
//	double Depth = (depth1+depth2)/2.;
	TH1D DepthBins("DB","",3,0,1.5);
	int depthBin = DepthBins.GetXaxis()->FindBin(depth1)-1;

	//B = A0 + A1*E
	double A0 = m_ChargeLossCoefficients[detID][side][depthBin][0];
	double A1 = m_ChargeLossCoefficients[detID][side][depthBin][1];
	double B = A0 + A1*trueSum;

	//try the Dmax thing
//	double Dmax = trueSum*(trueSum-511./2)/(trueSum+511./2);
//	if (diff < Dmax){ B = 0; }
	if (B < 0){ B = 0; }

	//get new sum
	double newSum;
	if (trueSum >= 300){
		newSum = trueSum - B*(trueSum - diff);
	}
	else {
		newSum = trueSum - (B/(2*trueSum))*(pow(trueSum,2) - pow(diff,2));
	}

	//get new strip hit energies: subtract same amount from energy1 and energy2
	double sumDiff = trueSum - newSum;
	double newE1, newE2;
	newE1 = energy1 - sumDiff/2.;
	newE2 = energy2 - sumDiff/2.;

	m_ChargeLossHist->Fill(trueSum,sumDiff);

	vector<double> retEnergy;

	retEnergy.push_back(newE1);
	retEnergy.push_back(newE2);

	return retEnergy;

}

////////////////////////////////////////////////////////////////////////////////

//! Calculate new summed energy of two strips affected by charge loss
bool MNCTDetectorEffectsEngineCOSI::InitializeChargeLoss()
{ 

	//coefficients[energy][detector][side][depth]
	vector<vector<vector<vector<double> > > > coefficients(4, vector<vector<vector<double> > > (nDets, vector<vector<double> > (nSides, vector<double> (3))));

	MFile File;
	if (File.Open(m_ChargeLossFileName) == false){
		cout << "Unable to open file: " << m_ChargeLossFileName << endl;
		return false;
	}

	MTokenizer Tokenizer;
	MString Line;

	vector<double> energies{122,356,662,1333};

	while (File.ReadLine(Line) == true){
		Tokenizer.Analyze(Line);
		//sometimes somehow I read an empty string
		if (Line.AreIdentical("")){ continue; }

		double energy = Tokenizer.GetTokenAtAsDouble(0);
		int det = Tokenizer.GetTokenAtAsInt(1);
		int side = Tokenizer.GetTokenAtAsInt(2);
		int depthBin = Tokenizer.GetTokenAtAsInt(3)-1;
		double B = Tokenizer.GetTokenAtAsDouble(5);

		int energyIndex = 0;
		for (unsigned int i=0; i<energies.size(); i++){
			if (energies[i] == energy){
				energyIndex = i;
				break;
			}
		}

		coefficients[energyIndex][det][side][depthBin] = B;
	}

  double *energyArr = &energies[0];
  double points[4];
  double A0;
  double A1;

  for (int det=0; det<nDets; det++){
    for (int side=0; side<nSides; side++){
			for (int depthBin=0; depthBin<3; depthBin++){

 	    	points[0] = coefficients[0][det][side][depthBin];
 	    	points[1] = coefficients[1][det][side][depthBin];
				points[2] = coefficients[2][det][side][depthBin];
				points[3] = coefficients[3][det][side][depthBin];

	      TGraph *g = new TGraph(4,energyArr,points);
	      TF1 *f = new TF1("f","[0]+[1]*x",energyArr[0],energyArr[3]);
//	      TF1 *f = new TF1("f","[0]+[1]*x",energies[0],energies[2]);
	      g->Fit("f","RQ");

	      A0 = f->GetParameter(0);
	      A1 = f->GetParameter(1);

	      m_ChargeLossCoefficients[det][side][depthBin][0] = A0;
	      m_ChargeLossCoefficients[det][side][depthBin][1] = A1;

	      delete g;
	      delete f;
	    }
	  }
	}
  
  return true;
}


/////////////////////////////////////////////////////////////////////////////////
//! Read in charge sharing factors
bool MNCTDetectorEffectsEngineCOSI::ParseChargeSharingFile()
{

	MParser Parser;
	if (Parser.Open(m_ChargeSharingFileName) == false){
		cout << "Unable to open charge sharing file" << endl;
		return false;
	}

	for (unsigned int i=0; i<nDets*nSides; i++){
		int det = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(0);
		int side = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
		double slope = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2);
		double yInt = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(3);

		TF1 *f = new TF1(MString(det)+"_"+MString(side),"[0]*x+[1]",0,2000);
		f->SetParameter(0,slope);
		f->SetParameter(1,yInt);

		m_ChargeSharingFactors[det][side] = f;

	}

	return true;

}

///////////////////////////////////////////////////////////////////////////////

//! Read in crosstalk coefficients
bool MNCTDetectorEffectsEngineCOSI::ParseCrosstalkFile()
{

	MParser Parser;
	if (Parser.Open(m_CrosstalkFileName, MFile::c_Read) == false) {
		cout << "Unable to open crosstalk file " << m_CrosstalkFileName << endl;
		return false;
	}

	for (unsigned int i=2; i<50; i++){
		int cdet = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(0);
		int cside = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1);
		int cskip = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2);
		double ca = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(3);
		double cb = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(4);
		m_CrosstalkCoefficients[cdet][cside][cskip][0] = ca;
		m_CrosstalkCoefficients[cdet][cside][cskip][1] = cb;
	}

	return true;

}

////////////////////////////////////////////////////////////////////////////////

//! Read in thresholds
bool MNCTDetectorEffectsEngineCOSI::ParseThresholdFile()
{
  MParser Parser;
  if (Parser.Open(m_ThresholdFileName, MFile::c_Read) == false) {
    cout << "Unable to open threshold file " << m_ThresholdFileName << endl;
    return false;
  }

	//vectors for averaging, for strips where there isn't threshold info for some reason
	vector<double> lldVals;
	vector<double> functionMaxVals;
	vector<double> par0Vals;
	vector<double> par1Vals;
	vector<double> par2Vals;
	vector<double> par3Vals;

  for (unsigned int i=0; i<Parser.GetNLines(); i++) {
    unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
		if (NTokens != 7){ continue; } //this shouldn't happen but just in case

		//decode identifier
		int identifier = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(0);
		int det = identifier / 1000;
		int strip = (identifier % 1000) / 10;
		bool isPos = identifier % 10;

		MReadOutElementDoubleStrip R;
		R.SetDetectorID(det);
		R.SetStripID(strip);
		R.IsPositiveStrip(isPos);

		double lldThresh = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(1);
		double functionMax = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(6);

		m_LLDThresholds[R] = lldThresh;

		TF1* erf = new TF1("erf"+MString(identifier),"[0]*(-1*TMath::Erf(([1]-x)/(sqrt(2)*[2]))+1)+[3]",lldThresh,functionMax);
		erf->SetParameter(1,Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2));
		erf->SetParameter(2,Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(3));
		erf->SetParameter(3,Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(4));
		erf->SetParameter(0,Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(5));

		m_FSTThresholds[R] = erf;

		lldVals.push_back(lldThresh);
		functionMaxVals.push_back(functionMax);
		par0Vals.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(5));
		par1Vals.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2));
		par2Vals.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(3));
		par3Vals.push_back(Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(4));

  }
 
	//add average value as a default
	double lldAvg = accumulate(lldVals.begin(),lldVals.end(),0.0)/lldVals.size();
	double funcMaxAvg = accumulate(functionMaxVals.begin(),functionMaxVals.end(),0.0)/functionMaxVals.size();
	double par0Avg = accumulate(par0Vals.begin(),par0Vals.end(),0.0)/par0Vals.size();
 	double par1Avg = accumulate(par1Vals.begin(),par1Vals.end(),0.0)/par1Vals.size();
	double par2Avg = accumulate(par2Vals.begin(),par2Vals.end(),0.0)/par2Vals.size();
	double par3Avg = accumulate(par3Vals.begin(),par3Vals.end(),0.0)/par3Vals.size();

	MReadOutElementDoubleStrip R;
	R.SetDetectorID(12);
	R.SetStripID(0);
	R.IsPositiveStrip(0);

	m_LLDThresholds[R] = lldAvg;

	TF1* erf = new TF1("erf12000","[0]*(-1*TMath::Erf(([1]-x)/(sqrt(2)*[2]))+1)+[3]",lldAvg,funcMaxAvg);
	erf->SetParameter(0,par0Avg);
	erf->SetParameter(1,par1Avg);
	erf->SetParameter(2,par2Avg);
	erf->SetParameter(3,par3Avg);

	m_FSTThresholds[R] = erf;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse ecal file: should be done once at the beginning to save all the poly3 coefficients
bool MNCTDetectorEffectsEngineCOSI::ParseEnergyCalibrationFile()
{
  MParser Parser;
  if (Parser.Open(m_EnergyCalibrationFileName, MFile::c_Read) == false){
    cout << "Unable to open calibration file " << m_EnergyCalibrationFileName << endl;
    return false;
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

    //for now Carolyn just does poly3 and poly4, so I am only doing those one
    if (CalibratorType == "poly3"){
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a3 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      //from fit parameters, define function
      TF1* melinatorfit = new TF1("poly3","[0]+[1]*x+[2]*x^2+[3]*x^3",0.,8162.);
      melinatorfit->FixParameter(0,a0);
      melinatorfit->FixParameter(1,a1);
      melinatorfit->FixParameter(2,a2);
      melinatorfit->FixParameter(3,a3);

      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_EnergyCalibration[CM.first] = melinatorfit;

    } else if (CalibratorType == "poly4"){
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a3 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a4 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
    	
      //from fit parameters, define function
      TF1* melinatorfit = new TF1("poly4","[0]+[1]*x+[2]*x^2+[3]*x^3+[4]*x^4",0.,8162.);
      melinatorfit->FixParameter(0,a0);
      melinatorfit->FixParameter(1,a1);
      melinatorfit->FixParameter(2,a2);
      melinatorfit->FixParameter(3,a3);	
      melinatorfit->FixParameter(4,a4);

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
      TF1* resolutionfit = new TF1("P1","[0]+[1]*x",0.,2000.);
      resolutionfit->SetParameter(0,f0);
      resolutionfit->SetParameter(1,f1);
      
      m_ResolutionCalibration[CR.first] = resolutionfit;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the dead strip file
bool MNCTDetectorEffectsEngineCOSI::ParseDeadStripFile()
{  
  //initialize m_DeadStrips: set all values to 0
  for (int i=0; i<nDets; i++) {
    for (int j=0; j<nSides; j++) {
      for (int k=0; k<nStrips; k++) {
        m_DeadStrips[i][j][k] = 0;
      }
    }
  }
  
  MString Name = m_DeadStripFileName;
  MFile::ExpandFileName(Name);

  ifstream deadStripFile;
  deadStripFile.open(Name);

  if (!deadStripFile.is_open()) {
    cout << "Error opening dead strip file: " << Name << endl;
    return false;
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
  
  return true;
}


void MNCTDetectorEffectsEngineCOSI::dummy_func(){
	//empty function to make break points for debugger
}

// MDummy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
