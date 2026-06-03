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


double g_MinCTD = -350;
double g_MaxCTD = 350;
int g_MinCounts = 100;

int g_HVStrips = 64;
int g_LVStrips = 64;

double g_CsPhotopeak = 661.7;

const int NCTDBins = 5;
double CTDBinWidth = (g_MaxCTD - g_MinCTD) / NCTDBins;

int GetCTDBin(double CTD) {
  if (CTD < g_MinCTD || CTD >= g_MaxCTD) return -1;
  return int((CTD - g_MinCTD) / CTDBinWidth);
}

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
  m_MinEnergy = 600;
  m_MaxEnergy = 700;
  
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
  // time code just to see
  TStopwatch watch;
  watch.Start();

  if (m_Interrupt == true) return false;

  // [CTD, HV Energy, LV Energy] for full detector values
 map<int, map<int, vector<double>>> FullDetEndpoints;

  // Store the input files
  vector<MString> FileNames;
  FileNames.push_back(m_FileName);
  cout << "file name stored" << endl;

  double CTDFitMin = g_MinCTD;
  double CTDFitMax = g_MaxCTD;

  MString InputFile = FileNames[0];
  cout << " input file stored as: " << InputFile << endl;
  vector<MString> HDFNames;

  TH1::SetDefaultSumw2();

  // Detector-level maps organized by [CTDBin][DetID]
  map<int, map<int, TH1D*>> FullDetCTDHistograms;
  map<int, map<int, TH1D*>> FullDetHVEnergyHistograms;
  map<int, map<int, TH1D*>> FullDetLVEnergyHistograms;

  // Read in the input files and make a list of hdf5 files to calibrate
  if ((InputFile.GetSubString(InputFile.Length() - 4)) == "hdf5") {
    HDFNames.push_back(InputFile);
    cout << "hdf names loaded correctly" << endl;
  } else if ((InputFile.GetSubString(InputFile.Length() - 3)) == "txt") {
    cout << "Reading input file " << InputFile << endl;
    cout << "WARNING: When passing a list of files, ensure that you have chosen the correct HDF5 continuous reading mode. Use the --nocontinue option to suppress continuous file reading." << endl;
    MFile F;
    if (F.Open(InputFile) == false) {
      cout << "Error: Failed to open input file." << endl;
    } else {
      MString Line;
      while (F.ReadLine(Line)) {
        MString Trimmed = Line.Trim();
        if ((Trimmed != "")) {
          if (F.Exists(Trimmed) == true) {
            HDFNames.push_back(Trimmed);
          } else {
            cout << "Error: Could not find file " << Trimmed << endl;
          }
        }
      }
    }
  } else {
    cout << "Error: Unrecognized file format: " << InputFile << endl;
  }

  // Analyze all the data and fill in the histograms
  for (unsigned int f = 0; f < HDFNames.size(); ++f) {

    MString File = HDFNames[f];
    cout << "Beginning analysis of file " << File << endl;

    // Create and initialize nuclearizer modules
    MSupervisor* S = MSupervisor::GetSupervisor();
    
    MModuleLoaderMeasurementsHDF* Loader;
    MModuleTACcut* TACCalibrator;
    MModuleEnergyCalibration* EnergyCalibrator;
    MModuleEventFilter* EventFilter;

    unsigned int MNumber = 0;
    cout << "Creating HDF5 loader" << endl;
    Loader = new MModuleLoaderMeasurementsHDF();
    Loader->SetFileNameStripMap(m_StripMapFile);
    Loader->SetFileName(File);
    Loader->SetLoadContinuationFiles(m_ContinueHDF5);
    S->SetModule(Loader, MNumber);
    ++MNumber;

    cout << "Creating TAC calibrator" << endl;
    TACCalibrator = new MModuleTACcut();
    TACCalibrator->SetTACCalFileName(m_TACCalFile);
    TACCalibrator->SetTACCutFileName(m_TACCutFile);
    S->SetModule(TACCalibrator, MNumber);
    ++MNumber;
    
    cout << "Creating energy calibrator" << endl;
    EnergyCalibrator = new MModuleEnergyCalibration();
    EnergyCalibrator->SetFileName(m_EcalFile);
    S->SetModule(EnergyCalibrator, MNumber);
    ++MNumber;

    cout << "Creating Event filter" << endl;
    EventFilter = new MModuleEventFilter();
    EventFilter->SetMinimumLVStrips(1);
    EventFilter->SetMaximumLVStrips(3);
    EventFilter->SetMinimumHVStrips(1);
    EventFilter->SetMaximumHVStrips(3);
    EventFilter->SetMinimumHits(0);
    EventFilter->SetMaximumHits(100);
    EventFilter->SetMinimumTotalEnergy(m_MinEnergy);
    EventFilter->SetMaximumTotalEnergy(m_MaxEnergy * 2); 
    S->SetModule(EventFilter, MNumber);
    ++MNumber;
    
    cout << "Creating strip pairing" << endl;
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
    cout<<"Modules initialized, starting event loop"<<endl;

    cout << "Analyzing..." << endl;
    while ((IsFinished == false) && (m_Interrupt == false)) {
      Event->Clear();
      if (Loader->IsReady()) {

        Loader->AnalyzeEvent(Event);
        TACCalibrator->AnalyzeEvent(Event);
        EnergyCalibrator->AnalyzeEvent(Event);
        bool Unfiltered = EventFilter->AnalyzeEvent(Event);

        if (Unfiltered == true) {
          
          Pairing->AnalyzeEvent(Event);

          if ((Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true) && (Unfiltered == true)) {
            
            for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
              double HVEnergy = 0.0;
              double LVEnergy = 0.0;
              vector<MStripHit*> HVStrips;
              vector<MStripHit*> LVStrips;

              MHit* H = Event->GetHit(h);
              int DetID = H->GetStripHit(0)->GetDetectorID();

              for (unsigned int sh = 0; sh < H->GetNStripHits(); ++sh) {
                MStripHit* SH = H->GetStripHit(sh);

                if ((m_ExcludeNN == false) || ((m_ExcludeNN == true) && (SH->IsNearestNeighbor() == false))) {
                  if (SH->IsLowVoltageStrip() == true) {
                    LVEnergy += SH->GetEnergy();
                    LVStrips.push_back(SH);
                  } else {
                    HVEnergy += SH->GetEnergy();
                    HVStrips.push_back(SH);
                  }
                }
              }

              if ((HVStrips.size() > 0) && (LVStrips.size() > 0)) {
                
                double HVEnergyFraction = 0;
                double LVEnergyFraction = 0;
                MStripHit* HVSH = GetDominantStrip(HVStrips, HVEnergyFraction); 
                MStripHit* LVSH = GetDominantStrip(LVStrips, LVEnergyFraction);
                
                if ((LVSH->HasCalibratedTiming() == true) && (HVSH->HasCalibratedTiming() == true)) {
                  
                  double CTD = LVSH->GetTiming() - HVSH->GetTiming();
                  int CTDBin = GetCTDBin(CTD);
                  if (CTDBin < 0) continue;

                  TH1D* FullCTDHist = FullDetCTDHistograms[CTDBin][DetID];
                  TH1D* FullHVHist  = FullDetHVEnergyHistograms[CTDBin][DetID];
                  TH1D* FullLVHist  = FullDetLVEnergyHistograms[CTDBin][DetID];

                  if (FullCTDHist == nullptr) {
                    char name[128]; sprintf(name, "CTD_Detector%d_bin%d", DetID, CTDBin);
                    FullCTDHist = new TH1D(name, name, (g_MaxCTD - g_MinCTD) / 2, g_MinCTD, g_MaxCTD);
                    FullDetCTDHistograms[CTDBin][DetID] = FullCTDHist;
                  }
                  if (FullHVHist == nullptr) {
                    char name[128]; sprintf(name, "HV_Detector%d_bin%d", DetID, CTDBin);
                    FullHVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy) * 2, m_MinEnergy, m_MaxEnergy);
                    FullDetHVEnergyHistograms[CTDBin][DetID] = FullHVHist;
                  }
                  if (FullLVHist == nullptr) {
                    char name[128]; sprintf(name, "LV_Detector%d_bin%d", DetID, CTDBin);
                    FullLVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy) * 2, m_MinEnergy, m_MaxEnergy);
                    FullDetLVEnergyHistograms[CTDBin][DetID] = FullLVHist;
                  }

                  FullCTDHist->Fill(CTD);
                  FullHVHist->Fill(HVEnergy);
                  FullLVHist->Fill(LVEnergy);
                }
              }
            }
          }
        }
      }
      IsFinished = Loader->IsFinished();
    }
  }
  
  // Do function fitting and recording for full detector outputs
  for (int c = 0; c < NCTDBins; ++c) {

    cout << "Processing CTD bin " << c << endl;
  
    for (auto const& [DetID, FullCTDHist] : FullDetCTDHistograms[c]) {
  
      TH1D* HVHist = FullDetHVEnergyHistograms[c][DetID];
      TH1D* LVHist = FullDetLVEnergyHistograms[c][DetID];

      if (FullCTDHist->Integral() > g_MinCounts && HVHist->Integral() > g_MinCounts && LVHist->Integral() > g_MinCounts) {

        double CTDGuess = FullCTDHist->GetBinCenter(FullCTDHist->GetMaximumBin());
        TF1* CTDFunction = GenerateCTDFunction(CTDFitMin, CTDFitMax, CTDGuess);
        TFitResultPtr CTDFit = FullCTDHist->Fit(CTDFunction, "SQ", "", CTDFitMin, CTDFitMax);

        TF1* PhotopeakFunctionHV = GeneratePhotopeakFunction();
        // PhotopeakFunctionHV->FixParameter(4, 0.5); 
        // PhotopeakFunctionHV->FixParameter(5, 0.80); 
        // PhotopeakFunctionHV->FixParameter(6, 0.13); 
        // PhotopeakFunctionHV->FixParameter(7, 0.028); 
        TFitResultPtr HVFit = HVHist->Fit(PhotopeakFunctionHV, "S", "", 645, 675);

        TF1* PhotopeakFunctionLV = GeneratePhotopeakFunction();
        TFitResultPtr LVFit = LVHist->Fit(PhotopeakFunctionLV, "S", "", 645, 675);

        if ((CTDFit >= 0) && (HVFit >= 0) && (LVFit >= 0)) {
      
          // Clear or initialize the vector for this specific bin and detector
          FullDetEndpoints[c][DetID].clear();
          
          // Parameter(2) is Mu for your CTD function, Parameter(1) is Mu for the Photopeaks
          FullDetEndpoints[c][DetID].push_back(CTDFit->Parameter(2));  // Index 0: CTD Centroid
          FullDetEndpoints[c][DetID].push_back(HVFit->Parameter(1));   // Index 1: HV Photopeak Mu
          FullDetEndpoints[c][DetID].push_back(LVFit->Parameter(1));   // Index 2: LV Photopeak Mu
        

          ofstream CTDFitFile(DetID + MString("_CTDFitResult_.txt"));
          streambuf* coutbuf = cout.rdbuf();
          cout.rdbuf(CTDFitFile.rdbuf());
          cout << "CTD Fit Results for Detector " << DetID << " in CTD bin " << c << endl;
          if (CTDFit >= 0) {
            CTDFit->Print();
          }
          cout.rdbuf(coutbuf);
          CTDFitFile.close();

          ofstream HVFitFile(DetID + MString("_CTDbin_") + c + MString("_HVEnergyFitResult_.txt"));
          coutbuf = cout.rdbuf();
          cout.rdbuf(HVFitFile.rdbuf());
          if (HVFit >= 0) {
            HVFit->Print();
          }
          cout.rdbuf(coutbuf);
          HVFitFile.close();
          
          ofstream LVFitFile(DetID + MString("_CTDbin_") + c + MString("_LVEnergyFitResult_.txt"));
          coutbuf = cout.rdbuf();
          cout.rdbuf(LVFitFile.rdbuf());
          if (LVFit >= 0) {
            LVFit->Print();
          }
          cout.rdbuf(coutbuf);
          LVFitFile.close();

          TFile CTDFile(m_OutFile + MString("_Det") + DetID + MString("_CTDbin_") + c + MString("_CTDHist_Illum.root"), "recreate");
          TCanvas* CTDCanvas = new TCanvas();
          CTDCanvas->cd();
          FullCTDHist->Draw("hist");
          CTDFunction->Draw("same");
          FullCTDHist->Write();
          CTDFile.Close();

          TFile HVHistFile(m_OutFile + MString("_Det") + DetID + MString("_CTDbin_") + c + MString("_HVEnergyHist_Illum.root"), "recreate");
          TCanvas* HVHistCanvas = new TCanvas();
          HVHistCanvas->cd();
          HVHist->Draw("Hist");
          PhotopeakFunctionHV->Draw("same");
          HVHistCanvas->Write();
          HVHistFile.Close();

          TFile LVHistFile(m_OutFile + MString("_Det") + DetID + MString("_CTDbin_") + c + MString("_LVEnergyHist_Illum.root"), "recreate");
          TCanvas* LVHistCanvas = new TCanvas();
          LVHistCanvas->cd();
          LVHist->Draw("Hist");
          PhotopeakFunctionLV->Draw("same");
          LVHistCanvas->Write();
          LVHistFile.Close();

        } else {
          cout << "Fits failed for CTD bin " << c << " Detector " << DetID << endl;
        }
      } else {
        cout << "Fewer than " << g_MinCounts << " counts in CTD bin " << c << " Detector " << DetID << endl;
      }
    }
  }

  // Setup parameter file
  ofstream OutputCalFile;
  OutputCalFile.open(m_OutFile + MString("_parameters.txt"));
  
  // Updated header matching your request
  OutputCalFile << "CTD_Bin" << '\t' 
                << "Det_ID" << '\t' 
                << "CTD_Centroid_ns" << '\t' 
                << "HV_Centroid_keV" << '\t' 
                << "LV_Centroid_keV" << endl;
  cout << "Parameter file set up" << endl;

  // Loop systematically over each CTD bin first
  for (int c = 0; c < NCTDBins; ++c) {
    cout << "Writing output tracking parameters for CTD bin: " << c << endl;

    // Loop over the detectors found inside this specific CTD bin
    for (auto const& [DetID, FitsVec] : FullDetEndpoints[c]) {
      
      double CTDCentroid = 0.0;
      double HVCentroid  = 0.0;
      double LVCentroid  = 0.0;

      // Ensure all 3 parameters (CTD mu, HV mu, LV mu) were successfully saved
      if (FitsVec.size() >= 3) {
        CTDCentroid = FitsVec[0];
        HVCentroid  = FitsVec[1];
        LVCentroid  = FitsVec[2];
      }

      // Write row entries corresponding purely to the detector level measurements
      OutputCalFile << c << '\t'
                    << DetID << '\t'
                    << CTDCentroid << '\t'
                    << HVCentroid << '\t'
                    << LVCentroid << endl;
    }
  }

  OutputCalFile.close();
  watch.Stop();
  cout << "total time (s): " << watch.CpuTime() << endl;
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


