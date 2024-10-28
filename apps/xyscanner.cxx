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


class Options
{
	public:
		MString EnergyCalibrationFilename;
		vector<vector<double>> EnergyWindows;
		MString RawDataFilename;
		bool TextOutput;
		int PatternMapStripID;
		Options(void){
			EnergyCalibrationFilename = "$NUCLEARIZER/resource/calibration/COSI16/Wanaka/EnergyCalibration_051016.ecal";
			RawDataFilename = "";
			TextOutput = false;
			return;
			PatternMapStripID = 26;
		}
		bool ParseOptionsFile(MString fname){
			MFile F;
			if(!F.Open(fname)){
				return false;
			} else {
				MString line;
				while(F.ReadLine(line)){
					vector<MString> Tokens = line.Tokenize(" ");
					//cout << "line = " << line << endl;
					if(line.BeginsWith("EnergyCalibrationFilename")){
						if(Tokens.size() == 2) EnergyCalibrationFilename = Tokens[1];
					} else if(line.BeginsWith("EnergyWindow")){
						if(Tokens.size() == 3){
							vector<double> E;
							E.push_back(Tokens[1].ToDouble());
							E.push_back(Tokens[2].ToDouble());
							EnergyWindows.push_back(E);
						}
					} else if(line.BeginsWith("RawDataFilename")){
						if(Tokens.size() == 2) RawDataFilename = Tokens[1];
					} else if(line.BeginsWith("TextOutput")){
						if(Tokens.size() == 2) Tokens[1].ToLower() == "true" ? TextOutput = true : TextOutput = false;
					} else if(line.BeginsWith("PatternMapStripID")){
						if(Tokens.size() == 2) PatternMapStripID = Tokens[1].ToInt();
						cout << "Setting PatternMapStripID = " << PatternMapStripID << endl;
					} else {
						//cout << "BAD INPUT IN OPTIONS FILE: " << line << endl;
					}

				}
				F.Close();
				return true;
			}
		}
};
Options* options = NULL;


bool SignalExit = false;
void Handler(int signo){
	SignalExit = true;
}
bool EnergyFilter(double Energy, const vector<vector<double>>& EnergyWindows);

