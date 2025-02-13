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
#include <TProfile.h>

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
#include "MBinaryFlightDataParser.h"
#include "MModuleLoaderMeasurementsBinary.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MModuleStripPairingGreedy.h"
#include "MModuleCrosstalkCorrection.h"
#include "MModuleChargeSharingCorrection.h"
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
	//! option to correct charge loss or not
	bool m_CorrectCL;

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
	Usage<<"         -c:   correct charge loss" << endl;
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

	m_CorrectCL = false;

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
			m_E0 = atof(argv[++i]);
		}

		if (Option == "-o"){
			m_OutFile = string(argv[++i]);
		}

		if (Option == "-c"){
			m_CorrectCL = true;
		}

	}

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool ChargeLossCorrection::Analyze()
{
	cout << "E0: " << m_E0 << endl;
	cout << "correctCL: " << m_CorrectCL << endl;


/*	TH2D* h2 = new TH2D("fracmap","fracmap",356*2,-356,356,35,356-30,356+5);
//	for (int i=-662; i<662; i++){
//		for (int j=662-30; j<662+5; j++){
	for (int i=0; i<356*2; i++){
		for (int j=0; j<35; j++){
			float c = ((float)i-356)/((float)j+356-30);
			cout << c << endl;
			h2->SetBinContent(i,j,c);
		}
	}
	TCanvas *ctemp = new TCanvas();
	h2->Draw("colz");
	ctemp->Print("frac_map.pdf");
*/	
	//time code just to see
	TStopwatch watch;
	watch.Start();

	if (m_Interrupt == true) return false;

  MSupervisor* S = MSupervisor::GetSupervisor();
  
  MModuleLoaderMeasurementsBinary* Loader = new MModuleLoaderMeasurementsBinary();
//	MModuleLoaderMeasurementsROA* Loader = new MModuleLoaderMeasurementsROA();
  Loader->SetFileName(m_FileName);
	Loader->SetDataSelectionMode(MBinaryFlightDataParserDataModes::c_Raw);
	Loader->SetAspectMode(MBinaryFlightDataParserAspectModes::c_Neither);
	Loader->EnableCoincidenceMerging(true);
  S->SetModule(Loader, 0);
 
  MModuleEnergyCalibrationUniversal* Calibrator = new MModuleEnergyCalibrationUniversal();
  Calibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI16/Wanaka/EnergyCalibration_053018.ecal");
	Calibrator->EnablePreampTempCorrection(false);
  S->SetModule(Calibrator, 1);
  
  MModuleStripPairingGreedy* Pairing = new MModuleStripPairingGreedy();
  S->SetModule(Pairing, 2);

	MModuleCrosstalkCorrection* CrossTalk = new MModuleCrosstalkCorrection();
	CrossTalk->SetFileName("/home/clio/Software/Nuclearizer/resource/calibration/COSI16/Wanaka/CrossTalkCorrection_Results_060518.txt");
	S->SetModule(CrossTalk,3);

	MModuleChargeSharingCorrection* ChargeLoss = new MModuleChargeSharingCorrection();
	S->SetModule(ChargeLoss,4);


  if (Loader->Initialize() == false) return false;
  if (Calibrator->Initialize() == false) return false;
  if (Pairing->Initialize() == false) return false;
	if (CrossTalk->Initialize() == false) return false;
  if (m_CorrectCL){
		if (ChargeLoss->Initialize() == false) return false;
	}


  map<int, TH2D*> Histograms;
 	vector<vector<vector<float> > > crossTalkCoeffs;
	crossTalkCoeffs = LoadCrossTalk(); 

  bool IsFinished = false;
  MReadOutAssembly* Event = new MReadOutAssembly();
  while (IsFinished == false && m_Interrupt == false) {
    Event->Clear();
		if (Loader->IsReady() ){
	    Loader->AnalyzeEvent(Event);
	    Calibrator->AnalyzeEvent(Event);
	    Pairing->AnalyzeEvent(Event);
			CrossTalk->AnalyzeEvent(Event);
			if (m_CorrectCL) {ChargeLoss->AnalyzeEvent(Event);}

			if (Event->HasAnalysisProgress(MAssembly::c_CrosstalkCorrection) == true) {
				vector<int> xStripIDs, yStripIDs;
				vector<float> xEnergy, yEnergy;
				for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
					//variables to add to histograms
					float sum = 0.0;
					float diff = 0.0;
					float scaled_sum = 0.0;
					float frac = 0.0;
					float xE, yE = 0.0;
					int detectorID = 0;
					int histID = -1;
	
					if (Event->GetHit(h)->GetNStripHits()==3){
						for (unsigned int s = 0; s < Event->GetHit(h)->GetNStripHits(); s++){
							detectorID = Event->GetHit(h)->GetStripHit(s)->GetDetectorID();
							if (Event->GetHit(h)->GetStripHit(s)->IsLowVoltageStrip()){
								xStripIDs.push_back(Event->GetHit(h)->GetStripHit(s)->GetStripID());
								xE = Event->GetHit(h)->GetStripHit(s)->GetEnergy();
								xEnergy.push_back(xE);
							}
							else {
								yStripIDs.push_back(Event->GetHit(h)->GetStripHit(s)->GetStripID());
								yE = Event->GetHit(h)->GetStripHit(s)->GetEnergy();
								yEnergy.push_back(yE);
							}
						}
						if (xStripIDs.size() == 2){
							if (fabs(xStripIDs.at(0)-xStripIDs.at(1)) == 1){
								sum = xEnergy.at(0) + xEnergy.at(1);
								if (m_CorrectCL){ sum = Event->GetHit(h)->GetEnergy(); }
								diff = fabs(xEnergy.at(0) - xEnergy.at(1));
								scaled_sum = sum/m_E0;
								frac = diff/sum;
								histID = 10*detectorID;
							}
						}
						else if (yStripIDs.size() == 2){
							if (fabs(yStripIDs.at(0)-yStripIDs.at(1)) == 1){
								sum = yEnergy.at(0) + yEnergy.at(1);
								if (m_CorrectCL){ sum = Event->GetHit(h)->GetEnergy(); }
								diff = fabs(yEnergy.at(0) - yEnergy.at(1));
								scaled_sum = sum/m_E0;
								frac = diff/sum;
								histID = 10*detectorID + 1;
							}
							else {
	//							cout << "not accepted: " << endl;
	//							cout << yStripIDs.at(0) << '\t' << yStripIDs.at(1) << endl;
	//							cout << yEnergy.at(0) << '\t' << yEnergy.at(1) << endl;
							}
						}
		      	if (histID != -1) {
							int nxbins = m_E0*2;
							int nybins = 35;
							int xlowlim = -m_E0;
							int xhighlim = m_E0;
							float ylowlim = (m_E0-30);  //30.0/m_E0;
							float yhighlim = (m_E0+5); //5.0/m_E0;

							if (m_CorrectCL){ yhighlim = (m_E0+30)/m_E0; nybins = 60; }

		      	  if (Histograms[histID] == 0) {
			      	   TH2D* Hist = new TH2D("", "", nxbins,xlowlim,xhighlim,nybins,ylowlim,yhighlim);
								if (histID%10 == 0){
			      	    Hist->SetTitle(MString("Detector ")+detectorID+MString(" p Side"));
									Hist->SetName(MString("Det")+detectorID+MString("Pside"));
								}
								else if (histID%10 == 1){
									Hist->SetTitle(MString("Detector ")+detectorID+MString(" n Side"));
									Hist->SetName(MString("Det")+detectorID+MString("Nside"));
								}
		      	    Hist->SetXTitle("Difference");
		      	    Hist->SetYTitle("Sum [keV]");
		      	    Histograms[histID] = Hist;
		      	  }
//							cout << "frac: " << frac << "    scaled_sum: " << scaled_sum << endl;
							//used to fill these with frac, scaled_sum: switched 190115
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

		} 
    IsFinished = Loader->IsFinished();
	}

	//fit and plot histograms
	//make fit functions

	float DMax = m_E0*((m_E0-511./2)/(m_E0+511./2));

	TF1* highE_fitFunc = new TF1("highE","[1]-[0]*([1]-x)",DMax+0.15*m_E0,m_E0);
//	TF1* highE_fitFunc = new TF1("highE","([1]-[0]*([1]-x))/[1]",(DMax+0.15*m_E0)/m_E0,m_E0);
	highE_fitFunc->FixParameter(1,m_E0);
	TF1* lowE_fitFunc = new TF1("lowE","[1]-([0]/(2*[1]))*([1]*[1]-x*x)",0,m_E0);
//	TF1* lowE_fitFunc = new TF1("lowE","1-([0]/(2*[1]*[1]))*([1]*[1]-x*x)",0,1);
	lowE_fitFunc->FixParameter(1,m_E0);

	//setup output file
	ofstream logFitStats;
	logFitStats.open("ChargeLossCorrection_"+MString(m_E0)+"keV.log");
	logFitStats << "Det" << '\t' << "side" << '\t' << "B" << endl << endl;


	//make map of TProfiles, save histograms
  map<int, TProfile*> Profiles;

	map<int, TH1D*> Projections;

	for (auto H: Histograms){
//		if (H.first == 0){
		TCanvas* C = new TCanvas();
//		C->SetLogz();
		C->cd();
		H.second->Draw("colz");

		int det = H.first/10;
		int side=0;
		if (H.first%10 != 0){ side=1; }
		TFile f(m_OutFile+MString("_Det")+det+MString("_Side")+side+MString("_Hist.root"),"new");
		H.second->Write();
		f.Close();

		TH2D* tempHist = (TH2D*)H.second->Clone();
		double max_val = tempHist->GetMaximum();
		cout << H.second->GetEntries() << '\t' << max_val << endl;
		for (int x=0; x<tempHist->GetNbinsX(); x++){
			for (int y=0; y<tempHist->GetNbinsY(); y++){
				if (tempHist->GetBinContent(x,y) < max_val/2.){
					tempHist->SetBinContent(x,y,0);
					tempHist->SetBinError(x,y,0);
				}
			}
		}
/*
		int nBins = H.second->GetNbinsX();
		for (int b=m_E0+0.05*nBins/2.+1; b<nBins; b+=0.05*nBins/2.){
			float f = H.second->GetXaxis()->GetBinCenter(b);
			cout << b << '\t' << f << endl;
//		if (H.first == 0){
			TH1D* Proj = H.second->ProjectionY(MString("f")+f,b-0.05*nBins,b,"");
			Proj->SetTitle(MString("Frac=")+f);
			Projections[b] = Proj;
		}
		for (auto P: Projections){
			TCanvas* C2 = new TCanvas();
			C2->cd();
			P.second->Draw();
		}

		}
*/
//		}

		TProfile* P = tempHist->ProfileX();
//		TProfile* P = H.second->ProfileX();
		Profiles[H.first] = P;
	}

	//fit Profiles and log fit statistics
	for (auto P: Profiles){
		TCanvas* C = new TCanvas();
		C->cd();

		double param = 0.0;
		if (m_E0 > 300){
			P.second->Fit("highE","R");
			param = highE_fitFunc->GetParameter(0);
		}
		else {
			P.second->Fit("lowE","R");
			param = lowE_fitFunc->GetParameter(0);
		}

//		P.second->Draw();

		int det = P.first/10;
		int side=0;
		if (P.first%10 != 0){ side=1; }

		logFitStats << det << '\t' << side << '\t' << param << endl;

		TFile f(m_OutFile+MString("_Det")+det+MString("_Side")+side+MString("_Profile.root"),"new");
		P.second->Write();
		f.Close();

//		C->Print(m_OutFile+MString("_Det")+det+MString("_Side")+side+MString(".pdf"));
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
