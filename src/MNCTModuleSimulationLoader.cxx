/*
* MNCTModuleSimulationLoader.cxx
*
*
* Copyright (C) by Jau-Shian Liang, Andreas Zoglauer.
* All rights reserved.
*
*
* This code implementation is the intellectual property of
* Jau-Shian Liang, Andreas Zoglauer.
*
* By copying, distributing or modifying the Program (or any work
* based on the Program) you indicate your acceptance of this statement,
* and all its terms.
*
*/


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModuleSimulationLoader
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleSimulationLoader.h"

// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MVector.h"
#include "MNCTModule.h"
#include "MDStrip3D.h"
#include "MNCTStripHit.h"
#include "MNCTArray.h"
#include "MNCTMath.h"
#include "MGUIOptionsSimulationLoader.h"
#include "MNCTInverseCrosstalkCorrection.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MSimGR.h"
#include "MDVolumeSequence.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleSimulationLoader)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleSimulationLoader::MNCTModuleSimulationLoader() : MNCTModule(), MFileEventsSim()
{
  // Construct an instance of MNCTModuleSimulationLoaderi
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Simulation loader and detector effects engine";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DetectorEffectsEngineForSimulatedData";
  
  
  // Set all types this modules handles
  AddModuleType(c_EventLoader);
  AddModuleType(c_EventLoaderMeasurement);
  AddModuleType(c_DetectorEffectsEngine);
  
  // Set all modules, which can follow this module
  AddSucceedingModuleType(c_EnergyCalibration);
  AddSucceedingModuleType(c_CrosstalkCorrection);
  AddSucceedingModuleType(c_ChargeSharingCorrection);
  AddSucceedingModuleType(c_DepthCorrection);
  AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_Aspect);
  AddSucceedingModuleType(c_Else);
  AddSucceedingModuleType(c_EventReconstruction);
  AddSucceedingModuleType(c_EventSaver);
  
  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleSimulationLoader::~MNCTModuleSimulationLoader()
{
  // Delete this instance of MNCTModuleSimulationLoader
  
  // We are not deleting the sim event!
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleSimulationLoader::Initialize()
{
  // Initialize the module
  cout << "\n\nInitializing MNCTModuleSimulationLoader..."<<endl;
  
  
  // Step 1: Load the simulation file
  MFileEventsSim::SetGeometry(m_Geometry);
  if (Open(m_FileName, c_Read) == false) return false;
  
  
  // Step 2: Initialize the detector effects engine:
  
  // Set up counters
  m_NEvent = 0;
  m_NAnalyzed = 0;
  m_NTriggered = 0;
  m_NSingleDet = 0;
  m_NMultipleDet = 0;
  m_NGuardring = 0;
  
  // Set up the detectors
  m_NCTDetectors.SetGeometry(m_Geometry);
  m_NCTDetectors.SetLoadDeadStrip(m_LoadDeadStrip);
  m_NCTDetectors.SetLoadCoincidence(m_LoadCoincidence);
  m_NCTDetectors.SetLoadAntiCoincidence(m_LoadAntiCoincidence);
  
  m_NCTDetectors.SetDetectorFile("$(NUCLEARIZER)/resource/detectorparameters/DetectorList_2014.dat");  
  if (m_LoadDeadStrip == true) {
    m_NCTDetectors.SetDeadStripFile("$(NUCLEARIZER)/resource/detectorparameters/DeadStripList_2014.dat");
  }
  if (m_LoadCoincidence == true) {
    m_NCTDetectors.SetCoincidenceFile("$(NUCLEARIZER)/resource/detectorparameters/CoincidenceVolumeList_2009.dat");
  }
  if (m_LoadAntiCoincidence == true) {      
    m_NCTDetectors.SetAntiCoincidenceFile("$(NUCLEARIZER)/resource/detectorparameters/AntiCoincidenceVolumeList_2009.dat");
  }
  m_NCTDetectors.Activate();
  
  // Load response files
  m_NCTResponse.SetNCTDetectorArray(&m_NCTDetectors);
  if (m_NCTResponse.Activate() == false) {
    merr << "NCTDetector is not set!!!"<<endl;
    return false;
  }
  
  // Initialize inverse cross-talk correction
  m_InverseCrosstalk.Initialize();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleSimulationLoader::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.
  
  MSimEvent* SimEvent = GetNextEvent();
  if (SimEvent == 0) {
    cout<<"MNCTModuleMeasurementLoaderROA: No more events!"<<endl;
    return false;
  }
  
  // Basics:
  Event->SetID(SimEvent->GetEventNumber());
  Event->SetTime(SimEvent->GetTime());
  
  mimp<<"Missing: Set event time, orientation, etc."<<show;
  
  // Strip hits:
  for (unsigned int h = 0; h < SimEvent->GetNHTs(); ++h) {
    MNCTHit* Hit = new MNCTHit(); // deleted on destruction of event
    Hit->SetEnergy(SimEvent->GetHTAt(h)->GetEnergy());
    Hit->SetPosition(SimEvent->GetHTAt(h)->GetPosition());
    
    mimp<<"Adding dummy position resolution"<<show;
    Hit->SetEnergyResolution(2.0);
    Hit->SetPositionResolution(MVector(0.2/sqrt(12.0), 0.2/sqrt(12.0), 0.1));
    
    Event->AddHit(Hit);
  }
  
  // Guardring hits
  for (unsigned int g = 0; g < SimEvent->GetNGRs(); ++g) {
    MNCTGuardringHit* GuardringHit = new MNCTGuardringHit(); // deleted on destruction of event
    GuardringHit->SetADCUnits(SimEvent->GetGRAt(g)->GetEnergy());
    GuardringHit->SetPosition(SimEvent->GetGRAt(g)->GetPosition());
    GuardringHit->SetDetectorID(99);
    
    Event->AddGuardringHit(GuardringHit);
  } 
  Event->SetDataRead();  
  
  
  // Don't forget to clean up
  delete SimEvent;
  
  
  // Finally apply the detector effects
  ApplyDetectorEffects(Event); 
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleSimulationLoader::ApplyDetectorEffects(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level 
  
  //#1: Load events
  mimp<<"MNCTModuleSimulationLoader::"<<show;
  
  Event->SetTrigger(false);//trigger flag will be updated below
  Event->SetTime(Event->GetTime()+m_TimeOffset0*100000.0+m_TimeOffset);//update time by adding time offset
  Event->SetTI((unsigned int) Event->GetTime().GetAsSeconds());//to pretend to be flight data
  //Event->SetCL((unsigned long)(fmod(Event->GetTime(),429.4)*10000000));//just a dummy!!
  m_NEvent++;
  
  if(Event->GetNHits()>0){
    if(m_Verbose)cout << "\nEvent:" << Event->GetID() << endl;
    if(m_Verbose)cout << "DetectorEffectsEngine: Analyzing..." << endl;
    m_NAnalyzed++;
  }else{
    return true;
  }
  
  //#2: Sort hits by volumes
  //create a volume sequence for each hit
  vector<MDVolumeSequence> VSeqs;
  for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
    VSeqs.push_back(m_Geometry->GetVolumeSequence(Event->GetHit(h)->GetPosition()));
    //cout << "Hit:Energy=" << Event->GetHit(h)->GetEnergy() << '\n';//debug
  }
  
  // Go through the hits, sort them detector by detector:
  vector<vector<MNCTHit*> > DetectorHits; // References won't work
  vector<vector<MNCTHit*> > CoinHits;
  vector<vector<MDVolumeSequence> > DetectorHitsVSeqs;
  vector<vector<MDVolumeSequence> > CoinHitsVSeqs;
  
  for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
    bool AlreadyAdded = false;
    //sort hits which are in coincidence or anti-coincidence volumes
    if(m_NCTDetectors.IsCoincidence(VSeqs[h].GetDeepestVolume())||
      m_NCTDetectors.IsAntiCoincidence(VSeqs[h].GetDeepestVolume())){
      for (unsigned int c = 0; c < CoinHits.size(); ++c) {
        massert(CoinHits[c].size() > 0); // If it's empty we made a programming error
        if (CoinHitsVSeqs[c][0].HasSameDetector(VSeqs[h])) {
          CoinHits[c].push_back(Event->GetHit(h));
          CoinHitsVSeqs[c].push_back(VSeqs[h]);
          AlreadyAdded = true;
          break;
        }
      }
      if (AlreadyAdded == false) {
        vector<MNCTHit*> v;
        v.push_back(Event->GetHit(h));
        CoinHits.push_back(v);
        vector<MDVolumeSequence> S;
        S.push_back(VSeqs[h]);
        CoinHitsVSeqs.push_back(S);
      }
      continue;
      }
      
      //if not in sensitive volumes (coincidence, anti-coincidence, detector), drop it!!
      if(VSeqs[h].GetDetector() == 0 || m_NCTDetectors.IsNCTDetector(VSeqs[h].GetDetector()->GetName()) == false) {
        if (m_Verbose) cout << "Hit: Not in detector volume!!!"<<endl;
        continue;
      }
      //sort hits which are in detector volume
      for (unsigned int d = 0; d < DetectorHits.size(); ++d) {
        massert(DetectorHits[d].size() > 0); // If it's empty we made a programming error
        if (DetectorHitsVSeqs[d][0].HasSameDetector(VSeqs[h])) {
          DetectorHits[d].push_back(Event->GetHit(h));
          DetectorHitsVSeqs[d].push_back(VSeqs[h]);
          AlreadyAdded = true;
          break;
        }
      }
      if (AlreadyAdded == false) {
        vector<MNCTHit*> v;
        v.push_back(Event->GetHit(h));
        DetectorHits.push_back(v);
        vector<MDVolumeSequence> S;
        S.push_back(VSeqs[h]);
        DetectorHitsVSeqs.push_back(S);
      }
  }
  
  //#3: Check coincidence and anti-coincidence
  bool CoTrigger=false;
  bool AntiTrigger=false;
  
  for(unsigned int c = 0; c < CoinHits.size(); ++c){
    double CoinE=0;
    MDVolume *DetVol = m_Geometry->GetVolume(CoinHits[c][0]->GetPosition());
    for(unsigned int h = 0; h < CoinHits[c].size(); ++h){
      CoinE+=CoinHits[c][h]->GetEnergy();
    }
    if(m_NCTDetectors.IsCoincidence(DetVol)){
      MNCTCoincidenceVolume *CoinVol = m_NCTDetectors.GetCoincidenceVolume(DetVol);
      if(CoinVol->IsTriggered(CoinE))CoTrigger=true;
    }else{
      MNCTCoincidenceVolume *AntiVol = m_NCTDetectors.GetAntiCoincidenceVolume(DetVol);
      if(AntiVol->IsTriggered(CoinE)){
        AntiTrigger=true;
        if(m_Verbose)cout << "Anti-coincidence triggered!!!"<<endl;
        if(m_Verbose)cout << "DetectorEffectsEngine: End of Analysis..."<<endl;
        return true;
      }
    }
  }
  
  if(m_LoadCoincidence){
    if(CoTrigger==false){
      if(m_Verbose)cout << "No coincidence volume triggered!!!"<<endl;
      if(m_Verbose)cout << "DetectorEffectsEngine: End of Analysis..."<<endl;
      return true;
    }else{
      if(m_Verbose)cout << "Coincidence triggered!!!"<<endl;
    }
  }
  
  //#4: Settings for main process
  const bool XSTRIP=true;
  const bool YSTRIP=false;
  int DetectorID;
  bool gring = false;
  bool Found = false;
  int MultiTrigger = 0;
  MNCTHitInVoxel HitVox;
  vector <vector <MNCTStripEnergyDepth> > StripEDs;
  vector<MNCTGuardringHit*> GHit;
  //vector<MNCTStripHit*> StripHits;
  
  //#5: Main process (detector by detector)
  for (unsigned int d = 0; d < DetectorHits.size(); ++d) {
    
    //#5-1: Determine detector ID
    // position -> volume -> ID
    MDVolumeSequence VS = m_Geometry->GetVolumeSequence(DetectorHits[d][0]->GetPosition());
    if (VS.GetDetector() == 0) {
      mout<<"Hit without corresponding detector... ignoring it..."<<endl;
      continue;
    }
    
    DetectorID = m_NCTDetectors.GetID(VS.GetDetector()->GetName());
    
    if (m_NCTDetectors.IsDetectorON(DetectorID) == false) continue;
    
    if(m_Verbose)cout << " DetectorID: " <<DetectorID << '\n';
    bool dettrigger=false;//detector trigger flag
    int count1=0,count2=0;
    
    //clean up containers
    //HitVox.Clear;
    StripEDs.clear();
    GHit.clear();
    
    //#5-2: Hit->GuargringHit/StripEnergyDepth, charge sharing
    for (unsigned int h = 0; h < DetectorHits[d].size(); ++h) {
      //MDStrip3D* D = dynamic_cast<MDStrip3D*>(DetectorHitsVSeqs[d][h].GetDetector());
      
      /*Determine strips number or gaurdring*/
      //checking guardring
      gring = m_NCTDetectors.IsGuardring(DetectorHitsVSeqs[d][h].GetPositionInDetector());//<--need to check
      
      if(gring){
        MNCTGuardringHit* NewGHit = new MNCTGuardringHit;
        NewGHit->SetDetectorID(DetectorID);
        NewGHit->SetADCUnits(DetectorHits[d][h]->GetEnergy());
        NewGHit->SetPosition(DetectorHits[d][h]->GetPosition());
        GHit.push_back(NewGHit);
        continue;
      }
      
      //cout << "energy=" << DetectorHits[d][h]->GetEnergy() << ""<<endl;//debug
      //      cout << "  Energy sharing..."<<endl;
      //derive strips number and position in voxel
      //D->DetermineStrips(DetectorHitsVSeqs[d][h].GetPositionInDetector(), xStrip, yStrip);//
      HitVox = m_NCTDetectors.PositionInVolume2Voxel(DetectorID,
                                                    DetectorHitsVSeqs[d][h].GetPositionInDetector(),
                                                    DetectorHits[d][h]->GetEnergy());//<--###check orientation of detector
      
      /*Sharing*/
      //return (strip, energy, depth) triplets
      //sharing of normal strip and guardring is not considered yet
      
      //check edge, no sharing at edge
      if(HitVox.GetDetectorID()==-1)continue;//skip hit which is not in sensitive region
      vector<MNCTStripEnergyDepth> xSEDs,ySEDs;
      
      if(m_RunEnergySharing == false ||
        m_NCTDetectors.IsNearGuardring(DetectorHitsVSeqs[d][h].GetPositionInDetector())){
        xSEDs = m_NCTResponse.noEnergySharing(HitVox,XSTRIP);
      ySEDs = m_NCTResponse.noEnergySharing(HitVox,YSTRIP);
      count2++;
        }else{ 
          xSEDs = m_NCTResponse.EnergySharing(HitVox,XSTRIP);//XStrip side sharing
          ySEDs = m_NCTResponse.EnergySharing(HitVox,YSTRIP);//YStrip side sharing
          count1++;
        }
        xSEDs.insert(xSEDs.end(),ySEDs.begin(),ySEDs.end());//push ySEDs to xSEDs' back
        
        
        /*Sort (strip, energy, timing) by strips*/
        // Check if such a strip already exists. If yes, add the energy:
        //cout << xSEDs.size() << '\n';//debug
        for (unsigned int i=0; i<xSEDs.size(); i++)//<--the size should be exactly equal to 2 or 4
        {
          Found = false;
          for (unsigned int s = 0; s < StripEDs.size(); ++s) {
            if (xSEDs[i].GetStrip()==StripEDs[s][0].GetStrip()){
              StripEDs[s].push_back(xSEDs[i]);
              Found = true;
              break;
            }
          }
          if (Found == false) {
            vector<MNCTStripEnergyDepth> NewSED;
            NewSED.push_back(xSEDs[i]);
            StripEDs.push_back(NewSED);
          }
        }
        
    }
    
    if(m_Verbose)cout << "  Run charge sharing hits: " << count1 << endl;
    if(m_Verbose)cout << "  Not run charge sharing hits: " << count2 << endl;
    
    //#5-3: Guardring Veto
    //if guardring triggered, the hits on the same detector but not else detector are vetoed
    double Eg=0;
    for(unsigned int i=0;i<GHit.size();i++)Eg+=GHit[i]->GetADCUnits();
    if(m_NCTResponse.GuardringTriger(Eg)){
      //Event->SetVeto(); //<-not sure yet
      MNCTGuardringHit* gr_temp = new MNCTGuardringHit;
      gr_temp->SetDetectorID(DetectorID);
      gr_temp->SetADCUnits(Eg);
      Event->AddGuardringHit(gr_temp);
      if(m_Verbose)cout << "Guardring triggered!!!"<<endl;
      continue;
    }
    
    //#5-4: Crosstalk effect
    vector <MNCTStripHit*> newstriphitsX;//pos-side StripHit
    vector <MNCTStripHit*> newstriphitsY;//neg-side StripHit
    vector <unsigned int> stripIndex;//used for recording index. must be careful!!
    for(unsigned int s=0;s<StripEDs.size();s++){
      MNCTStrip sStrip=StripEDs[s][0].GetStrip();
      double E=0,Et=0;
      //create strip hits (must delete it if not triggered)
      MNCTStripHit* newstriphit = new MNCTStripHit;
      //cout << StripEDs[s].size() << ';' << Et << ';';//debug
      for(unsigned int i=0;i<StripEDs[s].size();i++){
        E=StripEDs[s][i].GetEnergy();
        Et+=E;
      }
      newstriphit->SetDetectorID(DetectorID);
      newstriphit->IsXStrip(sStrip.IsXStrip());
      newstriphit->SetStripID(sStrip.GetStripID()+m_NCTDetectors.GetFirstStripID());
      newstriphit->SetEnergy(Et);
      
      if(sStrip.IsXStrip())
      {
        newstriphitsX.push_back(newstriphit);
        stripIndex.push_back(newstriphitsX.size()-1);
      }
      else 
      {
        newstriphitsY.push_back(newstriphit);
        stripIndex.push_back(newstriphitsY.size()-1);
      }
    }
    //run crosstalk
    if(m_Verbose && m_RunCrosstalk)cout << "  Run Crosstalk..." << endl;
    if(m_RunCrosstalk && newstriphitsX.size()>1) m_InverseCrosstalk.ApplyCrosstalk(newstriphitsX,DetectorID,1);
    if(m_RunCrosstalk && newstriphitsY.size()>1) m_InverseCrosstalk.ApplyCrosstalk(newstriphitsY,DetectorID,0);
    
    //#5-5: Triggering, determine depth, Depth->timing, Energy->ADC
    if(m_Verbose)cout << "  Trigering..."<<endl;
    for(unsigned int s=0;s<StripEDs.size();s++){
      MNCTStrip sStrip=StripEDs[s][0].GetStrip();
      double fast_threshold = m_NCTResponse.FastThreshold(sStrip);
      double slow_threshold = m_NCTResponse.SlowThreshold(sStrip);
      bool FLD=false,LLD=false;
      int ADC=0;
      double StripTiming=0;
      if(fast_threshold<35.)fast_threshold = 35.;//only for 2005's data?
      
      //get the crosstalked energy back and add noise
      double Et=0;
      MNCTStripHit* newstriphit;
      if(sStrip.IsXStrip())newstriphit=newstriphitsX[stripIndex[s]];
      else newstriphit=newstriphitsY[stripIndex[s]];
      Et=newstriphit->GetEnergy();//get the crosstalked energy back
      
      if(!m_NonlinearGain) Et+=m_NCTResponse.EnergyNoise(Et,sStrip);//add noise
      else Et+=m_NCTResponse.NonlinearEnergyNoise(Et,sStrip);//add noise
      
      /*FLD*/
      if(Et>fast_threshold && !m_NCTDetectors.IsDeadStrip(sStrip)){ //check dead strip
        FLD=true;
        //derive the strip timing;
        for(unsigned int i=0;i<StripEDs[s].size();i++){
          StripTiming+= StripEDs[s][i].GetEnergy()/Et * StripEDs[s][i].GetDepth();
        }
        StripTiming = MNCTMath::discretize(StripTiming,10,0);
        
        if(m_Verbose) cout << "    FLD "
          << DetectorID << ' ' << sStrip.IsXStrip() << ' ' << sStrip.GetStripID()+m_NCTDetectors.GetFirstStripID() << ' '
          << StripTiming << '\n';
      }else{
        StripTiming = -9999;//for LLD_only Event
      }
      
      /*LLD*/
      if(Et>slow_threshold && !m_NCTDetectors.IsDeadStrip(sStrip)){ //check dead strip
        LLD=true;
        
        if(!m_NonlinearGain) ADC=m_NCTResponse.Energy2ADC(Et,sStrip);
        else ADC=m_NCTResponse.NonlinearEnergy2ADC(Et,sStrip);
        //ADC=Et/0.22;
        
        if(m_Verbose) cout << "    LLD "
          << DetectorID << ' ' << sStrip.IsXStrip() << ' ' << sStrip.GetStripID()+m_NCTDetectors.GetFirstStripID() << ' '
          << Et << ' ' << ADC << '\n';
      }
      if((FLD || m_KeepLLDOnly) && LLD){// AND or OR? <--not sure yet
        newstriphit->SetTiming(StripTiming);
        newstriphit->SetADCUnits(ADC);
        newstriphit->SetEnergy(0.);//reset
        Event->AddStripHit(newstriphit);
      } else {
        delete newstriphit;//must delete untriggered StripHit
      }
      
      if(FLD && LLD){//AND or OR? (2009: AND)
        Event->SetTrigger(true);
        dettrigger=true;
      }
      
    }//End of triggering
    
    if(dettrigger)MultiTrigger++;
  }//End of main process
  
  //#6: Postprosecc
  //#6-1: Statistics
  if(Event->GetTrigger()){
    m_NTriggered++;
    
    if(MultiTrigger>1)m_NMultipleDet++;
    else m_NSingleDet++;
    
    if(Event->GetNGuardringHits())m_NGuardring++;
  }
  //#6-1: Move simulation hits
  //prevent programing bug of strip paring module
  Event->MoveHitsToSim();
  
  if(m_Verbose)cout << "DetectorEffectsEngine: End of Analysis..."<<endl;
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MNCTModuleSimulationLoader::Report()
{
  MString string_tmp;
  char s[32];
  
  string_tmp += "  Total input events: ";
  string_tmp += m_NEvent;
  string_tmp += "\n";
  
  string_tmp += "  No hits events: ";
  string_tmp += (m_NEvent - m_NAnalyzed);
  sprintf(s, " (%.2lf%%)\n", (m_NEvent-m_NAnalyzed)/(double)m_NEvent*100.0);
  string_tmp += s;
  
  string_tmp += "  Analyzed events: ";
  string_tmp += m_NAnalyzed;
  sprintf(s, " (%.2lf%%)\n", m_NAnalyzed/(double)m_NEvent*100.0);
  string_tmp += s;
  
  string_tmp += "  Triggered events: ";
  string_tmp += m_NTriggered;
  sprintf(s, " (%.2lf%%)\n", m_NTriggered/(double)m_NEvent*100.0);
  string_tmp += s;
  
  string_tmp += "    Single detector events: ";
  string_tmp += m_NSingleDet;
  sprintf(s, " (%.2lf%%)\n", m_NSingleDet/(double)m_NTriggered*100.0);
  string_tmp += s;
  
  string_tmp += "    Multiple detector events: ";
  string_tmp += m_NMultipleDet;
  sprintf(s, " (%.2lf%%)\n", m_NMultipleDet/(double)m_NTriggered*100.0);
  string_tmp += s;
  
  string_tmp += "    Has guardring events: ";
  string_tmp += m_NGuardring;
  sprintf(s, " (%.2lf%%)\n", m_NGuardring/(double)m_NTriggered*100.0);
  string_tmp += s;
  
  return string_tmp;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleSimulationLoader::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
  
  MGUIOptionsSimulationLoader* Options = new MGUIOptionsSimulationLoader(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleSimulationLoader::ReadXmlConfiguration(MXmlNode* Node)
{
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
  
  MXmlNode* TimeOffset0Node = Node->GetNode("TimeOffset0");
  MXmlNode* TimeOffsetNode = Node->GetNode("TimeOffset");
  MXmlNode* LoadDeadStripNode = Node->GetNode("LoadDeadStrip");
  MXmlNode* LoadCoincidenceNode = Node->GetNode("LoadCoincidence");
  MXmlNode* LoadAntiCoincidenceNode = Node->GetNode("LoadAntiCoincidence");
  MXmlNode* RunChargeSharingNode = Node->GetNode("RunChargeSharing");
  MXmlNode* RunCrosstalkNode = Node->GetNode("RunCrosstalk");
  MXmlNode* NonlinearGainNode = Node->GetNode("NonlinearGain");
  MXmlNode* KeepLLDOnlyNode = Node->GetNode("KeepLLDOnly");
  MXmlNode* VerboseNode = Node->GetNode("Verbose");
  
  if (TimeOffset0Node !=0) m_TimeOffset0 = TimeOffset0Node->GetValueAsInt();
  else m_TimeOffset0 = 0;
  
  if (TimeOffsetNode !=0) m_TimeOffset = TimeOffsetNode->GetValueAsDouble();
  else m_TimeOffset = 0;
  
  if (LoadDeadStripNode !=0) m_LoadDeadStrip = LoadDeadStripNode->GetValueAsBoolean();
  else m_LoadDeadStrip = true;
  
  if (LoadCoincidenceNode !=0) m_LoadCoincidence = LoadCoincidenceNode->GetValueAsBoolean();
  else m_LoadCoincidence = false;
  
  if (LoadAntiCoincidenceNode !=0) m_LoadAntiCoincidence = LoadAntiCoincidenceNode->GetValueAsBoolean();
  else m_LoadAntiCoincidence= false;
  
  if (RunChargeSharingNode !=0) m_RunEnergySharing = RunChargeSharingNode->GetValueAsBoolean();
  else m_RunEnergySharing = true;
  
  if (RunCrosstalkNode !=0) m_RunCrosstalk = RunCrosstalkNode->GetValueAsBoolean();
  else m_RunCrosstalk = true;
  
  if (NonlinearGainNode !=0) m_NonlinearGain = NonlinearGainNode->GetValueAsBoolean();
  else m_NonlinearGain = true;
  
  if (KeepLLDOnlyNode !=0) m_KeepLLDOnly = KeepLLDOnlyNode->GetValueAsBoolean();
  else m_KeepLLDOnly = false;
  
  if (VerboseNode !=0) m_Verbose = VerboseNode->GetValueAsBoolean();
  else m_Verbose = false;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleSimulationLoader::CreateXmlConfiguration()
{
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  new MXmlNode(Node, "FileName", m_FileName);
  new MXmlNode(Node, "TimeOffset0", m_TimeOffset0);
  new MXmlNode(Node, "TimeOffset", m_TimeOffset);
  new MXmlNode(Node, "LoadDeadStrip", m_LoadDeadStrip);
  new MXmlNode(Node, "LoadCoincidence", m_LoadCoincidence);
  new MXmlNode(Node, "LoadAntiCoincidence", m_LoadAntiCoincidence);
  new MXmlNode(Node, "RunChargeSharing", m_RunEnergySharing);
  new MXmlNode(Node, "RunCrosstalk", m_RunCrosstalk);
  new MXmlNode(Node, "NonlinearGain", m_NonlinearGain);
  new MXmlNode(Node, "KeepLLDOnly", m_KeepLLDOnly);
  new MXmlNode(Node, "Verbose", m_Verbose);
  
  return Node;
}


// MNCTModuleSimulationLoader.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
