/* 
 * DepthParameters.cxx
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
#include "MModuleLoaderMeasurementsROA.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MModuleEventFilter.h"
#include "MModuleStripPairingGreedy.h"
#include "MModuleStripPairingChiSquare.h"
#include "MModuleTACcut.h"
#include "MAssembly.h"
#include "MXmlDocument.h"
#include "MXmlNode.h"
#include "MSettings.h"

//int g_HistBins = 75;
//double g_MinCTD = -200;
//double g_MaxCTD = 200;
//double g_MinRatio = 0.94;
//double g_MaxRatio = 1.06;


////////////////////////////////////////////////////////////////////////////////
class MDataSet
{
public:
  //! Default constructor
  MDataSet() {};
  //! Default destructor
  virtual ~MDataSet() {};

  //
  void ToString() {
    cout<<"DataSet "<<m_Name<<endl;
    cout<<"  Energy: "<<m_Energy<<endl;
    cout<<"  Range: "<<m_Range<<endl;
    cout<<"  Low voltage files:"<<endl;
    for (auto F: m_LowVoltageSideFiles) {
      cout<<"    "<<F<<endl;
    }
    cout<<"  High voltage files:"<<endl;
    for (auto F: m_HighVoltageSideFiles) {
      cout<<"    "<<F<<endl;
    }
  }

  MString m_Name;
  double m_Energy;
  double m_Range;
  vector<MString> m_LowVoltageSideFiles;
  vector<MString> m_HighVoltageSideFiles;
};

/*

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
*/

////////////////////////////////////////////////////////////////////////////////



//! A standalone program based on MEGAlib and ROOT
class DepthParameters
{
public:
  //! Default constructor
  DepthParameters();
  //! Default destructor
  ~DepthParameters();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what eveer needs to be analyzed...
  bool GenerateCTD(MString m_FileName);
  bool Analyze();
  //!load cross talk correction
  //vector<vector<vector<float> > > LoadCrossTalk();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

  void dummy_func() { return; }

  //MStripHit* GetDominantStrip(vector<MStripHit*>& Strips, double& EnergyFraction);

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;
  //! The input file name
  MString m_Config;
  MString m_EcalFile;
  MString m_TACFile;
  MString m_DataPath;


  //! output file names
  MString m_OutFile;
  MString m_OutDir;
  //! energy E0
  // float m_E0;
  //! option to correct charge loss or not
  // bool m_CorrectCL;
  //! option to do a pixel-by-pixel calibration (instead of detector-by-detector)
//  bool m_PixelCorrect;
//  bool m_GreedyPairing;
  bool m_CardCageOverride;
  bool m_SinglePixel; // TO DO Implement
  bool m_ProcessAll; // TO DO Implement
  double m_SinglePixelHV; 
  double m_SinglePixelLV;
  int m_Nbins;
  int m_HighBin;
  int m_LowBin;
  int m_Nstrips;
  double m_Am241E;
  double m_NsigmasE;
  //vector<vector<string>> Am241FileNames;
  vector<vector<TH1D*>> Am241LvCtd;//(m_Nstrips, vector<TH1D*>(m_Nstrips,nullptr));
  vector<vector<TH1D*>> Am241HvCtd;//(m_Nstrips, vector<TH1D*>(m_Nstrips,nullptr));
  vector<vector<TH1D*>> Histograms;//(m_Nstrips, vector<TH1D*>(m_Nstrips, nullptr));
//  double m_MinEnergy;
//  double m_MaxEnergy;

};

////////////////////////////////////////////////////////////////////////////////

/*
double SymmetryFCN::operator()(vector<double> const &v) const
{
  double HVSlope = v[0];
  double HVIntercept = v[1];
  double LVSlope = v[2];
  double LVIntercept = v[3];

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
    double CorrectedHVEnergy = m_HVEnergy[i]/(1 - (HVSlope*CTDHVShift + HVIntercept)/100);
    double CorrectedLVEnergy = m_LVEnergy[i]/(1 - (LVSlope*CTDLVShift + LVIntercept)/100);
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

  return Asymmetry;

}
*/

////////////////////////////////////////////////////////////////////////////////

