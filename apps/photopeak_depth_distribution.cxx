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
#include "MDGeometry.h"
#include "MFileEventsTra.h"


class Options
{
	public:
		MString EnergyCalibrationFilename;
		MString GeometryFilename;
		//vector<vector<double>> EnergyWindows;
		vector<vector<double>> EnergyWindows;
		MString Filename;
		MString OutputFilename;
		int NDepthBins;
		Options(void): EnergyWindows(12){
			EnergyCalibrationFilename = "$NUCLEARIZER/resource/calibration/COSI16/Wanaka/EnergyCalibration_051016.ecal";
			GeometryFilename = "";
			Filename = "";
			OutputFilename = "photopeak_depth_histograms.root";
			NDepthBins = 15;
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
					//cout << "line = " << line << endl;
					if(line.BeginsWith("EnergyCalibrationFilename")){
						if(Tokens.size() == 2) EnergyCalibrationFilename = Tokens[1];
					} else if(line.BeginsWith("EnergyWindow")){
						if(Tokens.size() == 4){
							vector<double> E;
							int DetID = Tokens[1].ToInt();
							EnergyWindows[DetID].push_back(Tokens[2].ToDouble());
							EnergyWindows[DetID].push_back(Tokens[3].ToDouble());
						}
					} else if(line.BeginsWith("Filename")){
						if(Tokens.size() == 2) Filename = Tokens[1];
					} else if(line.BeginsWith("GeometryFilename")){
						if(Tokens.size() == 2) GeometryFilename = Tokens[1];
					} else if(line.BeginsWith("NDepthBins")){
						if(Tokens.size() == 2) NDepthBins = Tokens[1].ToInt();
					} else if(line.BeginsWith("OutputFilename")){
						if(Tokens.size() == 2) OutputFilename = Tokens[1];
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
	TApplication xyscan("photopeak_depth_distribution",0,0);
	MString InputFile;

	//parse input options file
	if(argc < 2){
		cout << "please pass input file and then options file. exiting..." << endl;
		return -1;
	} else {
		if(argc == 3){
			options = new Options();
			if(!options->ParseOptionsFile(MString(argv[2]))){
				cout << "error parsing options file, exiting..." << endl;
				return -1;
			}
		} 
		InputFile = argv[1];
	}

	MFileEventsTra F;
	if(!F.Open(InputFile)){
		cout << "failed to open input file, aborting... " << InputFile << endl;
	}

	//load geometry and set up histograms
	vector<double> DetectorThicknesses(12,0);
	vector<TH1D*> DepthHistograms(12,0);
	MDGeometry* G = new MDGeometry();
	if(!G->ScanSetupFile(options->GeometryFilename)){
		cout << "failed to load geometry file " << options->GeometryFilename << ", aborting..." << endl;
	} else {
		for(int i = 0; i < 12; ++i){
			char name[32]; snprintf(name,sizeof name,"GeWafer_%d",i);
			MDVolume* V = G->GetVolume(MString(name));
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
			char hname[32]; snprintf(hname, sizeof hname, "depth_photo_%d", i);
			DepthHistograms[i] = new TH1D(hname,hname,options->NDepthBins,0.0,DetectorThicknesses[i]);
		}
		cout<<"Geometry "<<G->GetName()<<" loaded!"<<endl;
	}

	unsigned int counter = 0;
	TH1D* BadEventsZH = new TH1D("badz","badz",1000,-10,10);
	while(1){
		MPhysicalEvent* E = F.GetNextEvent();
		if(SignalExit) break;
		if(E == 0) break; else{
			if(E->GetType() == MPhysicalEvent::c_Photo){
				MVector V = E->GetPosition();
				MDVolumeSequence VS = G->GetVolumeSequence(V, true, true);
				//MDVolumeSequence VS = G->GetVolumeSequence(V);
				MDDetector* D = VS.GetDetector();
				if(D == 0){
					BadEventsZH->Fill(E->GetPosition().GetZ());
					cout << "event ID is " << E->GetId() << "deepest volume for bad hit at " << E->GetPosition() << " is " << VS.GetDeepestVolume()->GetName() << endl;
					continue;
				}
				MString DS = D->GetName();
				if(DS.BeginsWith("Detector")){
					DS.RemoveAllInPlace("Detector");
					int DetID = DS.ToInt();
					if((E->GetEnergy() >= options->EnergyWindows[0][0]) && (E->GetEnergy() <= options->EnergyWindows[0][1])){
						//compute local position in detector
						MVector VD = VS.GetPositionInSensitiveVolume();
						double Depth = (DetectorThicknesses[DetID]/2.0) - VD.GetZ();
						DepthHistograms[DetID]->Fill(Depth);
						//do we want to select a region of the detector or just look at the total depth distribution?
					}
				}
			}
		}
		++counter;
		if((counter & 0xffff) == 0xffff){
		  cout << "processed " << counter << " events" << endl;
		}	  
	}

	TFile f(options->OutputFilename,"recreate");
	f.WriteTObject(BadEventsZH);
	for(const auto h: DepthHistograms){
		f.WriteTObject(h);
	}
	f.Close();

	return 0;
}
