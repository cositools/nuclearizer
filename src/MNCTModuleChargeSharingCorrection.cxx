/*
* MNCTModuleChargeSharingCorrection.cxx
*
*
* Copyright (C) 2009-2009 by Mark Bandstra.
* All rights reserved.
*
*
* This code implementation is the intellectual property of
* Mark Bandstra.
*
* By copying, distributing or modifying the Program (or any work
* based on the Program) you indicate your acceptance of this statement,
* and all its terms.
*
*/


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModuleChargeSharingCorrection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleChargeSharingCorrection.h"

// Standard libs:
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

// ROOT libs:
#include "TGClient.h"
#include "TFile.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MVector.h"
#include "MString.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleChargeSharingCorrection)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleChargeSharingCorrection::MNCTModuleChargeSharingCorrection() : MModule()
{
  // Construct an instance of MNCTModuleChargeSharingCorrection
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Charge sharing correction";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "ChargeSharingCorrection";
  
  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_StripPairing);
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_ChargeSharingCorrection);
  
  // Set all modules, which can follow this module
	AddSucceedingModuleType(MAssembly::c_NoRestriction);  

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options

	nSources = 4;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleChargeSharingCorrection::~MNCTModuleChargeSharingCorrection()
{
  // Delete this instance of MNCTModuleChargeSharingCorrection
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleChargeSharingCorrection::Initialize()
{
  // Initialize the module 
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleChargeSharingCorrection::AnalyzeEvent(MReadOutAssembly* Event) 
{
	//load correction files
	LoadCorrectionInfo();


  // Main data analysis routine, which updates the event to a new level 

  unsigned int NHits = Event->GetNHits();

	if (Event->IsStripPairingIncomplete() == false){
  
  for (unsigned int i_hit=0; i_hit<NHits; i_hit++)
  {
    // Count number of X and Y strips
    MNCTHit *H = Event->GetHit(i_hit);
    unsigned int NStripHits = H->GetNStripHits();
    unsigned int NXStripHits = 0, NYStripHits = 0;
    //int DetectorNumber;
    for (unsigned int i_sh=0; i_sh<NStripHits; i_sh++)
    {
      MNCTStripHit *SH = H->GetStripHit(i_sh);
      //DetectorNumber = SH->GetDetectorID();
      if (SH->IsXStrip() == true) { NXStripHits++; } else { NYStripHits++; }
    }
    
    //Check for 1 pixel event and sharing event..
    MNCTStripHit *SHX, *SHY;
    if ( (NStripHits==2) && (NXStripHits==1) && (NYStripHits==1) )
    {
      SHX = H->GetStripHit(0);
      SHY = H->GetStripHit(1);
      if ( !SHX->IsXStrip() )
      {
        SHX = H->GetStripHit(1);
        SHY = H->GetStripHit(0);
      }
      // assign negative side energy to this hit
      H->SetEnergy(SHY->GetEnergy());
      H->SetEnergyResolution(SHY->GetEnergyResolution());
    }
    else if (( (NStripHits==3) && (((NXStripHits==1)&&(NYStripHits==2)) ||
      ((NXStripHits==2)&&(NYStripHits==1))) ) ||
      ( (NStripHits==4) && (NXStripHits==2) && (NYStripHits==2)))
    { 

      int i_sxhit=0;
      int i_syhit=0;
      int XStripID[NXStripHits];
      int YStripID[NYStripHits];
      double XTotalEnergy=0.0,XEnergyVar=0.0;
      double YTotalEnergy=0.0,YEnergyVar=0.0;
			double XEnergy[NXStripHits];
			double YEnergy[NYStripHits];
			int detector = 0;
      // Get the X and Y strip numbers, and calculate the total energy for each side
      for (unsigned int i_s_hit=0; i_s_hit < NStripHits; i_s_hit++)
      {
        if (H->GetStripHit(i_s_hit)->IsXStrip() == true)
        {
          XStripID[i_sxhit] = H->GetStripHit(i_s_hit)->GetStripID();
					XEnergy[i_sxhit] = H->GetStripHit(i_s_hit)->GetEnergy();

          XTotalEnergy += H->GetStripHit(i_s_hit)->GetEnergy();
          XEnergyVar += (H->GetStripHit(i_s_hit)->GetEnergyResolution())
          *(H->GetStripHit(i_s_hit)->GetEnergyResolution());
          i_sxhit++;

					detector = H->GetStripHit(i_s_hit)->GetDetectorID();
        }
        else
        {
          YStripID[i_syhit] = H->GetStripHit(i_s_hit)->GetStripID();
					YEnergy[i_syhit] = H->GetStripHit(i_s_hit)->GetEnergy();

          YTotalEnergy += H->GetStripHit(i_s_hit)->GetEnergy();
          YEnergyVar += (H->GetStripHit(i_s_hit)->GetEnergyResolution())
          *(H->GetStripHit(i_s_hit)->GetEnergyResolution());
          i_syhit++;
        }
      }
      if (NXStripHits==2 && NYStripHits==1)
      {
        if (((XStripID[0]-XStripID[1])==1) || ((XStripID[0]-XStripID[1])==-1))
        {
					double correctedE0 = EstimateE0(XEnergy[0],XEnergy[1],detector,0);
					//for debugging
/*					if ((correctedE0 < 760&&correctedE0 > 560) || (XTotalEnergy<760 && XTotalEnergy>560) || (YTotalEnergy<760 && YTotalEnergy>560)){
						cout << "y total energy: " << YTotalEnergy << endl;
						cout << "x total energy: " << XTotalEnergy << endl;
						cout << "xEnergy[0]: " << XEnergy[0] << endl;
						cout << "xEnergy[1]: " << XEnergy[1] << endl;
						cout << "yEnergy[0]: " << YEnergy[0] << endl;
						cout << "corrected energy: " << correctedE0 << endl;
						dummy_func();
					}
*/
					//print original hit energy
/*					cout << "-------------------" << endl;
					cout << "p side" << endl;
					cout << "x total energy: " << XTotalEnergy << endl;
					cout << "y total energy: " << YTotalEnergy << endl;
					cout << "corrected energy: " << correctedE0 << endl;
					cout << "..........." << endl;
					cout << "original hit energy: " << H->GetEnergy() << endl;
*/
					//for now, choose the higher energy
	//				if (correctedE0 < YTotalEnergy){
	//					H->SetEnergy(YTotalEnergy);
	//					H->SetEnergyResolution(sqrt(YEnergyVar));
	//				}
	//				else {
	          H->SetEnergy(correctedE0);
	          H->SetEnergyResolution(sqrt(XEnergyVar));
	//				}
					//print final hit energy
//					cout << "final hit energy: " << H->GetEnergy() << endl;
//					dummy_func();
        }
      }
      else if (NYStripHits==2 && NXStripHits==1)
      {
        if (((YStripID[0]-YStripID[1])==1) || ((YStripID[0]-YStripID[1])==-1))
        {
					//looks like we don't need to do correction on Y side?!
//					double correctedE0 = EstimateE0(YEnergy[0],YEnergy[1],detector,1);
					//for now, choose the higher energy
//					if (correctedE0 < XTotalEnergy){

					//print original hit energy
/*					cout << "-----------------" << endl;
					cout << "n side" << endl;
					cout << "x total energy: " << XTotalEnergy << endl;
					cout << "y total energy: " << YTotalEnergy << endl;
					cout << "........" << endl;
					cout << "original hit energy: " << H->GetEnergy() << endl;
*/
					if (YTotalEnergy < XTotalEnergy){
						H->SetEnergy(XTotalEnergy);
						H->SetEnergyResolution(sqrt(XEnergyVar));
					}
					else {
//						H->SetEnergy(correctedE0);
						H->SetEnergy(YTotalEnergy);
          	H->SetEnergyResolution(sqrt(YEnergyVar));
					}

					//print final hit energy
//					cout << "final hit energy: " << H->GetEnergy() << endl;
//					dummy_func();
        }
      }
      else if (NYStripHits==2 && NXStripHits==2)
      {
        if ( (((YStripID[0]-YStripID[1])==1) || ((YStripID[0]-YStripID[1])==-1)) &&
          (((XStripID[0]-XStripID[1])==1) || ((XStripID[0]-XStripID[1])==-1)))
        {
					double correctedXE0 = EstimateE0(XEnergy[0],XEnergy[1],detector,0);
//					double correctedYE0 = EstimateE0(YEnergy[0],YEnergy[1],detector,1);
					//for now, choose the higher energy
//					if (correctedXE0 > correctedYE0){
					if (correctedXE0 > YTotalEnergy){
						H->SetEnergy(correctedXE0);
						H->SetEnergyResolution(sqrt(XEnergyVar));
					}
					else {
//          	H->SetEnergy(correctedYE0);
						H->SetEnergy(YTotalEnergy);
          	H->SetEnergyResolution(sqrt(YEnergyVar));
					}
        }
      } 
      else
      {
        // can't do anything to help this hit
      }
    }
    else
    {
      // can't do anything to help this hit
    }
  }
	}  
  return true;


};

vector<vector<double> > MNCTModuleChargeSharingCorrection::ParseOneSource(string filename){

	ifstream clFile;
	clFile.open(filename);

	vector<double> bOneDet;
	vector<vector<double> > bOneSource;

	string line;
	int c=0;
	double B;

	if (clFile.is_open()){
		while (!clFile.eof()){
			c++;
//			cout << c << endl;
			getline(clFile,line);

			if (c <= 24){
				B = stod(line);
				if (c%2 != 0){
					bOneDet.push_back(B);
				}
				else {
					bOneDet.push_back(B);
					bOneSource.push_back(bOneDet);
					bOneDet.clear();
				}
			}
		}
	}

	clFile.close();

//	cout << "291" << endl;
	return bOneSource;

};


void MNCTModuleChargeSharingCorrection::LoadCorrectionInfo(){

	vector<string> filenames;
 	filenames.push_back("./ChargeLossCorrection/ChargeLossCorrection_122.log");
	filenames.push_back("./ChargeLossCorrection/ChargeLossCorrection_356.log");
	filenames.push_back("./ChargeLossCorrection/ChargeLossCorrection_662.log");
	filenames.push_back("./ChargeLossCorrection/ChargeLossCorrection_1333.log");

	for (int i=0; i<filenames.size(); i++){
		vector<vector<double> > oneSource = ParseOneSource(filenames.at(i));
		B.push_back(oneSource);
		oneSource.clear();
	}

};


double MNCTModuleChargeSharingCorrection::Interpolate(double E0, int det, int side){

	double energies[4] = {122,356,662,1333};
	double points[nSources];
	points[0] = B.at(0).at(det).at(side);
	points[1] = B.at(1).at(det).at(side);
	points[2] = B.at(2).at(det).at(side);
	points[3] = B.at(3).at(det).at(side);

	TGraph* bGraph = new TGraph(nSources,energies,points);

	double val = bGraph->Eval(E0);
	return val;

};



double MNCTModuleChargeSharingCorrection::EstimateE0(double enOne, double enTwo, int det, int side){

	double sum = enOne+enTwo;
	double diff = fabs(enOne-enTwo);
//	double b = Interpolate(enOne,det,side);
	double b = Interpolate(sum,det,side);

	//debugging
/*	if (side == 0 && sum > 600){
		cout << "-----------------------------" << endl;
		cout << "sum: " << sum << endl;
		cout << "diff: " << diff << endl;
		cout << "b: " << b << endl;
	}
*/
	double correctedE0;
	if (sum < 300){
		correctedE0 = 1/(2-b)*(sum+sqrt(sum*sum - b*(2-b)*diff*diff));
	}
	else {
		correctedE0 = (sum-b*diff)/(1-b);
	}

	//debugging
/*	if (side == 0 && sum > 600){
		cout << "correctedE0: " << correctedE0 << endl;
		dummy_func();
	}
*/
	return correctedE0;

};


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleChargeSharingCorrection::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}

////////////////////////////////////////////////////////////////////////////////

void MNCTModuleChargeSharingCorrection::dummy_func(){

	return;

};


// MNCTModuleChargeSharingCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
