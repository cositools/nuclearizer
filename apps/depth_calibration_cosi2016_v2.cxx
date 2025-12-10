// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <unistd.h>
#include <signal.h>
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
#include "MDepthCalibrator.h"
#include "MModuleLoaderMeasurementsBinary.h"
#include "MBinaryFlightDataParser.h"

class MDEEStripHit
{
	public:
		//! The read-out element
		MReadOutElementDoubleStrip m_ROE;
		//! The ADC value
		double m_ADC;
		//! The timing value;
		double m_Timing;

		//! The simulated position
		MVector m_Position;
		//! The simulated energy deposit
		double m_Energy;

		//! True if this is a guard ring
		bool m_IsGuardRing;

		//! A list of original strip hits making up this strip hit
		vector<MDEEStripHit> m_SubStripHits;
};

class MStripHit_s : public MStripHit
{
	//use this derived class instead of MStripHit so that we can keep track of the depth 
	//from the simulation data

	public:
		void SetDepth(double Depth){ m_Depth = Depth; }
		double GetDepth(void){ return m_Depth; }
	protected:
		double m_Depth;
};

class Options
{
	public:
		vector<vector<double>> EnergyWindows;
		MString RawDataFilename;
		MString RawDataOutputFilename;
		MString SimulationFilename;
		MString SimulationOutputFilename;
		MString SplineFilename;
		MString GeometryFilename;
		MString EnergyCalibrationFilename;
		MString FitDataFile;
		MString FitSimFile;
		bool ProcessRealData;
		bool ProcessSimData;
		bool PerformFits;
		bool Use5nsBinning;
		int Divisions;
		int FitRebinSimCTD;
		int FitRebinDataCTD;
		Options(void){
			EnergyCalibrationFilename = "$(NUCLEARIZER)/resource/calibration/COSI16/Berkeley/EnergyCalibration.ecal";
			RawDataOutputFilename = "data_ctd.root";
			SimulationOutputFilename = "sim_data.root";
			FitDataFile = "data_ctd.root";
			FitSimFile = "sim_data.root";
			ProcessRealData = true;
			ProcessSimData = true;
			PerformFits = true;
			Use5nsBinning = true;
			FitRebinSimCTD = 0;
			FitRebinDataCTD = 0;
			Divisions = 1;
			return;
		}
		bool ParseOptionsFile(MString fname){
			MFile F;
			if(!F.Open(fname)){
				return false;
			} else {
				MString line;
				while(F.ReadLine(line)){
					vector<MString> Tokens = line.Tokenize(" ");
					if(line.BeginsWith("RawDataFilename")){
						if(Tokens.size() == 2) RawDataFilename = Tokens[1];
					} else if(line.BeginsWith("SimulationFilename")){
						if(Tokens.size() == 2) SimulationFilename = Tokens[1];
					} else if(line.BeginsWith("SplineFilename")){
						if(Tokens.size() == 2) SplineFilename = Tokens[1];
					} else if(line.BeginsWith("GeometryFilename")){
					  	if(Tokens.size() == 2) GeometryFilename = Tokens[1];	
					} else if(line.BeginsWith("ProcessRealData")){
						if(Tokens.size() == 2) ProcessRealData = Tokens[1].ToLower() == "true" ? true : false;
					} else if(line.BeginsWith("ProcessSimData")){
						if(Tokens.size() == 2) ProcessSimData = Tokens[1].ToLower() == "true" ? true : false;
					} else if(line.BeginsWith("PerformFits")){
						if(Tokens.size() == 2) PerformFits = Tokens[1].ToLower() == "true" ? true : false;
					} else if(line.BeginsWith("Use5nsBinning")){
						if(Tokens.size() == 2) Use5nsBinning = Tokens[1].ToLower() == "true" ? true : false;
					} else if(line.BeginsWith("EnergyWindow")){
						if(Tokens.size() == 3){
							vector<double> E;
							E.push_back(Tokens[1].ToDouble());
							E.push_back(Tokens[2].ToDouble());
							EnergyWindows.push_back(E);
						}
					} else if(line.BeginsWith("Divisions")){
						if(Tokens.size() == 2) Divisions = Tokens[1].ToInt();
					} else if(line.BeginsWith("EnergyCalibrationFilename")){
						if(Tokens.size() == 2) EnergyCalibrationFilename = Tokens[1];
					} else if(line.BeginsWith("SimulationOutputFilename")){
						if(Tokens.size() == 2) SimulationOutputFilename = Tokens[1];
					} else if(line.BeginsWith("RawDataOutputFilename")){
						if(Tokens.size() == 2) RawDataOutputFilename = Tokens[1];
					} else if(line.BeginsWith("FitDataFile")){
						if(Tokens.size() == 2) FitDataFile = Tokens[1];
					} else if(line.BeginsWith("FitSimFile")){
						if(Tokens.size() == 2) FitSimFile = Tokens[1];
					} else if(line.BeginsWith("FitRebinSimCTD")){
						if(Tokens.size() == 2) FitRebinSimCTD = Tokens[1].ToInt();
					} else if(line.BeginsWith("FitRebinDataCTD")){
						if(Tokens.size() == 2) FitRebinDataCTD = Tokens[1].ToInt();
					}
				}
				F.Close();
				return true;
			}
		}
};
Options* options = NULL;

