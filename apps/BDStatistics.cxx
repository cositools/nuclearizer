/* 
 * BDStatistics.cxx
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
#include <iomanip>
#include <map>
#include <fstream>
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
#include "MTokenizer.h"


////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class BDStatistics
{
public:
  //! Default constructor
  BDStatistics();
  //! Default destructor
  ~BDStatistics();
  
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
BDStatistics::BDStatistics() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
BDStatistics::~BDStatistics()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool BDStatistics::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: BDStatistics <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   evta file name"<<endl;
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
bool BDStatistics::Analyze()
{
  if (m_Interrupt == true) return false;

  map<MString, int> BDTypeCounter; 
  
  MFile File;
  if (File.Open(m_FileName) == false) {
    cout<<"Unable to open file: "<<m_FileName<<endl;
    return false;
  }
  
  int LineLength = 10000;
  char* LineBuffer = new char[LineLength];
  MTokenizer Tokenizer;

  bool IsStart = true;
  bool FoundBD = false;

  unsigned int NEvents = 0;
  unsigned int NBDEvents = 0;
  
  MString Line;
  while (File.ReadLine(Line) == true) {
    if (m_Interrupt == true) break;
    
    if (Line.BeginsWith("SE")) {
      if (IsStart == false) {
        ++NEvents;
        if (FoundBD == true) {
          ++NBDEvents;
        }
        FoundBD = false;
      } else {
        IsStart = false;
      }
    }
    if (Line.BeginsWith("BD")) {
      Tokenizer.Analyse(Line);
      if (Tokenizer.GetNTokens() <= 1) continue;
      BDTypeCounter[Tokenizer.GetTokenAtAsString(1)]++;
      FoundBD = true;
    }
  }
  
  cout<<endl;
  cout<<endl;
  cout<<"Events flagged as bad: "<<NBDEvents<<" out of "<<NEvents<<" corresponding to "<<setprecision(2)<<fixed<<100.0*double(NBDEvents)/double (NEvents)<<" %"<<endl;
  cout<<endl;
  cout<<"Distribution of BD flags (one event can have multiple BD flags): "<<endl;
  for (auto I = BDTypeCounter.begin(); I != BDTypeCounter.end(); ++I) {
    cout<<"  "<<(*I).first<<":";
    for (unsigned int i = (*I).first.Length(); i < 38; ++i) cout<<" ";
    cout.width(10); cout<<right<<(*I).second<<" (="<<setw(5)<<setprecision(2)<<fixed<<100.0*double((*I).second)/double (NEvents)<<"%)"<<endl;
  }
  cout<<endl;
  cout<<endl;
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


BDStatistics* g_Prg = 0;
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

  TApplication BDStatisticsApp("BDStatisticsApp", 0, 0);

  g_Prg = new BDStatistics();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  //BDStatisticsApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
