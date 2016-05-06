//TODO
/*
-redefine measured CTD histos so that bins are centered on 10' of nanoseconds
-define photopeak criteria... use energy resolutions for the strips, and say +/- 2.35 sigma 

	*/


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
#include "MNCTDepthCalibrator.h"
#include "MNCTModuleMeasurementLoaderBinary.h"
#include "MNCTBinaryFlightDataParser.h"

class MNCTDEEStripHit
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
		vector<MNCTDEEStripHit> m_SubStripHits;
};

class MNCTStripHit_s : public MNCTStripHit
{
	//use this derived class instead of MNCTStripHit so that we can keep track of the depth 
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
		MString SimulationFilename;
		MString SplineFilename;
		MString GeometryFilename;
		bool ProcessRealData;
		bool ProcessSimData;
		bool PerformFits;
		bool Use5nsBinning;
		void Options(){
			ProcessRealData = true;
			ProcessSimData = true;
			PerformFits = true;
			Use5nsBinning = true;
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
						if(Tokens.size() == 2) RawDataFile = Tokens[1];
					} else if(line.BeginsWith("SimulationFilename")){
						if(Tokens.size() == 2) SimulationFilename = Tokens[1];
					} else if(line.BeginsWith("SplineFilename")){
						if(Tokens.size() == 2) SplineFilename = Tokens[1];
					} else if(line.BeginsWith("GeometryFilename")){
					  	if(Tokens.size() == 2) GeometryFilename = Tokens[1];	
					} else if(line.BeginsWith("ProcessRealData")){
						if(Tokens.size() == 2) ProcessRealData = Tokens[1].ToBool();
					} else if(line.BeginsWith("ProcessSimData")){
						if(Tokens.size() == 2) ProcessSimData = Tokens[1].ToBool();
					} else if(line.BeginsWith("PerformFits")){
						if(Tokens.size() == 2) PerformFits = Tokens[1].ToBool();
					} else if(line.BeginsWith("Use5nsBinning")){
						if(Tokens.size() == 2) Use5nsBinning = Tokens[1].ToBool();
					} else if(line.BeginsWith("EnergyWindow")){
						if(Tokens.size() == 3){
							vector<double> E;
							E.push_back(Tokens[1].ToDouble());
							E.push_back(Tokens[2].ToDouble());
							EnergyWindows.push_back(E);
						}
					}
				}
				F.Close();
				return true;
			}
		}
};
						
double DetectorThicknesses[12];
bool EnergyFilter(double Energy);
MReadOutAssembly* RealizeSimEvent(MSimEvent* simEvent, MNCTModuleEnergyCalibrationUniversal* Calibrator);
double ctd_template_fit_function(double* v, double* par);
void FindEdgeBins(TH1D* H, int* L, int* R);

TH1D* Gctd; //used by root fit function 
Options* options;

