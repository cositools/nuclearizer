/* 
 * ThresholdDeterminator.cxx
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
#include <map>
#include <fstream> ///Cory
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
#include <TMath.h> //Cory
// MEGAlib
#include "MGlobal.h"
#include "MFile.h"
#include "MReadOutElementDoubleStrip.h"
#include "MFileReadOuts.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MReadOutSequence.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutDataTiming.h"
#include "MAssembly.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class ThresholdDeterminator
{
public:
  //! Default constructor
  ThresholdDeterminator();
  //! Default destructor
  ~ThresholdDeterminator();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }
  //! Read next event
  bool ReadNextEvent(MFileReadOuts& ROAFile, MReadOutAssembly* Event);

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The input file name
  MString m_FileName;
	//! The threshold file name
	MString m_THName;
};


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
ThresholdDeterminator::ThresholdDeterminator() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
ThresholdDeterminator::~ThresholdDeterminator()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool ThresholdDeterminator::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: ThresholdDeterminator <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   file name"<<endl;
  Usage<<"         -t:   threshold OUTPUT file name"<<endl;
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

		else if (Option == "-t") {
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
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } else if (Option != "-t") {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    } 
  

    else if (Option == "-t") {
      m_THName = argv[++i];
      cout<<"Accepting threshold OUTPUT file name: "<<m_THName<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


//Cory put fit function here b/c can't define function inside other function
Double_t fitf(Double_t *x, Double_t *par) // define function for fit
{
	Double_t arg1 = 0; //Double_t arg2 = 0;
	if (par[3] != 0) arg1 = (x[0]-par[2]) / (TMath::Sqrt(2.0)*par[3]);
	//if (par[3] != 0) arg2 = (x[0]-par[2]) / par[3];
	Double_t fitval = par[0] +par[1]*TMath::Erf( arg1 );
	return fitval;
}


//! Do whatever analysis is necessary
bool ThresholdDeterminator::Analyze()
{
  if (m_Interrupt == true) return false;

  MFileReadOuts File;
  if (File.Open(m_FileName, MFile::c_Read) == false) return false;
  File.ShowProgress(true);
  
  map<MReadOutElementDoubleStrip, TH1D*> Histograms; 
  
  MReadOutAssembly* Event = new MReadOutAssembly();
  while (ReadNextEvent(File, Event) == true) {
    for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
      MStripHit* SH = Event->GetStripHit(i);
      MReadOutElementDoubleStrip R = *dynamic_cast<MReadOutElementDoubleStrip*>(SH->GetReadOutElement());
      if (Histograms[R] != 0) {
        Histograms[R]->Fill(SH->GetADCUnits());
      } else {
        TH1D* Hist = new TH1D("", MString("Spectrum for ") + R.ToString(), 1000, 0, 1000); //Cory added h then took away
        Histograms[R] = Hist;
      }
    }    
  }
  delete Event;
  
  ofstream histfile; //output file type (make it writeable), class histfile
 	histfile.open(m_THName); 
  if (histfile.is_open() == false) {
    cerr<<"Unable to open thresholds file for writing!"<<endl;
    return false;
  }


  for (auto H: Histograms) {
    TH1D* Hist = H.second;
    
/*   
		TCanvas* C = new TCanvas();
    C->cd();
    Hist->Draw();
    C->Update(); 
*/

