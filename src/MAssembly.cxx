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
using namespace std;

// ROOT libs:
#include "TROOT.h"
#include "TRandom.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MStreams.h"
#include "MString.h"
#include "MFile.h"

// Nuclearizer libs:
#include "MFretalonRegistry.h"
#include "MReadOutDataTAC.h"
#include "MReadOutDataEnergy.h"
#include "MModule.h"
#include "MModuleTransmitterRealta.h"
#include "MModuleLoaderSimulationsCosima.h"
#include "MModuleLoaderMeasurementsROA.h"
#include "MModuleLoaderMeasurementsHDF.h"
#include "MModuleLoaderMeasurementsFITS.h"
#include "MModuleLoaderMeasurementsL0.h"
#include "MModuleEnergyCalibration.h"
#include "MModuleDepthCalibration.h"
#include "MModuleStripPairingMultiRoundChiSquare.h"
#include "MModuleStripPairingChiSquare.h"
#include "MModuleEventFilter.h"
#include "MModuleEventSaver.h"
#include "MModuleSaverMeasurementsL0.h"
#include "MModuleSaverMeasurementsFITS.h"
#include "MModuleResponseGenerator.h"
#include "MModuleRevan.h"
#include "MModuleTACcut.h"
// #include "MModuleNearestNeighbor.h"
#include "MModuleDiagnostics.h"
#include "MModuleDiagnosticsEnergyPerStrip.h"
#include "MModuleDEESMEX.h"



////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MAssembly)
#endif


////////////////////////////////////////////////////////////////////////////////