class DivisionTemplate
{
	public:
		TH1D* H;
		double max1_x;
		double max2_x;
		double max1;
		DivisionTemplate(void){
			H = NULL;
			max1_x = 0;
			max2_x = 0;
			max1 = 0;
		}
};
DivisionTemplate* Gctd = NULL;
						
double DetectorThicknesses[12];
bool EnergyFilter(double Energy, const vector<vector<double>>& EnergyWindows);
MReadOutAssembly* RealizeSimEvent(MSimEvent* simEvent, MModuleEnergyCalibrationUniversal* Calibrator);
double ctd_template_fit_function(double* v, double* par);
void FindEdgeBins(TH1D* H, int* L, int* R);
int PixelCodeToDivision(int PixelCode, int Divisions);

//TH1D* Gctd; //used by root fit function 

bool SignalExit = false;
void Handler(int signo){
	SignalExit = true;
}

int main(int argc, char** argv)
{
	signal(SIGINT, Handler);
	MGlobal::Initialize("Standalone","");
	TApplication dcal("dcal",0,0);
	TRandom3 RNG(0);

	//parse input options file
	if(argc != 2){
		cout << "please pass an options file as the only argument. exiting..." << endl;
		return -1;
	} else {
		options = new Options();
		if(!options->ParseOptionsFile(MString(argv[1]))){
			cout << "error parsing options file, exiting..." << endl;
			return -1;
		} else {
			if(options->EnergyWindows.size() == 0){
				cout << "No energy windows were specified, using all energies! (this is probably not what you want)" << endl;
				vector<double> E;
				E.push_back(0.0); E.push_back(1.0E7);
				options->EnergyWindows.push_back(E);
			}
		}
	}

	// Load geometry:
	MDGeometryQuest* Geometry = new MDGeometryQuest();
	if (Geometry->ScanSetupFile(options->GeometryFilename) == true) {
		Geometry->ActivateNoising(false);
		Geometry->SetGlobalFailureRate(0.0);
		for(int i = 0; i < 12; ++i){
			char name[32]; snprintf(name,sizeof name,"GeWafer_%d",i);
			MDVolume* V = Geometry->MDGeometry::GetVolume(MString(name));
			if(V != 0){
				MString s = V->GetShape()->GetGeomega();
				vector<MString> Tokens = s.Tokenize(" ");
				if((Tokens.size() == 4) && (s.BeginsWith("BRIK"))){
					DetectorThicknesses[i] = 2.0 * Tokens[3].ToDouble();
					cout << "Set detector " << i << " thickness to " << DetectorThicknesses[i] << " cm using geometry specification" << endl;
				} else {
					cout << "ERROR determining thickness for volume GeWafer_" << i << ", using default of 1.5 cm" << endl;
					DetectorThicknesses[i] = 1.5;
				}
			} else {
				cout << "ERROR could not find volume GeWafer_" << i << ", in gemoetry tree.  Using default thickness of 1.5 cm" << endl;
				DetectorThicknesses[i] = 1.5;
			}
		}
		cout<<"Geometry "<<Geometry->GetName()<<" loaded!"<<endl;
	} else {
		cout<<"Unable to load geometry "<<Geometry->GetName()<<" - Aborting!"<<endl;
		return false;
	}  

	//load splines
	MDepthCalibrator* m_DepthCalibrator = new MDepthCalibrator();
	if( m_DepthCalibrator->LoadSplinesFile(options->SplineFilename) == false){
		cout << "failed to load splines file, exiting..." << endl;
		return false;
	}

	//setup nuclearizer modules
	MModuleEnergyCalibrationUniversal* Calibrator = new MModuleEnergyCalibrationUniversal();
	Calibrator->SetFileName(options->EnergyCalibrationFilename);
	MModuleStripPairingGreedy* Pairing = new MModuleStripPairingGreedy();
	if (Calibrator->Initialize() == false){
		cout << "failed to initialize energy calibrator module, exiting..." << endl;
		return false;
	}
	if (Pairing->Initialize() == false){
		cout << "failed to initialize strip pairing module, exiting..." << endl;
		return false;
	}

	//setup variables for processing of real data
	std::map<int, TH1D*> Histograms; //map to store CTD histograms
	unsigned int counter = 0;

	if( options->ProcessRealData ){ //process real data

		MModuleLoaderMeasurementsBinary* Loader = new MModuleLoaderMeasurementsBinary();
		Loader->SetFileName(options->RawDataFilename);
		Loader->SetDataSelectionMode(MBinaryFlightDataParserDataModes::c_Raw);
		Loader->SetAspectMode(MBinaryFlightDataParserAspectModes::c_Neither);
		Loader->EnableCoincidenceMerging(false);
		if (Loader->Initialize() == false) {
			cout << "failed to initialize Loader module, exiting..." << endl;
			return false;
		}

		bool IsFinished = false;
		MReadOutAssembly* Event = new MReadOutAssembly();
		while (IsFinished == false) {
			if(SignalExit){
				SignalExit = false;
				cout << "exiting real data analysis" << endl;
				break;
			}
			Event->Clear();
			if( Loader->IsReady() ){
				Loader->AnalyzeEvent(Event);
				Calibrator->AnalyzeEvent(Event);
				Pairing->AnalyzeEvent(Event);
				unsigned int NHits = Event->GetNHits();
				for(unsigned int i = 0; i < NHits; i++){
					MHit* H = Event->GetHit(i);
					unsigned int NStripHits = H->GetNStripHits();
					if( NStripHits == 2 ){ //using 2-strip events only
						bool EnergyGood = EnergyFilter(H->GetEnergy(), options->EnergyWindows);
						if( EnergyGood ){
							int pixel_code;
							double timing;
							MStripHit *SHx, *SHy;
							if( H->GetStripHit(0)->IsLowVoltageStrip() && !H->GetStripHit(1)->IsLowVoltageStrip() ){
								SHx = H->GetStripHit(0); SHy = H->GetStripHit(1);
							} else if( H->GetStripHit(1)->IsLowVoltageStrip() && !H->GetStripHit(0)->IsLowVoltageStrip() ){
								SHx = H->GetStripHit(1); SHy = H->GetStripHit(0);
							} else {
								//we didn't have 1 x and 1 y strip, log this and continue...
								continue;
							}
							int DetID = SHx->GetDetectorID();
							pixel_code = 10000*DetID + 100*SHx->GetStripID() + SHy->GetStripID();
							if( (SHx->GetTiming() < 1E-6) || (SHy->GetTiming() < 1E-6) ){ //missing timing info
								continue; 
							}
							timing = ((SHx->GetTiming() - SHy->GetTiming()));
							if( Histograms[pixel_code] == NULL ){
								char name[64]; sprintf(name,"%d",pixel_code);
								//TH1D* new_hist = new TH1D(name, name, 60, -300.0, +300.0);
								TH1D* new_hist;
								if(options->Use5nsBinning){
									new_hist = new TH1D(name,name,120,-300.0,+300.0);
								} else {
									new_hist = new TH1D(name,name,60,-300.0,+300.0);
								}

								Histograms[pixel_code] = new_hist;
							}
							//if(timing >= -0.001) timing += 0.01; else timing -= 0.01; //to avoid roundoff problems near bin edges
							timing += 0.01;
							Histograms[pixel_code]->Fill(timing);
						}
					}
				}
				++counter;
				if( (counter & 0xffff) == 0 ) cout << "num raw events: " << counter << endl;
			}
			IsFinished = Loader->IsFinished();
		}

		//when done, overwrite the histograms in the root file
		//check if subdir exists, if not, then create it
		//mkdir will return 0 if it already exists
		TFile* rootF = new TFile(options->RawDataOutputFilename,"recreate");
		for(auto const &it: Histograms){
			TH1D* hist = it.second;
			rootF->WriteTObject(hist);
			//clear the TH1D from memory... leave it for now for debugging but later
			//we might end up using lots of memeory
		}
		rootF->Close();

	}

	//set up variables for simulation data processing
	//map<int, TH1D*> DivisionCTDs;
	unordered_map<int, TH1D*> DivisionCTDs;
	unordered_map<int, TH1D*> DivisionHistograms;

	if( options->ProcessSimData ){ //read in simulation data

		MFileEventsSim* Reader = new MFileEventsSim(Geometry);
		if (Reader->Open(options->SimulationFilename) == false) {
			cout << "Unable to open sim file " << options->SimulationFilename << " - Aborting!" << endl; 
			return false;
		}
		MSimEvent* simEvent = 0;
		counter = 0;
		vector<TH1D*> DepthHistograms;
		for(int i = 0; i < 12; ++i){
			char name[16]; sprintf(name,"depth_%d",i);
			TH1D* H = new TH1D(name,name,100,0.0,DetectorThicknesses[i]);
			DepthHistograms.push_back(H);
		}
		while ((simEvent = Reader->GetNextEvent()) != 0) {
			//Reader->GetNextEvent() will print out some info about hits not being in sensitive volumes... this method will automatically exclude the weird HTs so that the code that follows
			//doesn't have to deal with it.  these HTs come from hits that are at the sensitive Ge / Ge corner boundary.
			if(SignalExit){
				SignalExit = false;
				cout << "exiting simulation data analysis" << endl;
				break;
			}
			MReadOutAssembly* Event = RealizeSimEvent(simEvent, Calibrator);
			if( Event == NULL ){
				delete simEvent;
				continue;
			}
			Pairing->AnalyzeEvent(Event);
			for( unsigned int i = 0; i < Event->GetNHits(); ++i ){
				MHit* H = Event->GetHit(i);
				if((H->GetNStripHits() != 2) || (EnergyFilter(H->GetEnergy(),options->EnergyWindows) == false)){
					break; 
				} else {
					MStripHit_s *SHx, *SHy;
					if( H->GetStripHit(0)->IsLowVoltageStrip() && !H->GetStripHit(1)->IsLowVoltageStrip() ){
						SHx = dynamic_cast<MStripHit_s*>(H->GetStripHit(0)); SHy = dynamic_cast<MStripHit_s*>(H->GetStripHit(1));
					} else if( H->GetStripHit(1)->IsLowVoltageStrip() && !H->GetStripHit(0)->IsLowVoltageStrip() ){
						SHx = dynamic_cast<MStripHit_s*>(H->GetStripHit(1)); SHy = dynamic_cast<MStripHit_s*>(H->GetStripHit(0));
					} else {
						//we didn't have 1 x and 1 y strip, log this and continue...
						break;
					}
					//check that the depths agree
					double Depth;
					if( fabs(SHx->GetDepth() - SHy->GetDepth()) > 1.0E-6 ){
						cout << "depths don't agree!" << endl;
						break;
					} else {
						Depth = SHx->GetDepth();
					}

					int DetID = SHx->GetDetectorID();
					int pixel_code = (10000*DetID) + (100*SHx->GetStripID()) + SHy->GetStripID();
					int division_code = PixelCodeToDivision(pixel_code, options->Divisions);

					//fill the depth division histogram
					if(DivisionHistograms.count(division_code) == 1){
						DivisionHistograms[division_code]->Fill(Depth);
					} else {
						char name[32]; snprintf(name, sizeof name, "divdepth_%d", division_code);
						TH1D* H = new TH1D(name,name,100,0.0,DetectorThicknesses[DetID]);
						H->Fill(Depth);
						DivisionHistograms[division_code] = H;
					}

					//fill the depth (for the whole detector) histogram, and then fill the division CTD histogram
					DepthHistograms[DetID]->Fill(Depth);
					double Timing = m_DepthCalibrator->GetSpline(DetID,true)->Eval(Depth);
					double Noise = RNG.Gaus(0,12.5/2.3548);
					if(DivisionCTDs.count(division_code) == 1){
						DivisionCTDs[division_code]->Fill(Timing+Noise);
					} else {
						char name[32]; snprintf(name,sizeof name, "divctd_%d", division_code);
						TH1D* H = new TH1D(name, name, 240, -300.0, +300.0);
						DivisionCTDs[division_code] = H;
						H->Fill(Timing+Noise);
					}

				}
			}
			++counter;
			if( (counter & 0xffff) == 0 ) cout << "num sim events: " << counter << endl;
			delete simEvent;
			delete Event;
		}
		//write simulated CTD templates to root file
		/*
		TFile* rootF = new TFile(options->SimulationOutputFilename,"recreate");
		for(auto const &it: DivisionCTDs){
			TH1D* H = it.second;
			rootF->WriteTObject(H);
		}
		for(int i = 0; i < 12; ++i){
			rootF->WriteTObject(DepthHistograms[i]);
		}
		for(auto const &it: DivisionHistograms){
			TH1D* H = it.second;
			rootF->WriteTObject(H);
		}
		rootF->Close();
		*/
		TFile* rootF = new TFile(options->SimulationOutputFilename,"recreate");
		for(int i = 0; i < 12; ++i){
			DepthHistograms[i]->SetStats();
			rootF->WriteTObject(DepthHistograms[i]);
		}
		vector<int> DivisionCTDKeys;
		for(auto const &it: DivisionCTDs) DivisionCTDKeys.push_back(it.first);
		sort(DivisionCTDKeys.begin(),DivisionCTDKeys.end());
		for(size_t i = 0; i < DivisionCTDKeys.size(); ++i){
			TH1D* H = DivisionCTDs[DivisionCTDKeys[i]];
			H->SetStats();
			rootF->WriteTObject(H);
		}
		vector<int> DivisionHistogramsKeys;
		for(auto const &it: DivisionHistograms) DivisionHistogramsKeys.push_back(it.first);
		sort(DivisionHistogramsKeys.begin(),DivisionHistogramsKeys.end());
		for(size_t i = 0; i < DivisionHistogramsKeys.size(); ++i){
			TH1D* H = DivisionHistograms[DivisionHistogramsKeys[i]];
			H->SetStats();
			rootF->WriteTObject(H);
		}
		rootF->Close();





	}

	if( options->PerformFits ){
		/*
		TFile* rootF_mctd = new TFile("data_ctd.root","READ");
		TFile* rootF_sctd = new TFile("sim_ctd.root","READ");
		*/
		unordered_map<int, DivisionTemplate*> TemplateMap;
		TFile* rootF_mctd = new TFile(options->FitDataFile);
		TFile* rootF_sctd = new TFile(options->FitSimFile);
		if( rootF_mctd == NULL ){
			cout << "couldn't open measured CTD root file, exiting..." << endl;
			return -1;
		}
		if( rootF_sctd == NULL ){
			cout << "couldn't open sim CTD root file, exiting..." << endl;
			return -1;
		}
		FILE* fout = fopen("coeffs.txt","w");
		fprintf(fout,"#format is 1) pixel code (10000*det + 100*Xchannel + Ychannel)  2) stretch 3) offset 4) scale 5) chi2 reduced\n");
		TFile* rootF_fits = new TFile("fits.root","recreate");
		TF1* fitfunc = new TF1("ctd_fitfunc",ctd_template_fit_function,-300.0,+300.0,3);
		fitfunc->SetParNames("stretch","offset","scale");
		for( unsigned int D = 0; D < 12; ++D ){
			/*
			char det_name[8]; sprintf(det_name,"%d",D);
			TH1D* ctd_template = (TH1D*)rootF_sctd->Get(det_name);
			if(ctd_template == NULL){
				cout << "could not read CTD template for detector " << D << ", skipping this detector..." << endl;
				continue;
			} else {
				Gctd = ctd_template; //set this globally so that the root fit function can see it
			}
			TF1* fitfunc = new TF1("ctd_fitfunc",ctd_template_fit_function,-300.0,+300.0,3);
			fitfunc->SetParameters(1.0, 0.0, 1.0); // first param is stretch, second is offset, third is scale
			fitfunc->SetParNames("stretch","offset","scale");
			int Ledge_bin, Redge_bin; FindEdgeBins(ctd_template, &Ledge_bin, &Redge_bin);
			double max1_x = ctd_template->GetBinCenter(Ledge_bin); double max1 = ctd_template->GetBinContent(Ledge_bin);
			double max2_x = ctd_template->GetBinCenter(Redge_bin); //double max2 = ctd_template->GetBinContent(Redge_bin);
			*/
			for( unsigned int Xch = 1; Xch <= 37; ++Xch ){
				for( unsigned int Ych = 1; Ych <= 37; ++Ych ){

					//try to read the ctd
					int pixel_code = 10000*D + 100*Xch + Ych;
					char pixel_code_s[32]; sprintf(pixel_code_s,"%d",pixel_code);
					TH1D* H = (TH1D*)rootF_mctd->Get(pixel_code_s);

					if( H == NULL ) continue; else {

						//set up the template
						int division_code = PixelCodeToDivision(pixel_code, options->Divisions);
						if(TemplateMap.count(division_code)){
							Gctd = TemplateMap[division_code];
						} else {
							char division_name[32]; snprintf(division_name, sizeof division_name, "divctd_%d", division_code);
							TH1D* ctd_template = (TH1D*)rootF_sctd->Get(division_name);
							if(ctd_template == NULL){
								cout << "could not read CTD template for division " << division_code << ", skipping pixel" << pixel_code << endl;
								continue;
							} else {
								if(options->FitRebinSimCTD >= 2) ctd_template->Rebin(options->FitRebinSimCTD);
								DivisionTemplate* dt = new DivisionTemplate();
								dt->H = ctd_template;
								int Ledge_bin, Redge_bin; FindEdgeBins(ctd_template, &Ledge_bin, &Redge_bin);
								dt->max1_x = ctd_template->GetBinCenter(Ledge_bin);
								dt->max2_x = ctd_template->GetBinCenter(Redge_bin);
								dt->max1 = ctd_template->GetBinContent(Ledge_bin);
								TemplateMap[division_code] = dt;
								Gctd = dt; //set this globally so that the root fit function can see it
							}
						}

						if( H->GetEntries() < 200 ) continue; //histogram has too few counts
						if(options->FitRebinDataCTD >= 2) H->Rebin(options->FitRebinDataCTD);
						cout << "found pixel:" << pixel_code << ", starting fit..." << endl;
						int Ledge_bin,Redge_bin; FindEdgeBins(H, &Ledge_bin, &Redge_bin);
						double cmax1_x = H->GetBinCenter(Ledge_bin); double cmax1 = H->GetBinContent(Ledge_bin);
						double cmax2_x = H->GetBinCenter(Redge_bin); //double cmax2 = H->GetBinContent(Redge_bin);
						double stretch_guess = fabs(cmax1_x - cmax2_x)/fabs(Gctd->max1_x - Gctd->max2_x);
						cout << "stretch guess: "<<stretch_guess << endl;
						double offset_guess = cmax1_x - (stretch_guess * Gctd->max1_x);
						cout << "offset_guess: " << offset_guess << endl;
						double scale_guess = cmax1/Gctd->max1;
						cout << "scale_guess: " << scale_guess << endl;
						fitfunc->SetParameters(stretch_guess, offset_guess, scale_guess);
						H->Fit(fitfunc);
						double* P; P = fitfunc->GetParameters();
						double chi = fitfunc->GetChisquare()/(H->GetNbinsX()-3.0);
						fprintf(fout,"%d   %f   %f   %f   %f\n",pixel_code,P[0],P[1],P[2],chi); 
						char title[32]; snprintf(title, sizeof title, "%d-%d", pixel_code, division_code);
						H->SetTitle(title);
						H->SetStats();
						rootF_fits->WriteTObject(H);
					}
					delete H;
				}
			}
		}
		fclose(fout);
		rootF_fits->Close();
	}

	cout << "DONE" << endl;
	return 1;
}

