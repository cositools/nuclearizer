/* 
 * TrappingCorrectionCs137.cxx
 *
 *
 * Copyright (C) by Sophie Haight.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Sophie Haight.
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
#include <set>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <cmath>
#include <chrono>
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
#include <TF2.h>
#include <TF1Convolution.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TGraph2D.h>
#include <TFitResultPtr.h>
#include <TFitResult.h>
#include <TStopwatch.h>
#include <TProfile.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TError.h>
#include <TFile.h>
#include <TTree.h>
#include <TParameter.h>

// ROOT Fitting Engines
#include "Fit/Fitter.h"
#include "Fit/BinData.h"
#include "Fit/Chi2FCN.h"
#include "Math/WrappedMultiTF1.h"
#include "Math/Functor.h"

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
#include "MModuleTACCalibration.h"
#include "MAssembly.h"
#include "MModuleDepthCalibration.h"
#include "MDGeometryQuest.h"


int g_MinCounts = 1000;
int g_HVStrips = 64;
int g_LVStrips = 64;

double g_CsPhotopeak = 661.7;

// Number of depth bins at each detector face excluded from the joint fit (trim_index in depth_helpers.fit_CCE)
int g_TrimBins = 1;

// Simulation Template Path for Joint Trapping Fitting
MString g_SimulatedCCEPath = "sim_cce_template.csv";

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
  //! Return the verbosity of this app's own output
  int GetVerbosity() const { return m_Verbosity; }

  // Structs for CCE simulations & experimental points
  struct SimulatedCCE {
    vector<double> DepthMM;
    vector<double> HVElectronCCE;
    vector<double> HVHoleCCE;
    vector<double> LVElectronCCE;
    vector<double> LVHoleCCE;
  };

  struct ExperimentalPoint {
    int DetID;
    int DepthBin;
    double DepthMM; // mean calibrated depth of the events in this bin
    double HVCCE;
    double HVCCEErr;
    double LVCCE;
    double LVCCEErr;
  };

  // Helper method to look up the depth bin index for a given detector and depth (cm)
  int GetDepthBin(int DetID, double DepthCM) const;

  // Photopeak fitting function
  TF1* GeneratePhotopeakFunction();

  // Load Sim CCE files
  // Lines after "### <detector_ID>" belong to that detector
  // Lines before any such header to detector 0
  map<int, SimulatedCCE> LoadSimulatedCCE(const string& SimCSVPath, bool& HasDetectorHeaders);

  // Fit experimental CCE to simulated CCE to get trapping parameters
  // Returns false if no fit result could be obtained
  bool FitTrappingParameters(const vector<ExperimentalPoint>& ExpData,
                             const SimulatedCCE& Sim,
                             const string& OutputCSVPath,
                             const string& TrappingParamPath,
                             vector<double>& TrappingParams,
                             int DetID = 0);

  // Plot the fitted HV and LV photopeaks of one depth bin and save them as png in the plots directory
  void PlotPhotopeakFits(TH1D* HVHist, TF1* HVFunction, TH1D* LVHist, TF1* LVFunction, int DetID, int DepthBin);

  // Plot the experimental CCEs against the simulated and fitted CCE curves and save them as png in the plots directory
  void PlotCCEFit(const vector<ExperimentalPoint>& ExpData,
                  const SimulatedCCE& Sim,
                  const vector<double>& TrappingParams,
                  int DetID);

  // Output trapping parameters and simulated CCE into csv file
  void SaveTrappingParamFile(const string& OutputFilePath,
                             const string& TrappingParamFilePath,
                             const vector<double>& TrappingParams,
                             const SimulatedCCE& Sim,
                             int DetID);

  // Write one "### <detector_ID>" block in the format MModuleTrappingCorrection reads
  void WriteTrappingParamBlock(ostream& Out,
                               const vector<double>& TrappingParams,
                               const SimulatedCCE& Sim,
                               int DetID);

private:
  // One calibrated hit: everything the depth binning and the fits need, so they can be redone without recalibrating
  struct CalibratedHit {
    int DetID;
    float DepthCM;
    float HVEnergy;
    float LVEnergy;
  };

  // Run the calibration chain over the input files and store the calibrated hits and depth ranges
  bool CalibrateHits();
  // Save the calibrated hits, the depth ranges and the energy window to a ROOT file
  bool SaveCalibratedHits(const MString& FileName);
  // Load the calibrated hits, the depth ranges and the energy window from a ROOT file written by SaveCalibratedHits
  bool LoadHits(const MString& FileName);

  // Create the directory DirName next to the output file and set Prefix to the directory + output file name
  bool CreateOutputDirectory(const string& DirName, MString& Prefix);

  // For CCE plotting
  // True if a CCE error passes the error cut of the joint fit: err < 6 keV
  bool IsValidCCEErr(double CCEErr) const { return CCEErr > 0.0 && CCEErr <= 6.0 / g_CsPhotopeak; }
  // True if the depth bin is excluded from the joint fit by the trim at the detector edges
  bool IsTrimmedBin(int DepthBin) const { return DepthBin < g_TrimBins || DepthBin >= m_NDepthBins - g_TrimBins; }
  // CCE model: simulated electron and hole CCEs scaled by the trapping parameters A, B and C
  static double TrappedCCE(double A, double B, double C, double ElectronCCE, double HoleCCE) { return A * (1.0 - B * (1.0 - ElectronCCE)) * (1.0 - C * (1.0 - HoleCCE)); }

  // Set up evenly spaced depth bins per detector from the depth ranges of the depth calibration
  void InitializeDepthBins();

  // Depth bins
  int m_NDepthBins;
  map<int, vector<double>> m_DepthBinEdges; // m_NDepthBins + 1 edges in cm stored for each detectorID

  // Calibrated hits and the depth range (cm) of the depth calibration's depth grid of each detector
  vector<CalibratedHit> m_Hits;
  map<int, pair<double, double>> m_DepthRanges; // [DetID] -> (min depth, max depth)
  MString m_SaveCalibratedHitsFile; // ROOT file to save calibrated hits to
  MString m_LoadHitsFile; // ROOT file for loading calibrated hits 
  // Calibration files
  bool m_Interrupt;
  MString m_FileName;
  MString m_EcalFile;
  MString m_TACCalFile;
  MString m_StripMapFile;
  MString m_OutFile;
  bool m_SaveProducts; // True if the intermediate products (everything but the final trapping parameter file) are saved
  MString m_ProductsPrefix; // Prefix (directory + file name) for all intermediate products
  bool m_SavePlots; // True if the photopeak and CCE fit plots are saved
  MString m_PlotsPrefix; // Prefix (directory + file name) for all plots
  MString m_GeometryFile;
  MString m_DepthCoeffsFile;
  MString m_DepthSplinesFile;

  bool m_PixelCorrect;
  bool m_MultiRoundStripPairing;
  bool m_ExcludeNN = true;
  bool m_ContinueHDF5;

  double m_MinEnergy;
  double m_MaxEnergy;

  // Verbosity of this app's own output, independent of the global verbosity used by the nuclearizer modules
  int m_Verbosity;
};
////////////////////////////////////////////////////////////////////////////////

//! Constructor
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
  Usage<<"  Usage: TrappingCorrectionCs137 <options>"<<endl;
  Usage<<"    General options:"<<endl;
  Usage<<"         -i:   input file name (.hdf5 or .txt with list of hdf5s)"<<endl;
  Usage<<"         --emin:   minimum Event energy (default 600 keV)"<<endl;
  Usage<<"         --emax:   maximum Event energy (default 700 kev)"<<endl;
  Usage<<"         -e:   energy calibration file (.ecal)"<<endl;
  Usage<<"         --tcal:   TAC calibration file"<<endl;
  Usage<<"         -p:   do pixel-by-pixel correction"<<endl;
  Usage<<"         -m:   strip map file name (.map)"<<endl;
  Usage<<"         -g:   greedy strip pairing (default is chi-square)"<<endl;
  Usage<<"         -n:   exclude nearest neighbors"<<endl;
  Usage<<"         --geo:   geometry file (.geo.setup)"<<endl;
  Usage<<"         --coeffs:   depth calibration coefficients file (stretch/offset .csv)"<<endl;
  Usage<<"         --splines:   depth calibration CTD->depth splines file (.csv)"<<endl;
  Usage<<"         --sim :   simulated CCE file (.csv)"<<endl;
  Usage<<"         --trim :   number of depth bins at each face excluded from the trapping fit (default 1)"<<endl;
  Usage<<"         --bins :   number of depth bins (default 20)"<<endl;
  Usage<<"         -v --verbosity:   app output 0: Quiet, 1: Errors, 2: Warnings, 3: Info incl. depth bin fits and module setup (default 1)"<<endl;
  Usage<<"         --products:   save the intermediate products (fit logs, CCE tables, per detector csv files) in the directory trapping_param_products"<<endl;
  Usage<<"         --plots:   save the photopeak fit and CCE fit plots (png) in the directory trapping_param_plots"<<endl;
  Usage<<"         --save-hits:   save the calibrated hits to this ROOT file to redo the binning and fits later without recalibrating"<<endl;
  Usage<<"         --load-hits:   load the calibrated hits from this ROOT file instead of calibrating the data"<<endl;
  Usage<<"                        (Note: -i, -e, --tcal, -m, --geo, --coeffs and --splines are then not needed)"<<endl;
  Usage<<"         -o:   outfile (default YYYYMMDDHHMMSS)"<<endl;
  Usage<<"         -h:   print this help"<<endl;
  Usage<<endl;

  string Option;

  // Check for help and read the verbosity first
  m_Verbosity = c_Error;
  for (int i = 1; i < argc; i++) {
    Option = argv[i];
    if (Option == "-h" || Option == "--help" || Option == "?" || Option == "-?") {
      cout<<Usage.str()<<endl;
      return false;
    }
    if ((Option == "-v" || Option == "--verbosity") && i+1 < argc) {
      m_Verbosity = atoi(argv[i+1]);
    }
  }

  m_PixelCorrect = false;
  m_MultiRoundStripPairing = false; 
  m_ContinueHDF5 = true;
  m_MinEnergy = 600;
  m_MaxEnergy = 700;
  m_NDepthBins = 20;
  m_SaveProducts = false;
  m_SavePlots = false;

  // Progress tracking
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
    if ((Option == "-i") || (Option == "-o") || (Option == "--emin") || (Option == "--emax") || (Option == "--tcal") || (Option == "-m") || (Option == "--geo") || (Option == "--coeffs") || (Option == "--splines") || (Option == "--sim") || (Option == "--trim") || (Option == "--bins") || (Option == "-v") || (Option == "--verbosity") || (Option == "--save-hits") || (Option == "--load-hits")) {
      if (((argc > i+1) && (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0)) == false) {
        if (m_Verbosity >= c_Error) {
          cout<<"[Error] Option "<<argv[i][1]<<" needs a second argument!"<<endl;
          cout<<Usage.str()<<endl;
        }
        return false;
      }
    }

    /// Then fulfill the options:
    if (Option == "-i") {
      m_FileName = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting file name: "<<m_FileName<<endl;
    }

    if (Option == "-e") {
      m_EcalFile = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting file name: "<<m_EcalFile<<endl;
    }

    if (Option == "--emin") {
      m_MinEnergy = stod(argv[++i]);
    }

    if (Option == "--emax") {
      m_MaxEnergy = stod(argv[++i]);
    }

    if (Option == "--tcal") {
      m_TACCalFile = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting file name: "<<m_TACCalFile<<endl;
    }

    if (Option == "-o"){
      m_OutFile = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting file name: "<<m_OutFile<<endl;
    }

    if (Option == "-m"){
      m_StripMapFile = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting file name: "<<m_StripMapFile<<endl;
    }

    if (Option == "--geo") {
      m_GeometryFile = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting geometry file: "<<m_GeometryFile<<endl;
    }

    if (Option == "--coeffs") {
      m_DepthCoeffsFile = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting depth coefficients file: "<<m_DepthCoeffsFile<<endl;
    }

    if (Option == "--splines") {
      m_DepthSplinesFile = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting depth splines file: "<<m_DepthSplinesFile<<endl;
    }

    if (Option == "--trim") {
      g_TrimBins = stoi(argv[++i]);
      if (m_Verbosity >= c_Info) cout<<"Accepting number of trimmed depth bins per face: "<<g_TrimBins<<endl;
    }

    if (Option == "--bins") {
      m_NDepthBins = stoi(argv[++i]);
      if (m_Verbosity >= c_Info) cout<<"Accepting number of depth bins: "<<m_NDepthBins<<endl;
    }

    if (Option == "-v" || Option == "--verbosity") {
      ++i; // Already read above
      if (m_Verbosity >= c_Info) cout<<"Accepting verbosity: "<<m_Verbosity<<endl;
    }

    if (Option == "--save-hits") {
      m_SaveCalibratedHitsFile = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting file to save the calibrated hits: "<<m_SaveCalibratedHitsFile<<endl;
    }

    if (Option == "--load-hits") {
      m_LoadHitsFile = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting file to load the calibrated hits: "<<m_LoadHitsFile<<endl;
    }

    if (Option == "--sim") {
      g_SimulatedCCEPath = argv[++i];
      if (m_Verbosity >= c_Info) cout<<"Accepting sim CCE file: "<<g_SimulatedCCEPath<<endl;
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
    if (Option == "--products") {
      m_SaveProducts = true;
    }
    if (Option == "--plots") {
      m_SavePlots = true;
    }
    if (Option == "--nocontinue") {
      m_ContinueHDF5 = false;
    }

  }

  // The depth calibration files are only needed if the data is calibrated
  if (m_LoadHitsFile.IsEmpty() == true && (m_GeometryFile.IsEmpty() == true || m_DepthCoeffsFile.IsEmpty() == true || m_DepthSplinesFile.IsEmpty() == true)) {
    if (m_Verbosity >= c_Error) {
      cout<<"[Error] The depth calibration needs a geometry (--geo), coefficients (--coeffs) and splines (--splines) file!"<<endl;
      cout<<Usage.str()<<endl;
    }
    return false;
  }

  // Check that you don't trim too many depth bins
  if (m_NDepthBins <= 2*g_TrimBins) {
    if (m_Verbosity >= c_Error) {
      cout<<"[Error] The number of depth bins (--bins) must be larger than twice the number of trimmed bins per face (--trim)!"<<endl;
      cout<<Usage.str()<<endl;
    }
    return false;
  }

  // Set global verbosity to 0 s.t to supressed output from the nuclearizer modules 
  g_Verbosity = 0;

  // Wrap ROOT error output for canvases and plots in local/inputted verbosity settings
  if (m_Verbosity >= c_Info) {
    gErrorIgnoreLevel = kInfo;
  } else if (m_Verbosity >= c_Warning) {
    gErrorIgnoreLevel = kWarning;
  } else if (m_Verbosity >= c_Error) {
    gErrorIgnoreLevel = kError;
  } else {
    gErrorIgnoreLevel = kFatal;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////

//! Calibrate Cs-137 data and fit for trapping parameters
bool TrappingCorrectionCs137::Analyze()
{
  // time code just to see
  TStopwatch Watch;
  Watch.Start();

  // The start and the elapsed time are shown at the default verbosity (only suppressed at verbosity 0)
  time_t StartTime = time(nullptr);
  char StartTimeString[80];
  strftime(StartTimeString, 80, "%Y-%m-%d %H:%M:%S", localtime(&StartTime));
  if (m_Verbosity >= c_Error) cout << "TrappingCorrectionCs137 started at " << StartTimeString << endl;

  if (m_Interrupt == true) return false;

  // All intermediate products go into a "trapping_param_products" directory and all plots into a "trapping_param_plots" directory
  if (m_SaveProducts == true) {
    if (CreateOutputDirectory("trapping_param_products", m_ProductsPrefix) == false) return false;
  }
  if (m_SavePlots == true) {
    if (CreateOutputDirectory("trapping_param_plots", m_PlotsPrefix) == false) return false;
  }

  // [DepthBin][DetID] -> [HV CCE, HV CCE Err, LV CCE, LV CCE Err, Mean Depth (mm)] for all detectors
  map<int, map<int, vector<double>>> FullDetEndpoints;

  // Either call CalibrateHits to calibrate the data or read in calibrated hits file, depending on user inputs
  if (m_LoadHitsFile.IsEmpty() == true) {
    if (CalibrateHits() == false) return false;
  } else {
    if (LoadHits(m_LoadHitsFile) == false) return false;
  }
  if (m_SaveCalibratedHitsFile.IsEmpty() == false) {
    if (SaveCalibratedHits(m_SaveCalibratedHitsFile) == false) return false;
  }

  // Enable sum of weights squared for TH1 by default
  TH1::SetDefaultSumw2();

  // Detector-level maps organized by [DepthBin][DetID]
  map<int, map<int, TH1D*>> FullDetHVEnergyHistograms;
  map<int, map<int, TH1D*>> FullDetLVEnergyHistograms;

  // Sum and count of calibrated depths (cm) per [DepthBin][DetID] to evaluate the mean depth of each bin
  map<int, map<int, double>> DepthSums;
  map<int, map<int, long>> DepthCounts;

  // Sort the calibrated hits into the depth bins
  InitializeDepthBins();
  for (const CalibratedHit& Hit : m_Hits) {
    int DetID = Hit.DetID;
    int DepthBin = GetDepthBin(DetID, Hit.DepthCM);
    if (DepthBin < 0) continue;

    TH1D* FullHVHist  = FullDetHVEnergyHistograms[DepthBin][DetID];
    TH1D* FullLVHist  = FullDetLVEnergyHistograms[DepthBin][DetID];

    if (FullHVHist == nullptr) {
      char Name[128]; sprintf(Name, "HV_Detector%d_bin%d", DetID, DepthBin);
      FullHVHist = new TH1D(Name, Name, (m_MaxEnergy - m_MinEnergy) * 2, m_MinEnergy, m_MaxEnergy);
      FullDetHVEnergyHistograms[DepthBin][DetID] = FullHVHist;
    }
    if (FullLVHist == nullptr) {
      char Name[128]; sprintf(Name, "LV_Detector%d_bin%d", DetID, DepthBin);
      FullLVHist = new TH1D(Name, Name, (m_MaxEnergy - m_MinEnergy) * 2, m_MinEnergy, m_MaxEnergy);
      FullDetLVEnergyHistograms[DepthBin][DetID] = FullLVHist;
    }

    FullHVHist->Fill(Hit.HVEnergy);
    FullLVHist->Fill(Hit.LVEnergy);
    DepthSums[DepthBin][DetID] += Hit.DepthCM;
    DepthCounts[DepthBin][DetID] += 1;
  }


  // Store depth binned fit results in an output file
  ofstream MasterFitFile;
  if (m_SaveProducts == true) {
    MasterFitFile.open(m_ProductsPrefix + MString("_All_DepthBin_FitResults.txt"));
    MasterFitFile << "======================================================================" << endl;
    MasterFitFile << "MASTER PHOTOPEAK FIT LOG FOR ALL DEPTH BINS AND DETECTORS" << endl;
    MasterFitFile << "======================================================================" << endl << endl;
  }

  // Vector to hold experimental CCE points for joint fit
  vector<ExperimentalPoint> ExpPoints;

  // Do photopeak fitting and recording for full detector outputs
  for (int c = 0; c < m_NDepthBins; ++c) {

    if (m_Verbosity >= c_Info) cout << "Processing depth bin " << c << endl;

    for (auto const& [DetID, FullHVHist] : FullDetHVEnergyHistograms[c]) {

      TH1D* HVHist = FullDetHVEnergyHistograms[c][DetID];
      TH1D* LVHist = FullDetLVEnergyHistograms[c][DetID];

      if (HVHist->Integral() > g_MinCounts) {

        const char* FitOptions = (m_Verbosity >= c_Info) ? "S" : "SQ";

        TF1* PhotopeakFunctionHV = GeneratePhotopeakFunction();
        TFitResultPtr HVFit = HVHist->Fit(PhotopeakFunctionHV, FitOptions, "", 645, 675);

        TF1* PhotopeakFunctionLV = GeneratePhotopeakFunction();
        TFitResultPtr LVFit = LVHist->Fit(PhotopeakFunctionLV, FitOptions, "", 645, 675);

        if ((HVFit >= 0) && (LVFit >= 0)) {

          // Clear or initialize the vector for this specific bin and detector
          FullDetEndpoints[c][DetID].clear();

          double HVCentroid = HVFit->Parameter(1);
          double HVError    = HVFit->ParError(1);
          double LVCentroid = LVFit->Parameter(1);
          double LVError    = LVFit->ParError(1);

          // Convert Centroids (keV) to CCE (dimensionless ratio relative to 661.7 keV)
          double HVCCE    = HVCentroid / g_CsPhotopeak;
          double HVCCEErr = HVError / g_CsPhotopeak;
          double LVCCE    = LVCentroid / g_CsPhotopeak;
          double LVCCEErr = LVError / g_CsPhotopeak;

          FullDetEndpoints[c][DetID].push_back(HVCCE);     // HV CCE
          FullDetEndpoints[c][DetID].push_back(HVCCEErr);  // HV CCE Err
          FullDetEndpoints[c][DetID].push_back(LVCCE);     // LV CCE
          FullDetEndpoints[c][DetID].push_back(LVCCEErr);  // LV CCE Err

          // Evaluate the model at the mean calibrated depth of the events in this bin instead of the bin midpoint
          double MeanDepthMM = 10.0 * DepthSums[c][DetID] / DepthCounts[c][DetID]; // Multiply to convert cm to mm
          FullDetEndpoints[c][DetID].push_back(MeanDepthMM); // Mean depth (mm)

          ExpPoints.push_back({DetID, c, MeanDepthMM, HVCCE, HVCCEErr, LVCCE, LVCCEErr});

          // Fit log of this depth bin
          if (m_SaveProducts == true) {
            MasterFitFile << "------------------------------------------------------------" << endl;
            MasterFitFile << " DETECTOR ID: " << DetID << "  |  DEPTH BIN INDEX: " << c << "  |  MEAN DEPTH (mm): " << MeanDepthMM << endl;
            MasterFitFile << "------------------------------------------------------------" << endl;

            // Redirect cout to our master file stream, since TFitResult::Print only writes to cout
            streambuf* CoutBuf = cout.rdbuf();
            cout.rdbuf(MasterFitFile.rdbuf());

            cout << "HV photopeak fit:" << endl;
            HVFit->Print("V");
            cout << endl << "LV photopeak fit:" << endl;
            LVFit->Print("V");

            // Restore normal terminal routing
            cout.rdbuf(CoutBuf);

            MasterFitFile << endl << endl; // Add spacing between different bin entries
          }

          // Photopeak fit plots of this depth bin
          if (m_SavePlots == true) {
            PlotPhotopeakFits(HVHist, PhotopeakFunctionHV, LVHist, PhotopeakFunctionLV, DetID, c);
          }

        } else {
          if (m_Verbosity >= c_Warning) cout << "[Warning] Fits failed for depth bin " << c << " Detector " << DetID << endl;
        }
      } else {
        if (m_Verbosity >= c_Warning) cout << "[Warning] Fewer than " << g_MinCounts << " counts in depth bin " << c << " Detector " << DetID << endl;
      }
    }
  }

  // Close the fit log and write the experimental CCEs
  if (m_SaveProducts == true) {
    // Close output parameter file
    MasterFitFile.close();
    if (m_Verbosity >= c_Info) cout << "Master fit results log saved successfully." << endl;

    // Setup Experimental CCE parameter file
    ofstream OutputCalFile;
    OutputCalFile.open(m_ProductsPrefix + MString("_ExperimentalCCEs.txt"));

    // Header matching updated CCE outputs
    OutputCalFile << "Det_ID" << '\t'
                  << "Depth_Bin" << '\t'
                  << "Mean_Depth_mm" << '\t'
                  << "HV_CCE" << '\t'
                  << "HV_CCE_error" << '\t'
                  << "LV_CCE" << '\t'
                  << "LV_CCE_error" << endl;
    if (m_Verbosity >= c_Info) cout << "Parameter file set up" << endl;

    // Loop systematically over each depth bin first
    for (int c = 0; c < m_NDepthBins; ++c) {

      if (m_Verbosity >= c_Info) cout << "Writing output tracking parameters for depth bin: " << c << endl;

      // Loop over the detectors found inside this specific depth bin
      for (auto const& [DetID, FitsVec] : FullDetEndpoints[c]) {

        double HVCCE     = 0.0;
        double HVCCEErr  = 0.0;
        double LVCCE     = 0.0;
        double LVCCEErr  = 0.0;
        double MeanDepth = 0.0;

        if (FitsVec.size() >= 5) {
          HVCCE     = FitsVec[0];
          HVCCEErr  = FitsVec[1];
          LVCCE     = FitsVec[2];
          LVCCEErr  = FitsVec[3];
          MeanDepth = FitsVec[4];
        }

        // Write row entries corresponding to the detector level measurements
        OutputCalFile << DetID << '\t'
                      << c << '\t'
                      << MeanDepth << '\t'
                      << HVCCE << '\t'
                      << HVCCEErr << '\t'
                      << LVCCE << '\t'
                      << LVCCEErr << endl;
      }
    }

    OutputCalFile.close();
    if (m_Verbosity >= c_Info) cout << "Experimental CCE file saved successfully." << endl;
  }

  // ---- Fit experimental CCE to simulated CCE to extract trapping parameters for each detector ----
  if (m_Verbosity >= c_Info) cout << "\nStarting Joint Trapping Fitting and Matrix Generation..." << endl;

  // Load the simulated CCE files for all detectors
  bool HasDetectorHeaders = false;
  map<int, SimulatedCCE> SimCCEs = LoadSimulatedCCE(g_SimulatedCCEPath.Data(), HasDetectorHeaders);
  if (SimCCEs.empty() == true) {
    if (m_Verbosity >= c_Error) cerr << "[Error] Simulated CCE file is empty or file not found at: " << g_SimulatedCCEPath << endl;
    return false;
  }
  if (HasDetectorHeaders == false) {
    if (m_Verbosity >= c_Info) cout << "Simulated CCE file has no \"### <detector_ID>\" headers: using it for all detectors" << endl;
  }

  // All detectors with experimental points
  set<int> DetectorIDs;
  for (const auto& Point : ExpPoints) {
    DetectorIDs.insert(Point.DetID);
  }

  // Combined trapping parameter file for all detectors, readable by MModuleTrappingCorrection
  string CombinedCSV = string(m_OutFile.Data()) + "_trapping_params_" + to_string(m_NDepthBins) + "bins.csv";
  ofstream CombinedFile(CombinedCSV, ios::out | ios::trunc);
  if (CombinedFile.is_open() == false) {
    if (m_Verbosity >= c_Error) cerr << "[Error] Cannot create output file: " << CombinedCSV << endl;
    return false;
  }
  int NFittedDetectors = 0;

  for (int DetID : DetectorIDs) {
    string OutCSV   = string(m_ProductsPrefix.Data()) + "_Det_" + to_string(DetID) + "_sim_output.csv";
    string ParamCSV = string(m_ProductsPrefix.Data()) + "_Det_" + to_string(DetID) + "_trapping_params_" + to_string(m_NDepthBins) + "bins.csv";

    // Find this detector's simulated CCE curves by iterating through the map
    auto SimIter = (HasDetectorHeaders == true) ? SimCCEs.find(DetID) : SimCCEs.find(0);
    if (SimIter == SimCCEs.end() || SimIter->second.DepthMM.empty() == true) {
      if (m_Verbosity >= c_Error) cerr << "[Error] No simulated CCE file for Detector " << DetID << " in " << g_SimulatedCCEPath << ". Skipping joint fit." << endl;
      continue;
    }

    // Filter experimental points for current detector
    vector<ExperimentalPoint> DetExpPoints;
    for (const auto& Point : ExpPoints) {
      if (Point.DetID == DetID) {
        DetExpPoints.push_back(Point);
      }
    }

    vector<double> TrappingParams;
    if (FitTrappingParameters(DetExpPoints, SimIter->second, OutCSV, ParamCSV, TrappingParams, DetID) == true) {
      if (NFittedDetectors > 0) CombinedFile << "\n";
      WriteTrappingParamBlock(CombinedFile, TrappingParams, SimIter->second, DetID);
      ++NFittedDetectors;
    }
  }

  CombinedFile.close();
  if (m_Verbosity >= c_Info) cout << "Wrote trapping parameters for " << NFittedDetectors << " of " << DetectorIDs.size() << " detector(s) to " << CombinedCSV << endl;

  // Report elapsed time (signaling the end of the run) at the default verbosity setting
  Watch.Stop();
  if (m_Verbosity >= c_Error) cout << "TrappingCorrectionCs137 finished, elapsed time: " << Watch.RealTime() << " s (CPU time: " << Watch.CpuTime() << " s)" << endl;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TrappingCorrectionCs137::CalibrateHits()
{
  // Store the input files
  vector<MString> FileNames;
  FileNames.push_back(m_FileName);
  if (m_Verbosity >= c_Info) cout << "file name stored" << endl;

  MString InputFile = FileNames[0];
  if (m_Verbosity >= c_Info) cout << " input file stored as: " << InputFile << endl;
  vector<MString> HDFNames;

  // Load the geometry which the depth calibration needs for the detector dimensions
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFile) == true) {
    if (m_Verbosity >= c_Info) cout << "Geometry " << Geometry->GetName() << " loaded!" << endl;
  } else {
    if (m_Verbosity >= c_Error) cout << "[Error] Unable to load geometry " << m_GeometryFile << endl;
    return false;
  }

  // Add the Energy Calibration here as an avialable module s.t the Depth Cal can use it in its Initialize() function
  MSupervisor::GetSupervisor()->AddAvailableModule(new MModuleEnergyCalibration());

  // Read in the input files and make a list of hdf5 files to calibrate
  if ((InputFile.GetSubString(InputFile.Length() - 4)) == "hdf5") {
    HDFNames.push_back(InputFile);
    if (m_Verbosity >= c_Info) cout << "hdf names loaded correctly" << endl;
  } else if ((InputFile.GetSubString(InputFile.Length() - 3)) == "txt") {
    if (m_Verbosity >= c_Info) cout << "Reading input file " << InputFile << endl;
    if (m_Verbosity >= c_Warning) cout << "[Warning] When passing a list of files, ensure that you have chosen the correct HDF5 continuous reading mode. Use the --nocontinue option to suppress continuous file reading." << endl;
    MFile F;
    if (F.Open(InputFile) == false) {
      if (m_Verbosity >= c_Error) cout << "[Error] Failed to open input file." << endl;
      return false;
    } else {
      MString Line;
      while (F.ReadLine(Line) == true) {
        MString Trimmed = Line.Trim();
        if ((Trimmed != "")) {
          if (F.Exists(Trimmed) == true) {
            HDFNames.push_back(Trimmed);
          } else {
            if (m_Verbosity >= c_Error) cout << "[Error] Could not find file " << Trimmed << endl;
            return false;
          }
        }
      }
    }
  } else {
    if (m_Verbosity >= c_Error) cout << "[Error] Unrecognized file format: " << InputFile << endl;
    return false;
  }

  // Analyze all the data and fill in the histograms
  for (unsigned int f = 0; f < HDFNames.size(); ++f) {

    MString File = HDFNames[f];
    if (m_Verbosity >= c_Info) cout << "Beginning analysis of file " << File << endl;

    // Create and initialize nuclearizer modules
    MSupervisor* S = MSupervisor::GetSupervisor();

    MModuleLoaderMeasurementsHDF* Loader;
    MModuleTACCalibration* TACCalibrator;
    MModuleEnergyCalibration* EnergyCalibrator;
    MModuleEventFilter* EventFilter;
    MModuleDepthCalibration* DepthCalibrator;

    unsigned int MNumber = 0;
    if (m_Verbosity >= c_Info) cout << "Creating HDF5 loader" << endl;
    Loader = new MModuleLoaderMeasurementsHDF();
    Loader->SetFileNameStripMap(m_StripMapFile);
    Loader->SetFileName(File);
    Loader->SetLoadContinuationFiles(m_ContinueHDF5);
    S->SetModule(Loader, MNumber);
    ++MNumber;

    if (m_Verbosity >= c_Info) cout << "Creating TAC calibrator" << endl;
    TACCalibrator = new MModuleTACCalibration();
    TACCalibrator->SetTACCalFileName(m_TACCalFile);
    S->SetModule(TACCalibrator, MNumber);
    ++MNumber;

    if (m_Verbosity >= c_Info) cout << "Creating energy calibrator" << endl;
    EnergyCalibrator = new MModuleEnergyCalibration();
    EnergyCalibrator->SetFileName(m_EcalFile);
    EnergyCalibrator->SetSlowThresholdCutMode(MSlowThresholdCutModes::e_Fixed);
    EnergyCalibrator->SetSlowThresholdCutFixedValue(30.0);
    S->SetModule(EnergyCalibrator, MNumber);
    ++MNumber;

    if (m_Verbosity >= c_Info) cout << "Creating Event filter" << endl;
    EventFilter = new MModuleEventFilter();
    EventFilter->SetMinimumLVStrips(1);
    EventFilter->SetMaximumLVStrips(3);
    EventFilter->SetMinimumHVStrips(1);
    EventFilter->SetMaximumHVStrips(3);
    EventFilter->SetMinimumHits(0);
    EventFilter->SetMaximumHits(100);
    EventFilter->SetMinimumTotalEnergy(m_MinEnergy);
    EventFilter->SetMaximumTotalEnergy(m_MaxEnergy ); // Multiply by 2 because this is the event-level energy, i.e. sum over both sides
    S->SetModule(EventFilter, MNumber);
    ++MNumber;

    if (m_Verbosity >= c_Info) cout << "Creating strip pairing" << endl;
    MModule* Pairing;
    if (m_MultiRoundStripPairing == true) {
      Pairing = new MModuleStripPairingMultiRoundChiSquare();
    } else {
      Pairing = new MModuleStripPairingChiSquare();
    }
    S->SetModule(Pairing, MNumber);
    ++MNumber;

    if (m_Verbosity >= c_Info) cout << "Creating depth calibrator" << endl;
    DepthCalibrator = new MModuleDepthCalibration();
    DepthCalibrator->SetGeometry(Geometry);
    DepthCalibrator->SetCoeffsFileName(m_DepthCoeffsFile);
    DepthCalibrator->SetSplinesFileName(m_DepthSplinesFile);
    DepthCalibrator->SetUCSDOverride(false);
    DepthCalibrator->SetMaskMetrologyCorrectionEnable(false);
    DepthCalibrator->SetAccountForDisabledStrips(false);
    S->SetModule(DepthCalibrator, MNumber);
    ++MNumber;

    if (m_Verbosity >= c_Info) cout<<"Initializing Loader"<<endl;
    if (Loader->Initialize() == false) return false;
    if (m_Verbosity >= c_Info) cout<<"Initializing TAC calibrator"<<endl;
    if (TACCalibrator->Initialize() == false) return false;
    if (m_Verbosity >= c_Info) cout<<"Initializing Energy calibrator"<<endl;
    if (EnergyCalibrator->Initialize() == false) return false;
    if (m_Verbosity >= c_Info) cout<<"Initializing Event filter"<<endl;
    if (EventFilter->Initialize() == false) return false;
    if (m_Verbosity >= c_Info) cout<<"Initializing Pairing"<<endl;
    if (Pairing->Initialize() == false) return false;
    if (m_Verbosity >= c_Info) cout<<"Initializing Depth calibrator"<<endl;
    if (DepthCalibrator->Initialize() == false) return false;

    // Depth bins follow the detector thicknesses known to the depth calibration
    for (auto const& [DetID, Grid] : DepthCalibrator->GetDepthGrid()) {
      if (Grid.size() < 2) continue;
      m_DepthRanges[DetID] = make_pair(Grid.front(), Grid.back());
    }

    bool IsFinished = false;
    MReadOutAssembly* Event = new MReadOutAssembly();
    if (m_Verbosity >= c_Info) cout<<"Modules initialized, starting event loop"<<endl;

    if (m_Verbosity >= c_Info) cout << "Analyzing..." << endl;

    while ((IsFinished == false) && (m_Interrupt == false)) {
      Event->Clear();
      if (Loader->IsReady() == true) {

        Loader->AnalyzeEvent(Event);
        TACCalibrator->AnalyzeEvent(Event);
        EnergyCalibrator->AnalyzeEvent(Event);
        bool Unfiltered = EventFilter->AnalyzeEvent(Event);

        if (Unfiltered == true) {

          Pairing->AnalyzeEvent(Event);

          // Returns false for guard ring vetoed events
          bool DepthCalibrated = DepthCalibrator->AnalyzeEvent(Event);

          if ((DepthCalibrated == true) && (Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true)) {

            for (unsigned int h = 0; h < Event->GetNHits(); ++h) {

              MHit* H = Event->GetHit(h);

              // Skip hits without a calibrated depth 
              if (H->GetNoDepth() == true) continue;

              double HVEnergy = 0.0;
              double LVEnergy = 0.0;
              unsigned int NHVStrips = 0;
              unsigned int NLVStrips = 0;
              int DetID = H->GetStripHit(0)->GetDetectorID();

              for (unsigned int sh = 0; sh < H->GetNStripHits(); ++sh) {
                MStripHit* SH = H->GetStripHit(sh);

                if ((m_ExcludeNN == false) || ((m_ExcludeNN == true) && (SH->IsNearestNeighbor() == false))) {
                  if (SH->IsLowVoltageStrip() == true) {
                    LVEnergy += SH->GetEnergy();
                    ++NLVStrips;
                  } else {
                    HVEnergy += SH->GetEnergy();
                    ++NHVStrips;
                  }
                }
              }

              if ((NHVStrips == 0) || (NLVStrips == 0)) continue;

              // Local depth in cm -- the same quantity MModuleTrappingCorrection uses to apply the correction
              double DepthCM = H->GetLocalPosition().GetZ();
              m_Hits.push_back({DetID, float(DepthCM), float(HVEnergy), float(LVEnergy)});
            }
          }
        }
      }

      IsFinished = Loader->IsFinished();
    }
  } // end of loop over input files

  if (m_Verbosity >= c_Info) cout << "Calibrated " << m_Hits.size() << " hits" << endl;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TrappingCorrectionCs137::SaveCalibratedHits(const MString& FileName)
{
  // Write a ROOT file to store the calibrated hits
  TFile HitsFile(FileName.Data(), "recreate");
  if (HitsFile.IsZombie() == true) {
    if (m_Verbosity >= c_Error) cout << "[Error] Unable to create the hits file " << FileName << endl;
    return false;
  }

  // In a ROOT file, store:
  // the information from each calibrated hit (detector_ID, depth, HV energy, LV energy)
  // the depth bin ranges calculated by GetDepthBin

  int DetID;
  float DepthCM, HVEnergy, LVEnergy;
  TTree* HitsTree = new TTree("Hits", "Calibrated Cs-137 hits");
  HitsTree->Branch("DetID", &DetID, "DetID/I");
  HitsTree->Branch("DepthCM", &DepthCM, "DepthCM/F");
  HitsTree->Branch("HVEnergy", &HVEnergy, "HVEnergy/F");
  HitsTree->Branch("LVEnergy", &LVEnergy, "LVEnergy/F");
  for (const CalibratedHit& Hit : m_Hits) {
    DetID = Hit.DetID;
    DepthCM = Hit.DepthCM;
    HVEnergy = Hit.HVEnergy;
    LVEnergy = Hit.LVEnergy;
    HitsTree->Fill();
  }

  double MinDepthCM, MaxDepthCM;
  TTree* RangesTree = new TTree("DepthRanges", "Depth range (cm) of the depth calibration of each detector");
  RangesTree->Branch("DetID", &DetID, "DetID/I");
  RangesTree->Branch("MinDepthCM", &MinDepthCM, "MinDepthCM/D");
  RangesTree->Branch("MaxDepthCM", &MaxDepthCM, "MaxDepthCM/D");
  for (auto const& [ID, Range] : m_DepthRanges) {
    DetID = ID;
    MinDepthCM = Range.first;
    MaxDepthCM = Range.second;
    RangesTree->Fill();
  }

  // We only need hits that are within the filtered energy window
  TParameter<double> MinEnergy("MinEnergy", m_MinEnergy);
  TParameter<double> MaxEnergy("MaxEnergy", m_MaxEnergy);
  MinEnergy.Write();
  MaxEnergy.Write();

  HitsFile.Write();
  HitsFile.Close();

  if (m_Verbosity >= c_Info) cout << "Saved " << m_Hits.size() << " calibrated hits to " << FileName << endl;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TrappingCorrectionCs137::LoadHits(const MString& FileName)
{
  TFile HitsFile(FileName.Data(), "read");
  if (HitsFile.IsZombie() == true) {
    if (m_Verbosity >= c_Error) cout << "[Error] Unable to open the hits file " << FileName << endl;
    return false;
  }

  TTree* HitsTree = dynamic_cast<TTree*>(HitsFile.Get("Hits"));
  TTree* RangesTree = dynamic_cast<TTree*>(HitsFile.Get("DepthRanges"));
  if (HitsTree == nullptr || RangesTree == nullptr) {
    if (m_Verbosity >= c_Error) cout << "[Error] The file " << FileName << " does not contain calibrated hits written by --save-hits" << endl;
    return false;
  }

  int DetID;
  float DepthCM, HVEnergy, LVEnergy;
  HitsTree->SetBranchAddress("DetID", &DetID);
  HitsTree->SetBranchAddress("DepthCM", &DepthCM);
  HitsTree->SetBranchAddress("HVEnergy", &HVEnergy);
  HitsTree->SetBranchAddress("LVEnergy", &LVEnergy);
  m_Hits.clear();
  m_Hits.reserve(HitsTree->GetEntries());
  for (Long64_t e = 0; e < HitsTree->GetEntries(); ++e) {
    HitsTree->GetEntry(e);
    m_Hits.push_back({DetID, DepthCM, HVEnergy, LVEnergy});
  }

  double MinDepthCM, MaxDepthCM;
  RangesTree->SetBranchAddress("DetID", &DetID);
  RangesTree->SetBranchAddress("MinDepthCM", &MinDepthCM);
  RangesTree->SetBranchAddress("MaxDepthCM", &MaxDepthCM);
  m_DepthRanges.clear();
  for (Long64_t e = 0; e < RangesTree->GetEntries(); ++e) {
    RangesTree->GetEntry(e);
    m_DepthRanges[DetID] = make_pair(MinDepthCM, MaxDepthCM);
  }

  // Hits outside of the energy window of the calibration run are not in the file
  TParameter<double>* MinEnergy = dynamic_cast<TParameter<double>*>(HitsFile.Get("MinEnergy"));
  TParameter<double>* MaxEnergy = dynamic_cast<TParameter<double>*>(HitsFile.Get("MaxEnergy"));
  if (MinEnergy != nullptr && MaxEnergy != nullptr) {
    if (m_MinEnergy < MinEnergy->GetVal() || m_MaxEnergy > MaxEnergy->GetVal()) {
      if (m_Verbosity >= c_Warning) cout << "[Warning] The hits in " << FileName << " were calibrated with the energy window " << MinEnergy->GetVal() << "-" << MaxEnergy->GetVal()
                                         << " keV, which is narrower than the requested " << m_MinEnergy << "-" << m_MaxEnergy << " keV" << endl;
    }
  }

  HitsFile.Close();

  if (m_Verbosity >= c_Info) cout << "Loaded " << m_Hits.size() << " calibrated hits for " << m_DepthRanges.size() << " detector(s) from " << FileName << endl;

  return true;
}

////////////////////////////////////////////////////////////////////////////////////

void TrappingCorrectionCs137::InitializeDepthBins()
{
  // Evenly spaced depth bins spanning each detector's depth grid (in cm)
  m_DepthBinEdges.clear();
  for (auto const& [DetID, Range] : m_DepthRanges) {
    double MinDepth = Range.first;
    double MaxDepth = Range.second;
    vector<double> Edges;
    for (int i = 0; i <= m_NDepthBins; ++i) {
      Edges.push_back(MinDepth + (MaxDepth - MinDepth) * double(i) / double(m_NDepthBins));
    }
    m_DepthBinEdges[DetID] = Edges;
    if (m_Verbosity >= c_Info) cout << "Detector " << DetID << ": " << m_NDepthBins << " has measured depth bins from " << MinDepth << " cm to " << MaxDepth << " cm" << endl;
  }
}

//////////////////////////////////////////////////////////////////////////////////

int TrappingCorrectionCs137::GetDepthBin(int DetID, double DepthCM) const
{
  auto Iter = m_DepthBinEdges.find(DetID);
  if (Iter == m_DepthBinEdges.end()) return -1;
  const vector<double>& Edges = Iter->second;

  // Hard bounds check 
  if (DepthCM < Edges.front() || DepthCM > Edges.back()) return -1;

  // Binary search on the bin edges
  int Bin = distance(Edges.begin(), upper_bound(Edges.begin(), Edges.end(), DepthCM)) - 1;

  // Guard against edge cases at boundaries
  if (Bin >= m_NDepthBins) Bin = m_NDepthBins - 1;
  if (Bin < 0) Bin = 0;

  return Bin;
}

////////////////////////////////////////////////////////////////////////////////////

TF1* TrappingCorrectionCs137::GeneratePhotopeakFunction()
{
  // Component 1: Core Gaussian
  // exp(-(x-x0)^2 / (2*sigma^2))
  MString GaussStr = "exp(-(x-[1])^2 / (2*[2]^2))";

  // Component 2: Exponential Tail + Shelf
  // BoverA * exp(gamma*(x-x0)) * 0.5 * erfc((x-x0)/(sigma*sigma_ratio*sqrt(2)))
  MString ExpTailStr = "[3] * exp([4]*(x-[1])) * 0.5 * erfc((x-[1])/([2]*[5]*sqrt(2)))";

  // Component 3: Linear Tail + Shelf
  // BoverA * CoverB * (1 + D*(x-x0)) * 0.5 * erfc((x-x0)/(sigma*sigma_ratio*sqrt(2)))
  MString LinTailStr = "[3] * [6] * (1 + [7]*(x-[1])) * 0.5 * erfc((x-[1])/([2]*[5]*sqrt(2)))";

  // Combine components with an overall normalization scaling factor [0]
  MString FullFormula = "[0] * (" + GaussStr + " + " + ExpTailStr + " + " + LinTailStr + ")";

  // Instantiate TF1 over your expected fit window
  TF1* PhotopeakFunction = new TF1("PhotopeakFunction", FullFormula.Data(), 645, 675);

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

  return PhotopeakFunction;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

map<int, TrappingCorrectionCs137::SimulatedCCE> TrappingCorrectionCs137::LoadSimulatedCCE(const string& SimCSVPath, bool& HasDetectorHeaders)
{
  map<int, SimulatedCCE> Sims;
  HasDetectorHeaders = false;
  ifstream File(SimCSVPath);

  if (File.is_open() == false) {
    if (m_Verbosity >= c_Error) cerr << "[Error] Could not open simulation CSV at: " << SimCSVPath << endl;
    return Sims;
  }

  // Lines before the first "### <detector_ID>" header belong to detector 0
  int DetID = 0;

  string Line;
  while (getline(File, Line)) {
    // Detector section header: "### <detector_ID>"
    if (Line.rfind("###", 0) == 0) {
      stringstream HeaderStream(Line.substr(3));
      int NewID;
      if ((HeaderStream >> NewID).fail() == false) {
        DetID = NewID;
        HasDetectorHeaders = true;
        if (Sims.count(DetID) > 0 && Sims[DetID].DepthMM.empty() == false) {
          if (m_Verbosity >= c_Warning) cerr << "[Warning] Detector " << DetID << " appears more than once in " << SimCSVPath << "; appending rows" << endl;
        }
      } else {
        if (m_Verbosity >= c_Warning) cerr << "[Warning] Could not read a detector_ID from header line: " << Line << endl;
      }
      continue;
    }

    // Skip empty lines, comment lines starting with #, or header text
    if (Line.empty() || Line[0] == '#' || Line.find("z_depth") != string::npos) {
      continue;
    }

    stringstream LineStream(Line);
    string Value;
    vector<double> Row;

    while (getline(LineStream, Value, ',')) {
      try {
        Row.push_back(stod(Value));
      } catch (...) {
        Row.clear();
        break; // Skip non-numeric header lines
      }
    }

    // Expecting 5 columns: [z_depth_mm, e_CCE_HV, h_CCE_HV, e_CCE_LV, h_CCE_LV]
    if (Row.size() == 5) {
      SimulatedCCE& Sim = Sims[DetID];
      Sim.DepthMM.push_back(Row[0]);
      Sim.HVElectronCCE.push_back(Row[1]);
      Sim.HVHoleCCE.push_back(Row[2]);
      Sim.LVElectronCCE.push_back(Row[3]);
      Sim.LVHoleCCE.push_back(Row[4]);
    }
  }

  for (auto const& [ID, Sim] : Sims) {
    if (m_Verbosity >= c_Info) cout << "Loaded simulated CCE file for Detector " << ID << " with " << Sim.DepthMM.size() << " depth points" << endl;
  }

  return Sims;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TrappingCorrectionCs137::FitTrappingParameters(const vector<ExperimentalPoint>& ExpData,
                                                    const SimulatedCCE& Sim,
                                                    const string& OutputCSVPath,
                                                    const string& TrappingParamPath,
                                                    vector<double>& TrappingParams,
                                                    int DetID)
{
  if (Sim.DepthMM.empty() == true) {
    if (m_Verbosity >= c_Error) cerr << "[Error] Simulated CCE file for Detector " << DetID << " is empty" << endl;
    return false;
  }

  /////////////// Data Filters /////////////////////

  // Reject data points with high error or zero error
  auto IsHVValid = [&](const ExperimentalPoint& P) {
    return IsValidCCEErr(P.HVCCEErr);
  };
  auto IsLVValid = [&](const ExperimentalPoint& P) {
    return IsValidCCEErr(P.LVCCEErr);
  };

  vector<ExperimentalPoint> DetExpData;
  int NRejected = 0, NTrimmed = 0, NHV = 0, NLV = 0;

  // Only retain HV or LV data points w acceptable errors
  for (const auto& Point : ExpData) {
    if (Point.DetID != DetID && DetID != -1) continue;
    // Exclude the depth bins at the detector edges according toward inputted trim
    // This is to avoid fitting the trapping parameters in regions where the simulated CCE is not well characterized
    if (IsTrimmedBin(Point.DepthBin) == true) { NTrimmed++; continue; }
    const bool HVValid = IsHVValid(Point), LVValid = IsLVValid(Point);
    if (HVValid == false && LVValid == false) { NRejected++; continue; }   // neither side usable
    DetExpData.push_back(Point);
    if (HVValid == true) NHV++;
    if (LVValid == true) NLV++;
  }

  if (DetExpData.empty() == true) {
    if (m_Verbosity >= c_Warning) cout << "[Warning] No valid data points left after error filtering for Detector "
              << DetID << endl;
    return false;
  }
  if (m_Verbosity >= c_Info) cout << "Detector " << DetID << ": " << NTrimmed << " edge depth bins trimmed, "
            << NRejected << " points rejected for their errors" << endl;

  // Set up and sort 1D interpolations for Sim CCEs
  TGraph HVElectronGraph(Sim.DepthMM.size(), Sim.DepthMM.data(), Sim.HVElectronCCE.data());
  TGraph HVHoleGraph(Sim.DepthMM.size(), Sim.DepthMM.data(), Sim.HVHoleCCE.data());
  TGraph LVElectronGraph(Sim.DepthMM.size(), Sim.DepthMM.data(), Sim.LVElectronCCE.data());
  TGraph LVHoleGraph(Sim.DepthMM.size(), Sim.DepthMM.data(), Sim.LVHoleCCE.data());

  HVElectronGraph.Sort(); HVHoleGraph.Sort();
  LVElectronGraph.Sort(); LVHoleGraph.Sort();

  double MinSimDepth = Sim.DepthMM.front();
  double MaxSimDepth = Sim.DepthMM.back();

  // Chi2 cost fuction to fit trapping parameters
  auto Chi2Function = [&](const double* P) -> double {
    double Chi2 = 0.0;

    double AHV = P[0];
    double ALV = P[1];
    double B   = P[2];
    double C   = P[3];

    for (const auto& Point : DetExpData) {
      // Calibrated  mean depth from MModuleDepthCalibration converted from cm to mm during Analyze to match the simulated CCE file
      double Z = Point.DepthMM;

      // If the measured depth is outide of simulation bounds assign it the closest CCE w/i the simulation
      if (Z < MinSimDepth) Z = MinSimDepth;
      if (Z > MaxSimDepth) Z = MaxSimDepth;

      // Evaluate TGraphs
      double HVElectron = HVElectronGraph.Eval(Z);
      double HVHole     = HVHoleGraph.Eval(Z);
      double LVElectron = LVElectronGraph.Eval(Z);
      double LVHole     = LVHoleGraph.Eval(Z);

      double PredHV = TrappedCCE(AHV, B, C, HVElectron, HVHole);
      double PredLV = TrappedCCE(ALV, B, C, LVElectron, LVHole);

      // For valid data points, find chi2
      if (IsHVValid(Point) == true) {
        double Pull = (Point.HVCCE - PredHV) / Point.HVCCEErr;
        Chi2 += Pull * Pull;
      }
      if (IsLVValid(Point) == true) {
        double Pull = (Point.LVCCE - PredLV) / Point.LVCCEErr;
        Chi2 += Pull * Pull;
      }
    }

    return (isnan(Chi2) == true) ? 1e9 : Chi2;
  };

  // Wrap lambda inside ROOT::Math::Functor
  ROOT::Math::Functor FCN(Chi2Function, 4);

  // Configure Fitter
  ROOT::Fit::Fitter Fitter;

  // Initialize parameters and steps:
  // A_hv, A_lv start at the largest measured CCE on each side, B at 1 and C at 9
  double MaxHVCCE = 0.0, MaxLVCCE = 0.0;
  for (const auto& Point : DetExpData) {
    if (IsHVValid(Point) == true) MaxHVCCE = max(MaxHVCCE, Point.HVCCE);
    if (IsLVValid(Point) == true) MaxLVCCE = max(MaxLVCCE, Point.LVCCE);
  }
  double InitParams[4] = {(NHV > 0) ? MaxHVCCE : 1.0, (NLV > 0) ? MaxLVCCE : 1.0, 1.0, 9.0};
  double InitSteps[4]  = {1e-4, 1e-4, 0.05, 0.05};

  Fitter.Config().SetMinimizer("Minuit2", "Migrad");
  Fitter.Config().MinimizerOptions().SetStrategy(2);
  Fitter.Config().MinimizerOptions().SetTolerance(1e-4);
  Fitter.Config().MinimizerOptions().SetMaxFunctionCalls(100000);
  Fitter.Config().MinimizerOptions().SetPrintLevel((m_Verbosity >= c_Info) ? 1 : 0);
  Fitter.Config().SetParamsSettings(4, InitParams, InitSteps);
  // MINOS errors on all parameters
  Fitter.Config().SetMinosErrors(true);

  //  A_hv, A_lv unbounded, B and C only required to be >= 0.
  Fitter.Config().ParSettings(2).SetLowerLimit(0.0);
  Fitter.Config().ParSettings(3).SetLowerLimit(0.0);

  Fitter.SetFCN(FCN);

  // Run the minimization 
  bool FitSucceeded = Fitter.FitFCN();
  // If the fitter didn't run
  if (FitSucceeded == false) {
    if (m_Verbosity >= c_Error) cerr << "[Error] Joint trapping fit failed for Detector " << DetID << endl;
    return false;
  }

  ///////////////////////// Printed fit statistics /////////////////////////////////////////

  const ROOT::Fit::FitResult& Result = Fitter.Result();

  if (Result.IsValid() == false) {
    if (m_Verbosity >= c_Warning) cerr << "[Warning] Fit result not valid (status " << Result.Status() << ") for Detector " << DetID << endl;
  }

  // Write the fitted trapping parameters
  TrappingParams.assign(4, 0.0);
  for (int i = 0; i < 4; ++i) TrappingParams[i] = Result.Parameter(i);

  double CalculatedChi2 = Chi2Function(TrappingParams.data());
  int NPoints = NHV + NLV; // Total data points (HV + LV)
  int NDF = NPoints - 4;

  if (m_Verbosity >= c_Info) {
    cout << "\n================ JOINT FIT RESULTS ================" << endl;
    cout << "  Detector ID : " << DetID << endl;
    cout << "  A_hv        : " << TrappingParams[0] << endl;
    cout << "  A_lv        : " << TrappingParams[1] << endl;
    cout << "  B (e-trap)  : " << TrappingParams[2] << endl;
    cout << "  C (h-trap)  : " << TrappingParams[3] << endl;
    cout << "  Chi2 (Calc) : " << CalculatedChi2 << endl;
    cout << "  NDF         : " << NDF << endl;
    cout << "  Chi2 / NDF  : " << (NDF > 0 ? CalculatedChi2 / NDF : -1) << endl;
    cout << "====================================================\n" << endl;
  }

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  if (m_SavePlots == true) {
    PlotCCEFit(ExpData, Sim, TrappingParams, DetID);
  }
  if (m_SaveProducts == true) {
    SaveTrappingParamFile(OutputCSVPath, TrappingParamPath, TrappingParams, Sim, DetID);
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TrappingCorrectionCs137::PlotCCEFit(const vector<ExperimentalPoint>& ExpData,
                                         const SimulatedCCE& Sim,
                                         const vector<double>& TrappingParams,
                                         int DetID)
{
  // Only draw into the png file, without opening a window
  bool WasBatch = gROOT->IsBatch();
  gROOT->SetBatch(true);

  // One pad per side: HV on the left, LV on the right
  TCanvas* Canvas = new TCanvas(Form("CCEFitCanvas_Det%d", DetID), Form("CCE fit Detector %d", DetID), 1400, 600);
  Canvas->Divide(2, 1);

  const double B = TrappingParams[2];
  const double C = TrappingParams[3];

  for (int s = 0; s < 2; ++s) {
    const bool IsHV = (s == 0);
    const double A = TrappingParams[s];
    const vector<double>& ElectronCCE = (IsHV == true) ? Sim.HVElectronCCE : Sim.LVElectronCCE;
    const vector<double>& HoleCCE = (IsHV == true) ? Sim.HVHoleCCE : Sim.LVHoleCCE;

    // Simulated CCE (A = B = C = 1) and the simulated CCE scaled by the fitted trapping parameters
    TGraph* SimGraph = new TGraph();
    TGraph* FitGraph = new TGraph();
    for (size_t i = 0; i < Sim.DepthMM.size(); ++i) {
      SimGraph->SetPoint(i, Sim.DepthMM[i], TrappedCCE(1.0, 1.0, 1.0, ElectronCCE[i], HoleCCE[i]));
      FitGraph->SetPoint(i, Sim.DepthMM[i], TrappedCCE(A, B, C, ElectronCCE[i], HoleCCE[i]));
    }
    SimGraph->Sort();
    FitGraph->Sort();
    SimGraph->SetLineColor(kGray + 2);
    SimGraph->SetLineStyle(2);
    SimGraph->SetLineWidth(2);
    FitGraph->SetLineColor(kRed + 1);
    FitGraph->SetLineWidth(2);

    // Experimental CCEs, split into the points used in the fit and the ones excluded by the trim or the error cut
    TGraphErrors* UsedGraph = new TGraphErrors();
    TGraphErrors* ExcludedGraph = new TGraphErrors();
    for (const auto& Point : ExpData) {
      if (Point.DetID != DetID && DetID != -1) continue;
      double CCE = (IsHV == true) ? Point.HVCCE : Point.LVCCE;
      double CCEErr = (IsHV == true) ? Point.HVCCEErr : Point.LVCCEErr;
      TGraphErrors* Graph = (IsTrimmedBin(Point.DepthBin) == false && IsValidCCEErr(CCEErr) == true) ? UsedGraph : ExcludedGraph;
      int N = Graph->GetN();
      Graph->SetPoint(N, Point.DepthMM, CCE);
      Graph->SetPointError(N, 0.0, CCEErr);
    }
    UsedGraph->SetMarkerStyle(20);
    UsedGraph->SetMarkerColor(kBlack);
    UsedGraph->SetLineColor(kBlack);
    ExcludedGraph->SetMarkerStyle(24);
    ExcludedGraph->SetMarkerColor(kGray + 1);
    ExcludedGraph->SetLineColor(kGray + 1);

    TMultiGraph* MultiGraph = new TMultiGraph(Form("CCEFit_Det%d_%s", DetID, (IsHV == true) ? "HV" : "LV"),
                                              Form("Detector %d %s side;Depth (mm);CCE", DetID, (IsHV == true) ? "HV" : "LV"));
    MultiGraph->SetBit(kCanDelete);
    MultiGraph->Add(SimGraph, "L");
    MultiGraph->Add(FitGraph, "L");

    TLegend* Legend = new TLegend(0.40, 0.15, 0.85, 0.38);
    Legend->SetBit(kCanDelete);
    Legend->AddEntry(SimGraph, "Simulated CCE", "l");
    Legend->AddEntry(FitGraph, Form("Fit: A=%.4f, B=%.3f, C=%.3f", A, B, C), "l");
    if (UsedGraph->GetN() > 0) {
      MultiGraph->Add(UsedGraph, "P");
      Legend->AddEntry(UsedGraph, "Measured (used in fit)", "pe");
    } else {
      delete UsedGraph;
    }
    if (ExcludedGraph->GetN() > 0) {
      MultiGraph->Add(ExcludedGraph, "P");
      Legend->AddEntry(ExcludedGraph, "Measured (excluded)", "pe");
    } else {
      delete ExcludedGraph;
    }

    Canvas->cd(s + 1);
    MultiGraph->Draw("A");
    Legend->Draw();
  }

  MString PlotFile = m_PlotsPrefix + MString("_Det_") + DetID + MString("_CCE_fit.png");
  Canvas->SaveAs(PlotFile.Data());
  delete Canvas;

  gROOT->SetBatch(WasBatch);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TrappingCorrectionCs137::PlotPhotopeakFits(TH1D* HVHist, TF1* HVFunction, TH1D* LVHist, TF1* LVFunction, int DetID, int DepthBin)
{
  // Only draw into the png file, without opening a window
  bool WasBatch = gROOT->IsBatch();
  gROOT->SetBatch(true);

  // One pad per side: HV on the left, LV on the right
  TCanvas* Canvas = new TCanvas(Form("PhotopeakCanvas_Det%d_Bin%d", DetID, DepthBin), Form("Photopeak fits Detector %d depth bin %d", DetID, DepthBin), 1400, 600);
  Canvas->Divide(2, 1);

  TH1D* Hists[2] = {HVHist, LVHist};
  TF1* Functions[2] = {HVFunction, LVFunction};
  for (int s = 0; s < 2; ++s) {
    Canvas->cd(s + 1);
    Hists[s]->SetTitle(Form("Detector %d depth bin %d %s side;Energy (keV);Counts", DetID, DepthBin, (s == 0) ? "HV" : "LV"));
    Hists[s]->Draw("Hist");
    Functions[s]->Draw("same");
  }

  MString PlotFile = m_PlotsPrefix + MString("_Det_") + DetID + MString("_Depthbin_") + DepthBin + MString("_photopeak_fits.png");
  Canvas->SaveAs(PlotFile.Data());
  delete Canvas;

  gROOT->SetBatch(WasBatch);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool TrappingCorrectionCs137::CreateOutputDirectory(const string& DirName, MString& Prefix)
{
  // The directory is created next to the output file, and the files in it start with the output file name
  filesystem::path OutPath(m_OutFile.Data());
  filesystem::path Dir = OutPath.parent_path() / DirName;
  error_code DirError;
  filesystem::create_directories(Dir, DirError);
  if (filesystem::is_directory(Dir) == false) {
    if (m_Verbosity >= c_Error) cout << "[Error] Unable to create directory " << Dir.string() << endl;
    return false;
  }
  Prefix = MString((Dir / OutPath.filename()).string());
  if (m_Verbosity >= c_Info) cout << "Files will be stored in " << Dir.string() << endl;
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TrappingCorrectionCs137::SaveTrappingParamFile(const string& OutputFilePath,
                                                    const string& TrappingParamFilePath,
                                                    const vector<double>& TrappingParams,
                                                    const SimulatedCCE& Sim,
                                                    int DetID)
{
  // Lambda function to write CSV content
  auto WriteFileContent = [&](const string& FilePath) {
    ofstream CSVFile(FilePath, ios::out | ios::trunc);
    if (CSVFile.is_open() == false) {
      if (m_Verbosity >= c_Error) cerr << "[Error] Cannot create output file: " << FilePath << endl;
      return;
    }
    WriteTrappingParamBlock(CSVFile, TrappingParams, Sim, DetID);
    CSVFile.close();
  };

  WriteFileContent(OutputFilePath);
  WriteFileContent(TrappingParamFilePath);

  if (m_Verbosity >= c_Info) cout << "Successfully generated trapping parameter file for Detector "
            << DetID << " at: " << OutputFilePath << endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TrappingCorrectionCs137::WriteTrappingParamBlock(ostream& Out,
                                                      const vector<double>& TrappingParams,
                                                      const SimulatedCCE& Sim,
                                                      int DetID)
{
  //  Detector ID
  Out << "### " << DetID << "\n";

  // Trapping parameters
  Out << "# A_HV, A_LV, B, C\n";
  Out << TrappingParams[0] << "," << TrappingParams[1] << "," << TrappingParams[2] << "," << TrappingParams[3] << "\n";

  // Column Headers
  Out << "# z_depth_cm,e_CCE_HV,h_CCE_HV,e_CCE_LV,h_CCE_LV\n";

  // Array Data (Converted Depth from mm to cm)
  for (size_t i = 0; i < Sim.DepthMM.size(); ++i) {
    double DepthCM = Sim.DepthMM[i] / 10.0; // sims are in mm but nuclearizer expects cm
    Out << DepthCM << ","
        << Sim.HVElectronCCE[i] << ","
        << Sim.HVHoleCCE[i] << ","
        << Sim.LVElectronCCE[i] << ","
        << Sim.LVHoleCCE[i] << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////


TrappingCorrectionCs137* g_Prg = 0;
int g_NInterruptCatches = 1;

////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    if (g_Prg->GetVerbosity() >= c_Warning) cout<<"Catched signal Ctrl-C (ID="<<a<<"):"<<endl;
    g_Prg->Interrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Main program
int main(int argc, char** argv)
{
  // Catch a user interrupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables
  MGlobal::Initialize("TrappingCorrectionCs137", "A standalone program to produce fitted charge trapping parameters to be used in MModuleTrappingCorrection");

  TApplication TrappingCorrectionApp("TrappingCorrectionApp", 0, 0);

  g_Prg = new TrappingCorrectionCs137();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    if (g_Prg->GetVerbosity() >= c_Error) cerr << "Error during parsing of command line!" << endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    if (g_Prg->GetVerbosity() >= c_Error) cerr << "Error during analysis!" << endl;
    return -2;
  } 

  TrappingCorrectionApp.Run();

  if (g_Prg->GetVerbosity() >= c_Info) cout << "Program exited normally!" << endl;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////