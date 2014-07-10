/* 
 * MNCTDetectorEffectsEngineCOSI.cxx
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

// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <map>
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
#include "MFileEventsSim.h"
#include "MDVolumeSequence.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MReadOutElementDoubleStrip.h"

/******************************************************************************/


/******************************************************************************/

class MNCTDetectorEffectsEngineCOSI
{
public:
  //! Default constructor
  MNCTDetectorEffectsEngineCOSI();
  //! Default destructor
  ~MNCTDetectorEffectsEngineCOSI();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze whatever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;

  //! Simulation file name
  MString m_FileName;
  //! Geometry file name
  MString m_GeometryFileName;
  
  //! Tiny helper class for MNCTDetectorEffectsEngineCOSI describing a special strip hit
  class MNCTDEEStripHit
  {
  public:
    //! The read-out element
    MReadOutElementDoubleStrip m_ROE;
    //! The ADC value
    double m_ADC;
    //! The timing value;
    double m_Timing;
  
    //! The simulated position
    MVector m_Position;
    //! The simulated energy deposit
    double m_Energy;
  
    //! True if this is a guard ring
    bool m_IsGuardRing;
    
    //! A list of original strip hits making up this strip hit
    vector<MNCTDEEStripHit> m_SubStripHits;
  };

  
};

/******************************************************************************/


/******************************************************************************
 * Default constructor
 */
MNCTDetectorEffectsEngineCOSI::MNCTDetectorEffectsEngineCOSI() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


/******************************************************************************
 * Default destructor
 */
MNCTDetectorEffectsEngineCOSI::~MNCTDetectorEffectsEngineCOSI()
{
  // Intentionally left blank
}


/******************************************************************************
 * Parse the command line
 */
bool MNCTDetectorEffectsEngineCOSI::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<"cosidee - The COSI detector effects engine"<<endl;
  Usage<<endl;
  Usage<<"  Usage: cosidee <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f" || Option == "-o") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    //else if (Option == "-??") {
    //  if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
    //    cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
    //    cout<<Usage.str()<<endl;
    //    return false;
    //  }
    //}

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option == "-g") {
      m_GeometryFileName = argv[++i];
      cout<<"Accepting file name: "<<m_GeometryFileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  if (m_FileName == "") {
    cout<<"Error: Need a simulation file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_GeometryFileName == "") {
    cout<<"Error: Need a geometry file name!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  if (m_FileName.EndsWith(".sim") == false) {
    cout<<"Error: Need a simulation file name, not a "<<m_FileName<<" file "<<endl;
    cout<<Usage.str()<<endl;
    return false;
  }

  return true;
}


/******************************************************************************
 * Do whatever analysis is necessary
 */
