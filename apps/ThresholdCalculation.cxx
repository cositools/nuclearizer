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
#include "MStripHit.h"
#include "MReadOutSequence.h"
#include "MSupervisor.h"
#include "MModuleLoaderMeasurementsROA.h"
#include "MBinaryFlightDataParser.h"
#include "MModuleLoaderMeasurementsBinary.h"
#include "MModuleLoaderSimulationsBalloon.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MModuleStripPairingGreedy.h"
#include "MModuleCrosstalkCorrection.h"
#include "MModuleChargeSharingCorrection.h"
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
	void FSTThresholdsLine(map<int,TH1D*> FSTSpec, map<int,float> &lld_thresholds, map<int,float> &spectrum_kink,MModuleEnergyCalibrationUniversal* Calibrator);
	//! Calculate fast thresholds
	void FSTThresholdsErf(map<int,TH1D*> FSTSpec, map<int,float> &lld_thresholds, map<int,float> &spectrum_kink, MModuleEnergyCalibrationUniversal* Calibrator);
	//! Calculate fast thresholds
	void FSTThresholdsErfFixedMean(map<int,TH1D*> FSTSpec, map<int,TH1D*> LLDSpec, map<int,float> &lld_thresholds, map<int,float> &spectrum_kink, MModuleEnergyCalibrationUniversal* Calibrator);
	//! Calculate fast thresholds
	void FSTThresholdsFastOverTotal(map<int,TH1D*> FSTSpec, map<int,TH1D*> LLDSpec, map<int,float> &lld_thresholds, map<int,float> &spectrum_kink, MModuleEnergyCalibrationUniversal* Calibrator);



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

  MModuleLoaderMeasurementsBinary* Loader = new MModuleLoaderMeasurementsBinary();
  Loader->SetFileName(m_FileName);
	Loader->SetDataSelectionMode(MBinaryFlightDataParserDataModes::c_Raw);
	Loader->SetAspectMode(MBinaryFlightDataParserAspectModes::c_Neither);
	Loader->EnableCoincidenceMerging(true);
  S->SetModule(Loader, 0);

/*
	MModuleLoaderSimulationsBalloon* Loader = new MModuleLoaderSimulationsBalloon();
	Loader->SetSimulationFileName(m_FileName);
	MDGeometryQuest* G = new MDGeometryQuest();
	if (G->ScanSetupFile("~/Software/MassModel/COSI.DetectorHead.geo.setup") == true){
		G->ActivateNoising(false);
		G->SetGlobalFailureRate(0.0);
		Loader->SetGeometry(G);
	}
	Loader->SetEnergyCalibrationFileName("/home/clio/Software/Nuclearizer/resource/calibration/COSI16/Wanaka/EnergyCalibration_053018.ecal");
	Loader->SetThresholdFileName("/volumes/cronus/users/clio/DetectorEffectsEngineTests/Thresholds/thresholds_erf_Run194_FstOverTotal.txt");
//	Loader->SetThresholdFileName("/volumes/cronus/users/clio/DetectorEffectsEngineTests/Thresholds/thresholds_erf_Run194_ADC_AboveZero.txt");
	Loader->SetDeadStripFileName("/home/clio/Software/Nuclearizer/resource/calibration/COSI16/Wanaka/DeadStripList.txt");
	Loader->SetDepthCalibrationCoeffsFileName("/home/clio/Software/Nuclearizer/resource/calibration/COSI16/Berkeley/depth_calibration_coeffs_v2.txt");
	Loader->SetDepthCalibrationSplinesFileName("/home/clio/Software/Nuclearizer/resource/calibration/COSI16/Berkeley/depth_calibration_curves.ctd");
	S->SetModule(Loader,0);
*/
 
  MModuleEnergyCalibrationUniversal* Calibrator = new MModuleEnergyCalibrationUniversal();
  Calibrator->SetFileName("/home/clio/Software/Nuclearizer/resource/calibration/COSI16/Wanaka/EnergyCalibration_053018.ecal");
	Calibrator->EnablePreampTempCorrection(false);
  S->SetModule(Calibrator, 1);

  if (Loader->Initialize() == false) return false;
  if (Calibrator->Initialize() == false) return false;

  map<int, TH1D*> LLDSpec;
	map<int, TH1D*> FSTSpec;
	map<int, TH1D*> LLDSpec2;

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
					bool isPos = Event->GetStripHit(sh)->IsLowVoltageStrip();

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
							TH1D* Hist2 = new TH1D("LLD2_"+MString(identifier),"LLD "+MString(identifier),8192,0,8192);
							LLDSpec[identifier] = Hist;
							LLDSpec2[identifier] = Hist2;
						}
						LLDSpec[identifier]->Fill(adc);
						LLDSpec2[identifier]->Fill(adc);
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
/*	for (auto H: LLDSpec){
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
*/

