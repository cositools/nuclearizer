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
#include "TRandom.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"
#include "MStreams.h"
#include "MString.h"
#include "MTimer.h"
#include "MFile.h"

// Nuclearizer libs:
#include "MReadOutAssembly.h"
#include "MModule.h"
#include "MGUIExpoCombinedViewer.h"
#include "MModuleTransmitterRealta.h"

#include "MModuleLoaderSimulationsBalloon.h"
#include "MModuleLoaderSimulationsSMEX.h"
#include "MModuleLoaderMeasurementsROA.h"
#include "MModuleLoaderMeasurementsHDF.h"
#include "MModuleReceiverBalloon.h"
#include "MModuleLoaderMeasurementsBinary.h"
#include "MModuleEnergyCalibration.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MModuleCrosstalkCorrection.h"
#include "MModuleChargeSharingCorrection.h"
#include "MModuleDepthCalibration.h"
#include "MModuleDepthCalibrationB.h"
#include "MModuleDepthCalibration2024.h"
#include "MModuleStripPairingGreedy.h"
#include "MModuleStripPairingChiSquare.h"
#include "MModuleEventFilter.h"
#include "MModuleEventSaver.h"
#include "MModuleResponseGenerator.h"

#include "MModuleTACcut.h"

#include "MModuleDiagnostics.h"
#include "MModuleDiagnosticsEnergyPerStrip.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


MAssembly::MAssembly()
{
  // standard constructor
    
  m_Interrupt = false;
  m_UseGui = true;
  
  g_Verbosity = c_Error;
  
  m_Supervisor = MSupervisor::GetSupervisor();
  
  // Fixed seed to reproduce DEE results
  gRandom->SetSeed(20170912);
  
  MString Cfg = "~/.nuclearizer.cfg";
  MFile::ExpandFileName(Cfg);
  m_Supervisor->SetConfigurationFileName(Cfg);
  
  m_Supervisor->UseMultiThreading(true);
  
  m_Supervisor->AddAvailableModule(new MModuleLoaderSimulationsBalloon());
  m_Supervisor->AddAvailableModule(new MModuleLoaderSimulationsSMEX());
  m_Supervisor->AddAvailableModule(new MModuleLoaderMeasurementsROA());
  m_Supervisor->AddAvailableModule(new MModuleLoaderMeasurementsHDF());
  m_Supervisor->AddAvailableModule(new MModuleReceiverBalloon());
  m_Supervisor->AddAvailableModule(new MModuleLoaderMeasurementsBinary());
  
  m_Supervisor->AddAvailableModule(new MModuleEventFilter());
  m_Supervisor->AddAvailableModule(new MModuleEnergyCalibrationUniversal());

  m_Supervisor->AddAvailableModule(new MModuleStripPairingGreedy());
  m_Supervisor->AddAvailableModule(new MModuleStripPairingChiSquare());
  m_Supervisor->AddAvailableModule(new MModuleChargeSharingCorrection());
  m_Supervisor->AddAvailableModule(new MModuleDepthCalibration());
  m_Supervisor->AddAvailableModule(new MModuleDepthCalibrationB());
  m_Supervisor->AddAvailableModule(new MModuleDepthCalibration2024());

  m_Supervisor->AddAvailableModule(new MModuleCrosstalkCorrection());  
  
  m_Supervisor->AddAvailableModule(new MModuleEventSaver());
  m_Supervisor->AddAvailableModule(new MModuleTransmitterRealta());
  m_Supervisor->AddAvailableModule(new MModuleResponseGenerator());
  m_Supervisor->AddAvailableModule(new MModuleTACcut());

  m_Supervisor->AddAvailableModule(new MModuleDiagnostics());
  m_Supervisor->AddAvailableModule(new MModuleDiagnosticsEnergyPerStrip());

  m_Supervisor->Load();
  
  m_Supervisor->SetUIProgramName("Nuclearizer");
  m_Supervisor->SetUIPicturePath("$(NUCLEARIZER)/resource/icons/Nuclearizer.xpm");
  m_Supervisor->SetUISubTitle("The detector calibrator of the COmpton Spectrometer and Imager, COSI");
  m_Supervisor->SetUILeadAuthor("Andreas Zoglauer");
  m_Supervisor->SetUICoAuthors("Alan Chiu, Alex Lowell, Andreas Zoglauer,\nAres Hernandez, Carolyn Kierans, Clio Sleator,\nDaniel Perez-Becker, Eric Bellm, Jau-Shian Liang,\nMark Bandstra");
}



////////////////////////////////////////////////////////////////////////////////


MAssembly::~MAssembly()
{
  // standard destructor
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
  Usage<<"      -C --change-configuration <pattern>:"<<endl;
  Usage<<"             Replace any value in the configuration file (-C can be used multiple times)"<<endl;
  Usage<<"             E.g. to change the roa file, one would set pattern to:"<<endl;
  Usage<<"             -C ModuleOptions.XmlTagMeasurementLoaderROA.FileName=My.roa"<<endl;
  Usage<<"      -a --auto:"<<endl;
  Usage<<"             Automatically start analysis without GUI"<<endl;
  Usage<<"      -m --multithreading:"<<endl;
  Usage<<"             0: false (default), else: true"<<endl;
  Usage<<"      -g --geometry:"<<endl;
  Usage<<"             Use this geometry file"<<endl;
  Usage<<"      -t --test:"<<endl;
  Usage<<"             Perform a test run to see if nuclearizer can be started up correctly."<<endl;
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
        Option == "-g" || Option == "--geometry" ||
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
      m_Supervisor->UseMultiThreading((atoi(argv[++i]) != 0 ? true : false));
      cout<<"Command-line parser: Using multithreading: "<<(atoi(argv[i]) != 0 ? "yes" : "no")<<endl;
    } else if (Option == "--test" || Option == "-t") {
      // Parse later
    } else if (Option == "--auto" || Option == "-a") {
      // Parse later
    }
  }
  
  // Look if we need to change the configuration
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--change-configuration" || Option == "-C") {
      if (m_Supervisor->ChangeConfiguration(argv[++i]) == false) {
        cout<<"ERROR: Command-line parser: Unable to change this configuration value: "<<argv[i]<<endl;        
      } else {
        cout<<"Command-line parser: Changing this configuration value: "<<argv[i]<<endl;
      }
    }
  }  
  
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--geometry" || Option == "-g") {
      m_Supervisor->SetGeometryFileName(argv[++i]);
      cout<<"Command-line parser: Use geometry file "<<argv[i]<<endl;
    }
  }
  
  // Now parse all high level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--auto" || Option == "-a") {
      m_UseGui = false;
      gROOT->SetBatch(true);
      m_Supervisor->UseUI(false);
      m_Supervisor->Analyze();
      m_Supervisor->Exit();
      return false;
    } else if (Option == "--test" || Option == "-t") {
        m_UseGui = false;
        gROOT->SetBatch(true);
        m_Supervisor->UseUI(false);
        m_Supervisor->Analyze(true);
        m_Supervisor->Exit();
        return false;
      }
  }
  
  if (m_UseGui == true) {
    if (m_Supervisor->LaunchUI() == false) {
      return false; 
    }
  }
  
  return true;
}


// MAssembly: the end...
////////////////////////////////////////////////////////////////////////////////