int main(int argc, char** argv)
{

	signal(SIGINT, Handler);
	MGlobal::Initialize("Standalone","");
	TApplication xyscan("xyscan",0,0);
	MString InputRawFile;

	//parse input options file
	if(argc < 2){
		cout << "please pass an options file as the only argument. exiting..." << endl;
		return -1;
	} else {
		if(argc == 3){
			options = new Options();
			if(!options->ParseOptionsFile(MString(argv[2]))){
				cout << "error parsing options file, exiting..." << endl;
				return -1;
			}
		} 
		InputRawFile = argv[1];

	}

	//setup nuclearizer modules
	MModuleEnergyCalibrationUniversal* Calibrator = new MModuleEnergyCalibrationUniversal();
	Calibrator->SetFileName(options->EnergyCalibrationFilename);
	if (Calibrator->Initialize() == false){
		cout << "failed to initialize energy calibrator module, exiting..." << endl;
		return false;
	}

	unsigned int counter = 0;
	MModuleLoaderMeasurementsBinary* Loader = new MModuleLoaderMeasurementsBinary();
	Loader->SetFileName(InputRawFile);
	Loader->SetDataSelectionMode(MBinaryFlightDataParserDataModes::c_Raw);
	Loader->SetAspectMode(MBinaryFlightDataParserAspectModes::c_Neither);
	Loader->EnableCoincidenceMerging(false);
	if (Loader->Initialize() == false) {
		cout << "failed to initialize Loader module, exiting..." << endl;
		return false;
	}

	//open ROOT file and setup histograms
	TFile* rootF = new TFile((MString(InputRawFile) + ".root").Data(),"recreate");

	//set up histograms
	TH1D* HTotal = new TH1D("HTotal","HTotal",37,(1 - 0.5),(37 + 0.5));
	unordered_map<int,TH2D*> HdT; //time differences between energy collecting strip and all TOnly neighbors
	unordered_map<int,TH2D*> HdT_symm; //time difference between TOnly neighbors
	unordered_map<int,TH2D*> H; //just the raw timing values
	for(int i = 1; i <= 37; ++i){
		char name[32]; snprintf(name,sizeof name,"dt_%d",i);
		TH2D* h = new TH2D(name,name,7,(-3.0 - 0.5),(3.0 + 0.5),61,(0.0 - 2.5),(300.0 + 2.5));
		h->SetOption("colz");
		HdT[i] = h;
		snprintf(name,sizeof name,"dtsym_%d",i);
		h = new TH2D(name,name,7,(-3.0 - 0.5),(3.0 + 0.5),61,(-150.0 - 2.5),(+150.0 + 2.5));
		h->SetOption("colz");
		HdT_symm[i] = h;
		snprintf(name,sizeof name,"t_%d",i);
		h = new TH2D(name,name,7,(-3.0 - 0.5),(3.0 + 0.5),61,(0.0 - 2.5),(300.0 + 2.5));
		h->SetOption("colz");
		H[i] = h;
	}

	//setup counters
	unsigned int NEvents = 0;
	unsigned int NEvents1Neighbor = 0;
	unsigned int NEvents2Neighbor = 0;
	unsigned int NEvents3Neighbor = 0;
	unsigned int NEvents4Neighbor = 0;
	unsigned int NEvents4PlusNeighbor = 0;

	//setup pattern record
	map<vector<int>,unsigned int> PatternMap;


	bool IsFinished = false;
	MReadOutAssembly* Event = new MReadOutAssembly();
	FILE* fout = NULL;
	if(options->TextOutput) fout = fopen("out.out","w");
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
			unsigned int NSH = Event->GetNStripHits();
			MStripHit *XSH, *YSH;
			if(NSH == 2){
				if(Event->GetStripHit(0)->IsLowVoltageStrip()){
					if(!Event->GetStripHit(1)->IsLowVoltageStrip()){
						XSH = Event->GetStripHit(0);
						YSH = Event->GetStripHit(1);
					} else {
						continue;
					}
				} else {
					if(Event->GetStripHit(1)->IsLowVoltageStrip()){
						XSH = Event->GetStripHit(1);
						YSH = Event->GetStripHit(0);
					} else {
						continue;
					}
				}
				bool XEnergyGood = EnergyFilter(XSH->GetEnergy(), options->EnergyWindows);
				bool YEnergyGood = EnergyFilter(YSH->GetEnergy(), options->EnergyWindows);
				if(XEnergyGood && YEnergyGood){
					if(options->TextOutput){
						fprintf(fout,"ID %d\n",Event->GetID());
						size_t NTonly = Event->GetNStripHitsTOnly();
						fprintf(fout,"X %d:%d ---> ",XSH->GetStripID(),(int)XSH->GetTiming());
						for(int i = 0; i < NTonly; ++i){
							MStripHit* SH = Event->GetStripHitTOnly(i);
							if(SH->IsLowVoltageStrip()) fprintf(fout,"%d:%d ",SH->GetStripID(),(int)SH->GetTiming());
						}
						fprintf(fout,"\nY %d:%d ---> ", YSH->GetStripID(),(int)YSH->GetTiming());
						for(int i = 0; i < NTonly; ++i){
							MStripHit* SH = Event->GetStripHitTOnly(i);
							if(!SH->IsLowVoltageStrip()) fprintf(fout,"%d:%d ",SH->GetStripID(),(int)SH->GetTiming());
						}
						fprintf(fout,"\n");
					}

					//make map of TOnly strip hits on the X side
					map<int,MStripHit*> TOnlyNeighbors;
					for(int i = 0; i < Event->GetNStripHitsTOnly(); ++i){
						auto SH = Event->GetStripHitTOnly(i);
						if(SH->IsLowVoltageStrip()){
							if(abs(SH->GetStripID() - XSH->GetStripID()) <= 3){ //keep in mind that sometimes we have more than one TOnly trigger on each side (like 4 neighboring strips)
								TOnlyNeighbors[SH->GetStripID()] = SH;
							}
						}
					}

					int XSH_id = XSH->GetStripID();
					
					//store pattern
					if(XSH_id == options->PatternMapStripID){
						vector<int> Pattern;
						for(const auto i: TOnlyNeighbors) Pattern.push_back(i.first - XSH_id);
						if(PatternMap.count(Pattern)){
							++PatternMap[Pattern];
						} else {
							PatternMap[Pattern] = 1;
						}
					}


					HTotal->Fill(XSH_id);
					++NEvents;
					switch(TOnlyNeighbors.size()){
						case 0:
							break;
						case 1:
							++NEvents1Neighbor;
							break;
						case 2:
							++NEvents2Neighbor;
							break;
						case 3:
							++NEvents3Neighbor;
							break;
						case 4:
							++NEvents4Neighbor;
							break;
						default:
							++NEvents4PlusNeighbor;
					}

					//loop over all TOnly, compute time difference with energy collecting strip, and add to HdT histogram. also fill in raw timing histogram
					for(const auto p: TOnlyNeighbors){
						MStripHit* SH = p.second;
						double dID = (double)(SH->GetStripID() - XSH_id);
						double dt = XSH->GetTiming() - SH->GetTiming();
						HdT[XSH_id]->Fill(dID,dt);
						H[XSH_id]->Fill(dID,SH->GetTiming());
					}
					H[XSH_id]->Fill(0.0,XSH->GetTiming());

					//loop over pairs of TOnly neighbors
					for(int i = 1; i <= 3; ++i){
						if(TOnlyNeighbors.count(XSH_id + i)){
							if(TOnlyNeighbors.count(XSH_id - i)){
								auto RSH = TOnlyNeighbors[XSH_id + i];
								auto LSH = TOnlyNeighbors[XSH_id - i];
								HdT_symm[XSH_id]->Fill(i,RSH->GetTiming() - LSH->GetTiming());
							}
						}
					}
				}
			}
		}

		++counter;
		if( (counter & 0xffff) == 0 ) cout << "num raw events: " << counter << endl;
		IsFinished = Loader->IsFinished();
	}

	for(int i = 1; i <= 37; ++i){
		rootF->WriteTObject(HdT[i]);
		rootF->WriteTObject(HdT_symm[i]);
		rootF->WriteTObject(H[i]);
	}
	rootF->WriteTObject(HTotal);
	rootF->Close();

	if(options->TextOutput) fclose(fout);
	unsigned int NEventsWithNeighbors = NEvents1Neighbor + NEvents2Neighbor + NEvents3Neighbor + NEvents4Neighbor + NEvents4PlusNeighbor;
	cout << "Counters summary" << endl;
	cout << "NEvents = " << NEvents << endl;
	cout << "NEventsWithNeighbors = " << NEventsWithNeighbors << endl;
	cout << "Percentage of total events that had at least one TOnly neighbor = " << ((double)NEventsWithNeighbors/(double)NEvents)*100.0 << " %" << endl;
	cout << "NEvents1Neighbor = " << NEvents1Neighbor << " (" << ((double)NEvents1Neighbor/(double)NEventsWithNeighbors)*100.0 << " %)" << endl;
	cout << "NEvents2Neighbor = " << NEvents2Neighbor << " (" << ((double)NEvents2Neighbor/(double)NEventsWithNeighbors)*100.0 << " %)" << endl;
	cout << "NEvents3Neighbor = " << NEvents3Neighbor << " (" << ((double)NEvents3Neighbor/(double)NEventsWithNeighbors)*100.0 << " %)" << endl;
	cout << "NEvents4Neighbor = " << NEvents4Neighbor << " (" << ((double)NEvents4Neighbor/(double)NEventsWithNeighbors)*100.0 << " %)" << endl;
	cout << "NEvents4PlusNeighbor = " << NEvents4PlusNeighbor << " (" << ((double)NEvents4PlusNeighbor/(double)NEventsWithNeighbors)*100.0 << " %)" << endl;

	map<unsigned int,vector<int>> PatternMapInv;
	for(const auto i: PatternMap) PatternMapInv[i.second] = i.first;
	for(const auto i: PatternMapInv){
		cout << "PN " << i.first << " (";
		for(const auto j:i.second){
			cout << j << ",";
		}
		cout << ")" << endl;
	}



	return 1;
}

bool EnergyFilter(double Energy, const vector<vector<double>>& EnergyWindows){

	for(const auto it: EnergyWindows){
		if((Energy >= it[0]) && (Energy <= it[1])){
			return true;
		}
	}

	return false;
}

