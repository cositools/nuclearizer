/* 
 * ThresholdCalculation.cxx
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
#include "MNCTStripHit.h"
#include "MReadOutSequence.h"
#include "MSupervisor.h"
#include "MNCTModuleMeasurementLoaderROA.h"
#include "MNCTBinaryFlightDataParser.h"
#include "MNCTModuleMeasurementLoaderBinary.h"
#include "MNCTModuleEnergyCalibrationUniversal.h"
#include "MNCTModuleStripPairingGreedy_b.h"
#include "MNCTModuleCrosstalkCorrection.h"
#include "MNCTModuleChargeSharingCorrection.h"
#include "MAssembly.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class ThresholdCalculation
{
public:
  //! Default constructor
  ThresholdCalculation();
  //! Default destructor
  ~ThresholdCalculation();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
	//! Calculate LLD Thresholds
	void LLDThresholds(map<int,TH1D*> LLDSpec, map<int,float> &lld_thresholds, map<int,float> &spectrum_kink);
	//! Calculate fast thresholds
	void FSTThresholdsLine(map<int,TH1D*> FSTSpec, map<int,float> &lld_thresholds, map<int,float> &spectrum_kink,MNCTModuleEnergyCalibrationUniversal* Calibrator);
	//! Calculate fast thresholds
	void FSTThresholdsErf(map<int,TH1D*> FSTSpec, map<int,float> &lld_thresholds, map<int,float> &spectrum_kink, MNCTModuleEnergyCalibrationUniversal* Calibrator);


private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The input file name
  MString m_FileName;
	//! output file names
	MString m_OutFile;

};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ThresholdCalculation::ThresholdCalculation() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
ThresholdCalculation::~ThresholdCalculation()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool ThresholdCalculation::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ThresholdCalculation <options>"<<endl;
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
    } 

	}

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool ThresholdCalculation::Analyze()
{

	if (m_Interrupt == true) return false;

  MSupervisor* S = MSupervisor::GetSupervisor();
  
//  MNCTModuleMeasurementLoaderBinary* Loader = new MNCTModuleMeasurementLoaderBinary();
	MNCTModuleMeasurementLoaderROA* Loader = new MNCTModuleMeasurementLoaderROA();
  Loader->SetFileName(m_FileName);
//	Loader->SetDataSelectionMode(MNCTBinaryFlightDataParserDataModes::c_Raw);
//	Loader->SetAspectMode(MNCTBinaryFlightDataParserAspectModes::c_Magnetometer);
//	Loader->EnableCoincidenceMerging(true);
  S->SetModule(Loader, 0);
  
  MNCTModuleEnergyCalibrationUniversal* Calibrator = new MNCTModuleEnergyCalibrationUniversal();
  Calibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI16/Wanaka/EnergyCalibration_060719.ecal");
  S->SetModule(Calibrator, 1);

  if (Loader->Initialize() == false) return false;
  if (Calibrator->Initialize() == false) return false;

  map<int, TH1D*> LLDSpec;
	map<int, TH1D*> FSTSpec;

  bool IsFinished = false;
  MReadOutAssembly* Event = new MReadOutAssembly();
  while (IsFinished == false && m_Interrupt == false) {
    Event->Clear();
		if (Loader->IsReady() ){
	    Loader->AnalyzeEvent(Event);
	    Calibrator->AnalyzeEvent(Event);

			if (Event->HasAnalysisProgress(MAssembly::c_EnergyCalibration) == true) {
				for (unsigned int sh=0; sh<Event->GetNStripHits(); sh++){

					int det = Event->GetStripHit(sh)->GetDetectorID();
					int strip = Event->GetStripHit(sh)->GetStripID();
					bool isPos = Event->GetStripHit(sh)->IsPositiveStrip();

					double energy = Event->GetStripHit(sh)->GetEnergy();
					double adc = Event->GetStripHit(sh)->GetADCUnits();
					double timing = Event->GetStripHit(sh)->GetTiming();

					int identifier = det*1000+strip*10;
					if (isPos){ identifier++; }

					//LLD only
					if (timing == 0){
						if (LLDSpec[identifier] == 0){
							//make histogram if it doesn't exist yet
							TH1D* Hist = new TH1D("LLD"+MString(identifier),"LLD "+MString(identifier),8192,0,8192);
							LLDSpec[identifier] = Hist;
						}
						LLDSpec[identifier]->Fill(adc);
					}
					//fast
					else{
						if (FSTSpec[identifier] == 0){
							//make histogram if it doesn't exist yet
							TH1D* Hist = new TH1D("FST"+MString(identifier),"FST "+MString(identifier),8192,0,8192);
							FSTSpec[identifier] = Hist;
						}
						FSTSpec[identifier]->Fill(adc);
					}
				}
			}
		} 
    IsFinished = Loader->IsFinished();
	}


	//save spectra
	for (auto H: LLDSpec){
//	for (auto H: FSTSpec){
		char name[16];
		sprintf(name,"lld_%05d.root",H.first);
		TFile f(name,"new");
		H.second->SetLineColor(kGreen);
		H.second->GetXaxis()->SetRangeUser(0,500);
		H.second->Write();
		if (FSTSpec[H.first] != 0){
			FSTSpec[H.first]->GetXaxis()->SetRangeUser(0,500);
			FSTSpec[H.first]->Write();
		}
		f.Close();
	}


/*
	for (auto H: FSTSpec){
		char name [17];
		sprintf(name,"fast_%05d.root",H.first);
		TFile f(name,"new");
		H.second->Write();
		f.Close();
	}
*/

