/* 
 * ResponseToXSPEC.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */

// Standard
#include <iostream>
#include <string>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include "fitsio.h"
#include <stdio.h>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1D.h>

// MEGAlib
#include "MGlobal.h"
#include "MResponseMatrixON.h"
#include "MResponseMatrixAxis.h"
#include "MFileEventsTra.h"
#include "MFileEventsSim.h"
#include "MSettingsMimrec.h"
#include "MEventSelector.h"

////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class ResponseToXSPEC
{
public:
  //! Default constructor
  ResponseToXSPEC();
  //! Default destructor
  ~ResponseToXSPEC();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what ever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
	//! collapse 4d matrix to 2d
	MResponseMatrixO2 CollapseMatrix(MResponseMatrixON RspON);
	//! make rmf file
	bool MakeRMF(MResponseMatrixO2 Rsp);
	//! make arf file
	bool MakeARF(MResponseMatrixO2 Rsp);
	//! make pha file
	bool MakePHA(MResponseMatrixO2 Rsp);
	//! read spectrum from tra file
	vector<float> SpectrumFromTra(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);
	//! read spectrum from sim file
	vector<float> SpectrumFromSim(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh);

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
	//! input filename
	MString m_FileName;
	//! input tra or sim file
	MString m_DataFileName;
	//! mimrec config file
	MString m_CfgFileName;
	//! geometry file name
	MString m_GeoFileName;
	//! output file name
	MString m_OutFileName;
	//! power law index for response
	float m_alpha;
	//! minimum energy of response
	float m_Emin;
	//! maximum energy of response
	float m_Emax;
	//! exposure time of pha spectrum
	float m_Exposure;

};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ResponseToXSPEC::ResponseToXSPEC() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
ResponseToXSPEC::~ResponseToXSPEC()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool ResponseToXSPEC::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ResponseToXSPEC <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   rsp file name"<<endl;
	Usage<<"         -t:   tra file"<<endl;
	Usage<<"         -g:   geometry file"<<endl;
	Usage<<"         -c:   mimrec config file"<<endl;
	Usage<<"         -o:   output file name"<<endl;
	Usage<<"         -a:   power law index"<<endl;
	Usage<<"         -e:   minimum simulation energy"<<endl;
	Usage<<"         -E:   maximum simulation energy"<<endl;
	Usage<<"         -x:   exposure time"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-f") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-t") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-g") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
	  if (Option == "-c") {
     if (!((argc > i+1) && 
           (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-o") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-a") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-e") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-E") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
		if (Option == "-x") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 

    // Multiple arguments template
    /*
    else if (Option == "-??") {
      if (!((argc > i+2) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
    */

    // Then fulfill the options:
    if (Option == "-f") {
      m_FileName = argv[++i];
      cout<<"Accepting response matrix file name: "<<m_FileName<<endl;
    }
    else if (Option == "-t") {
      m_DataFileName = argv[++i];
      cout<<"Accepting tra file name: "<<m_DataFileName<<endl;
    }
    else if (Option == "-g") {
      m_GeoFileName = argv[++i];
      cout<<"Accepting geometry file name: "<<m_GeoFileName<<endl;
    }
    else if (Option == "-c") {
      m_CfgFileName = argv[++i];
      cout<<"Accepting mimrec configuration file name: "<<m_CfgFileName<<endl;
    }
    else if (Option == "-o") {
      m_OutFileName = argv[++i];
      cout<<"Accepting output file name: "<<m_OutFileName<<endl;
    }
    else if (Option == "-a") {
      m_alpha = atof(argv[++i]);
      cout<<"Accepting power law index: "<<m_alpha<<endl;
    }
    else if (Option == "-e") {
      m_Emin = atof(argv[++i]);
      cout<<"Accepting minimum energy: "<<m_Emin<<endl;
    }
    else if (Option == "-E") {
      m_Emax = atof(argv[++i]);
      cout<<"Accepting maximum energy: "<<m_Emax<<endl;
    }
    else if (Option == "-x") {
      m_Exposure = atof(argv[++i]);
      cout<<"Accepting exposure time: "<<m_Exposure<<endl;
    }
		else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }

  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool ResponseToXSPEC::Analyze()
{
  if (m_Interrupt == true) return false;

	//load response matrix
	MResponseMatrixON RspON;
	if (!RspON.Read(m_FileName)){
		cout << "failed to read rsp matrix, aborting..." << endl;
		return false;
	}

	MResponseMatrixO2 RspO2 = CollapseMatrix(RspON);

	MakeRMF(RspO2);
	MakeARF(RspO2);

	MakePHA(RspO2);

	return true;

}

MResponseMatrixO2 ResponseToXSPEC::CollapseMatrix(MResponseMatrixON RspON){

		//make 2D response matrix, transfer 4D Rsp to 2D one of just energies
		MResponseMatrixO2 RspO2;

		RspO2.SetSimulatedEvents(RspON.GetSimulatedEvents());
		RspO2.SetFarFieldStartArea(RspON.GetFarFieldStartArea());

		//get theta phi bin: this part will need to be changed when source is moving
		int ThPhBin = RspON.GetAxis(1).GetAxisBin(40,180);
		cout << ThPhBin << endl;

		MResponseMatrixAxis IdealAx = RspON.GetAxis(0);
		MResponseMatrixAxis MeasuredAx = RspON.GetAxis(2);

		if (!(IdealAx.Has1DBinEdges() && MeasuredAx.Has1DBinEdges())){
			cout << "no 1D bin edges...aborting" << endl;
			return RspO2;
		}

		vector<double> idealEdgesD = IdealAx.Get1DBinEdges();
		vector<float> idealEdges(idealEdgesD.begin(), idealEdgesD.end());
		vector<double> measuredEdgesD = MeasuredAx.Get1DBinEdges();
		vector<float> measuredEdges(measuredEdgesD.begin(), measuredEdgesD.end());

		RspO2.SetAxisNames(IdealAx.GetNames()[0], MeasuredAx.GetNames()[0]);
		RspO2.SetAxis(idealEdges,measuredEdges);


		vector<unsigned int> v;
		for (unsigned int i=0; i<IdealAx.GetNumberOfBins(); i++){
			for (unsigned int j=0; j<MeasuredAx.GetNumberOfBins(); j++){
				v.clear();
				v.push_back(i);
				v.push_back(ThPhBin);
				v.push_back(j);

				float binContent = RspON.Get(v);
				RspO2.SetBinContent(i,j,binContent);
			}
		}


		return RspO2;

}


bool ResponseToXSPEC::MakeRMF(MResponseMatrixO2 Rsp){

	//(1) Normalize rsp: each row normalized to 1 detected photon
	int nBinsMeasured = Rsp.GetAxisBins(2);
	int nBinsIdeal = Rsp.GetAxisBins(1);

	int sum;
	for (int x=0; x<nBinsIdeal; x++){
		sum = 0;
		for (int y=0; y<nBinsMeasured; y++){
			sum += Rsp.GetBinContent(x,y);
		}
		for (int y=0; y<nBinsMeasured; y++){
			float content = Rsp.GetBinContent(x,y);
			if (content != 0){
				content = content/sum;
				Rsp.SetBinContent(x,y,content);
			}
		}
	}

	
/*	int sum;
	for (int y=0; y<nBinsMeasured; y++){
		sum = 0;
		for (int x=0; x<nBinsIdeal; x++){
			sum += Rsp.GetBinContent(x,y);
		}
		if (y == 500){ cout << sum << endl; }
		for (int x=0; x<nBinsIdeal; x++){
			float content = Rsp.GetBinContent(x,y);
			if (content != 0){
				content = content/sum;
				Rsp.SetBinContent(x,y,content);
			}
		}
	}
*/


	//(2) Make arrays for EBOUNDS and MATRIX
	//EBOUNDS defines channels
	//Just use measured energy bins as defined in Rsp
	vector<int> channels;
	vector<float> eMin; vector<float> eMax;

	for (int y=0; y<nBinsMeasured; y++){
		channels.push_back(y);
		eMin.push_back(Rsp.GetAxisLowEdge(y,2));
		eMax.push_back(Rsp.GetAxisHighEdge(y,2));
//		cout << Rsp.GetAxisLowEdge(y,2) << '\t' << Rsp.GetAxisHighEdge(y,2) << ":" << '\t';
//		cout << Rsp.GetAxisHighEdge(y,2) - Rsp.GetAxisLowEdge(y,2) << endl;
	}

	//MATRIX is a bit more complicated...
	vector<float> eLow; vector<float> eHigh;
	vector<int> Ngrp;
	vector<vector<int> > Fchan; vector<vector<int> > Nchan;
	vector<vector<float> > Mat;

	for (int x=0; x<nBinsIdeal; x++){

		bool inSubset = false; //keeps track if we are in the middle of a channel subset
		int Ngrp_temp = 0; //counts number of channel subsets
		vector<int> Fchan_temp; vector<int> Nchan_temp;
		vector<float> Mat_temp;

		for (int y=0; y<nBinsMeasured; y++){
			float value = Rsp.GetBinContent(x,y);
			//I think following two lines make things worse..
//			float bin = Rsp.GetAxisHighEdge(x,1)-Rsp.GetAxisLowEdge(x,1);
//			value = value / bin;
			if (inSubset && value != 0){
				Mat_temp.push_back(value);
			}
			else if (inSubset && value == 0){
				Nchan_temp.push_back(y-Fchan_temp.at(Fchan_temp.size()-1));
				inSubset = false;
			}
			else if (!inSubset && value != 0){
				Ngrp_temp++;
				Fchan_temp.push_back(y);
				Mat_temp.push_back(value);
				inSubset = true;
			}
			else {continue; }
		}

		eLow.push_back(Rsp.GetAxisLowEdge(x,1));
		eHigh.push_back(Rsp.GetAxisHighEdge(x+1,1));

		//so that there aren't empty values
		if (Ngrp_temp == 0){
			Fchan_temp.push_back(0);
			Nchan_temp.push_back(0);
			Mat_temp.push_back(0);
		}

		Ngrp.push_back(Ngrp_temp);
		Fchan.push_back(Fchan_temp);
		Nchan.push_back(Nchan_temp);
		Mat.push_back(Mat_temp);

/*		if (Ngrp_temp == 0){
			for (int y=0; y<nBinsMeasured; y++){
				if (Rsp.GetBinContent(x,y) != 0){
					cout << "PROBLEM" << endl;
				}
			}
		}

		if (!(Fchan_temp.size() == Nchan_temp.size() && Fchan_temp.size() == Ngrp_temp)){
			cout << "FALSE" << endl;
		}
		int s = 0;
		for (int i=0; i<Nchan_temp.size(); i++){
			s += Nchan_temp.at(i);
		}
		if (s != Mat_temp.size()){ cout << "false" << endl; }
*/

	}

/*	for (int i=0; i<Ngrp.size(); i++){
		if (Ngrp.at(i) != 0){ cout << i << endl; }
	}*/

	//(3) Make and fill FITS file
	fitsfile *rmf = NULL;

	//almost all cfitsio routines have status parameter as argument
	//if it's not set to 0, the routine will fail
	int status = 0;
	if (fits_create_file(&rmf, (m_OutFileName+MString(".rmf")).Data(), &status)){
		cout << "error in fits_create_file" << endl;
		fits_report_error(stdout, status);
	}

	// (3a) create image as primary (for some reason all responses have this
	status = 0;
	long ax[2];
	ax[0] = 0; ax[1] = 0;
	if (fits_create_img(rmf, FLOAT_IMG, 0, ax, &status)){
		cout << "error in fits_create_img" << endl;
		fits_report_error(stdout, status);
	}

	//add keywords to header
	status = 0;
	if (fits_write_key(rmf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(rmf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	// (3b) make two binary extensions: one for matrix, one for channel definitions

	//make MATRIX binary extension: redistribution matrix
	status = 0;
	const char* m_column_names[] = {"ENERG_LO", "ENERG_HI", "N_GRP", "F_CHAN", "N_CHAN", "MATRIX"};
	const char* m_format[] = {"E", "E", "I", "PI", "PI", "PE"}; //not sure about last 3
	const char* m_units[] = {"keV", "keV", "", "", "", ""};
	int nrows = eLow.size();

	if (fits_create_tbl(rmf, BINARY_TBL, nrows, 6, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"MATRIX", &status)){
		cout << "error in fits_create_tbl" << endl;
		fits_report_error(stdout, status);
	}

	//1D arrays are easier
	float* eLowArr = &eLow[0];
	float* eHighArr = &eHigh[0];
	int* NgrpArr = &Ngrp[0];

	if (fits_write_col(rmf, TFLOAT, 1, 1, 1, nrows, eLowArr, &status)){
		cout << "error in fits_write_col: MATRIX" << endl;
		fits_report_error(stdout, status);
	}

	if (fits_write_col(rmf, TFLOAT, 2, 1, 1, nrows, eHighArr, &status)){
		cout << "error in fits_write_col: MATRIX" << endl;
		fits_report_error(stdout, status);
	}

	if (fits_write_col(rmf, TINT, 3, 1, 1, nrows, NgrpArr, &status)){
		cout << "error in fits_write_col: MATRIX" << endl;
		fits_report_error(stdout, status);
	}

	for (int i=255; i<265; i++){
		int* test = &Fchan[i][0];
		for (unsigned int j=0; j<Fchan.at(i).size(); j++){
			cout << Fchan.at(i).at(j) << '\t';
			cout << test[j] << '\t';
		}
		cout << endl;
	}

	//2d arrays: have to go one element at a time
	int size;
	int* FchanArr;
	int* NchanArr;
	float* MatArr;
	for (int i=0; i<nrows; i++){

		//Fchan
		FchanArr = &Fchan[i][0];
		size = Fchan[i].size();

		status = 0;
		if (fits_write_col(rmf, TINT, 4, i+1, 1, size, FchanArr, &status)){
			cout << "error in fits_write_col: MATRIX" << endl;
			fits_report_error(stdout, status);
		}

		//Nchan
		NchanArr = &Nchan[i][0];
		size = Fchan[i].size();

		status = 0;
		if (fits_write_col(rmf, TINT, 5, i+1, 1, size, NchanArr, &status)){
			cout << "error in fits_write_col: MATRIX" << endl;
			fits_report_error(stdout, status);
		}

		//Mat
		MatArr = &Mat[i][0];
		size = Mat[i].size();

		status = 0;
		if (fits_write_col(rmf, TFLOAT, 6, i+1, 1, size, MatArr, &status)){
			cout << "error in fits_write_col: MATRIX" << endl;
			fits_report_error(stdout, status);
		}

	}


	//MATRIX header keywords
	if (fits_write_key(rmf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "mission/satellite name", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "instrument/detector name", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"FILTER", (char*)"NONE", "filter in use", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"CHANTYPE", (char*)"PI", "channel type (PHA or PI)", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	int detchans = channels.size();
	if (fits_write_key(rmf, TINT, (char*)"DETCHANS", &detchans, "total number of detector channels", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLASS", (char*)"OGIP", "format conforms to OGIP standard", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLAS1", (char*)"RESPONSE", "dataset relates to spectral response", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLAS2", (char*)"RSP_MATRIX", "dataset is a spectral response matrix", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUVERS", (char*)"1.3.0", "version of format", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}
	int number = 0;
	if (fits_write_key(rmf, TINT, (char*)"TLMIN4", &number, "minimum value legally allowed in column 4", &status)){
		cout << "error in fits_write_key: MATRIX header" << endl;
		fits_report_error(stdout, status);
	}




	//make EBOUNDS binary extension: to define 'channels'
	//COSI doesn't have channels, so we use energy bins
	status = 0;
	nrows = channels.size();
	const char* e_column_names[] = {"CHANNEL","E_MIN","E_MAX"};
	const char* e_format[] = {"I","E","E"};
	const char* e_units[] = {"","keV","keV"};

	if (fits_create_tbl(rmf, BINARY_TBL, nrows, 3, (char**)e_column_names, (char**)e_format, (char**)e_units, (char*)"EBOUNDS", &status)){
		cout << "error in fits_create_tbl" << endl;
		fits_report_error(stdout, status);
	}

	//add values to table
	int* channelArr = &channels[0];
	float* eMinArr = &eMin[0];
	float* eMaxArr = &eMax[0];

	status = 0;
	if (fits_write_col(rmf, TINT, 1, 1, 1, nrows, channelArr, &status)){
		cout << "error in fits_write_col: EBOUNDS table" << endl;
		fits_report_error(stdout, status);
	}

	if (fits_write_col(rmf, TFLOAT, 2, 1, 1, nrows, eMinArr, &status)){
		cout << "error in fits_write_col: EBOUNDS table" << endl;
		fits_report_error(stdout, status);
	}

	if (fits_write_col(rmf, TFLOAT, 3, 1, 1, nrows, eMaxArr, &status)){
		cout << "error in fits_write_col: EBOUNDS table" << endl;
		fits_report_error(stdout, status);
	}


	//EBOUNDS header keywords
	if (fits_write_key(rmf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "mission/satellite", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "instrument/detector name", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"FILTER", (char*)"NONE", "filter in use", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"CHANTYPE", (char*)"PI", "channel type (PHA or PI)", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TINT, (char*)"DETCHANS", &detchans, "total number of detector channels", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLASS", (char*)"OGIP", "format conforms to OGIP standard", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLAS1", (char*)"RESPONSE", "dataset relates to spectral response", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUCLAS2", (char*)"EBOUNDS", "dataset is a spectral response matrix", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}
	if (fits_write_key(rmf, TSTRING, (char*)"HDUVERS", (char*)"1.2.0", "version of format", &status)){
		cout << "error in fits_write_key: EBOUNDS header" << endl;
		fits_report_error(stdout, status);
	}



	status = 0;
	if (fits_close_file(rmf, &status)){
		cout << "error in fits_close_file" << endl;
		fits_report_error(stdout, status);
	}
	cout << "file closed" << endl;

  return true;
}


bool ResponseToXSPEC::MakeARF(MResponseMatrixO2 Rsp){

	int nBinsMeasured = Rsp.GetAxisBins(2);
	int nBinsIdeal = Rsp.GetAxisBins(1);

	int sum = 0;
	for (int x=0; x<nBinsIdeal; x++){
		sum += Rsp.GetBinContent(x,500);
	}
	cout << sum << endl;


	//(1) Define channels; figure out how many Nstart counts are in each channel
	vector<float> eLow; vector<float> eHigh;

	for (int y=0; y<nBinsMeasured; y++){
		eLow.push_back(Rsp.GetAxisLowEdge(y,2));
		eHigh.push_back(Rsp.GetAxisHighEdge(y,2));
	}

	//will need to rewrite this part based on what Andreas puts in RSP
	int nChan = eLow.size();
	double NStart = Rsp.GetSimulatedEvents();
	double AStart = Rsp.GetFarFieldStartArea();

	cout << "NStart: " << NStart << endl;
	cout << "AStart: " << AStart << endl;
	cout << "Alpha: " << m_alpha << endl;
	cout << "Emin: " << m_Emin << endl;
	cout << "Emax: " << m_Emax << endl;

	//assume f(E)dE = K(E^-a)dE
	//need to find K
	float K = NStart*(1-m_alpha)/(pow(m_Emax,1-m_alpha)-pow(m_Emin,1-m_alpha));

	vector<float> NStartPerChan;
	for (int c=0; c<nChan; c++){
		float deltaE = eHigh.at(c)-eLow.at(c);
		//set energy to middle of bin?
		float E = eHigh.at(c)-deltaE/2.;
		float N = K*pow(E,-m_alpha)*deltaE;
		NStartPerChan.push_back(N);
	}

	//(2) Make effective area array
	vector<float> Aeff;
	for (int x=0; x<nBinsIdeal; x++){
		float nMeasuredCounts = 0;
		for (int y=0; y<nBinsMeasured; y++){
			nMeasuredCounts += Rsp.GetBinContent(x,y);
		}
		Aeff.push_back(AStart*nMeasuredCounts/NStartPerChan.at(x));
//		Aeff.push_back(NStartPerChan.at(y));
//		Aeff.push_back(nMeasuredCounts/deltaE);
	}

	cout << nBinsMeasured << '\t' << nChan << '\t' << nBinsIdeal << endl;

	ofstream ofs("arf.txt",ofstream::out);
	for (int c=0; c<nChan; c++){
		ofs << eHigh.at(c) << '\t' << Aeff.at(c) << endl;
	}
	ofs.close();

	//(3) Make and fill FITS file
	fitsfile *arf = NULL;

	//almost all cfitsio routines have status parameter as argument
	//if it's not set to 0, the routine will fail
	int status = 0;
	if (fits_create_file(&arf, (m_OutFileName+MString(".arf")).Data(), &status)){
		cout << "error in fits_create_file" << endl;
		fits_report_error(stdout, status);
	}

	// (3a) create image as primary (for some reason all responses have this
	status = 0;
	long ax[2];
	ax[0] = 0; ax[1] = 0;
	if (fits_create_img(arf, FLOAT_IMG, 0, ax, &status)){
		cout << "error in fits_create_img" << endl;
		fits_report_error(stdout, status);
	}

	//add keywords to header
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	// (3b) make binary table
	const char* m_column_names[] = {"ENERG_LO", "ENERG_HI", "SPECRESP"};
	const char* m_format[] = {"E", "E", "E"};
	const char* m_units[] = {"keV", "keV", "cm**2"};
	int nrows = eLow.size();

	status = 0;
	if (fits_create_tbl(arf, BINARY_TBL, nrows, 3, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"SPECRESP", &status)){
		cout << "error in fits_create_tbl" << endl;
		fits_report_error(stdout, status);
	}


	// (3c) fill table
	float* eLowArr = &eLow[0];
	float* eHighArr = &eHigh[0];
	float* AeffArr = &Aeff[0];

	status = 0;
	if (fits_write_col(arf, TFLOAT, 1, 1, 1, nrows, eLowArr, &status)){
		cout << "error in fits_write_col: SPECRESP table" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_col(arf, TFLOAT, 2, 1, 1, nrows, eHighArr, &status)){
		cout << "error in fits_write_col: SPECRESP table" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_col(arf, TFLOAT, 3, 1, 1, nrows, AeffArr, &status)){
		cout << "error in fits_write_col: SPECRESP table" << endl;
		fits_report_error(stdout, status);
	}

	//table header
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"TELESCOP", (char*)"COSI", "mission/satellite name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"INSTRUME", (char*)"COSI", "detector/instrument name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"EXTNAME", (char*)"SPECRESP", "name of this binary extension", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"FILTER", (char*)"NONE", "filter in use", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"HDUCLAS1", (char*)"RESPONSE", "dataset relates to spectral response", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"HDUCLAS2", (char*)"SPECRESP", "extension contains an ARF", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(arf, TSTRING, (char*)"HDUVERS", (char*)"1.1.0", "version of format", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}



	status = 0;
	if (fits_close_file(arf, &status)){
		cout << "error in fits_close_file" << endl;
		fits_report_error(stdout, status);
	}
	cout << "file closed" << endl;


	return true;

}

vector<float> ResponseToXSPEC::SpectrumFromTra(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	//return this vector if something is wrong with one of the files
	vector<float> returnIfBad;

	//initialize mimrec stuff
	//load geometry
	MDGeometryQuest* Geometry = new MDGeometryQuest();
	if (Geometry->ScanSetupFile(m_GeoFileName) == true){
		cout << "Geometry " << Geometry->GetName() << " loaded!" << endl;
	}
	else{
		cout << "Unable to load geometry " << Geometry->GetName() << " - Aborting!" << endl;
		return returnIfBad;
	}

	//mimrec settings
	MSettingsMimrec MimrecSettings;
	if (MimrecSettings.Read(m_CfgFileName) == false){
		cout << "Unable to open mimrec configuration file " << m_CfgFileName;
		cout << " - Aborting!" << endl;
		return returnIfBad;
	}
	MEventSelector MimrecEventSelector;
	MimrecEventSelector.SetSettings(&MimrecSettings);

	MFileEventsTra EventFile;
	if (EventFile.Open(m_DataFileName) == false){
		cout << "Unable to open file " << m_DataFileName<<" - Aborting!" << endl;
		return returnIfBad;
	}


	//make energy-vs-channel array
	int nChans = eLow.size();
	vector<float> energy(nChans,0.);

	EventFile.ShowProgress();

	MPhysicalEvent* Event;

	while ((Event = EventFile.GetNextEvent()) != 0){
		if (Event->GetType() == MPhysicalEvent::c_Compton || Event->GetType() == MPhysicalEvent::c_Photo){
			if (MimrecEventSelector.IsQualifiedEvent(Event) == true){
				double E = Event->GetEnergy();

				for (int c=0; c<nChans; c++){
					if (E >= eLow.at(c) && E < eHigh.at(c)){
						energy[c] += 1;
						break;
					}
				}
			}
		}
	}

	ofstream ofs("spec.txt",ofstream::out);
	for (int c=0; c<nChans; c++){
		ofs << eLow.at(c) << '\t' << energy[c] << endl;
	}
	ofs.close();

	return energy;

}

vector<float> ResponseToXSPEC::SpectrumFromSim(MResponseMatrixO2 Rsp, vector<float> eLow, vector<float> eHigh){

	vector<float> returnIfBad;

	//load geometry
	MDGeometryQuest* Geometry = new MDGeometryQuest();
	if (Geometry->ScanSetupFile(m_GeoFileName) == true){
		cout << "Geometry " << Geometry->GetName() << " loaded!" << endl;
	}
	else{
		cout << "Unable to load geometry " << Geometry->GetName() << " - Aborting!" << endl;
		return returnIfBad;
	}

	MFileEventsSim EventFile(Geometry);
	if (EventFile.Open(m_DataFileName) == false){
		cout << "Unable to open file " << m_DataFileName<<" - Aborting!" << endl;
		return returnIfBad;
	}

	int nChans = eLow.size();
	vector<float> energy(nChans, 0);
	MSimEvent* Event;

	while ((Event = EventFile.GetNextEvent()) != 0){
		double E = Event->GetTotalEnergyDeposit();

		for (int c=0; c<nChans; c++){
			if (E >= eLow.at(c) && E < eHigh.at(c)){
				energy[c] += 1;
				break;
			}
		}
	}

	ofstream ofs("spec.txt",ofstream::out);
	for (int c=0; c<nChans; c++){
		ofs << eLow.at(c) << '\t' << energy[c] << endl;
	}
	ofs.close();

	return energy;

}

bool ResponseToXSPEC::MakePHA(MResponseMatrixO2 Rsp){

	int nBinsMeasured = Rsp.GetAxisBins(2);

	//get channels from response matrix
	vector<int> channels;
	vector<float> eLow; vector<float> eHigh;

	for (int y=0; y<nBinsMeasured; y++){
		channels.push_back(y);
		eLow.push_back(Rsp.GetAxisLowEdge(y,2));
		eHigh.push_back(Rsp.GetAxisHighEdge(y,2));
	}

	//make energy-vs-channel array
	int nChans = channels.size();

	//figure out if file is tra or or sim
	vector<float> energy;
	vector<MString> SplitFName = m_DataFileName.Tokenize(".");
	if (SplitFName.at(SplitFName.size()-1) == "sim" || SplitFName.at(SplitFName.size()-2) == "sim"){
		energy = SpectrumFromSim(Rsp, eLow, eHigh);
	}
	else{
		energy = SpectrumFromTra(Rsp, eLow, eHigh);
	}

	/***************************************************/
	//make pha fits file
	fitsfile *pha = NULL;

	//used for keywords that aren't strings
	float number;

	//almost all cfitsio routines have status parameter as argument
	//if it's not set to 0, the routine will fail
	int status = 0;
	if (fits_create_file(&pha, (m_OutFileName+MString(".pha")).Data(), &status)){
		cout << "error in fits_create_file" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	long ax[2];
	ax[0] = 0; ax[1] = 0;
	if (fits_create_img(pha, FLOAT_IMG, 0, ax, &status)){
		cout << "error in fits_create_img" << endl;
		fits_report_error(stdout, status);
	}

	//add keywords to header
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"TELESCOP", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"INSTRUME", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	//make binary table
	const char* m_column_names[] = {"CHANNEL", "COUNTS"};
	const char* m_format[] = {"J", "J"};
	const char* m_units[] = {"", "count"};
	int nrows = nChans;

	status = 0;
	if (fits_create_tbl(pha, BINARY_TBL, nrows, 2, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"SPECTRUM", &status)){
		cout << "error in fits_create_tbl" << endl;
		fits_report_error(stdout, status);
	}


	//fill table
	int* chanArr = &channels[0];
	float* energyArr = &energy[0];

	status = 0;
	if (fits_write_col(pha, TINT, 1, 1, 1, nrows, chanArr, &status)){
		cout << "error in fits_write_col: pha SPECTRUM table" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_col(pha, TFLOAT, 2, 1, 1, nrows, energyArr, &status)){
		cout << "error in fits_write_col: pha SPECTRUM table" << endl;
		fits_report_error(stdout, status);
	}

	//keywords to binary table header
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"TELESCOP", (char*)"COSI", "telescope/mission name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"INSTRUME", (char*)"COSI", "instrument/detector name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"EXTNAME", (char*)"SPECTRUM", "name of extension", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"FILTER", (char*)"NONE", "filter type if any", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: have exposure time NOT be hard coded
	status = 0;
	if (fits_write_key(pha, TFLOAT, (char*)"EXPOSURE", &m_Exposure, "integration time in seconds", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: background file name NOT hard coded
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"BACKFILE", (char*)(m_OutFileName+MString("_bk.pha")).Data(), "background filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 1.0;
	if (fits_write_key(pha, TFLOAT, (char*)"BACKSCAL", &number, "background scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"CORRFILE", (char*)"NONE", "associated correction filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: check this!
	status = 0;
	number = 1.0;
	if (fits_write_key(pha, TFLOAT, (char*)"CORRSCAL", &number, "correction file scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"RESPFILE", (char*)(m_OutFileName+MString(".rmf")).Data(), "associated rmf filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"ANCRFILE", (char*)(m_OutFileName+MString(".arf")).Data(), "associated arf filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: check this
	status = 0;
	number = 1.0;
	if (fits_write_key(pha, TFLOAT, (char*)"AREASCAL", &number, "area scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(pha, TINT, (char*)"STAT_ERR", &number, "no statistical error specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(pha, TINT, (char*)"SYS_ERR", &number, "no systematic error specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(pha, TINT, (char*)"GROUPING", &number, "no grouping of the data has been defined", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(pha, TINT, (char*)"QUALITY", &number, "no data quality information specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}


	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"HDUCLASS", (char*)"OGIP", "format conforms to OGIP standard", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"HDUCLAS1", (char*)"SPECTRUM", "PHA dataset", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"HDUVERS", (char*)"1.2.1", "version of format", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: check this
	status = 0;
	bool P = true;
	if (fits_write_key(pha, TLOGICAL, (char*)"POISSERR", &P, "Poissonian errors to be assumed", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TSTRING, (char*)"CHANTYPE", (char*)"PI", "channel type (PHA or PI)", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(pha, TINT, (char*)"DETCHANS", &nChans, "total number of detector channels", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}




	status = 0;
	if (fits_close_file(pha, &status)){
		cout << "error in fits_close_file" << endl;
		fits_report_error(stdout, status);
	}
	cout << "pha file closed" << endl;



	/***************************************************/
	//make background pha file
	fitsfile *bkg = NULL;

	//almost all cfitsio routines have status parameter as argument
	//if it's not set to 0, the routine will fail
	status = 0;
	if (fits_create_file(&bkg, (m_OutFileName+MString("_bk.pha")).Data(), &status)){
		cout << "error in fits_create_file" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_create_img(bkg, FLOAT_IMG, 0, ax, &status)){
		cout << "error in fits_create_img" << endl;
		fits_report_error(stdout, status);
	}

	//add keywords to header
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"TELESCOP", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"INSTRUME", (char*)"COSI", "", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	//make binary table
	status = 0;
	if (fits_create_tbl(bkg, BINARY_TBL, nrows, 2, (char**)m_column_names, (char**)m_format, (char**)m_units, (char*)"SPECTRUM", &status)){
		cout << "error in fits_create_tbl" << endl;
		fits_report_error(stdout, status);
	}


	//fill table
	float bkEnArr[nChans] = {0};

	status = 0;
	if (fits_write_col(bkg, TINT, 1, 1, 1, nrows, chanArr, &status)){
		cout << "error in fits_write_col: bkg SPECTRUM table" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_col(bkg, TFLOAT, 2, 1, 1, nrows, bkEnArr, &status)){
		cout << "error in fits_write_col: bkg SPECTRUM table" << endl;
		fits_report_error(stdout, status);
	}

	//keywords to binary table header
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"TELESCOP", (char*)"COSI", "telescope/mission name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"INSTRUME", (char*)"COSI", "instrument/detector name", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"EXTNAME", (char*)"SPECTRUM", "name of extension", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"FILTER", (char*)"NONE", "filter type if any", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: have exposure time NOT be hard coded
	status = 0;
	if (fits_write_key(bkg, TFLOAT, (char*)"EXPOSURE", &m_Exposure, "integration time in seconds", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: background file name NOT hard coded
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"BACKFILE", (char*)"NONE", "background filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 1.0;
	if (fits_write_key(bkg, TFLOAT, (char*)"BACKSCAL", &number, "background scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"CORRFILE", (char*)"NONE", "associated correction filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: check this!
	status = 0;
	number = 1.0;
	if (fits_write_key(bkg, TFLOAT, (char*)"CORRSCAL", &number, "correction file scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"RESPFILE", (char*)"NONE", "associated rmf filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"ANCRFILE", (char*)"NONE", "associated arf filename", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 1.0;
	if (fits_write_key(bkg, TFLOAT, (char*)"AREASCAL", &number, "area scaling factor", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(bkg, TINT, (char*)"STAT_ERR", &number, "no statistical error specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(bkg, TINT, (char*)"SYS_ERR", &number, "no systematic error specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(bkg, TINT, (char*)"GROUPING", &number, "no grouping of the data has been defined", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	number = 0;
	if (fits_write_key(bkg, TINT, (char*)"QUALITY", &number, "no data quality information specified", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}

	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"HDUCLASS", (char*)"OGIP", "format conforms to OGIP standard", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"HDUCLAS1", (char*)"SPECTRUM", "PHA dataset", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"HDUVERS", (char*)"1.2.1", "version of format", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	//TODO: check this
	status = 0;
	if (fits_write_key(bkg, TLOGICAL, (char*)"POISSERR", &P, "Poissonian errors to be assumed", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TSTRING, (char*)"CHANTYPE", (char*)"PI", "channel type (PHA or PI)", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}
	status = 0;
	if (fits_write_key(bkg, TINT, (char*)"DETCHANS", &nChans, "total number of detector channels", &status)){
		cout << "error in fits_write_key" << endl;
		fits_report_error(stdout, status);
	}




	status = 0;
	if (fits_close_file(bkg, &status)){
		cout << "error in fits_close_file" << endl;
		fits_report_error(stdout, status);
	}
	cout << "bkg file closed" << endl;



	return true;

}


////////////////////////////////////////////////////////////////////////////////


ResponseToXSPEC* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Main program
int main(int argc, char** argv)
{
  // Catch a user interupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Standalone", "a standalone example program");

  TApplication ResponseToXSPECApp("ResponseToXSPECApp", 0, 0);

  g_Prg = new ResponseToXSPEC();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

	if (gROOT->IsBatch() == false) {
  	ResponseToXSPECApp.Run();
	}

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