void FindEdgeBins(TH1D* H, int* L, int* R){

	int i;

	H->SetAxisRange(-300.0,0.0);
	double maxL = H->GetMaximum();
	for(i = 1; i <= H->GetNbinsX(); ++i){
		if( H->GetBinContent(i) >= (maxL/3.0) ) break;
	}
	*L = i;

	H->SetAxisRange(0.0,+300.0);
	double maxR = H->GetMaximum();
	for(i = H->GetNbinsX(); i > 0; --i){
		if( H->GetBinContent(i) >= (maxR/3.0) ) break;
	}
	*R = i;

	H->SetAxisRange(-300.0,+300.0);

	return;

}

double ctd_template_fit_function(double* v, double* par){

	double stretch = par[0];
	double offset = par[1];
	double scale = par[2];
	double x = v[0];
	TH1D* H = Gctd->H; //Gctd is current global ctd template

	//apply inverse stretch/offset to find the x value with respect to the original histogram bins
	double x_0 = (x - offset)/stretch;

	//now find the two bin centers that bracket x
	int bin1 = H->FindBin(x_0);
	double x1 = H->GetBinCenter(bin1);
	double y1 = H->GetBinContent(bin1);
	int bin2;
	if( x_0 <= x1 ) bin2 = bin1 - 1; else bin2 = bin1 + 1;
	double x2 = H->GetBinCenter(bin2);
	double y2 = H->GetBinContent(bin2);

	//transform x1 and x2
	x1 = x1*stretch + offset; x2 = x2*stretch + offset;
	//transform y1 and y2
	y1 = y1*scale; y2 = y2*scale;

	//find slope and intercept of line connecting (x1,y1) and (x2,y2)
	double m = (y2-y1)/(x2-x1);
	double b = y1 - m*x1;

	return m*x + b;

}

