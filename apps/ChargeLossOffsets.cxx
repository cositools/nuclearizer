// run with ChargeLossOffsets -f /data/BackupPalestine... -o isotope_Crosstalk_Offset.txt -e lineenergy



//Still need to make the map < HistID,TH1D > Histograms
//Need to implement the option of only selecting one detector or one side for analysis

////////////////////////////////////////////////////////////////////////////////
/* 
 * ChargeLossOffsets.cxx
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
#include <iomanip>
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
#include <TPaveText.h>

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
#include "MBinaryFlightDataParser.h"
#include "MModuleLoaderMeasurementsBinary.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MModuleStripPairingGreedy.h"
#include "MModuleCrosstalkCorrection.h"
#include "MModuleChargeSharingCorrection.h"
#include "MAssembly.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT which plots the cross-talk histograms for each side of the 12 COSI'14 detectors and will probably eventually fit the histograms to find the cross talk corrections for each.
class ChargeLossOffsets
{
public:
  //! Default constructor
  ChargeLossOffsets();
  //! Default destructor
  ~ChargeLossOffsets();
  
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
	double lineEnergy;
	//! output file names
	MString m_Outfile;
	//!bool about whether to include the charge loss
	bool correctCL;


};



////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ChargeLossOffsets::ChargeLossOffsets() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
ChargeLossOffsets::~ChargeLossOffsets()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool ChargeLossOffsets::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage ChargeLossOffsets <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   .roa input file name"<<endl;
//	Usage<<"         -d:   detector #, if you want to only do one at a time" << endl;
//	Usage<<"         -s:   side can be n or p, again, to only do one at a time" << endl;
	Usage<<"				 -e:   energy of the line to fit in keV"<<endl;
//	Usage<<"         -o:   outfile to save the offsets" << endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;


	lineEnergy = 0.0;
	m_Outfile = "ChargeLossCorrection";
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
	
		if (Option == "-e"){
			lineEnergy = atof(argv[i+1]);
		//} else {
		//cout << "Error: You need to give the line energy!" <<endl;
		//cout<<Usage.str()<<endl;
	//	return false;
		}

		if (Option == "-o"){
			m_Outfile = argv[i+1];
			cout << "Accepting output file name: " << m_Outfile << endl;
		}


		if (Option == "-c"){
			correctCL = atoi(argv[i+1]);
		}
//		else { correctCL = false; }
		cout << "correcting charge loss?" << '\t' << correctCL << endl;

	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool ChargeLossOffsets::Analyze()
{

	//time code just to see
	TStopwatch watch;
	watch.Start();

	if (m_Interrupt == true) return false;

  MSupervisor *S = MSupervisor::GetSupervisor();
  
  MModuleLoaderMeasurementsROA* Loader = new MModuleLoaderMeasurementsROA();
//	MModuleLoaderMeasurementsBinary* Loader = new MModuleLoaderMeasurementsBinary();
  Loader->SetFileName(m_FileName);
//	Loader->SetDataSelectionMode(MBinaryFlightDataParserDataModes::c_Raw);
//	Loader->SetAspectMode(MBinaryFlightDataParserAspectModes::c_Magnetometer);
//	Loader->EnableCoincidenceMerging(true);
  S->SetModule(Loader, 0);
 
  MModuleEnergyCalibrationUniversal* Calibrator = new MModuleEnergyCalibrationUniversal();
//  Calibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI14/EnergyCalibration.ecal");
	Calibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI16/EnergyCalibration.ecal");
  S->SetModule(Calibrator, 1);
  
  MModuleStripPairingGreedy* Pairing = new MModuleStripPairingGreedy();
	//Pairing->SetMode(0);
  S->SetModule(Pairing, 2);

	MModuleCrosstalkCorrection* CrossTalk = new MModuleCrosstalkCorrection();
	CrossTalk->SetFileName("/home/clio/Software/Nuclearizer/resource/calibration/COSI16/Wanaka/CrossTalkCorrection_Results_060716.txt");
	S->SetModule(CrossTalk,3);

	MModuleChargeSharingCorrection* ChargeLoss = new MModuleChargeSharingCorrection();
	S->SetModule(ChargeLoss,4);

  if (Loader->Initialize() == false) return false;
  if (Calibrator->Initialize() == false) return false;
  if (Pairing->Initialize() == false) return false;
	if (CrossTalk->Initialize() == false) return false; 
	if (ChargeLoss->Initialize() == false) return false;

  map<int, TH1D*> Histograms; 
 
  bool IsFinished = false;
  MReadOutAssembly* Event = new MReadOutAssembly();

	while (IsFinished == false && m_Interrupt == false) {
		Event->Clear();
		if (Loader->IsReady() ){
			Loader->AnalyzeEvent(Event);
			Calibrator->AnalyzeEvent(Event);
	 		Pairing->AnalyzeEvent(Event);
//			CrossTalk->AnalyzeEvent(Event);
	
			if (correctCL==1){ChargeLoss->AnalyzeEvent(Event);}
	
//			CrossTalk->AnalyzeEvent(Event);

//			if (Event->HasAnalysisProgress(MAssembly::c_CrosstalkCorrection) == true) {
			if (Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true) {
				int nHits = Event->GetNHits();
				for (int h=0; h<nHits; h++){
					MHit *Hit = Event->GetHit(h);
					//exclude anything w multiple hits per strip
					if (Hit->GetStripHitMultipleTimesX() == false && Hit->GetStripHitMultipleTimesY() == false){
					int nStripHits = Hit->GetNStripHits();
					int nXStripHits = 0, nYStripHits = 0;
	
					int detector = 0;
					int histID = -1;
	
					double hitEnergy = 0.0;
					double xEnergy = 0.0;
					double yEnergy = 0.0;

					vector<int> xStripIDs;
					vector<int> yStripIDs;
	
					for (int sh=0; sh<nStripHits; sh++){
						MStripHit *StripHit = Hit->GetStripHit(sh);
						detector = StripHit->GetDetectorID();
	
						if (StripHit->IsLowVoltageStrip() == true){
							nXStripHits++;
							xEnergy += StripHit->GetEnergy();
							xStripIDs.push_back(StripHit->GetStripID());
						}
						else {
							nYStripHits++;
							yEnergy += StripHit->GetEnergy();
							yStripIDs.push_back(StripHit->GetStripID());
						}
					}
					//get all hits with 2 adj strips on p side
					if (nXStripHits==2 && nYStripHits==1){
//						cout << xEnergy << '\t' << Hit->GetEnergy() << endl;
						if (fabs(xStripIDs[0]-xStripIDs[1])==1){
							histID = 10*detector;
							hitEnergy = xEnergy;
						}
					}
					//get all hits with 2 adj strips on n side
					else if (nXStripHits==1 && nYStripHits==2){
						if (fabs(yStripIDs[0]-yStripIDs[1])==1){
							histID = 10*detector+1;
							hitEnergy = yEnergy;
						}
					}
	
					if (histID != -1){
						if (Histograms[histID] == 0){
	//						TH1D* Hist = new TH1D("","",100,lineEnergy-30,lineEnergy+30);
							TH1D* Hist = new TH1D("","",500,lineEnergy-30,lineEnergy+30);
							Hist->SetXTitle("Energy (keV)");
							Hist->SetYTitle("Counts");
	
							Histograms[histID] = Hist;
						}
						hitEnergy = Hit->GetEnergy();
						Histograms[histID]->Fill(hitEnergy);
					}
				}
			}
		}
		}
  	IsFinished = Loader->IsFinished();
	}

	//create file to save all of the offset numbers in...
/*	ofstream crosstalk_offsets;
	crosstalk_offsets.open(m_OutFile);
	crosstalk_offsets <<LineEnergy<<"\n \n";
*/
  cout << "drawing histograms" << endl; 

  TF1 * f_gausstail = new TF1("f_gausstail","[0]*( [1]/(2*[2])*exp( (x - [3])/[2] + [4]^2/(2*[2]^2) )*(1 - TMath::Erf( (x-[3])/sqrt(2)/[4] + [4]/sqrt(2)/[2] ) ) ) + (1 - [0])*( [1]/(sqrt(2*pi)*[4])*exp( -(x - [3])^2/(2*[4]^2) ) ) + [5]",(lineEnergy-30), (lineEnergy+30));
	f_gausstail->SetParName(0,"eta");
	f_gausstail->SetParName(1,"N");
	f_gausstail->SetParName(2,"lambda");
	f_gausstail->SetParName(3,"mu");
	f_gausstail->SetParName(4,"sigma");
	f_gausstail->SetParName(5,"y-offset");
	f_gausstail->SetParameter(0,0.5);
	f_gausstail->SetParLimits(0,0.15,0.8);
	f_gausstail->SetParameter(1,1000);
	f_gausstail->SetParLimits(1,100,100000);
	f_gausstail->SetParameter(2,10);
	f_gausstail->SetParLimits(2,2,30);
	f_gausstail->SetParameter(3,(lineEnergy+1));
	f_gausstail->SetParLimits(3,(lineEnergy-2),(lineEnergy+25));
	f_gausstail->SetParameter(4,1.5);
	f_gausstail->SetParLimits(4,0.5,6);
	//f_gausstail->SetParameter(5,30);

	//TF1 *f1 = new TF1("f1","gaus", 650,680);


	for (int det=0; det<12; det++){
//  for (auto H: Histograms){

		TCanvas* C = new TCanvas();
		C->SetWindowSize(800,500);
		C->Divide(2);
		for (int side=0; side<2; side++){
			int histID = det*10 + side;
			C->cd(side+1);	

			if (side == 0){
				Histograms[histID]->SetTitle(MString("Detector ")+det+MString(" p side"));
				Histograms[histID]->SetName(MString("Det")+det+MString("Pside"));
			}
			else {
				Histograms[histID]->SetTitle(MString("Detector ")+det+MString(" n side"));
				Histograms[histID]->SetName(MString("Det")+det+MString("Nside"));
			}

			Histograms[histID]->Draw();
//			Histograms[histID]->Fit("f_gausstail","R");

			TFile f(m_Outfile+MString("_Det")+det+MString("_Side")+side+MString("_Spectrum.root"),"new");
			Histograms[histID]->Write();
			f.Close();

			double mean = f_gausstail->GetParameter(3);
			double meanErr = f_gausstail->GetParError(3);
			double fwhm = f_gausstail->GetParameter(4);
			double fwhmErr = f_gausstail->GetParError(4);
			double tailfrac = f_gausstail->GetParameter(0);
			double tailfracErr = f_gausstail->GetParError(0);
			double tailconst = f_gausstail->GetParameter(2);
			double tailconstErr = f_gausstail->GetParError(2);
			double chisq = f_gausstail->GetChisquare();
			int ndof = f_gausstail->GetNDF();

			double meanDiff = mean-lineEnergy;

			//mean and FWHM
			stringstream s_ptText1;
			s_ptText1 << "#mu - E0 = " << setprecision(3) << meanDiff << " +/- " << setprecision(3) << meanErr;
			string ptText1 = s_ptText1.str();
			stringstream s_ptText2;
			s_ptText2 << "FWHM = " << setprecision(3) << fwhm << " +/- " << setprecision(3) << fwhmErr;
			string ptText2 = s_ptText2.str();
			//tailfrac and tailconst
			stringstream s_ptText3;
			s_ptText3 << "tailfrac = " << setprecision(3) << tailfrac << "+/-" << setprecision(3) << tailfracErr;
			string ptText3 = s_ptText3.str();
			stringstream s_ptText4;
			s_ptText4 << "tailconst = " << setprecision(3) << tailconst << "+/-" << setprecision(3) << tailconstErr;
			string ptText4 = s_ptText4.str();
			//chisq / dof
			stringstream s_ptText5;
			s_ptText5 << "#chi^2 / ndof = " << setprecision(3) << chisq << " / " << ndof;
			string ptText5 = s_ptText5.str();

			TPaveText *pt = new TPaveText(0.16,0.82,0.35,0.72,"NDC");
			pt->SetFillColor(kWhite);
			pt->SetBorderSize(0);
			pt->AddText(ptText1.c_str());
			pt->AddText(ptText2.c_str());
			pt->AddText(ptText3.c_str());
			pt->AddText(ptText4.c_str());
			pt->AddText(ptText5.c_str());

			pt->SetTextFont(132);
			pt->Draw();

			C->Update();
		}

		C->Print(m_Outfile+MString("_")+round(lineEnergy)+MString("keV_det")+det+MString(".pdf"));
	//fit the histogram and save the output
	}
	
//	crosstalk_offsets.close();
  watch.Stop();
  cout << "total time (s): " << watch.CpuTime() << endl;
 
  return true;
}



////////////////////////////////////////////////////////////////////////////////


ChargeLossOffsets* g_Prg = 0;
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

  TApplication ChargeLossOffsetsApp("ChargeLossOffsetsApp", 0, 0);

  g_Prg = new ChargeLossOffsets();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  ChargeLossOffsetsApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