MAssembly::MAssembly()
{
  // standard constructor

  m_UseGui = true;
  m_HasCommandLineError = false;

  g_Verbosity = c_Error;

  // Register new read-out data:
  MReadOutDataTAC TAC;
  MFretalonRegistry::Instance().Register(TAC);

  MReadOutDataEnergy Energy;
  MFretalonRegistry::Instance().Register(Energy);

  // Create the supervisor
  m_Supervisor = MSupervisor::GetSupervisor();

  // Fixed seed to reproduce DEE results
  gRandom->SetSeed(20170912);

  MString Cfg = "~/.nuclearizer.cfg";
  MFile::ExpandFileName(Cfg);
  m_Supervisor->SetConfigurationFileName(Cfg);

  m_Supervisor->UseMultiThreading(true);

  m_Supervisor->AddAvailableModule(new MModuleLoaderSimulationsCosima());
  m_Supervisor->AddAvailableModule(new MModuleLoaderMeasurementsROA());
  m_Supervisor->AddAvailableModule(new MModuleLoaderMeasurementsHDF());
  m_Supervisor->AddAvailableModule(new MModuleLoaderMeasurementsFITS());
  m_Supervisor->AddAvailableModule(new MModuleLoaderMeasurementsL0());

  m_Supervisor->AddAvailableModule(new MModuleDEESMEX());

  m_Supervisor->AddAvailableModule(new MModuleEventFilter());
  m_Supervisor->AddAvailableModule(new MModuleEnergyCalibration());

  m_Supervisor->AddAvailableModule(new MModuleStripPairingMultiRoundChiSquare());
  m_Supervisor->AddAvailableModule(new MModuleStripPairingChiSquare());
  m_Supervisor->AddAvailableModule(new MModuleDepthCalibration());

  m_Supervisor->AddAvailableModule(new MModuleEventSaver());
  m_Supervisor->AddAvailableModule(new MModuleSaverMeasurementsL0());
  m_Supervisor->AddAvailableModule(new MModuleSaverMeasurementsFITS());
  m_Supervisor->AddAvailableModule(new MModuleSaverMeasurementsFITS("XmlTagSaverMeasurementsFITSL1a", 0, "Save events to L1a FITS"));
  m_Supervisor->AddAvailableModule(new MModuleSaverMeasurementsFITS("XmlTagSaverMeasurementsFITSL1b", 1, "Save events to L1b FITS"));
  m_Supervisor->AddAvailableModule(new MModuleSaverMeasurementsFITS("XmlTagSaverMeasurementsFITSL2", 2, "Save events to L2 FITS"));
  m_Supervisor->AddAvailableModule(new MModuleTransmitterRealta());
  m_Supervisor->AddAvailableModule(new MModuleResponseGenerator());
  m_Supervisor->AddAvailableModule(new MModuleRevan());
  m_Supervisor->AddAvailableModule(new MModuleTACcut());
  // m_Supervisor->AddAvailableModule(new MModuleNearestNeighbor());

  m_Supervisor->AddAvailableModule(new MModuleDiagnostics());
  m_Supervisor->AddAvailableModule(new MModuleDiagnosticsEnergyPerStrip());

  m_Supervisor->Load();

  m_Supervisor->SetUIProgramName("Nuclearizer");
  m_Supervisor->SetUIPicturePath("$(NUCLEARIZER)/resource/icons/Nuclearizer.xpm");
  m_Supervisor->SetUISubTitle("The detector calibrator of the COmpton Spectrometer and Imager, COSI");
  m_Supervisor->SetUILeadAuthor("Andreas Zoglauer");
  m_Supervisor->SetUICoAuthors("Robin Anthony-Petersen, Mark Bandstra, Jackie Beechert, \nEric Bellm, Emily Broadbent, Alan Chiu, \nValentina Fioretti, Julian Gerber, Felix Hagemann, \nSophie Haight, Ares Hernandez, Carolyn Kierans, \nHadar Lazar, Jau-Shian Liang, Alex Lowell, \nParshad Patel, Daniel Perez-Becker, Sean Pike \nJarred Roberts, Nicole Rodriguez Cavero, \nField Rogers, Clio Sleator");
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
  Usage<<"      -c --configuration <filename>.cfg:"<<endl;
  Usage<<"             Use this file as configuration file."<<endl;
  Usage<<"             If no configuration file is given ~/.nuclearizer.cfg is used"<<endl;
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
  Usage<<"      -h --help -? ?:"<<endl;
  Usage<<"             Print these command line options"<<endl;
  Usage<<"             Help gets preference over all other options"<<endl;
  Usage<<endl;

  // Each call starts without an error, otherwise an error of an earlier call would stick
  m_HasCommandLineError = false;

  // Store some options temporarily:
  MString Option;

  // Check for help
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--help" || Option == "-h" || Option == "-?" || Option == "?") {
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  // First check if all options are ok:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // Single argument
    if (Option == "--configuration" || Option == "-c" ||
        Option == "--change-configuration" || Option == "-C" ||
        Option == "--geometry" || Option == "-g" ||
        Option == "--multithreading" || Option == "-m" ||
        Option == "--verbosity" || Option == "-v") {
      if (argc <= i + 1 || argv[i + 1][0] == '-') {
        cout<<"ERROR: Command-line parser: Option "<<Option<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        m_HasCommandLineError = true;
        return false;
      }
    }
    // No argument
    else if (Option == "--auto" || Option == "-a" ||
             Option == "--test" || Option == "-t") {
      // Nothing to check
    }
    // Anything else which looks like an option is unknown - the arguments of the
    // above options never start with a "-", thus they cannot end up here
    else if (Option.BeginsWith("-") == true) {
      cout<<"WARNING: Command-line parser: Unknown option: "<<Option<<endl;
    }
  }

  // Now parse all low level options
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "--configuration" || Option == "-c") {
      // If the configuration file cannot be read, we continue with an empty one
      if (m_Supervisor->Load(argv[++i]) == false) {
        cout<<"WARNING: Command-line parser: Unable to load configuration file "<<argv[i]<<endl;
      } else {
        cout<<"Command-line parser: Use configuration file "<<argv[i]<<endl;
      }
    } else if (Option == "--verbosity" || Option == "-v") {
      const MString Value = argv[++i];
      if (Value.Is<int>() == false) {
        cout<<"ERROR: Command-line parser: Option "<<Option<<" needs an integer argument, not \""<<Value<<"\"!"<<endl;
        cout<<Usage.str()<<endl;
        m_HasCommandLineError = true;
        return false;
      }
      int Verbosity = Value.ToInt();
      if (Verbosity < c_Quiet || Verbosity > c_Info) {
        cout<<"ERROR: Command-line parser: Option "<<Option<<" needs an argument between "<<c_Quiet<<" and "<<c_Info<<", not \""<<Value<<"\"!"<<endl;
        cout<<Usage.str()<<endl;
        m_HasCommandLineError = true;
        return false;
      }
      g_Verbosity = Verbosity;
      cout<<"Command-line parser: Verbosity "<<g_Verbosity<<endl;
    } else if (Option == "--multithreading" || Option == "-m") {
      const MString Value = argv[++i];
      if (Value.Is<int>() == false) {
        cout<<"ERROR: Command-line parser: Option "<<Option<<" needs an integer argument, not \""<<Value<<"\"!"<<endl;
        cout<<Usage.str()<<endl;
        m_HasCommandLineError = true;
        return false;
      }
      const bool UseMultiThreading = (Value.ToInt() != 0);
      m_Supervisor->UseMultiThreading(UseMultiThreading);
      cout<<"Command-line parser: Using multithreading: "<<(UseMultiThreading == true ? "yes" : "no")<<endl;
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
