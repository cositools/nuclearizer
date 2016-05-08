/* 
 * ChargeLossCorrection.cxx
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
#include <vector>
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
#include "MNCTStripHit.h"
#include "MReadOutSequence.h"
#include "MSupervisor.h"
#include "MNCTModuleMeasurementLoaderROA.h"
#include "MNCTModuleEnergyCalibrationUniversal.h"
#include "MNCTModuleStripPairingGreedy_b.h"
#include "MNCTModuleChargeSharingCorrection.h"
#include "MAssembly.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class ChargeLossCorrection
{
public:
  //! Default constructor
  ChargeLossCorrection();
  //! Default destructor
  ~ChargeLossCorrection();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
	//!load cross talk correction
	vector<vector<vector<float> > > LoadCrossTalk();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

	void dummy_func() { return; }

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The input file name
  MString m_FileName;
	//! output file names
	MString m_OutFile;
	//! energy E0
	float m_E0;

};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ChargeLossCorrection::ChargeLossCorrection() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
ChargeLossCorrection::~ChargeLossCorrection()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool ChargeLossCorrection::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ChargeLossCorrection <options>"<<endl;
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

		if (Option == "-e"){
			m_E0 = atof(argv[i+1]);
		}

		if (Option == "-o"){
			m_OutFile = string(argv[i+1]);
		}


	}

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool ChargeLossCorrection::Analyze()
{
	cout << "E0: " << m_E0 << endl;

	//time code just to see
	TStopwatch watch;
	watch.Start();

	if (m_Interrupt == true) return false;

  MSupervisor* S = MSupervisor::GetSupervisor();
  
  MNCTModuleMeasurementLoaderROA* Loader = new MNCTModuleMeasurementLoaderROA();
  Loader->SetFileName(m_FileName);
  S->SetModule(Loader, 0);
  
  MNCTModuleEnergyCalibrationUniversal* Calibrator = new MNCTModuleEnergyCalibrationUniversal();
  Calibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI14/EnergyCalibration.ecal");
  S->SetModule(Calibrator, 1);
  
  MNCTModuleStripPairingGreedy_b* Pairing = new MNCTModuleStripPairingGreedy_b();
  S->SetModule(Pairing, 2);

//	MNCTModuleChargeSharingCorrection* ChargeLoss = new MNCTModuleChargeSharingCorrection();
//	S.SetModule(ChargeLoss, 3);

  if (Loader->Initialize() == false) return false;
  if (Calibrator->Initialize() == false) return false;
  if (Pairing->Initialize() == false) return false;
  
  map<int, TH2D*> Histograms;
 	vector<vector<vector<float> > > crossTalkCoeffs;
	crossTalkCoeffs = LoadCrossTalk(); 

  bool IsFinished = false;
  MReadOutAssembly* Event = new MReadOutAssembly();
  while (IsFinished == false && m_Interrupt == false) {
    Event->Clear();
    Loader->AnalyzeEvent(Event);
    Calibrator->AnalyzeEvent(Event);
    Pairing->AnalyzeEvent(Event);

	  if (Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true) {
			vector<int> xStripIDs, yStripIDs;
			vector<float> xEnergy, yEnergy;
			for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
				//variables to add to histograms
				float sum = 0.0;
				float diff = 0.0;
				float xE, yE = 0.0;
				float dE = 0.0;
				int detectorID = 0;
				int histID = -1;

				if (Event->GetHit(h)->GetNStripHits()==3){
					for (unsigned int s = 0; s < Event->GetHit(h)->GetNStripHits(); s++){
						detectorID = Event->GetHit(h)->GetStripHit(s)->GetDetectorID();
						if (Event->GetHit(h)->GetStripHit(s)->IsXStrip()){
							xStripIDs.push_back(Event->GetHit(h)->GetStripHit(s)->GetStripID());
							xE = Event->GetHit(h)->GetStripHit(s)->GetEnergy();
							//cross talk correction
							dE = crossTalkCoeffs[detectorID][1][0] + crossTalkCoeffs[detectorID][1][1]*xE;
							xEnergy.push_back(xE-dE);
						}
						else {
							yStripIDs.push_back(Event->GetHit(h)->GetStripHit(s)->GetStripID());
							yE = Event->GetHit(h)->GetStripHit(s)->GetEnergy();
							//cross talk correction
							dE = crossTalkCoeffs[detectorID][0][0] + crossTalkCoeffs[detectorID][0][1]*yE;
							yEnergy.push_back(yE-dE);
						}
					}
					if (xStripIDs.size() == 2){
						if (fabs(xStripIDs.at(0)-xStripIDs.at(1)) == 1){
			//				cout << "P accepted: " << endl;
			//				cout << xStripIDs.at(0) << '\t' << xStripIDs.at(1) << endl;
			//				cout << xEnergy.at(0) << '\t' << xEnergy.at(1) << endl;
							sum = xEnergy.at(0) + xEnergy.at(1);
							diff = fabs(xEnergy.at(0) - xEnergy.at(1));
							histID = 10*detectorID;
//							cout << "sum: " << sum << endl;
//							cout << "n energy: " << yEnergy.at(0) << endl;
//							cout << "diff: " << diff << endl;
							dummy_func();
						}
					}
					else if (yStripIDs.size() == 2){
						if (fabs(yStripIDs.at(0)-yStripIDs.at(1)) == 1){
//							cout << "N accepted: " << endl;
//							cout << yStripIDs.at(0) << '\t' << yStripIDs.at(1) << endl;
//							cout << yEnergy.at(0) << '\t' << yEnergy.at(1) << endl;
							sum = yEnergy.at(0) + yEnergy.at(1);
							diff = fabs(yEnergy.at(0) - yEnergy.at(1));
							histID = 10*detectorID + 1;
//							cout << "sum: " << sum << endl;
//							cout << "p energy: " << xEnergy.at(0) << endl;
//							cout << "diff: " << diff << endl;
							dummy_func();
						}
						else {
//							cout << "not accepted: " << endl;
//							cout << yStripIDs.at(0) << '\t' << yStripIDs.at(1) << endl;
//							cout << yEnergy.at(0) << '\t' << yEnergy.at(1) << endl;
						}
					}
	      	if (histID != -1) {
						int nxbins = 0;
						int nybins = 0;
						int xlowlim = 0;
						int xhighlim = 0;
						int ylowlim = 0;
						int yhighlim = 0;
						if (m_E0 == 662.){
							ylowlim = 630;
							yhighlim = 680;
							nybins = 50;
							xlowlim = -600;
							xhighlim = 600;
							nxbins = 1200;
						}
						else if (m_E0 == 1333.){
							ylowlim = 1300;
							yhighlim = 1350;
							nybins = 50;
							xlowlim = -1500;
							xhighlim = 1500;
							nxbins = 3000;
						}
						else if (m_E0 == 356.){
							ylowlim = 325;
							yhighlim = 365;
							nybins = 40;
							xlowlim = -300;
							xhighlim = 300;
							nxbins = 600;
						}
						else if (m_E0 == 122){
							ylowlim = 90;
							yhighlim = 130;
							nybins = 40;
							xlowlim = -100;
							xhighlim = 100;
							nxbins = 200;
						}
	      	  if (Histograms[histID] == 0) {
		      	   TH2D* Hist = new TH2D("", "", nxbins,xlowlim,xhighlim,nybins,ylowlim,yhighlim);
							if (histID%10 == 0){
		      	    Hist->SetTitle(MString("Detector ")+detectorID+MString(" p Side"));
							}
							else if (histID%10 == 1){
								Hist->SetTitle(MString("Detector ")+detectorID+MString(" n Side"));
							}
	      	    Hist->SetXTitle("Difference [keV]");
	      	    Hist->SetYTitle("Sum [keV]");
	      	    Histograms[histID] = Hist;
	      	  }
	      	  Histograms[histID]->Fill(diff,sum);
						Histograms[histID]->Fill(-diff,sum);
	      	}
	  	  }
//				cout << "----------" << endl;
				xEnergy.clear();
				yEnergy.clear();
				xStripIDs.clear();
				yStripIDs.clear();
			}
		}
 
    IsFinished = Loader->IsFinished();
	}


	//fit and plot histograms
	//make fit functions
	int low=300;
	int high=600;
	if (m_E0 == 356){
		low = 150;
		high = 300;
	}
	else if (m_E0 == 1333){
		low = 1000;
		high = 1500;
	}
	TF1* highE_fitFunc = new TF1("highE","[1]-[0]*([1]-x)",low,high);
	highE_fitFunc->FixParameter(1,m_E0);
	TF1* lowE_fitFunc = new TF1("lowE","[1]-([0]/(2*[1]))*([1]*[1]-x*x)",0,100);
	lowE_fitFunc->FixParameter(1,m_E0);

	//setup output file
	ofstream logFitStats;
	logFitStats.open("ChargeLossCorrection.log");
	logFitStats << "Det" << '\t' << "side" << '\t' << "B" << endl << endl;

	int det=0;
	int side=0;
	int i=0;
	for (auto H: Histograms){
		TCanvas* C = new TCanvas();
		C->SetWindowSize(1000,1000);
//		C->SetLogz();
		C->cd();
		H.second->Draw("colz");
		double param = 0.0;
		if (m_E0 > 300){
			H.second->Fit("highE","R");
			param = highE_fitFunc->GetParameter(0);
		}
		else {
			H.second->Fit("lowE","R");
			param = lowE_fitFunc->GetParameter(0);
		}
		C->Update();

		det = i/2;
		side = i%2;
		logFitStats << det << '\t' << side << '\t' << param << endl;

		C->Print(m_OutFile+MString("_Det")+det+MString("_Side")+side+MString(".pdf"));
		i++;
	}

	watch.Stop();
	cout << "total time (s): " << watch.CpuTime() << endl;
 
  return true;
}


/////////////////////////////////////////////////////////////////////////////////

vector<vector<vector<float> > > ChargeLossCorrection::LoadCrossTalk(){

	float crossTalkCoeffsArr[12][2][2];

	//detector 0, n side, a0 and a1
	crossTalkCoeffsArr[0][0][0] = 0.0367227;
	crossTalkCoeffsArr[0][0][1] = 0.014819;
	//detector 0, p side, a0 and a1
	crossTalkCoeffsArr[0][1][0] = 0.145619;
	crossTalkCoeffsArr[0][1][1] = 0.015549;

	//detector 1, n side, a0 and a1
	crossTalkCoeffsArr[1][0][0] = 0.484833;
	crossTalkCoeffsArr[1][0][1] = 0.014315;
	//detector 1, p side, a0 and a1
	crossTalkCoeffsArr[1][1][0] = 0.739853;
	crossTalkCoeffsArr[1][1][1] = 0.0164997;

	//detector 2, side 0, a0 and a1
	crossTalkCoeffsArr[2][0][0] = 0.145073;
	crossTalkCoeffsArr[2][0][1] = 0.014666;
	//detector 2, side 1, a0 and a1
	crossTalkCoeffsArr[2][1][0] = 0.856062;
	crossTalkCoeffsArr[2][1][1] = 0.0156687;

	//detector 3, side 0, a0 and a1
	crossTalkCoeffsArr[3][0][0] = -2.82417;
	crossTalkCoeffsArr[3][0][1] = 0.0192134;
	//detector 3, side 1, a0 and a1
	crossTalkCoeffsArr[3][1][0] = 0.351416;
	crossTalkCoeffsArr[3][1][1] = 0.0151572;

	//detector 4, side 0, a0 and a1
	crossTalkCoeffsArr[4][0][0] = 0.227316;
	crossTalkCoeffsArr[4][0][1] = 0.0145653;
	//detector 4, side 1, a0 and a1
	crossTalkCoeffsArr[4][1][0] = 0.0348345;
	crossTalkCoeffsArr[4][1][1] = 0.0159006;

	//detector 5, side 0, a0 and a1
	crossTalkCoeffsArr[5][0][0] = 0.151713;
	crossTalkCoeffsArr[5][0][1] = 0.0144765;
	//detector 5, side 1, a0 and a1
	crossTalkCoeffsArr[5][1][0] = 0.438013;
	crossTalkCoeffsArr[5][1][1] = 0.0162422;

	//detector 6, side 0, a0 and a1
	crossTalkCoeffsArr[6][0][0] = 0.184349;
	crossTalkCoeffsArr[6][0][1] = 0.0141778;
	//detector 6, side 1, a0 and a1
	crossTalkCoeffsArr[6][1][0] = 0.190339;
	crossTalkCoeffsArr[6][1][1] = 0.0158667;

	//detector 7, side 0, a0 and a1
	crossTalkCoeffsArr[7][0][0] = 0.10767;
	crossTalkCoeffsArr[7][0][1] = 0.0144137;
	//detector 7, side 1, a0 and a1
	crossTalkCoeffsArr[7][1][0] = 0.938774;
	crossTalkCoeffsArr[7][1][1] = 0.0151262;

	//detector 8, side 0, a0 and a1
	crossTalkCoeffsArr[8][0][0] = -0.0966355;
	crossTalkCoeffsArr[8][0][1] = 0.0145684;
	//detector 8, side 1, a0 and a1
	crossTalkCoeffsArr[8][1][0] = 0.0115163;
	crossTalkCoeffsArr[8][1][1] = 0.0158602;

	//detector 9, side 0, a0 and a1
	crossTalkCoeffsArr[1][0][0] = 0.248337;
	crossTalkCoeffsArr[1][0][1] = 0.0144792;
	//detector 9, side 1, a0 and a1
	crossTalkCoeffsArr[1][1][0] = -0.133137;
	crossTalkCoeffsArr[1][1][1] = 0.016439;

	//detector 10, side 0, a0 and a1
	crossTalkCoeffsArr[10][0][0] = 0.068672;
	crossTalkCoeffsArr[10][0][1] = 0.0137224;
	//detector 10, side 1, a0 and a1
	crossTalkCoeffsArr[10][1][0] = 0.384968;
	crossTalkCoeffsArr[10][1][1] = 0.0151702;

	//detector 11, side 0, a0 and a1
	crossTalkCoeffsArr[11][0][0] = 0.195844;
	crossTalkCoeffsArr[11][0][1] = 0.0135414;
	//detector 11, side 1, a0 and a1
	crossTalkCoeffsArr[11][1][0] = 0.375227;
	crossTalkCoeffsArr[11][1][1] = 0.0156169;

	vector<vector<vector<float> > > crossTalkCoeffs;
	vector<vector<float> > tempOne;
	vector<float> tempTwo;
	for (int i=0; i<12; i++){
		for (int j=0; j<2; j++){
			for (int k=0; k<2; k++){
				tempTwo.push_back(0);
			}
			tempOne.push_back(tempTwo);
			tempTwo.clear();
		}
		crossTalkCoeffs.push_back(tempOne);
		tempOne.clear();
	}

	for (int i=0; i<12; i++){
		for (int j=0; j<2; j++){
			for (int k=0; k<2; k++){
				crossTalkCoeffs[i][j][k] = crossTalkCoeffsArr[i][j][k];
			}
		}
	}

	return crossTalkCoeffs;

};


////////////////////////////////////////////////////////////////////////////////


ChargeLossCorrection* g_Prg = 0;
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

  TApplication ChargeLossCorrectionApp("ChargeLossCorrectionApp", 0, 0);

  g_Prg = new ChargeLossCorrection();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  ChargeLossCorrectionApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
