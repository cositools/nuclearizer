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

// MEGAlib
#include "MGlobal.h"
#include "MFile.h"
#include "MReadOutElementDoubleStrip.h"
#include "MFileReadOuts.h"
#include "MReadOutAssembly.h"
#include "MNCTStripHit.h"
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
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    } 
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


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
      MNCTStripHit* SH = Event->GetStripHit(i);
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
  histfile.open("Thresholds.dat"); 
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
    
    for (int b = 1; b <= Hist->GetNbinsX(); ++b) {
      if (Hist->GetBinContent(b)  > 10) { 
        histfile<<"TH "<<H.first.ToParsableString(true)<<" "<<Hist->GetBinCenter(b)<<endl;
        break;
      }
    }
  }

  histfile.close();
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


bool ThresholdDeterminator::ReadNextEvent(MFileReadOuts& ROAFile, MReadOutAssembly* Event)
{
  // Return next single event from file... or 0 if there are no more.
  
  Event->Clear();

  MReadOutSequence ROS;
  ROAFile.ReadNext(ROS);

  if (ROS.GetNumberOfReadOuts() == 0) {
    cout<<"No more read-outs available in File"<<endl;
    return false;
  }
   
  Event->SetID(ROS.GetID());
  Event->SetTime(ROS.GetTime());
  Event->SetCL(ROS.GetClock());
  
  
  for (unsigned int r = 0; r < ROS.GetNumberOfReadOuts(); ++r) {
    MReadOut RO = ROS.GetReadOut(r);
    const MReadOutElementDoubleStrip* Strip = 
      dynamic_cast<const MReadOutElementDoubleStrip*>(&(RO.GetReadOutElement()));
      
    const MReadOutDataADCValue* ADC = 
      dynamic_cast<const MReadOutDataADCValue*>(RO.GetReadOutData().Get(MReadOutDataADCValue::m_TypeID));
    const MReadOutDataTiming* Timing = 
      dynamic_cast<const MReadOutDataTiming*>(RO.GetReadOutData().Get(MReadOutDataTiming::m_TypeID));
    
    
    MNCTStripHit* SH = new MNCTStripHit();
    SH->SetDetectorID(Strip->GetDetectorID());
    SH->IsXStrip(Strip->IsPositiveStrip());
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