/*
	map<int, float> lld_thresholds;
	map<int, float> spectrum_kink;
	LLDThresholds(LLDSpec,lld_thresholds,spectrum_kink);
//	FSTThresholdsLine(FSTSpec,lld_thresholds,spectrum_kink,Calibrator);
	FSTThresholdsErf(FSTSpec,lld_thresholds,spectrum_kink,Calibrator);
*/

	cout << "all spectra analyzed and saved" << endl;

  return true;
}


//////////////////////////////////////////////////////////////////////////////////

void ThresholdCalculation::LLDThresholds(map<int, TH1D*> LLDSpec, map<int, float> &lld_thresholds, map<int, float> &spectrum_kink){

	//first find threshold
	float thresh = 0;
//	map<int, float> lld_thresholds;

	for (auto H: LLDSpec){
		for (int b=0; b<H.second->GetNbinsX(); b++){
			if (H.second->GetBinContent(b) != 0){
				thresh = H.second->GetBinCenter(b);
				break;
			}
		}
		lld_thresholds[H.first] = thresh;
	}

	//then find point where LLD spectrum dies off

	for (auto H: LLDSpec){
		//limit the fit range like this to just fit the slope of the LLD spectrum and not overcount
		// LLD counts after the kink
		TF1* line = new TF1("line","[0]*x+[1]",lld_thresholds[H.first]+50,lld_thresholds[H.first]+200);
		line->SetParLimits(0,-100000,0);
		H.second->Fit("line","RQ");

		//calculate x intercept
		float slope = line->GetParameter(0);
		float yInt = line->GetParameter(1);
		float xInt = -(yInt/slope);

		spectrum_kink[H.first] = xInt;

		delete line;
	}

}



void ThresholdCalculation::FSTThresholdsLine(map<int, TH1D*> FSTSpec, map<int, float> &lld_thresholds, map<int, float> &spectrum_kink, MNCTModuleEnergyCalibrationUniversal* Calibrator){

	map<int, float> slope;
	map<int, float> yInt;


	for (auto H: FSTSpec){

		TF1* line = new TF1("line","[0]*x+[1]",lld_thresholds[H.first],spectrum_kink[H.first]);
		H.second->Fit("line","R");
		slope[H.first] = line->GetParameter(0)/H.second->GetBinContent(H.second->FindBin(spectrum_kink[H.first]));
		yInt[H.first] = line->GetParameter(1)/H.second->GetBinContent(H.second->FindBin(spectrum_kink[H.first]));

		//add TF1 to spectrum file
		char name[16];
		sprintf(name,"fst_%05d.root",H.first);
		TFile f(name,"new");
		H.second->Write();
		line->Write();
		f.Close();

		delete line;
	}

	//save results in file
	FILE * fp;
	fp = fopen("thresholds_line.txt","w+");
	for (auto H: FSTSpec){
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsPositiveStrip(H.first % 10);

		//put everything back in energy
		double lld_thresh = Calibrator->GetEnergy(R,lld_thresholds[H.first]);
		double rollover = Calibrator->GetEnergy(R,spectrum_kink[H.first]);
		double slopeEn = Calibrator->GetEnergy(R,slope[H.first]);
		double yIntEn = Calibrator->GetEnergy(R,yInt[H.first]);

		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\n",H.first,lld_thresh,rollover,slopeEn,yIntEn);
	}

	fclose(fp);

}

void ThresholdCalculation::FSTThresholdsErf(map<int, TH1D*> FSTSpec, map<int, float> &lld_thresholds, map<int, float> &spectrum_kink, MNCTModuleEnergyCalibrationUniversal* Calibrator){

	map<int, float> mean;
	map<int, float> sigma;
	map<int, float> constant;

	for (auto H: FSTSpec){
		//set this up for energy / ADC conversions
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsPositiveStrip(H.first % 10);

		TF1* erf = new TF1("erf","[0]*TMath::Erf(([1]-x)/(sqrt(2)*[2]))+[3]",lld_thresholds[H.first],spectrum_kink[H.first]+180);
		erf->SetParameters(-40,200,90,10);
		erf->SetParLimits(1,0,spectrum_kink[H.first]);

//		erf->FixParameter(2,Calibrator->GetADC(R,20));
		H.second->Fit("erf","R");
		mean[H.first] = erf->GetParameter(1);
		sigma[H.first] = erf->GetParameter(2);

		//add TF1 to spectrum file
		char name[16];
		sprintf(name,"fst_%05d.root",H.first);
		TFile f(name,"new");
		H.second->Write();
		H.second->GetXaxis()->SetRangeUser(0,Calibrator->GetADC(R,100));
		erf->Write();
		f.Close();

		delete erf;
	}

	//save results in file
	FILE * fp;
	fp = fopen("thresholds_erf_sim.txt","w+");
	for (auto H: FSTSpec){
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsPositiveStrip(H.first % 10);

		//put everything back in energy
		double lld_thresh = Calibrator->GetEnergy(R,lld_thresholds[H.first]);
//		double rollover = Calibrator->GetEnergy(R,spectrum_kink[H.first]);
		double meanEn = Calibrator->GetEnergy(R,mean[H.first]);
		double sigEn = Calibrator->GetEnergy(R,sigma[H.first]);

		fprintf(fp,"%05d\t%f\t%f\t%f\n",H.first,lld_thresh,meanEn,sigEn);
//		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\n",H.first,lld_thresholds[H.first],spectrum_kink[H.first],mean[H.first],sigma[H.first]);
	}

	fclose(fp);

}

////////////////////////////////////////////////////////////////////////////////



ThresholdCalculation* g_Prg = 0;
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

  TApplication ThresholdCalculationApp("ThresholdCalculationApp", 0, 0);

  g_Prg = new ThresholdCalculation();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  ThresholdCalculationApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