/*
	for (auto H: FSTSpec){
		char name [17];
		sprintf(name,"fast_%05d.root",H.first);
		TFile f(name,"new");
		H.second->Write();
		f.Close();
	}
*/


	map<int, float> lld_thresholds;
	map<int, float> spectrum_kink;
	LLDThresholds(LLDSpec,lld_thresholds,spectrum_kink);
//	FSTThresholdsLine(FSTSpec,lld_thresholds,spectrum_kink,Calibrator);
//	FSTThresholdsErf(FSTSpec,lld_thresholds,spectrum_kink,Calibrator);
//	FSTThresholdsErfFixedMean(FSTSpec,LLDSpec2,lld_thresholds,spectrum_kink,Calibrator);
	FSTThresholdsFastOverTotal(FSTSpec,LLDSpec2,lld_thresholds,spectrum_kink,Calibrator);


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
				thresh = H.second->GetBinLowEdge(b);
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



void ThresholdCalculation::FSTThresholdsLine(map<int, TH1D*> FSTSpec, map<int, float> &lld_thresholds, map<int, float> &spectrum_kink, MModuleEnergyCalibrationUniversal* Calibrator){

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
		R.IsLowVoltageStrip(H.first % 10);

		//put everything back in energy
		double lld_thresh = Calibrator->GetEnergy(R,lld_thresholds[H.first]);
		double rollover = Calibrator->GetEnergy(R,spectrum_kink[H.first]);
		double slopeEn = Calibrator->GetEnergy(R,slope[H.first]);
		double yIntEn = Calibrator->GetEnergy(R,yInt[H.first]);

		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\n",H.first,lld_thresh,rollover,slopeEn,yIntEn);
	}

	fclose(fp);

}

