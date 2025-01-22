/*
* MModuleChargeSharingCorrection.cxx
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
// MModuleChargeSharingCorrection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleChargeSharingCorrection.h"

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


#ifdef ___CLING___
ClassImp(MModuleChargeSharingCorrection)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleChargeSharingCorrection::MModuleChargeSharingCorrection() : MModule()
{
  // Construct an instance of MModuleChargeSharingCorrection
  
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

//	m_nSources = 4;
	m_nSources = 3;
}


////////////////////////////////////////////////////////////////////////////////


MModuleChargeSharingCorrection::~MModuleChargeSharingCorrection()
{
  // Delete this instance of MModuleChargeSharingCorrection
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleChargeSharingCorrection::Initialize()
{
  // Initialize the module 

 	//load correction files
	LoadCorrectionInfo();
//	LoadCorrectionInfoUpdated();
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleChargeSharingCorrection::AnalyzeEvent(MReadOutAssembly* Event) 
{


  // Main data analysis routine, which updates the event to a new level 

  unsigned int NHits = Event->GetNHits();

	if (Event->IsStripPairingIncomplete() == false){
  
  for (unsigned int i_hit=0; i_hit<NHits; i_hit++)
  {
    // Count number of X and Y strips
    MHit *H = Event->GetHit(i_hit);
    unsigned int NStripHits = H->GetNStripHits();
    unsigned int NXStripHits = 0, NYStripHits = 0;
    //int DetectorNumber;
    for (unsigned int i_sh=0; i_sh<NStripHits; i_sh++)
    {
      MStripHit *SH = H->GetStripHit(i_sh);
      //DetectorNumber = SH->GetDetectorID();
      if (SH->IsLowVoltageStrip() == true) { NXStripHits++; } else { NYStripHits++; }
    }
    
    //Check for 1 pixel event and sharing event..
    MStripHit *SHX, *SHY;
    if ( (NStripHits==2) && (NXStripHits==1) && (NYStripHits==1) )
    {
      SHX = H->GetStripHit(0);
      SHY = H->GetStripHit(1);
      if ( !SHX->IsLowVoltageStrip() )
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
        if (H->GetStripHit(i_s_hit)->IsLowVoltageStrip() == true)
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
					double correctedE0 = EstimateE0(XEnergy[0],XEnergy[1],YTotalEnergy,detector,0);
					//for now, choose the higher energy
					if (correctedE0 < YTotalEnergy){
						H->SetEnergy(YTotalEnergy);
						H->SetEnergyResolution(sqrt(YEnergyVar));
					}
					else {
//					if (correctedE0 < YTotalEnergy){
	          H->SetEnergy(correctedE0);
	          H->SetEnergyResolution(sqrt(XEnergyVar));
	//				}
					}
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
					double correctedXE0 = EstimateE0(XEnergy[0],XEnergy[1],YTotalEnergy,detector,0);
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

vector<vector<double> > MModuleChargeSharingCorrection::ParseOneSource(string filename){

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

void MModuleChargeSharingCorrection::LoadCorrectionInfoUpdated(){

	string filename = "./ChargeLossCorrectionScaled_Ba133.log";

	m_Bfrac = ParseOneSource(filename);

};


void MModuleChargeSharingCorrection::LoadCorrectionInfo(){

	vector<string> filenames;
 	filenames.push_back("./ChargeLossCorrectionScaled_Co57.log");
	filenames.push_back("./ChargeLossCorrectionScaled_Ba133.log");
	filenames.push_back("./ChargeLossCorrectionScaled_Cs137_2.log");
//	filenames.push_back("./ChargeLossCoeffs_Co60.log");

	for (unsigned int i=0; i<filenames.size(); i++){
		vector<vector<double> > oneSource = ParseOneSource(filenames.at(i));
		m_B.push_back(oneSource);
		oneSource.clear();
	}

	//find coefficients for linear interpolation
//	double energies[4] = {122,356,662,1333};
	double energies[3] = {122,356,662};
	double points[m_nSources];
	double A0;
	double A1;

	for (int det=0; det<12; det++){
		for (int side=0; side<2; side++){
//			points[0] = m_B.at(0).at(det).at(side);
//			points[1] = m_B.at(1).at(det).at(side);
//			points[2] = m_B.at(2).at(det).at(side);
//			points[3] = m_B.at(3).at(det).at(side);

			points[0] = m_B.at(0).at(det).at(side);
			points[1] = m_B.at(1).at(det).at(side);
			points[2] = m_B.at(2).at(det).at(side);

			TGraph *g = new TGraph(m_nSources,energies,points);
			TF1 *f = new TF1("f","[0]+[1]*x",energies[0],energies[m_nSources-1]);
			g->Fit("f","RQ");

			A0 = f->GetParameter(0);
			A1 = f->GetParameter(1);

			m_linInterpCoeffs[det][side][0] = A0;
			m_linInterpCoeffs[det][side][1] = A1;

			delete g;
			delete f;

		}
	}

	cout << "Charge loss correction loaded" << endl;

};


double MModuleChargeSharingCorrection::Interpolate(double E0, int det, int side){

	double A0 = m_linInterpCoeffs[det][side][0];
	double A1 = m_linInterpCoeffs[det][side][1];

	double val = A0+A1*E0;

//	cout << m_B.at(1).at(det).at(side) << '\t' << val << endl;

	return val;

};



double MModuleChargeSharingCorrection::EstimateE0(double enOne, double enTwo, double eOtherSide, int det, int side){

	double sum = enOne+enTwo;
	double diff = fabs(enOne-enTwo);
	double frac = diff/sum;
	double scaled_sum = sum/eOtherSide;

	//NOT SURE ABOUT THIS
//	if (scaled_sum > 1){ return eOtherSide; }//enOne+enTwo; }

//	double b = Interpolate(enOne,det,side);
//	double b = Interpolate(sum,det,side);
		double b = Interpolate(eOtherSide,det,side);
//	double b = 0.01;
//	double b = m_Bfrac[det][side];

//	double DMax = sum*((sum-511./2)/(sum+511./2));
	double E0 = 356;
	double DMax = E0*((E0-511./2)/(E0+511./2));

	double correctedE0 = sum;
//	if (diff >= DMax+0.15*sum){
//	if (diff >= DMax+0.15*E0){
//	if (frac >= 0.5){
//	if (scaled_sum < 0.75){
//	if (sum < 300){
//		correctedE0 = sum/(1/(2-b)*(scaled_sum+sqrt(scaled_sum*scaled_sum - b*(2-b)*frac*frac)));
//		if (sum<eOtherSide){
//		correctedE0 = 1/(2-b)*(sum+sqrt(sum*sum - b*(2-b)*frac*frac));
//		}
/*		if (sum < eOtherSide){
			cout << enOne << '\t' << enTwo << '\t';
			cout << sum << '\t' << correctedE0 << '\t' << eOtherSide << endl;
		}*/
//	}
//	else {
//		correctedE0 = (sum-b*diff)/(1-b);
		if (sum<eOtherSide){
		correctedE0 = (sum-b*frac)/(1-b);
		}
/*		if (sum < eOtherSide-10){
			cout << enOne << '\t' << enTwo << '\t' << sum << '\t';
			cout << correctedE0 << '\t' << eOtherSide << '\t' << endl;
		}*/
//	}
//	}

//	if (correctedE0 > eOtherSide) { cout << correctedE0 << '\t' << eOtherSide << endl; correctedE0 = eOtherSide; }

//	if (correctedE0/662. > 1){ correctedE0 = sum; }

	return correctedE0;

};


////////////////////////////////////////////////////////////////////////////////


void MModuleChargeSharingCorrection::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}

////////////////////////////////////////////////////////////////////////////////

void MModuleChargeSharingCorrection::dummy_func(){

	return;

};


// MModuleChargeSharingCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
