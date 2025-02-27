/*
 * MDetectorEffectsEngineSMEX.cxx
 *
 *
 * Copyright (C) by Clio Sleator, Carolyn Kierans, Andreas Zoglauer, Parshad Patel.
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
// MDetectorEffectEngine
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDetectorEffectsEngineSMEX.h"

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

// MEGAlib
#include "MGlobal.h"
#include "MStreams.h"
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MDStrip2D.h"
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MReadOutElementDoubleStrip.h"

// Nuclearizer
#include "MDetectorEffectsEngineSMEX.h"
#include "MDepthCalibrator.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDetectorEffectsEngineSMEX)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MDetectorEffectsEngineSMEX::MDetectorEffectsEngineSMEX()
{
  m_Geometry = nullptr;
  m_OwnGeometry = false;
  m_ShowProgressBar = false;
  m_SaveToFile = false;
  m_ApplyFudgeFactor = true;
  m_ChargeLossHist = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MDetectorEffectsEngineSMEX::~MDetectorEffectsEngineSMEX()
{
  // Intentionally left blank
  
  if (m_OwnGeometry == true) delete m_Geometry;

  for (auto& C: m_EnergyCalibration) {
    delete C.second;
  }
  
  for (auto& C: m_ResolutionCalibration) {
    delete C.second;
  }
}


////////////////////////////////////////////////////////////////////////////////



//! Initialize the module
bool MDetectorEffectsEngineSMEX::Initialize()
{
  m_Random.SetSeed(12345);
  
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
  //load guard ring threshold information
  if (ParseGuardRingThresholdFile() == false) return false;
  
  //load charge sharing factors
  if (ParseChargeSharingFile() == false) return false;
  //load charge loss coefficients
  if (InitializeChargeLoss() == false) return false;
  //load crosstalk coefficients
  if (ParseCrosstalkFile() == false) return false;
  
  // //initialize dead time and trigger rates
  // for (int i=0; i<nASICs; i++){
  //   m_ASICDeadTime[i] = 0; m_TotalDeadTime[i] = 0.; m_TriggerRates[i]=0;
  //   for (int j=0; j<nDTBuffSlots; j++){
  //     m_DeadTimeBuffer[i][j] = -1;
  //   }
  // }

  //initialize dead time and trigger rates
  for (int i=0; i<nDets; i++){
    m_ASICDeadTime[i] = 0; m_TotalDeadTime[i] = 0.; m_TriggerRates[i]=0;
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
  
  m_DepthCalibrator = new MDepthCalibrator();
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
  
  m_NumberOfSimulatedEvents = 0;
  
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
  m_ShieldThreshold = 80.;
  m_ShieldPulseDuration = 1.5e-6;  // From measurements this seems to be ~1.5 us (is this energy dependent?)
  // m_CCDelay = 700.e-9; // Card cage delay, not used now.
  m_StripDelay = 500e-9;
  m_StripDeadTime = 1e-6;
  m_ShieldDelayBefore = 300e-9; // SCB delay before an event is read out
  m_ShieldDelayAfter = 200e-9; // SCB delay before and after an event
  m_ShieldDelay = m_ShieldDelayBefore + m_ShieldDelayAfter;
  m_ShieldVetoWindowSize = 0.4e-6; // Shield veto window size, unchanged from SMEX.
  m_ASICDeadTimePerChannel = 1e-6;
  activated1 = 0;
  activated2 = 0;
  activated3 = 0;
  // for shield veto: gets updated with shield event times
  // start at -10s so that it doesn't veto beginning events by accident
  m_ShieldTime = -10;
  m_LastGoodHitShieldTime = m_ShieldTime;
  m_ShieldDeadTime1 = 1.5e-6;
  m_ShieldDeadTime2 = 1.5e-6;
  m_ShieldDeadTime3 = 1.5e-6;
  m_TotalShieldDeadTime1 = 0;
  m_TotalShieldDeadTime2 = 0;
  m_TotalShieldDeadTime3 = 0;

  
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

//!Function to define deadtime
double MDetectorEffectsEngineSMEX::dTimeShields(int detectors_activated){
  double asic_deadtime = 1e-6;
  double shield_decayTime = 1.5e-6;
  double delayTimeBefore = 300e-9;
  double delayTimeAfter = 200e-9;

  double shields_deadtime = detectors_activated * asic_deadtime;
  if (shield_decayTime > shields_deadtime) {
      shields_deadtime += (shield_decayTime - shields_deadtime);
  }
  shields_deadtime += delayTimeBefore + delayTimeAfter;
  return shields_deadtime;
}


////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

//! Function to define asic deadtime
// ** Needs more work **
// Check if there are neighboring channels. Count the channels only once. The channel map needs to be sorted. 
// Channels needs to be checked to be within delay time window.
double MDetectorEffectsEngineSMEX::dTimeGeDs(vector<int> channels) {
    // Enable line is shared for 4 asics per detector, two per side.
    // Only called if one channel is activated.

    double GeD_deadtime = 0;
    int temp = 0;
    int channelNext = 0;
    int highestCountNear = 0;
    double waitTime = 400e-9;
    double asic_deadtime = 1e-6;
    int channels_activated_count = 1;
    vector<int>::iterator channels_activated;
    // int count_near = 1;
    // int channels_activated = channels.size();
    sort(channels.begin(), channels.end());
    channels_activated = unique(channels.begin(), channels.end());
    channels.resize(distance(channels.begin(), channels_activated));
    channels_activated_count = channels.size();

    if (channels_activated_count < 32) {
      GeD_deadtime = waitTime + (channels_activated_count*asic_deadtime);
    }
    else {
      GeD_deadtime = waitTime + (channels_activated_count*asic_deadtime)+(asic_deadtime*2);
    }

    return GeD_deadtime;
}

///////////////////////////////////////////////////////////////////////////////

//! Analyze whatever needs to be analyzed...
bool MDetectorEffectsEngineSMEX::GetNextEvent(MReadOutAssembly* Event) {

  MSimEvent* SimEvent = nullptr;
  //int RunningID = 0;
  
  while ((SimEvent = m_Reader->GetNextEvent(false)) != nullptr) {
    
    //cout<<endl<<endl<<"ID: "<<SimEvent->GetID()<<endl;
    
    // Always update the number of simulated events, since for that number it doesn't matter if the event passes or not
    m_NumberOfSimulatedEvents = SimEvent->GetSimulationEventID();
    
    
    if (SimEvent->GetNHTs() == 0) {
      //cout<<SimEvent->GetID()<<": No hits"<<endl;
      delete SimEvent;
      continue;
    }
    
    // Step (-1): Include aspect information
    //		cout << SimEvent->GetGalacticPointingXAxis() << endl;
    //		cout << SimEvent->GetGalacticPointingZAxis() << endl;
    
    bool HasOverflow = false;
    
    double eventInitialEnergy = 0.;
    for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){
      MSimHT* HT = SimEvent->GetHTAt(h);
      eventInitialEnergy += HT->GetEnergy();
    }
    
    
    // Step (0): Check whether events should be vetoed
    double evt_time = SimEvent->GetTime().GetAsSeconds();
    bool hasDetHits = false;
    bool hasShieldHits = false;
    bool increaseShieldDeadTime1 = false;
    bool increaseShieldDeadTime2 = false;
    bool increaseShieldDeadTime3 = false;
    m_IsShieldDead = false;
    
    // // first check if there's another shield hit above the threshold
    // // if so, veto event
    // for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){
    //   MSimHT* HT = SimEvent->GetHTAt(h);
    //   if (HT->GetDetectorType() == 4) {
    //     MDVolumeSequence* VS = HT->GetVolumeSequence();
    //     MDDetector* Detector = VS->GetDetector();
    //     MString DetName = Detector->GetName();
        
    //     //ONLY veto CsI shields, NOT NaI for polarization calibration
    //     if (DetName.GetSubString(0,6) == "Shield"){
          
    //       double energy = HT->GetEnergy();
    //       energy = NoiseShieldEnergy(energy,DetName);
    //       HT->SetEnergy(energy);
          
    //       if (energy > m_ShieldThreshold) {
    //         if (m_ShieldTime + m_ShieldVetoWindowSize < evt_time){ hasShieldHits = true; }
    //         increaseShieldDeadTime = true;
    //         //this is handling paralyzable dead time
    //         m_ShieldTime = evt_time;
    //       }
    //     }
    //     else if (HT->GetDetectorType() == 3){ hasDetHits = true; }
    //   }
    // }

    // Need to update deadtime for the current detector if there is an another shield that read out an event. 
    // Shields are paired so that Det 1, 2 are read using 1 ASIC and so forth. This is actually 4 BGOs per ASIC.
    // This will change in the future when more detectors are added. Need to change the ShieldVetoWindowSize add to something more realistic.
    for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){
      increaseShieldDeadTime1 = false;
      increaseShieldDeadTime2 = false;
      increaseShieldDeadTime3 = false;        
      MSimHT* HT = SimEvent->GetHTAt(h);
      if (HT->GetDetectorType() == 4) {
        MDVolumeSequence* VS = HT->GetVolumeSequence();
        MDDetector* Detector = VS->GetDetector();
        MString DetName = Detector->GetName();
        if (DetName.GetSubString(0,6) == "Shield"){
          int DetNum = atoi(DetName.GetSubString(6,7));
          double energy = HT->GetEnergy();
          energy = NoiseShieldEnergy(energy,DetName);
          HT->SetEnergy(energy);
          switch(DetNum){
            case 1 :
              if ((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDelayBefore > evt_time)){
                activated1 += 1;
                increaseShieldDeadTime1 = true;
                m_ShieldTime = evt_time;
                hasShieldHits = true;
                m_IsShieldDead = false;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime1 > evt_time)){
                m_IsShieldDead = true;
                // activated1 = 1;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime1 < evt_time)){
                m_ShieldDeadTime1 = 1.5e-6;
                hasShieldHits = true;
                m_LastGoodHitShieldTime = evt_time;
                m_ShieldTime = evt_time;
                activated1 = 1;
                m_IsShieldDead = false;
              }
              break;
            case 2 :
              if ((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDelayBefore > evt_time)){
                activated1 += 1;
                increaseShieldDeadTime1 = true;
                m_ShieldTime = evt_time;
                hasShieldHits = true;
                m_IsShieldDead = false;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime1 > evt_time)){
                m_IsShieldDead = true;
                // activated1 = 1;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime1 < evt_time)){
                m_ShieldDeadTime1 = 1.5e-6;
                hasShieldHits = true;
                m_LastGoodHitShieldTime = evt_time;
                m_ShieldTime = evt_time;
                activated1 = 1;
                m_IsShieldDead = false;
              }
              break;
            case 3 :
              if ((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDelayBefore > evt_time)){
                activated2 += 1;
                increaseShieldDeadTime2 = true;
                m_ShieldTime = evt_time;
                hasShieldHits = true;
                m_IsShieldDead = false;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime2 > evt_time)){
                m_IsShieldDead = true;
                // activated1 = 1;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime2 < evt_time)){
                m_ShieldDeadTime2 = 1.5e-6;
                hasShieldHits = true;
                m_LastGoodHitShieldTime = evt_time;
                m_ShieldTime = evt_time;
                activated2 = 1;
                m_IsShieldDead = false;
              }
              break;
            case 4 :
              if ((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDelayBefore > evt_time)){
                activated2 += 1;
                increaseShieldDeadTime2 = true;
                m_ShieldTime = evt_time;
                hasShieldHits = true;
                m_IsShieldDead = false;

              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime2 > evt_time)){
                m_IsShieldDead = true;
                // activated1 = 1;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime2 < evt_time)){
                m_ShieldDeadTime2 = 1.5e-6;
                hasShieldHits = true;
                m_LastGoodHitShieldTime = evt_time;
                m_ShieldTime = evt_time;
                activated2 = 1;
                m_IsShieldDead = false;
              }
              break;
            case 5 :
              if ((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDelayBefore > evt_time)){
                activated3 += 1;
                increaseShieldDeadTime3 = true;
                m_ShieldTime = evt_time;
                hasShieldHits = true;
                m_IsShieldDead = false;

              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime3 > evt_time)){
                m_IsShieldDead = true;
                // activated1 = 1;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime3 < evt_time)){
                m_ShieldDeadTime3 = 1.5e-6;
                hasShieldHits = true;
                m_LastGoodHitShieldTime = evt_time;
                m_ShieldTime = evt_time;
                activated3 = 1;
                m_IsShieldDead = false;
              }
              break;
            case 6 :
              if ((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDelayBefore > evt_time)){
                activated3 += 1;
                increaseShieldDeadTime3 = true;
                m_ShieldTime = evt_time;
                hasShieldHits = true;
                m_IsShieldDead = false;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime3 > evt_time)){
                m_IsShieldDead = true;
                // activated1 = 1;
              }
              else if((energy > m_ShieldThreshold) && (m_LastGoodHitShieldTime + m_ShieldDeadTime3 < evt_time)){
                m_ShieldDeadTime3 = 1.5e-6;
                hasShieldHits = true;
                m_LastGoodHitShieldTime = evt_time;
                m_ShieldTime = evt_time;
                activated3 = 1;
                m_IsShieldDead = false;
              }
              break;
            }
          }
        }
        else if (HT->GetDetectorType() == 3){ hasDetHits = true; }
      }
    
    
    if (hasShieldHits == true){ m_NumShieldCounts++; }
    if (increaseShieldDeadTime1 == true){ 
      m_ShieldDeadTime1 = dTimeShields(activated1); 
      m_TotalShieldDeadTime1 += m_ShieldDeadTime1;
    }
    if (increaseShieldDeadTime2 == true){ 
      m_ShieldDeadTime2 = dTimeShields(activated2); 
      m_TotalShieldDeadTime2 += m_ShieldDeadTime2;
    }
    if (increaseShieldDeadTime3 == true){ 
      m_ShieldDeadTime3 = dTimeShields(activated3); 
      m_TotalShieldDeadTime3 += m_ShieldDeadTime3;
    }
    
    //3 cases to veto events:
    //(1) shield active starts in veto window
    //(2) shield active ends in veto window
    //(3) shield active during the entire veto window
    //this if statement could perhaps be condensed but I'm less confused this way
    // if ((m_ShieldTime + m_ShieldDelay > evt_time + m_StripDelay && m_ShieldTime + m_ShieldDelay < evt_time + m_StripDelay + m_ShieldVetoWindowSize) || 
    //   (m_ShieldTime + m_ShieldDelay + m_ShieldPulseDuration > evt_time + m_StripDelay && m_ShieldTime + m_ShieldDelay + m_ShieldPulseDuration > evt_time + m_StripDelay + m_ShieldVetoWindowSize) || 
    //   (m_ShieldTime + m_ShieldDelay < evt_time + m_StripDelay && m_ShieldTime + m_ShieldDelay + m_ShieldPulseDuration > evt_time + m_StripDelay + m_ShieldVetoWindowSize)){
    //   // 		  delete SimEvent;
    //   //      continue;
    //   //don't delete the event yet: need to apply dead time to the card cage first!
    //   m_ShieldVeto = true;
    //   }
    //   else { m_ShieldVeto = false; }
    if (m_IsShieldDead) {
      m_ShieldVeto = true;
    }
    else {m_ShieldVeto = false;}
    
    // for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){ 
    //   MSimHT* HT = SimEvent->GetHTAt(h);
    //   if (HT->GetDetectorType() == 3) {
        
    //   }
    // }

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
      list<MDEEStripHit> StripHits;
      vector<MDEEStripHit> GuardRingHitsFromChargeSharing;
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
        
        
        MDEEStripHit pSide;
        MDEEStripHit nSide;
        
        //should be unique identifiers
        pSide.m_ID = h*10;
        nSide.m_ID = h*10+5;
        
        pSide.m_OppositeStrip = nSide.m_ID;
        nSide.m_OppositeStrip = pSide.m_ID;
        
        pSide.m_ROE.IsLowVoltageStrip(true);
        nSide.m_ROE.IsLowVoltageStrip(false);
        
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
        
        //cout<<"Set energy: "<<HT->GetEnergy()<<endl;

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
        
        // Get the origins: these are the IA indices
        // We have to do a bit of a convoluted assignment since different version of MEGAlib have different types (int vs. unsigned int)
        auto HTOrigins = HT->GetOrigins();
        vector<int> Origins(HTOrigins.begin(), HTOrigins.end());

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
        
        for (unsigned int o = 0; o < Origins.size(); o++){
          int iaID = Origins[o];
          //for some reason Origin[o] is 0 when the IAs aren't saved,
          //which makes the code crash unless I do this
          if (iaID == 0){
            iaID++;
          }
          MSimIA* ia = SimEvent->GetIAById(iaID);
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
          
          MDVolumeSequence IAVolSeq = m_Geometry->GetVolumeSequence(IAPosition, false, false);
          MVector IAPositionInDetector = IAVolSeq.GetPositionInSensitiveVolume();
          if (IAVolSeq.GetDetector() == 0 || IAVolSeq.GetSensitiveVolume() == 0){
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
          
          double xDetectorHalfWidth = 0.5*dynamic_cast<MDStrip2D*>(Detector)->GetWidthX();
          double xDetectorOffset = dynamic_cast<MDStrip2D*>(Detector)->GetOffsetX();
          double xInvDetectorPitch = 1.0/dynamic_cast<MDStrip2D*>(Detector)->GetPitchX();
          
          double yDetectorHalfWidth = 0.5*dynamic_cast<MDStrip2D*>(Detector)->GetWidthY();
          double yDetectorOffset = dynamic_cast<MDStrip2D*>(Detector)->GetOffsetY();
          double yInvDetectorPitch = 1.0/dynamic_cast<MDStrip2D*>(Detector)->GetPitchY();
          
          double xInDet = IAPositionInDetector.X() + xDetectorHalfWidth - xDetectorOffset;
          double yInDet = IAPositionInDetector.Y() + yDetectorHalfWidth - yDetectorOffset;
          
          int nStripID = 0;
          int pStripID = 0;

          // ---> Time critical
          for (int i = 0; i < NChargeCarriers + 1; ++i) {
            //last iteration is for extra energy -- change EnergyPerChargeCarrier just for last iteration
            if (i == NChargeCarriers) { 
              EnergyPerChargeCarrier = ExtraEnergy; 
            }
            
            
            // First n side
            // Draw random x and y from 2D gaussian with mean = 0, sigma = 1
            double y = m_Random.Rndm();
            double z = m_Random.Rndm();
            double x = z * 6.28318530717958623;
            double r = sqrt(-2*log(y));
            DriftX = r * sin(x) * DriftRadiusSigmaN;
            DriftY = r * cos(x) * DriftRadiusSigmaN;
            
            // We need both to know when we are in the guard ring
            int nStripIDinterim = (int) floor((DriftX + xInDet)*xInvDetectorPitch);
            int pStripIDinterim = (int) floor((DriftY + yInDet)*yInvDetectorPitch);
            if (nStripIDinterim < 0 || nStripIDinterim > 36 || pStripIDinterim < 0 || pStripIDinterim > 36) {
              nStripID = 38;
            } else {
              nStripID = 37 - nStripIDinterim; 
            }

            
            
            // Then p side
            
            y = m_Random.Rndm();
            z = m_Random.Rndm();
            x = z * 6.28318530717958623;
            r = sqrt(-2*log(y));
            DriftX = r * sin(x) * DriftRadiusSigmaP;
            DriftY = r * cos(x) * DriftRadiusSigmaP;
            
            nStripIDinterim = (int) floor((DriftX + xInDet)*xInvDetectorPitch);
            pStripIDinterim = (int) floor((DriftY + yInDet)*yInvDetectorPitch);
            if (nStripIDinterim < 0 || nStripIDinterim > 36 || pStripIDinterim < 0 || pStripIDinterim > 36) {
              pStripID = 38;
            } else {
              pStripID = 37 - pStripIDinterim; 
            }
            
            
            
            // Save which strips have been hit
            if (pStripsEnergies.count(pStripID) == 0){
              pStripsEnergies[pStripID] = 0.;
            }
            pStripsEnergies[pStripID] += EnergyPerChargeCarrier;
            
            if (nStripsEnergies.count(nStripID) == 0){
              nStripsEnergies[nStripID] = 0.;
            }
            nStripsEnergies[nStripID] += EnergyPerChargeCarrier;
          }
          // <--- Time critical
          
          /*
          // Clio's original
          for (int i=0; i<NChargeCarriers+1; i++){
            //last iteration is for extra energy -- change EnergyPerChargeCarrier just for last iteration
            if (i == NChargeCarriers){ EnergyPerChargeCarrier = ExtraEnergy; }
            
            //first n side
            //Rannor draws random x and y from 2D gaussian with mean = 0, sigma = 1
            m_Random.Rannor(DriftX,DriftY);
            DriftX *= DriftRadiusSigmaN;
            DriftY *= DriftRadiusSigmaN;
            
            MVector DriftPositionN = IAPositionInDetector + MVector(DriftX, DriftY, 0);
            
            MDGridPoint GPDriftN = Detector->GetGridPoint(DriftPositionN);
            int nStripID;
            //if position isn't in detector (0) or is guard ring (7) assign as guard ring
            if (GPDriftN.GetType() == MDGridPoint::c_GuardRing || GPDriftN.GetType() == MDGridPoint::c_Unknown){
              nStripID = 38;
            }
            else { nStripID = 38-(GPDriftN.GetXGrid()+1); }
            
            //then p side
            m_Random.Rannor(DriftX,DriftY);
            DriftX *= DriftRadiusSigmaP;
            DriftY *= DriftRadiusSigmaP;
            
            MVector DriftPositionP = IAPositionInDetector + MVector(DriftX, DriftY, 0);
            
            MDGridPoint GPDriftP = Detector->GetGridPoint(DriftPositionP);
            int pStripID;
            if (GPDriftP.GetType() == MDGridPoint::c_GuardRing || GPDriftP.GetType() == MDGridPoint::c_Unknown){
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
          */
        }
        
        //lists of strips that charge cloud hit: at least one must be original strip
        //what if no charge is on the original strip? this happens occasionally
        bool pOrigHit = false;
        bool nOrigHit = false;
        
        for (auto P: pStripsEnergies) {
          //change the energy of original strip
          if (pSide.m_ROE.GetStripID() == P.first){
            //cout<<"pSide: "<<P.second<<endl;
            pSide.m_Energy = P.second;
            pSide.m_EnergyOrig = P.second;
            pOrigHit = true;
          }
          //make new strip hit if needed
          //guard ring hit
          else if (P.first == 38){
            MDEEStripHit chargeShareGRHit;
            chargeShareGRHit.m_ROE.IsLowVoltageStrip(true);
            chargeShareGRHit.m_ROE.SetDetectorID(pSide.m_ROE.GetDetectorID());
            chargeShareGRHit.m_ROE.SetStripID(38);
            chargeShareGRHit.m_Energy = P.second;
            chargeShareGRHit.m_Position = MVector(0,0,0); // apparently not important
            GuardRingHitsFromChargeSharing.push_back(chargeShareGRHit);
          }
          //normal strip hit
          else {
            MDEEStripHit chargeShareStrip;
            chargeShareStrip.m_ROE.IsLowVoltageStrip(true);
            chargeShareStrip.m_ROE.SetStripID(P.first);
            chargeShareStrip.m_ROE.SetDetectorID(pSide.m_ROE.GetDetectorID());
            chargeShareStrip.m_Timing = pSide.m_Timing;
            //cout<<"chargeShareStrip: "<<P.second<<endl;
            chargeShareStrip.m_Energy = P.second;
            chargeShareStrip.m_EnergyOrig = P.second;
            chargeShareStrip.m_Depth = pSide.m_Depth;
            chargeShareStrip.m_Position = pSide.m_Position;
            chargeShareStrip.m_Origins = pSide.m_Origins;
            chargeShareStrip.m_HitIndex = pSide.m_HitIndex;
            StripHits.push_back(chargeShareStrip);
          }
        }
        
        for (auto N: nStripsEnergies){
          if (nSide.m_ROE.GetStripID() == N.first){
            nSide.m_Energy = N.second;
            nSide.m_EnergyOrig = N.second;
            nOrigHit = true;
          }
          else if (N.first == 38){
            MDEEStripHit chargeShareGRHit;
            chargeShareGRHit.m_ROE.IsLowVoltageStrip(false);
            chargeShareGRHit.m_ROE.SetDetectorID(nSide.m_ROE.GetDetectorID());
            chargeShareGRHit.m_ROE.SetStripID(38);
            chargeShareGRHit.m_Energy = N.second;
            chargeShareGRHit.m_Position = MVector(0,0,0);
            GuardRingHitsFromChargeSharing.push_back(chargeShareGRHit);
          }
          else {
            MDEEStripHit chargeShareStrip;
            chargeShareStrip.m_ROE.IsLowVoltageStrip(false);
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
        
        if (pOrigHit){ StripHits.push_back(pSide); }
        if (nOrigHit){ StripHits.push_back(nSide); }
        
        m_TotalHitsCounter++;
        
      }
    
    // Check if event if within delay time window and add the channel activated to the channel list. 
    // Need to group channels by asic and make different channel vectors for each ASIC.
    // Call dTimeGeDs when all channels are done in one ASIC.

      // for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){     
      //   MSimHT* HT = SimEvent->GetHTAt(h);
      //   if (HT->GetDetectorType() == 3) {
      //     if (evt_time > m_ShieldTime) {}
      //   }
      // }


      //delete event and update deadtime if the event was vetoed by the shields
      //can't do this earlier because need to know which detectors got hit
      // if (m_ShieldVeto){
      //   for (int i=0; i<nASICs; i++){
      //     for (int det=0; det<nDets; det++){
      //       if (detectorsHitForShieldVeto[det] == 1){
      //       //make sure CC not already dead
      //         if (evt_time > m_LastHitTimeByDet[det] + m_ASICDeadTime[i]){
      //           m_ASICDeadTime[i] = 1e-6;
      //           m_LastHitTimeByDet[det] = evt_time;
      //           m_TotalDeadTime[det] += m_ASICDeadTime[i];
      //         }
      //       }
      //     }
      //   }
      //   delete SimEvent;
      //   continue;
      // }

      //delete event and update deadtime if the event was vetoed by the shields
      //can't do this earlier because need to know which detectors got hit
      if (m_ShieldVeto){
        for (int det=0; det<nDets; det++){
          if (detectorsHitForShieldVeto[det] == 1){
            //make sure CC not already dead
            if (evt_time > m_LastHitTimeByDet[det] + m_ASICDeadTime[det]){
              m_ASICDeadTime[det] = 1e-6;
              m_LastHitTimeByDet[det] = evt_time;
              m_TotalDeadTime[det] += m_ASICDeadTime[det];
            }
          }
        }
        delete SimEvent;
        continue;
      }

      
      list<MDEEStripHit> GuardRingHits;
      // (1b) The guard ring hits
      vector<int> GRIndices;
      for (unsigned int h = 0; h < SimEvent->GetNGRs(); ++h) {
        MSimGR* GR = SimEvent->GetGRAt(h);
        MDVolumeSequence* VS = GR->GetVolumeSequence();
        MDDetector* Detector = VS->GetDetector();
        MString DetectorName = Detector->GetName();
        DetectorName.RemoveAllInPlace("Detector_");
        int DetectorID = DetectorName.ToInt();
        
        MDEEStripHit GuardRingHitP;
        GuardRingHitP.m_ROE.IsLowVoltageStrip(true);
        GuardRingHitP.m_ROE.SetDetectorID(DetectorID);
        GuardRingHitP.m_ROE.SetStripID(38); // ?
        GuardRingHitP.m_Energy = GR->GetEnergy();
        GuardRingHitP.m_Position = MVector(0, 0, 0); // <-- not important
        
        MDEEStripHit GuardRingHitN;
        GuardRingHitN.m_ROE.IsLowVoltageStrip(false);
        GuardRingHitN.m_ROE.SetDetectorID(DetectorID);
        GuardRingHitN.m_ROE.SetStripID(38); // ?
        GuardRingHitN.m_Energy = GR->GetEnergy();
        GuardRingHitN.m_Position = MVector(0, 0, 0); // <-- not important
        
        //add extra energy from charge sharing to guard ring hits already present
        for (unsigned int gr=0; gr<GuardRingHitsFromChargeSharing.size(); gr++){
          if (GuardRingHitP.m_ROE == GuardRingHitsFromChargeSharing[gr].m_ROE){
            GuardRingHitP.m_Energy += GuardRingHitsFromChargeSharing[gr].m_Energy;
            GRIndices.push_back(gr);
          }
          if (GuardRingHitN.m_ROE == GuardRingHitsFromChargeSharing[gr].m_ROE){
            GuardRingHitN.m_Energy += GuardRingHitsFromChargeSharing[gr].m_Energy;
            GRIndices.push_back(gr);
          }
        }
        
        GuardRingHits.push_back(GuardRingHitP);
        GuardRingHits.push_back(GuardRingHitN);
      }
      
      //add guard ring hits from charge sharing that aren't already present
      for (unsigned int h=0; h<GuardRingHitsFromChargeSharing.size(); h++){
        //did we already count this hit?
        if (find(GRIndices.begin(), GRIndices.end(), h) == GRIndices.end()){
          GuardRingHits.push_back(GuardRingHitsFromChargeSharing[h]);
        }
      }

      
      // (1c): Merge strip hits
      list<MDEEStripHit> MergedStripHits;
      while (StripHits.size() > 0) {
        MDEEStripHit Start;
        Start.m_SubStripHits.push_back(StripHits.front());
        StripHits.pop_front();
        Start.m_ROE = Start.m_SubStripHits.front().m_ROE;
        
        //      cout << "------" << endl;
        //      cout << Start.m_SubStripHits[0].m_Energy << '\t';
        //      cout << Start.m_SubStripHits[0].m_OppositeStrip->m_Energy << endl;
        
        list<MDEEStripHit>::iterator i = StripHits.begin();
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
      for (MDEEStripHit& Hit: MergedStripHits){
        int nIndep = 0;
        int nSubHits = Hit.m_SubStripHits.size();
        if (nSubHits > 1){
          for (int i=0; i<nSubHits; i++){
            bool sharedOrigin = false;
            for (int j=0; j<nSubHits; j++){
              if (i != j){
                MDEEStripHit& SubHit1 = Hit.m_SubStripHits.at(i);
                MDEEStripHit& SubHit2 = Hit.m_SubStripHits.at(j);
                
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
      for (MDEEStripHit& Hit: MergedStripHits) {
        Hit.m_Origins.clear();
        for (MDEEStripHit& SubHit: Hit.m_SubStripHits) {
          for (int& Origin: SubHit.m_Origins) {
            Hit.m_Origins.push_back(Origin);
          }
        }
        Hit.m_Origins.sort();
        Hit.m_Origins.unique();      
      }
      

      
      // Step (2): Calculate and noise timing
      const double TimingNoise = 3.76; //ns//I have been assuming 12.5 ns FWHM on the CTD... so the 1 sigma error on the timing value should be (12.5/2.35)/sqrt(2)
      for (MDEEStripHit& Hit: MergedStripHits) {
        
        //find lowest timing value 
        double LowestNoisedTiming = Hit.m_SubStripHits.front().m_Timing + m_Random.Gaus(0,TimingNoise);
        for(size_t i = 1; i < Hit.m_SubStripHits.size(); ++i){
          double Timing = Hit.m_SubStripHits.at(i).m_Timing + m_Random.Gaus(0,TimingNoise);
          //SubHit.m_Timing += m_Random.Gaus(0,TimingNoise);
          if( Timing < LowestNoisedTiming ) LowestNoisedTiming = Timing;
        }
        LowestNoisedTiming -= fmod(LowestNoisedTiming,5.0); //round down to nearest multiple of 5
        Hit.m_Timing = LowestNoisedTiming;
      }
      
      // Step (3): Calculate and noise ADC values including cross talk, charge loss, charge sharing, ADC overflow!
      
      // (3a) Add energy of all subhits to get energy of each striphit
      for (MDEEStripHit& Hit: MergedStripHits) { 
        double Energy = 0;
        double EnergyOrig = 0;
        for (MDEEStripHit SubHit: Hit.m_SubStripHits) {
          Energy += SubHit.m_Energy;
          EnergyOrig += SubHit.m_EnergyOrig;
        }
        
        //cout<<"Merged energy: "<<Energy<<endl;
        Hit.m_Energy = Energy;
        Hit.m_EnergyOrig = EnergyOrig;
      }
      

      // (3b) Charge loss
      list<MDEEStripHit>::iterator sh1, sh2;
      for (sh1 = MergedStripHits.begin(); sh1 != MergedStripHits.end(); ++sh1){
        for (sh2 = sh1; sh2 != MergedStripHits.end(); ++sh2){
          if (sh1 == sh2){ continue; }
          
          //check if strip hits are adjacent
          bool adjacent = false;
          int stripID1 = (*sh1).m_ROE.GetStripID();
          int stripID2 = (*sh2).m_ROE.GetStripID();
          int detID1 = (*sh1).m_ROE.GetDetectorID();
          int detID2 = (*sh2).m_ROE.GetDetectorID();
          bool side1 = (*sh1).m_ROE.IsLowVoltageStrip();
          bool side2 = (*sh2).m_ROE.IsLowVoltageStrip();
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

              //cout<<energy1<<":"<<energy2<<endl;
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
      list<MDEEStripHit>::iterator i = MergedStripHits.begin();
      int i2 = 0;
      while (i != MergedStripHits.end()) {
        int sdet = (*i).m_ROE.GetDetectorID();
        bool bside = (*i).m_ROE.IsLowVoltageStrip();
        int sside = 0;
        if (bside == true) {sside = 1;}
        int sstrip = (*i).m_ROE.GetStripID();
        double senergy = (*i).m_Energy;
        
        sim_arr[i2][0] = i2;
        sim_arr[i2][1] = sdet;
        sim_arr[i2][2] = sside;
        sim_arr[i2][3] = sstrip;
        sim_arr[i2][4] = senergy;
        
        //cout<<"Energy: "<<senergy<<endl;

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
      
      list<MDEEStripHit>::iterator l = MergedStripHits.begin();
      int l2 = 0;
      while (l != MergedStripHits.end()) {
        (*l).m_Energy = real_energies[l2];
        
        ++l;
        ++l2;
      }
      
      
      // (3d) Give each striphit an noised ADC value; handle ADC overflow
      list<MDEEStripHit>::iterator A = MergedStripHits.begin();
      while (A != MergedStripHits.end()) {
        double Energy = (*A).m_Energy;
        (*A).m_ADC = EnergyToADC((*A),Energy);
        if ((*A).m_ADC > 8029){  // number for McBride
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
      list<MDEEStripHit>::iterator j = MergedStripHits.begin();
      while (j != MergedStripHits.end()) {
        int det = (*j).m_ROE.GetDetectorID();
        int stripID = (*j).m_ROE.GetStripID();
        bool side_b = (*j).m_ROE.IsLowVoltageStrip();
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
      list<MDEEStripHit>::iterator k = MergedStripHits.begin();
      while (k != MergedStripHits.end()) {
        
        //so that we can use default value if necessary
        MReadOutElementDoubleStrip ROE_map_key = (*k).m_ROE;
        if (m_LLDThresholds.count((*k).m_ROE) == 0){
          ROE_map_key.SetDetectorID(12);
          ROE_map_key.SetStripID(0);
          ROE_map_key.IsLowVoltageStrip(0);
        }
        
        if ((*k).m_ADC < m_LLDThresholds[ROE_map_key]) {
          //cout<<(*k).m_ADC<<" < "<<m_LLDThresholds[ROE_map_key]<<endl;
          k = MergedStripHits.erase(k);
        } else {
          double prob = m_Random.Rndm();
          if (prob > m_FSTThresholds[ROE_map_key]->Eval((*k).m_ADC)){
            (*k).m_Timing = 0.0;
          }
          ++k;
        }
      }
      

      
      // (4c) Take care of guard ring vetoes
      list<MDEEStripHit>::iterator gr = GuardRingHits.begin();
      vector<int> grHit = vector<int>(nDets,0);
      while (gr != GuardRingHits.end()) {
        if ((*gr).m_Energy > m_GuardRingThresholds[(*gr).m_ROE]){
          int detID = (*gr).m_ROE.GetDetectorID();
          grHit[detID] = 1;
        }
        ++gr;
      }
      list<MDEEStripHit>::iterator grVeto = MergedStripHits.begin();
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
          if (evt_time > m_LastHitTimeByDet[det] + m_ASICDeadTime[det]){
            m_ASICDeadTime[det] = 1e-6;
            m_LastHitTimeByDet[det] = evt_time;
            m_TotalDeadTime[det] += m_ASICDeadTime[det];
          }
        }
      }
      
      
      // (4d) Make sure there is at least one strip left on each side of each detector
      //  If not, remove remaining strip(s) from detector because they won't trigger detector
      vector<int> xExists = vector<int>(nDets,0);
      vector<int> yExists = vector<int>(nDets,0);
      
      //look for (at least) one strip on each side
      list<MDEEStripHit>::iterator tr = MergedStripHits.begin();
      while (tr != MergedStripHits.end()) {
        int DetID = (*tr).m_ROE.GetDetectorID();
        if ((*tr).m_Timing != 0){
          if ((*tr).m_ROE.IsLowVoltageStrip()){ xExists[DetID] = 1; }
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
          if (evt_time > m_LastHitTimeByDet[det] + m_ASICDeadTime[det]){
            m_ASICDeadTime[det] = 1e-6;
            m_LastHitTimeByDet[det] = evt_time;
            m_TotalDeadTime[det] += m_ASICDeadTime[det];
          }
        }
      }
      
      // Step (5): Split into card cage events - i.e. split by detector
      /*    vector<vector<MDEEStripHit>> CardCagedStripHits;
       *    for (MDEEStripHit Hit: MergedStripHits) {
       *      bool Found = false;
       *      for (vector<MDEEStripHit>& V: CardCagedStripHits) {
       *        if (V[0].m_ROE.GetDetectorID() == Hit.m_ROE.GetDetectorID()) {
       *          V.push_back(Hit);
       *          Found = true;
  }
  }
  if (Found == false) {
    vector<MDEEStripHit> New;
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

//       if (MergedStripHits.size() == 0){
//        cout<<"Nothing left before 6.5"<<endl;
//      }
      
      
      //Step (6.5): Dead time
      //figure out which detectors are currently dead -- deadtime depends on how many channels were activated.
      vector<int> updateLastHitTime = vector<int>(nDets,0);
      vector<int> detIsDead = vector<int>(nDets,0);
      
      for (int d=0; d<nDets; d++){
        //second conditional for running multiple sim files when t starts at 0
        if (m_LastHitTimeByDet[d] + m_ASICDeadTime[d] > evt_time && m_LastHitTimeByDet[d]<evt_time){ detIsDead[d] = 1; }
      }
      
      //erase strip hits in dead detectors
      list<MDEEStripHit>::iterator DT = MergedStripHits.begin();
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
          m_ASICDeadTime[d] = 1e-6;
          m_TotalDeadTime[d] += m_ASICDeadTime[d];
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
       *			cout << "************" << endl;
       *			cout << "evt_time: " << evt_time << '\t' << "last time: " << m_LastHitTime << endl;
       *			cout << "Buffer values: " << endl;
       *			for (int i=0; i<16; i++){
       *				cout << m_DeadTimeBuffer[0][i] << '\t';
  }
  cout << endl;
  
  cout << "next empty slot: " << bufferFull[0] << endl;
  }
  */
      //erase strip hits in detectors when buffer is full
      list<MDEEStripHit>::iterator DH = MergedStripHits.begin();
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
      
      //update trigger rates
      set<int> detectorsHit;
      list<MDEEStripHit>::iterator TR = MergedStripHits.begin();
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
        /*
        // Clio's version
        map<int,double> initialEnergyByIA;
        map<int,double> finalEnergyByIA;
        map<int,vector<unsigned int> > HitIndexByIA;
        
        for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){
          MSimHT* Hit = SimEvent->GetHTAt(h);
          int initIA = Hit->GetSmallestOrigin();
          //again I have this problem if the IAs aren't in the sim
          if (initIA == 0){ initIA++; }
          MString IAprocess = SimEvent->GetIAById(initIA)->GetProcess();
          while (IAprocess != "INIT"){
            initIA = SimEvent->GetIAById(initIA)->GetOriginID();
            IAprocess = SimEvent->GetIAById(initIA)->GetProcess();
          }
          
          double initialEnergy = SimEvent->GetIAById(initIA)->GetSecondaryEnergy();
          double finalEnergy = 0.0;
          for (list<MDEEStripHit>::iterator p=MergedStripHits.begin(); p!=MergedStripHits.end(); ++p){
            if ((*p).m_ROE.IsLowVoltageStrip() == false && (*p).m_HitIndex == h){
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
            double prob = m_Random.Rndm();
            if (prob > threshold){
              eraseHit[i.first] = true;
            }
            else { eraseHit[i.first] = false; }
          }
        }
        //erase strip hits from IAs where probability was above the threshold
        for (auto i: eraseHit){
          if (i.second == true){
            list<MDEEStripHit>::iterator p = MergedStripHits.begin();
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
        */ 
        
        // Normally we just have the INIT's in the simulations, thus we have to simplify this:
        // If the total measured energy is in any of the INIT windows, test for erasing
        
        // Sum up all energies:
        double TotalMeasuredEnergy = 0.0;
        for (list<MDEEStripHit>::iterator p = MergedStripHits.begin(); p != MergedStripHits.end(); ++p){
          if ((*p).m_ROE.IsLowVoltageStrip() == false) {
            TotalMeasuredEnergy += (*p).m_EnergyOrig;
          }
        }
        
        // Now check every INIT if the energy is withing the window:
        for (unsigned int ia = 0; ia < SimEvent->GetNIAs(); ++ia) {
          
          double initialEnergy = SimEvent->GetIAAt(0)->GetSecondaryEnergy();
          
          // Clio's 
          double sigma = 8.35e-4*initialEnergy+1.69;
          double windowSize = 1.5*sigma;
          double threshold = 7.04e-5*initialEnergy+0.79;
          
          //cout<<"Measued: "<<TotalMeasuredEnergy<<": init: "<<initialEnergy<<endl;
          if (TotalMeasuredEnergy > initialEnergy-windowSize && TotalMeasuredEnergy < initialEnergy+windowSize) {
            //cout<<" In photo peak "<<endl;
            // In window, test for erasal
            double prob = m_Random.Rndm();
            if (prob > threshold){
              MergedStripHits.clear();
            }

            break;
          //} else {
            //cout<<"Not in photo peak"<<endl;
          }
        }

      } // End photo peak fudge factor
      
      //cout<<"Check is events left"<<endl;

      // Check if there are any strips left
      if (MergedStripHits.size() == 0){
        delete SimEvent;
        continue;
      }

      //cout<<"yes"<<endl;
      
      double finalEventEnergy = 0;
      int nNStripHits = 0;
      for (MDEEStripHit Hit: MergedStripHits){
        if (!Hit.m_ROE.IsLowVoltageStrip()){
          finalEventEnergy += Hit.m_Energy;
          nNStripHits++;
        }
      }
      if (finalEventEnergy > eventInitialEnergy+100){
        cout << eventInitialEnergy << '\t' << finalEventEnergy << endl;
        cout << "SIM HITS: " << endl;
        for (unsigned int h=0; h<SimEvent->GetNHTs(); h++){
          cout << SimEvent->GetHTAt(h)->GetEnergy() << endl;
        }
        cout << "DEE STRIP HITS: " << endl;
        for (MDEEStripHit Hit: MergedStripHits){
          if (!Hit.m_ROE.IsLowVoltageStrip()){
            cout << Hit.m_Energy << endl;
          }
        }
        cout << endl << endl;
      }
      
      
      // (1) Move the information to the read-out-assembly
      Event->SetID(SimEvent->GetID());
      Event->SetTimeUTC(SimEvent->GetTime());
      
      for (unsigned int i = 0; i < IAs.size(); ++i) {
        Event->AddSimIA(*IAs[i]);
      }
      for (MDEEStripHit Hit: MergedStripHits){
        MStripHit* SH = new MStripHit();
        SH->SetDetectorID(Hit.m_ROE.GetDetectorID());
        SH->SetStripID(Hit.m_ROE.GetStripID());
        SH->IsXStrip(Hit.m_ROE.IsLowVoltageStrip());
        SH->SetADCUnits(Hit.m_ADC);
        SH->SetTiming(Hit.m_Timing);
        SH->SetPreampTemp(20);
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
        for (MDEEStripHit Hit: MergedStripHits){
          m_Roa<<"UH "<<Hit.m_ROE.GetDetectorID()<<" "<<Hit.m_ROE.GetStripID()<<" "<<(Hit.m_ROE.IsLowVoltageStrip() ? "p" : "n")<<" "<<Hit.m_ADC<<" "<<Hit.m_Timing<<" "<<Hit.m_PreampTemp;
          
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
bool MDetectorEffectsEngineSMEX::Finalize()
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
  cout << "Total shield dead time for group 1: " << m_TotalShieldDeadTime1 << endl;
  cout << "Total shield dead time for group 2: " << m_TotalShieldDeadTime2 << endl;
  cout << "Total shield dead time for group 3: " << m_TotalShieldDeadTime3 << endl;
  // cout << "Total shield dead time for all shields:" << m_TotalShieldDeadTime << endl;
  
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


// 
////////////////////////////////////////////////////////////////////////////////


//! Convert energy to ADC value by reversing energy calibration done in 
//! MModuleEnergyCalibrationUniversal.cxx
int MDetectorEffectsEngineSMEX::EnergyToADC(MDEEStripHit& Hit, double mean_energy)
{  
  //first, need to simulate energy spread
  //static TRandom3 r(0);
  TF1* FitRes = m_ResolutionCalibration[Hit.m_ROE];
  //resolution is a function of energy
  double EnergyResolutionFWHM = 3; //default to 3keV...does this make sense?
  if (FitRes != 0){
    EnergyResolutionFWHM = FitRes->Eval(mean_energy);
    //cout<<"Energy Res: "<<EnergyResolutionFWHM<<" (FWHM) at "<<mean_energy<<endl;
  }
  
  //get energy from gaussian around mean_energy with sigma=EnergyResolution
  //TRandom3 r(0);
   double energy = m_Random.Gaus(mean_energy,EnergyResolutionFWHM/2.35);
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
    // find roots - while considering the limits of the fit function
    double MaxEnergy = 10000.0;
    if (energy >= MaxEnergy || energy > Fit->GetMaximum(0, 8191)) {
      //cout<<"Info: Setting AD units to max (8191): E="<<energy<<" vs. E_max="<<Fit->GetMaximum(0, 8191)<<endl;
      ADC_double = 8191; 
    } else if (energy <= 0 || energy < Fit->GetMinimum(0, 8191)) {
      //cout<<"Info: Setting AD units to min (0): E="<<energy<<" vs. E_min"<<Fit->GetMinimum(0, 8191)<<endl;
      ADC_double = 0.0;
    } else {
      ADC_double = Fit->GetX(energy, 0., MaxEnergy);
      //cout<<"energy:"<<energy<<"(ad: "<<ADC_double<<") vs. E_min="<<Fit->GetMinimum()<<"   E_max="<<Fit->GetMaximum()<<endl;
    }
  }
  
  
  int ADC = int(ADC_double);
  return ADC;
}


////////////////////////////////////////////////////////////////////////////////


//! Noise shield energy with measured resolution
double MDetectorEffectsEngineSMEX::NoiseShieldEnergy(double energy, MString shield_name)
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
  
  double noised_energy = m_Random.Gaus(energy,sigma);
  
  return noised_energy;
  
}

////////////////////////////////////////////////////////////////////////////////

//! Calculate new summed energy of two strips affected by charge loss
vector<double> MDetectorEffectsEngineSMEX::ApplyChargeLoss(double energy1, double energy2, int detID, int side, double depth1, double depth2) 
{  
  vector<double> retEnergy;
  if (energy1 == 0 && energy2 == 0) {
    retEnergy.push_back(0);
    retEnergy.push_back(0);
    return retEnergy;
  }

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
  // vector<double> retEnergy;
  
  retEnergy.push_back(newE1);
  retEnergy.push_back(newE2);
  
  return retEnergy;
  
}

////////////////////////////////////////////////////////////////////////////////

//! Calculate new summed energy of two strips affected by charge loss
bool MDetectorEffectsEngineSMEX::InitializeChargeLoss()
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
bool MDetectorEffectsEngineSMEX::ParseChargeSharingFile()
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
bool MDetectorEffectsEngineSMEX::ParseCrosstalkFile()
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

//! Read in guard ring thresholds
bool MDetectorEffectsEngineSMEX::ParseGuardRingThresholdFile()
{
  
  MParser Parser;
  if (Parser.Open(m_GuardRingThresholdFileName, MFile::c_Read) == false) {
    cout << "Unable to open guard ring threshold file " << m_GuardRingThresholdFileName << endl;
    return false;
  }
  
  for (unsigned int i=2; i<26; i++){
    int detector = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(0);
    int side = Parser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
    double threshold = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(2);
    
    MReadOutElementDoubleStrip R;
    R.SetDetectorID(detector);
    R.SetStripID(38);
    R.IsLowVoltageStrip(side);
    
    m_GuardRingThresholds[R] = threshold;
  }
  
  return true;
  
}

////////////////////////////////////////////////////////////////////////////////

//! Read in thresholds
bool MDetectorEffectsEngineSMEX::ParseThresholdFile()
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
    R.IsLowVoltageStrip(isPos);
    
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
  R.IsLowVoltageStrip(0);
  
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
bool MDetectorEffectsEngineSMEX::ParseEnergyCalibrationFile()
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
        R.IsLowVoltageStrip(Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "p");
        
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
bool MDetectorEffectsEngineSMEX::ParseDeadStripFile()
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
    int strip = lineVec.at(2)-1; //in file, strips go from 1-65; in m_DeadStrips they go from 0-63
    lineVec.clear();
    
    //any dead strips have their value in m_DeadStrips set to 1 
    m_DeadStrips[det][side][strip] = 1;
  }
  
  return true;
}


void MDetectorEffectsEngineSMEX::dummy_func(){
  //empty function to make break points for debugger
}

// MDummy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

