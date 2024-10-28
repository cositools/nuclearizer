// run with CrossTalkOffsets -f /data/BackupPalestine... -o isotope_Crosstalk_Offset.txt -e lineenergy



//Still need to make the map < HistID,TH1D > Histograms
//Need to implement the option of only selecting one detector or one side for analysis

////////////////////////////////////////////////////////////////////////////////
/* 
 * CrossTalkOffsets.cxx
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
#include <TMath.h>

// MEGAlib
#include "MGlobal.h"
#include "MFile.h"
#include "MReadOutElementDoubleStrip.h"
#include "MFileReadOuts.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MReadOutSequence.h"
//#include "MReadOutDataADCValueWithTiming.h"
#include "MSupervisor.h"
#include "MModuleLoaderMeasurementsROA.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MModuleStripPairingGreedy.h"
#include "MAssembly.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT which plots the cross-talk histograms for each side of the 12 COSI'14 detectors and will probably eventually fit the histograms to find the cross talk corrections for each.
class CrossTalkOffsets
{
public:
  //! Default constructor
  CrossTalkOffsets();
  //! Default destructor
  ~CrossTalkOffsets();
  
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
	//! side: 1 for p, 0 for n to just do one at a time
//	bool m_Side;
	//! detector ID if you want to just do one at a time
//	int m_DetID;
	//! Energy of line to fit
	double LineEnergy;
	//! output file names
	MString m_OutFile;

};



////////////////////////////////////////////////////////////////////////////////


//! Default constructor
CrossTalkOffsets::CrossTalkOffsets() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
CrossTalkOffsets::~CrossTalkOffsets()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool CrossTalkOffsets::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage CrossTalkOffsets <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   .roa input file name"<<endl;
//Usage<<"         -d:   detector #, if you want to only do one at a time" << endl;
//Usage<<"         -s:   side can be n or p, again, to only do one at a time" << endl;
  Usage<<"	   -e:   energy of the line to fit in keV"<<endl;
  Usage<<"         -o:   outfile to save the offsets" << endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;


  LineEnergy = 0.0;
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
      	if (!((argc > i+1) && (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        	cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        	cout<<Usage.str()<<endl;
        	return false;
      	}		
    }	 

    // Then fulfill the options:
    if (Option == "-f") {
      	m_FileName = argv[++i];
      	cout<<"Accepting file name: "<<m_FileName<<endl;
    } 
	/*	if (Option == "-s"){
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
			//get detector ID
			if (atoi(argv[i+1]) < 12 && atoi(argv[i+1]) >= 0){
				m_DetID = atoi(argv[i+1]);
			}
			else {
				cout << "Error: detector # must be between 0 and 11" << endl;
				cout << Usage.str() << endl;
				return false;
			}
		} */
    if (Option == "-o"){
	m_OutFile = string(argv[i+1]);
	cout<<"Accepting output file name: "<<m_OutFile<<endl;
    }
	
    if (Option == "-e"){
	LineEnergy = atof(argv[i+1]);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool CrossTalkOffsets::Analyze()
{

  if (LineEnergy == 0) {
    cout<<"You need to give the line energy!"<<endl;
    return false;
  }
	
  //time code just to see
  TStopwatch watch;
  watch.Start();

  if (m_Interrupt == true) return false;

  MSupervisor* S = MSupervisor::GetSupervisor();
  
  MModuleLoaderMeasurementsROA* Loader = new MModuleLoaderMeasurementsROA();
  Loader->SetFileName(m_FileName);
  S->SetModule(Loader, 0);
   
  MModuleEnergyCalibrationUniversal* EnergyCalibrator = new MModuleEnergyCalibrationUniversal();
  EnergyCalibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI16/Wanaka/EnergyCalibration_053018.ecal");
  S->SetModule(EnergyCalibrator, 1);
  
  MModuleStripPairingGreedy* Pairing = new MModuleStripPairingGreedy();
  //Pairing->SetMode(0);
  S->SetModule(Pairing, 2);

  if (Loader->Initialize() == false) return false;
  if (EnergyCalibrator->Initialize() == false) return false;
  if (Pairing->Initialize() == false) return false;
  
  cout<<"LineEnergy: "<<LineEnergy<<endl;
  
  map<int, TH1D*> Histograms; //I'm going to make a somewhat complicated map between this int here and the detector ID and side and the relation between the strips (nearest neightbour, skip1, skip2, etc..)
//int = (DetectorID + 12*side)*(neighbours skipped + 1) (0 for p (x) side 1 for n (y) side), so HistID = 0-11 Nearest neightbour histograms from the p side of Det#0-11 and HistID = 36-47 skip1 historgrams from the n sides of Det#0-11. cause we also have four histograms for each detector (nn, skip1, skip2 and skip3) - so that's  2*12*4 = 96 historgrams! blah! 
  int HistID;
  

  map<int, TH1D*> SingleSite_Hist;
  for (int i = 0; i < 12; i++) {
	TH1D* SSHist = new TH1D("SingleSiteHist_" + MString(i), "SingleSiteHist_" + MString(i), 200, (LineEnergy-20), (LineEnergy+20));
	SSHist->SetXTitle("Energy (keV)");
	SSHist->SetYTitle("Counts");
	SingleSite_Hist[i] = SSHist;
  }

  bool IsFinished = false;
  MReadOutAssembly* Event = new MReadOutAssembly();

	while (IsFinished == false && m_Interrupt == false) {
		Event->Clear();
		Loader->AnalyzeEvent(Event);
		EnergyCalibrator->AnalyzeEvent(Event);
 		Pairing->AnalyzeEvent(Event);

		if (Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true) {
			//for (unsigned int DetectorID = 0; DetectorID < 12; ++DetectorID) { //Cycle through each detector for each event looking for two hits
			for (int d = 0; d < 12; ++d) {	
				int DetectorID;
				DetectorID = d;
				int GoodHit[2];
				int num_Single_DetHits = 0; //number of single site hits within one detector. We want this to be =2 for cross talk corrections.
				//cout<<"NHits :"<<Event->GetNHits()<<endl;

		        
				//HistID = DetectorID + 12*SideID, where SideID = 0 for n side and 1 for p side.	
				//initiallize all 71 of the histograms
        			//cout<<"D-ID: "<<DetectorID<<" - "<<Histograms[DetectorID]<<endl;
				if (Histograms[DetectorID] == 0) { //Initiate the 6 histograms belonging to the current detector
					for (int side = 0; side < 2; ++side) {
						for (int skip = 0; skip < 3; ++skip) {
							HistID = DetectorID + 12*side +  24*skip;
							TH1D* Hist = new TH1D("", MString("Detector ") + DetectorID + MString(", Side ") + side + MString(", Skip ") + skip, 200, (LineEnergy-50), (LineEnergy+50));
						  Hist->SetXTitle("Energy (keV)");
						  Hist->SetYTitle("Counts");
						 // cout<<"ID: "<<HistID<<endl;
              					  Histograms[HistID] = Hist;
						}
					}
	  			}


				for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
					//for each Event, cycle through and find if there are two hits in one detector, if so, move on with these two hits. 
					//only use events with two interactions (hits) and two strips in each hit, just to make it simplier
				
					//if (Event->GetHit(h)->GetNStripHits() == 0){ //Andreas recommened this tidbit when my program seg-faulted on Daniel's strip pairing code
					//	continue;
					//}
					
					if ( (Event->GetHit(h)->GetStripHit(0)->GetDetectorID() == DetectorID) && (Event->GetHit(h)->GetNStripHits() == 2)) {
						//count how many time we have a 2-strip hit (ie. easy single site) in the detector we are currently looking at
						GoodHit[num_Single_DetHits] = h; //and save the hit number for future reference
						num_Single_DetHits = num_Single_DetHits + 1;
						//cout<<"Found "<<num_Single_DetHits<<" good hits"<<endl;
					}
				}


				//Make a histogram of true single-site events with only two strips to double check energy calibration
				if ((num_Single_DetHits == 1) && (Event->GetHit(0)->GetNStripHits() == 2)) {	
					if (Event->GetHit(0)->GetStripHit(0)->IsLowVoltageStrip() == true) {
						SingleSite_Hist[DetectorID]->Fill(Event->GetHit(0)->GetStripHit(1)->GetEnergy());
					} else {
			//			SingleSite_Hist[DetectorID]->Fill(Event->GetHit(0)->GetStripHit(0)->GetEnergy());
					}
					//SingleSite_Hist[DetectorID]->Fill(Event->GetHit(0)->GetEnergy());
				}				

		
				//use variables h1 and h2 instead of the vector GoodHit[] for simplicity of reading. Could change this later...
				if (num_Single_DetHits == 2) { 

					int nXhit = 0;
					int nYhit = 0;
					double x_energy[2] = {0.0, 0.0};
					double y_energy[2] = {0.0, 0.0};
					int x_strip[2] = {0, 0};
					int y_strip[2] = {0, 0};
					
					//cout<<"Found a GOODHIT!"<<endl;
					//Get the detector side, strip number, and energy from each of the hits
					for (unsigned int i = 0; i < 2; ++i) { //loop through the two hits...
						for (unsigned int s = 0; s < 2; ++s) { //...each with two strips...
							//...to record the energy and strip number of each strip hit fo later comparison 
							if (Event->GetHit(GoodHit[i])->GetStripHit(s)->IsLowVoltageStrip() == true) {
								x_energy[nXhit] = Event->GetHit(GoodHit[i])->GetStripHit(s)->GetEnergy();
								x_strip[nXhit] = Event->GetHit(GoodHit[i])->GetStripHit(s)->GetStripID();
								nXhit = nXhit + 1;
							} else {
								y_energy[nYhit] = Event->GetHit(GoodHit[i])->GetStripHit(s)->GetEnergy();
								y_strip[nYhit] = Event->GetHit(GoodHit[i])->GetStripHit(s)->GetStripID();
								nYhit = nYhit + 1;
							}
						}
					}
					//Now we have the energys and strip numbers associated with our two hit event with only four active strips. Now we need to determine if we have adjacent, or skip1, etc, strips
				
					//cout<<"X hit 1: "<<x_strip[0]<<" "<<x_energy[0]<<endl;
					//cout<<"X hit 2: "<<x_strip[1]<<" "<<x_energy[1]<<endl;
					//cout<<"Y hit 1: "<<y_strip[0]<<" "<<y_energy[0]<<endl;
					//cout<<"Y hit 2: "<<y_strip[1]<<" "<<y_energy[1]<<endl;
					
					//initialize the variables used to fill the histograms
					double x_adj_energy = 0.0;
					double y_adj_energy = 0.0;
					double x_skip1_energy = 0.0;
					double y_skip1_energy = 0.0;
					double x_skip2_energy = 0.0;
					double y_skip2_energy = 0.0;
				
					//Check adjacency on both sides to determine which histogram to fill	
					if ( (abs(x_strip[0] - x_strip[1]) != 1) && (abs(y_strip[0] - y_strip[1]) == 1) ){
						//cout << "Found adjacent strips on the y-side!" << endl;
						y_adj_energy += y_energy[0] + y_energy[1];
						HistID = DetectorID + 12*1 + 24*0; 		
	          			Histograms[HistID]->Fill(y_adj_energy);		
					}
					if ( (abs(y_strip[0] - y_strip[1]) != 1) && (abs(x_strip[0] - x_strip[1]) == 1) ){
						//cout << "Found adjacent strips on the x-side!" << endl;
						x_adj_energy += x_energy[0] + x_energy[1];
						HistID = DetectorID + 12*0 + 24*0;
						Histograms[HistID]->Fill(x_adj_energy);
					}
					if ( (abs(x_strip[0] - x_strip[1]) != 1) && (abs(y_strip[0] - y_strip[1]) == 2) ) {
						//cout << "Found Skip1 strips on the y-side!" << endl;
						y_skip1_energy += y_energy[0] + y_energy[1];
						HistID = DetectorID + 12*1 + 24*1;
						Histograms[HistID]->Fill(y_skip1_energy);
					}
					if ( (abs(y_strip[0] - y_strip[1]) != 1) && (abs(x_strip[0] - x_strip[1]) == 2) ) {
						//cout << "Found Skip1 strips on the x-side!" << endl;
						x_skip1_energy += x_energy[0] + x_energy[1];
						HistID = DetectorID + 12*0 + 24*1;
						Histograms[HistID]->Fill(x_skip1_energy);
					}
					if ( (abs(x_strip[0] - x_strip[1]) != 1) && (abs(y_strip[0] - y_strip[1]) == 3) ) {
						//cout << "Found Skip2 strips on the y-side!" << endl;
						y_skip2_energy += y_energy[0] + y_energy[1];
						HistID = DetectorID + 12*1 + 24*2;
						Histograms[HistID]->Fill(y_skip2_energy);
					}
					if ( (abs(y_strip[0] - y_strip[1]) != 1) && (abs(x_strip[0] - x_strip[1]) == 3) ) {
						//cout << "Found Skip2 strips on the x-side!" << endl;
						x_skip2_energy += x_energy[0] + x_energy[1];
						HistID = DetectorID + 12*0 + 24*2;
						Histograms[HistID]->Fill(x_skip2_energy);
					}
				}
			}
		}

	  	IsFinished = Loader->IsFinished();
	}

	//create file to save all of the offset numbers in...
	ofstream crosstalk_offsets;
	crosstalk_offsets.open(m_OutFile);
	crosstalk_offsets <<LineEnergy<<"\n \n";

	cout << "drawing histograms" << endl; 

  TF1 * f_gausstail = new TF1("f_gausstail","[0]*( [1]/(2*[2])*exp( (x - [3])/[2] + [4]^2/(2*[2]^2) )*(1 - TMath::Erf( (x-[3])/sqrt(2)/[4] + [4]/sqrt(2)/[2] ) ) ) + (1 - [0])*( [1]/(sqrt(2*pi)*[4])*exp( -(x - [3])^2/(2*[4]^2) ) ) + [5]",(LineEnergy -25), (LineEnergy+35));
	f_gausstail->SetParName(0,"eta");
	f_gausstail->SetParName(1,"N");
	f_gausstail->SetParName(2,"lambda");
	f_gausstail->SetParName(3,"mu");
	f_gausstail->SetParName(4,"sigma");
	f_gausstail->SetParName(5,"y-offset");
	f_gausstail->SetParameter(0,0.5);
	f_gausstail->SetParLimits(0,0.15,0.8);
	f_gausstail->SetParameter(1,300);
	f_gausstail->SetParLimits(1,100,10000000);
	f_gausstail->SetParameter(2,10);
	f_gausstail->SetParLimits(2,2,30);
	f_gausstail->SetParameter(3,(LineEnergy+1));
	f_gausstail->SetParLimits(3,(LineEnergy-2),(LineEnergy+25));
	f_gausstail->SetParameter(4,1.5);
	f_gausstail->SetParLimits(4,0.5,6);
	//f_gausstail->SetParameter(5,30);

	//TF1 *f1 = new TF1("f1","gaus", 650,680);

	//fit the histogram and save the output 
	for (auto H: Histograms){
		double offset = 0.0;
		double offset_err = 0.0;
		TCanvas* C = new TCanvas();
		C->SetWindowSize(600,600);
		C->cd();
		H.second->Draw("colz");
		C->Update();
		H.second->Fit("f_gausstail", "R");
		TF1 *fit = H.second->GetFunction("f_gausstail");
		offset = (fit->GetParameter(3) - LineEnergy);
		offset_err = fit->GetParError(3);
		cout<<"Offset from fit: "<<offset<<"+/-"<<offset_err<<endl;
		C->Update();
		crosstalk_offsets << H.first;
		crosstalk_offsets << ' ' << offset << ' ' <<offset_err<<'\n';
		C->Print(MString("CrossTalkOffset_")+round(LineEnergy)+MString("_")+H.first+MString(".root"));
	}
	
	crosstalk_offsets.close();
	watch.Stop();
	cout << "total time (s): " << watch.CpuTime() << endl;
 

	cout<<"\n /////////////// Single Site Fit ////////////// \n \n"<<endl;


	for (int i = 0; i < 12; i++) {
		cout<<"\n\n Detector "<<i<<endl;
		TCanvas * C = new TCanvas();
		SingleSite_Hist[i]->Draw();
		SingleSite_Hist[i]->Fit("f_gausstail", "R");
		TF1 *fit = SingleSite_Hist[i]->GetFunction("f_gausstail");
		double offset = (fit->GetParameter(3) - LineEnergy);
		double offset_err = fit->GetParError(3);
		cout<<"Offset from fit: "<<offset<<"+/-"<<offset_err<<endl;
		C->Update();
		C->Print(MString("SingleSite_Det")+i+MString("_2Strips.root"));
	}


	return true;
}


////////////////////////////////////////////////////////////////////////////////


CrossTalkOffsets* g_Prg = 0;
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

  TApplication CrossTalkOffsetsApp("CrossTalkOffsetsApp", 0, 0);

  g_Prg = new CrossTalkOffsets();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  CrossTalkOffsetsApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