bool EnergyFilter(double Energy, const vector<vector<double>>& EnergyWindows){

	for(const auto it: EnergyWindows){
		if((Energy >= it[0]) && (Energy <= it[1])){
			return true;
		}
	}

	return false;
}

MReadOutAssembly* RealizeSimEvent(MSimEvent* simEvent, MModuleEnergyCalibrationUniversal* Calibrator){

	MReadOutAssembly* Event = new MReadOutAssembly();

	for( unsigned int i = 0; i < simEvent->GetNHTs(); ++i ){

		MSimHT* HT = simEvent->GetHTAt(i);
		MDVolumeSequence* VS = HT->GetVolumeSequence();
		MDDetector* Detector = VS->GetDetector();
		MString DetectorName = Detector->GetName();
		if(!DetectorName.BeginsWith("Detector")){ //shield hit
			//delete Event;
			//return NULL;
			continue;
		}
		DetectorName.RemoveAllInPlace("Detector");
		int DetectorID = DetectorName.ToInt();

		MDEEStripHit pSide;
		MDEEStripHit nSide;

		pSide.m_ROE.IsLowVoltageStrip(true);
		nSide.m_ROE.IsLowVoltageStrip(false);

		// Convert detector name in detector ID
		pSide.m_ROE.SetDetectorID(DetectorID);
		nSide.m_ROE.SetDetectorID(DetectorID);

		// Convert position into
		MVector PositionInDetector = VS->GetPositionInSensitiveVolume();
		MDGridPoint GP = Detector->GetGridPoint(PositionInDetector);
		double Depth = PositionInDetector.GetZ();
		//carolyn says -0.75 is the high voltage side
		//i need to map this so that +1.5 is the high voltage side, WRT to charge transport stuff
		Depth = -(Depth - DetectorThicknesses[DetectorID]/2.0);

		// Not sure about if p or n-side is up, but we can debug this later
		pSide.m_ROE.SetStripID(GP.GetXGrid()+1);
		nSide.m_ROE.SetStripID(GP.GetYGrid()+1);
		int xID = pSide.m_ROE.GetStripID();
		int yID = nSide.m_ROE.GetStripID();

		pSide.m_Energy = HT->GetEnergy();
		nSide.m_Energy = HT->GetEnergy();
		double HTEnergy = HT->GetEnergy();

		//	pSide.m_Position = PositionInDetector;
		//	nSide.m_Position = PositionInDetector;

		//at this point we have enough info to generate the strip hits
		MStripHit_s* XStrip = new MStripHit_s();
		MStripHit_s* YStrip = new MStripHit_s();
		XStrip->SetDetectorID(DetectorID); YStrip->SetDetectorID(DetectorID);
		XStrip->SetTiming(0.0); YStrip->SetTiming(0.0);
		XStrip->IsLowVoltageStrip(true); YStrip->IsLowVoltageStrip(false);
		XStrip->SetStripID(xID); YStrip->SetStripID(yID);
		XStrip->SetEnergy(HTEnergy); YStrip->SetEnergy(HTEnergy);
		double XEnRes = Calibrator->LookupEnergyResolution( XStrip, HTEnergy ); XStrip->SetEnergyResolution(XEnRes);
		double YEnRes = Calibrator->LookupEnergyResolution( YStrip, HTEnergy ); YStrip->SetEnergyResolution(YEnRes);
		XStrip->SetDepth( Depth ); YStrip->SetDepth( Depth );
		Event->AddStripHit( (MStripHit*)XStrip ); Event->AddStripHit( (MStripHit*)YStrip );

	}

	return Event;

}

int PixelCodeToDivision(int PixelCode, int Divisions){
	int d = PixelCode/10000;
	int y = PixelCode % 100;
	int x = ((PixelCode % 10000) - y)/100;
	y--; //map strips into range 0 - 36
	x--;
	int xd = x/(36/Divisions);
	if(xd == Divisions) xd--;
	int yd = y/(36/Divisions);
	if(yd == Divisions) yd--;
	//return (d*1000) + (xd + 1)*(yd + 1);
	return (d*1000) + (xd * Divisions) + yd;
}
