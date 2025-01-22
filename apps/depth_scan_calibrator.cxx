//plan:
//-input file contains filenames for scans
//-for each input file, get the position and integration time from filename
//-open file, loop through readout assemblys
//-don't need strip pairing
//-DO probably want cross-talk correction
//-check for number of strip hits on each side, for use 1x1 and also maybe 1x2 and 2x1 if the sharing isn't strong

// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <unistd.h>
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
#include <TSpline.h>
#include <TRandom.h>
#include <TMultiGraph.h>
#include <TPaveStats.h>
#include <TGraphErrors.h>

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
#include "MFileEventsSim.h"
#include "MDGeometryQuest.h"
#include "MModuleLoaderMeasurementsBinary.h"
#include "MBinaryFlightDataParser.h"
#include "MFitFunctions.h"

class options{
	public:
		vector<vector<double>*> EnergyWindows;
		vector<int> GraphPixel;
		bool Use2StripEvents;
		bool Use3StripEvents;
		bool Use4StripEvents;
		bool Use5nsBinning;
		unordered_map<int,bool> UseDetectors;
};


//prototypes:
bool EnergyOK(double XE, double YE, options* Op);
bool ParseOptions(MString OptionsFileName, options* Op);

int main(int argc, char * argv[]){


	//arg1 is file containing filenames
	//if arg2 is there, it is options file

	MGlobal::Initialize("Standalone","");
	TApplication dcalscan("dcalscan",0,0);
	options* Options = new options();
	gStyle->SetOptFit(1);
	gROOT->SetBatch(kTRUE);

	bool UseDefaults = false;
	if(argc == 2){
		UseDefaults = true;
	} else if(argc == 3){
		bool ParseSuccess = ParseOptions(MString(argv[2]), Options);
		if(ParseSuccess == false){
			UseDefaults = true;
			delete Options;
			Options = new options();
		}
	} else {
		cout << "first parameter is scan file names, second optional parameter is option file, aborting..." <<endl;
		return -1;
	}

	if(UseDefaults){
		//use default options
		vector<double>* v = new vector<double>();
		v->push_back(350.0); v->push_back(477.0);
		Options->EnergyWindows.push_back(v);
		v = new vector<double>();
		v->push_back(656.0); v->push_back(668.0);
		Options->EnergyWindows.push_back(v);

		Options->Use2StripEvents = true;
		Options->Use3StripEvents = true;
		Options->Use4StripEvents = true;
		Options->Use5nsBinning = false;
		for(int i = 0; i < 12; ++i){
			Options->UseDetectors[i] = true;
		}
	}

	cout << boolalpha << endl;
	cout << ">>>>>>>>>>>>>>>>>>Options>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	cout << "Use2StripEvents " << Options->Use2StripEvents << endl;
	cout << "Use3StripEvents " << Options->Use3StripEvents << endl;
	cout << "Use4StripEvents " << Options->Use4StripEvents << endl;
	cout << "Use5nsBinning " << Options->Use5nsBinning << endl;
	for(const auto v: Options->EnergyWindows){
		cout << "Energy window: " << v->at(0) << " to " << v->at(1) << " keV" << endl;
	}
	for(const auto d: Options->UseDetectors){
		cout << "Use Detector: " << d.first << endl;
	}
	for(const auto i: Options->GraphPixel){
		cout << "Make graph for pixel: " << i << endl;
	}
	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;



	MFile Filenames; Filenames.Open(MString(argv[1]));
	MString FName;
	MModuleLoaderMeasurementsBinary* Loader = new MModuleLoaderMeasurementsBinary();
	MModuleEnergyCalibrationUniversal* EnergyCalibrator = new MModuleEnergyCalibrationUniversal();
	EnergyCalibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI16/Berkeley/EnergyCalibration.ecal");

	//define gaussian fit function and set parameter limits
	TF1* GausFit = new TF1("mygaus", "[0]*TMath::Gaus(x,[1],[2],0)", -300.0, +300.0);
	TF1* LorentzFit = new TF1("lorentz", "[0]*TMath::CauchyDist(x,[1],[2])", -300.0, +300.0);
	GausFit->SetParName(0,"amplitude");
	GausFit->SetParName(1,"mean");
	GausFit->SetParName(2,"sigma");
	GausFit->SetParLimits(0, 0.0, 10000.0);
	GausFit->SetParLimits(1, -300.0, +300.0);
	GausFit->SetParLimits(2, 0.0, 100.0);
	LorentzFit->SetParName(0,"amplitude");
	LorentzFit->SetParName(1,"mean");
	LorentzFit->SetParName(2,"sigma");
	LorentzFit->SetParLimits(1,-300.0,+300.0);
	LorentzFit->SetParLimits(2,-300.0,+300.0);

	TF1* AsymmetricalGauss = new TF1("asymgaus",AsymGaus,-300,+300,5);
	AsymmetricalGauss->SetParName(0,"offset");
	AsymmetricalGauss->SetParName(1,"height");
	AsymmetricalGauss->SetParName(2,"mean");
	AsymmetricalGauss->SetParName(3,"left-sigma");
	AsymmetricalGauss->SetParName(4,"right-sigma");

	vector<map<int,vector<double>*>*> ParMaps;

	//crosstalk...
	while( Filenames.ReadLine(FName) ){
		//set up the nuclearizer modules we need
		Loader->SetFileName(FName);
		Loader->SetDataSelectionMode(MBinaryFlightDataParserDataModes::c_Raw);
		Loader->SetAspectMode(MBinaryFlightDataParserAspectModes::c_Neither);
		Loader->EnableCoincidenceMerging(false);

		cout << "initializing modules for FName = " << FName.GetString() << "... ";
		if( Loader->Initialize() == false){
			cout << "error initializing data loader... continuing" << endl;
			continue;
		}
		if( EnergyCalibrator->Initialize() == false){
			cout << "error initializing energy calibrator... continuing" << endl;
			continue;
		}
		cout << "done" << endl;

		MReadOutAssembly* Event = new MReadOutAssembly();
		bool IsFinished = false;
		map<int, TH1D*> CTDHistograms;
		map<int,vector<double>*>* FitParams = new map<int,vector<double>*>();

		unsigned int E2 = 0; unsigned int E3 = 0; unsigned int E4 = 0; unsigned int EMisc = 0;

		while( IsFinished == false ){
			Event->Clear();
			if( Loader->IsReady() ){
				Loader->AnalyzeEvent(Event);
				EnergyCalibrator->AnalyzeEvent(Event);
				//crosstalk

				vector<MStripHit*> XSH; vector<MStripHit*> YSH;
				for(size_t i = 0; i < Event->GetNStripHits(); ++i){
					MStripHit* SH = Event->GetStripHit(i);
					if(SH->IsLowVoltageStrip()) XSH.push_back(SH); else YSH.push_back(SH);
				}

				int event_type = 0;
				double CTD = 0.0;
				int pixel_code = 0xffffffff;

				if( (XSH.size() == 1) && (YSH.size() == 1) ){
					if( Options->Use2StripEvents ){
						MStripHit* X = XSH.at(0);
						MStripHit* Y = YSH.at(0);
						if( EnergyOK(X->GetEnergy(), Y->GetEnergy(), Options) ){
							event_type |= 1;
							CTD = X->GetTiming() - Y->GetTiming();
							pixel_code = 10000*X->GetDetectorID() + 100*X->GetStripID() + Y->GetStripID();
							++E2;
						}
					}
				} else if( ((XSH.size() == 2) && (YSH.size() == 1)) || ((XSH.size() == 1) && (YSH.size() == 2)) ){
					if( Options->Use3StripEvents ){
						vector<MStripHit*> *TwoStripSide, *OneStripSide; 
						bool EnergyIsOk = false;

						if(XSH.size() == 2){
							TwoStripSide = &XSH;
							OneStripSide = &YSH;
							EnergyIsOk = EnergyOK(XSH.at(0)->GetEnergy() + XSH.at(1)->GetEnergy(), YSH.at(0)->GetEnergy(), Options);
						} else {
							TwoStripSide = &YSH;
							OneStripSide = &XSH;
							EnergyIsOk = EnergyOK(XSH.at(0)->GetEnergy(), YSH.at(0)->GetEnergy() + YSH.at(1)->GetEnergy(), Options);
						}

						if( EnergyIsOk && (abs(TwoStripSide->at(0)->GetStripID() - TwoStripSide->at(1)->GetStripID()) == 1) ){
							MStripHit* DominantStrip;
							if(TwoStripSide->at(0)->GetEnergy() >= TwoStripSide->at(1)->GetEnergy()) DominantStrip = TwoStripSide->at(0); else DominantStrip = TwoStripSide->at(1);
							event_type |= 2;
							if(TwoStripSide == &XSH){
								pixel_code = 10000*DominantStrip->GetDetectorID() + 100*DominantStrip->GetStripID() + OneStripSide->at(0)->GetStripID();
								CTD = DominantStrip->GetTiming() - OneStripSide->at(0)->GetTiming();
							} else {
								pixel_code = 10000*DominantStrip->GetDetectorID() + 100*OneStripSide->at(0)->GetStripID() + DominantStrip->GetStripID();
								CTD = OneStripSide->at(0)->GetTiming() - DominantStrip->GetTiming();
							}
							++E3;
						}
					}
				} else if( (XSH.size() == 2) && (YSH.size() == 2) ){
					if( Options->Use4StripEvents ){
						bool XNeighbors = abs(XSH.at(0)->GetStripID() - XSH.at(1)->GetStripID()) == 1;
						bool YNeighbors = abs(YSH.at(0)->GetStripID() - YSH.at(1)->GetStripID()) == 1;
						bool EnergyIsOk = EnergyOK(XSH.at(0)->GetEnergy() + XSH.at(1)->GetEnergy(), YSH.at(0)->GetEnergy() + YSH.at(1)->GetEnergy(), Options);
						if( XNeighbors && YNeighbors && EnergyIsOk ){
							MStripHit *X, *Y;
							if(XSH.at(0)->GetEnergy() >= XSH.at(1)->GetEnergy()) X = XSH.at(0); else X = XSH.at(1);
							if(YSH.at(0)->GetEnergy() >= YSH.at(1)->GetEnergy()) Y = YSH.at(0); else Y = YSH.at(1);
							event_type |= 4;
							CTD = X->GetTiming() - Y->GetTiming();
							pixel_code = 10000*X->GetDetectorID() + 100*X->GetStripID() + Y->GetStripID();
							++E4;
						}
					}
				} else {
					++EMisc;
				}


				if(event_type > 0){
					if( Options->UseDetectors.count(pixel_code/10000) ){
						if( CTDHistograms.count(pixel_code) == 0 ){
							char name[64]; sprintf(name,"%d",pixel_code);
							TH1D* new_hist;
							if( Options->Use5nsBinning == true ){
								new_hist = new TH1D(name, name, 120,(-300.0)-2.5, (+300.0)-2.5);
							} else {
								//new_hist = new TH1D(name, name, 60, (-300.0)-5.0, (+300.0)-5.0);
								new_hist = new TH1D(name, name, 37, (-230.0)-5.0, (+140.0)-5.0);
							}

							CTDHistograms[pixel_code] = new_hist;
						}
						CTDHistograms[pixel_code]->Fill(CTD);
					}
				}
			}

			IsFinished = Loader->IsFinished();

		}

		MString RootFileName = FName.Append(".ctd.root");
		TFile* rootF = new TFile(RootFileName.Data(),"recreate");
		for(auto const &it: CTDHistograms) {
			//vector<double>* V = new vector<double>(6);
			vector<double>* V = new vector<double>(6);
			TH1D* H = it.second;
			double Maximum = H->GetMaximum();
			double Mean = H->GetMean();
			double Sigma = H->GetMeanError();
			GausFit->SetParameter(0,Maximum);
			GausFit->SetParameter(1,Mean);
			GausFit->SetParameter(2,Sigma);
			LorentzFit->SetParameter(0,Maximum);
			LorentzFit->SetParameter(1,Mean);
			LorentzFit->SetParameter(2,Sigma);
			AsymmetricalGauss->SetParLimits(0,0.0, Maximum);
			AsymmetricalGauss->SetParLimits(1,0.0, 1.2*Maximum);
			AsymmetricalGauss->SetParLimits(2,-300.0,+300.0);
			AsymmetricalGauss->SetParLimits(3,1, 100);
			AsymmetricalGauss->SetParLimits(4,1,100);
			AsymmetricalGauss->SetParameter(0,0.0);
			AsymmetricalGauss->SetParameter(1,Maximum);
			AsymmetricalGauss->SetParameter(2,Mean);
			AsymmetricalGauss->SetParameter(3,Sigma);
			AsymmetricalGauss->SetParameter(4,Sigma);
			//H->Fit(GausFit,"Lq");
			//H->Fit(LorentzFit,"Lq");
			H->Fit(AsymmetricalGauss,"Lq");
			H->UseCurrentStyle();
			(*V)[0] = LorentzFit->GetParameter(1); (*V)[1] = LorentzFit->GetParError(1);
			//(*V)[2] = GausFit->GetParameter(1); (*V)[3] = GausFit->GetParError(1);
			(*V)[4] = Mean; (*V)[5] = Sigma;
			(*V)[2] = AsymmetricalGauss->GetParameter(2); (*V)[3] = AsymmetricalGauss->GetParError(2);
			(*FitParams)[it.first] = V;
			rootF->WriteTObject(it.second);
			delete H;
		}
		rootF->Close();
		cout << "2-strip events: " << E2 << endl;
		cout << "3-strip events: " << E3 << endl;
		cout << "4-strip events: " << E4 << endl;
		cout << "misc. events:   " << EMisc << endl;
		ParMaps.push_back(FitParams);


	}

	TFile* rootF = new TFile("pixel_plots.root","recreate");
	for(int det = 0; det < 12; ++det){
		for(int xch = 1; xch < 38; ++xch){
			for(int ych = 1; ych < 38; ++ych){
				int pixel = 10000*det + 100*xch + ych;
				bool HaveData = false;
				for(auto const parmap: ParMaps){
					if((*parmap).count(pixel)){
						HaveData = true;
						break;
					}
				} 
				if(HaveData){
					//make the graph
					vector<double> X;
					vector<double> Y_lorentz, Y_gauss, Y_standard;
					vector<double> EX;
					vector<double> EY_lorentz, EY_gauss, EY_standard;
					int x_ind = 0;
					for(auto const parmap: ParMaps){
						if((*parmap).count(pixel)){
							vector<double>* values = (*parmap)[pixel];
							X.push_back((double)x_ind); ++x_ind;
							EX.push_back(0.0);
							Y_lorentz.push_back((*values)[0]);
							EY_lorentz.push_back((*values)[1]);
							Y_gauss.push_back((*values)[2]);
							EY_gauss.push_back((*values)[3]);
							Y_standard.push_back((*values)[4]);
							EY_standard.push_back((*values)[5]);
						} 
					}
					char name[64]; sprintf(name, "%d", pixel);
					TCanvas* tc = new TCanvas(name,name,400,400);
					//tc->Divide(1,3);
					TMultiGraph* mg = new TMultiGraph();

					/*
					sprintf(name,"%d-lorentz",pixel);
					TGraphErrors* tg_lorentz = new TGraphErrors(X.size(), (double *) &X[0], (double *) &Y_lorentz[0], (double *) &EX[0], (double *) &EY_lorentz[0]);
					tg_lorentz->SetLineColor(kRed);
					tg_lorentz->SetMarkerStyle(21);
					tg_lorentz->SetTitle(name);
					//tg_lorentz->Draw("ALP");
					mg->Add(tg_lorentz);
					*/

					sprintf(name,"%d-gauss",pixel);
					TGraphErrors* tg_gauss = new TGraphErrors(X.size(), (double *) &X[0], (double *) &Y_gauss[0], (double *) &EX[0], (double *) &EY_gauss[0]);
					tg_gauss->SetLineColor(kBlue);
					tg_gauss->SetMarkerStyle(21);
					tg_gauss->SetTitle(name);
					//tg_gauss->Draw("ALP");
					mg->Add(tg_gauss);

					/*
					sprintf(name,"%d-standard",pixel);
					TGraphErrors* tg_standard = new TGraphErrors(X.size(), (double *) &X[0], (double *) &Y_standard[0], (double *) &EX[0], (double *) &EY_standard[0]);
					tg_standard->SetMarkerStyle(21);
					tg_standard->SetTitle(name);
					//tg_standard->Draw("ALP");
					mg->Add(tg_standard);
					*/

					mg->Draw("ALP");
					tc->BuildLegend(0.2,0.67,0.45,0.88);
					tc->Update();
					rootF->WriteTObject(tc);
				} else {
					continue;
				}
			}
		}
	}
	rootF->Close();

	return 0;

}

