/* 
 * TrappingCorrection.cxx
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
#include <TStopwatch.h>
#include <TProfile.h>
#include <Minuit2/FCNBase.h>
#include <Minuit2/MnMigrad.h>
#include <Minuit2/MnMinos.h>
#include <Minuit2/FunctionMinimum.h>
#include <Minuit2/MnUserParameters.h>
#include <Minuit2/MnApplication.h>
#include <Minuit2/MnStrategy.h>
using namespace ROOT::Minuit2;

// MEGAlib
#include "MGlobal.h"
#include "MFile.h"
#include "MReadOutElementDoubleStrip.h"
#include "MFileReadOuts.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MReadOutSequence.h"
#include "MSupervisor.h"
#include "MModuleLoaderMeasurements.h"
#include "MModuleLoaderMeasurementsROA.h"
#include "MModuleLoaderMeasurementsHDF.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MModuleEventFilter.h"
#include "MModuleStripPairingGreedy.h"
#include "MModuleStripPairingChiSquare.h"
#include "MModuleTACcut.h"
#include "MAssembly.h"


int g_HistBins = 75;
double g_MinCTD = -250;
double g_MaxCTD = 250;
double g_MinRatio = 0.96;
double g_MaxRatio = 1.04;


////////////////////////////////////////////////////////////////////////////////


class SymmetryFCN : public FCNBase
{
public:
  //! Operator which returns the symmetry of m_CTDHistogram given the parameters passed
  double operator()(vector<double> const &v) const override;
  double Up() const override { return 1; }

  void AddCTD(double CTD){ m_CTD.push_back(CTD); }
  void AddHVEnergy(double HVEnergy, double HVEnergyResolution){ m_HVEnergy.push_back(HVEnergy); m_HVEnergyResolution.push_back(HVEnergyResolution); }
  void AddLVEnergy(double LVEnergy, double LVEnergyResolution){ m_LVEnergy.push_back(LVEnergy); m_LVEnergyResolution.push_back(LVEnergyResolution); }

  vector<double> GetCTD(){ return m_CTD; }
  vector<double> GetHVEnergy(){ return m_HVEnergy; }
  vector<double> GetLVEnergy(){ return m_LVEnergy; }
  vector<double> GetHVEnergyResolution(){ return m_HVEnergyResolution; }
  vector<double> GetLVEnergyResolution(){ return m_LVEnergyResolution; }

  void SetCTD(vector<double> CTD){ m_CTD = CTD; }
  void SetHVEnergy(vector<double> HVEnergy, vector<double> HVEnergyResolution){ m_HVEnergy = HVEnergy; m_HVEnergyResolution = HVEnergyResolution; }
  void SetLVEnergy(vector<double> LVEnergy, vector<double> LVEnergyResolution){ m_LVEnergy = LVEnergy; m_LVEnergyResolution = LVEnergyResolution; }

  //! The measured CTD and HV/LV energies
  vector<double> m_CTD;
  vector<double> m_HVEnergy;
  vector<double> m_LVEnergy;
  vector<double> m_HVEnergyResolution;
  vector<double> m_LVEnergyResolution;

};


////////////////////////////////////////////////////////////////////////////////


class ChiSquaredFCN : public SymmetryFCN
{
public:
  //! Operator which returns the symmetry of m_CTDHistogram given the parameters passed
  double operator()(vector<double> const &v) const override;
  double Up() const override { return 1; }

};


////////////////////////////////////////////////////////////////////////////////



//! A standalone program based on MEGAlib and ROOT
class TrappingCorrection
{
public:
  //! Default constructor
  TrappingCorrection();
  //! Default destructor
  ~TrappingCorrection();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what ever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  void dummy_func() { return; }

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
  bool m_GreedyPairing;
  bool m_ExcludeNN;

  double m_MinEnergy;
  double m_MaxEnergy;

};

////////////////////////////////////////////////////////////////////////////////


double SymmetryFCN::operator()(vector<double> const &v) const
{
  double HVSlope = v[0];
  double LVSlope = v[1];

  char name[64]; sprintf(name,"name");
  int HistBins = g_HistBins;
  if (HistBins%2 == 0) {
    HistBins += 1;
  }
  TH2D CorrectedHistogram(name, name, HistBins, g_MinCTD, g_MaxCTD, HistBins, g_MinRatio, g_MaxRatio);

  for (unsigned int i = 0; i < m_CTD.size(); ++i) {
    double CTDHVShift = m_CTD[i] + g_MaxCTD;
    double CTDLVShift = m_CTD[i] + g_MinCTD;
    // Correct the HV and LV energies by dividing by the CCE. DeltaCCE is defined as a linear function with units percentage energy lost.
    double CorrectedHVEnergy = m_HVEnergy[i]/(1 - (HVSlope*CTDHVShift)/100);
    double CorrectedLVEnergy = m_LVEnergy[i]/(1 - (LVSlope*CTDLVShift)/100);
    CorrectedHistogram.Fill(m_CTD[i], CorrectedHVEnergy/CorrectedLVEnergy);
  }

  vector<vector<double>> BinValues;
  vector<vector<double>> ReflectedBinValues;
  double Asymmetry = 0;

  for (unsigned int y = 0; y < CorrectedHistogram.GetNbinsY(); ++y) {
    
    vector<double> XValues;

    for (unsigned int x = 0; x < CorrectedHistogram.GetNbinsX(); ++x) {
      XValues.push_back(CorrectedHistogram.GetBinContent(x,y));
    }

    // BinValues.push_back(XValues);
    vector<double> ReflectedXValues = XValues;
    reverse(ReflectedXValues.begin(), ReflectedXValues.end());

    for (unsigned int x = 0; x < XValues.size(); ++x) {
      Asymmetry += pow(XValues[x] - ReflectedXValues[x], 2)/(XValues[x] + ReflectedXValues[x]);
    }
  }

  return Asymmetry*Asymmetry;

}


////////////////////////////////////////////////////////////////////////////////


double ChiSquaredFCN::operator()(vector<double> const &v) const
{
  double HVLVFactor = v[0];
  double HVSlope = v[1];
  double LVSlope = v[2];

  double ChiSquare = 0;

  for (unsigned int i = 0; i < m_CTD.size(); ++i) {
    double CTDHVShift = m_CTD[i] + g_MaxCTD;
    double CTDLVShift = m_CTD[i] + g_MinCTD;
    // Correct the HV and LV energies by dividing by the CCE. DeltaCCE is defined as a linear function with units percentage energy lost.
    double CorrectedHVEnergy = m_HVEnergy[i]/((1 - (HVSlope*CTDHVShift)/100)*HVLVFactor);
    double CorrectedLVEnergy = m_LVEnergy[i]/(1 - (LVSlope*CTDLVShift)/100);

    ChiSquare += pow(CorrectedHVEnergy - CorrectedLVEnergy, 2)/(m_HVEnergyResolution[i] + m_LVEnergyResolution[i]);
  }

  // ChiSquare /= m_CTD.size();

  return ChiSquare;

}


//! Default constructor
TrappingCorrection::TrappingCorrection() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
TrappingCorrection::~TrappingCorrection()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool TrappingCorrection::ParseCommandLine(int argc, char** argv)
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
  m_GreedyPairing = false;
  m_ExcludeNN = false;
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
      m_GreedyPairing = true;
    }

    if (Option == "-n"){
      m_ExcludeNN = true;
    }

  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool TrappingCorrection::Analyze()
{
  //time code just to see
  TStopwatch watch;
  watch.Start();

  if (m_Interrupt == true) return false;

  vector<MString> FileNames;

  if ((m_FileName.GetSubString(m_FileName.Length() - 4)) == "hdf5") {
    FileNames.push_back(m_FileName);
  } else if ((m_FileName.GetSubString(m_FileName.Length() - 3)) == "txt") {
    MFile F;
    if (F.Open(m_FileName)==false) {
      cout<<"Error: Failed to open input file."<<endl;
    } else {
      MString Line;
      while (F.ReadLine(Line)) {
        FileNames.push_back(Line.Trim());
      }
    }
  }

  map<unsigned int, TH2D*> Histograms;
  map<unsigned int, SymmetryFCN*> FCNs;

  for (unsigned int f = 0; f<FileNames.size(); ++f) {

    MString InFile = FileNames[f];



    cout<<"Beginning analysis of file "<<InFile<<endl;

    MSupervisor* S = MSupervisor::GetSupervisor();
    
  	MModuleLoaderMeasurementsHDF* Loader;
  	MModuleTACcut* TACCalibrator;
  	MModuleEnergyCalibrationUniversal* EnergyCalibrator;
  	MModuleEventFilter* EventFilter;

    unsigned int MNumber = 0;

    if ((InFile.GetSubString(InFile.Length() - 4)) == "hdf5") {
      cout<<"Creating HDF5 loader"<<endl;
    } else {
      cout<<"Input file must be hdf5 format. Got: "<<InFile<<". Exiting."<<endl;
      return false;
    }
    Loader = new MModuleLoaderMeasurementsHDF();
    Loader->SetFileNameStripMap(m_StripMapFile);
    Loader->SetFileName(InFile);
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
    //! Only use events with 1 to 3 Strip Hits on each side to avoid strip pairing complications
    EventFilter = new MModuleEventFilter();
    EventFilter->SetMinimumLVStrips(1);
    EventFilter->SetMaximumLVStrips(3);
    EventFilter->SetMinimumHVStrips(1);
    EventFilter->SetMaximumHVStrips(3);
    EventFilter->SetMinimumHits(0);
    EventFilter->SetMaximumHits(100);
    EventFilter->SetMinimumTotalEnergy(m_MinEnergy);
    EventFilter->SetMaximumTotalEnergy(m_MaxEnergy);
    S->SetModule(EventFilter, MNumber);
    ++MNumber;
    
    cout<<"Creating strip pairing"<<endl;
    MModule* Pairing;
    if (m_GreedyPairing == true){
      Pairing = new MModuleStripPairingGreedy();
    }
    else {
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

    cout<<"Analyzing..."<<endl;
    while ((IsFinished == false) && (m_Interrupt == false)) {
      Event->Clear();

      if (Loader->IsReady()){

        Loader->AnalyzeEvent(Event);
        TACCalibrator->AnalyzeEvent(Event);
        EnergyCalibrator->AnalyzeEvent(Event);
        bool Unfiltered = EventFilter->AnalyzeEvent(Event);

        if (Unfiltered == true) {
          
          Pairing->AnalyzeEvent(Event);

          if (Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true) {
            for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
              double HVEnergy = 0.0;
              double LVEnergy = 0.0;
              double HVEnergyResolution = 0.0;
              double LVEnergyResolution = 0.0;
              vector<MStripHit*> HVStrips;
              vector<MStripHit*> LVStrips;

              MHit* H = Event->GetHit(h);
              
              int DetID = H->GetStripHit(0)->GetDetectorID();
              TH2D* Hist = Histograms[DetID];
              SymmetryFCN* FCN = FCNs[DetID]; 
              
              if (Hist == nullptr) {
                char name[64]; sprintf(name,"Detector %d (Uncorrected)",DetID);
                Hist = new TH2D(name, name, g_HistBins, g_MinCTD, g_MaxCTD, g_HistBins, g_MinRatio, g_MaxRatio);
                Hist->SetXTitle("CTD (ns)");
                Hist->SetYTitle("HV/LV Energy Ratio");
                Histograms[DetID] = Hist;
              }

              if (FCN == nullptr) {
                FCN = new SymmetryFCN();
                FCNs[DetID] = FCN;
              }

              for (unsigned int sh = 0; sh < H->GetNStripHits(); ++sh) {
                MStripHit* SH = H->GetStripHit(sh);

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

              if ((HVStrips.size()>0) && (LVStrips.size()>0)) {
                double HVEnergyFraction = 0;
                double LVEnergyFraction = 0;
                MStripHit* HVSH = GetDominantStrip(HVStrips, HVEnergyFraction); 
                MStripHit* LVSH = GetDominantStrip(LVStrips, LVEnergyFraction); 
                double EnergyFraction = HVEnergy/LVEnergy;
                if ((LVSH->HasCalibratedTiming()==true) && (HVSH->HasCalibratedTiming()==true)) {
                  double CTD = LVSH->GetTiming() - HVSH->GetTiming();
                  Hist->Fill(CTD, EnergyFraction);
                  FCN->AddCTD(CTD);
                  FCN->AddHVEnergy(HVEnergy, HVEnergyResolution);
                  FCN->AddLVEnergy(LVEnergy, LVEnergyResolution);
                }
              }
            }
          }
        }
      }
      IsFinished = Loader->IsFinished();
    }
  }

  //setup output file
  ofstream OutputCalFile;
  OutputCalFile.open(m_OutFile+MString("_parameters.txt"));
  OutputCalFile<<"Det"<<'\t'<<"HV Slope"<<'\t'<<"LV Slope"<<'\t'<<"HV/LV Factor"<<endl<<endl;

  for (auto H: Histograms) {
    
    int DetID = H.first;
    TFile f(m_OutFile+MString("_Det")+DetID+MString("_Hist_Uncorr.root"),"recreate");

    TCanvas* C = new TCanvas();
    C->SetLogz();
    C->cd();
    H.second->Draw("colz");

    H.second->Write();
    f.Close();

  }

  for (auto F: FCNs) {

    int DetID = F.first;
    MnUserParameters* InitialStateSym = new MnUserParameters();
    InitialStateSym->Add("HVSlope", 2e-3, 1e-4, 0, 3e-1);
    InitialStateSym->Add("LVSlope", 0, 1e-4, -3e-2, 0);

    InitialStateSym->Fix("LVSlope");

    MnMigrad migradSym(*F.second, *InitialStateSym);
     // Minimize
    FunctionMinimum MinimumSym = migradSym();

    MnUserParameters ParametersSym = MinimumSym.UserParameters();
    double HVSlope = ParametersSym.Value("HVSlope");
    double LVSlope = ParametersSym.Value("LVSlope");

    // output
    cout<<MinimumSym<<endl;

    // MnUserParameters* InitialStateChi = new MnUserParameters();
    // InitialStateChi->Add("HVLVFactor", 1.0, 0.01, 0.95, 1.05);
    // InitialStateChi->Add("HVSlope", ParametersSym.Value("HVSlope"), ParametersSym.Error("HVSlope"));
    // InitialStateChi->Add("LVSlope", ParametersSym.Value("LVSlope"), ParametersSym.Error("LVSlope"));

    // InitialStateChi->Fix("HVSlope");
    // InitialStateChi->Fix("LVSlope");


    // ChiSquaredFCN* ChiSquaredF = new ChiSquaredFCN();
    // ChiSquaredF->SetCTD(F.second->GetCTD());
    // ChiSquaredF->SetHVEnergy(F.second->GetHVEnergy(), F.second->GetHVEnergyResolution());
    // ChiSquaredF->SetLVEnergy(F.second->GetLVEnergy(), F.second->GetLVEnergyResolution());
    // MnMigrad migradChi(*ChiSquaredF, *InitialStateChi);
    //  // Minimize
    // FunctionMinimum MinimumChi = migradChi();

    // MnUserParameters ParametersChi = MinimumChi.UserParameters();
    // double HVLVFactor = ParametersChi.Value("HVLVFactor");
    double HVLVFactor = 1;
    // // output
    // cout<<MinimumChi<<endl;

    char name[64]; sprintf(name,"Detector %d (Corrected)",DetID);
    TH2D* Hist = new TH2D(name, name, g_HistBins, g_MinCTD, g_MaxCTD, g_HistBins, g_MinRatio, g_MaxRatio);
    Hist->SetXTitle("CTD (ns)");
    Hist->SetYTitle("HV/LV Energy Ratio");

    vector<double> CTDList = F.second->GetCTD();
    vector<double> HVEnergyList = F.second->GetHVEnergy();
    vector<double> LVEnergyList = F.second->GetLVEnergy();
    for (unsigned int i=0; i<CTDList.size(); ++i) {
      
      double CTDHVShift = CTDList[i] + g_MaxCTD;
      double CTDLVShift = CTDList[i] + g_MinCTD;
      // Correct the HV and LV energies by dividing by the CCE. DeltaCCE is defined as a linear function with units percentage energy lost.
      double CorrectedHVEnergy = HVEnergyList[i]/((1 - (HVSlope*CTDHVShift)/100)*HVLVFactor);
      double CorrectedLVEnergy = LVEnergyList[i]/(1 - (LVSlope*CTDLVShift)/100);
      
      Hist->Fill(CTDList[i], CorrectedHVEnergy/CorrectedLVEnergy);
    }

    TCanvas* C = new TCanvas();
    C->SetLogz();
    C->cd();
    Hist->Draw("colz");

    TFile f(m_OutFile+MString("_Det")+DetID+MString("_Hist_Corr.root"),"recreate");
    Hist->Write();
    f.Close();

    OutputCalFile<<to_string(DetID)<<'\t'<<to_string(HVSlope)<<'\t'<<to_string(LVSlope)<<'\t'<<to_string(HVLVFactor)<<endl<<endl;

  }
  OutputCalFile.close();

  watch.Stop();
  cout<<"total time (s): "<<watch.CpuTime()<<endl;
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


TrappingCorrection* g_Prg = 0;
int g_NInterruptCatches = 1;

MStripHit* TrappingCorrection::GetDominantStrip(vector<MStripHit*>& Strips, double& EnergyFraction)
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

  g_Prg = new TrappingCorrection();

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
