/*
 * StripEnergyThresholdFinder.cxx
 *
 * Author: Jarred Roberts
 * Affiliation: UC San Diego, Department of Astronomy & Astrophysics
 *
 * Description:
 *   Application for determining per-strip energy thresholds for germanium detectors.
 *   Includes both slow (energy-based) and fast (timing-based) threshold estimation,
 *   along with diagnostic outputs and ROOT-based visualization tools.
 *
 * Notes:
 *   - Built on MEGAlib and Nuclearizer frameworks
 *   - Uses ROOT for analysis and visualization
 *
 * Copyright (C) 2026 Jarred Roberts
 *
 * This software is provided for research and academic use.
 * Redistribution and modification should follow the licensing
 * terms of the parent frameworks (MEGAlib/Nuclearizer) where applicable.
 */

// NOTES:
// Compile Command
// make (in the main nuclearizer directory)

// Run command example
// $MEGALIB/bin/StripEnergyThresholdFinder /your/yaml/file/directory/file.yaml

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <cmath>
#include <string>
#include <sstream>

#include <filesystem>
namespace fs = std::filesystem;


using namespace std;

/* ROOT */
#include <TROOT.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>
#include "TLegend.h"
#include <TLine.h>
#include <getopt.h>
#include "TStyle.h"
#include <iomanip>
#include <chrono>

/* YAML */
#include <yaml-cpp/yaml.h>

/* MEGAlib */
#include "MGlobal.h"
#include "MSupervisor.h"
#include "MModuleEnergyCalibration.h"
#include "MModuleLoaderMeasurementsHDF.h"
#include "MReadOutAssembly.h"
#include "MReadOutElementDoubleStrip.h"
#include "MStripHit.h"
#include "MString.h"



// -------------------------------------------------------------
// TAC calibration helper
// -------------------------------------------------------------

class TACCalHelper
{
 public:
  struct Coeff {
    double slope = 0;
    double offset = 0;
  };

  bool Load(const string& fileName)
  {
    ifstream in(fileName);
    if (!in) {
      cout << "Failed to open TAC calibration file: " << fileName << endl;
      return false;
    }

    string line;
    getline(in, line); // skip header

    while (getline(in, line)) {
      if (line.empty()) {
        continue;
      }

      stringstream ss(line);

      int Strip_id, det, side, Strip;
      double slope, slope_err, offset, offset_err;

      ss >> Strip_id >> det >> side >> Strip >> slope >> slope_err >> offset >> offset_err;

      MReadOutElementDoubleStrip R;
      R.SetDetectorID(det);
      R.SetStripID(Strip);
      R.IsLowVoltageStrip(side == 0);

      m_Coeffs[R] = { slope, offset };
    }

    return true;
  }

  double TACToEnergy(MReadOutElementDoubleStrip R, double TAC) const
  {

    if (m_Coeffs.count(R) == 1) {
      const Coeff& c = m_Coeffs.at(R);
      return c.slope * TAC + c.offset;
    } else {
      return 0;
    }
  }

 private:
  map<MReadOutElementDoubleStrip, Coeff> m_Coeffs;
};


// -------------------------------------------------------------
// Hit selection
// -------------------------------------------------------------

bool PassHitSelection(MStripHit* SH)
{
  if (SH == nullptr) {
    return false;
  }

  // Basic sanity checks only
  if (SH->GetStripID() < 0) {
    return false;
  }
  if (SH->GetDetectorID() < 0) {
    return false;
  }

  // Require signal
  if (SH->GetADCUnits() <= 0) {
    return false;
  }

  return true;
}


//! Class for determining strip energy thresholds from histogrammed data
class MStripThresholdFinder
{
 public:
  MStripThresholdFinder()
  {
  }

  bool ParseCommandLine(int Argc, char** Argv);
  bool BuildHistograms();

  void FindSlowThresholds();
  void FindFastThresholds();

  void WriteCSV();
  void WriteDiagnostics();

  MString GetOutputPrefix() const
  {
    return m_OutputPrefix;
  }

 private:
  // --- Configuration ---
  MModuleEnergyCalibration m_EnergyCalibration;
  vector<string> m_InputFiles;
  MString m_CalibrationFile;
  MString m_StripMapFile;
  MString m_OutputPrefix;

  int m_MinEntries = 10;
  double m_FallbackThreshold = 20.0;

  int m_HistogramBins = 512;
  double m_HistogramMaxADC = 4096.0;
  double m_NoiseSearchMaxADC = 2200.0;

  long m_MaxEvents = -1;

  // --- Data ---
  map<MReadOutElementDoubleStrip, TH1D*> m_ADCHistograms;
  map<MReadOutElementDoubleStrip, map<int, pair<int, int>>> m_TimingCounts;

  map<MReadOutElementDoubleStrip, TH1D*> dt0_hists;
  map<MReadOutElementDoubleStrip, TH1D*> dt1_hists;

  // --- Results ---
  map<MReadOutElementDoubleStrip, double> m_SlowThresholds;
  map<MReadOutElementDoubleStrip, double> m_SlowThresholdsADC;

  map<MReadOutElementDoubleStrip, double> m_FastThresholds;
  map<MReadOutElementDoubleStrip, double> m_FastThresholdsADC;

  // --- Helpers ---
  int FindNoisePeakBin(TH1D* Histogram) const;
  int FindThresholdBin(TH1D* Histogram, int PeakBin) const;
  int FindCrossoverADC(const map<int, pair<int, int>>& ADCMap, int FirstNonzero) const;

  // --- Diagnostics (temporary restore for plotting) ---
  vector<double> m_StripIndex_LV;
  vector<double> m_StripIndex_HV;

  vector<double> m_ThresholdValues_LV;
  vector<double> m_ThresholdValues_HV;
};