//Cory Changes begin here
	int uloc = 1; // declare this outside of for loop blocks to access later
	int lloc = 1;
  for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
      if (Hist->GetBinContent(b)  > 10) { 
				lloc = b;
        histfile<<"TH (LOWER) "<<H.first.ToParsableString(true)<<" "<<Hist->GetBinCenter(b)<<endl; //Cory changed Hist->GetBinCenter(b) to Nlow
        break;
      }
    }
	//Cory upper threshhold

	for (int b = Hist->GetNbinsX(); b >= 0; b = b-1) {
      if (Hist->GetBinContent(b)  > 10 ) { 
				uloc = b;
      	histfile<<"TH (UPPER) "<<H.first.ToParsableString(true)<<" "<<Hist->GetBinCenter(b)<<endl;
        break;
      }
    }
	/* Double_t fitf(Double_t *x, Double_t *par) // define function for fit
    {
      Double_t arg1 = 0; Double_t arg2 = 0;
      if (par[3] != 0) arg1 = (x[0]-par[2]) / (Tmath::Sqrt(2)*par[3]);
      //if (par[3] != 0) arg2 = (x[0]-par[2]) / par[3];
      Double_t fitval = par[0] +par[1]*TMath::Erf( (-1.0)*arg1*arg1 );
		return fitval;
		}*/
	
		TF1 *funcl = new TF1("fit_lower", fitf, lloc-5, lloc+5, 4); //TF1 object for fcn, lloc pm 50 defines range , last # is # of par
    funcl->SetParameters((35/2), (35/2), lloc /*lloc*/, 0.05  /*sigma*/ ); //initialized par vals
    funcl->SetParNames("Offset_Lower", "Scale_factor_Lower", "Mu_Lower", "Sigma_Lower"); //nice names for pars 
    Hist->Fit("fit_lower"); 

		TF1 *funcu = new TF1("fit_upper", fitf, uloc-5, uloc+5, 4); //TF1 object for fcn, lloc pm 50 defines range , last # is # of par
    funcu->SetParameters((35/2), -(35/2), uloc /*uloc*/, 0.05 /*sigma*/ ); //initialized par vals
    funcu->SetParNames("Offset_Upper", "Scale_factor_Upper", "Mu_Upper", "Sigma_Upper"); //nice names for pars 
    Hist->Fit("fit_upper");

//To get additional parameters (errors etc) in threshold file, uncomment this region
/*
   	for (int i=0;i<funcl->GetNpar();i++) {
			histfile<<funcl->GetParName(i)<<" "<<funcl->GetParameter(i)<<endl;
   }

  	 for (int i=0;i<funcu->GetNpar();i++) {
      histfile<<funcu->GetParName(i)<<" "<<funcu->GetParameter(i)<<endl;
   }
*/

  }

  histfile.close();
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


bool ThresholdDeterminator::ReadNextEvent(MFileReadOuts& ROAFile, MReadOutAssembly* Event)
{
  // Return next single event from file... or 0 if there are no more.
  
  Event->Clear();

  ROAFile.ReadNext(*Event);

  if (Event->GetNumberOfReadOuts() == 0) {
    cout<<"No more read-outs available in File"<<endl;
    return false;
  }
  
  for (unsigned int r = 0; r < Event->GetNumberOfReadOuts(); ++r) {
    MReadOut RO = Event->GetReadOut(r);
    const MReadOutElementDoubleStrip* Strip = 
      dynamic_cast<const MReadOutElementDoubleStrip*>(&(RO.GetReadOutElement()));
      
    const MReadOutDataADCValue* ADC = 
      dynamic_cast<const MReadOutDataADCValue*>(RO.GetReadOutData().Get(MReadOutDataADCValue::m_TypeID));
    const MReadOutDataTiming* Timing = 
      dynamic_cast<const MReadOutDataTiming*>(RO.GetReadOutData().Get(MReadOutDataTiming::m_TypeID));
    
    
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(Strip->GetDetectorID());
    SH->IsXStrip(Strip->IsLowVoltageStrip());
    SH->SetStripID(Strip->GetStripID());
    
    SH->SetTiming(Timing->GetTiming());
    SH->SetADCUnits(ADC->GetADCValue());
    Event->AddStripHit(SH);
  }
  
  Event->SetAnalysisProgress(MAssembly::c_EventLoader);
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////


ThresholdDeterminator* g_Prg = 0;
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
  // signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize("Standalone", "a standalone example program");

  TApplication ThresholdDeterminatorApp("ThresholdDeterminatorApp", 0, 0);

  g_Prg = new ThresholdDeterminator();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //ThresholdDeterminatorApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