int main(int argc, char** argv)
{

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

	MGlobal::Initialize("Standalone","");
	TApplication dcal("dcal",0,0);
	TRandom3 RNG(0);

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
	MNCTDepthCalibrator* m_DepthCalibrator = new MNCTDepthCalibrator();
	if( m_DepthCalibrator->LoadSplinesFile(MString(argv[4])) == false){
		cout << "failed to load splines file, exiting..." << endl;
		return false;
	}

	//setup nuclearizer modules
	MNCTModuleMeasurementLoaderBinary* Loader = new MNCTModuleMeasurementLoaderBinary();
	Loader->SetFileName(options->RawDataFilename);
	Loader->SetDataSelectionMode(MNCTBinaryFlightDataParserDataModes::c_Raw);
	Loader->SetAspectMode(MNCTBinaryFlightDataParserAspectModes::c_Neither);
	Loader->EnableCoincidenceMerging(false);
	MNCTModuleEnergyCalibrationUniversal* Calibrator = new MNCTModuleEnergyCalibrationUniversal();
	Calibrator->SetFileName("$(NUCLEARIZER)/resource/calibration/COSI16/Berkeley/EnergyCalibration.ecal");
	MNCTModuleStripPairingGreedy_b* Pairing = new MNCTModuleStripPairingGreedy_b();
	if (Loader->Initialize() == false) return false;
	if (Calibrator->Initialize() == false) return false;
	if (Pairing->Initialize() == false) return false;

	//setup variables for processing of real data
	std::map<int, TH1D*> Histograms; //map to store CTD histograms
	unsigned int counter = 0;

	if( Level & 0x1 ){ //process real data

		bool IsFinished = false;
		MReadOutAssembly* Event = new MReadOutAssembly();
		while (IsFinished == false) {
			Event->Clear();
			if( Loader->IsReady() ){
				Loader->AnalyzeEvent(Event);
				Calibrator->AnalyzeEvent(Event);
				Pairing->AnalyzeEvent(Event);
				unsigned int NHits = Event->GetNHits();
				for(unsigned int i = 0; i < NHits; i++){
					MNCTHit* H = Event->GetHit(i);
					unsigned int NStripHits = H->GetNStripHits();
					if( NStripHits == 2 ){ //using 2-strip events only
						bool EnergyGood = EnergyFilter(H->GetEnergy());
						if( EnergyGood ){
							int pixel_code;
							double timing;
							MNCTStripHit *SHx, *SHy;
							if( H->GetStripHit(0)->IsXStrip() && !H->GetStripHit(1)->IsXStrip() ){
								SHx = H->GetStripHit(0); SHy = H->GetStripHit(1);
							} else if( H->GetStripHit(1)->IsXStrip() && !H->GetStripHit(0)->IsXStrip() ){
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
							timing = ((SHx->GetTiming() - SHy->GetTiming())); //add five for bin centering
							if( Histograms[pixel_code] == NULL ){
								char name[64]; sprintf(name,"%d",pixel_code);
								//TH1D* new_hist = new TH1D(name, name, 60, -300.0, +300.0);
								TH1D* new_hist = new TH1D(name,name,120,-300.0,+300.0);
								Histograms[pixel_code] = new_hist;
							}
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
		TFile* rootF = new TFile("data_ctd.root","recreate");
		for(auto const &it: Histograms){
			TH1D* hist = it.second;
			rootF->WriteTObject(hist);
			//clear the TH1D from memory... leave it for now for debugging but later
			//we might end up using lots of memeory
		}
		rootF->Close();

	}

	//set up variables for simulation data processing
	std::map<int, TH1D*> simHistograms;

	if( Level & 0x2 ){ //read in simulation data

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
			TH1D* H = new TH1D(name,name,1000,0.0,DetectorThicknesses[i]);
			DepthHistograms.push_back(H);
		}
		while ((simEvent = Reader->GetNextEvent()) != 0) {
			//Reader->GetNextEvent() will print out some info about hits not being in sensitive volumes... this method will automatically exclude the weird HTs so that the code that follows
			//doesn't have to deal with it.  these HTs come from hits that are at the sensitive Ge / Ge corner boundary.
			MReadOutAssembly* Event = RealizeSimEvent(simEvent, Calibrator);
			if( Event == NULL ){
				continue;
			}
			Pairing->AnalyzeEvent(Event);
			for( unsigned int i = 0; i < Event->GetNHits(); ++i ){
				MNCTHit* H = Event->GetHit(i);
				if( (H->GetNStripHits() != 2) || (EnergyFilter(H->GetEnergy()) == false) ) continue; else {
					MNCTStripHit_s *SHx, *SHy;
					if( H->GetStripHit(0)->IsXStrip() && !H->GetStripHit(1)->IsXStrip() ){
						SHx = dynamic_cast<MNCTStripHit_s*>(H->GetStripHit(0)); SHy = dynamic_cast<MNCTStripHit_s*>(H->GetStripHit(1));
					} else if( H->GetStripHit(1)->IsXStrip() && !H->GetStripHit(0)->IsXStrip() ){
						SHx = dynamic_cast<MNCTStripHit_s*>(H->GetStripHit(1)); SHy = dynamic_cast<MNCTStripHit_s*>(H->GetStripHit(0));
					} else {
						//we didn't have 1 x and 1 y strip, log this and continue...
						continue;
					}
					//check that the depths agree
					double Depth;
					if( fabs(SHx->GetDepth() - SHy->GetDepth()) > 1.0E-6 ){
						cout << "depths don't agree!" << endl;
						continue;
					} else {
						Depth = SHx->GetDepth();
					}
					int DetID = SHx->GetDetectorID();
					DepthHistograms[DetID]->Fill(Depth);
					double Timing = m_DepthCalibrator->GetSpline(DetID,true)->Eval(Depth);
					double Noise = RNG.Gaus(0,12.5/2.3548);
					TH1D* hist = simHistograms[DetID];
					if( hist == NULL ){
						char name[64]; sprintf(name,"%d",DetID);
						hist = new TH1D(name, name, 8*60, -300.0, +300.0);
						simHistograms[DetID] = hist;
					}
					hist->Fill(Timing+Noise);
				}
			}
			++counter;
			if( (counter & 0xffff) == 0 ) cout << "num sim events: " << counter << endl;
			delete simEvent;
			delete Event;
		}
		//write simulated CTD templates to root file
		TFile* rootF = new TFile("sim_ctd.root","recreate");
		for(auto const &it: simHistograms){
			TH1D* hist = it.second;
			rootF->WriteTObject(hist);
		}
		for(int i = 0; i < 12; ++i){
			rootF->WriteTObject(DepthHistograms[i]);
		}
		rootF->Close();
	}

	if( Level & 0x4 ){
		//perform the fits

		//loop over pixel codes in order and try to read the pixel data from the root file
		//first check if the root file exists

		TFile* rootF_mctd = new TFile("data_ctd.root","READ");
		TFile* rootF_sctd = new TFile("sim_ctd.root","READ");

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

		for( unsigned int D = 0; D < 12; ++D ){
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

			//find maxima of CTD edges
			/*
			ctd_template->SetAxisRange(-300.0,0.0);
			double max1 = ctd_template->GetMaximum();
			double max1_bin = ctd_template->GetMaximumBin();
			double max1_x = ctd_template->GetBinCenter(max1_bin);

			ctd_template->SetAxisRange(0.0,300.0);
			double max2 = ctd_template->GetMaximum();
			double max2_bin = ctd_template->GetMaximumBin();
			double max2_x = ctd_template->GetBinCenter(max2_bin);

			ctd_template->SetAxisRange(-300.0,300.0);
			*/

			int Ledge_bin, Redge_bin; FindEdgeBins(ctd_template, &Ledge_bin, &Redge_bin);
			double max1_x = ctd_template->GetBinCenter(Ledge_bin); double max1 = ctd_template->GetBinContent(Ledge_bin);
			double max2_x = ctd_template->GetBinCenter(Redge_bin); //double max2 = ctd_template->GetBinContent(Redge_bin);

			for( unsigned int Xch = 1; Xch <= 37; ++Xch ){
				for( unsigned int Ych = 1; Ych <= 37; ++Ych ){
					int pixel_code = 10000*D + 100*Xch + Ych;
					char pixel_code_s[32]; sprintf(pixel_code_s,"%d",pixel_code);
					TH1D* H = (TH1D*)rootF_mctd->Get(pixel_code_s);
					if( H == NULL ) continue; else {
						if( H->GetEntries() < 200 ) continue; //histogram has too few counts
						cout << "found pixel:" << pixel_code << ", starting fit..." << endl;
						//idea, use best fit from last pixel as starting values here since these parameters vary spacially (especially stretch factor), assuming chi2 is good enough
						//MIGHT NEED TO CALL SET PARAMETERS AGAIN HERE 
//						if( !UseLast ) fitfunc->SetParameters(1.0, 0.0, 1.0);

						FindEdgeBins(H, &Ledge_bin, &Redge_bin);
						double cmax1_x = H->GetBinCenter(Ledge_bin); double cmax1 = H->GetBinContent(Ledge_bin);
						double cmax2_x = H->GetBinCenter(Redge_bin); //double cmax2 = H->GetBinContent(Redge_bin);


						double stretch_guess = fabs(cmax1_x - cmax2_x)/fabs(max1_x - max2_x);
						cout << "stretch guess: "<<stretch_guess << endl;
						double offset_guess = cmax1_x - (stretch_guess * max1_x);
						cout << "offset_guess: " << offset_guess << endl;
						double scale_guess = cmax1/max1;
						cout << "scale_guess: " << scale_guess << endl;

						fitfunc->SetParameters(stretch_guess, offset_guess, scale_guess);

//						fitfunc->SetParameters(1.1,0.0,1.0);
						H->Fit(fitfunc);
						double* P; P = fitfunc->GetParameters();
//						double chi = fitfunc->GetChisquare()/57.0;//57 is number of bins minus number of fit parameters
						double chi = fitfunc->GetChisquare()/(H->GetNbinsX()-3.0);
						fprintf(fout,"%d   %f   %f   %f   %f\n",pixel_code,P[0],P[1],P[2],chi); 
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
	TH1D* H = Gctd; //Gctd is current global ctd template

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

bool EnergyFilter(double Energy){

	double continuum_low = 200.0;
	double continuum_high = 477.0;
	double photopeak_low = 650.0;
	double photopeak_high = 670.0;

	if( (Energy >= continuum_low) && (Energy <= continuum_high) ){
		return true;
	} else if( (Energy >= photopeak_low) && (Energy <= photopeak_high) ){
		return true;
	} else {
		return false;

	}
}

MReadOutAssembly* RealizeSimEvent(MSimEvent* simEvent, MNCTModuleEnergyCalibrationUniversal* Calibrator){

	MReadOutAssembly* Event = new MReadOutAssembly();

	for( unsigned int i = 0; i < simEvent->GetNHTs(); ++i ){

		MSimHT* HT = simEvent->GetHTAt(i);
		MDVolumeSequence* VS = HT->GetVolumeSequence();
		MDDetector* Detector = VS->GetDetector();
		MString DetectorName = Detector->GetName();
		if(!DetectorName.BeginsWith("Detector")){
			delete Event;
			return NULL;
		}
		DetectorName.RemoveAllInPlace("Detector");
		int DetectorID = DetectorName.ToInt();

		MNCTDEEStripHit pSide;
		MNCTDEEStripHit nSide;

		pSide.m_ROE.IsPositiveStrip(true);
		nSide.m_ROE.IsPositiveStrip(false);

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
		MNCTStripHit_s* XStrip = new MNCTStripHit_s();
		MNCTStripHit_s* YStrip = new MNCTStripHit_s();
		XStrip->SetDetectorID(DetectorID); YStrip->SetDetectorID(DetectorID);
		XStrip->SetTiming(0.0); YStrip->SetTiming(0.0);
		XStrip->IsXStrip(true); YStrip->IsXStrip(false);
		XStrip->SetStripID(xID); YStrip->SetStripID(yID);
		XStrip->SetEnergy(HTEnergy); YStrip->SetEnergy(HTEnergy);
		double XEnRes = Calibrator->LookupEnergyResolution( XStrip, HTEnergy ); XStrip->SetEnergyResolution(XEnRes);
		double YEnRes = Calibrator->LookupEnergyResolution( YStrip, HTEnergy ); YStrip->SetEnergyResolution(YEnRes);
		XStrip->SetDepth( Depth ); YStrip->SetDepth( Depth );
		Event->AddStripHit( (MNCTStripHit*)XStrip ); Event->AddStripHit( (MNCTStripHit*)YStrip );

	}

	return Event;

}

