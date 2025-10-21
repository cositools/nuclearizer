/* 
 * TrappingCorrectionAm241.cxx
 *
 *
 * Copyright (C) by Sean Pike.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Sean Pike.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */

// Standard
#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include <csignal>
#include <cstdlib>
#include <map>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdio>
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
#include <TF1.h>
#include <TF1Convolution.h>
#include <TGraph2D.h>
#include <TFitResultPtr.h>
#include <TFitResult.h>
#include <TStopwatch.h>
#include <TProfile.h>

// MEGAlib
#include "MGlobal.h"
#include "MFile.h"
#include "MReadOutElementDoubleStrip.h"
#include "MFileReadOuts.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MReadOutSequence.h"
#include "MSupervisor.h"
#include "MModuleLoaderMeasurementsHDF.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MModuleEventFilter.h"
#include "MModuleStripPairingGreedy.h"
#include "MModuleStripPairingChiSquare.h"
#include "MModuleTACcut.h"
#include "MAssembly.h"


double g_MinCTD = -400;
double g_MaxCTD = 400;
int g_MinCounts = 250;

int g_HVStrips = 64;
int g_LVStrips = 64;

double g_AmPhotopeak = 59.54;

////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class TrappingCorrectionAm241
{
public:
  //! Default constructor
  TrappingCorrectionAm241();
  //! Default destructor
  ~TrappingCorrectionAm241();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what ever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  //! Produce functions for fitting
  TF1* GenerateCTDFunction(double CTDFitMin, double CTDFitMax, double CTDGuess, double FlipSwitch);
  TF1* GeneratePhotopeakFunction();

  MStripHit* GetDominantStrip(vector<MStripHit*>& Strips, double& EnergyFraction);

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The input file name
  MString m_HVFileName;
  MString m_LVFileName;
  MString m_EcalFile;
  MString m_TACCalFile;
  MString m_TACCutFile;
  MString m_StripMapFile;
  //! output file names
  MString m_OutFile;
  //! option to do a pixel-by-pixel calibration (instead of detector-by-detector)
  bool m_PixelCorrect;
  bool m_GreedyPairing;
  bool m_ExcludeNN;
  bool m_ContinueHDF5;

  double m_MinEnergy;
  double m_MaxEnergy;

};

////////////////////////////////////////////////////////////////////////////////


