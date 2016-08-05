/* 
 * MNCTDetectorEffectsEngineCOSI.cxx
 *
 *
 * Copyright (C) by Alex Lowell, Cori Gerrity, Carolyn Kierans, Clio Sleator, Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * the copyright holders.
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
using namespace std;

// ROOT
#include <TApplication.h>

// MEGAlib
#include "MGlobal.h"

// Nuclearizer
#include "MNCTDetectorEffectsEngineCOSI.h"


/******************************************************************************/

MNCTDetectorEffectsEngineCOSI* g_Prg = 0;
bool g_Interrupt = false;

/******************************************************************************/



/******************************************************************************
 * Parse the command line
 */
bool ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<"dee - The COSI detector effects engine"<<endl;
  Usage<<endl;
  Usage<<"  Usage: dee <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -f:   simulation file name"<<endl;
  Usage<<"         -g:   geometry file name"<<endl;
  Usage<<"         -e:   energy calibration file name"<<endl;
  Usage<<"         -t:   thresholds file name"<<endl;
  Usage<<"         -d:   dead strip file name"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<"         -D:   depth calibration coefficients filename"<<endl;
  Usage<<"         -s:   depth calibration splines file"<<endl;
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
    if (Option == "-f" || Option == "-o") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    // Multiple arguments_
    //else if (Option == "-??") {
    //  if (!((argc > i+2) && argv[i+1][0] != '-' && argv[i+2][0] != '-')){
    //    cout<<"Error: Option "<<argv[i][1]<<" needs two arguments!"<<endl;
    //    cout<<Usage.str()<<endl;
    //    return false;
    //  }
    //}

    // Then fulfill the options:
    if (Option == "-f") {
      g_Prg->SetSimulationFileName(argv[++i]);
      MString OutputFileName = argv[i];
      OutputFileName.ReplaceAllInPlace(".sim", ".roa");
      g_Prg->SetRoaFileName(OutputFileName);
      cout<<"Accepting simulation file name: "<<argv[i]<<endl;
      cout<<"Accepting roa file name: "<<OutputFileName<<endl;      
    } else if (Option == "-g") {
      g_Prg->SetGeometryFileName(argv[++i]);
      cout<<"Accepting geometry file name: "<<argv[i]<<endl;
    } else if (Option == "-e") {
      g_Prg->SetEnergyCalibrationFileName(argv[++i]);
      cout << "Accepting energy calibration file name: "<<argv[i]<<endl;
    } else if (Option == "-t") {
      g_Prg->SetThresholdFileName(argv[++i]);
      cout << "Accepting threshold file name: "<<argv[i]<<endl;
    } else if (Option == "-d") {
      g_Prg->SetDeadStripFileName(argv[++i]);
      cout << "Accepting dead strip file name: "<<argv[i]<<endl;
    } else if (Option == "-D"){
      g_Prg->SetDepthCalibrationCoeffsFileName(argv[++i]);
      cout << "Accepting depth calibration coefficients file name: "<<argv[i]<<endl;
    } else if (Option == "-s"){
      g_Prg->SetDepthCalibrationSplinesFileName(argv[++i]);
      cout << "Accepting depth calibration splines file name: "<<argv[i]<<endl;
    } else {
      cout<<"Error: Unknown option \""<<Option<<"\"!"<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  return true;
}


/******************************************************************************
 * Called when an interrupt signal is flagged
 * All catched signals lead to a well defined exit of the program
 */
void CatchSignal(int a)
{
  cout<<"Catched signal Ctrl-C:"<<endl;

  if (g_Interrupt == true) {
    cout<<"Aborting..."<<endl;
    abort();
  }
  
  g_Interrupt = true;
  cout<<"Trying to cancel the analysis..."<<endl;
  cout<<"If you hit Ctrl-C again, then I will abort immediately!"<<endl;
}


/******************************************************************************
 * Main program
 */
int main(int argc, char** argv)
{
  // Set a default error handler and catch some signals...
  signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication MNCTDetectorEffectsEngineCOSIApp("MNCTDetectorEffectsEngineCOSIApp", 0, 0);

  g_Prg = new MNCTDetectorEffectsEngineCOSI();

  if (ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  }
  
  g_Prg->ShowProgressBar(true);
  
  if (g_Prg->Initialize() == false) {
    cerr<<"Initialize failed!"<<endl;
    return -2;    
  }
  
  MReadOutAssembly ROA;
  
  while (g_Interrupt == false && g_Prg->GetNextEvent(&ROA) == true) {
    ROA.Clear();
  }
  
  g_Prg->Finalize();
  
  //MNCTDetectorEffectsEngineCOSIApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}

/*
 * Cosima: the end...
 ******************************************************************************/
