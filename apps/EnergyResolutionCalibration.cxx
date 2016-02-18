//THis program parses the EnergyCalibration.ecal file created by melinator to make the model fits for the energy resolution as a function of energy for each strip

//Standard
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include <map>
#include <sys/types.h>
//#include <dirent.h>
//#include <stdio.h>

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
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TLine.h>

// MEGAlib
#include "MGlobal.h"
#include "MFile.h"
#include "MAssembly.h"
#include "MSupervisor.h"
#include "MString.h"
#include "MStreams.h"
#include "MFileReadOuts.h"
#include "MReadOutAssembly.h"
#include "MNCTStripHit.h"
#include "MReadOutSequence.h"
#include "MNCTModuleMeasurementLoaderROA.h"

// Nuclearizer
#include "MReadOutElement.h"
#include "MReadOutElementDoubleStrip.h"



////////////////////////////////////////////////////////////////////////////

int main()
{


	TApplication Plots("Plots", 0, 0);

//open and read and parse the .ecal file and make a plot for each detector side of all the strips with the correct fit - using P1 from Mark's thesis (a0  +a1*x)^(1/2) for now.
//The parameters from the fit will be appended to the .ecal file and then when loading the .ecal file through Nuclearizer with MNCTModuleEnergyCalibrationUniversal.cxx, the resolution will be parsed out

	TF1 *P1 = new TF1("P1","([0] + [1]*x)^(1/2)",0,2000);
	P1->SetParameter(0,5);
	P1->SetParameter(0,0.001);
//	TF1 *P0 = new TF1("P0", "[0]^(1/2)",0,2000);
//	P0->SetParameter(0,5);
//	TF1 *P2 = new TF1("P2", "([0] + [1]*x + [2]*x^2)^(1/2)",0,2000);
//	P2->SetParameter(0,5);
//	P2->SetParameter(1,0.001);
//	P2->SetParameter(2,0.00001);

	ofstream energyresolution_calibration;
	energyresolution_calibration.open("EnergyResolution_Calibration.txt");
	//energyresolution_calibration<<"Detector | Strip | Side | F_0 | F_1"<<"\n \n"; 
	energyresolution_calibration.close();

	

	
	map<int, TMultiGraph*> DetGraphs;
	map<int, TMultiGraph*> ResidGraphs;

	int GraphID;
	for (int d = 0; d < 12; ++d) {
		for (int s = 0; s < 2; ++s) {
			GraphID = d + 12*s;
			TMultiGraph *mg = new TMultiGraph();
			ostringstream title;
			title << "Detector "<<d<<", Side "<<s;
			const char* t = title.str().c_str();
			mg->SetTitle(t);
			DetGraphs[GraphID] = mg;
			TMultiGraph *mgr = new TMultiGraph();
			mgr->SetTitle("Residuals");
			ResidGraphs[GraphID] = mgr;
		}
	}


	MParser Parser;
	if (Parser.Open("~/Software/Nuclearizer/resource/calibration/COSI16/EnergyCalibration.ecal") == false) {
		cout<<"Unable to open calibration file "<<endl;
		return false;
	}
	
	map<MReadOutElementDoubleStrip, unsigned int> CP_ROEToLine;
	
	for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
		unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
		if (NTokens < 2) continue;
		if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "CP") == true) {
			if (Parser.GetTokenizerAt(i)->IsTokenAt(1, "dss") == true) {
				MReadOutElementDoubleStrip R;
				R.SetDetectorID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2));
				R.SetStripID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3));
				R.IsPositiveStrip(Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "p");
				CP_ROEToLine[R] = i;
			}
		}
	}

	for (auto CP: CP_ROEToLine) {
		//cout<<CP.first<<endl;
		if (CP_ROEToLine.find(CP.first) != CP_ROEToLine.end()) {
			unsigned int i = CP_ROEToLine[CP.first];
			if (Parser.GetTokenizerAt(i)->IsTokenAt(5, "pakw") == true) {
				if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(6) < 3) {
					cout<<"Not enough calibartion points (only "<<Parser.GetTokenizerAt(i)->GetTokenAtAsInt(6)<<") for strip: "<<CP.first<<endl;
					continue;
				}
			} else {
				cout<<"Unknown calibration point descriptor found: "<<Parser.GetTokenizerAt(i)->GetTokenAt(5)<<endl;
				continue;
			}
		} else {
			cout<<"No good calibration for the following strips found: "<<CP.first<<endl;
			continue;
		}
			
			
	
		//int GraphID;
		GraphID = CP.first.GetDetectorID() + CP.first.IsPositiveStrip()*12;
		TGraph *g = new TGraph();
		g->SetMarkerStyle(5);
		TGraph *gr = new TGraph();
		gr->SetMarkerStyle(5);
	
		unsigned int Pos = 6;
		unsigned int NumPoints = Parser.GetTokenizerAt(CP.second)->GetTokenAtAsInt(Pos);
		double energy[NumPoints];
		double res[NumPoints];
		int k1 = 0;
		int k2 = 0;
		for (unsigned int j = Pos + 2; j <= (Pos + (NumPoints*3)); j = j + 3) {
			//cout<<j<<" "<<Parser.GetTokenizerAt((CP.second))->GetTokenAtAsDouble(j)<<" ";
			energy[k1] = Parser.GetTokenizerAt(CP.second)->GetTokenAtAsDouble(j);
			res[k1] = Parser.GetTokenizerAt(CP.second)->GetTokenAtAsDouble(j+1);
			if (energy[k1] != 510.99){
				g->SetPoint(k2,energy[k1],res[k1]);
				++k2;
			}	 //exclude the 511 line because the resolution is systematically higher
			++k1;
		}
		g->Fit("P1","Q");
		DetGraphs[GraphID]->Add(g);

		TF1 *fit = g->GetFunction("P1");
		fit->SetLineWidth(1);
		double f0 = fit->GetParameter(0);
		double f1 = fit->GetParameter(1);