//! Default constructor
TrappingCorrectionAm241::TrappingCorrectionAm241() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
TrappingCorrectionAm241::~TrappingCorrectionAm241()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool TrappingCorrectionAm241::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  This app reads HV-illuminated and LV-illuminated Am241 data and determines the amount of charge loss occurring across the detector. When comparing the two illuminations, the energy read by each side of the detector is shifted. This app measures and records this shift, as well as the CTD endpoints on the HV and LV faces of the detector."<<endl;
  Usage<<endl;
  Usage<<"  Usage: TrappingCorrectionAm241 <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         --HVfile:   HV illumination input file name (.hdf5 or .txt with list of hdf5s)"<<endl;
  Usage<<"         --LVfile:   LV illumination input file name (.hdf5 or .txt with list of hdf5s)"<<endl;
  Usage<<"         --emin:   minimum Event energy (default 40 keV)"<<endl;
  Usage<<"         --emax:   maximum Event energy (default 70 kev)"<<endl;
  Usage<<"         -e:   energy calibration file (.ecal)"<<endl;
  Usage<<"         --tcal:   TAC calibration file"<<endl;
  Usage<<"         --tcut:   TAC cut file"<<endl;
  Usage<<"         -p:   do pixel-by-pixel correction"<<endl;
  Usage<<"         -m:   strip map file name (.map)"<<endl;
  Usage<<"         -g:   greedy strip pairing (default is chi-square)"<<endl;
  Usage<<"         -n:   exclude nearest neighbors"<<endl;
  Usage<<"         -o:   outfile (default YYYYMMDDHHMMSS)"<<endl;
  Usage<<"         --nocontinue:  Suppress continuous HDF5 loading"<<endl;
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

  m_PixelCorrect = false;
  m_GreedyPairing = false;
  m_ContinueHDF5 = true;
  m_MinEnergy = 40;
  m_MaxEnergy = 70;
  
  time_t rawtime;
  tm* timeinfo;
  time(&rawtime);
  timeinfo = gmtime(&rawtime);

  char buffer [80];
  strftime(buffer,80,"%Y%m%d%H%M%S",timeinfo);

  m_OutFile = MString(buffer);

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if ((Option == "--HVfile") || (Option == "--LVfile") || (Option == "-o") || (Option == "--emin") || (Option == "--emax") || (Option == "--tcal") || (Option == "--tcut") || (Option == "-m")) {
      if (!((argc > i+1) && (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 

    // Then fulfill the options:
    if (Option == "--HVfile") {
      m_HVFileName = argv[++i];
      cout<<"Accepting file name: "<<m_HVFileName<<endl;
    } else if (Option == "--LVfile") {
      m_LVFileName = argv[++i];
      cout<<"Accepting file name: "<<m_LVFileName<<endl;
    } else if (Option == "-e") {
      m_EcalFile = argv[++i];
      cout<<"Accepting file name: "<<m_EcalFile<<endl;
    } else if (Option == "--emin") {
      m_MinEnergy = stod(argv[++i]);
    } else if (Option == "--emax") {
      m_MaxEnergy = stod(argv[++i]);
    } else if (Option == "--tcal") {
      m_TACCalFile = argv[++i];
      cout<<"Accepting file name: "<<m_TACCalFile<<endl;
    } else if (Option == "--tcut") {
      m_TACCutFile = argv[++i];
      cout<<"Accepting file name: "<<m_TACCutFile<<endl;
    } else if (Option == "-m") {
      m_StripMapFile = argv[++i];
      cout<<"Accepting file name: "<<m_StripMapFile<<endl;
    } else if (Option == "-o") {
      m_OutFile = argv[++i];
      cout<<"Accepting file name: "<<m_OutFile<<endl;
    } else if (Option == "-p") {
      m_PixelCorrect = true;
    } else if (Option == "-g") {
      m_GreedyPairing = true;
    } else if (Option == "-n") {
      m_ExcludeNN = true;
    } else if (Option == "--nocontinue") {
      m_ContinueHDF5 = false;
    } else {
      cout<<"Argument not recognized:"<<Option<<endl;
      cout<<Usage.str()<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool TrappingCorrectionAm241::Analyze()
{
  //time code just to see
  TStopwatch watch;
  watch.Start();

  if (m_Interrupt == true) return false;

  // [CTD, HV Energy, LV Energy] for HV illumination and LV illumination
  map<unsigned int, vector<vector<double>>> Endpoints;
  map<unsigned int, vector<vector<double>>> FullDetEndpoints;

  // Store the HV and LV input files
  vector<MString> FileNames;
  FileNames.push_back(m_HVFileName);
  FileNames.push_back(m_LVFileName);

  // Map the side integer to HV and LV labels
  // i.e. 0=HV, 1=LV
  vector<MString> IllumSide;
  IllumSide.push_back(MString("HV"));
  IllumSide.push_back(MString("LV"));

  // Make a directory in which to store the pixel-level data 
  MString PixelDir = m_OutFile + MString("_pixeldata");
  if (m_PixelCorrect==true) {
    if (std::filesystem::create_directories(PixelDir.Data())==false) {
      cout<<"Directory '"<<PixelDir<<"' already exists or could not be created."<<endl;
    }
  }

  // For each side (HV and LV) calibrate the input data, construct histograms, and fit models
  for (unsigned int s=0; s<2; ++s) {

    // Prep CTD model parameters. Mirror the model about the x-axis depending which side is being analyzed. 
    double CTDFitMin, CTDFitMax, FlipSwitch;
    if (IllumSide[s]=="HV") {
      FlipSwitch = 1;
      CTDFitMin = g_MinCTD;
      CTDFitMax = 0;
    } else {
      FlipSwitch = -1;
      CTDFitMin = 0;
      CTDFitMax = g_MaxCTD;
    }

    MString InputFile = FileNames[s];
    vector<MString> HDFNames;

    TH1::SetDefaultSumw2();

    map<int, TH1D*> CTDHistograms;
    map<int, TH1D*> HVEnergyHistograms;
    map<int, TH1D*> LVEnergyHistograms;

    // Read in the input files and make a list of hdf5 files to calibrate
    if ((InputFile.GetSubString(InputFile.Length() - 4)) == "hdf5") {
      HDFNames.push_back(InputFile);
    } else if ((InputFile.GetSubString(InputFile.Length() - 3)) == "txt") {
      cout<<"Reading input file "<<InputFile<<endl;
      cout<<"WARNING: When passing a list of files, ensure that you have chosen the correct HDF5 continuous reading mode. Use the --nocontinue option to suppress continuous file reading."<<endl;
      MFile F;
      if (F.Open(InputFile)==false) {
        cout<<"Error: Failed to open input file."<<endl;
      } else {
        MString Line;
        while (F.ReadLine(Line)) {
          MString Trimmed = Line.Trim();
          if ((Trimmed != "")) {
            if (F.Exists(Trimmed)==true) {
              HDFNames.push_back(Trimmed);
            } else {
              cout<<"Error: Could not find file "<<Trimmed<<endl;
            }
          }
        }
      }
    } else {
      cout<<"Error: Unrecognized file format: "<<InputFile<<endl;
    }

    // Analyze all the data and fill in the histograms
    for (unsigned int f = 0; f<HDFNames.size(); ++f) {

      MString File = HDFNames[f];
      cout<<"Beginning analysis of file "<<File<<endl;

      // Create and initialize nuclearizer modules
      MSupervisor* S = MSupervisor::GetSupervisor();
      
    	MModuleLoaderMeasurementsHDF* Loader;
    	MModuleTACcut* TACCalibrator;
    	MModuleEnergyCalibrationUniversal* EnergyCalibrator;
    	MModuleEventFilter* EventFilter;

      unsigned int MNumber = 0;
      cout<<"Creating HDF5 loader"<<endl;
      Loader = new MModuleLoaderMeasurementsHDF();
      Loader->SetFileNameStripMap(m_StripMapFile);
      Loader->SetFileName(File);
      Loader->SetLoadContinuationFiles(m_ContinueHDF5);
      S->SetModule(Loader, MNumber);
      ++MNumber;

      cout<<"Creating TAC calibrator"<<endl;
      TACCalibrator = new MModuleTACcut();
      TACCalibrator->SetTACCalFileName(m_TACCalFile);
      TACCalibrator->SetTACCutFileName(m_TACCutFile);
      S->SetModule(TACCalibrator, MNumber);
      ++MNumber;
     
      cout<<"Creating energy calibrator"<<endl;
      EnergyCalibrator = new MModuleEnergyCalibrationUniversal();
      EnergyCalibrator->SetFileName(m_EcalFile);
      EnergyCalibrator->EnablePreampTempCorrection(false);
      S->SetModule(EnergyCalibrator, MNumber);
      ++MNumber;

      cout<<"Creating Event filter"<<endl;
      // Only use events with 1 to 3 Strip Hits on each side
      // After strip pairing, we'll also filter to make sure we're only looking at single Hits
      EventFilter = new MModuleEventFilter();
      EventFilter->SetMinimumLVStrips(1);
      EventFilter->SetMaximumLVStrips(3);
      EventFilter->SetMinimumHVStrips(1);
      EventFilter->SetMaximumHVStrips(3);
      EventFilter->SetMinimumHits(0);
      EventFilter->SetMaximumHits(100);
      EventFilter->SetMinimumTotalEnergy(m_MinEnergy);
      EventFilter->SetMaximumTotalEnergy(m_MaxEnergy*2); // Multiply by 2 because this is the event-level energy, i.e. sum over both sides
      S->SetModule(EventFilter, MNumber);
      ++MNumber;
      
      cout<<"Creating strip pairing"<<endl;
      MModule* Pairing;
      if (m_GreedyPairing == true) {
        Pairing = new MModuleStripPairingGreedy();
      } else {
        Pairing = new MModuleStripPairingChiSquare();
      }
      S->SetModule(Pairing, MNumber);

      cout<<"Initializing Loader"<<endl;
      if (Loader->Initialize() == false) return false;
      cout<<"Initializing TAC calibrator"<<endl;
      if (TACCalibrator->Initialize() == false) return false;
      cout<<"Initializing Energy calibrator"<<endl;
      if (EnergyCalibrator->Initialize() == false) return false;
      cout<<"Initializing Event filter"<<endl;
      if (EventFilter->Initialize() == false) return false;
      cout<<"Initializing Pairing"<<endl;
      if (Pairing->Initialize() == false) return false;

      bool IsFinished = false;
      MReadOutAssembly* Event = new MReadOutAssembly();

      // Pass Events through each module. Once calibrated, add the Event to the histograms
      cout<<"Analyzing..."<<endl;
      while ((IsFinished == false) && (m_Interrupt == false)) {
        Event->Clear();

        if (Loader->IsReady()) {

          // Load Event from HDF5 file, then do TAC cut/calibration, energy calibration, and filtering
          Loader->AnalyzeEvent(Event);
          TACCalibrator->AnalyzeEvent(Event);
          EnergyCalibrator->AnalyzeEvent(Event);
          bool Unfiltered = EventFilter->AnalyzeEvent(Event);

          if (Unfiltered == true) {
            
            // Pair strips
            Pairing->AnalyzeEvent(Event);

            // Only look at events with 1 Hit since we're interested in the 60keV photopeak.
            if ((Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true) && (Unfiltered==true) && (Event->GetNHits()==1)) {
              
              // for each Hit
              for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
                double HVEnergy = 0.0;
                double LVEnergy = 0.0;
                double HVEnergyResolution = 0.0;
                double LVEnergyResolution = 0.0;
                vector<MStripHit*> HVStrips;
                vector<MStripHit*> LVStrips;

                MHit* H = Event->GetHit(h);
                
                int DetID = H->GetStripHit(0)->GetDetectorID();

                // for each Strip Hit in the Hit
                for (unsigned int sh = 0; sh < H->GetNStripHits(); ++sh) {
                  MStripHit* SH = H->GetStripHit(sh);

                  // Sum up the HV and LV energies, excluding nearest neighbors if m_ExcludeNN==true
                  if ((m_ExcludeNN==false) || ((m_ExcludeNN==true) && (SH->IsNearestNeighbor()==false))) {
                    if (SH->IsLowVoltageStrip()==true) {
                      LVEnergy += SH->GetEnergy();
                      LVEnergyResolution += (SH->GetEnergyResolution())*(SH->GetEnergyResolution());
                      LVStrips.push_back(SH);
                    } else {
                      HVEnergy += SH->GetEnergy();
                      HVEnergyResolution += (SH->GetEnergyResolution())*(SH->GetEnergyResolution());
                      HVStrips.push_back(SH);
                    }
                  }
                }

                // As long as there are Strip Hits remaining on each side, add the energies and CTDs to histograms
                if ((HVStrips.size()>0) && (LVStrips.size()>0)) {
                  
                  double HVEnergyFraction = 0;
                  double LVEnergyFraction = 0;
                  MStripHit* HVSH = GetDominantStrip(HVStrips, HVEnergyFraction); 
                  MStripHit* LVSH = GetDominantStrip(LVStrips, LVEnergyFraction);
                  
                  if ((LVSH->HasCalibratedTiming()==true) && (HVSH->HasCalibratedTiming()==true)) {
                    
                    double CTD = LVSH->GetTiming() - HVSH->GetTiming();
                    
                    int PixelID = (10000*DetID) + (100*LVSH->GetStripID()) + (HVSH->GetStripID());

                    // If this PixelID hasn't been encountered yet, make an entry for it in Endpoints 
                    if (Endpoints.find(PixelID)==Endpoints.end()) {
                      vector<double> tempHVvec;
                      vector<double> tempLVvec;
                      vector<vector<double>> tempvec;
                      Endpoints[PixelID] = tempvec;
                      Endpoints[PixelID].push_back(tempHVvec);
                      Endpoints[PixelID].push_back(tempLVvec);
                    }

                    TH1D* CTDHist = CTDHistograms[PixelID];
                    TH1D* HVHist = HVEnergyHistograms[PixelID];
                    TH1D* LVHist = LVEnergyHistograms[PixelID];
                    
                    // If we didnt find an entry for the CTD and energy histograms, make new ones
                    if (CTDHist == nullptr) {
                      char name[64]; sprintf(name,"CTD: PixelID %d %s Illumination",PixelID,IllumSide[s].Data());
                      CTDHist = new TH1D(name, name, (g_MaxCTD - g_MinCTD)/2, g_MinCTD, g_MaxCTD);
                      CTDHist->SetXTitle("CTD (ns)");
                      CTDHist->SetYTitle("Hits");
                      CTDHistograms[PixelID] = CTDHist;
                    }
                    if (HVHist == nullptr) {
                      char name[64]; sprintf(name,"HV energy: PixelID %d %s Illumination",PixelID,IllumSide[s].Data());
                      HVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy)*2, m_MinEnergy, m_MaxEnergy);
                      HVHist->SetXTitle("HV Energy (keV)");
                      HVHist->SetYTitle("Hits");
                      HVEnergyHistograms[PixelID] = HVHist;
                    }
                    if (LVHist == nullptr) {
                      char name[64]; sprintf(name,"LV energy: PixelID %d %s Illumination",PixelID,IllumSide[s].Data());
                      LVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy)*2, m_MinEnergy, m_MaxEnergy);
                      LVHist->SetXTitle("LV Energy (keV)");
                      LVHist->SetYTitle("Hits");
                      LVEnergyHistograms[PixelID] = LVHist;
                    }

                    // Add the data to the histograms
                    CTDHist->Fill(CTD);
                    HVHist->Fill(HVEnergy);
                    LVHist->Fill(LVEnergy);
                  }
                }
              }
            }
          }
        }
        IsFinished = Loader->IsFinished();
      }
    }
    
    map<int, TH1D*> FullDetCTDHistograms;
    map<int, TH1D*> FullDetHVEnergyHistograms;
    map<int, TH1D*> FullDetLVEnergyHistograms;
    
    // Loop over all pixels for which there are pixel-level histograms and sum to get full detector histograms
    // Fit to the pixel-level histograms and record the results
    for (auto H: CTDHistograms) {
      
      int PixelID = H.first;
      int DetID = (PixelID-(PixelID%10000))/10000;

      // If this DetID hasn't been encountered yet, make an entry for it in Endpoints
      if (FullDetEndpoints.find(DetID)==FullDetEndpoints.end()) {
        vector<double> tempHVvec;
        vector<double> tempLVvec;
        vector<vector<double>> tempvec;
        FullDetEndpoints[DetID] = tempvec;
        FullDetEndpoints[DetID].push_back(tempHVvec);
        FullDetEndpoints[DetID].push_back(tempLVvec);
      }

      TH1D* CTDHist = FullDetCTDHistograms[DetID];
      TH1D* HVHist = FullDetHVEnergyHistograms[DetID];
      TH1D* LVHist = FullDetLVEnergyHistograms[DetID];
      
      // If we didnt find an entry for the CTD and energy histograms, make new ones
      if (CTDHist == nullptr) {
        char name[64]; sprintf(name,"CTD: DetID %d %s Illumination",DetID,IllumSide[s].Data());
        CTDHist = new TH1D(name, name, (g_MaxCTD - g_MinCTD)/2, g_MinCTD, g_MaxCTD);
        CTDHist->SetXTitle("CTD (ns)");
        CTDHist->SetYTitle("Hits");
        FullDetCTDHistograms[DetID] = CTDHist;
      }
      if (HVHist == nullptr) {
        char name[64]; sprintf(name,"HV energy: DetID %d %s Illumination",DetID,IllumSide[s].Data());
        HVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy)*2, m_MinEnergy, m_MaxEnergy);
        HVHist->SetXTitle("HV Energy (keV)");
        HVHist->SetYTitle("Hits");
        FullDetHVEnergyHistograms[DetID] = HVHist;
      }
      if (LVHist == nullptr) {
        char name[64]; sprintf(name,"LV energy: DetID %d %s Illumination",DetID,IllumSide[s].Data());
        LVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy)*2, m_MinEnergy, m_MaxEnergy);
        LVHist->SetXTitle("LV Energy (keV)");
        LVHist->SetYTitle("Hits");
        FullDetLVEnergyHistograms[DetID] = LVHist;
      }

      CTDHist->Add(CTDHist, H.second);
      HVHist->Add(HVHist, HVEnergyHistograms[PixelID]);
      LVHist->Add(LVHist, LVEnergyHistograms[PixelID]);

      if (m_PixelCorrect==true) {

        // Fit histograms and save the results
        // Only perform fits if the total counts in each is above the threshold given by g_MinCounts
        if ((H.second->Integral() > g_MinCounts) && (HVEnergyHistograms[PixelID]->Integral() > g_MinCounts) && (LVEnergyHistograms[PixelID]->Integral() > g_MinCounts)) {
          
          // Initialize and run the CTD fit
          double CTDGuess = H.second->GetBinCenter(H.second->GetMaximumBin());
          TF1* CTDFunction = GenerateCTDFunction(CTDFitMin, CTDFitMax, CTDGuess, FlipSwitch);
          TFitResultPtr CTDFit = H.second->Fit(CTDFunction, "SQ", "", CTDFitMin, CTDFitMax);

          // Save the results of the CTD fit
          ofstream CTDFitFile(PixelDir +MString("/") + PixelID+MString("_CTDFitResult_")+IllumSide[s]+ MString("Illum.txt"));
          streambuf* coutbuf = cout.rdbuf();
          cout.rdbuf(CTDFitFile.rdbuf());
          if (CTDFit.Get()) {
              CTDFit->Print();
          }
          cout.rdbuf(coutbuf);
          CTDFitFile.close();
          
          // Initialize and run the HV photopeak fit
          TF1* PhotopeakFunctionHV = GeneratePhotopeakFunction();
          TFitResultPtr HVFit = HVEnergyHistograms[PixelID]->Fit(PhotopeakFunctionHV, "SQ", "", 55, 70);

          // Save the results of the HV photopeak fit
          ofstream HVFitFile(PixelDir +MString("/") + PixelID+MString("_HVEnergyFitResult_")+IllumSide[s]+ MString("Illum.txt"));
          coutbuf = cout.rdbuf();
          cout.rdbuf(HVFitFile.rdbuf());
          if (HVFit.Get()) {
              HVFit->Print();
          }
          cout.rdbuf(coutbuf);
          HVFitFile.close();

          // Initialize and run the LV photopeak fit
          TF1* PhotopeakFunctionLV = GeneratePhotopeakFunction();
          TFitResultPtr LVFit = LVEnergyHistograms[PixelID]->Fit(PhotopeakFunctionLV, "SQ", "", 55, 70);
          
          // Save the results of the LV photopeak fit
          ofstream LVFitFile(PixelDir +MString("/") + PixelID+MString("_LVEnergyFitResult_")+IllumSide[s]+ MString("Illum.txt"));
          coutbuf = cout.rdbuf();
          cout.rdbuf(LVFitFile.rdbuf());
          if (LVFit.Get()) {
              LVFit->Print();
          }
          cout.rdbuf(coutbuf);
          LVFitFile.close();

          // Save the resulting HV and LV photopeak centroids and CTD centroids to the Endpoints map.
          // Only save these values if the fits ran successfully and if their reduced chi-square is less than 5
          if ((!(CTDFit->IsEmpty())) && (!(HVFit->IsEmpty())) && (!(LVFit->IsEmpty())) && ((CTDFit->Chi2()/CTDFit->Ndf()) < 5) && ((HVFit->Chi2()/HVFit->Ndf()) < 5) && ((LVFit->Chi2()/LVFit->Ndf()) < 5)) {
            Endpoints[PixelID][s].push_back(CTDFit->Parameter(2));
            Endpoints[PixelID][s].push_back(HVFit->Parameter(1));
            Endpoints[PixelID][s].push_back(LVFit->Parameter(1));
          } else {
            cout<<"Fits failed for Pixel "<<PixelID<<endl;
          }
        } else {
          cout<<"Fewer than "<<g_MinCounts<<" counts in Pixel ID "<<PixelID<<endl;
        }

        TFile CTDHistFile(PixelDir+MString("/")+PixelID+MString("_CTDHist_") + IllumSide[s]+ MString("Illum.root"),"recreate");
        H.second->Write();
        CTDHistFile.Close();

        TFile HVHistFile(PixelDir+MString("/")+PixelID+MString("_HVEnergyHist_") + IllumSide[s]+ MString("Illum.root"),"recreate");
        HVEnergyHistograms[PixelID]->Write();
        HVHistFile.Close();

        TFile LVHistFile(PixelDir+MString("/")+PixelID+MString("_LVEnergyHist_") + IllumSide[s]+ MString("Illum.root"),"recreate");
        LVEnergyHistograms[PixelID]->Write();
        LVHistFile.Close();
      }
    }

    // Do the same function fitting and recording as above, but for the full detectors rather than pixel-by-pixel
    for (auto H: FullDetCTDHistograms) {

      int DetID = H.first;

      if ((H.second->Integral() > g_MinCounts) && (FullDetHVEnergyHistograms[DetID]->Integral() > g_MinCounts) && (FullDetLVEnergyHistograms[DetID]->Integral() > g_MinCounts)) {

        double CTDGuess = H.second->GetBinCenter(H.second->GetMaximumBin());
        TF1* CTDFunction = GenerateCTDFunction(CTDFitMin, CTDFitMax, CTDGuess, FlipSwitch);
        TFitResultPtr CTDFit = H.second->Fit(CTDFunction, "SQ", "", CTDFitMin, CTDFitMax);

        TF1* PhotopeakFunctionHV = GeneratePhotopeakFunction();
        TFitResultPtr HVFit = FullDetHVEnergyHistograms[DetID]->Fit(PhotopeakFunctionHV, "SQ", "", 55, 70);

        TF1* PhotopeakFunctionLV = GeneratePhotopeakFunction();
        TFitResultPtr LVFit = FullDetLVEnergyHistograms[DetID]->Fit(PhotopeakFunctionLV, "SQ", "", 55, 70);

        if ((!(CTDFit->IsEmpty())) && (!(HVFit->IsEmpty())) && (!(LVFit->IsEmpty()))) {
          FullDetEndpoints[DetID][s].push_back(CTDFit->Parameter(2));
          FullDetEndpoints[DetID][s].push_back(HVFit->Parameter(1));
          FullDetEndpoints[DetID][s].push_back(LVFit->Parameter(1));

          ofstream CTDFitFile(DetID+MString("_CTDFitResult_")+IllumSide[s]+ MString("Illum.txt"));
          streambuf* coutbuf = cout.rdbuf();
          cout.rdbuf(CTDFitFile.rdbuf());
          if (CTDFit.Get()) {
              CTDFit->Print();
          }
          cout.rdbuf(coutbuf);
          CTDFitFile.close();

          ofstream HVFitFile(DetID+MString("_HVEnergyFitResult_")+IllumSide[s]+ MString("Illum.txt"));
          coutbuf = cout.rdbuf();
          cout.rdbuf(HVFitFile.rdbuf());
          if (HVFit.Get()) {
              HVFit->Print();
          }
          cout.rdbuf(coutbuf);
          HVFitFile.close();
          
          ofstream LVFitFile(DetID+MString("_LVEnergyFitResult_")+IllumSide[s]+ MString("Illum.txt"));
          coutbuf = cout.rdbuf();
          cout.rdbuf(LVFitFile.rdbuf());
          if (LVFit.Get()) {
              LVFit->Print();
          }
          cout.rdbuf(coutbuf);
          LVFitFile.close();

          TFile CTDFile(m_OutFile+MString("_Det")+DetID+MString("_CTDHist_") + IllumSide[s]+ MString("Illum.root"),"recreate");

          TCanvas* CTDCanvas = new TCanvas();
          CTDCanvas->cd();
          H.second->Draw("hist");
          CTDFunction->Draw("same");

          H.second->Write();
          CTDFile.Close();

          TFile HVHistFile(m_OutFile+MString("_Det")+DetID+MString("_HVEnergyHist_") + IllumSide[s]+ MString("Illum.root"),"recreate");

          TCanvas* HVHistCanvas = new TCanvas();
          HVHistCanvas->cd();
          FullDetHVEnergyHistograms[DetID]->Draw("hist");
          PhotopeakFunctionHV->Draw("same");

          FullDetHVEnergyHistograms[DetID]->Write();
          HVHistFile.Close();

          TFile LVHistFile(m_OutFile+MString("_Det")+DetID+MString("_LVEnergyHist_") + IllumSide[s]+ MString("Illum.root"),"recreate");

          TCanvas* LVHistCanvas = new TCanvas();
          LVHistCanvas->cd();
          FullDetLVEnergyHistograms[DetID]->Draw("hist");
          PhotopeakFunctionLV->Draw("same");

          FullDetLVEnergyHistograms[DetID]->Write();
          LVHistFile.Close();

        } else {
          cout<<"Fits failed for Det "<<DetID<<endl;
        }
      } else {
        cout<<"Fewer than "<<g_MinCounts<<" counts in Det "<<DetID<<endl;
      }
    }
  }

  //setup parameter file
  ofstream OutputCalFile;
  OutputCalFile.open(m_OutFile+MString("_parameters.txt"));
  OutputCalFile<<"# Det ID"<<'\t'<<"HV Strip ID"<<'\t'<<"LV Strip ID"<<'\t'<<"HV Illum CTD"<<'\t'<<"LV Illum CTD"<<'\t'<<"HV Illum. HV Centroid"<<'\t'<<"LV Illum. HV Centroid"<<'\t'<<"HV Illum. LV Centroid"<<'\t'<<"LV Illum. LV Centroid"<<endl<<endl;

  map<int, TH2D*> DeltaHVMap;
  map<int, TH2D*> DeltaLVMap;

  map<int, TH1D*> DeltaHVHist;
  map<int, TH1D*> DeltaLVHist;

  // Write the results of good fits to the parameter file and produce summary plots.
  // If an individual pixel did not produce a good fit, default to the detector-level parameters.
  // Loop over each detector
  for (auto E: FullDetEndpoints) {
    
    int DetID = E.first;

    double HVIllumCTD = 0;
    double LVIllumCTD = 0;
    double HVIllumHVCentroid = 0;
    double LVIllumHVCentroid = 0;
    double HVIllumLVCentroid = 0;
    double LVIllumLVCentroid = 0;

    if ((E.second[0].size() > 0) && (E.second[1].size() > 0)) {
      HVIllumCTD = E.second[0][0];
      LVIllumCTD = E.second[1][0];
      HVIllumHVCentroid = E.second[0][1];
      LVIllumHVCentroid = E.second[1][1];
      HVIllumLVCentroid = E.second[0][2];
      LVIllumLVCentroid = E.second[1][2];
    }

    // Loop over each pixel
    for (int hv=0; hv<g_HVStrips; ++hv) {
      for (int lv=0; lv<g_LVStrips; ++lv) {
        if (m_PixelCorrect==true) {

          int PixelID = (10000*DetID) + (100*lv) + hv;

          TH2D* TempHVMap = DeltaHVMap[DetID];
          TH2D* TempLVMap = DeltaLVMap[DetID];

          TH1D* TempHVHist = DeltaHVHist[DetID];
          TH1D* TempLVHist = DeltaLVHist[DetID];
          
          // Create the histograms and pixel maps if they don't exist yet
          if (TempHVMap == nullptr) {

            char HVname[64]; sprintf(HVname,"Delta HV Map: Det %d",DetID);
            TempHVMap = new TH2D(HVname, HVname, g_HVStrips, -0.5, g_HVStrips-0.5, g_LVStrips, -0.5, g_LVStrips-0.5);
            TempHVMap->SetXTitle("HV Strip");
            TempHVMap->SetYTitle("LV Strip");
            TempHVMap->SetZTitle("Delta HV Energy");
            
            char LVname[64]; sprintf(LVname,"Delta LV Map: Det %d",DetID);
            TempLVMap = new TH2D(LVname, LVname, g_HVStrips, -0.5, g_HVStrips-0.5, g_LVStrips, -0.5, g_LVStrips-0.5);
            TempLVMap->SetXTitle("HV Strip");
            TempLVMap->SetYTitle("LV Strip");
            TempLVMap->SetZTitle("Delta LV Energy");

            DeltaHVMap[DetID] = TempHVMap;
            DeltaLVMap[DetID] = TempLVMap;
          }

          if (TempHVHist == nullptr) {

            char HVname[64]; sprintf(HVname,"Delta HV Hist: Det %d",DetID);
            TempHVHist = new TH1D(HVname, HVname, 50, -3.0, 3.0);
            TempHVHist->SetXTitle("Delta HV Energy (keV)");
            TempHVHist->SetYTitle("Number of pixels");
            
            char LVname[64]; sprintf(LVname,"Delta LV Hist: Det %d",DetID);
            TempLVHist = new TH1D(LVname, LVname, 50, -3.0, 3.0);
            TempLVHist->SetXTitle("Delta LV Energy (keV)");
            TempLVHist->SetYTitle("Number of pixels");

            DeltaHVHist[DetID] = TempHVHist;
            DeltaLVHist[DetID] = TempLVHist;
          }
          
          DeltaHVMap[DetID]->SetBinContent(hv+1, lv+1, -100);
          DeltaLVMap[DetID]->SetBinContent(hv+1, lv+1, -100);

          // Retrieve the stored HV and LV energies and CTDs
          // Plot the energy shifts in the HV and LV Maps, and fill in the histograms
          if (Endpoints.find(PixelID)!=Endpoints.end()) {
            if ((Endpoints[PixelID][0].size() > 0) && (Endpoints[PixelID][1].size() > 0)) {
              
              HVIllumCTD = Endpoints[PixelID][0][0];
              LVIllumCTD = Endpoints[PixelID][1][0];
              HVIllumHVCentroid = Endpoints[PixelID][0][1];
              LVIllumHVCentroid = Endpoints[PixelID][1][1];
              HVIllumLVCentroid = Endpoints[PixelID][0][2];
              LVIllumLVCentroid = Endpoints[PixelID][1][2];

              // Normalize the differences in Energy to account for calibration differences
              double HVDiff = (HVIllumHVCentroid - LVIllumHVCentroid)*(g_AmPhotopeak/HVIllumHVCentroid);
              double LVDiff = (HVIllumLVCentroid - LVIllumLVCentroid)*(g_AmPhotopeak/HVIllumLVCentroid);

              DeltaHVMap[DetID]->SetBinContent(hv+1, lv+1, HVDiff);
              DeltaLVMap[DetID]->SetBinContent(hv+1, lv+1, LVDiff);

              DeltaHVHist[DetID]->Fill(HVDiff);
              DeltaLVHist[DetID]->Fill(LVDiff);
            }
          }
        }

        // Record the Energies and CTDs in the parameter file
        // Note that we default to the detector value if the pixel value isn't available and to 0 if the detector value isn't available.
        OutputCalFile<<to_string(DetID)<<'\t'<<to_string(hv)<<'\t'<<to_string(lv)<<'\t'<<to_string(HVIllumCTD)<<'\t'<<to_string(LVIllumCTD)<<'\t'<<to_string(HVIllumHVCentroid)<<'\t'<<to_string(LVIllumHVCentroid)<<'\t'<<to_string(HVIllumLVCentroid)<<'\t'<<to_string(LVIllumLVCentroid)<<endl<<endl;
      }
    }
  }

  OutputCalFile.close();

  // Save and plot the summary figures.
  if (m_PixelCorrect==true) {
    for (auto H: DeltaHVMap) {
      
      int DetID = H.first;

      TFile HVFile(m_OutFile+MString("_Det")+DetID+MString("_DeltaHVMap.root"),"recreate");
      TCanvas* HVCanvas = new TCanvas();
      HVCanvas->cd();
      H.second->SetMinimum(-5.);
      H.second->Draw("colz");
      H.second->Write();
      HVFile.Close();

      TFile LVFile(m_OutFile+MString("_Det")+DetID+MString("_DeltaLVMap.root"),"recreate");
      TCanvas* LVCanvas = new TCanvas();
      LVCanvas->cd();
      DeltaLVMap[DetID]->SetMinimum(-5.);
      DeltaLVMap[DetID]->Draw("colz");
      DeltaLVMap[DetID]->Write();
      LVFile.Close();

      TFile HVHistFile(m_OutFile+MString("_Det")+DetID+MString("_DeltaHVHist.root"),"recreate");
      TCanvas* HVHistCanvas = new TCanvas();
      HVHistCanvas->cd();
      DeltaHVHist[DetID]->Draw("hist");
      DeltaHVHist[DetID]->Write();
      HVHistFile.Close();

      TFile LVHistFile(m_OutFile+MString("_Det")+DetID+MString("_DeltaLVHist.root"),"recreate");
      TCanvas* LVHistCanvas = new TCanvas();
      LVHistCanvas->cd();
      DeltaLVHist[DetID]->Draw("hist");
      DeltaLVHist[DetID]->Write();
      LVHistFile.Close();
    }
  }

  watch.Stop();
  cout<<"total time (s): "<<watch.CpuTime()<<endl;
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


