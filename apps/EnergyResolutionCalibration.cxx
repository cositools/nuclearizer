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
#include <TGraphErrors.h>
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
#include "MStripHit.h"
#include "MReadOutSequence.h"
#include "MModuleLoaderMeasurementsROA.h"

// Nuclearizer
#include "MReadOutElement.h"
#include "MReadOutElementDoubleStrip.h"



////////////////////////////////////////////////////////////////////////////

//int main()
int main(int argc, char* argv[])
{

	// Added by Jackie on 19/12/03 to take arguments
        if (argc < 3) {
             // We expect two arguments: the energy calibration .ecal file and the desired output .txt file name
             cout << "Please pass an .ecal file and output .txt file name as arguments." << endl;
             return 1;  
        }


	//TApplication Plots("Plots", 0, 0);

//open and read and parse the .ecal file and make a plot for each detector side of all the strips with the correct fit - using P1 from Mark's thesis (a0  +a1*x)^(1/2) for now.
//The parameters from the fit will be appended to the .ecal file and then when loading the .ecal file through Nuclearizer with MModuleEnergyCalibrationUniversal.cxx, the resolution will be parsed out


	TF1 *P1 = new TF1("P1","[0] + [1]*x",0,2000);
	P1->SetParameter(0,3);
	P1->SetParameter(1,0.01);
//	TF1 *P1 = new TF1("P1","([0] + [1]*x)^(1/2)",0,2000);
//	P1->SetParameter(0,30);
//	P1->SetParameter(1,0.01);
//	TF1 *P0 = new TF1("P0", "[0]^(1/2)",0,2000);
//	P0->SetParameter(0,5);
//	TF1 *P2 = new TF1("P2", "([0] + [1]*x + [2]*x^2)^(1/2)",0,2000);
//	P2->SetParameter(0,5);
//	P2->SetParameter(1,0.001);
//	P2->SetParameter(2,0.00001);

	ofstream energyresolution_calibration;

	// Added below on 19/12/03 by Jackie to take output .txt file name as an argument
        std::string energyres_calib_file;
        energyres_calib_file = argv[2];
        energyresolution_calibration.open(energyres_calib_file);

	//energyresolution_calibration.open("EnergyResolution_Calibration.txt");
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

	TMultiGraph * Det3avgres = new TMultiGraph();
	TGraphErrors *Det3avgres_n = new TGraphErrors();
	TGraphErrors *Det3avgres_p = new TGraphErrors();
	double averageres_Det3n[10][40] = {0};
	double energy_Det3n[14] = {0};
	int numstrips_Det3n[10] = {0};
	double averageres_Det3p[10][40] = {0};
	double energy_Det3p[14] = {0};
	int numstrips_Det3p[10] = {0};


	MParser Parser;

	// Added below on 19/12/03 by Jackie to take path to energy calibration .ecal file as an argument
        std::string energy_calib_file;
        energy_calib_file = argv[1];
        if (Parser.Open(energy_calib_file) == false) {


	//if (Parser.Open("/home/jacqueline/MEGAlib/nuclearizer/resource/calibration/COSI19/Berkeley/EnergyCalibration.ecal") == false) {
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
				R.IsLowVoltageStrip(Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "p");
				CP_ROEToLine[R] = i;
			}
		}
	}

	int numamstrips = 0;


	for (auto CP: CP_ROEToLine) {
		//cout<<CP.first<<endl;
		if (CP_ROEToLine.find(CP.first) != CP_ROEToLine.end()) {
			unsigned int i = CP_ROEToLine[CP.first];			
			
			
			if (Parser.GetTokenizerAt(i)->IsTokenAt(5, "pakw") == true) {
			
			// Below check of 3 calibration points commented out by J. Beechert on 19/11/15.
			// Melinator now offers polynomial fits of order < 3, so we no longer want to throw an error if we see only 1 or 2 calibration points.	
			// J.Beechert on 20/06/11: instead of commenting out the error, change it to give an error if the order is 0 
			//if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(6) < 3) {
			  if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(6) == 0) {
					cout<<"Not enough calibration points (only "<<Parser.GetTokenizerAt(i)->GetTokenAtAsInt(6)<<") for strip: "<<CP.first<<endl;
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
		GraphID = CP.first.GetDetectorID() + CP.first.IsLowVoltageStrip()*12;
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
			if ( ((GraphID == 11) || GraphID == 23) && (energy[k1] != 510.99) && (k1 < NumPoints)) {
				if (energy[k1] == 59.541) {
					energy_Det3n[0] = energy[k1];
					numstrips_Det3n[0]++;
					if (CP.first.IsLowVoltageStrip() == 0) {averageres_Det3n[0][CP.first.GetStripID()] = res[k1];
					} else {averageres_Det3p[0][CP.first.GetStripID()] = res[k1]; };
				} else if (energy[k1] == 80.997) {
                                        energy_Det3n[1] = energy[k1];
                                        numstrips_Det3n[1]++;
                                        if (CP.first.IsLowVoltageStrip() == 0) {averageres_Det3n[1][CP.first.GetStripID()] = res[k1];
                                        } else {averageres_Det3p[1][CP.first.GetStripID()] = res[k1]; };
				} else if (energy[k1] == 122.061) {
					energy_Det3n[2] = energy[k1];
                                        numstrips_Det3n[2]++;
                                        if (CP.first.IsLowVoltageStrip() == 0) {averageres_Det3n[2][CP.first.GetStripID()] = res[k1];
					 } else {averageres_Det3p[2][CP.first.GetStripID()] = res[k1]; };
				} else if (energy[k1] == 356.017) {
					energy_Det3n[3]= energy[k1];
					numstrips_Det3n[3]++;
					if (CP.first.IsLowVoltageStrip() == 0) { averageres_Det3n[3][CP.first.GetStripID()] = res[k1];
					} else {averageres_Det3p[3][CP.first.GetStripID()] = res[k1]; };
				} else if (energy[k1] == 383.851) {
					energy_Det3n[4] = energy[k1];
					numstrips_Det3n[4]++;
					if (CP.first.IsLowVoltageStrip() == 0) {averageres_Det3n[4][CP.first.GetStripID()] = res[k1];
					} else {averageres_Det3p[4][CP.first.GetStripID()] = res[k1]; };
				} else if (energy[k1] == 661.657) {
					energy_Det3n[5] = energy[k1];
					numstrips_Det3n[5]++;
					if (CP.first.IsLowVoltageStrip() == 0) {averageres_Det3n[5][CP.first.GetStripID()] = res[k1];
					} else {averageres_Det3p[5][CP.first.GetStripID()] = res[k1]; };
				} else if (energy[k1] == 898.042) {
					energy_Det3n[6] = energy[k1];
					numstrips_Det3n[6]++;
					if (CP.first.IsLowVoltageStrip() == 0) {averageres_Det3n[6][CP.first.GetStripID()] = res[k1];
					} else {averageres_Det3p[6][CP.first.GetStripID()] = res[k1]; };
				} else if (energy[k1] == 1173.24) {
					energy_Det3n[7] = energy[k1];
					numstrips_Det3n[7]++;
					if (CP.first.IsLowVoltageStrip() == 0) {averageres_Det3n[7][CP.first.GetStripID()] = res[k1];
					} else {averageres_Det3p[7][CP.first.GetStripID()] = res[k1]; };
				} else if (energy[k1] == 1332.5) {
					energy_Det3n[8] = energy[k1];
					numstrips_Det3n[8]++;
					if (CP.first.IsLowVoltageStrip() == 0) {averageres_Det3n[8][CP.first.GetStripID()] = res[k1];
					} else {averageres_Det3p[8][CP.first.GetStripID()] = res[k1]; };
				}	
				
			}

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

		// Edited the below to use the output .txt file name passed as an argument
		energyresolution_calibration.open(energyres_calib_file, ios::out | ios::app);
		//energyresolution_calibration.open("EnergyResolution_Calibration.txt", ios::out | ios::app);
		
		if (CP.first.IsLowVoltageStrip() == 0) {
//			energyresolution_calibration<<"CR dss "<<CP.first.GetDetectorID()<<" "<<CP.first.GetStripID()<<" n P0 "<<f0<<"\n";
			energyresolution_calibration<<"CR dss "<<CP.first.GetDetectorID()<<" "<<CP.first.GetStripID()<<" n P1 "<<f0<<" "<<f1<<"\n";
		} else {
//			energyresolution_calibration<<"CR dss "<<CP.first.GetDetectorID()<<" "<<CP.first.GetStripID()<<" p P1 "<<f0<<"\n";
			energyresolution_calibration<<"CR dss "<<CP.first.GetDetectorID()<<" "<<CP.first.GetStripID()<<" p P1 "<<f0<<" "<<f1<<"\n";
		}
	 // cout<<f0<<" "<<f1<<endl;
		energyresolution_calibration.close();
		
		//cout<<NumPoints<<" "<<k2<<endl;	
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


	for (int i = 0; i < 10; ++i) {

		for (int j = 0; j < 38; ++j) {
	//		cout<<"averageres_Det3n[i][j] "<<averageres_Det3n[i][j]<<endl;
			averageres_Det3n[i][38] += averageres_Det3n[i][j];
			averageres_Det3n[i][38] += averageres_Det3p[i][j];
		}
		if (i == 0) {
			cout<<"Sum Am241: "<<averageres_Det3n[0][38]<<" Number of strips: "<<(numstrips_Det3n[0]+numstrips_Det3p[0])<<endl;
		}
		averageres_Det3n[i][38] = averageres_Det3n[i][38]/(numstrips_Det3n[i]+numstrips_Det3p[i]);
		//averageres_Det3p[i][38] = averageres_Det3p[i][38]/numstrips_Det3p[i];
	
		if (i == 0) { cout<<"average: "<<averageres_Det3n[i][38]<<endl; };
	
		for (int j = 0; j < 37; ++j) {
			if (averageres_Det3n[i][j] != 0) { averageres_Det3n[i][39] += pow(averageres_Det3n[i][j] - averageres_Det3n[i][38],2);}
			if (averageres_Det3p[i][j] != 0) { averageres_Det3n[i][39] += pow(averageres_Det3p[i][j] - averageres_Det3n[i][38],2);}
		}
		averageres_Det3n[i][39] = sqrt(averageres_Det3n[i][39]/(numstrips_Det3n[i]+numstrips_Det3p[i]));
		//averageres_Det3p[i][39] = sqrt(averageres_Det3p[i][39]/numstrips_Det3p[i]);

               if (i == 0) {
                        cout<<"Std Am241: "<<averageres_Det3n[0][39]<<endl;
                }

		averageres_Det3n[i][38] = averageres_Det3n[i][38]/energy_Det3n[i]*100;
		//averageres_Det3p[i][38] = averageres_Det3p[i][38]/energy_Det3p[i];
		averageres_Det3n[i][39] = averageres_Det3n[i][39]/energy_Det3n[i]*100;
		//averageres_Det3p[i][39] = averageres_Det3p[i][39]/energy_Det3p[i];

		//Convert to MeV
		energy_Det3n[i]= energy_Det3n[i]/1000;


		cout<<"energy: "<<energy_Det3n[i]<<", average: "<<averageres_Det3n[i][38]<<", std: "<<averageres_Det3n[i][39]<<", num strips: "<<numstrips_Det3n[i]<<endl;
		Det3avgres_n->SetPoint(i, energy_Det3n[i], averageres_Det3n[i][38]);
		Det3avgres_n->SetPointError(i, 0, averageres_Det3n[i][39]/sqrt(numstrips_Det3n[i]));

		//Det3avgres_p->SetPoint(i, energy_Det3p[i], averageres_Det3p[i][38]);
		//Det3avgres_p->SetPointError(i, 0, averageres_Det3p[i][39]/sqrt(numstrips_Det3p[i]));

	}


	TCanvas *C = new TCanvas();
	C->SetLeftMargin(0.11);
	C->SetRightMargin(0.05);
	C->SetBottomMargin(0.15);
	C->SetLogx();
//	C->SetLogy();
	Det3avgres_n->SetMarkerStyle(8);
	Det3avgres_n->SetMarkerSize(0.8);
//	Det3avgres_n->SetLineColor(kBlue);
	Det3avgres_n->SetLineWidth(1);
	Det3avgres_n->SetMarkerColor(kAzure+7);
	gStyle->SetEndErrorSize(4);
//	Det3avgres_p->SetMarkerStyle(1);
//	Det3avgres_p->SetLineWidth(2);
//	Det3avgres_p->SetMarkerColor(kRed);
//	Det3avgres_p->SetLineColor(kRed);
	
//	Det3avgres->Add(Det3avgres_n);
///	Det3avgres->GetXaxis()->SetRangeUser(50, 1400);	

//	Det3avgres->Add(Det3avgres_p);
	Det3avgres_n->Draw("AP");
	Det3avgres_n->GetXaxis()->SetRangeUser(0.050, 1.500);
	Det3avgres_n->GetYaxis()->SetRangeUser(0.1,5);
	Det3avgres_n->GetYaxis()->SetNoExponent(true);
	Det3avgres_n->GetXaxis()->SetNoExponent(true);
	Det3avgres_n->GetXaxis()->SetTitle("Energy [MeV]");
	Det3avgres_n->GetXaxis()->SetTitleSize(0.06);
	Det3avgres_n->GetXaxis()->SetTitleOffset(1.15);
	Det3avgres_n->GetXaxis()->SetLabelSize(0.04);
	Det3avgres_n->GetYaxis()->SetTitle("FWHM/Energy (%)");
	Det3avgres_n->GetYaxis()->SetTitleSize(0.06);
	Det3avgres_n->GetYaxis()->SetLabelSize(0.04);
	Det3avgres_n->GetYaxis()->SetTitleOffset(0.8);


	TF1 * pol1fit = new TF1("pol1fit", "[0]*pow(x,[1])+[2]", 0.050, 1.5);
	pol1fit->SetParameter(0, 20);
	pol1fit->SetParameter(1,-0.5);
	Det3avgres_n->Fit("pol1fit");

	TF1 *fitfun = (TF1*)Det3avgres_n->GetListOfFunctions()->FindObject("pol1fit");
	fitfun->SetLineColor(kBlue+1);
	fitfun->SetLineWidth(1);
	Det3avgres_n->GetYaxis()->SetNdivisions(505);
//	Det3avgres_n->GetYaxis()->SetMoreLogLabels(true);
	Det3avgres_n->Draw("AP");
	C->Update();

        ostringstream title;
        title<<"EnergyResolution_Det11.pdf";
        cout<<title<<endl;
        C->Print(title.str().c_str());




	for (auto mg: DetGraphs) {
		TCanvas * C = new TCanvas();
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

		ostringstream  title;	
		title<<"EnergyResolution_P1_Det"<<(mg.first % 12)<<"_Side"<<((mg.first/12) % 2 == 1)<<".pdf";
		cout<<title<<endl;
		C->Print(title.str().c_str());
	}

	//Plots.Run();




}
