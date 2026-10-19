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

// Systematic floor on the photopeak centroid errors (keV), added in quadrature in the joint fit
double g_SysErrKeV = 0.0;

// Number of depth bins at each detector face excluded from the joint fit (trim_index in depth_helpers.fit_CCE)
int g_TrimBins = 1;

// Simulation Template Path for Joint Trapping Fitting
MString g_SimulatedCCEPath = "sim_cce_template.csv";

class TrappingCorrectionCs137
{
public:
  TrappingCorrectionCs137();
  ~TrappingCorrectionCs137();

  // Helper method to look up the depth bin index for a given detector and depth (cm)
  int GetDepthBin(int DetID, double DepthCM) const;

  // Structs for CCE simulations & experimental points
  struct SimulatedCCE {
    std::vector<double> z_depth_mm;
    std::vector<double> e_cce_hv;
    std::vector<double> h_cce_hv;
    std::vector<double> e_cce_lv;
    std::vector<double> h_cce_lv;
  };

  struct ExperimentalPoint {
    int det_id;
    int depth_bin;
    double depth_mm; // mean calibrated depth of the events in this bin
    double hv_cce;
    double hv_cce_err;
    double lv_cce;
    double lv_cce_err;
  };
  
  bool ParseCommandLine(int argc, char** argv);
  bool Analyze();
  void Interrupt() { m_Interrupt = true; }

  // Photopeak fitting function
  TF1* GeneratePhotopeakFunction();

  // Load Sim CCE files
  // Lines after "### <detector ID>" belong to that detector; lines before any such header to detector 0
  map<int, SimulatedCCE> LoadSimulatedCCE(const std::string& sim_csv_path, bool& HasDetectorHeaders);
  
  // Output trapping paramters and simulated CCE insto csv file
  void SaveTrappingParamFile(const std::string& output_filepath,
                                 const std::string& trapping_param_filepath,
                                 const std::vector<double>& popt,
                                 const SimulatedCCE& sim,
                                 int detector_id);

  // Write one "### <detector ID>" block in the format MModuleTrappingCorrection reads
  void WriteTrappingParamBlock(std::ostream& out,
                               const std::vector<double>& popt,
                               const SimulatedCCE& sim,
                               int detector_id);

 // Fit experimental CCE to simulated CCE to get trapping parameters
  // Returns false if no fit result could be obtained; on success popt holds A_HV, A_LV, B, C
  bool FitTrappingParameters(const std::vector<ExperimentalPoint>& exp_data,
                                     const SimulatedCCE& sim,
                                     const std::string& output_csv_path,
                                     const std::string& trapping_param_path,
                                     std::vector<double>& popt,
                                     int detector_id = 0,
                                     double sys_err_kev = 0.0);

private:
  // Set up evenly spaced depth bins per detector from the depth calibration's depth grid
  void InitializeDepthBins(MModuleDepthCalibration* DepthCalibrator);

  // Depth bins
  static const int NDepthBins = 20; // Set to your desired bin count
  map<int, vector<double>> m_DepthBinEdges; // [DetID] -> NDepthBins + 1 edges in cm

  bool m_Interrupt;
  MString m_FileName;
  MString m_EcalFile;
  MString m_TACCalFile;
  MString m_StripMapFile;
  MString m_OutFile;
  MString m_GeometryFile;
  MString m_DepthCoeffsFile;
  MString m_DepthSplinesFile;
  
  bool m_PixelCorrect;
  bool m_MultiRoundStripPairing;
  bool m_ExcludeNN = true;
  bool m_ContinueHDF5; 

  double m_MinEnergy;
  double m_MaxEnergy;
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
  Usage<<"         --syserr :   systematic error floor on centroids in keV (default 0)"<<endl;
  Usage<<"         --trim :   number of depth bins at each face excluded from the trapping fit (default 1)"<<endl;
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
  m_MultiRoundStripPairing = false; // should this be changed??
  m_ContinueHDF5 = true;
  m_MinEnergy = 600;
  m_MaxEnergy = 700;
  
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
    if ((Option == "-i") || (Option == "-o") || (Option == "--emin") || (Option == "--emax") || (Option == "--tcal") || (Option == "-m") || (Option == "--geo") || (Option == "--coeffs") || (Option == "--splines") || (Option == "--sim") || (Option == "--syserr") || (Option == "--trim")) {
      if (!((argc > i+1) && (argv[i+1][0] != '-' || isalpha(argv[i+1][1]) == 0))){
        cout<<"Error: Option "<<argv[i][1]<<" needs a second argument!"<<endl;
        cout<<Usage.str()<<endl;
        return false;
      }
    }  

    /// Then fulfill the options:
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

    if (Option == "-o"){
      m_OutFile = argv[++i];
      cout<<"Accepting file name: "<<m_OutFile<<endl;
    }

    if (Option == "-m"){
      m_StripMapFile = argv[++i];
      cout<<"Accepting file name: "<<m_StripMapFile<<endl;
    }

    if (Option == "--geo") {
      m_GeometryFile = argv[++i];
      cout<<"Accepting geometry file: "<<m_GeometryFile<<endl;
    }