void ThresholdCalculation::FSTThresholdsErf(map<int, TH1D*> FSTSpec, map<int, float> &lld_thresholds, map<int, float> &spectrum_kink, MModuleEnergyCalibrationUniversal* Calibrator){

	map<int, float> mean;
	map<int, float> sigma;
	map<int, float> constant;

	for (auto H: FSTSpec){
		//set this up for energy / ADC conversions
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsLowVoltageStrip(H.first % 10);

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
	fp = fopen("thresholds_erf.txt","w+");
	for (auto H: FSTSpec){
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsLowVoltageStrip(H.first % 10);

		//put everything back in energy
/*		double lld_thresh = Calibrator->GetEnergy(R,lld_thresholds[H.first]);
//		double rollover = Calibrator->GetEnergy(R,spectrum_kink[H.first]);
		double meanEn = Calibrator->GetEnergy(R,mean[H.first]);
		double sigEn = Calibrator->GetEnergy(R,sigma[H.first]);
*/

		//or leave everything in ADC for the DEE
		double lld_thresh = lld_thresholds[H.first];
		double meanEn = mean[H.first];
		double sigEn = sigma[H.first];

		fprintf(fp,"%05d\t%f\t%f\t%f\n",H.first,lld_thresh,meanEn,sigEn);
//		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\n",H.first,lld_thresholds[H.first],spectrum_kink[H.first],mean[H.first],sigma[H.first]);
	}

	fclose(fp);

}


void ThresholdCalculation::FSTThresholdsErfFixedMean(map<int, TH1D*> FSTSpec, map<int, TH1D*> LLDSpec, map<int, float> &lld_thresholds, map<int, float> &spectrum_kink, MModuleEnergyCalibrationUniversal* Calibrator){

	map<int, float> mean;
	map<int, float> sigma;
	map<int, float> offset;
	map<int, float> amplitude;

	FILE * fp2;
	fp2 = fopen("overlap_start_stop.txt","w+");

	for (auto H: FSTSpec){
		if (LLDSpec[H.first] == 0){
			mean[H.first] = -1;
			continue;
		}
		int start_bin, stop_bin;
		//go between lld threshold and spectrum kink to find first bin where LLD <= FST
		for (int bin=LLDSpec[H.first]->FindBin(lld_thresholds[H.first]); bin<LLDSpec[H.first]->FindBin(spectrum_kink[H.first]); bin++){
			if (H.second->GetBinContent(bin) >= LLDSpec[H.first]->GetBinContent(bin)){
				start_bin = bin;
				break;
			}
		}
		//go the other way to find first bin where LLD >= FST
		for (int bin=LLDSpec[H.first]->FindBin(spectrum_kink[H.first]); bin>LLDSpec[H.first]->FindBin(lld_thresholds[H.first]); bin--){
			if (H.second->GetBinContent(bin) <= LLDSpec[H.first]->GetBinContent(bin)){
				stop_bin = bin;
				break;
			}
		}

		fprintf(fp2,"%05d\t%d\t%d\n",H.first,start_bin,stop_bin);

		if (start_bin == stop_bin){
			mean[H.first] = start_bin;
		}
		else if (start_bin < stop_bin){
			mean[H.first] = start_bin + (stop_bin-start_bin)/2.;
		}
		else {
			cout << "start bin > stop bin: " << H.first << endl;
			mean[H.first] = start_bin;
		}

	}

	fclose(fp2);

	FILE* fp3;
	fp3 = fopen("erf_at_0.txt","w");

	gROOT->SetBatch(kTRUE);
	for (auto H: FSTSpec){
		//set this up for energy / ADC conversions
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsLowVoltageStrip(H.first % 10);

		TF1* erf = new TF1("erf","[0]*(-1*TMath::Erf(([1]-x)/(sqrt(2)*[2]))+1)+[3]",lld_thresholds[H.first],spectrum_kink[H.first]+180);
		erf->SetParameters(40,200,90,10);
		erf->SetParLimits(0,0,1000000);
		erf->SetParLimits(1,lld_thresholds[H.first],spectrum_kink[H.first]);
		erf->SetParLimits(2,0,1000000);
		erf->SetParLimits(3,0,1000000);
/*		if (mean[H.first] != -1){
			erf->FixParameter(1,mean[H.first]);
			erf->FixParameter(2,(spectrum_kink[H.first]-mean[H.first])/sqrt(2));
		}
		else {
			erf->SetParLimits(1,0,spectrum_kink[H.first]);
		}
*/
//		erf->FixParameter(2,Calibrator->GetADC(R,20));
		H.second->Fit("erf","R");
		mean[H.first] = erf->GetParameter(1);
		sigma[H.first] = erf->GetParameter(2);
		offset[H.first] = erf->GetParameter(3);
		amplitude[H.first] = erf->GetParameter(0);

		fprintf(fp3,"%f\n",erf->Eval(0));

		//add TF1 to spectrum file
		char name[20];
		sprintf(name,"lld_fst_%05d.root",H.first);
		TCanvas *c1 = new TCanvas("c1"+MString(H.first));

		H.second->GetXaxis()->SetTitle("ADC");
		H.second->GetYaxis()->SetTitle("Counts");
		H.second->GetXaxis()->SetRangeUser(0,500);
		H.second->Draw();

		if (LLDSpec[H.first] != 0){
			LLDSpec[H.first]->SetLineColor(kGreen);
			LLDSpec[H.first]->GetXaxis()->SetRangeUser(0,500);
			LLDSpec[H.first]->Draw("same");
		}

//		erf->Write();
//		TFile f(name,"new");
		c1->Print(name);
//		f.Close();

		delete erf;
	}
	fclose(fp3);

	//save results in file
	FILE * fp;
	fp = fopen("thresholds_erf.txt","w+");
	for (auto H: FSTSpec){
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsLowVoltageStrip(H.first % 10);

		//or leave everything in ADC for the DEE
		double lld_thresh = lld_thresholds[H.first];
		double meanEn = mean[H.first];
		double sigEn = sigma[H.first];

		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\t%f\t%f\n",H.first,lld_thresh,meanEn,sigEn,offset[H.first],amplitude[H.first],spectrum_kink[H.first]+180);
//		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\n",H.first,lld_thresholds[H.first],spectrum_kink[H.first],mean[H.first],sigma[H.first]);
	}

	fclose(fp);

	//make file in keV just to compare data to simulations
	FILE * fpkeV;
	fpkeV = fopen("thresholds_erf_keV.txt","w+");
	for (auto H: FSTSpec){
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsLowVoltageStrip(H.first % 10);

		//put everything back in energy
		double lld_thresh = Calibrator->GetEnergy(R,lld_thresholds[H.first]);
//		double rollover = Calibrator->GetEnergy(R,spectrum_kink[H.first]);
		double meanEn = Calibrator->GetEnergy(R,mean[H.first]);
		double sigEn = Calibrator->GetEnergy(R,sigma[H.first]);

		fprintf(fpkeV,"%05d\t%f\t%f\t%f\n",H.first,lld_thresh,meanEn,sigEn);
//		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\n",H.first,lld_thresholds[H.first],spectrum_kink[H.first],mean[H.first],sigma[H.first]);
	}

	fclose(fpkeV);

}

void ThresholdCalculation::FSTThresholdsFastOverTotal(map<int, TH1D*> FSTSpec, map<int, TH1D*> LLDSpec, map<int, float> &lld_thresholds, map<int, float> &spectrum_kink, MModuleEnergyCalibrationUniversal* Calibrator){

	map<int, float> mean;
	map<int, float> sigma;
	map<int, float> offset;
	map<int, float> amplitude;

	map<int, TH1D*> FastOverTotal;

	gROOT->SetBatch(kTRUE);
	for (auto H: FSTSpec){
		if (LLDSpec[H.first] != 0){
			TH1D* Hist = new TH1D("FOT_"+MString(H.first),"fast over total "+MString(H.first),8192,0,8192);
			FastOverTotal[H.first] = Hist;
			for (int b=1; b<H.second->GetNbinsX()+1; b++){
				double denom = H.second->GetBinContent(b)+LLDSpec[H.first]->GetBinContent(b);
				double binContent = 0;
				if (denom != 0){
					binContent = H.second->GetBinContent(b)/denom;
				}
				FastOverTotal[H.first]->SetBinContent(b,binContent);
			}
		}

		//set this up for energy / ADC conversions
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsLowVoltageStrip(H.first % 10);

		TF1* erf = new TF1("erf","[0]*(-1*TMath::Erf(([1]-x)/(sqrt(2)*[2]))+1)+[3]",lld_thresholds[H.first],spectrum_kink[H.first]+180);
		erf->SetParameters(1,200,90,0);
		erf->SetParLimits(0,0,1000000);
//		erf->SetParLimits(1,lld_thresholds[H.first],spectrum_kink[H.first]);
		erf->SetParLimits(1,0,1000000);
		erf->SetParLimits(2,0,1000000);
		erf->SetParLimits(3,0,1000000);
/*		if (mean[H.first] != -1){
			erf->FixParameter(1,mean[H.first]);
			erf->FixParameter(2,(spectrum_kink[H.first]-mean[H.first])/sqrt(2));
		}
		else {
			erf->SetParLimits(1,0,spectrum_kink[H.first]);
		}
*/
//		erf->FixParameter(2,Calibrator->GetADC(R,20));

		if (FastOverTotal[H.first] != 0){
			FastOverTotal[H.first]->Fit("erf","R");

			mean[H.first] = erf->GetParameter(1);
			sigma[H.first] = erf->GetParameter(2);
			offset[H.first] = erf->GetParameter(3);
			amplitude[H.first] = erf->GetParameter(0);
		}

		//add TF1 to spectrum file
		char name[20];
		sprintf(name,"lld_fst_%05d.root",H.first);
		TCanvas *c1 = new TCanvas("c1"+MString(H.first));

		H.second->GetXaxis()->SetTitle("ADC");
		H.second->GetYaxis()->SetTitle("Counts");
		H.second->GetXaxis()->SetRangeUser(0,500);
		H.second->Draw();

		if (LLDSpec[H.first] != 0){
			LLDSpec[H.first]->SetLineColor(kGreen);
			LLDSpec[H.first]->GetXaxis()->SetRangeUser(0,500);
			LLDSpec[H.first]->Draw("same");
		}

//		erf->Write();
//		TFile f(name,"new");
		c1->Print(name);
//		f.Close();

		if (FastOverTotal[H.first] != 0){
			TCanvas *c2 = new TCanvas("c2"+MString(H.first));

			FastOverTotal[H.first]->GetXaxis()->SetRangeUser(0,500);
			FastOverTotal[H.first]->GetXaxis()->SetTitle("ADC");
			FastOverTotal[H.first]->GetYaxis()->SetTitle("Counts");
			FastOverTotal[H.first]->Draw();

			char name2[25];
			sprintf(name2,"fastOverTotal_%05d.root",H.first);
			c2->Print(name2);
		}

		delete erf;
	}

	//save results in file
	FILE * fp;
	fp = fopen("thresholds_erf.txt","w+");
	for (auto H: FSTSpec){
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsLowVoltageStrip(H.first % 10);

		//or leave everything in ADC for the DEE
		double lld_thresh = lld_thresholds[H.first];
		double meanEn = mean[H.first];
		double sigEn = sigma[H.first];

		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\t%f\t%f\n",H.first,lld_thresh,meanEn,sigEn,offset[H.first],amplitude[H.first],spectrum_kink[H.first]+180);
//		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\n",H.first,lld_thresholds[H.first],spectrum_kink[H.first],mean[H.first],sigma[H.first]);
	}

	fclose(fp);

	//make file in keV just to compare data to simulations
	FILE * fpkeV;
	fpkeV = fopen("thresholds_erf_keV.txt","w+");
	for (auto H: FSTSpec){
		MReadOutElementDoubleStrip R;
		R.SetDetectorID(H.first / 1000);
		R.SetStripID((H.first % 1000) / 10);
		R.IsLowVoltageStrip(H.first % 10);

		//put everything back in energy
		double lld_thresh = Calibrator->GetEnergy(R,lld_thresholds[H.first]);
//		double rollover = Calibrator->GetEnergy(R,spectrum_kink[H.first]);
		double meanEn = Calibrator->GetEnergy(R,mean[H.first]);
		double sigEn = Calibrator->GetEnergy(R,sigma[H.first]);

		fprintf(fpkeV,"%05d\t%f\t%f\t%f\n",H.first,lld_thresh,meanEn,sigEn);
//		fprintf(fp,"%05d\t%f\t%f\t%f\t%f\n",H.first,lld_thresholds[H.first],spectrum_kink[H.first],mean[H.first],sigma[H.first]);
	}

	fclose(fpkeV);

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
