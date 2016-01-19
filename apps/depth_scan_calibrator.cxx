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
#include "MAssembly.h"
#include "MFileEventsSim.h"
#include "MDGeometryQuest.h"
#include "MNCTModuleMeasurementLoaderBinary.h"
#include "MNCTBinaryFlightDataParser.h"

class options{
	public:
		vector<vector<double>*> EnergyWindows;
		bool Use2StripEvents;
		bool Use3StripEvents;
		bool Use4StripEvents;
		bool Use5nsBinning;
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

	/*
	if(argc == 2){
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

	} else if(argc == 3){
		//parse options file
	} else {
		cout << "first parameter is scan file names, second optional parameter is option file, aborting..." <<endl;
		return -1;
	}
	*/

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
	cout << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;



	MFile Filenames; Filenames.Open(MString(argv[1]));
	MString FName;
	MNCTModuleMeasurementLoaderBinary* Loader = new MNCTModuleMeasurementLoaderBinary();
	MNCTModuleEnergyCalibrationUniversal* EnergyCalibrator = new MNCTModuleEnergyCalibrationUniversal();
	EnergyCalibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI16/Berkeley/EnergyCalibration.ecal");

	//define gaussian fit function and set parameter limits
	TF1* GausFit = new TF1("mygaus", "[0]*TMath::Gaus(x,[1],[2],0)", -300.0, +300.0);
	TF1* LorentzFit = new TF1("lorentz", "[0]*TMath::CauchyDist(x,[1],[2])", -300.0, +300.0);
	GausFit->SetParName(0,"amplitude");
	GausFit->SetParName(1,"mean");
	GausFit->SetParName(2,"sigma");
	GausFit->SetParLimits(0, 0.0, 1000.0);
	GausFit->SetParLimits(1, -300.0, +300.0);
	GausFit->SetParLimits(2, 0.0, 80.0);

	//crosstalk...
	while( Filenames.ReadLine(FName) ){
		//set up the nuclearizer modules we need
		Loader->SetFileName(FName);
		Loader->SetDataSelectionMode(MNCTBinaryFlightDataParserDataModes::c_Raw);
		Loader->SetAspectMode(MNCTBinaryFlightDataParserAspectModes::c_Neither);
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
		unsigned int E2 = 0; unsigned int E3 = 0; unsigned int E4 = 0; unsigned int EMisc = 0;

		while( IsFinished == false ){
			Event->Clear();
			if( Loader->IsReady() ){
				Loader->AnalyzeEvent(Event);
				EnergyCalibrator->AnalyzeEvent(Event);
				//crosstalk

				vector<MNCTStripHit*> XSH; vector<MNCTStripHit*> YSH;
				for(size_t i = 0; i < Event->GetNStripHits(); ++i){
					MNCTStripHit* SH = Event->GetStripHit(i);
					if(SH->IsXStrip()) XSH.push_back(SH); else YSH.push_back(SH);
				}

				int event_type = 0;
				double CTD = 0.0;
				int pixel_code = 0xffffffff;

				if( (XSH.size() == 1) && (YSH.size() == 1) ){
					if( Options->Use2StripEvents ){
						MNCTStripHit* X = XSH.at(0);
						MNCTStripHit* Y = YSH.at(0);
						if( EnergyOK(X->GetEnergy(), Y->GetEnergy(), Options) ){
							event_type |= 1;
							CTD = X->GetTiming() - Y->GetTiming();
							pixel_code = 10000*X->GetDetectorID() + 100*X->GetStripID() + Y->GetStripID();
							++E2;
						}
					}
				} else if( ((XSH.size() == 2) && (YSH.size() == 1)) || ((XSH.size() == 1) && (YSH.size() == 2)) ){
					if( Options->Use3StripEvents ){
						vector<MNCTStripHit*> *TwoStripSide, *OneStripSide; 
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
							MNCTStripHit* DominantStrip;
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
							MNCTStripHit *X, *Y;
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
					if( CTDHistograms[pixel_code] == NULL ){
						char name[64]; sprintf(name,"%d",pixel_code);
						TH1D* new_hist;
						if( Options->Use5nsBinning == true ){
							new_hist = new TH1D(name, name, 120,(-300.0)-2.5, (+300.0)-2.5);
						} else {
							new_hist = new TH1D(name, name, 60, (-300.0)-5.0, (+300.0)-5.0);
						}

						CTDHistograms[pixel_code] = new_hist;
					}
					CTDHistograms[pixel_code]->Fill(CTD);
				}
			}

			IsFinished = Loader->IsFinished();

		}

		MString RootFileName = FName.Append(".ctd.root");
		TFile* rootF = new TFile(RootFileName.Data(),"recreate");
		for(auto const &it: CTDHistograms) {
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
			H->Fit("mygaus","Lq");
			H->Fit("lorentz","Lq");
			H->UseCurrentStyle();
			rootF->WriteTObject(it.second);
		}
		rootF->Close();
		cout << "2-strip events: " << E2 << endl;
		cout << "3-strip events: " << E3 << endl;
		cout << "4-strip events: " << E4 << endl;
		cout << "misc. events:   " << EMisc << endl;


	}

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
		}
	}

	return true;
}