bool MNCTDetectorEffectsEngineCOSI::Analyze()
{
  // Load geometry:
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    Geometry->ActivateNoising(false);
    Geometry->SetGlobalFailureRate(0.0);
    cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
  } else {
    cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
    return false;
  }  

  MFileEventsSim* Reader = new MFileEventsSim(Geometry);
  if (Reader->Open(m_FileName) == false) {
    cout<<"Unable to open sim file "<<m_FileName<<" - Aborting!"<<endl; 
    return false;
  }
  Reader->ShowProgress();
  
  MString RoaFileName = m_FileName;
  RoaFileName.ReplaceAllInPlace(".sim", ".roa");
  
  ofstream out;
  out.open(RoaFileName);
  out<<endl;
  out<<"TYPE   ROA"<<endl;
  out<<"UF     doublesidedstrip adcwithtiming"<<endl;
  out<<endl;
  
  MSimEvent* Event = 0;
  int RunningID = 0;
  while ((Event = Reader->GetNextEvent()) != 0) {
    // Hitting Ctrl-C raises this flag
    if (m_Interrupt == true) return false;

    if (Event->GetNHTs() == 0) {
      delete Event;
      continue;
    }

    //cout<<Event->GetID()<<endl;
    
    // Step (1): Convert positions into strip hits
    list<MNCTDEEStripHit> StripHits;
    
    // The real strips
    for (unsigned int h = 0; h < Event->GetNHTs(); ++h) {
      MSimHT* HT = Event->GetHTAt(h);
      MDVolumeSequence* VS = HT->GetVolumeSequence();
      MDDetector* Detector = VS->GetDetector();
      MString DetectorName = Detector->GetName();
      DetectorName.RemoveAllInPlace("Detector_");
      int DetectorID = DetectorName.ToInt();
      
      MNCTDEEStripHit pSide;
      MNCTDEEStripHit nSide;

      pSide.m_ROE.IsPositiveStrip(true);
      nSide.m_ROE.IsPositiveStrip(false);

      // Convert detector name in detector ID
      pSide.m_ROE.SetDetectorID(DetectorID);
      nSide.m_ROE.SetDetectorID(DetectorID);
      
      // Convert position into
      MVector PositionInDetector = VS->GetPositionInSensitiveVolume();
      MDGridPoint GP = Detector->GetGridPoint(PositionInDetector);
      
      // Not sure about if p or n-side is up, but we can debug this later
      pSide.m_ROE.SetStripID(GP.GetXGrid());
      nSide.m_ROE.SetStripID(GP.GetYGrid());
      
      pSide.m_Energy = HT->GetEnergy();
      nSide.m_Energy = HT->GetEnergy();
      
      pSide.m_Position = PositionInDetector;
      nSide.m_Position = PositionInDetector;
      
      StripHits.push_back(pSide);
      StripHits.push_back(nSide);
    }
    
    // and the guard ring hits
    for (unsigned int h = 0; h < Event->GetNGRs(); ++h) {
      MSimGR* GR = Event->GetGRAt(h);
      MDVolumeSequence* VS = GR->GetVolumeSequence();
      MDDetector* Detector = VS->GetDetector();
      MString DetectorName = Detector->GetName();
      DetectorName.RemoveAllInPlace("Detector_");
      int DetectorID = DetectorName.ToInt();
      
      MNCTDEEStripHit GuardRingHit;
      GuardRingHit.m_ROE.IsPositiveStrip(true); // <-- not important
      GuardRingHit.m_ROE.SetDetectorID(DetectorID);
      GuardRingHit.m_ROE.SetStripID(38); // ?
      
      GuardRingHit.m_Energy = GR->GetEnergy();
      GuardRingHit.m_Position = MVector(0, 0, 0); // <-- not important
    }
    
    
    // Step (2): Merge strip hits
    list<MNCTDEEStripHit> MergedStripHits;
    while (StripHits.size() > 0) {
      MNCTDEEStripHit Start;
      Start.m_SubStripHits.push_back(StripHits.front());
      StripHits.pop_front();
      Start.m_ROE = Start.m_SubStripHits.front().m_ROE;
       
      list<MNCTDEEStripHit>::iterator i = StripHits.begin();
      while (i != StripHits.end()) {
        if ((*i).m_ROE == Start.m_ROE) {
          Start.m_SubStripHits.push_back(*i);
          i = StripHits.erase(i);
        } else {
          ++i;
        }
      }
      MergedStripHits.push_back(Start);
    }
    

    
    // Step (3): Calculate and noise timing
    for (MNCTDEEStripHit& Hit: MergedStripHits) {
      double Energy = 0;
      double WeightedDepth = 0;
      for (MNCTDEEStripHit SubHit: Hit.m_SubStripHits) {
        Energy += SubHit.m_Energy;
        WeightedDepth += SubHit.m_Energy*SubHit.m_Position.GetZ();
      }
      double Depth = WeightedDepth / Energy;
            
      // Let's do a dummy timing:
      Hit.m_Timing = gRandom->Integer(2) + (3 + int(15*(0.75+Depth)));
    }
    
    
    // Step (4): Calculate and noise ADC values including charge loss, charge sharing, ADC overflow! 
    for (MNCTDEEStripHit& Hit: MergedStripHits) {
      double Energy = 0;
      for (MNCTDEEStripHit SubHit: Hit.m_SubStripHits) {
        Energy += SubHit.m_Energy;
      }
      
      // Let's do a dummy energy:
      Hit.m_Energy = Energy;
      Hit.m_ADC = int(10*Energy);
    }
    
    
    // Step (5): Apply thresholds and triggers including guard ring hits
    //            (a) use the trigger threshold calibration and invert it here 
    //            (b) take care of guard ring hits with their special thresholds
    //            (c) take care of hits in dead strips
    //            (d) throw out hits which did not trigger
    list<MNCTDEEStripHit>::iterator i = MergedStripHits.begin();
    while (i != MergedStripHits.end()) {
      if ((*i).m_Energy < 0) { // Dummy threshold of 0 keV sharp
        i = MergedStripHits.erase(i);
      } else {
        ++i;
      }
    }
    
    
    // Step (6): Split into card cage events - i.e. split by detector
    vector<vector<MNCTDEEStripHit>> CardCagedStripHits;
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
    
    
    // Step (7): Determine and noise the global event time
    vector<double> CardCageTiming(CardCagedStripHits.size());
    for (double& T: CardCageTiming) {
      T = Event->GetTime();
    }
    
    
    // Step (8): Dump event to file in ROA format
    for (unsigned int c = 0; c < CardCagedStripHits.size(); ++c) {
      out<<"SE"<<endl;
      out<<"# ID "<<Event->GetID()<<endl;
      out<<"ID "<<++RunningID<<endl;
      out<<"TI "<<CardCageTiming[c]<<endl;
      for (MNCTDEEStripHit Hit: CardCagedStripHits[c]) {
        out<<"SH "<<Hit.m_ROE.GetDetectorID()<<" "<<Hit.m_ROE.GetStripID()<<" "<<(Hit.m_ROE.IsPositiveStrip() ? "p" : "n")<<" "<<Hit.m_ADC<<" "<<Hit.m_Timing<<endl;
      }
    }
   
    
    // Never forget to delete the event
    delete Event;
  }

  // Some cleanup
  out<<"EN"<<endl<<endl;
  out.close();
  delete Reader;
  delete Geometry;
  
  return true;
}


/******************************************************************************/

MNCTDetectorEffectsEngineCOSI* g_Prg = 0;
int g_NInterrupts = 2;

/******************************************************************************/


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C:"<<endl;
  
  --g_NInterrupts;
  if (g_NInterrupts <= 0) {
    cout<<"Aborting..."<<endl;
    abort();
  } else {
    cout<<"Trying to cancel the analysis..."<<endl;
    if (g_Prg != 0) {
      g_Prg->Interrupt();
    }
    cout<<"If you hit "<<g_NInterrupts<<" more times, then I will abort immediately!"<<endl;
  }
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Set a default error handler and catch some signals...
  signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication MNCTDetectorEffectsEngineCOSIApp("MNCTDetectorEffectsEngineCOSIApp", 0, 0);

  g_Prg = new MNCTDetectorEffectsEngineCOSI();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //MNCTDetectorEffectsEngineCOSIApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