//		double f2 = fit->GetParameter(2);

		energyresolution_calibration.open("EnergyResolution_Calibration.txt", ios::out | ios::app);
		if (CP.first.IsPositiveStrip() == 0) {
			energyresolution_calibration<<"CR dss "<<CP.first.GetDetectorID()<<" "<<CP.first.GetStripID()<<" n P1 "<<f0<<" "<<f1<<"\n";
		} else {
			energyresolution_calibration<<"CR dss "<<CP.first.GetDetectorID()<<" "<<CP.first.GetStripID()<<" p P1 "<<f0<<" "<<f1<<"\n";
		}
	 // cout<<f0<<" "<<f1<<endl;
		energyresolution_calibration.close();
		
		cout<<NumPoints<<" "<<k2<<endl;	
		double resid[k2];
		k2 = 0;
		for (unsigned int p = 0; p < NumPoints; ++p) {
			if (energy[p] != 510.99) {
				resid[k2] = res[p] - fit->Eval(energy[p]);
				gr->SetPoint(k2,energy[p],resid[k2]);
				++k2;
			}
		}
		ResidGraphs[GraphID]->Add(gr);
	}

	Parser.Close();


	for (auto mg: DetGraphs) {
		TCanvas *C = new TCanvas();
		C->SetWindowSize(600,700);
		C->Divide(1,2);
    C->cd(1);
		mg.second->Draw("AP");
		mg.second->GetXaxis()->SetTitle("Energy (keV)");
		mg.second->GetYaxis()->SetTitle("Energy Resolution (keV)");
		mg.second->Draw("AP");
		//C->Update();
		C->cd(2);
		ResidGraphs[mg.first]->Draw("AP");
		ResidGraphs[mg.first]->GetXaxis()->SetTitle("Energy (keV)");
		ResidGraphs[mg.first]->GetYaxis()->SetTitle("Residuals");
		TLine *l = new TLine(0,0,1900,0);
		l->Draw();
		C->cd(2);
		C->Update();

		ostringstream title;
		title<<"EnergyResolution_P1_Det"<<(mg.first % 12)<<"_Side"<<((mg.first/12) % 2 == 1)<<".pdf";
		const char* t = title.str().c_str();
		C->Print(t);
	}

	Plots.Run();
}
