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
#include <TStopwatch.h>

// MEGAlib
#include "MGlobal.h"
#include "MFile.h"
#include "MReadOutElementDoubleStrip.h"
#include "MFileReadOuts.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MReadOutSequence.h"
#include "MSupervisor.h"
#include "MModuleLoaderMeasurementsROA.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MModuleStripPairingGreedy.h"
#include "MAssembly.h"
#include "MModuleLoaderMeasurementsBinary.h"

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
	//! side: 1 for p, 0 for n
	bool m_Side;
	//! 1 for detectors, 0 for strips
	bool m_DetOp;
	//! detector ID
	unsigned int m_DetID;
	//! output file names
	MString m_OutFile;

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
	Usage<<"         -d:   look detector by detector" << endl;
	Usage<<"         -s:   look strip by strip for one detector" << endl;
	Usage<<"                example: -s detID side"<< endl;
	Usage<<"                side can be n or p"<< endl;
	Usage<<"         -o:   outfile" << endl;
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
    } 

		if (Option == "-s"){
			m_DetOp = 0;
			//get detector ID
			if (atoi(argv[i+1]) < 12 && atoi(argv[i+1]) >= 0){
				m_DetID = atoi(argv[i+1]);
			}
			else {
				cout << "Error: detector must be between 0 and 12" << endl;
				cout << Usage.str() << endl;
				return false;
			}
			//get side
			if (string(argv[i+2]) == "n"){
				m_Side = 0;
			}
			else if (string(argv[i+2]) == "p"){
				m_Side = 1;
			}
			else {
				cout << "Error: side must be n or p" << endl;
				cout << Usage.str() << endl;
				return false;
			}
		}

		if (Option == "-d"){
			m_DetOp = 1;
		}

		if (Option == "-o"){
			m_OutFile = string(argv[i+1]);
		}


	}

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool pnGraphs::Analyze()
{
	//time code just to see
	TStopwatch watch;
	watch.Start();

	if (m_Interrupt == true) return false;

  MSupervisor* S = MSupervisor::GetSupervisor();
  
  MModuleLoaderMeasurementsROA* Loader = new MModuleLoaderMeasurementsROA();
  Loader->SetFileName(m_FileName);
  S->SetModule(Loader, 0);
  
  MModuleEnergyCalibrationUniversal* Calibrator = new MModuleEnergyCalibrationUniversal();
  Calibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI14/EnergyCalibration.ecal");
  S->SetModule(Calibrator, 1);
  
  MModuleStripPairingGreedy* Pairing = new MModuleStripPairingGreedy();
  S->SetModule(Pairing, 2);

  MModuleLoaderMeasurementsBinary* MLB = new MModuleLoaderMeasurementsBinary();
  MLB->Initialize();
  
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
			if (m_DetOp) {
	      for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
	        int pNStrips = 0;
	        double pEnergy = 0.0;
	        int nNStrips = 0;
	        double nEnergy = 0.0;    

	        int DetectorID = 0;
	        for (unsigned int s = 0; s < Event->GetHit(h)->GetNStripHits(); ++s) {
	          DetectorID = Event->GetHit(h)->GetStripHit(s)->GetDetectorID();
	          if (Event->GetHit(h)->GetStripHit(s)->IsLowVoltageStrip() == true) {
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


			else {
				unsigned int nStrips = 37;
				unsigned int detectorID = 0;
				unsigned int stripID = 0;
				//currently just making plots for 1 detector, x strips
				for (unsigned int strip=1; strip<=nStrips; strip++){
					int pNStrips = 0;
					double pEnergy = 0.0;
					int nNStrips = 0;
					double nEnergy = 0.0;

					bool addHitToHist = false;

					for (unsigned int h=0; h<Event->GetNHits(); h++){
						for (unsigned int s=0; s<Event->GetHit(h)->GetNStripHits(); s++){
							detectorID = Event->GetHit(h)->GetStripHit(s)->GetDetectorID();
							if (Event->GetHit(h)->GetStripHit(s)->IsLowVoltageStrip()){
								if (m_Side == 1){
									stripID = Event->GetHit(h)->GetStripHit(s)->GetStripID();
									if (stripID == strip && m_DetID == detectorID){
										addHitToHist = true;
									}
								}
								++pNStrips;
								pEnergy += Event->GetHit(h)->GetStripHit(s)->GetEnergy();
							}
							else {
								if (m_Side == 0){
									stripID = Event->GetHit(h)->GetStripHit(s)->GetStripID();
									if (stripID == strip && m_DetID == detectorID){
										addHitToHist = true;
									}
								}
								++nNStrips;
								nEnergy += Event->GetHit(h)->GetStripHit(s)->GetEnergy();
							}
						}
						if (pNStrips>0 && nNStrips>0 && addHitToHist == true){
							//histogram stuff
		          if (Histograms[strip] == 0) {
								MString histTitle;
								if (m_Side == 1){
									histTitle = MString("Detector ")+detectorID+MString(", p Strip ")+strip;
								}
								else {
									histTitle = MString("Detector ")+detectorID+MString(", n Strip ")+strip;
								}
	  	          TH2D* Hist = new TH2D("", histTitle, 1400, 0, 1400, 1400, 0, 1400);
	  	          Hist->SetXTitle("p-Side Energy [keV]");
	  	          Hist->SetYTitle("n-Side Energy [keV]");
	  	          Histograms[strip] = Hist;
	  	        }
	  	        Histograms[strip]->Fill(pEnergy, nEnergy);
	  	      }
						addHitToHist = false;
					}
				}
			}
		}
 
    IsFinished = Loader->IsFinished();
	}


	//line where graph should be
	double x_arr[700], y_arr[700];
	for (int i=0; i<700; i++){
		x_arr[i] = i*2;
		y_arr[i] = i*2;
	}
		
	TGraph *graph = new TGraph(700,x_arr,y_arr);

	if (m_DetOp){
		int i=0;
		for (auto H: Histograms){
			TCanvas* C = new TCanvas();
			C->SetWindowSize(1000,1000);
			C->SetLogz();
			C->cd();
			H.second->Draw("colz");
			graph->Draw("same");
			C->Update();
			C->Print(m_OutFile+MString("_")+i+MString(".pdf"));
			i++;
		}
	}
	else {
		int numStrips = 37;
		for (int H=1; H<=numStrips; H+=4){
		  TCanvas* C = new TCanvas();
			C->Divide(2,2);
		  C->SetWindowSize(1000, 1000);
//		  C->SetLogz();
			int endStrip = H;
			for (int i=0; i<4; i++){
				if (H+i <= numStrips){
				  C->cd(i+1);
					gPad->SetLogz();
					if (Histograms.find(H+i) != Histograms.end()){
						Histograms[H+i]->Draw("colz");
						endStrip = H+i;
					}
				  C->Update();
				}
			}
			if (m_Side == true){
				C->Print(m_OutFile+MString("_Det")+m_DetID+MString("_pStrips_")+H+MString("-")+endStrip+MString(".pdf"));
			}
			else {
				C->Print(m_OutFile+MString("_Det")+m_DetID+MString("_nStrips_")+H+MString("-")+endStrip+MString(".pdf"));
			}
		}
  }
 
	watch.Stop();
	cout << "total time (s): " << watch.CpuTime() << endl;
 
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
