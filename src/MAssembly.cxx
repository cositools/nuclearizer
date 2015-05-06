/*
* MAssembly.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MAssembly
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MAssembly.h"

// Standard libs:
#include <iostream>
#include <sstream>
#include <vector>
#include <csignal>
using namespace std;

// ROOT libs:
#include "TROOT.h"
#include "TCanvas.h"
#include "TView.h"
#include "TGMsgBox.h"
#include "TH2.h"
#include "TCanvas.h"
#include "TApplication.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MString.h"
#include "MTimer.h"

// Nuclearizer libs:
#include "MGUIMainNuclearizer.h"
#include "MReadOutAssembly.h"
#include "MModule.h"
#include "MGUIExpoCombinedViewer.h"
#include "MModuleTransmitterRealta.h"

#include "MNCTModuleMeasurementLoaderROA.h"
//#include "MNCTModuleMeasurementLoaderNCT2009.h"
#include "MNCTModuleMeasurementLoaderGRIPS2013.h"
#include "MNCTModuleReceiverCOSI2014.h"
#include "MNCTModuleMeasurementLoaderBinary.h"
#include "MNCTModuleSimulationLoader.h"
#include "MNCTModuleEnergyCalibration.h"
#include "MNCTModuleEnergyCalibrationUniversal.h"
#include "MNCTModuleEnergyCalibrationLinear.h"
#include "MNCTModuleEnergyCalibrationNonlinear.h"
#include "MNCTModuleCrosstalkCorrection.h"
#include "MNCTModuleChargeSharingCorrection.h"
#include "MNCTModuleDepthAndStripCalibration.h"
#include "MNCTModuleDepthCalibration.h"
#include "MNCTModuleDepthCalibrationLinearStrip.h"
#include "MNCTModuleDepthCalibrationLinearPixel.h"
#include "MNCTModuleDepthCalibration3rdPolyPixel.h"
#include "MNCTModuleStripPairing.h"
#include "MNCTModuleStripPairingGreedy.h"
#include "MNCTModuleStripPairingGreedy_a.h"
#include "MNCTModuleStripPairingGreedy_b.h"
#include "MNCTModuleFlagHits.h"
#include "MNCTModuleAspect.h"
#include "MNCTModuleEventFilter.h"
#include "MNCTModuleDumpEvent.h"
#include "MNCTModuleEventSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


MAssembly::MAssembly()
{
  // standard constructor
    
  m_Interrupt = false;
  m_UseGui = true;
  
  m_Supervisor = new MSupervisor();
  
  m_Supervisor->SetConfigurationFileName(gSystem->ConcatFileName(gSystem->HomeDirectory(), ".nuclearizer.cfg"));
  m_Supervisor->UseMultiThreading(true);
  
  m_Supervisor->AddAvailableModule(new MNCTModuleSimulationLoader());
  m_Supervisor->AddAvailableModule(new MNCTModuleMeasurementLoaderROA());
  m_Supervisor->AddAvailableModule(new MNCTModuleMeasurementLoaderGRIPS2013());
  //m_Supervisor->AddAvailableModule(new MNCTModuleMeasurementLoaderNCT2009());
  m_Supervisor->AddAvailableModule(new MNCTModuleReceiverCOSI2014());
  m_Supervisor->AddAvailableModule(new MNCTModuleMeasurementLoaderBinary());
  
  m_Supervisor->AddAvailableModule(new MNCTModuleEventFilter());
  //m_Supervisor->AddAvailableModule(new MNCTModuleEnergyCalibration());
  m_Supervisor->AddAvailableModule(new MNCTModuleEnergyCalibrationUniversal());
  m_Supervisor->AddAvailableModule(new MNCTModuleEnergyCalibrationLinear());
  m_Supervisor->AddAvailableModule(new MNCTModuleEnergyCalibrationNonlinear());
  m_Supervisor->AddAvailableModule(new MNCTModuleCrosstalkCorrection());
  m_Supervisor->AddAvailableModule(new MNCTModuleChargeSharingCorrection());
  //m_Supervisor->AddAvailableModule(new MNCTModuleDepthAndStripCalibration());
  //m_Supervisor->AddAvailableModule(new MNCTModuleDepthCalibration());
  //m_Supervisor->AddAvailableModule(new MNCTModuleDepthCalibrationLinearStrip());
  m_Supervisor->AddAvailableModule(new MNCTModuleDepthCalibrationLinearPixel());
  m_Supervisor->AddAvailableModule(new MNCTModuleDepthCalibration3rdPolyPixel());
  //m_Supervisor->AddAvailableModule(new MNCTModuleStripPairing());
  //m_Supervisor->AddAvailableModule(new MNCTModuleStripPairingGreedy());
  m_Supervisor->AddAvailableModule(new MNCTModuleStripPairingGreedy_a());
  m_Supervisor->AddAvailableModule(new MNCTModuleStripPairingGreedy_b());
	m_Supervisor->AddAvailableModule(new MNCTModuleFlagHits());
  m_Supervisor->AddAvailableModule(new MNCTModuleAspect());
  //m_Supervisor->AddAvailableModule(new MNCTModuleDumpEvent());
  m_Supervisor->AddAvailableModule(new MNCTModuleEventSaver());
  m_Supervisor->AddAvailableModule(new MModuleTransmitterRealta());
  //m_Supervisor->AddAvailableModule(new MNCTModuleEventReconstruction());

  m_Supervisor->Load();
  
  m_Supervisor->SetUIProgramName("Nuclearizer");
  m_Supervisor->SetUIPicturePath("$(NUCLEARIZER)/resource/icons/Nuclearizer.xpm");
  m_Supervisor->SetUISubTitle("A measurement and simulation calibrator for COSI and GRIPS");
  m_Supervisor->SetUILeadAuthor("Andreas Zoglauer");
  m_Supervisor->SetUICoAuthors("Alan Chiu, Alex Lowell, Andreas Zoglauer,\nAres Hernandez, Carolyn Kierans, Clio Sleator,\nDaniel Perez-Becker, Eric Bellm, Jau-Shian Liang,\nMark Bandstra");
}



////////////////////////////////////////////////////////////////////////////////


MAssembly::~MAssembly()
{
  // standard destructor
  
  delete m_Supervisor;
}


////////////////////////////////////////////////////////////////////////////////


bool MAssembly::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: Nuclearizer <options>"<<endl;
  Usage<<endl;
  Usage<<"      -c --configuration <filename>.xml.cfg:"<<endl;
  Usage<<"             Use this file as configuration file."<<endl;
  Usage<<"             If no configuration file is give ~/.nuclearizer.xml.cfg is used"<<endl;
  Usage<<"      -a --auto:"<<endl;
  Usage<<"             Automatically start analysis without GUI"<<endl;
  Usage<<"      -m --multithreading:"<<endl;
  Usage<<"             0: false (default), else: true"<<endl;
  Usage<<"      -v --verbosity:"<<endl;
  Usage<<"             Verbosity: 0: Quiet, 1: Errors, 2: Warnings, 3: Info"<<endl;
  Usage<<"      -h --help:"<<endl;
  Usage<<"             You know the answer..."<<endl;
  Usage<<endl;
  
  // Store some options temporarily:
  MString Option;
  
  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }
  
  // First check if all options are ok:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    
    // Single argument
    if (Option == "-c" || Option == "--configuration" ||
        Option == "-m" || Option == "--multithreading") {
      if (!((argc > i+1) && argv[i+1][0] != '-')){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }
  }
  
  // Now parse all low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--configuration" || Option == "-c") {
      m_Supervisor->Load(argv[++i]);
      cout<<"Command-line parser: Use configuration file "<<argv[i]<<endl;
    } else if (Option == "--verbosity" || Option == "-v") {
      g_Verbosity = atoi(argv[++i]);
      cout<<"Command-line parser: Verbosity "<<g_Verbosity<<endl;
    } else if (Option == "--multithreading" || Option == "-m") {
      m_Supervisor->UseMultiThreading((atoi(argv[i]) != 0 ? true : false));
      cout<<"Command-line parser: Using multithreading: "<<(atoi(argv[i]) != 0 ? "yes" : "no")<<endl;
    } else if (Option == "--auto" || Option == "-a") {
      // Parse later
    }
  }
  
  // Now parse all high level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--auto" || Option == "-a") {
      m_UseGui = false;
      gROOT->SetBatch(true);
      m_Supervisor->Analyze();
      m_Supervisor->Exit();
      return false;
    }
  }
  
  if (m_UseGui == true) {
    m_Supervisor->LaunchUI();
  }
  
  return true;
}


//////////////////////////////////////////////////////////////////////////////////


MAssembly* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C: sent the signal to interrupt, call Ctrl-C again for abort."<<endl;
    g_Prg->SetInterrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! In the beginning Andreas created main and Andreas said "Let there be code!"
//! After many years of coding and debugging, Andreas saw all that he had made, 
//! and it was very good.
int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Catch a user interrupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication* AppNuclearizer = new TApplication("Nuclearizer", 0, 0);

  MAssembly Nuclearizer;
  g_Prg = &Nuclearizer;
  if (Nuclearizer.ParseCommandLine(argc, argv) == false) {
    return 0;
  } else {
    AppNuclearizer->Run();
  }  

  return 0;
}


// MAssembly: the end...
////////////////////////////////////////////////////////////////////////////////
