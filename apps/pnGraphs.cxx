/* 
 * pnGraphs.cxx
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
#include <cstdlib>
#include <map>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>

// MEGAlib
#include "MGlobal.h"
#include "MFile.h"
#include "MReadOutElementDoubleStrip.h"
#include "MFileReadOuts.h"
#include "MReadOutAssembly.h"
#include "MNCTStripHit.h"
#include "MReadOutSequence.h"
#include "MReadOutDataADCValueWithTiming.h"
#include "MSupervisor.h"
#include "MNCTModuleMeasurementLoaderROA.h"
#include "MNCTModuleEnergyCalibrationUniversal.h"
#include "MNCTModuleStripPairingGreedy_b.h"
#include "MAssembly.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class pnGraphs
{
public:
  //! Default constructor
  pnGraphs();
  //! Default destructor
  ~pnGraphs();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }


private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The input file name
  MString m_FileName;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
pnGraphs::pnGraphs() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
pnGraphs::~pnGraphs()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool pnGraphs::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: pnGraphs <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name"<<endl;
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
    if (Option == "-f") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments template
    /*
    else if (Option == "-??") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
    */

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    } 
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool pnGraphs::Analyze()
{
  if (m_Interrupt == true) return false;

  MSupervisor S;
  
  MNCTModuleMeasurementLoaderROA* Loader = new MNCTModuleMeasurementLoaderROA();
  Loader->SetFileName(m_FileName);
  S.SetModule(Loader, 0);
  
  MNCTModuleEnergyCalibrationUniversal* Calibrator = new MNCTModuleEnergyCalibrationUniversal();
  Calibrator->SetFileName("$(NUCLEARIZER)/cal/EnergyCalibration.ecal");
  S.SetModule(Calibrator, 1);
  
  MNCTModuleStripPairingGreedy_b* Pairing = new MNCTModuleStripPairingGreedy_b();
  S.SetModule(Pairing, 2);

  if (Loader->Initialize() == false) return false;
  if (Calibrator->Initialize() == false) return false;
  if (Pairing->Initialize() == false) return false;
  
  map<int, TH2D*> Histograms;
  
  
  bool IsFinished = false;
  MReadOutAssembly* Event = new MReadOutAssembly();
  while (IsFinished == false && m_Interrupt == false) {
    Event->Clear();
    Loader->AnalyzeEvent(Event);
    Calibrator->AnalyzeEvent(Event);
    Pairing->AnalyzeEvent(Event);
    
    if (Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true) {
      for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
        int pNStrips = 0;
        double pEnergy = 0.0;
        int nNStrips = 0;
        double nEnergy = 0.0;
    
        int DetectorID = 0;
        for (unsigned int s = 0; s < Event->GetHit(h)->GetNStripHits(); ++s) {
          DetectorID = Event->GetHit(h)->GetStripHit(s)->GetDetectorID();
          if (Event->GetHit(h)->GetStripHit(s)->IsXStrip() == true) {
            ++pNStrips;
            pEnergy += Event->GetHit(h)->GetStripHit(s)->GetEnergy(); 
          } else {
            ++nNStrips;
            nEnergy += Event->GetHit(h)->GetStripHit(s)->GetEnergy(); 
          }
        }
        if (pNStrips > 0 && nNStrips > 0) {
          if (Histograms[DetectorID] == 0) {
            TH2D* Hist = new TH2D("", MString("Detector ") + DetectorID, 1400, 0, 1400, 1400, 0, 1400);
            Hist->SetXTitle("p-Side Energy [keV]");
            Hist->SetYTitle("n-Side Energy [keV]");
            Histograms[DetectorID] = Hist;
          }
          Histograms[DetectorID]->Fill(pEnergy, nEnergy);
        }
      }
    }
    
    IsFinished = Loader->IsFinished();
  }
  
  for (auto H: Histograms) {
    TCanvas* C = new TCanvas();
    C->SetWindowSize(1000, 1000);
    C->SetLogz();
    C->cd();
    H.second->Draw("colz");
    C->Update();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


pnGraphs* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Main program
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Standalone", "a standalone example program");

  TApplication pnGraphsApp("pnGraphsApp", 0, 0);

  g_Prg = new pnGraphs();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  pnGraphsApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