int main(int Argc, char** Argv)
{
  setvbuf(stdout, NULL, _IONBF, 0);
  MGlobal::Initialize("Standalone", "ThresholdFinder");

  MStripThresholdFinder Finder;

  if (!Finder.ParseCommandLine(Argc, Argv)) {
    return 1;
  }
  if (!Finder.BuildHistograms()) {
    return 1;
  }

  Finder.FindSlowThresholds();
  Finder.FindFastThresholds();

  Finder.WriteCSV();
  Finder.WriteDiagnostics();


  // -------------------------------------------------------------
  // Helpful ROOT instructions
  // -------------------------------------------------------------

  cout << endl;
  cout << "Diagnostics written to "
       << Finder.GetOutputPrefix()
       << "_diagnostics.root" << endl;
  cout << endl;

  cout << "Example ROOT commands for diagnostics:" << endl;
  cout << "---------------------------------------" << endl;

  cout << "Open file:" << endl;
  cout << "  root -l " << Finder.GetOutputPrefix() << "_diagnostics.root" << endl;
  cout << endl;

  cout << "Slow threshold diagnostic plots:" << endl;
  cout << "---------------------------------------" << endl;

  cout << "Slow threshold distribution:" << endl;
  cout << "  cSlowDist->Draw()" << endl;
  cout << endl;

  cout << endl;
  cout << "Slow threshold vs Strip:" << endl;
  cout << "  cSlowThresh->Draw()" << endl;
  cout << endl;

  cout << "Example energy spectrum with threshold:" << endl;
  cout << "  Energy_0_h_10->Draw()" << endl;
  cout << "  Energy_0_h_10->GetXaxis()->SetRangeUser(0,30)" << endl;
  cout << endl;

  cout << "Pixel threshold heat maps:" << endl;
  cout << "  cSlowPixel->Draw()" << endl;
  cout << endl;


  cout << endl;
  cout << "FAST threshold diagnostics:" << endl;
  cout << "---------------------------------------" << endl;

  cout << "Fast threshold distribution (keV):" << endl;
  cout << "  cFastDist->Draw()" << endl;
  cout << endl;

  cout << "HV and LV FAST thresholds:" << endl;
  cout << "  cFastThresh->Draw()" << endl;
  cout << endl;

  cout << "dt0 vs dt1 with fast threshold:" << endl;
  cout << "  cFast_0_l_10->Draw();" << endl;


  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MStripThresholdFinder::ParseCommandLine(int argc, char** argv)
{
  if (argc < 2) {
    cout << "Usage: ./StripEnergyThresholdFinder config.yaml" << endl;
    return false;
  }


  string configFile = argv[1];
  YAML::Node config = YAML::LoadFile(configFile);


  //int minEntries=10;
  //double fallbackThreshold=20;
  m_HistogramBins = 2048;
  m_HistogramMaxADC = 4096;
  m_NoiseSearchMaxADC = 2200;

  // -------------------------------------------------------------
  // Command line override parameters (optional)
  // -------------------------------------------------------------

  int cmd_min_entries = -1;
  double cmd_noise_search_max_ADC = -1;
  double cmd_fallback_threshold_keV = -1;

  string cmd_data_file = "";
  string cmd_calibration_file = "";
  string cmd_Strip_map = "";
  string cmd_output_prefix = "";
  m_MaxEvents = -1;

  // -------------------------------------------------------------
  // YAML file input loading
  // -------------------------------------------------------------

  if (config["analysis"]) {
    if (config["analysis"]["min_entries"]) {
      //minEntries=config["analysis"]["min_entries"].as<int>();
      m_MinEntries = config["analysis"]["min_entries"].as<int>();
    }

    if (config["analysis"]["fallback_threshold_keV"]) {
      //fallbackThreshold=config["analysis"]["fallback_threshold_keV"].as<double>();
      m_FallbackThreshold = config["analysis"]["fallback_threshold_keV"].as<double>();
    }

    if (config["analysis"]["histogram_bins"]) {
      m_HistogramBins = config["analysis"]["histogram_bins"].as<int>();
    }

    if (config["analysis"]["histogram_max_ADC"]) {
      m_HistogramMaxADC = config["analysis"]["histogram_max_ADC"].as<double>();
    }

    if (config["analysis"]["noise_search_max_ADC"]) {
      m_NoiseSearchMaxADC = config["analysis"]["noise_search_max_ADC"].as<double>();
    }
  }


  m_InputFiles = config["input"]["data_files"].as<vector<string>>();

  if (m_InputFiles.empty()) {
    cerr << "Error: No input files provided." << endl;
    return 1;
  }


  m_CalibrationFile = config["input"]["calibration_file"].as<string>().c_str();
  m_StripMapFile = config["input"]["strip_map"].as<string>().c_str();
  m_OutputPrefix = config["output"]["prefix"].as<string>().c_str();


  if (m_EnergyCalibration.ReadEnergyCalibrationFile(m_CalibrationFile) == false) {
    cout << "Failed to load calibration file: " << m_CalibrationFile << endl;
    return false;
  }


  // -------------------------------------------------------------
  // Modern command line parser using getopt_long
  // -------------------------------------------------------------

  static struct option long_options[] = {
    { "min_entries", required_argument, 0, 'm' },
    { "noise_search_max_ADC", required_argument, 0, 'n' },
    { "fallback_threshold_keV", required_argument, 0, 'f' },

    /* NEW overrides */
    { "data_file", required_argument, 0, 'd' },
    { "calibration_file", required_argument, 0, 'c' },
    { "Strip_map", required_argument, 0, 's' },
    { "output_prefix", required_argument, 0, 'o' },
    { "max_events", required_argument, 0, 'e' },

    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0 }
  };

  optind = 2; // skip program name and YAML file

  int opt;
  while ((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    switch (opt) {
    case 'm':
      cmd_min_entries = atoi(optarg);
      break;

    case 'n':
      cmd_noise_search_max_ADC = atof(optarg);
      break;

    case 'f':
      cmd_fallback_threshold_keV = atof(optarg);
      break;

    case 'd':
      m_InputFiles.push_back(optarg);
      break;

    case 'e':
      m_MaxEvents = atol(optarg);
      break;

    case 'c':
      cmd_calibration_file = optarg;
      break;

    case 's':
      cmd_Strip_map = optarg;
      break;

    case 'o':
      cmd_output_prefix = optarg;
      break;

    case 'h':
      cout << endl;
      cout << "Usage:" << endl;
      cout << "  ./StripEnergyThresholdFinder config.yaml [options]" << endl;
      cout << endl;
      cout << "Options:" << endl;
      cout << "Input/Output overrides:" << endl;
      cout << "  --data_file FILE            Override YAML data file" << endl;
      cout << "  --calibration_file FILE     Override calibration file" << endl;
      cout << "  --Strip_map FILE            Override Strip map" << endl;
      cout << "  --output_prefix NAME        Override output file prefix" << endl;
      cout << endl;
      cout << "  --min_entries N              Minimum histogram entries" << endl;
      cout << "  --noise_search_max_ADC N     Max ADC for noise search" << endl;
      cout << "  --fallback_threshold_keV N   Default threshold if fit fails" << endl;
      cout << "  --help                       Show this message" << endl;
      cout << endl;
      exit(0);

    default:
      break;
    }
  }

  if (cmd_min_entries >= 0) {
    m_MinEntries = cmd_min_entries;
  }

  if (cmd_noise_search_max_ADC >= 0) {
    m_NoiseSearchMaxADC = cmd_noise_search_max_ADC;
  }

  if (cmd_fallback_threshold_keV >= 0) {
    m_FallbackThreshold = cmd_fallback_threshold_keV;
  }


  // -------------------------------------------------------------
  // Apply command line overrides for input/output
  // -------------------------------------------------------------


  if (cmd_calibration_file != "") {
    m_CalibrationFile = cmd_calibration_file;
  }

  if (cmd_Strip_map != "") {
    m_StripMapFile = cmd_Strip_map;
  }

  if (cmd_output_prefix != "") {
    m_OutputPrefix = cmd_output_prefix;
  }

  // Determine output directory from first data file
  fs::path dataPath(m_InputFiles.back());
  fs::path outputDir = dataPath.parent_path();

  // Resolve m_OutputPrefix path
  fs::path outPath(m_OutputPrefix.Data());

  if (!outPath.is_absolute()) {
    m_OutputPrefix = (outputDir / outPath).string();
  }

  // Debug print
  cout << "Resolved output prefix: " << m_OutputPrefix << endl;


  // -------------------------------------------------------------
  // Save configuration to log file
  // -------------------------------------------------------------

  cout << endl;
  cout << "  calibration_file:        " << m_CalibrationFile.Data() << endl;
  cout << "  Strip_map:               " << m_StripMapFile << endl;
  cout << "  output_prefix:           " << m_OutputPrefix << endl;

  cout << "  data_files:" << endl;
  for (auto& f : m_InputFiles) {
    cout << "    " << f << endl;
  }

  cout << endl;
  cout << "Active analysis configuration:" << endl;
  cout << "  min_entries:            " << m_MinEntries << endl;
  cout << "  fallback_threshold_keV: " << m_FallbackThreshold << endl;
  cout << "  NoiseSearchMaxADC:      " << m_NoiseSearchMaxADC << endl;
  cout << "  histogram_bins:         " << m_HistogramBins << endl;
  cout << "  histogram_max_ADC:      " << m_HistogramMaxADC << endl;
  cout << endl;

  MString StripMapFile = m_StripMapFile.Data();

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Build per-strip ADC and timing histograms from input data
bool MStripThresholdFinder::BuildHistograms()
{
  MString StripMapFile = m_StripMapFile;
  long max_events = m_MaxEvents;

  //map<MReadOutElementDoubleStrip,TH1D*> histograms;
  map<MReadOutElementDoubleStrip, vector<int>> hist_counts;
  map<MReadOutElementDoubleStrip, vector<int>> dt0_counts;
  map<MReadOutElementDoubleStrip, vector<int>> dt1_counts;
  map<MReadOutElementDoubleStrip, double> ADC_to_keV_scale;
  map<MReadOutElementDoubleStrip, TH1D*> histograms_TAC;

  map<MReadOutElementDoubleStrip, map<int, pair<int, int>>> timingCounts;

  // -------------------------------------------------------------
  // Build ADC histograms from data
  // -------------------------------------------------------------

  MModuleLoaderMeasurementsHDF* Loader = new MModuleLoaderMeasurementsHDF();

  Loader->SetFileName(m_InputFiles[0].c_str());

  cout << "Loading file: " << m_InputFiles[0] << endl;
  cout << "Number of input files: " << m_InputFiles.size() << endl;

  Loader->SetFileNameStripMap(m_StripMapFile.Data());

  //NEW Energy Calibrator / MEGAlib module
  MSupervisor* S = MSupervisor::GetSupervisor();

  unsigned int ModuleIndex = 0;

  // Loader
  S->SetModule(Loader, ModuleIndex);
  ++ModuleIndex;

  // Energy calibration module
  MModuleEnergyCalibration* EnergyCalibrator = new MModuleEnergyCalibration();
  EnergyCalibrator->SetFileName(m_CalibrationFile);
  S->SetModule(EnergyCalibrator, ModuleIndex);
  ++ModuleIndex;


  if (!Loader->Initialize())

  {
    cerr << "Failed to initialize loader!" << endl;
    return false;
  }

  if (!EnergyCalibrator->Initialize()) {
    cerr << "Failed to initialize energy calibration!" << endl;
    return false;
  }

  MReadOutAssembly* Event = new MReadOutAssembly();

  long event_counter = 0;
  //cout << std::unitbuf;
  auto start_time = std::chrono::steady_clock::now();
  while (!Loader->IsFinished()) {

    if (max_events > 0 && event_counter >= max_events) {
      break;
    }

    Event->Clear();

    if (Loader->IsReady()) {
      //Loader->AnalyzeEvent(Event);
      Loader->AnalyzeEvent(Event);
      EnergyCalibrator->AnalyzeEvent(Event);
      event_counter++;

      // -------------------------------------------------------------
      // GLOBAL progress
      // -------------------------------------------------------------

      if (event_counter % 1000000 == 0) {
        auto now = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(now - start_time).count();

        double rate = event_counter / elapsed;

        cout << "Processed events: " << event_counter
             << " | Rate: " << rate << " events/s"
             << endl;
      }


      int NStrips = Event->GetNStripHits();

      for (int i = 0; i < NStrips; ++i) {
        MStripHit* SH = Event->GetStripHit(i);

        if (!PassHitSelection(SH)) {
          continue;
        }

        double ADC = SH->GetADCUnits();
        //double energy = SH->GetEnergy();

        MReadOutElementDoubleStrip R;
        R.SetDetectorID(SH->GetDetectorID());
        R.SetStripID(SH->GetStripID());
        R.IsLowVoltageStrip(SH->IsLowVoltageStrip());


        //it_hist->second->Fill(ADC);
        int bin = (int) (ADC / m_HistogramMaxADC * m_HistogramBins);

        if (bin >= 0 && bin < m_HistogramBins) {
          if (hist_counts.find(R) == hist_counts.end()) {
            hist_counts[R] = vector<int>(m_HistogramBins, 0);
          }

          hist_counts[R][bin]++;
        }

        // -------------------------------------------------------------
        // FAST timing accumulation (dt0 vs dt1)
        // -------------------------------------------------------------

        //int ADC_bin = static_cast<int>(ADC);
        double energy = SH->GetEnergy();

        double TAC = SH->GetTAC();
        int ADC_bin = static_cast<int>(ADC);

        // --- dt0 vs dt1 separation ---
        bool is_dt1 = (TAC > 8000); // initial threshold


        // Initialize bin if needed
        if (timingCounts[R].count(ADC_bin) == 0) {
          timingCounts[R][ADC_bin] = { 0, 0 };
        }

        double maxEnergy = m_EnergyCalibration.GetEnergy(R, m_HistogramMaxADC);

        int ebin = (int) (energy / maxEnergy * m_HistogramBins);


        if (is_dt1) {
          timingCounts[R][ADC_bin].second++;
        } else {
          timingCounts[R][ADC_bin].first++;
        }


        // -------------------------------------------------------------
        // SEPARATE: fill smooth dt0/dt1 histograms in ENERGY space
        // -------------------------------------------------------------

        auto& counts = (is_dt1 ? dt1_counts[R] : dt0_counts[R]);

        if (counts.empty()) {
          counts.resize(m_HistogramBins, 0);
        }

        if (ebin >= 0 && ebin < m_HistogramBins) {
          counts[ebin]++;
        }
      }
    }
  }
  cout << endl;

  // Save into class


  for (auto& kv : hist_counts) {
    MReadOutElementDoubleStrip R = kv.first;
    vector<int>& counts = kv.second;

    string name = "h_" + to_string(R.GetDetectorID()) + "_" + (R.IsLowVoltageStrip() ? 'l' : 'h') + "_" + to_string(R.GetStripID());

    TH1D* h = new TH1D(
      name.c_str(),
      name.c_str(),
      m_HistogramBins,
      0,
      m_HistogramMaxADC);

    for (int i = 0; i < m_HistogramBins; ++i) {
      h->SetBinContent(i + 1, counts[i]);
    }

    m_ADCHistograms[R] = h;
  }

  // -------------------------------------------------------------
  // Rebuild dt0 histograms
  // -------------------------------------------------------------
  for (auto& kv : dt0_counts) {
    MReadOutElementDoubleStrip R = kv.first;
    vector<int>& counts = kv.second;

    double maxE = m_EnergyCalibration.GetEnergy(R, m_HistogramMaxADC);
    //double maxE = 3000.0;  // keV, safe upper bound


    string name = "dt0_" + to_string(R.GetDetectorID()) + "_" + (R.IsLowVoltageStrip() ? 'l' : 'h') + "_" + to_string(R.GetStripID());

    TH1D* h = new TH1D(
      name.c_str(),
      name.c_str(),
      m_HistogramBins,
      0,
      maxE);

    for (int i = 0; i < m_HistogramBins; ++i) {
      h->SetBinContent(i + 1, counts[i]);
    }

    dt0_hists[R] = h;
  }

  // -------------------------------------------------------------
  // Rebuild dt1 histograms
  // -------------------------------------------------------------
  for (auto& kv : dt1_counts) {
    MReadOutElementDoubleStrip R = kv.first;
    vector<int>& counts = kv.second;

    double maxE = m_EnergyCalibration.GetEnergy(R, m_HistogramMaxADC);
    //double maxE = 3000.0;

    string name = "dt1_" + to_string(R.GetDetectorID()) + "_" + (R.IsLowVoltageStrip() ? 'l' : 'h') + "_" + to_string(R.GetStripID());

    TH1D* h = new TH1D(
      name.c_str(),
      name.c_str(),
      m_HistogramBins,
      0,
      maxE);

    for (int i = 0; i < m_HistogramBins; ++i) {
      h->SetBinContent(i + 1, counts[i]);
    }

    dt1_hists[R] = h;
  }

  m_TimingCounts = timingCounts;

  return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//! Determine slow thresholds from ADC histograms
void MStripThresholdFinder::FindSlowThresholds()
{

  m_StripIndex_LV.clear();
  m_StripIndex_HV.clear();
  m_ThresholdValues_LV.clear();
  m_ThresholdValues_HV.clear();


  // -------------------------------------------------------------
  // Threshold finding algorithm
  // -------------------------------------------------------------

  map<MReadOutElementDoubleStrip, double> thresholds;
  map<MReadOutElementDoubleStrip, double> thresholdsADC;

  // Progress tracking (slow thresholds)
  //int totalStrips = m_ADCHistograms.size();
  //int processedStrips = 0;

  map<int, double> thresholdLV;
  map<int, double> thresholdHV;

  map<int, double> thresholdLV_ADC;
  map<int, double> thresholdHV_ADC;


  // -------------------------------------------------------------
  // TAC threshold storage
  // -------------------------------------------------------------

  // HV/LV split

  map<int, double> thresholdLV_TAC;
  map<int, double> thresholdHV_TAC;

  map<int, double> thresholdLV_TAC_ADC;
  map<int, double> thresholdHV_TAC_ADC;


  // -------------------------------------------------------------
  // Diagnostic storage vectors
  // These vectors allow us to build ROOT diagnostic plots later
  // -------------------------------------------------------------

  vector<double> StripIndex;
  vector<double> thresholdValues;
  vector<double> noisePeakADC;

  // -------------------------------------------------------------
  // Separate vectors for HV and LV Strip diagnostics
  // -------------------------------------------------------------

  vector<double> StripIndex_LV;
  vector<double> StripIndex_HV;

  vector<double> thresholdValues_LV;
  vector<double> thresholdValues_HV;


  // -------------------------------------------------------------
  // TAC diagnostic vectors
  // -------------------------------------------------------------

  vector<double> thresholdValues_TAC_LV;
  vector<double> thresholdValues_TAC_HV;

  vector<double> TACPeakADC_LV;
  vector<double> TACPeakADC_HV;


  for (auto& kv : m_ADCHistograms) {

    MReadOutElementDoubleStrip R = kv.first;
    TH1D* hist = kv.second;

    if (hist->GetEntries() < m_MinEntries) {
      thresholds[R] = m_FallbackThreshold;
      continue;
    }

    hist->Smooth(3);

    int maxSearchBin = hist->FindBin(m_NoiseSearchMaxADC);

    int startBin = -1;

    for (int b = 1; b <= maxSearchBin; b++) {
      if (hist->GetBinContent(b) > 5) {
        startBin = b;
        break;
      }
    }

    if (startBin < 0) {
      thresholds[R] = m_FallbackThreshold;
      continue;
    }

    int peakBin = startBin;
    double peakCounts = hist->GetBinContent(startBin);

    for (int b = startBin + 1; b <= maxSearchBin; b++) {
      double c = hist->GetBinContent(b);

      if (c > peakCounts) {
        peakCounts = c;
        peakBin = b;
      } else if (b > peakBin && c < peakCounts * 0.9) {
        break;
      }
    }

    int thresholdBin = peakBin;

    if (R.GetStripID() == 64) {
      for (int b = peakBin + 1; b <= maxSearchBin; b++) {
        if (hist->GetBinContent(b) <= peakCounts * 0.5) {
          thresholdBin = b;
          break;
        }
      }
    } else {
      for (int b = peakBin + 1; b <= maxSearchBin; b++) {
        double c = hist->GetBinContent(b);

        if (c < hist->GetBinContent(thresholdBin)) {
          thresholdBin = b;
        }

        if (b > peakBin && c > peakCounts * 0.5) {
          break;
        }
      }
    }


    // -------------------------------------------------------------
    // Shift threshold slightly to the right of the noise trough
    // This prevents thresholds from sitting inside the noise tail
    // -------------------------------------------------------------

    int shiftBins = 2; // move threshold up by a couple of bins
    thresholdBin = min(thresholdBin + shiftBins, hist->GetNbinsX());

    double thresholdADC = hist->GetBinCenter(thresholdBin);

    /* Convert ADC → keV using SLOW calibration */
    double thresholdKeV =
      m_EnergyCalibration.GetEnergy(R, thresholdADC);


    if (R.IsLowVoltageStrip() == true) {
      m_StripIndex_LV.push_back(R.GetStripID());
      m_ThresholdValues_LV.push_back(thresholdKeV);
    } else {
      m_StripIndex_HV.push_back(R.GetStripID());
      m_ThresholdValues_HV.push_back(thresholdKeV);
    }


    // Store SLOW thresholds
    thresholds[R] = thresholdKeV;
    thresholdsADC[R] = thresholdADC;
  }
  cout << endl;

  // Save results into class members
  m_SlowThresholds = thresholds;
  m_SlowThresholdsADC = thresholdsADC;


  cout << "LV points: " << m_StripIndex_LV.size() << endl;
  cout << "HV points: " << m_StripIndex_HV.size() << endl;
}


void MStripThresholdFinder::FindFastThresholds()
{

  if (m_TimingCounts.empty()) {
    cout << "Warning: No timing data available for fast threshold calculation." << endl;
  }

  map<MReadOutElementDoubleStrip, double> thresholds_TAC_ADC;

  // -------------------------------------------------------------
  // Fast threshold finder (dt0 vs dt1 crossover)
  // -------------------------------------------------------------

  for (auto& kv : m_TimingCounts) {
    MReadOutElementDoubleStrip R = kv.first;
    auto& ADCMap = kv.second;

    // Skip guard ring for FAST thresholds
    if (R.GetStripID() == 64) {
      continue;
    }

    int totalCounts = 0;
    for (auto& a : ADCMap) {
      totalCounts += a.second.first + a.second.second;
    }


    // Proper m_MinEntries guard
    if (totalCounts < m_MinEntries) {
      m_FastThresholds[R] = m_FallbackThreshold;
      continue;
    }

    // Find first nonzero ADC
    int first_nonzero = -1;
    if (kv.second.empty()) {
      continue;
    }

    for (auto& a : ADCMap) {
      if (a.second.first + a.second.second > 0) {
        first_nonzero = a.first + 10;
        break;
      }
    }

    if (first_nonzero < 0) {
      m_FastThresholds[R] = m_FallbackThreshold;
      continue;
    }


    int bestADC = -1;
    int minDiff = 1e9;

    // -------------------------------------------------------------
    // Direct crossover detection (no smoothing bias)
    // -------------------------------------------------------------
    int crossoverADC = -1;

    for (auto& a : ADCMap) {
      int ADC_val = a.first;
      int n0 = a.second.first;
      int n1 = a.second.second;

      // Require minimum statistics to avoid noise triggers
      if (n0 + n1 < 10) {
        continue;
      }

      if (n1 > n0) {
        crossoverADC = ADC_val;
        break;
      }
    }

    int nbins = 21;

    // Extend search window for stability
    int searchMax = first_nonzero + 800;

    for (int ADC = first_nonzero; ADC < searchMax; ADC++) {
      int n_dt0 = 0;
      int n_dt1 = 0;

      for (int a = ADC; a < ADC + nbins; a++) {
        if (ADCMap.find(a) == ADCMap.end()) {
          continue;
        }

        n_dt0 += ADCMap[a].first;
        n_dt1 += ADCMap[a].second;
      }

      int diff;

      if (n_dt0 == 0 && n_dt1 == 0) {
        diff = 1000000;
      } else {
        diff = abs(n_dt1 - n_dt0);
      }

      if (diff < minDiff) {
        minDiff = diff;
        bestADC = ADC;
      }
    }

    if (bestADC < 0) {
      m_FastThresholds[R] = m_FallbackThreshold;
      continue;
    }


    // prefer physical crossover if available
    int fast_thresh_ADC;

    if (crossoverADC > 0) {
      fast_thresh_ADC = crossoverADC;
    } else {
      // fallback to old method
      fast_thresh_ADC = bestADC + nbins / 2;
    }

    m_FastThresholdsADC[R] = fast_thresh_ADC;

    double fast_thresh_keV =
      m_EnergyCalibration.GetEnergy(R, fast_thresh_ADC);

    m_FastThresholds[R] = fast_thresh_keV;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void MStripThresholdFinder::WriteCSV()
{

  if (m_SlowThresholds.empty() && m_FastThresholds.empty()) {
    cout << "Warning: No thresholds available to write to CSV." << endl;
  }

  // -------------------------------------------------------------
  // Write CSV threshold tables (HV and LV)
  // -------------------------------------------------------------

  ofstream csv_HV(m_OutputPrefix + "_Slow_HV_thresholds.csv");
  ofstream csv_LV(m_OutputPrefix + "_Slow_LV_thresholds.csv");

  /* CSV headers */

  csv_HV << "detector_side,Strip,threshold_ADC,threshold_keV\n";
  csv_LV << "detector_side,Strip,threshold_ADC,threshold_keV\n";

  /* Write rows */

  for (const auto& kv : m_SlowThresholds) {

    MReadOutElementDoubleStrip R = kv.first;
    int Strip = R.GetStripID();

    double thr_keV = kv.second;
    double thr_ADC = m_SlowThresholdsADC[R];


    if (R.IsLowVoltageStrip() == true) {
      csv_LV << "l,"
             << Strip << ","
             << thr_ADC << ","
             << thr_keV << "\n";
    } else {
      csv_HV << "h,"
             << Strip << ","
             << thr_ADC << ","
             << thr_keV << "\n";
    }
  }


  // -------------------------------------------------------------
  // Write CSV threshold tables (SLOW + FAST)
  // -------------------------------------------------------------

  // --- FAST CSV ---
  ofstream csv_TAC_HV(m_OutputPrefix + "_Fast_HV_thresholds.csv");
  ofstream csv_TAC_LV(m_OutputPrefix + "_Fast_LV_thresholds.csv");

  csv_TAC_HV << "detector_side,Strip,threshold_ADC,threshold_keV\n";
  csv_TAC_LV << "detector_side,Strip,threshold_ADC,threshold_keV\n";

  cout << "Writing FAST CSV entries: " << m_FastThresholds.size() << endl;

  for (const auto& kv : m_FastThresholds) {
    MReadOutElementDoubleStrip R = kv.first;
    int Strip = R.GetStripID();

    double thr_ADC = m_FastThresholdsADC[R];
    double thr_keV = kv.second;

    if (R.IsLowVoltageStrip() == true) {
      csv_TAC_LV << "l," << Strip << "," << thr_ADC << "," << thr_keV << "\n";
    } else {
      csv_TAC_HV << "h," << Strip << "," << thr_ADC << "," << thr_keV << "\n";
    }
  }


  csv_HV.close();
  csv_LV.close();
  csv_TAC_HV.close();
  csv_TAC_LV.close();
}


void MStripThresholdFinder::WriteDiagnostics()
{

  if (m_SlowThresholds.empty() && m_FastThresholds.empty()) {
    cout << "Warning: No thresholds available for diagnostics." << endl;
  }

  // -------------------------------------------------------------
  // ROOT output
  // -------------------------------------------------------------

  TFile f((m_OutputPrefix + "_diagnostics.root").Data(), "RECREATE");

  // -------------------------------------------------------------
  // SLOW threshold per Strip (HV vs LV scatter)
  // -------------------------------------------------------------
  TGraph* gSlowThresh_LV = new TGraph();
  TGraph* gSlowThresh_HV = new TGraph();

  gSlowThresh_LV->SetName("SlowThresh_LV");
  gSlowThresh_HV->SetName("SlowThresh_HV");

  // Axis titles
  gSlowThresh_LV->SetTitle("Slow Threshold per Strip;Strip;Threshold (keV)");

  // Styling
  gSlowThresh_LV->SetMarkerStyle(20); // circle
  gSlowThresh_LV->SetMarkerSize(1.0);
  gSlowThresh_LV->SetMarkerColor(kBlue);

  gSlowThresh_HV->SetMarkerStyle(20);
  gSlowThresh_HV->SetMarkerSize(1.0);
  gSlowThresh_HV->SetMarkerColor(kRed);

  // Fill graphs
  for (const auto& kv : m_SlowThresholds) {
    MReadOutElementDoubleStrip R = kv.first;
    int Strip = R.GetStripID();
    double Threshold = kv.second;


    if (R.IsLowVoltageStrip() == true) {
      gSlowThresh_LV->SetPoint(gSlowThresh_LV->GetN(), Strip, Threshold);
    } else {
      gSlowThresh_HV->SetPoint(gSlowThresh_HV->GetN(), Strip, Threshold);
    }
  }

  // -------------------------------------------------------------
  // Canvas with legend
  // -------------------------------------------------------------
  TCanvas* cSlowThresh = new TCanvas("cSlowThresh", "Slow Threshold per Strip", 800, 600);
  cSlowThresh->cd();

  // Draw LV first (sets axes)
  gSlowThresh_LV->Draw("AP");

  // Optional: auto-scale Y axis
  double ymin = 1e9, ymax = -1e9;
  for (const auto& kv : m_SlowThresholds) {
    //if (kv.first.GetStripID() == 64) continue;
    double v = kv.second;
    if (v < ymin) {
      ymin = v;
    }
    if (v > ymax) {
      ymax = v;
    }
  }
  double pad = 0.1 * (ymax - ymin);
  gSlowThresh_LV->GetYaxis()->SetRangeUser(ymin - pad, ymax + pad);

  // Overlay HV
  gSlowThresh_HV->Draw("P SAME");

  // Legend
  TLegend* legSlowScatter = new TLegend(0.7, 0.72, 0.8, 0.8);
  legSlowScatter->AddEntry(gSlowThresh_LV, "LV", "p");
  legSlowScatter->AddEntry(gSlowThresh_HV, "HV", "p");

  legSlowScatter->SetTextSize(0.02);
  legSlowScatter->SetBorderSize(1);
  legSlowScatter->SetFillStyle(0);

  legSlowScatter->Draw();

  // Finalize
  cSlowThresh->Modified();
  cSlowThresh->Update();

  // Save
  cSlowThresh->Write();
  gSlowThresh_LV->Write();
  gSlowThresh_HV->Write();

  // -------------------------------------------------------------
  // SLOW threshold distribution (HV vs LV separated)
  // -------------------------------------------------------------
  TH1D* hSlowDist_LV = new TH1D(
    "SlowThresholdDistribution_LV",
    "Slow Threshold Distribution;Threshold (keV);Counts",
    100, 0, 50);

  TH1D* hSlowDist_HV = new TH1D(
    "SlowThresholdDistribution_HV",
    "Slow Threshold Distribution;Threshold (keV);Counts",
    100, 0, 50);

  // Styling
  hSlowDist_LV->SetLineColor(kBlue);
  hSlowDist_LV->SetLineWidth(2);

  hSlowDist_HV->SetLineColor(kRed);
  hSlowDist_HV->SetLineWidth(2);

  // Fill histograms
  for (const auto& kv : m_SlowThresholds) {
    MReadOutElementDoubleStrip R = kv.first;
    double Threshold = kv.second;

    //if(Strip == 64) continue;

    if (R.IsLowVoltageStrip() == true) {
      hSlowDist_LV->Fill(Threshold);
    } else {
      hSlowDist_HV->Fill(Threshold);
    }
  }

  // -------------------------------------------------------------
  // Canvas with legend
  // -------------------------------------------------------------
  TCanvas* cSlowDist = new TCanvas("cSlowDist", "Slow Threshold Distribution", 800, 600);
  cSlowDist->cd();

  // Draw LV first
  hSlowDist_LV->Draw("HIST");

  // Overlay HV
  hSlowDist_HV->Draw("HIST SAME");

  // Legend
  TLegend* legSlow = new TLegend(0.7, 0.72, 0.8, 0.8);
  legSlow->AddEntry(hSlowDist_LV, "LV", "l");
  legSlow->AddEntry(hSlowDist_HV, "HV", "l");
  legSlow->Draw();

  // Finalize
  cSlowDist->Modified();
  cSlowDist->Update();

  // Write everything
  cSlowDist->Write();
  hSlowDist_LV->Write();
  hSlowDist_HV->Write();

  // -------------------------------------------------------------
  // FAST threshold per Strip histogram
  // -------------------------------------------------------------
  TH1D* hFastThreshPerStrip = new TH1D(
    "FastThresholds_per_Strip",
    "Fast Threshold per Strip;Strip;Threshold (keV)",
    65, 0, 65);

  for (const auto& kv : m_FastThresholds) {
    MReadOutElementDoubleStrip R = kv.first;
    int Strip = R.GetStripID();
    double Threshold_keV = kv.second;

    if (Strip == 64) {
      continue; // skip guard ring
    }

    hFastThreshPerStrip->SetBinContent(Strip + 1, Threshold_keV);
  }

  hFastThreshPerStrip->Write();

  // -------------------------------------------------------------
  // Threshold vs Strip for HV and LV sides
  // -------------------------------------------------------------

  if (!m_StripIndex_LV.empty()) {
    TGraph Threshold_vs_Strip_LV(
      m_StripIndex_LV.size(),
      m_StripIndex_LV.data(),
      m_ThresholdValues_LV.data());

    Threshold_vs_Strip_LV.SetName("Threshold_vs_Strip_LV");
    Threshold_vs_Strip_LV.SetTitle("Threshold vs Strip (LV)");
    Threshold_vs_Strip_LV.SetMarkerStyle(20);
    Threshold_vs_Strip_LV.SetMarkerColor(kBlue);

    Threshold_vs_Strip_LV.Write();
  }

  if (!m_StripIndex_HV.empty()) {
    TGraph Threshold_vs_Strip_HV(
      m_StripIndex_HV.size(),
      m_StripIndex_HV.data(),
      m_ThresholdValues_HV.data());

    Threshold_vs_Strip_HV.SetName("Threshold_vs_Strip_HV");
    Threshold_vs_Strip_HV.SetTitle("Threshold vs Strip (HV)");
    Threshold_vs_Strip_HV.SetMarkerStyle(20);
    Threshold_vs_Strip_HV.SetMarkerColor(kRed);

    Threshold_vs_Strip_HV.Write();
  }

  for (auto& kv : dt0_hists) {
    const MReadOutElementDoubleStrip& R = kv.first;

    if (dt1_hists.find(R) == dt1_hists.end()) {
      continue;
    }

    TH1D* dt0 = kv.second;
    TH1D* dt1 = dt1_hists[R];

    // -------------------------------
    // Create canvas
    // -------------------------------
    string cname = "cFast_" + to_string(R.GetDetectorID()) + "_" + (R.IsLowVoltageStrip() ? 'l' : 'h') + "_" + to_string(R.GetStripID());
    TCanvas* c = new TCanvas(cname.c_str(), cname.c_str(), 800, 600);

    // -------------------------------
    // Styling
    // -------------------------------
    dt0->SetLineColor(kBlack);
    dt1->SetLineColor(kBlue);

    dt0->SetTitle(Form(
      "Fast Shaper Threshold (det=%d %c Strip=%d);Energy (keV);Counts",
      R.GetDetectorID(), R.IsLowVoltageStrip() ? 'l' : 'h', R.GetStripID()));


    // -------------------------------
    // Draw both
    // -------------------------------
    dt0->Draw("HIST");
    dt1->Draw("HIST SAME");
    dt0->GetXaxis()->SetRangeUser(0, 100);

    gPad->Update();


    TLegend* leg = new TLegend(0.65, 0.7, 0.88, 0.88);
    leg->AddEntry(dt0, "dt0", "l");
    leg->AddEntry(dt1, "dt1", "l");

    // -------------------------------
    // Threshold line (NOW IN keV)
    // -------------------------------
    if (m_FastThresholds.count(R) == 1) {

      double Threshold_keV = m_FastThresholds[R];

      double ymin = gPad->GetUymin();
      double ymax = gPad->GetUymax();

      TLine* line = new TLine(Threshold_keV, ymin, Threshold_keV, ymax);
      line->SetLineColor(kRed);
      line->SetLineWidth(2);
      line->Draw("SAME");

      // Legend entry (fix from earlier)
      leg->AddEntry(line, "Fast Thresholdeshold", "l");
    }

    //leg->AddEntry((TObject*)0, "Fast Threshold", ""); // label only
    leg->Draw();

    // Save
    c->Write();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////

  // -------------------------------------------------------------
  // Write ADC spectra and create energy spectra with thresholds
  // -------------------------------------------------------------

  for (auto& kv : m_ADCHistograms) {
    MReadOutElementDoubleStrip R = kv.first;
    TH1D* ADCHist = kv.second;

    ADCHist->Write();

    //string name="Slow Threshold Det "+to_string(key.det)+", Side"+key.side+", Strip"+to_string(key.Strip);
    string name = "Energy_" + to_string(R.GetDetectorID()) + "_" + (R.IsLowVoltageStrip() ? 'l' : 'h') + "_" + to_string(R.GetStripID());

    double maxE = m_EnergyCalibration.GetEnergy(R, m_HistogramMaxADC);
    //double maxE = 3000.0;

    TH1D* energyHist = new TH1D(
      name.c_str(),
      name.c_str(),
      ADCHist->GetNbinsX(),
      0,
      maxE);

    energyHist->SetTitle(Form(
      "Slow Threshold (det=%d side=%c Strip=%d);Energy (keV);Counts",
      R.GetDetectorID(), R.IsLowVoltageStrip() ? 'l' : 'h', R.GetStripID()));

    energyHist->GetXaxis()->SetRangeUser(0, 100);

    for (int b = 1; b <= ADCHist->GetNbinsX(); b++) {
      double ADC = ADCHist->GetBinCenter(b);
      double energy = m_EnergyCalibration.GetEnergy(R, ADC);
      //double energy = adc;   // TEMP: keep histogram consistent

      double counts = ADCHist->GetBinContent(b);

      int ebin = energyHist->FindBin(energy);
      energyHist->AddBinContent(ebin, counts);
    }

    double Threshold = m_SlowThresholds[R];

    TLine* line = new TLine(Threshold, 0, Threshold, energyHist->GetMaximum());
    line->SetLineColor(kRed);
    line->SetLineWidth(2);

    energyHist->GetListOfFunctions()->Add(line);

    energyHist->Write();
  }


  // -------------------------------------------------------------
  // FAST threshold distribution (HV vs LV separated)
  // -------------------------------------------------------------
  TH1D* hFastDist_LV = new TH1D(
    "FastThresholdDistribution_LV",
    "Fast Threshold Distribution;Threshold (keV);Counts",
    100, 0, 100);

  TH1D* hFastDist_HV = new TH1D(
    "FastThresholdDistribution_HV",
    "Fast Threshold Distribution;Threshold (keV);Counts",
    100, 0, 100);

  // Styling
  hFastDist_LV->SetLineColor(kBlue);
  hFastDist_LV->SetLineWidth(2);

  hFastDist_HV->SetLineColor(kRed);
  hFastDist_HV->SetLineWidth(2);

  // Fill histograms
  for (const auto& kv : m_FastThresholds) {
    MReadOutElementDoubleStrip R = kv.first;
    double Threshold = kv.second;

    if (R.GetStripID() == 64) {
      continue; // The GR has no fast threshold
    }

    if (R.IsLowVoltageStrip() == true) {
      hFastDist_LV->Fill(Threshold);
    } else {
      hFastDist_HV->Fill(Threshold);
    }
  }

  // -------------------------------------------------------------
  // Canvas with legend
  // -------------------------------------------------------------
  TCanvas* cFastDist = new TCanvas("cFastDist", "Fast Threshold Distribution", 800, 600);
  cFastDist->cd();

  // Draw LV first
  hFastDist_LV->Draw("HIST");

  // Overlay HV
  hFastDist_HV->Draw("HIST SAME");

  // Legend
  TLegend* leg2 = new TLegend(0.70, 0.72, 0.8, 0.8);
  leg2->AddEntry(hFastDist_LV, "LV", "l");
  leg2->AddEntry(hFastDist_HV, "HV", "l");
  leg2->Draw();

  // Finalize
  cFastDist->Modified();
  cFastDist->Update();

  // Write everything
  cFastDist->Write();
  hFastDist_LV->Write();
  hFastDist_HV->Write();


  // Distribution in ADC
  TH1D* FastThresholdDistribution_ADC = new TH1D(
    "FastThresholdDistribution_ADC",
    "Fast Threshold Distribution (ADC);ADC;Counts",
    200, 0, 2000);

  for (const auto& kv : m_FastThresholdsADC) {
    FastThresholdDistribution_ADC->Fill(kv.second);
  }

  FastThresholdDistribution_ADC->Write();


  // -------------------------------------------------------------
  // FAST threshold per Strip (use TGraph for proper axes)
  // -------------------------------------------------------------
  TGraph* gFastThresh_LV = new TGraph();
  TGraph* gFastThresh_HV = new TGraph();

  gFastThresh_LV->SetName("FastThresh_LV");
  gFastThresh_HV->SetName("FastThresh_HV");

  // Axis titles
  gFastThresh_LV->SetTitle("Fast Threshold per Strip;Strip;Threshold (keV)");

  // Styling
  gFastThresh_LV->SetMarkerStyle(20); // circle
  gFastThresh_LV->SetMarkerSize(1.0);
  gFastThresh_LV->SetMarkerColor(kBlue);

  gFastThresh_HV->SetMarkerStyle(20);
  gFastThresh_HV->SetMarkerSize(1.0);
  gFastThresh_HV->SetMarkerColor(kRed);

  // Fill graphs
  for (const auto& kv : m_FastThresholds) {
    MReadOutElementDoubleStrip R = kv.first;
    double Threshold = kv.second;

    if (R.GetStripID() == 64) {
      continue; // No fast GR threshold
    }

    if (R.IsLowVoltageStrip() == true) {
      gFastThresh_LV->SetPoint(gFastThresh_LV->GetN(), R.GetStripID(), Threshold);
    } else {
      gFastThresh_HV->SetPoint(gFastThresh_HV->GetN(), R.GetStripID(), Threshold);
    }
  }

  // Make sure ROOT file is the active directory
  f.cd();

  // -------------------------------------------------------------
  // Canvas with legend (FIXED VERSION)
  // -------------------------------------------------------------
  TCanvas* cFast = new TCanvas("cFastThresh", "Fast Threshold per Strip", 800, 600);
  cFast->cd();

  // Draw LV first
  gFastThresh_LV->Draw("AP");

  // Force axis AFTER draw
  gFastThresh_LV->GetYaxis()->SetRangeUser(0, 60);

  // Draw HV
  gFastThresh_HV->Draw("P SAME");

  // Create legend AFTER graphs are drawn
  TLegend* leg = new TLegend(0.7, 0.72, 0.8, 0.8);
  leg->AddEntry(gFastThresh_LV, "LV", "p");
  leg->AddEntry(gFastThresh_HV, "HV", "p");

  // Make legend clearly visible
  leg->SetBorderSize(1);
  leg->SetFillColor(0);
  leg->SetTextSize(0.025);

  leg->Draw();
  cFast->GetListOfPrimitives()->Add(leg);

  // force rendering
  cFast->Modified();
  cFast->Update();

  // Write AFTER everything is finalized
  cFast->Write();

  gFastThresh_LV->Write();
  gFastThresh_HV->Write();


  // -------------------------------------------------------------
  // SLOW threshold pixel map (LV vs HV)
  // -------------------------------------------------------------
  TH2D* hSlowPixelMap = new TH2D(
    "SlowThresholdPixelMap",
    "Slow Threshold Pixel Map;LV Strip;HV Strip;Threshold Value [keV]",
    64, -0.5, 63.5, // LV: 0–63
    61, -0.5, 63.5 // HV: 0–60
  );

  // Temporary storage
  map<int, double> slowLV;
  map<int, double> slowHV;

  // Separate m_SlowThresholds by side
  for (const auto& kv : m_SlowThresholds) {
    MReadOutElementDoubleStrip R = kv.first;
    int Strip = R.GetStripID();

    double Threshold = kv.second;

    if (Strip == 64) {
      continue; // skip guard ring
    }

    if (R.IsLowVoltageStrip() == true) {
      slowLV[Strip] = Threshold;
    } else {
      slowHV[Strip] = Threshold;
    }
  }

  // Fill pixel map (HV vs LV)
  for (const auto& lv : slowLV) {
    int lv_Strip = lv.first;
    double lv_Threshold = lv.second;

    for (const auto& hv : slowHV) {
      int hv_Strip = hv.first;
      double hv_Threshold = hv.second;

      //double value = 0.5 * (lv_thr + hv_thr);  // average
      double value = std::max(lv_Threshold, hv_Threshold); // select the higher threshold value

      //hSlowPixelMap->Fill(lv_Strip, hv_Strip, value);
      int binX = hSlowPixelMap->GetXaxis()->FindBin(lv_Strip);
      int binY = hSlowPixelMap->GetYaxis()->FindBin(hv_Strip);

      hSlowPixelMap->SetBinContent(binX, binY, value);
    }
  }

  // -------------------------------------------------------------
  // Draw heatmap
  // -------------------------------------------------------------
  TCanvas* cSlowPixel = new TCanvas("cSlowPixel", "Slow Threshold Pixel Map", 800, 700);
  cSlowPixel->cd();

  gStyle->SetPalette(112); // safe Viridis

  hSlowPixelMap->SetStats(0);

  // Flip Y-axis so 0 is at top (your requirement)
  hSlowPixelMap->GetYaxis()->SetRangeUser(60.5, -0.5);

  hSlowPixelMap->Draw("COLZ");

  cSlowPixel->Write();
  hSlowPixelMap->Write();


  //------------------------------------------------------------
  // New Plots
  //------------------------------------------------------------


  // Plot Slow Thresholds per Strip

  TGraph* gSlowLV = new TGraph();
  TGraph* gSlowHV = new TGraph();

  for (const auto& kv : m_SlowThresholds) {
    MReadOutElementDoubleStrip R = kv.first;
    double Threshold = kv.second;

    //if (R.GetStripID() == 64) continue; // skip guard ring if needed

    if (R.IsLowVoltageStrip() == true) {
      gSlowLV->SetPoint(gSlowLV->GetN(), R.GetStripID(), Threshold);
    } else {
      gSlowHV->SetPoint(gSlowHV->GetN(), R.GetStripID(), Threshold);
    }
  }

  // Style (reuse your preferences here)
  gSlowLV->SetTitle("Slow Threshold vs Strip (LV);Strip;Threshold [keV]");
  gSlowHV->SetTitle("Slow Threshold vs Strip (HV);Strip;Threshold [keV]");

  TCanvas* cSlowStrip = new TCanvas("cSlowStrip", "Slow Threshold vs Strip", 1200, 500);
  cSlowStrip->Divide(2, 1);

  cSlowStrip->cd(1);
  gSlowLV->Draw("AP");

  cSlowStrip->cd(2);
  gSlowHV->Draw("AP");

  cSlowStrip->Write();

  // PLot Fast Thresholds per Strip

  TGraph* gFastLV = new TGraph();
  TGraph* gFastHV = new TGraph();

  for (const auto& kv : m_FastThresholds) {
    MReadOutElementDoubleStrip R = kv.first;
    int Strip = R.GetStripID();
    double Threshold = kv.second;

    if (Strip == 64) {
      continue;
    }

    if (R.IsLowVoltageStrip() == true) {
      gFastLV->SetPoint(gFastLV->GetN(), Strip, Threshold);
    } else {
      gFastHV->SetPoint(gFastHV->GetN(), Strip, Threshold);
    }
  }

  TCanvas* cFastStrip = new TCanvas("cFastStrip", "Fast Threshold vs Strip", 1200, 500);
  cFastStrip->Divide(2, 1);

  cFastStrip->cd(1);
  gFastLV->Draw("AP");

  cFastStrip->cd(2);
  gFastHV->Draw("AP");

  cFastStrip->Write();

  f.Close();
}