bool EnergyOK(double XE, double YE, options* Op){
	if( fabs(XE - YE) > 30.0 ){
		return false;
	}

	double AvgEnergy = (XE + YE)/2.0;

	for(auto EW: Op->EnergyWindows){
		if( (AvgEnergy >= EW->at(0)) && (AvgEnergy <= EW->at(1)) ){
			return true;
		}
	}

	return false;

}

bool ParseOptions(MString OptionsFileName, options* Options){

	MFile OptionsFile;
	bool IsOpen = OptionsFile.Open(OptionsFileName);

	if(IsOpen == false){
		return false;
	}

	MString Line;
	while( OptionsFile.ReadLine(Line) ){
		if( Line.BeginsWith("Use2StripEvents") ){
			if( Line.Contains("true") ){
				Options->Use2StripEvents = true;
			} else {
				Options->Use2StripEvents = false;
			}
		} else if( Line.BeginsWith("Use3StripEvents") ){
			if( Line.Contains("true") ){
				Options->Use3StripEvents = true;
			} else {
				Options->Use3StripEvents = false;
			}
		} else if( Line.BeginsWith("Use4StripEvents") ){
			if( Line.Contains("true") ){
				Options->Use4StripEvents = true;
			} else {
				Options->Use4StripEvents = false;
			}
		} else if( Line.BeginsWith("Use5nsBinning") ){
			if( Line.Contains("true") ){
				Options->Use5nsBinning = true;
			} else {
				Options->Use5nsBinning = false;
			}
		} else if( Line.BeginsWith("EnergyWindow") ){
			//add an energy window
			vector<MString> Tokens = Line.Tokenize(" ");
			if( Tokens.size() == 3){
				vector<double>* v = new vector<double>();
				v->push_back(Tokens[1].ToDouble());
				v->push_back(Tokens[2].ToDouble());
				Options->EnergyWindows.push_back(v);
			} else {
				cout << "ParseOptions(): error parsing energy window, ignoring..." << endl;
			}
		} else if( Line.BeginsWith("UseDetector") ){
			vector<MString> Tokens = Line.Tokenize(" ");
			if(Tokens.size() == 2){
				int DetectorID = Tokens[1].ToInt();
				if( (DetectorID >= 0) && (DetectorID < 12) ){
					Options->UseDetectors[DetectorID] = true;
				} else {
					cout << "ParseOptions(): invalid detector ID: " << DetectorID << " ignoring..." << endl;
				}
			} else {
				cout << "ParseOptions(): parsing error for option UseDetector, ignoring..." << endl;
			}
		} else if( Line.BeginsWith("GraphPixel")){
			vector<MString> Tokens = Line.Tokenize(" ");
			if(Tokens.size() == 2){
				int Pixel = Tokens[1].ToInt();
				Options->GraphPixel.push_back(Pixel);
			}
		}
	}

	return true;
}

