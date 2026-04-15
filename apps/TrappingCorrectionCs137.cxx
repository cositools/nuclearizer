/* 
 * TrappingCorrectionCs137.cxx
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
#include <fstream>
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
#include "MModuleEnergyCalibration.h"
#include "MModuleEventFilter.h"
#include "MModuleStripPairingMultiRoundChiSquare.h"
#include "MModuleStripPairingChiSquare.h"
#include "MModuleTACcut.h"
#include "MAssembly.h"


double g_MinCTD = -400;
double g_MaxCTD = 400;
int g_MinCounts = 100;

int g_HVStrips = 64;
int g_LVStrips = 64;

double g_CsPhotopeak = 661.7;

////////////////////////////////////////////////////////////////////////////////


//! A standalone program based on MEGAlib and ROOT
class TrappingCorrectionCs137
{
public:
  //! Default constructor
  TrappingCorrectionCs137();
  //! Default destructor
  ~TrappingCorrectionCs137();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what ever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  //! Produce functions for fitting
  TF1* GenerateCTDFunction(double CTDFitMin, double CTDFitMax, double CTDGuess);
  TF1* GeneratePhotopeakFunction();

  MStripHit* GetDominantStrip(vector<MStripHit*>& Strips, double& EnergyFraction);

  private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The input file name
  MString m_FileName;
  MString m_EcalFile;
  MString m_TACCalFile;
  MString m_TACCutFile;
  MString m_StripMapFile;
  //! output file names
  MString m_OutFile;
  //! option to do a pixel-by-pixel calibration (instead of detector-by-detector)
  bool m_PixelCorrect;
  bool m_MultiRoundStripPairing;
  bool m_ExcludeNN;
  bool m_ContinueHDF5;

  double m_MinEnergy;
  double m_MaxEnergy;

};

////////////////////////////////////////////////////////////////////////////////


//! Default constructor
TrappingCorrectionCs137::TrappingCorrectionCs137() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
TrappingCorrectionCs137::~TrappingCorrectionCs137()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool TrappingCorrectionCs137::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: TrappingCorrection <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -i:   input file name (.hdf5 or .txt with list of hdf5s)"<<endl;
  Usage<<"         --emin:   minimum Event energy (default 30 keV)"<<endl;
  Usage<<"         --emax:   maximum Event energy (default 5000 kev)"<<endl;
  Usage<<"         -e:   energy calibration file (.ecal)"<<endl;
  Usage<<"         --tcal:   TAC calibration file"<<endl;
  Usage<<"         --tcut:   TAC cut file"<<endl;
  Usage<<"         -p:   do pixel-by-pixel correction"<<endl;
  Usage<<"         -m:   strip map file name (.map)"<<endl;
  Usage<<"         -g:   greedy strip pairing (default is chi-square)"<<endl;
  Usage<<"         -n:   exclude nearest neighbors"<<endl;
  Usage<<"         -o:   outfile (default YYYYMMDDHHMMSS)"<<endl;
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
  m_MultiRoundStripPairing = false;
  m_ContinueHDF5 = true;
  m_MinEnergy = 40;
  m_MaxEnergy = 5000;
  
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
    if ((Option == "-i") || (Option == "-o") || (Option == "--emin") || (Option == "--emax") || (Option == "--tcal") || (Option == "--tcut") || (Option == "-m")) {
      if (!((argc > i+1) && (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }  

    // Then fulfill the options:
    if (Option == "-i") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } 

    if (Option == "-e") {
      m_EcalFile = argv[++i];
      cout<<"Accepting file name: "<<m_EcalFile<<endl;
    } 

    if (Option == "--emin") {
      m_MinEnergy = stod(argv[++i]);
    } 

    if (Option == "--emax") {
      m_MaxEnergy = stod(argv[++i]);
    } 

    if (Option == "--tcal") {
      m_TACCalFile = argv[++i];
      cout<<"Accepting file name: "<<m_TACCalFile<<endl;
    } 

    if (Option == "--tcut") {
      m_TACCutFile = argv[++i];
      cout<<"Accepting file name: "<<m_TACCutFile<<endl;
    } 

    if (Option == "-o"){
      m_OutFile = argv[++i];
      cout<<"Accepting file name: "<<m_OutFile<<endl;
    }

    if (Option == "-m"){
      m_StripMapFile = argv[++i];
      cout<<"Accepting file name: "<<m_StripMapFile<<endl;
    }

    if (Option == "-p"){
      m_PixelCorrect = true;
    }

    if (Option == "-g"){
      m_MultiRoundStripPairing = true;
    }

    if (Option == "-n"){
      m_ExcludeNN = true;
    }
    if (Option == "--nocontinue") {
      m_ContinueHDF5 = false;
    } 

  }

  return true;
}




////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool TrappingCorrectionCs137::Analyze()
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
  FileNames.push_back(m_FileName);
  cout<<"file name stored"<<endl;

  // // Map the side integer to HV and LV labels
  // // i.e. 0=HV, 1=LV
  // vector<MString> IllumSide;
  // IllumSide.push_back(MString("HV"));
  // IllumSide.push_back(MString("LV"));
  // cout<<"HV and LV integers mapped"<<endl;

  // Make a directory in which to store the pixel-level data 
  MString PixelDir = m_OutFile + MString("_pixeldata");
  if (m_PixelCorrect==true) {
    if (std::filesystem::create_directories(PixelDir.Data())==false) {
      cout<<"Directory '"<<PixelDir<<"' already exists or could not be created."<<endl;
    }
  }


  // Prep CTD model parameters. Mirror the model about the x-axis depending which side is being analyzed. 
 
  double CTDFitMin = g_MinCTD;
  double CTDFitMax = g_MaxCTD;

  MString InputFile = FileNames[0];
  vector<MString> HDFNames;

  TH1::SetDefaultSumw2();

  map<int, TH1D*> CTDHistograms;
  map<int, TH1D*> HVEnergyHistograms;
  map<int, TH1D*> LVEnergyHistograms;

  // Read in the input files and make a list of hdf5 files to calibrate
  if ((InputFile.GetSubString(InputFile.Length() - 4)) == "hdf5") {
    HDFNames.push_back(InputFile);
    cout<<"hdf names loaded correctly"<<endl;
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
    MModuleEnergyCalibration* EnergyCalibrator;
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
    EnergyCalibrator = new MModuleEnergyCalibration();
    EnergyCalibrator->SetFileName(m_EcalFile);
    // EnergyCalibrator->EnablePreampTempCorrection(false);
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
    if (m_MultiRoundStripPairing == true) {
      Pairing = new MModuleStripPairingMultiRoundChiSquare();
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
      cout<<"event is clear??"<<endl;
      if (Loader->IsReady()) {
        cout<<"loader is ready"<<endl;

        // Load Event from HDF5 file, then do TAC cut/calibration, energy calibration, and filtering
        Loader->AnalyzeEvent(Event);
        cout<<"event loaded"<<endl;
        TACCalibrator->AnalyzeEvent(Event);
        cout<<"TAc cal loaded"<<endl;
        EnergyCalibrator->AnalyzeEvent(Event);
        cout<<"ecal loaded"<<endl;
        bool Unfiltered = EventFilter->AnalyzeEvent(Event);

        if (Unfiltered == true) {
          
          // Pair strips
          Pairing->AnalyzeEvent(Event);
          cout<<"pairing events"<<endl;

          // Only look at events with 1 Hit since we're interested in the 60keV photopeak.
          if ((Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true) && (Unfiltered==true) ) {
            
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
                    char name[64]; sprintf(name,"CTD: PixelID %d  ",PixelID,PixelDir.Data());
                    CTDHist = new TH1D(name, name, (g_MaxCTD - g_MinCTD)/2, g_MinCTD, g_MaxCTD);
                    CTDHist->SetXTitle("CTD (ns)");
                    CTDHist->SetYTitle("Hits");
                    CTDHistograms[PixelID] = CTDHist;
                  }
                  if (HVHist == nullptr) {
                    char name[64]; sprintf(name,"HV energy: PixelID %d  ",PixelID,PixelDir.Data());
                    HVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy)*2, m_MinEnergy, m_MaxEnergy);
                    HVHist->SetXTitle("HV Energy (keV)");
                    HVHist->SetYTitle("Hits");
                    HVEnergyHistograms[PixelID] = HVHist;
                  }
                  if (LVHist == nullptr) {
                    char name[64]; sprintf(name,"LV energy: PixelID %d  ",PixelID,PixelDir.Data());
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
      char name[64]; sprintf(name,"CTD: DetID %d  ",DetID,PixelDir.Data());
      CTDHist = new TH1D(name, name, (g_MaxCTD - g_MinCTD)/2, g_MinCTD, g_MaxCTD);
      CTDHist->SetXTitle("CTD (ns)");
      CTDHist->SetYTitle("Hits");
      FullDetCTDHistograms[DetID] = CTDHist;
    }
    if (HVHist == nullptr) {
      char name[64]; sprintf(name,"HV energy: DetID %d  ",DetID,PixelDir.Data());
      HVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy)*2, m_MinEnergy, m_MaxEnergy);
      HVHist->SetXTitle("HV Energy (keV)");
      HVHist->SetYTitle("Hits");
      FullDetHVEnergyHistograms[DetID] = HVHist;
    }
    if (LVHist == nullptr) {
      char name[64]; sprintf(name,"LV energy: DetID %d  ",DetID,PixelDir.Data());
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
      cout<<H.second->Integral()<<" counts in CTD histogram for Pixel ID "<<PixelID<<endl;
      cout<<HVEnergyHistograms[PixelID]->Integral()<<" counts in HV energy histogram for Pixel ID "<<PixelID<<endl;
      cout<<LVEnergyHistograms[PixelID]->Integral()<<" counts in LV energy histogram for Pixel ID "<<PixelID<<endl;

      if ((H.second->Integral() > g_MinCounts) && (HVEnergyHistograms[PixelID]->Integral() > g_MinCounts) && (LVEnergyHistograms[PixelID]->Integral() > g_MinCounts)) {
        
        // Initialize and run the CTD fit
        double CTDGuess = H.second->GetBinCenter(H.second->GetMaximumBin());
        TF1* CTDFunction = GenerateCTDFunction(CTDFitMin, CTDFitMax, CTDGuess);
        TFitResultPtr CTDFit = H.second->Fit(CTDFunction, "SQ", "", CTDFitMin, CTDFitMax);

        // Save the results of the CTD fit
        ofstream CTDFitFile(PixelDir +MString("/") + PixelID+MString("_CTDFitResult_")+ MString(".txt"));
        streambuf* coutbuf = cout.rdbuf();
        cout.rdbuf(CTDFitFile.rdbuf());
        if (CTDFit >= 0) {
            CTDFit->Print();
        }
        cout.rdbuf(coutbuf);
        CTDFitFile.close();
        
        // Initialize and run the HV photopeak fit
        TF1* PhotopeakFunctionHV = GeneratePhotopeakFunction();
        TFitResultPtr HVFit = HVEnergyHistograms[PixelID]->Fit(PhotopeakFunctionHV, "SQ", "", 642, 682);

        // Save the results of the HV photopeak fit
        ofstream HVFitFile(PixelDir +MString("/") + PixelID+MString("_HVEnergyFitResult_")+ MString(".txt"));
        coutbuf = cout.rdbuf();
        cout.rdbuf(HVFitFile.rdbuf());
        if (HVFit >= 0) {
            HVFit->Print();
        }
        cout.rdbuf(coutbuf);
        HVFitFile.close();

        // Initialize and run the LV photopeak fit
        TF1* PhotopeakFunctionLV = GeneratePhotopeakFunction();
        TFitResultPtr LVFit = LVEnergyHistograms[PixelID]->Fit(PhotopeakFunctionLV, "SQ", "", 642, 682);
        
        // Save the results of the LV photopeak fit
        ofstream LVFitFile(PixelDir +MString("/") + PixelID+MString("_LVEnergyFitResult_")+ MString(".txt"));
        coutbuf = cout.rdbuf();
        cout.rdbuf(LVFitFile.rdbuf());
        if (LVFit >= 0) {
            LVFit->Print();
        }
        cout.rdbuf(coutbuf);
        LVFitFile.close();

        // Save the resulting HV and LV photopeak centroids and CTD centroids to the Endpoints map.
        // Only save these values if the fits ran successfully and if their reduced chi-square is less than 5
        if ((CTDFit >= 0) && (HVFit >= 0) && (LVFit >= 0)) { 
          if (((CTDFit->Chi2()/CTDFit->Ndf()) < 25) && ((HVFit->Chi2()/HVFit->Ndf()) < 25) && ((LVFit->Chi2()/LVFit->Ndf()) < 25)) {
            Endpoints[PixelID][0].push_back(CTDFit->Parameter(2));
            Endpoints[PixelID][0].push_back(HVFit->Parameter(1));
            Endpoints[PixelID][0].push_back(LVFit->Parameter(1));
          } else {
            cout<<"Fits for Pixel "<<PixelID<<" did not pass the chi2 cut"<<endl; 
          }
        } else {
          cout<<"Fits failed for Pixel "<<PixelID<<endl;
        }
      } else {
        cout<<"Fewer than "<<g_MinCounts<<" counts in Pixel ID "<<PixelID<<endl;
      }

      TFile CTDHistFile(PixelDir+MString("/")+PixelID+MString("_CTDHist_") + MString("Illum.root"),"recreate");
      H.second->Write();
      CTDHistFile.Close();

      TFile HVHistFile(PixelDir+MString("/")+PixelID+MString("_HVEnergyHist_") + MString("Illum.root"),"recreate");
      HVEnergyHistograms[PixelID]->Write();
      HVHistFile.Close();

      TFile LVHistFile(PixelDir+MString("/")+PixelID+MString("_LVEnergyHist_") + MString("Illum.root"),"recreate");
      LVEnergyHistograms[PixelID]->Write();
      LVHistFile.Close();

    }
  }

  // Do the same function fitting and recording as above, but for the full detectors rather than pixel-by-pixel
  for (auto H: FullDetCTDHistograms) {

    cout<<"Analyzing Full Detector Histograms"<<endl;

    int DetID = H.first;

    cout<<"full detector HV counts"<<FullDetHVEnergyHistograms[DetID]->Integral()<<endl;
    cout<<"full detector LV counts"<<FullDetLVEnergyHistograms[DetID]->Integral()<<endl;
    cout<<"full detector CTD counts"<<FullDetCTDHistograms[DetID]->Integral()<<endl;
    
    if ((H.second->Integral() > g_MinCounts) && (FullDetHVEnergyHistograms[DetID]->Integral() > g_MinCounts) && (FullDetLVEnergyHistograms[DetID]->Integral() > g_MinCounts)) {

      double CTDGuess = H.second->GetBinCenter(H.second->GetMaximumBin());
      TF1* CTDFunction = GenerateCTDFunction(CTDFitMin, CTDFitMax, CTDGuess);
      TFitResultPtr CTDFit = H.second->Fit(CTDFunction, "SQ", "", CTDFitMin, CTDFitMax);

      TF1* PhotopeakFunctionHV = GeneratePhotopeakFunction();
      TFitResultPtr HVFit = FullDetHVEnergyHistograms[DetID]->Fit(PhotopeakFunctionHV, "SQ", "", 642, 682);

      TF1* PhotopeakFunctionLV = GeneratePhotopeakFunction();
      TFitResultPtr LVFit = FullDetLVEnergyHistograms[DetID]->Fit(PhotopeakFunctionLV, "SQ", "", 642, 682);

      if ((CTDFit >= 0) && (HVFit >= 0) && (LVFit >= 0)) {
        FullDetEndpoints[DetID][0].push_back(CTDFit->Parameter(2));
        FullDetEndpoints[DetID][0].push_back(HVFit->Parameter(1));
        FullDetEndpoints[DetID][0].push_back(LVFit->Parameter(1));

        ofstream CTDFitFile(DetID+MString("_CTDFitResult_")+ MString(".txt"));
        streambuf* coutbuf = cout.rdbuf();
        cout.rdbuf(CTDFitFile.rdbuf());
        if (CTDFit >= 0) {
            CTDFit->Print();
        }
        cout.rdbuf(coutbuf);
        CTDFitFile.close();

        ofstream HVFitFile(DetID+MString("_HVEnergyFitResult_")+ MString(".txt"));
        coutbuf = cout.rdbuf();
        cout.rdbuf(HVFitFile.rdbuf());
        if (HVFit >= 0) {
            HVFit->Print();
        }
        cout.rdbuf(coutbuf);
        HVFitFile.close();
        
        ofstream LVFitFile(DetID+MString("_LVEnergyFitResult_")+ MString(".txt"));
        coutbuf = cout.rdbuf();
        cout.rdbuf(LVFitFile.rdbuf());
        if (LVFit >= 0) {
            LVFit->Print();
        }
        cout.rdbuf(coutbuf);
        LVFitFile.close();

        TFile CTDFile(m_OutFile+MString("_Det")+DetID+MString("_CTDHist_") + MString("Illum.root"),"recreate");

        TCanvas* CTDCanvas = new TCanvas();
        CTDCanvas->cd();
        H.second->Draw("hist");
        CTDFunction->Draw("same");

        H.second->Write();
        CTDFile.Close();

        TFile HVHistFile(m_OutFile+MString("_Det")+DetID+MString("_HVEnergyHist_") + MString("Illum.root"),"recreate");

        TCanvas* HVHistCanvas = new TCanvas();
        HVHistCanvas->cd();
        FullDetHVEnergyHistograms[DetID]->Draw("hist");
        PhotopeakFunctionHV->Draw("same");

        FullDetHVEnergyHistograms[DetID]->Write();
        HVHistFile.Close();

        TFile LVHistFile(m_OutFile+MString("_Det")+DetID+MString("_LVEnergyHist_") + MString("Illum.root"),"recreate");

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


  //setup parameter file
  ofstream OutputCalFile;
  OutputCalFile.open(m_OutFile+MString("_parameters.txt"));
  OutputCalFile<<"# Det ID"<<'\t'<<"HV Strip ID"<<'\t'<<"LV Strip ID"<<'\t'<<"HV Illum CTD"<<'\t'<<"LV Illum CTD"<<'\t'<<"HV Illum. HV Centroid"<<'\t'<<"LV Illum. HV Centroid"<<'\t'<<"HV Illum. LV Centroid"<<'\t'<<"LV Illum. LV Centroid"<<endl<<endl;

  // map<int, TH2D*> DeltaHVMap;
  // map<int, TH2D*> DeltaLVMap;

  // map<int, TH1D*> DeltaHVHist;
  // map<int, TH1D*> DeltaLVHist;

  // Write the results of good fits to the parameter file and produce summary plots.
  // If an individual pixel did not produce a good fit, default to the detector-level parameters.
  // Loop over each detector
  for (auto E: FullDetEndpoints) {
    
    int DetID = E.first;

    double CTD = 0;
    double HVCentroid = 0;
    double LVCentroid = 0;

    if ((E.second[0].size() > 0) && (E.second[1].size() > 0)) {
      CTD = E.second[0][0];
      HVCentroid = E.second[1][0];
      LVCentroid = E.second[0][1];
    }

    // Loop over each pixel
    for (int hv=0; hv<g_HVStrips; ++hv) {
      for (int lv=0; lv<g_LVStrips; ++lv) {
        if (m_PixelCorrect==true) {

          int PixelID = (10000*DetID) + (100*lv) + hv;

    //       TH2D* TempHVMap = DeltaHVMap[DetID];
    //       TH2D* TempLVMap = DeltaLVMap[DetID];

    //       TH1D* TempHVHist = DeltaHVHist[DetID];
    //       TH1D* TempLVHist = DeltaLVHist[DetID];
          
    //       // Create the histograms and pixel maps if they don't exist yet
    //       if (TempHVMap == nullptr) {

    //         char HVname[64]; sprintf(HVname,"Delta HV Map: Det %d",DetID);
    //         TempHVMap = new TH2D(HVname, HVname, g_HVStrips, -0.5, g_HVStrips-0.5, g_LVStrips, -0.5, g_LVStrips-0.5);
    //         TempHVMap->SetXTitle("HV Strip");
    //         TempHVMap->SetYTitle("LV Strip");
    //         TempHVMap->SetZTitle("Delta HV Energy");
            
    //         char LVname[64]; sprintf(LVname,"Delta LV Map: Det %d",DetID);
    //         TempLVMap = new TH2D(LVname, LVname, g_HVStrips, -0.5, g_HVStrips-0.5, g_LVStrips, -0.5, g_LVStrips-0.5);
    //         TempLVMap->SetXTitle("HV Strip");
    //         TempLVMap->SetYTitle("LV Strip");
    //         TempLVMap->SetZTitle("Delta LV Energy");

    //         DeltaHVMap[DetID] = TempHVMap;
    //         DeltaLVMap[DetID] = TempLVMap;
    //       }

    //       if (TempHVHist == nullptr) {

    //         char HVname[64]; sprintf(HVname,"Delta HV Hist: Det %d",DetID);
    //         TempHVHist = new TH1D(HVname, HVname, 50, -3.0, 3.0);
    //         TempHVHist->SetXTitle("Delta HV Energy (keV)");
    //         TempHVHist->SetYTitle("Number of pixels");
            
    //         char LVname[64]; sprintf(LVname,"Delta LV Hist: Det %d",DetID);
    //         TempLVHist = new TH1D(LVname, LVname, 50, -3.0, 3.0);
    //         TempLVHist->SetXTitle("Delta LV Energy (keV)");
    //         TempLVHist->SetYTitle("Number of pixels");

    //         DeltaHVHist[DetID] = TempHVHist;
    //         DeltaLVHist[DetID] = TempLVHist;
    //       }
          
    //       DeltaHVMap[DetID]->SetBinContent(hv+1, lv+1, -100);
    //       DeltaLVMap[DetID]->SetBinContent(hv+1, lv+1, -100);

    //       // Retrieve the stored HV and LV energies and CTDs
    //       // Plot the energy shifts in the HV and LV Maps, and fill in the histograms
    //       if (Endpoints.find(PixelID)!=Endpoints.end()) {
    //         if ((Endpoints[PixelID][0].size() > 0) && (Endpoints[PixelID][1].size() > 0)) {
              
    //           HVIllumCTD = Endpoints[PixelID][0][0];
    //           LVIllumCTD = Endpoints[PixelID][1][0];
    //           HVIllumHVCentroid = Endpoints[PixelID][0][1];
    //           LVIllumHVCentroid = Endpoints[PixelID][1][1];
    //           HVIllumLVCentroid = Endpoints[PixelID][0][2];
    //           LVIllumLVCentroid = Endpoints[PixelID][1][2];

    //           // Normalize the differences in Energy to account for calibration differences
    //           double HVDiff = (HVIllumHVCentroid - LVIllumHVCentroid)*(g_CsPhotopeak/HVIllumHVCentroid);
    //           double LVDiff = (HVIllumLVCentroid - LVIllumLVCentroid)*(g_CsPhotopeak/HVIllumLVCentroid);

    //           DeltaHVMap[DetID]->SetBinContent(hv+1, lv+1, HVDiff);
    //           DeltaLVMap[DetID]->SetBinContent(hv+1, lv+1, LVDiff);

    //           DeltaHVHist[DetID]->Fill(HVDiff);
    //           DeltaLVHist[DetID]->Fill(LVDiff);
    //         }
    //       }
    //     }

        // Record the Energies and CTDs in the parameter file
        // Note that we default to the detector value if the pixel value isn't available and to 0 if the detector value isn't available.
        OutputCalFile<<to_string(DetID)<<'\t'<<to_string(hv)<<'\t'<<to_string(lv)<<'\t'<<to_string(CTD)<<'\t'<<to_string(CTD)<<'\t'<<to_string(HVCentroid)<<'\t'<<to_string(LVCentroid)<<endl<<endl;
      }
    }
  }

  OutputCalFile.close();
  watch.Stop();
  cout<<"total time (s): "<<watch.CpuTime()<<endl;
 
  return true;
  }
}

  // Save and plot the summary figures.
  // if (m_PixelCorrect==true) {
  //   for (auto H: DeltaHVMap) {
      
  //     int DetID = H.first;

  //     TFile HVFile(m_OutFile+MString("_Det")+DetID+MString("_DeltaHVMap.root"),"recreate");
  //     TCanvas* HVCanvas = new TCanvas();
  //     HVCanvas->cd();
  //     H.second->SetMinimum(-5.);
  //     H.second->Draw("colz");
  //     H.second->Write();
  //     HVFile.Close();

  //     TFile LVFile(m_OutFile+MString("_Det")+DetID+MString("_DeltaLVMap.root"),"recreate");
  //     TCanvas* LVCanvas = new TCanvas();
  //     LVCanvas->cd();
  //     DeltaLVMap[DetID]->SetMinimum(-5.);
  //     DeltaLVMap[DetID]->Draw("colz");
  //     DeltaLVMap[DetID]->Write();
  //     LVFile.Close();

  //     TFile HVHistFile(m_OutFile+MString("_Det")+DetID+MString("_DeltaHVHist.root"),"recreate");
  //     TCanvas* HVHistCanvas = new TCanvas();
  //     HVHistCanvas->cd();
  //     DeltaHVHist[DetID]->Draw("hist");
  //     DeltaHVHist[DetID]->Write();
  //     HVHistFile.Close();

  //     TFile LVHistFile(m_OutFile+MString("_Det")+DetID+MString("_DeltaLVHist.root"),"recreate");
  //     TCanvas* LVHistCanvas = new TCanvas();
  //     LVHistCanvas->cd();
  //     DeltaLVHist[DetID]->Draw("hist");
  //     DeltaLVHist[DetID]->Write();
  //     LVHistFile.Close();
  //   }
  // }

//   watch.Stop();
//   cout<<"total time (s): "<<watch.CpuTime()<<endl;

//   return true;
// }


////////////////////////////////////////////////////////////////////////////////


TF1* TrappingCorrectionCs137::GeneratePhotopeakFunction()
{
  // Gaussian with a low-E shelf
  TF1* PhotopeakFunction = new TF1("PhotopeakFunction", "gaus(0) + [0]*[3]*(1 - erf((x-[1])/(sqrt(2)*[2])))", 650, 670);

  PhotopeakFunction->SetParName(0, "Gauss norm");
  PhotopeakFunction->SetParName(1, "Mu");
  PhotopeakFunction->SetParName(2, "Sigma");
  PhotopeakFunction->SetParName(3, "Shelf norm");

  PhotopeakFunction->SetParameter("Gauss norm", 1000);
  PhotopeakFunction->SetParameter("Mu", 662);
  PhotopeakFunction->SetParameter("Sigma", 2);
  PhotopeakFunction->SetParameter("Shelf norm", 0.05);

  PhotopeakFunction->SetParLimits(0, 10, 1e8);
  PhotopeakFunction->SetParLimits(1, 650, 670);
  PhotopeakFunction->SetParLimits(2, 1.0, 10);
  PhotopeakFunction->SetParLimits(3, 0, 0.1);

  return PhotopeakFunction;
}


////////////////////////////////////////////////////////////////////////////////


TF1* TrappingCorrectionCs137::GenerateCTDFunction(double CTDFitMin, double CTDFitMax, double CTDGuess)
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

  CTDFunction->SetParLimits(0, 0, 1e8);
  CTDFunction->SetParLimits(1, 0.01, 1);
  CTDFunction->SetParLimits(2, CTDFitMin, CTDFitMax);
  CTDFunction->SetParLimits(3, 6, 30);

  return CTDFunction;
}


////////////////////////////////////////////////////////////////////////////////


TrappingCorrectionCs137* g_Prg = 0;
int g_NInterruptCatches = 1;

MStripHit* TrappingCorrectionCs137::GetDominantStrip(vector<MStripHit*>& Strips, double& EnergyFraction)
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

  g_Prg = new TrappingCorrectionCs137();

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