/*
double ChiSquaredFCN::operator()(vector<double> const &v) const
{
  double HVSlope = v[0];
  double HVIntercept = v[1];
  double LVSlope = v[2];
  double LVIntercept = v[3];

  double ChiSquare = 0;

  for (unsigned int i = 0; i < m_CTD.size(); ++i) {
    double CTDHVShift = m_CTD[i] + g_MaxCTD;
    double CTDLVShift = m_CTD[i] + g_MinCTD;
    // Correct the HV and LV energies by dividing by the CCE. DeltaCCE is defined as a linear function with units percentage energy lost.
    double CorrectedHVEnergy = m_HVEnergy[i]/(1 - (HVSlope*CTDHVShift + HVIntercept)/100);
    double CorrectedLVEnergy = m_LVEnergy[i]/(1 - (LVSlope*CTDLVShift + LVIntercept)/100);

    ChiSquare += pow(CorrectedHVEnergy - CorrectedLVEnergy, 2)/(m_HVEnergyResolution[i] + m_LVEnergyResolution[i]);
  }

  // ChiSquare /= m_CTD.size();

  return ChiSquare;

}
*/

//! Default constructor
DepthParameters::DepthParameters() : m_Interrupt(false)
{
  gStyle->SetPalette(1, 0);
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
DepthParameters::~DepthParameters()
{
  // Intentionally left blank
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the command line
bool DepthParameters::ParseCommandLine(int argc, char** argv)
{
  ostringstream Usage;
  Usage<<endl;
  Usage<<"  Usage: DepthParameters <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -config: configuration file (.yaml)"<<endl;
  Usage<<"           -path:path to data files; default is '.'"<<endl;
  Usage<<"           -p:   only fit one pixel (HV, LV)"<<endl;
  Usage<<"           -b:   number of bins (default = 200)"<<endl;
  Usage<<"           -a:   process all files (even if ctd list already saved)"<<endl;
//  Usage<<"         -i:   input file name (.roa)"<<endl;
//  Usage<<"         --emin:   minimum Event energy"<<endl;
//  Usage<<"         --emax:   maximum Event energy"<<endl;
  Usage<<"         -t:   TAC calibration file (otherwise use tac from yaml)"<<endl;
//  Usage<<"         -g:   greedy strip pairing (default is chi-square)"<<endl;
  Usage<<"         -c:   Card cage data (i.e. no TAC calibration required)"<<endl;
  Usage<<"         -d:   out directory (directory for ctd lists and also outfile; default '.')"<<endl;
  Usage<<"         -o:   outfile (default <config_name>_YYYYMMDDHHMMSS"<<endl;
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

  /*
  m_PixelCorrect = false;
  m_GreedyPairing = false;
  m_MinEnergy = 0;
  m_MaxEnergy = 5000;
  */
  bool outfile_in = false;
  bool taccal_in = false;
  bool ecal_in = false;
  m_Nbins = 200;
  m_HighBin = 200;
  m_LowBin = -200;
  m_ProcessAll = false;
  m_Nstrips = 64;
  m_Am241E = 59.5; // this should be in the config xml file todo
  m_NsigmasE = 3; // this should be in the config xml file todo 

  m_DataPath = ".";

  // Now parse the command line options:
  for (int i = 1; i < argc; i++) {
    Option = argv[i];

    // First check if each option has sufficient arguments:
    // Single argument
    if (Option == "-config" || Option == "-o" || Option == "-e" || Option == "-t" || Option == "-path" || Option == "-b" || Option == "-d") {
      if (!((argc > i+1) && 
            (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    } 
    if (Option == "-p" ) {
      if (!((argc > i+2) && 
            (argv[i+2][0] != '-' || isalpha(argv[i+2][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs two additional arguments!"<<endl;
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
    if (Option == "-config"){
      m_Config = argv[++i];
      cout<<"Using configuration: "<<m_Config<<endl;
    }
    if (Option == "-path"){
      m_DataPath = argv[++i];
      if (m_DataPath.EndsWith("/")) {
        m_DataPath.Remove(m_DataPath.Length() - 1, 1);
      }

    }
  /*  if (Option == "-i") {
      m_FileName = argv[++i];
      cout<<"Accepting file name: "<<m_FileName<<endl;
    } 
*/
    if (Option == "-e") {
      m_EcalFile = argv[++i];
      cout<<"Accepting file name: "<<m_EcalFile<<endl;
      ecal_in = true;
    } 
/*
    if (Option == "--emin") {
      m_MinEnergy = stod(argv[++i]);
    } 

    if (Option == "--emax") {
      m_MaxEnergy = stod(argv[++i]);
    } 
*/
    if (Option == "-t") {
      m_TACFile = argv[++i];
      cout<<"Accepting file name: "<<m_TACFile<<endl;
      taccal_in = true;
    } 

    if (Option == "-d"){
      m_OutDir = argv[++i];
      cout<<"Accepting output directory name: "<<m_OutDir<<endl;
      // todo: make directory if not exists... 
    }
    if (Option == "-o"){
      m_OutFile = argv[++i];
      cout<<"Accepting file name: "<<m_OutFile<<endl;
      outfile_in = true;
    }
    if (Option == "-b"){
      m_Nbins = stod(argv[++i]);
      cout<<"Using: "<<m_Nbins<<" bins"<<endl;
      outfile_in = true;
    }

    if (Option == "-c"){
      m_CardCageOverride = true;
    }
    if (Option == "-a"){
      m_ProcessAll = true;
    }

    if (Option == "-p"){
      m_SinglePixel = true;
      m_SinglePixelHV = stod(argv[++i]);
      m_SinglePixelLV = stod(argv[++i]);
    }
/*
    if (Option == "-g"){
      m_GreedyPairing = true;
    }
*/
  }

  if (!outfile_in){
    time_t rawtime;
    tm* timeinfo;
    time(&rawtime);
    timeinfo = gmtime(&rawtime);
    char buffer [80];
    strftime(buffer,0,"%Y%m%d%H%M%S",timeinfo);
    MString base = m_Config;
    MString suffix = ".yaml";

    if (base.EndsWith(suffix)) {
      base = base.GetSubString(0, base.Length() - suffix.Length());
    }
    m_OutFile = base + "_" + MString(buffer);
  }

  // obtain the ecal and taccal from the xml file 
  MXmlDocument* Doc = new MXmlDocument(m_Config);
  if (Doc->Load(m_Config) == false) {
    cout<<"Unable to load file"<<m_Config<<endl;
    return false;
  }
  MXmlNode* NodeA = nullptr;
  if ((NodeA = Doc->GetNode("Analysis")) != nullptr) {
    MXmlNode* aNode;
    if ((aNode = NodeA->GetNode("Ecal")) != 0 && !ecal_in) {
      m_EcalFile = aNode->GetValueAsString();
      cout << "loaded ecal filename from config: "<<m_EcalFile<<endl;
    } else {
      cout << "no ecal specified in config!"<<endl;
      return false;
    }
    if ((aNode = NodeA->GetNode("Taccal")) != 0 && !taccal_in) {
      m_TACFile = aNode->GetValueAsString();
      cout << "loaded taccal filename from config: "<<m_TACFile<<endl;
    } else {
      cout << "no taccal specified in config!"<<endl;
      return false;
    }
  } else {
    cout<<"Node Analysis not found"<<endl;
    return false;
  }


  cout << "loaded xml" << endl;

  // initialize Histograms
  Am241LvCtd.resize(m_Nstrips, vector<TH1D*>(m_Nstrips, nullptr));
  Am241HvCtd.resize(m_Nstrips, vector<TH1D*>(m_Nstrips, nullptr));
  Histograms.resize(m_Nstrips, vector<TH1D*>(m_Nstrips, nullptr));
  for (int LV = 0; LV < m_Nstrips; ++LV) {
    for (int HV = 0; HV < m_Nstrips; ++HV) {
      TString name = Form("h_%d_%d", LV, HV);
      Histograms[LV][HV] = new TH1D(name, name, m_Nbins, m_LowBin, m_HighBin);
      name = Form("hlv_%d_%d", LV, HV);
      Am241LvCtd[LV][HV] = new TH1D(name,name,m_Nbins,m_LowBin, m_HighBin);
      name = Form("hhv_%d_%d", LV, HV);
      Am241HvCtd[LV][HV] = new TH1D(name,name,m_Nbins,m_LowBin, m_HighBin);
    }
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do whatever analysis is necessary
bool DepthParameters::GenerateCTD(MString m_FileName)
{
  MString out_ctd_root = m_OutDir +  "/"+ m_FileName + "_" + to_string(m_LowBin) + "_" + to_string(m_HighBin) + "_" + to_string(m_Nbins) + ".root";
  // todo make outdir_ctd 

  // load from file if exists
  if (!m_ProcessAll && false) { // to do and file exists!! todo 
    bool loaded_all = true;
    TFile* inFile = new TFile(out_ctd_root, "READ");
    for (int i = 0; i < m_Nstrips; ++i) {
      for (int j = 0; j < m_Nstrips; ++j) {
        TString histName = Form("h_%d_%d", i, j);
        Histograms[i][j] = (TH1D*) inFile->Get(histName);
        if (!Histograms[i][j]) {
          cout << "Missing: " << histName << endl;
	  loaded_all = false;
        }
      }
    }
    if (loaded_all) return true;
  }



  //time code just to see
  TStopwatch watch;
  watch.Start();


  // initialize the vector of TH1Ds!
  for (int i = 0; i < m_Nstrips; ++i) {
    for (int j = 0; j < m_Nstrips; ++j) {
      Histograms[i][j]->Reset();// = new TH1D(name, name, m_Nbins, m_LowBin, m_HighBin);
    }
  }

  if (m_Interrupt == true) return false;

  MSupervisor* S = MSupervisor::GetSupervisor();
  
	MModuleLoaderMeasurementsROA* Loader;
	MModuleTACcut* TACCalibrator;
	MModuleEnergyCalibrationUniversal* EnergyCalibrator;
	MModuleEventFilter* EventFilter;

  unsigned int MNumber = 0;
  cout<<"Creating ROA loader"<<endl;
  vector<vector<MModuleLoaderMeasurementsROA*>> Loaders;
  Loader = new MModuleLoaderMeasurementsROA();
  // would be really nice to be able to load multiple filenames to one Loader...
  Loader->SetFileName(m_FileName);
  S->SetModule(Loader, MNumber);
  ++MNumber;

  if (m_CardCageOverride==false) {
	  cout<<"Creating TAC calibrator"<<endl;
	  TACCalibrator = new MModuleTACcut();
	  TACCalibrator->SetTACCalFileName(m_TACFile);
	  S->SetModule(TACCalibrator, MNumber);
	  ++MNumber;
  }
 
  cout<<"Creating energy calibrator"<<endl;
  EnergyCalibrator = new MModuleEnergyCalibrationUniversal();
  EnergyCalibrator->SetFileName(m_EcalFile);
  EnergyCalibrator->EnablePreampTempCorrection(false);
  S->SetModule(EnergyCalibrator, MNumber);
  ++MNumber;

  cout<<"Creating Event filter"<<endl;
  //! Only use events with 1 Strip Hit on each side to avoid strip pairing complications
  EventFilter = new MModuleEventFilter();
  EventFilter->SetMinimumLVStrips(1);
  EventFilter->SetMaximumLVStrips(1);
  EventFilter->SetMinimumHVStrips(1);
  EventFilter->SetMaximumHVStrips(1);
  //EventFilter->SetMinimumTotalEnergy(m_MinEnergy);
  //EventFilter->SetMaximumTotalEnergy(m_MaxEnergy);
  S->SetModule(EventFilter, MNumber);
  ++MNumber;

  cout<<"Initializing Loader"<<endl;
  if (Loader->Initialize() == false) return false;
  if (m_CardCageOverride==false) {
  	cout<<"Initializing TAC calibrator"<<endl;
  	if (TACCalibrator->Initialize() == false) return false;
  }
  cout<<"Initializing Energy calibrator"<<endl;
  if (EnergyCalibrator->Initialize() == false) return false;
  cout<<"Initializing Event filter"<<endl;
  if (EventFilter->Initialize() == false) return false; 
  
  //map<int, TH2D*> Histograms;
  //map<int, SymmetryFCN*> FCNs;

  bool IsFinished = false;
  MReadOutAssembly* Event = new MReadOutAssembly();

  cout<<"Generating CTDs..."<<endl;
  while (IsFinished == false && m_Interrupt == false) {
    Event->Clear();

    if (Loader->IsReady() ){
      Loader->AnalyzeEvent(Event);

      if (m_CardCageOverride==false) {
      	TACCalibrator->AnalyzeEvent(Event);
      }

      EnergyCalibrator->AnalyzeEvent(Event);
      bool Unfiltered = EventFilter->AnalyzeEvent(Event);
      if (Unfiltered==true) {
        if (true) { //for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
          double HVEnergy = 0.0;
          double LVEnergy = 0.0;
          double HVEnergyResolution = 0.0;
          double LVEnergyResolution = 0.0;
          //vector<MStripHit*> HVStrips;
          //vector<MStripHit*> LVStrips;
	  MStripHit* LVSH;
	  MStripHit* HVSH;

          //MHit* H = Event->GetHit(h);
          
          //int DetID = H->GetStripHit(0)->GetDetectorID();
          //TH2D* Hist = Histograms[DetID];
          //SymmetryFCN* FCN = FCNs[DetID]; 
          
          /*if (Hist == nullptr) {
            char name[64]; sprintf(name,"Detector %d (Uncorrected)",DetID);
            Hist = new TH2D(name, name, g_HistBins, g_MinCTD, g_MaxCTD, g_HistBins, g_MinRatio, g_MaxRatio);
            Hist->SetXTitle("CTD (ns)");
            Hist->SetYTitle("HV/LV Energy Ratio");
            Histograms[DetID] = Hist;
          }*/

      /*if (FCN == nullptr) {
        FCN = new SymmetryFCN();
        FCNs[DetID] = FCN;
      }*/

          for (unsigned int sh = 0; sh < Event->GetNStripHits(); ++sh) {
          //for (unsigned int sh = 0; sh < H->GetNStripHits(); ++sh) {
            //MStripHit* SH = H->GetStripHit(sh);
            MStripHit* SH = Event->GetStripHit(sh);

            if (SH->IsLowVoltageStrip()==true) {
              LVEnergy += SH->GetEnergy();
              LVEnergyResolution += (SH->GetEnergyResolution())*(SH->GetEnergyResolution());
              LVSH = SH;
            } else {
              HVEnergy += SH->GetEnergy();
              HVEnergyResolution += (SH->GetEnergyResolution())*(SH->GetEnergyResolution());
              HVSH = SH;
            }
          }

          //double HVEnergyFraction = 0;
          //double LVEnergyFraction = 0;
          //MStripHit* HVSH = GetDominantStrip(HVStrips, HVEnergyFraction); 
          //MStripHit* LVSH = GetDominantStrip(LVStrips, LVEnergyFraction); 
          //double EnergyFraction = HVEnergy/LVEnergy;
          if (LVEnergy < m_Am241E + m_NsigmasE*LVEnergyResolution && LVEnergy > m_Am241E - m_NsigmasE*LVEnergyResolution && HVEnergy < m_Am241E + m_NsigmasE*HVEnergyResolution && HVEnergy > m_Am241E - m_NsigmasE*HVEnergyResolution) {
	    double CTD = LVSH->GetTiming() - HVSH->GetTiming();
            if (m_CardCageOverride == true) {
              CTD *= -1;
            }
	    int LVStripID = LVSH->GetStripID();
	    int HVStripID = HVSH->GetStripID();
	  
            Histograms[LVStripID][HVStripID]->Fill(CTD);
            //FCN->AddCTD(CTD);
            //FCN->AddHVEnergy(HVEnergy, HVEnergyResolution);
            //FCN->AddLVEnergy(LVEnergy, LVEnergyResolution);
	  }
        }
      }
    }
    IsFinished = Loader->IsFinished();
  }
  // write ctd histograms to file
  TFile* outFile = new TFile(out_ctd_root, "RECREATE");
  for (int i = 0; i < m_Nstrips; ++i) {
    for (int j = 0; j < m_Nstrips; ++j) {
      if (Histograms[i][j]) {
        Histograms[i][j]->Write();  // Writes histogram to file
      }
    }
  }
  outFile->Close();
  delete outFile;
  
  watch.Stop();
  cout<<m_FileName<<" ; total time (s): "<<watch.CpuTime()<<endl;
 
  return true;
}


/////////////////////////////////////////////////////

//! Do whatever analysis is necessary
bool DepthParameters::Analyze()
{
  // read the xml file to obtain the list of datasets!
  vector<MDataSet> m_DataSets;
  MXmlDocument* Doc = new MXmlDocument(m_Config);
  if (Doc->Load(m_Config) == false) {
    cout<<"Unable to load file"<<m_Config<<endl;
    return false;
  }
  MXmlNode* DataSets = nullptr;
  if ((DataSets = Doc->GetNode("DataSets")) != nullptr) {
    for (unsigned int d = 0; d < DataSets->GetNNodes(); ++d) {
      MXmlNode* DataSet = DataSets->GetNode(d);
      if (DataSet->GetName() == "DataSet") {
        MDataSet D;
        MXmlNode* aNode;
        if ((aNode = DataSet->GetNode("Name")) != 0) {
          D.m_Name = aNode->GetValueAsString();
        }
        if ((aNode = DataSet->GetNode("Energy")) != 0) {
          D.m_Energy = aNode->GetValueAsDouble();
        }
        if ((aNode = DataSet->GetNode("Range")) != 0) {
          D.m_Range = aNode->GetValueAsDouble();
        }
        if ((aNode = DataSet->GetNode("HighVoltageFiles")) != 0) {
          for (unsigned int f = 0; f < aNode->GetNNodes(); ++f) {
            MXmlNode* File = aNode->GetNode(f);
            if (File->GetName() == "File") {
              D.m_HighVoltageSideFiles.push_back(File->GetValueAsString());
            }
          }
        }
        if ((aNode = DataSet->GetNode("LowVoltageFiles")) != 0) {
          for (unsigned int f = 0; f < aNode->GetNNodes(); ++f) {
            MXmlNode* File = aNode->GetNode(f);
            if (File->GetName() == "File") {
              D.m_LowVoltageSideFiles.push_back(File->GetValueAsString());
            }
          }
        }
        D.ToString();
        m_DataSets.push_back(D);
      }
    }
  } else {
    cout<<"Node DataSets not found"<<endl;
    return false;
  }

	
  // generate the ctds for each source and source position...
  for (long unsigned int i = 0; i < m_DataSets.size(); ++i) {
    MDataSet D = m_DataSets[i];
    for (long unsigned int j = 0; j < D.m_LowVoltageSideFiles.size(); j++){
      MString fname = D.m_HighVoltageSideFiles[j];
      if (m_DataPath != "." && !fname.Contains("/")) fname = m_DataPath + "/" + fname;
      if (GenerateCTD(fname) == false) {
        cout<<"Error during ctd generation!"<<fname<<endl;
        return false;
      }
      for (int LV = 0; LV < m_Nstrips; ++LV) {
        for (int HV = 0; HV < m_Nstrips; ++HV) {
          Am241LvCtd[i][j]->Add(Histograms[i][j]);
	}
      }  
    }
    for (long unsigned int j = 0; j < D.m_HighVoltageSideFiles.size(); j++){
      MString fname = D.m_HighVoltageSideFiles[j];
      if (m_DataPath != "." && !fname.Contains("/")) fname = m_DataPath + "/" + fname;
      if (GenerateCTD(fname) == false) {
        cout<<"Error during ctd generation!"<<fname<<endl;
        return false;
      }
      for (int LV = 0; LV < m_Nstrips; ++LV) {
        for (int HV = 0; HV < m_Nstrips; ++HV) {
          Am241HvCtd[i][j]->Add(Histograms[i][j]);
	}
      }  
    }
  } 

/*

  //setup output file
  ofstream OutputCalFile;
  OutputCalFile.open(m_OutFile+MString("_parameters.txt"));
  OutputCalFile<<"Det"<<'\t'<<"HV Slope"<<'\t'<<"HV Intercept"<<'\t'<<"LV Slope"<<'\t'<<"LV Intercept"<<endl<<endl;

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
    InitialStateSym->Add("HVSlope", 1e-3, 1e-4, 0, 3e-1);
    InitialStateSym->Add("HVIntercept", 0, 0.01, -2, 2);
    InitialStateSym->Add("LVSlope", 0, 1e-4, -3e-2, 0);
    InitialStateSym->Add("LVIntercept", 0, 0.01, -2, 2);

    InitialStateSym->Fix("LVSlope");
    InitialStateSym->Fix("LVIntercept");
    InitialStateSym->Fix("HVIntercept");


    MnMigrad migradSym(*F.second, *InitialStateSym);
     // Minimize
    FunctionMinimum MinimumSym = migradSym();

    MnUserParameters ParametersSym = MinimumSym.UserParameters();
    // double HVSlope = ParametersSym.Value("HVSlope");
    // double HVIntercept = ParametersSym.Value("HVIntercept");
    // double LVSlope = ParametersSym.Value("LVSlope");
    // double LVIntercept = ParametersSym.Value("LVIntercept");

    // output
    cout<<MinimumSym<<endl;

    MnUserParameters* InitialStateChi = new MnUserParameters();
    InitialStateChi->Add("HVSlope", ParametersSym.Value("HVSlope"), ParametersSym.Error("HVSlope"));
    InitialStateChi->Add("HVIntercept", 0, 0.01, -2, 2);
    InitialStateChi->Add("LVSlope", 0, 1e-4, -3e-2, 0);
    InitialStateChi->Add("LVIntercept", 0, 0.01, -2, 2);

    InitialStateChi->Fix("LVSlope");
    InitialStateChi->Fix("LVIntercept");
    InitialStateChi->Fix("HVSlope");


    ChiSquaredFCN* ChiSquaredF = new ChiSquaredFCN();
    ChiSquaredF->SetCTD(F.second->GetCTD());
    ChiSquaredF->SetHVEnergy(F.second->GetHVEnergy(), F.second->GetHVEnergyResolution());
    ChiSquaredF->SetLVEnergy(F.second->GetLVEnergy(), F.second->GetLVEnergyResolution());
    MnMigrad migradChi(*ChiSquaredF, *InitialStateChi);
     // Minimize
    FunctionMinimum MinimumChi = migradChi();

    MnUserParameters ParametersChi = MinimumChi.UserParameters();
    double HVSlope = ParametersChi.Value("HVSlope");
    double HVIntercept = ParametersChi.Value("HVIntercept");
    double LVSlope = ParametersChi.Value("LVSlope");
    double LVIntercept = ParametersChi.Value("LVIntercept");
    // output
    cout<<MinimumChi<<endl;

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
      double CorrectedHVEnergy = HVEnergyList[i]/(1 - (HVSlope*CTDHVShift + HVIntercept)/100);
      double CorrectedLVEnergy = LVEnergyList[i]/(1 - (LVSlope*CTDLVShift + LVIntercept)/100);
      
      Hist->Fill(CTDList[i], CorrectedHVEnergy/CorrectedLVEnergy);
    }

    TCanvas* C = new TCanvas();
    C->SetLogz();
    C->cd();
    Hist->Draw("colz");

    TFile f(m_OutFile+MString("_Det")+DetID+MString("_Hist_Corr.root"),"recreate");
    Hist->Write();
    f.Close();

    OutputCalFile<<to_string(DetID)<<'\t'<<to_string(HVSlope)<<'\t'<<to_string(HVIntercept)<<'\t'<<to_string(LVSlope)<<'\t'<<to_string(LVIntercept)<<endl<<endl;

  }
  OutputCalFile.close();
*/
  return true;
}

////////////////////////////////////////////////////////////////////////////////


DepthParameters* g_Prg = 0;
int g_NInterruptCatches = 1;
/*
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
*/

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

  TApplication DepthParameterApp("DepthParameterApp", 0, 0);

  g_Prg = new DepthParameters();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr<<"Error during parsing of command line!"<<endl;
    return -1;
  } 

  if (g_Prg->Analyze() == false) {
    cerr<<"Error during analysis generation!"<<endl;
    return -2;
  }

  DepthParameterApp.Run();

  cout<<"Program exited normally!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////