TF1* TrappingCorrectionCs137::GeneratePhotopeakFunction()
{
  // Component 1: Core Gaussian
  // exp(-(x-x0)^2 / (2*sigma^2))
  MString gaussStr = "exp(-(x-[1])^2 / (2*[2]^2))";

  // Component 2: Exponential Tail + Shelf
  // BoverA * exp(gamma*(x-x0)) * 0.5 * erfc((x-x0)/(sigma*sigma_ratio*sqrt(2)))
  MString expTailStr = "[3] * exp([4]*(x-[1])) * 0.5 * erfc((x-[1])/([2]*[5]*sqrt(2)))";

  // Component 3: Linear Tail + Shelf
  // BoverA * CoverB * (1 + D*(x-x0)) * 0.5 * erfc((x-x0)/(sigma*sigma_ratio*sqrt(2)))
  MString linTailStr = "[3] * [6] * (1 + [7]*(x-[1])) * 0.5 * erfc((x-[1])/([2]*[5]*sqrt(2)))";

  // Combine components with an overall normalization scaling factor [0]
  MString fullFormula = "[0] * (" + gaussStr + " + " + expTailStr + " + " + linTailStr + ")";

  // Instantiate TF1 over your expected fit window
  TF1* PhotopeakFunction = new TF1("PhotopeakFunction", fullFormula.Data(), 645, 675);

  // Set Parameter Names
  PhotopeakFunction->SetParName(0, "Amplitude");
  PhotopeakFunction->SetParName(1, "x0 (Mu)");
  PhotopeakFunction->SetParName(2, "Sigma Gauss");
  PhotopeakFunction->SetParName(3, "BoverA");
  PhotopeakFunction->SetParName(4, "Gamma");
  PhotopeakFunction->SetParName(5, "Sigma Ratio");
  PhotopeakFunction->SetParName(6, "CoverB");
  PhotopeakFunction->SetParName(7, "D (Lin Slope)");

  // Provide initial sensible guesses for a Cs137 photopeak
  PhotopeakFunction->SetParameter("Amplitude", 1000);
  PhotopeakFunction->SetParameter("x0 (Mu)", 661.7);
  PhotopeakFunction->SetParameter("Sigma Gauss", 2.0);
  PhotopeakFunction->SetParameter("BoverA", 0.05);
  PhotopeakFunction->SetParameter("Gamma", 0.5);
  PhotopeakFunction->SetParameter("Sigma Ratio", 0.85);
  PhotopeakFunction->SetParameter("CoverB", 0.13);
  PhotopeakFunction->SetParameter("D (Lin Slope)", 0.028);

  // Set boundary limits to stabilize convergence
  PhotopeakFunction->SetParLimits(0, 1, 1e8);
  PhotopeakFunction->SetParLimits(1, 645, 675);     // Keeps peak centered around 662 keV
  PhotopeakFunction->SetParLimits(2, 0.5, 10);      // Prevents sigma from blowing up or hitting zero
  PhotopeakFunction->SetParLimits(3, 0.0, 1.0);     // Tail shouldn't be larger than the main peak
  PhotopeakFunction->SetParLimits(4, 0.001, 2.0);   // Standard range for exponential decay factor
  PhotopeakFunction->SetParLimits(5, 0.1, 5.0);     // Ratio of shelf width to peak width
  PhotopeakFunction->SetParLimits(6, 0.0, 5.0);     
  PhotopeakFunction->SetParLimits(7, -1.0, 1.0);
  // // Gaussian with a low-E shelf
  // TF1* PhotopeakFunction = new TF1("PhotopeakFunction", "gaus(0) + [0]*[3]*(1 - erf((x-[1])/(sqrt(2)*[2])))", 620, 680);

  // PhotopeakFunction->SetParName(0, "Gauss norm");
  // PhotopeakFunction->SetParName(1, "Mu");
  // PhotopeakFunction->SetParName(2, "Sigma");
  // PhotopeakFunction->SetParName(3, "Shelf norm");

  // PhotopeakFunction->SetParameter("Gauss norm", 1000);
  // PhotopeakFunction->SetParameter("Mu", 661.7);
  // PhotopeakFunction->SetParameter("Sigma", 2);
  // PhotopeakFunction->SetParameter("Shelf norm", 0.05);

  // PhotopeakFunction->SetParLimits(0, 10, 1e8);
  // PhotopeakFunction->SetParLimits(1, 652, 672);
  // PhotopeakFunction->SetParLimits(2, 1.0, 10);
  // PhotopeakFunction->SetParLimits(3, 0, 0.1);

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
  double MaxEnergy = -numeric_limits<double>::max(); 
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