    if (Option == "--coeffs") {
      m_DepthCoeffsFile = argv[++i];
      cout<<"Accepting depth coefficients file: "<<m_DepthCoeffsFile<<endl;
    }

    if (Option == "--splines") {
      m_DepthSplinesFile = argv[++i];
      cout<<"Accepting depth splines file: "<<m_DepthSplinesFile<<endl;
    }

    if (Option == "--syserr") {
      g_SysErrKeV = stod(argv[++i]);
      cout<<"Accepting systematic error floor: "<<g_SysErrKeV<<" keV"<<endl;
    }

    if (Option == "--trim") {
      g_TrimBins = stoi(argv[++i]);
      cout<<"Accepting number of trimmed depth bins per face: "<<g_TrimBins<<endl;
    }

    if (Option == "--sim") {
      g_SimulatedCCEPath = argv[++i];
      cout<<"Accepting sim CCE file: "<<g_SimulatedCCEPath<<endl;
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

  if (m_GeometryFile.IsEmpty() == true || m_DepthCoeffsFile.IsEmpty() == true || m_DepthSplinesFile.IsEmpty() == true) {
    cout<<"Error: The depth calibration needs a geometry (--geo), coefficients (--coeffs) and splines (--splines) file!"<<endl;
    cout<<Usage.str()<<endl;
    return false;
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

  // [DepthBin][DetID] -> [HV CCE, HV CCE Err, LV CCE, LV CCE Err, Mean Depth (mm)] for full detector values
  map<int, map<int, vector<double>>> FullDetEndpoints;

  // Store the input files
  vector<MString> FileNames;
  FileNames.push_back(m_FileName);
  cout << "file name stored" << endl;

  MString InputFile = FileNames[0];
  cout << " input file stored as: " << InputFile << endl;
  vector<MString> HDFNames;

  // Enable sum of weights squared for TH1 by default
  TH1::SetDefaultSumw2();

  // Detector-level maps organized by [DepthBin][DetID]
  map<int, map<int, TH1D*>> FullDetHVEnergyHistograms;
  map<int, map<int, TH1D*>> FullDetLVEnergyHistograms;

  // Sum and count of calibrated depths (cm) per [DepthBin][DetID], to evaluate the model at the mean depth of each bin
  map<int, map<int, double>> DepthSums;
  map<int, map<int, long>> DepthCounts;

  // Load the geometry, which the depth calibration needs for the detector dimensions
  MDGeometryQuest* Geometry = new MDGeometryQuest();
  if (Geometry->ScanSetupFile(m_GeometryFile) == true) {
    cout << "Geometry " << Geometry->GetName() << " loaded!" << endl;
  } else {
    cout << "Error: Unable to load geometry " << m_GeometryFile << endl;
    return false;
  }

  // MModuleDepthCalibration::Initialize() looks up the energy calibration among the supervisor's
  // *available* modules, which only MAssembly registers -- register one here for this standalone app
  MSupervisor::GetSupervisor()->AddAvailableModule(new MModuleEnergyCalibration());

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
      return false;
    } else {
      MString Line;
      while (F.ReadLine(Line)) {
        MString Trimmed = Line.Trim();
        if ((Trimmed != "")) {
          if (F.Exists(Trimmed) == true) {
            HDFNames.push_back(Trimmed);
          } else {
            cout << "Error: Could not find file " << Trimmed << endl;
            return false;
          }
        }
      }
    }
  } else {
    cout << "Error: Unrecognized file format: " << InputFile << endl;
    return false;
  }

  // --- PROGRESS TRACKING VARIABLES ---
  long long event_count = 0;
  long long processed_hits = 0;
  long long filled_events = 0;
  auto start_time = std::chrono::steady_clock::now();
  // -----------------------------------