TF1* TrappingCorrectionAm241::GeneratePhotopeakFunction()
{
  // Gaussian with a low-E shelf
  TF1* PhotopeakFunction = new TF1("PhotopeakFunction", "gaus(0) + [0]*[3]*(1 - erf((x-[1])/(sqrt(2)*[2])))", 55, 70);

  PhotopeakFunction->SetParName(0, "Gauss norm");
  PhotopeakFunction->SetParName(1, "Mu");
  PhotopeakFunction->SetParName(2, "Sigma");
  PhotopeakFunction->SetParName(3, "Shelf norm");

  PhotopeakFunction->SetParameter("Gauss norm", 1000);
  PhotopeakFunction->SetParameter("Mu", 60);
  PhotopeakFunction->SetParameter("Sigma", 2);
  PhotopeakFunction->SetParameter("Shelf norm", 0.05);

  PhotopeakFunction->SetParLimits(0, 10, 1e8);
  PhotopeakFunction->SetParLimits(1, 55, 65);
  PhotopeakFunction->SetParLimits(2, 1.0, 10);
  PhotopeakFunction->SetParLimits(3, 0, 0.1);

  return PhotopeakFunction;
}


////////////////////////////////////////////////////////////////////////////////


TF1* TrappingCorrectionAm241::GenerateCTDFunction(double CTDFitMin, double CTDFitMax, double CTDGuess, double FlipSwitch)
{
  // Exponentially modified gaussian
  TF1* CTDFunction = new TF1("CTDFunction", "[0]*([1]/2)*exp(([1]/2)*(([1]*[3]*[3]) - 2*[4]*(x-[2])))*erfc((([1]*[3]*[3]) - [4]*(x-[2]))/([3]*sqrt(2)))", CTDFitMin, CTDFitMax);

  CTDFunction->SetParName(0, "Norm");
  CTDFunction->SetParName(1, "Lambda");
  CTDFunction->SetParName(2, "Mu");
  CTDFunction->SetParName(3, "Sigma");
  CTDFunction->SetParName(4, "Flip");

  CTDFunction->SetParameter("Norm", 1000);
  CTDFunction->SetParameter("Lambda", 0.05);
  CTDFunction->SetParameter("Sigma", 12);
  CTDFunction->SetParameter("Mu", CTDGuess);
  CTDFunction->FixParameter(4, FlipSwitch);

  CTDFunction->SetParLimits(0, 0, 1e8);
  CTDFunction->SetParLimits(1, 0.01, 1);
  CTDFunction->SetParLimits(2, CTDFitMin, CTDFitMax);
  CTDFunction->SetParLimits(3, 6, 30);

  return CTDFunction;
}


////////////////////////////////////////////////////////////////////////////////


TrappingCorrectionAm241* g_Prg = 0;
int g_NInterruptCatches = 1;

MStripHit* TrappingCorrectionAm241::GetDominantStrip(vector<MStripHit*>& Strips, double& EnergyFraction)
{
  double MaxEnergy = -numeric_limits<double>::max(); // AZ: When both energies are zero (which shouldn't happen) we still pick one
  double TotalEnergy = 0.0;
  MStripHit* MaxStrip = nullptr;

  // Iterate through strip hits and get the strip with highest energy
  for (const auto SH : Strips) {
    double Energy = SH->GetEnergy();
    TotalEnergy += Energy;
    if (Energy > MaxEnergy) {
      MaxStrip = SH;
      MaxEnergy = Energy;
    }
  }
  if (TotalEnergy == 0) {
    EnergyFraction = 0;
  } else {
    EnergyFraction = MaxEnergy/TotalEnergy;
  }
  return MaxStrip;
}


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

  TApplication TrappingCorrectionApp("TrappingCorrectionApp", 0, 0);

  g_Prg = new TrappingCorrectionAm241();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis!"<<endl;
    return -2;
  } 

  TrappingCorrectionApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