  // Analyze all the data and fill in the histograms
  for (unsigned int f = 0; f < HDFNames.size(); ++f) {

    MString File = HDFNames[f];
    cout << "Beginning analysis of file " << File << endl;

    // Create and initialize nuclearizer modules
    MSupervisor* S = MSupervisor::GetSupervisor();
    
    MModuleLoaderMeasurementsHDF* Loader;
    MModuleTACCalibration* TACCalibrator;
    MModuleEnergyCalibration* EnergyCalibrator;
    MModuleEventFilter* EventFilter;
    MModuleDepthCalibration* DepthCalibrator;

    unsigned int MNumber = 0;
    cout << "Creating HDF5 loader" << endl;
    Loader = new MModuleLoaderMeasurementsHDF();
    Loader->SetFileNameStripMap(m_StripMapFile);
    Loader->SetFileName(File);
    Loader->SetLoadContinuationFiles(m_ContinueHDF5);
    S->SetModule(Loader, MNumber);
    ++MNumber;

    cout << "Creating TAC calibrator" << endl;
    TACCalibrator = new MModuleTACCalibration();
    TACCalibrator->SetTACCalFileName(m_TACCalFile);
    S->SetModule(TACCalibrator, MNumber);
    ++MNumber;
    
    cout << "Creating energy calibrator" << endl;
    EnergyCalibrator = new MModuleEnergyCalibration();
    EnergyCalibrator->SetFileName(m_EcalFile);
    EnergyCalibrator->SetSlowThresholdCutMode(MSlowThresholdCutModes::e_Fixed);
    EnergyCalibrator->SetSlowThresholdCutFixedValue(30.0);
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
    EventFilter->SetMaximumTotalEnergy(m_MaxEnergy ); // Multiply by 2 because this is the event-level energy, i.e. sum over both sides
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
    ++MNumber;

    cout << "Creating depth calibrator" << endl;
    DepthCalibrator = new MModuleDepthCalibration();
    DepthCalibrator->SetGeometry(Geometry);
    DepthCalibrator->SetCoeffsFileName(m_DepthCoeffsFile);
    DepthCalibrator->SetSplinesFileName(m_DepthSplinesFile);
    DepthCalibrator->SetUCSDOverride(false);
    DepthCalibrator->SetMaskMetrologyCorrectionEnable(false);
    DepthCalibrator->SetAccountForDisabledStrips(false);
    S->SetModule(DepthCalibrator, MNumber);
    ++MNumber;

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
    cout<<"Initializing Depth calibrator"<<endl;
    if (DepthCalibrator->Initialize() == false) return false;

    // Depth bins follow the detector thicknesses known to the depth calibration
    InitializeDepthBins(DepthCalibrator);

    bool IsFinished = false;
    MReadOutAssembly* Event = new MReadOutAssembly();
    cout<<"Modules initialized, starting event loop"<<endl;

    cout << "Analyzing..." << endl;

  while ((IsFinished == false) && (m_Interrupt == false)) {
    Event->Clear();
    if (Loader->IsReady()) {

      Loader->AnalyzeEvent(Event);
      event_count++;
      TACCalibrator->AnalyzeEvent(Event);
      EnergyCalibrator->AnalyzeEvent(Event);
      bool Unfiltered = EventFilter->AnalyzeEvent(Event);

      if (Unfiltered == true) {
        
        Pairing->AnalyzeEvent(Event);

        // Returns false for guard ring vetoed events
        bool DepthCalibrated = DepthCalibrator->AnalyzeEvent(Event);

        if ((DepthCalibrated == true) && (Event->HasAnalysisProgress(MAssembly::c_StripPairing) == true)) {
          
          for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
            processed_hits++;

            MHit* H = Event->GetHit(h);

            // Skip hits without a calibrated depth (no timing, out of range CTD, multiple hits on a strip, ...)
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

            // Local depth in cm, the same quantity MModuleTrappingCorrection uses to apply the correction
            double DepthCM = H->GetLocalPosition().GetZ();
            int DepthBin = GetDepthBin(DetID, DepthCM);
            if (DepthBin < 0) continue;

            TH1D* FullHVHist  = FullDetHVEnergyHistograms[DepthBin][DetID];
            TH1D* FullLVHist  = FullDetLVEnergyHistograms[DepthBin][DetID];

            if (FullHVHist == nullptr) {
              char name[128]; sprintf(name, "HV_Detector%d_bin%d", DetID, DepthBin);
              FullHVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy) * 2, m_MinEnergy, m_MaxEnergy);
              FullDetHVEnergyHistograms[DepthBin][DetID] = FullHVHist;
            }
            if (FullLVHist == nullptr) {
              char name[128]; sprintf(name, "LV_Detector%d_bin%d", DetID, DepthBin);
              FullLVHist = new TH1D(name, name, (m_MaxEnergy - m_MinEnergy) * 2, m_MinEnergy, m_MaxEnergy);
              FullDetLVEnergyHistograms[DepthBin][DetID] = FullLVHist;
            }

            FullHVHist->Fill(HVEnergy);
            FullLVHist->Fill(LVEnergy);
            DepthSums[DepthBin][DetID] += DepthCM;
            DepthCounts[DepthBin][DetID] += 1;
            filled_events++;
          }
        }
      }
    }

    IsFinished = Loader->IsFinished();
  }
  } // end of loop over input files

  // --- FINAL SUMMARY PRINT ---
  auto end_time = std::chrono::steady_clock::now();
  double total_time = std::chrono::duration<double>(end_time - start_time).count();

  cout << "\n==========================================" << endl;
  cout << " Finished Event Loop!" << endl;
  cout << " Total Events Processed: " << event_count << endl;
  cout << " Total Hits Examined:    " << processed_hits << endl;
  cout << " Total Histograms Filled:" << filled_events << endl;
  cout << " Time Elapsed:           " << total_time << " s" << endl;
  cout << " Exit Reason:            " << (m_Interrupt ? "User Interrupt (SIGINT)" : "Loader Finished") << endl;
  cout << "==========================================\n" << endl;

  // Store depth binned fit results in an output file 
  ofstream MasterFitFile;
  MasterFitFile.open(m_OutFile + MString("_All_DepthBin_FitResults.txt"));
  MasterFitFile << "======================================================================" << endl;
  MasterFitFile << "MASTER PHOTOPEAK FIT LOG FOR ALL DEPTH BINS AND DETECTORS" << endl;
  MasterFitFile << "======================================================================" << endl << endl;

  // Vector to hold experimental CCE points for joint fit
  std::vector<ExperimentalPoint> exp_points;

  // Do function fitting and recording for full detector outputs
  for (int c = 0; c < NDepthBins; ++c) {

    cout << "Processing depth bin " << c << endl;
  
    for (auto const& [DetID, FullHVHist] : FullDetHVEnergyHistograms[c]) {
  
      TH1D* HVHist = FullDetHVEnergyHistograms[c][DetID];
      TH1D* LVHist = FullDetLVEnergyHistograms[c][DetID];

      if (HVHist->Integral() > g_MinCounts) {

        TF1* PhotopeakFunctionHV = GeneratePhotopeakFunction();
        TFitResultPtr HVFit = HVHist->Fit(PhotopeakFunctionHV, "S", "", 645, 675);

        TF1* PhotopeakFunctionLV = GeneratePhotopeakFunction();
        TFitResultPtr LVFit = LVHist->Fit(PhotopeakFunctionLV, "S", "", 645, 675);

        if ((HVFit >= 0) && (LVFit >= 0)) {
      
          // Clear or initialize the vector for this specific bin and detector
          FullDetEndpoints[c][DetID].clear();
          
          double hv_centroid = HVFit->Parameter(1);
          double hv_error    = HVFit->ParError(1);
          double lv_centroid = LVFit->Parameter(1);
          double lv_error    = LVFit->ParError(1);

          // Convert Centroids (keV) to CCE (dimensionless ratio relative to 661.7 keV)
          double hv_cce     = hv_centroid / g_CsPhotopeak;
          double hv_cce_err = hv_error / g_CsPhotopeak;
          double lv_cce     = lv_centroid / g_CsPhotopeak;
          double lv_cce_err = lv_error / g_CsPhotopeak;

          FullDetEndpoints[c][DetID].push_back(hv_cce);      // HV CCE
          FullDetEndpoints[c][DetID].push_back(hv_cce_err);  // HV CCE Err
          FullDetEndpoints[c][DetID].push_back(lv_cce);      // LV CCE
          FullDetEndpoints[c][DetID].push_back(lv_cce_err);  // LV CCE Err

          // Evaluate the model at the mean calibrated depth of the events in this bin, not the bin midpoint
          double mean_depth_mm = 10.0 * DepthSums[c][DetID] / DepthCounts[c][DetID];
          FullDetEndpoints[c][DetID].push_back(mean_depth_mm); // Mean depth (mm)

          exp_points.push_back({DetID, c, mean_depth_mm, hv_cce, hv_cce_err, lv_cce, lv_cce_err});

          MasterFitFile << "------------------------------------------------------------" << endl;
          MasterFitFile << " DETECTOR ID: " << DetID << "  |  DEPTH BIN INDEX: " << c << "  |  MEAN DEPTH (mm): " << mean_depth_mm << endl;
          MasterFitFile << "------------------------------------------------------------" << endl;
          
          // Redirect cout to our master file stream
          std::streambuf* coutbuf = cout.rdbuf();
          cout.rdbuf(MasterFitFile.rdbuf());
          
          // Restore normal terminal routing
          cout.rdbuf(coutbuf);

          MasterFitFile << endl << endl; // Add spacing between different bin entries

          TFile HVHistFile(m_OutFile + MString("_Det") + DetID + MString("_Depthbin_") + c + MString("_HVEnergyHist_Illum.root"), "recreate");
          TCanvas* HVHistCanvas = new TCanvas();
          HVHistCanvas->cd();
          HVHist->Draw("Hist");
          PhotopeakFunctionHV->Draw("same");
          HVHistCanvas->Write();
          HVHistFile.Close();

          TFile LVHistFile(m_OutFile + MString("_Det") + DetID + MString("_Depthbin_") + c + MString("_LVEnergyHist_Illum.root"), "recreate");
          TCanvas* LVHistCanvas = new TCanvas();
          LVHistCanvas->cd();
          LVHist->Draw("Hist");
          PhotopeakFunctionLV->Draw("same");
          LVHistCanvas->Write();
          LVHistFile.Close();

        } else {
          cout << "Fits failed for depth bin " << c << " Detector " << DetID << endl;
        }
      } else {
        cout << "Fewer than " << g_MinCounts << " counts in depth bin " << c << " Detector " << DetID << endl;
      }
    }
  }

  // Close output parameter file
  MasterFitFile.close();
  cout << "Master fit results log saved successfully." << endl;

  // Setup Experimental CCE parameter file
  ofstream OutputCalFile;
  OutputCalFile.open(m_OutFile + MString("_ExperimentalCCEs.txt"));
  
  // Header matching updated CCE outputs
  OutputCalFile << "Det_ID" << '\t' 
                << "Depth_Bin" << '\t' 
                << "Mean_Depth_mm" << '\t' 
                << "HV_CCE" << '\t'
                << "HV_CCE_error" << '\t'
                << "LV_CCE" << '\t'
                << "LV_CCE_error" << endl;
  cout << "Parameter file set up" << endl;

  // Loop systematically over each depth bin first
  for (int c = 0; c < NDepthBins; ++c) {

    cout << "Writing output tracking parameters for depth bin: " << c << endl;

    // Loop over the detectors found inside this specific depth bin
    for (auto const& [DetID, FitsVec] : FullDetEndpoints[c]) {
      
      double HV_CCE     = 0.0;
      double HV_CCEErr  = 0.0;
      double LV_CCE     = 0.0;
      double LV_CCEErr  = 0.0;
      double MeanDepth  = 0.0;

      if (FitsVec.size() >= 5) {
        HV_CCE    = FitsVec[0];
        HV_CCEErr = FitsVec[1];
        LV_CCE    = FitsVec[2];
        LV_CCEErr = FitsVec[3];
        MeanDepth = FitsVec[4];
      }

      // Write row entries corresponding purely to the detector level measurements
      OutputCalFile << DetID << '\t'
                    << c << '\t'
                    << MeanDepth << '\t'
                    << HV_CCE << '\t' 
                    << HV_CCEErr << '\t' 
                    << LV_CCE << '\t'
                    << LV_CCEErr << endl;
    }
  }

  OutputCalFile.close();
  cout << "Experimental CCE file saved successfully." << endl;
  
  // ---- Fit experimental CCE to simulated CCE to extract trapping parameters for each detector ----
  cout << "\nStarting Joint Trapping Fitting and Matrix Generation..." << endl;

  // Load the simulation templates for all detectors
  bool HasDetectorHeaders = false;
  map<int, SimulatedCCE> SimTemplates = LoadSimulatedCCE(g_SimulatedCCEPath.Data(), HasDetectorHeaders);
  if (SimTemplates.empty()) {
    std::cerr << "[FATAL] Simulation template is empty or file not found at: " << g_SimulatedCCEPath << std::endl;
    return false;
  }
  if (HasDetectorHeaders == false) {
    cout << "Simulation template has no \"### <detector ID>\" headers: using it for all detectors" << endl;
  }

  // All detectors with experimental points
  std::set<int> DetectorIDs;
  for (const auto& pt : exp_points) {
    DetectorIDs.insert(pt.det_id);
  }

  // Combined trapping parameter file for all detectors, readable by MModuleTrappingCorrection
  std::string combined_csv = std::string(m_OutFile.Data()) + "_trapping_params_" + std::to_string(NDepthBins) + "bins.csv";
  std::ofstream CombinedFile(combined_csv, std::ios::out | std::ios::trunc);
  if (!CombinedFile.is_open()) {
    std::cerr << "[Error] Cannot create output file: " << combined_csv << std::endl;
    return false;
  }
  int NFittedDetectors = 0;

  for (int det_id : DetectorIDs) {
    std::string out_csv   = "Det_" + std::to_string(det_id) + "_sim_output.csv";
    std::string param_csv = "Det_" + std::to_string(det_id) + "_trapping_params_" + std::to_string(NDepthBins) + "bins.csv";

    // Pick this detector's simulation template (a template without detector headers applies to all detectors)
    auto SimIter = (HasDetectorHeaders == true) ? SimTemplates.find(det_id) : SimTemplates.find(0);
    if (SimIter == SimTemplates.end() || SimIter->second.z_depth_mm.empty()) {
      std::cerr << "[Error] No simulation template for Detector " << det_id << " in " << g_SimulatedCCEPath << ". Skipping joint fit." << std::endl;
      continue;
    }

    // Filter experimental points for current detector
    std::vector<ExperimentalPoint> det_exp_points;
    for (const auto& pt : exp_points) {
      if (pt.det_id == det_id) {
        det_exp_points.push_back(pt);
      }
    }

    std::vector<double> popt;
    if (FitTrappingParameters(det_exp_points, SimIter->second, out_csv, param_csv, popt, det_id, g_SysErrKeV) == true) {
      if (NFittedDetectors > 0) CombinedFile << "\n";
      WriteTrappingParamBlock(CombinedFile, popt, SimIter->second, det_id);
      ++NFittedDetectors;
    }
  }

  CombinedFile.close();
  cout << "Wrote trapping parameters for " << NFittedDetectors << " of " << DetectorIDs.size() << " detector(s) to " << combined_csv << endl;

  watch.Stop();
  cout << "total time (s): " << watch.CpuTime() << endl;
 
  return true;
}

////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////

map<int, TrappingCorrectionCs137::SimulatedCCE> TrappingCorrectionCs137::LoadSimulatedCCE(const std::string& csv_path, bool& HasDetectorHeaders) {
  map<int, SimulatedCCE> sims;
  HasDetectorHeaders = false;
  std::ifstream file(csv_path);

  if (!file.is_open()) {
    std::cerr << "[Error] Could not open simulation CSV at: " << csv_path << std::endl;
    return sims;
  }

  // Lines before the first "### <detector ID>" header belong to detector 0
  int det_id = 0;

  std::string line;
  while (std::getline(file, line)) {
    // Detector section header: "### <detector ID>"
    if (line.rfind("###", 0) == 0) {
      std::stringstream hs(line.substr(3));
      int new_id;
      if (hs >> new_id) {
        det_id = new_id;
        HasDetectorHeaders = true;
        if (sims.count(det_id) > 0 && sims[det_id].z_depth_mm.empty() == false) {
          std::cerr << "[Warning] Detector " << det_id << " appears more than once in " << csv_path << "; appending rows" << std::endl;
        }
      } else {
        std::cerr << "[Warning] Could not read a detector ID from header line: " << line << std::endl;
      }
      continue;
    }

    // Skip empty lines, comment lines starting with #, or header text
    if (line.empty() || line[0] == '#' || line.find("z_depth") != std::string::npos) {
      continue;
    }

    std::stringstream ss(line);
    std::string val;
    std::vector<double> row;

    while (std::getline(ss, val, ',')) {
      try {
        row.push_back(std::stod(val));
      } catch (...) {
        row.clear();
        break; // Skip non-numeric header lines
      }
    }

    // Expecting 5 columns: [z_depth_mm, e_CCE_HV, h_CCE_HV, e_CCE_LV, h_CCE_LV]
    if (row.size() == 5) {
      SimulatedCCE& sim = sims[det_id];
      sim.z_depth_mm.push_back(row[0]);
      sim.e_cce_hv.push_back(row[1]);
      sim.h_cce_hv.push_back(row[2]);
      sim.e_cce_lv.push_back(row[3]);
      sim.h_cce_lv.push_back(row[4]);
    }
  }

  for (auto const& [id, sim] : sims) {
    cout << "Loaded simulation template for Detector " << id << " with " << sim.z_depth_mm.size() << " depth points" << endl;
  }

  return sims;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void TrappingCorrectionCs137::SaveTrappingParamFile(const std::string& output_filepath,
                                 const std::string& trapping_param_filepath,
                                 const std::vector<double>& popt,
                                 const SimulatedCCE& sim,
                                 int detector_id) 
{
  // Lambda function to write CSV content
  auto write_file_content = [&](const std::string& filepath) {
    std::ofstream csv_file(filepath, std::ios::out | std::ios::trunc);
    if (!csv_file.is_open()) {
      std::cerr << "[Error] Cannot create output file: " << filepath << std::endl;
      return;
    }
    WriteTrappingParamBlock(csv_file, popt, sim, detector_id);
    csv_file.close();
  };

  write_file_content(output_filepath);
  write_file_content(trapping_param_filepath);

  std::cout << "Successfully generated trapping parameter file for Detector " 
            << detector_id << " at: " << output_filepath << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void TrappingCorrectionCs137::WriteTrappingParamBlock(std::ostream& out,
                                                      const std::vector<double>& popt,
                                                      const SimulatedCCE& sim,
                                                      int detector_id)
{
  //  Detector ID
  out << "### " << detector_id << "\n";

  // Trapping parameters
  out << "# A_HV, A_LV, B, C\n";
  out << popt[0] << "," << popt[1] << "," << popt[2] << "," << popt[3] << "\n";

  // Column Headers 
  out << "# z_depth_cm,e_CCE_HV,h_CCE_HV,e_CCE_LV,h_CCE_LV\n";

  // Array Data (Converted Depth from mm to cm)
  for (size_t i = 0; i < sim.z_depth_mm.size(); ++i) {
    double sim_z_cm = sim.z_depth_mm[i] / 10.0; // sims are in mm but nuclearizer expects cm
    out << sim_z_cm << ","
        << sim.e_cce_hv[i] << ","
        << sim.h_cce_hv[i] << ","
        << sim.e_cce_lv[i] << ","
        << sim.h_cce_lv[i] << "\n";
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool TrappingCorrectionCs137::FitTrappingParameters(const std::vector<ExperimentalPoint>& exp_data,
  const SimulatedCCE& sim,
  const std::string& output_csv_path,
  const std::string& trapping_param_path,
  std::vector<double>& popt,
  int detector_id,
  double sys_err_kev) 
{
if (sim.z_depth_mm.empty()) {
std::cerr << "[Error] Simulation template for Detector " << detector_id << " is empty" << std::endl;
return false;
}

/////////////// Data Filters /////////////////////

// Reject data points with high error or zero error
const double max_allowed_cce_err = 6.0 / g_CsPhotopeak;  

auto hv_ok = [&](const ExperimentalPoint& p) {
  return p.hv_cce_err > 0.0 && p.hv_cce_err <= max_allowed_cce_err;
};
auto lv_ok = [&](const ExperimentalPoint& p) {
  return p.lv_cce_err > 0.0 && p.lv_cce_err <= max_allowed_cce_err;
};

std::vector<ExperimentalPoint> det_exp_data;
int rejected_counts = 0, trimmed_counts = 0, n_hv = 0, n_lv = 0;

// Only retain HV or LV data points w acceptable errors
for (const auto& pt : exp_data) {
  if (pt.det_id != detector_id && detector_id != -1) continue;
  // Exclude the depth bins at each face, as depth_helpers.fit_CCE does with trim_index
  if (pt.depth_bin < g_TrimBins || pt.depth_bin >= NDepthBins - g_TrimBins) { trimmed_counts++; continue; }
  const bool h = hv_ok(pt), l = lv_ok(pt);
  if (!h && !l) { rejected_counts++; continue; }   // neither side usable
  det_exp_data.push_back(pt);
  if (h) n_hv++;
  if (l) n_lv++;
}

if (det_exp_data.empty()) {
std::cout << "[Info] No valid data points left after error filtering for Detector " 
<< detector_id << std::endl;
return false;
}
std::cout << "[Info] Detector " << detector_id << ": " << trimmed_counts << " edge depth bins trimmed, "
          << rejected_counts << " points rejected for their errors" << std::endl;

// Set up and sort 1D Interpolation Graphs for Sim CCEs
TGraph g_hv_e(sim.z_depth_mm.size(), sim.z_depth_mm.data(), sim.e_cce_hv.data());
TGraph g_hv_h(sim.z_depth_mm.size(), sim.z_depth_mm.data(), sim.h_cce_hv.data());
TGraph g_lv_e(sim.z_depth_mm.size(), sim.z_depth_mm.data(), sim.e_cce_lv.data());
TGraph g_lv_h(sim.z_depth_mm.size(), sim.z_depth_mm.data(), sim.h_cce_lv.data());

g_hv_e.Sort(); g_hv_h.Sort();
g_lv_e.Sort(); g_lv_h.Sort();

double z_min_sim = sim.z_depth_mm.front();
double z_max_sim = sim.z_depth_mm.back();

// Systematic floor on the centroid errors, added in quadrature
const double sys_cce = sys_err_kev / g_CsPhotopeak;

// Chi2 cost fuction to fit trapping parameters
auto chi2_lambda = [&](const double* p) -> double {
  double chi2 = 0.0;
  
  double A_hv = p[0];
  double A_lv = p[1];
  double B    = p[2];
  double C    = p[3];

  for (const auto& pt : det_exp_data) {
    // Calibrated depth from MModuleDepthCalibration (mm), same coordinate as the simulation template
    double z = pt.depth_mm;

    // If the measured depth is outide of simulation bounds assign it the closest CCE w/i the simulation
    if (z < z_min_sim) z = z_min_sim;
    if (z > z_max_sim) z = z_max_sim;

    // Evaluate TGraphs
    double e_hv = g_hv_e.Eval(z);
    double h_hv = g_hv_h.Eval(z);
    double e_lv = g_lv_e.Eval(z);
    double h_lv = g_lv_h.Eval(z);


    double pred_hv = A_hv * (1.0 - B * (1.0 - e_hv)) * (1.0 - C * (1.0 - h_hv));
    double pred_lv = A_lv * (1.0 - B * (1.0 - e_lv)) * (1.0 - C * (1.0 - h_lv));

    double err_hv = std::hypot(pt.hv_cce_err, sys_cce);
    double err_lv = std::hypot(pt.lv_cce_err, sys_cce);

    // For valid data points, find chi2
    if (hv_ok(pt)) {
      double d = (pt.hv_cce - pred_hv) / err_hv;
      chi2 += d * d;
    }
    if (lv_ok(pt)) {
      double d = (pt.lv_cce - pred_lv) / err_lv;
      chi2 += d * d;
    }
  }
  
  return std::isnan(chi2) ? 1e9 : chi2;
};

//DEBUG
double py_best[4] = {1.00143528, 1.00046766, 1.0786083, 0.29929985};
std::cout << "chi2 at python optimum = " << chi2_lambda(py_best) << std::endl;   // expect ≈ 781.4

// Wrap lambda inside ROOT::Math::Functor
ROOT::Math::Functor fcn(chi2_lambda, 4);

// Configure Fitter
ROOT::Fit::Fitter fitter;

// Initialize parameters and steps, matching depth_helpers.fit_CCE:
// A_hv, A_lv start at the largest measured CCE on each side, B at 1 and C at 9
double max_hv_cce = 0.0, max_lv_cce = 0.0;
for (const auto& pt : det_exp_data) {
  if (hv_ok(pt)) max_hv_cce = std::max(max_hv_cce, pt.hv_cce);
  if (lv_ok(pt)) max_lv_cce = std::max(max_lv_cce, pt.lv_cce);
}
double init_params[4] = {(n_hv > 0) ? max_hv_cce : 1.0, (n_lv > 0) ? max_lv_cce : 1.0, 1.0, 9.0};
double init_steps[4]  = {1e-4, 1e-4, 0.05, 0.05};

fitter.Config().SetMinimizer("Minuit2", "Migrad");
fitter.Config().MinimizerOptions().SetStrategy(2);
fitter.Config().MinimizerOptions().SetTolerance(1e-4);
fitter.Config().MinimizerOptions().SetMaxFunctionCalls(100000);
fitter.Config().SetParamsSettings(4, init_params, init_steps);
// MINOS errors on all parameters, as in depth_helpers.fit_CCE
fitter.Config().SetMinosErrors(true);

// As in depth_helpers.fit_CCE: A_hv, A_lv unbounded, B and C only required to be >= 0.
// No upper limits, so strongly damaged detectors (e.g. C > 2 after proton irradiation) are not pinned at a bound.
fitter.Config().ParSettings(2).SetLowerLimit(0.0);
fitter.Config().ParSettings(3).SetLowerLimit(0.0);

fitter.SetFCN(fcn);

// Run the minimization -- SetFCN alone does not fit, and Result() asserts if no fit was run
bool ok = fitter.FitFCN();
if (!ok) {
  std::cerr << "[Error] Joint trapping fit failed for Detector " << detector_id << std::endl;
  return false;
}

///////////////////////// Printed fit statistics /////////////////////////////////////////

// Extract Chi2 manually if fitter result still reports -1
const ROOT::Fit::FitResult& result = fitter.Result();
result.Print(std::cout);   // status, EDM, parameter errors
if (!result.IsValid()) {
  std::cerr << "[Warning] Fit result not valid (status " << result.Status() << ") for Detector " << detector_id << std::endl;
}
popt.assign(4, 0.0);
for (int i = 0; i < 4; ++i) popt[i] = result.Parameter(i);

// Residual pulls vs. depth: random scatter -> errors underestimated (floor justified), structure -> model or depth mismatch
std::cout << "  depth_mm   pull_HV   pull_LV" << std::endl;
for (const auto& pt : det_exp_data) {
  double z = std::clamp(pt.depth_mm, z_min_sim, z_max_sim);
  double pred_hv = popt[0] * (1 - popt[2]*(1 - g_hv_e.Eval(z))) * (1 - popt[3]*(1 - g_hv_h.Eval(z)));
  double pred_lv = popt[1] * (1 - popt[2]*(1 - g_lv_e.Eval(z))) * (1 - popt[3]*(1 - g_lv_h.Eval(z)));
  std::cout << "  " << pt.depth_mm << "  "
            << (hv_ok(pt) ? (pt.hv_cce - pred_hv) / std::hypot(pt.hv_cce_err, sys_cce) : NAN) << "  "
            << (lv_ok(pt) ? (pt.lv_cce - pred_lv) / std::hypot(pt.lv_cce_err, sys_cce) : NAN) << std::endl;
}

double calculated_chi2 = chi2_lambda(popt.data());
int n_points = n_hv + n_lv; // Total data points (HV + LV)
int ndf = n_points - 4;

std::cout << "\n================ JOINT FIT RESULTS ================" << std::endl;
std::cout << "  Detector ID : " << detector_id << std::endl;
std::cout << "  Sys. floor  : " << sys_err_kev << " keV" << std::endl;
std::cout << "  A_hv        : " << popt[0] << std::endl;
std::cout << "  A_lv        : " << popt[1] << std::endl;
std::cout << "  B (e-trap)  : " << popt[2] << std::endl;
std::cout << "  C (h-trap)  : " << popt[3] << std::endl;
std::cout << "  Chi2 (Calc) : " << calculated_chi2 << std::endl;
std::cout << "  NDF         : " << ndf << std::endl;
std::cout << "  Chi2 / NDF  : " << (ndf > 0 ? calculated_chi2 / ndf : -1) << std::endl;
std::cout << "====================================================\n" << std::endl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SaveTrappingParamFile(output_csv_path, trapping_param_path, popt, sim, detector_id);
return true;
}

///////////////////////////////////////////////////////////////////////////////////

void TrappingCorrectionCs137::InitializeDepthBins(MModuleDepthCalibration* DepthCalibrator)
{
  // Evenly spaced depth bins spanning each detector's depth grid (cm), which runs from -thickness/2 to +thickness/2
  m_DepthBinEdges.clear();
  for (auto const& [DetID, Grid] : DepthCalibrator->GetDepthGrid()) {
    if (Grid.size() < 2) continue;
    double MinDepth = Grid.front();
    double MaxDepth = Grid.back();
    vector<double> Edges;
    for (int i = 0; i <= NDepthBins; ++i) {
      Edges.push_back(MinDepth + (MaxDepth - MinDepth) * double(i) / double(NDepthBins));
    }
    m_DepthBinEdges[DetID] = Edges;
    cout << "Detector " << DetID << ": " << NDepthBins << " depth bins from " << MinDepth << " cm to " << MaxDepth << " cm" << endl;
  }
}

//////////////////////////////////////////////////////////////////////////////////

int TrappingCorrectionCs137::GetDepthBin(int DetID, double DepthCM) const 
{
  auto Iter = m_DepthBinEdges.find(DetID);
  if (Iter == m_DepthBinEdges.end()) return -1;
  const vector<double>& Edges = Iter->second;

  // Hard bounds check (the upper edge is included so hits at the electrode aren't dropped)
  if (DepthCM < Edges.front() || DepthCM > Edges.back()) return -1;

  // Binary search on the bin edges
  int bin = std::distance(Edges.begin(), std::upper_bound(Edges.begin(), Edges.end(), DepthCM)) - 1;

  // Guard against edge cases at boundaries
  if (bin >= NDepthBins) bin = NDepthBins - 1;
  if (bin < 0) bin = 0;

  return bin;
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
  // Catch a user interrupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGAlib variables
  MGlobal::Initialize("TrappingCorrectionCs137", "A standalone program to determine depth-dependent shifts of the Cs137 peak");

  TApplication TrappingCorrectionApp("TrappingCorrectionApp", 0, 0);

  g_Prg = new TrappingCorrectionCs137();

  if (g_Prg->ParseCommandLine(argc, argv) == false) {
    cerr << "Error during parsing of command line!" << endl;
    return -1;
  } 
  if (g_Prg->Analyze() == false) {
    cerr << "Error during analysis!" << endl;
    return -2;
  } 

  TrappingCorrectionApp.Run();

  cout << "Program exited normally!" << endl;

  return 0;
}

////////////////////////////////////////////////////////////////////////////////