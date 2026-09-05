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

// This app uses an XML configuration file, with optional
// command-line arguments to override selected settings.

// Run command example
// $MEGALIB/bin/StripEnergyThresholdFinder /path/to/your/xml/config/file.xml

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

/* MEGAlib XML */
#include "MXmlDocument.h"
#include "MXmlNode.h"

/* MEGAlib */
#include "MGlobal.h"
#include "MSupervisor.h"
#include "MModuleEnergyCalibration.h"
#include "MModuleLoaderMeasurementsHDF.h"
#include "MReadOutAssembly.h"
#include "MReadOutElementDoubleStrip.h"
#include "MStripHit.h"
#include "MString.h"


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

  MString GetOutputPrefix() const { return m_OutputPrefix; }

 private:
  // --- Configuration ---
  MModuleEnergyCalibration m_EnergyCalibration;
  vector<string> m_InputFileNames;
  MString m_CalibrationFileName;
  MString m_StripMapFileName;
  MString m_OutputPrefix;

  unsigned int m_DetectorID = 0;
  int m_MinEntries = 10;
  double m_FallbackThresholdKeV = 20.0;
  double m_FastFallbackThresholdKeV = 35.0;
  int m_HistogramBins = 512;
  double m_HistogramMaxADC = 4096.0;
  double m_NoiseSearchMaxKeV = 40.0;
  double m_NoiseStartMinCounts = 50.0;

  long m_MaxEvents = -1;

  // --- Data ---
  map<MReadOutElementDoubleStrip, TH1D*> m_ADCHistograms;
  map<MReadOutElementDoubleStrip, map<int, pair<int, int>>> m_TimingCounts;

  map<MReadOutElementDoubleStrip, TH1D*> dt0_hists;
  map<MReadOutElementDoubleStrip, TH1D*> dt1_hists;

  // --- Results ---
  // Slow software thresholds
  map<MReadOutElementDoubleStrip, double> m_SlowThresholds;
  map<MReadOutElementDoubleStrip, double> m_SlowThresholdsADC;

  // Slow hardware thresholds
  map<MReadOutElementDoubleStrip, double> m_SlowHardwareThresholds;
  map<MReadOutElementDoubleStrip, double> m_SlowHardwareThresholdsADC;

  // Fast thresholds
  map<MReadOutElementDoubleStrip, double> m_FastThresholds;
  map<MReadOutElementDoubleStrip, double> m_FastThresholdsADC;

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

  // Set verbosity to c_Error to display errors in the nuclearizer modules
  //g_Verbosity = c_Error;

  MStripThresholdFinder Finder;

  if (Finder.ParseCommandLine(Argc, Argv) == false) {
    return 1;
  }
  if (Finder.BuildHistograms() == false) {
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

  cout << "Example energy spectrum with software and hardware thresholds:" << endl;
  cout << "Format = Energy_DetectorID_Side(h/l)_StripID:" << endl;
  cout << "  Energy_0_h_10->Draw()" << endl;
  cout << endl;

  cout << "Pixel threshold heat maps:" << endl;
  cout << "  cSlowPixel->Draw()" << endl;
  cout << endl;

  cout << endl;
  cout << "SLOW HARDWARE threshold diagnostics:" << endl;
  cout << "---------------------------------------" << endl;
  cout << endl;

  cout << "Slow hardware threshold distribution:" << endl;
  cout << "  cSlowHardwareDist->Draw()" << endl;
  cout << endl;

  cout << "Slow hardware threshold vs Strip:" << endl;
  cout << "  cSlowHardwareThresh->Draw()" << endl;
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
  cout << "  cFast_0_l_10->Draw()" << endl;


  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

bool MStripThresholdFinder::ParseCommandLine(int argc, char** argv)
{
  if (argc < 2) {
    cout << "Usage: StripEnergyThresholdFinder config.xml" << endl;
    return false;
  }

  // -------------------------------------------------------------
  // Default analysis parameters
  // -------------------------------------------------------------

  m_HistogramBins = 2048;
  m_HistogramMaxADC = 4096;
  m_NoiseSearchMaxKeV = 40.0;

  // -------------------------------------------------------------
  // Command line override parameters (optional)
  // -------------------------------------------------------------

  int cmd_min_entries = -1;
  double cmd_noise_search_max_keV = -1;
  double cmd_fallback_threshold_keV = -1;
  double cmd_fast_fallback_threshold_keV = -1;

  string cmd_calibration_file = "";
  string cmd_strip_map = "";
  string cmd_output_prefix = "";

  m_MaxEvents = -1;

  // -------------------------------------------------------------
  // Load XML configuration
  // -------------------------------------------------------------

  MString configFile = argv[1];

  MXmlDocument* Document = new MXmlDocument();

  if (Document->Load(configFile) == false) {
    cerr << "Error: Failed to load XML configuration file: "
         << configFile << endl;
    delete Document;
    return false;
  }

  // -------------------------------------------------------------
  // Read analysis configuration
  // -------------------------------------------------------------

  MXmlNode* AnalysisNode = Document->GetNode("Analysis");

  if (AnalysisNode != nullptr) {

    MXmlNode* Node = nullptr;

    Node = AnalysisNode->GetNode("MinEntries");
    if (Node != nullptr) {
      m_MinEntries = Node->GetValueAsInt();
    }

    Node = AnalysisNode->GetNode("FallbackThresholdKeV");
    if (Node != nullptr) {
      m_FallbackThresholdKeV = Node->GetValueAsDouble();
    }

	  Node = AnalysisNode->GetNode("FastFallbackKeV");
    if (Node != nullptr) {
      m_FastFallbackThresholdKeV = Node->GetValueAsDouble();
    }

    Node = AnalysisNode->GetNode("NoiseSearchMaxKeV");
    if (Node != nullptr) {
      m_NoiseSearchMaxKeV = Node->GetValueAsDouble();
    }
  }

  // -------------------------------------------------------------
  // Read input configuration
  // -------------------------------------------------------------

  MXmlNode* InputNode = Document->GetNode("Input");

  if (InputNode == nullptr) {
    cerr << "Error: Missing <Input> section in XML configuration." << endl;
    delete Document;
    return false;
  }

  // Read one or more input data files
  MXmlNode* DataFilesNode = InputNode->GetNode("DataFiles");

  if (DataFilesNode != nullptr) {

    for (unsigned int i = 0; i < DataFilesNode->GetNNodes(); ++i) {

      MXmlNode* DataFileNode = DataFilesNode->GetNode(i);

      if (DataFileNode != nullptr &&
          DataFileNode->GetName() == "DataFile") {
        m_InputFileNames.push_back(DataFileNode->GetValue().Data());
      }
    }
  }

  if (m_InputFileNames.empty() == true) {
    cerr << "Error: No input files provided." << endl;
    delete Document;
    return false;
  }

  // Energy calibration file
  MXmlNode* CalibrationFileNode =
    InputNode->GetNode("CalibrationFile");

  if (CalibrationFileNode != nullptr) {
    m_CalibrationFileName = CalibrationFileNode->GetValue();
  }



  // Strip map
  MXmlNode* StripMapNode =
    InputNode->GetNode("StripMap");

  if (StripMapNode != nullptr) {
    m_StripMapFileName = StripMapNode->GetValue();
  }

  // -------------------------------------------------------------
  // Read output configuration
  // -------------------------------------------------------------

  MXmlNode* OutputNode = Document->GetNode("Output");

  if (OutputNode != nullptr) {

    MXmlNode* PrefixNode = OutputNode->GetNode("Prefix");

    if (PrefixNode != nullptr) {
      m_OutputPrefix = PrefixNode->GetValue();
    }
  }

  // We have copied everything we need from the XML tree
  delete Document;
  Document = nullptr;

  // -------------------------------------------------------------
  // Command line parser using getopt_long
  // -------------------------------------------------------------

  static struct option long_options[] = {
    { "min_entries", required_argument, 0, 'm' },
    { "noise_search_max_keV", required_argument, 0, 'n' },
    { "fallback_threshold_keV", required_argument, 0, 'f' },
    { "fast_fallback_threshold_keV", required_argument, 0, 'F' },

    { "data_file", required_argument, 0, 'd' },
    { "calibration_file", required_argument, 0, 'c' },
    { "strip_map", required_argument, 0, 's' },
    { "output_prefix", required_argument, 0, 'o' },
    { "max_events", required_argument, 0, 'e' },

    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0 }
  };

  optind = 2; // skip program name and XML file

  int opt;

  while ((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1) {

    switch (opt) {

    case 'm':
      cmd_min_entries = atoi(optarg);
      break;

    case 'n':
      cmd_noise_search_max_keV = atof(optarg);
      break;

    case 'f':
      cmd_fallback_threshold_keV = atof(optarg);
      break;

    case 'F':
      cmd_fast_fallback_threshold_keV = atof(optarg);
      break;

    case 'd':
      m_InputFileNames.push_back(optarg);
      break;

    case 'e':
      m_MaxEvents = atol(optarg);
      break;

    case 'c':
      cmd_calibration_file = optarg;
      break;

    case 's':
      cmd_strip_map = optarg;
      break;

    case 'o':
      cmd_output_prefix = optarg;
      break;

    case 'h':
      cout << endl;
      cout << "Usage:" << endl;
      cout << "  StripEnergyThresholdFinder config.xml [options]" << endl;
      cout << endl;

      cout << "Options:" << endl;

      cout << "Input/Output overrides:" << endl;
      cout << "  --data_file FILE                 Add input data file" << endl;
      cout << "  --calibration_file FILE          Override calibration file" << endl;
      cout << "  --strip_map FILE                 Override Strip map" << endl;
      cout << "  --output_prefix NAME             Override output file prefix" << endl;
      cout << endl;

      cout << "  --min_entries N                  Minimum histogram entries" << endl;
      cout << "  --noise_search_max_keV N         Max energy for noise search (keV)" << endl;
      cout << "  --fallback_threshold_keV N       Default threshold if fit fails" << endl;
      cout << "  --fast_fallback_threshold_keV N  Fast fallback threshold (keV)" << endl;
      cout << "  --max_events N                   Maximum number of events to process" << endl;
      cout << "  --help                           Show this message" << endl;
      cout << endl;

      exit(0);

    default:
      break;
    }
  }

  // -------------------------------------------------------------
  // Apply command line analysis overrides
  // -------------------------------------------------------------

  if (cmd_min_entries >= 0) {
    m_MinEntries = cmd_min_entries;
  }

  if (cmd_noise_search_max_keV >= 0) {
    m_NoiseSearchMaxKeV = cmd_noise_search_max_keV;
  }

  if (cmd_fallback_threshold_keV >= 0) {
    m_FallbackThresholdKeV = cmd_fallback_threshold_keV;
  }

  if (cmd_fast_fallback_threshold_keV >= 0) {
    m_FastFallbackThresholdKeV = cmd_fast_fallback_threshold_keV;
  }

  // -------------------------------------------------------------
  // Apply command line input/output overrides
  // -------------------------------------------------------------

  if (cmd_calibration_file != "") {
    m_CalibrationFileName = cmd_calibration_file;
  }

  if (cmd_strip_map != "") {
    m_StripMapFileName = cmd_strip_map;
  }

  if (cmd_output_prefix != "") {
    m_OutputPrefix = cmd_output_prefix;
  }

  // -------------------------------------------------------------
  // Load energy calibration
  //
  // We do this after command line overrides so that an overridden
  // calibration filename is actually the calibration we load.
  // -------------------------------------------------------------

  if (m_EnergyCalibration.ReadEnergyCalibrationFile(m_CalibrationFileName) == false) {
    cout << "Failed to load calibration file: "
         << m_CalibrationFileName << endl;
    return false;
  }

  // -------------------------------------------------------------
  // Determine output directory from first data file
  // -------------------------------------------------------------

  fs::path dataPath(m_InputFileNames.back());
  fs::path outputDir = dataPath.parent_path();

  fs::path outPath(m_OutputPrefix.Data());

  if (outPath.is_absolute() == false) {
    m_OutputPrefix = (outputDir / outPath).string();
  }

  cout << "Resolved output prefix: "
       << m_OutputPrefix << endl;

  // -------------------------------------------------------------
  // Print active configuration
  // -------------------------------------------------------------

  cout << endl;

  cout << "  calibration_file:        "
       << m_CalibrationFileName.Data() << endl;

  cout << "  strip_map:               "
       << m_StripMapFileName << endl;

  cout << "  output_prefix:           "
       << m_OutputPrefix << endl;

  cout << "  data_files:" << endl;

  for (auto& f : m_InputFileNames) {
    cout << "    " << f << endl;
  }

  cout << endl;
  cout << "Active analysis configuration:" << endl;

  cout << "  min_entries:              "
       << m_MinEntries << endl;

  cout << "  fallback_threshold_keV:   "
       << m_FallbackThresholdKeV << endl;

  cout << "  fast_fallback_threshold_keV: "
       << m_FastFallbackThresholdKeV << endl;

  cout << "  noise_search_max_keV:     "
       << m_NoiseSearchMaxKeV << endl;

  cout << "  histogram_bins:           "
       << m_HistogramBins << endl;

  cout << "  histogram_max_ADC:        "
       << m_HistogramMaxADC << endl;

  cout << endl;

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Build per-strip ADC and timing histograms from input data
bool MStripThresholdFinder::BuildHistograms()
{
  long max_events = m_MaxEvents;

  map<MReadOutElementDoubleStrip, vector<int>> hist_counts;
  map<MReadOutElementDoubleStrip, vector<int>> dt0_counts;
  map<MReadOutElementDoubleStrip, vector<int>> dt1_counts;
  map<MReadOutElementDoubleStrip, double> ADC_to_keV_scale;


  map<MReadOutElementDoubleStrip, map<int, pair<int, int>>> timingCounts;

  // -------------------------------------------------------------
  // Build ADC histograms from data
  // -------------------------------------------------------------

  MModuleLoaderMeasurementsHDF* Loader = new MModuleLoaderMeasurementsHDF();

  Loader->SetFileName(m_InputFileNames[0].c_str());

  cout << "Loading file: " << m_InputFileNames[0] << endl;
  cout << "Number of input files: " << m_InputFileNames.size() << endl;

  Loader->SetFileNameStripMap(m_StripMapFileName.Data());
  Loader->SetIncludeNearestNeighbor(false);

  //NEW Energy Calibrator / MEGAlib module
  MSupervisor* S = MSupervisor::GetSupervisor();

  unsigned int ModuleIndex = 0;

  // Loader
  S->SetModule(Loader, ModuleIndex);
  ++ModuleIndex;

  // Energy calibration module
  MModuleEnergyCalibration* EnergyCalibrator = new MModuleEnergyCalibration();
  EnergyCalibrator->SetFileName(m_CalibrationFileName);
  S->SetModule(EnergyCalibrator, ModuleIndex);
  ++ModuleIndex;


  if (Loader->Initialize() == false) {
    cerr << "Failed to initialize loader!" << endl;
    return false;
  }

  if (EnergyCalibrator->Initialize() == false) {
    cerr << "Failed to initialize energy calibration!" << endl;
    return false;
  }

  MReadOutAssembly* Event = new MReadOutAssembly();

  long event_counter = 0;
  auto start_time = std::chrono::steady_clock::now();
  while (Loader->IsFinished() == false) {

    if (max_events > 0 && event_counter >= max_events) {
      break;
    }

    Event->Clear();

    if (Loader->IsReady() == true) {
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

	    // -------------------------------------------------------------
      // TEMPORARY DIAGNOSTIC:
      // Identify and print events containing unexpected detector IDs.
      // Uncomment this block when investigating suspicious events.
      // -------------------------------------------------------------

      /*
      bool suspiciousEvent = false;

      for (int j = 0; j < NStrips; ++j) {
        MStripHit* TestHit = Event->GetStripHit(j);

        if (TestHit->GetDetectorID() != 0) {
          suspiciousEvent = true;
          break;
        }
      }

      if (suspiciousEvent) {

        cout << endl;
        cout << "========================================" << endl;
        cout << "SUSPICIOUS EVENT " << event_counter
             << "   NStrips=" << NStrips << endl;

        for (int j = 0; j < NStrips; ++j) {

          MStripHit* TestHit = Event->GetStripHit(j);

          cout << "  Hit " << j
               << " Det=" << TestHit->GetDetectorID()
               << " Side=" << (TestHit->IsLowVoltageStrip() ? "LV" : "HV")
               << " Strip=" << TestHit->GetStripID()
               << " ADC=" << TestHit->GetADCUnits()
               << " Energy=" << TestHit->GetEnergy()
               << " TAC=" << TestHit->GetTAC()
               << endl;
        }

        cout << "========================================" << endl;
      }
      */


      for (int i = 0; i < NStrips; ++i) {
        MStripHit* SH = Event->GetStripHit(i);

        if (SH == nullptr) {
          continue;
        }

        // Process only the detector selected in the configuration.
        if (SH->GetDetectorID() != m_DetectorID) {
          continue;
        }

        double ADC = SH->GetADCUnits();

        MReadOutElementDoubleStrip R;
        R.SetDetectorID(SH->GetDetectorID());
        R.SetStripID(SH->GetStripID());
        R.IsLowVoltageStrip(SH->IsLowVoltageStrip());

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

        double energy = SH->GetEnergy();

        double TAC = SH->GetTAC();
        int ADC_bin = static_cast<int>(ADC);

        // --- dt0 vs dt1 separation ---
        bool is_dt1 = (TAC > 8000); // initial threshold


        // Initialize bin if needed
        if (timingCounts[R].count(ADC_bin) == 0) {
          timingCounts[R][ADC_bin] = { 0, 0 };
        }

        // Cache maximum calibrated energy once per strip
        if (ADC_to_keV_scale.find(R) == ADC_to_keV_scale.end()) {

          // cout << "CACHE calibration request: Det "
          //      << R.GetDetectorID()
          //      << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
          //      << " Strip " << R.GetStripID()
          //      << endl;

          ADC_to_keV_scale[R] = m_EnergyCalibration.GetEnergy(R, m_HistogramMaxADC);
        }

        double maxEnergy = ADC_to_keV_scale[R];

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

        if (counts.empty() == true) {
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

    //double maxE = m_EnergyCalibration.GetEnergy(R, m_HistogramMaxADC);
    double maxE = ADC_to_keV_scale[R];
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

    //double maxE = m_EnergyCalibration.GetEnergy(R, m_HistogramMaxADC);
    double maxE = ADC_to_keV_scale[R];

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

  map<MReadOutElementDoubleStrip, double> thresholds;
  map<MReadOutElementDoubleStrip, double> thresholdsADC;

  m_StripIndex_LV.clear();
  m_StripIndex_HV.clear();
  m_ThresholdValues_LV.clear();
  m_ThresholdValues_HV.clear();


  for (auto& kv : m_ADCHistograms) {

    MReadOutElementDoubleStrip R = kv.first;
    TH1D* hist = kv.second;

    if (hist->GetEntries() < m_MinEntries) {

      thresholds[R] = m_FallbackThresholdKeV;

	    // Mark the ADC value as invalid since we cannot easily convert back from
	    // the fallback keV value to ADC bins
      thresholdsADC[R] = -1.0;

	    // If a fallback threshold is used for a slow threshold channel we print a warning
      cout << "WARNING: SLOW threshold could not be determined for Det "
           << R.GetDetectorID()
           << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
           << " Strip " << R.GetStripID()
           << ": only " << hist->GetEntries()
           << " histogram entries. "
           << "Using fallback threshold = "
           << m_FallbackThresholdKeV << " keV."
           << endl;

      continue;
    }

    hist->Smooth(3);

    //int maxSearchBin = hist->FindBin(m_NoiseSearchMaxADC);

    double maxSearchADC =
      m_EnergyCalibration.GetADC(R, m_NoiseSearchMaxKeV);

    int maxSearchBin = hist->FindBin(maxSearchADC);

    // Keep the search inside the histogram
    maxSearchBin = min(maxSearchBin, hist->GetNbinsX());

    //--------------------------------------------------------------
    // Original slow threshold algorithm
    //--------------------------------------------------------------
    int startBin = -1;

    for (int b = 1; b <= maxSearchBin; b++) {
      if (hist->GetBinContent(b) > m_NoiseStartMinCounts) {
        startBin = b;
        break;
      }
    }

    if (startBin < 0) {

    thresholds[R] = m_FallbackThresholdKeV;
    thresholdsADC[R] = -1.0;

      cout << "WARNING: SLOW threshold could not be determined for Det "
           << R.GetDetectorID()
           << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
           << " Strip " << R.GetStripID()
           << ": no valid noise distribution found below "
           << m_NoiseSearchMaxKeV << " keV. "
           << "Using fallback threshold = "
           << m_FallbackThresholdKeV << " keV."
           << endl;

      continue;
    }

    int peakBin = startBin;
    double peakCounts = hist->GetBinContent(startBin);

    const int declineBinsRequired = 3;
    int declineCount = 0;

    for (int b = startBin + 1; b <= maxSearchBin; b++) {

      double c = hist->GetBinContent(b);

      // if (R.GetStripID() == 64 && !R.IsLowVoltageStrip()) {
      //   cout << "GR PEAK SEARCH:"
      //        << " bin=" << b
      //        << " ADC=" << hist->GetBinCenter(b)
      //        << " counts=" << c
      //        << " currentPeakADC=" << hist->GetBinCenter(peakBin)
      //        << " currentPeakCounts=" << peakCounts
      //        << " declineCount=" << declineCount
      //        << endl;
      // }

      if (c > peakCounts) {

        // Still climbing: update candidate peak
        peakCounts = c;
        peakBin = b;
        declineCount = 0;

      } else {

        // Counts are now below the candidate peak
        declineCount++;

        // A sustained decline means we passed the first real peak
        if (declineCount >= declineBinsRequired) {
          break;
        }
      }
    }
    // -------------------------------------------------------------
    // TEMPORARY DIAGNOSTIC: report identified noise peak
    // -------------------------------------------------------------

    // Print out the noise peaks to the temrinal
    /* cout << "NOISE PEAK: Det "
         << R.GetDetectorID()
         << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
         << " Strip " << R.GetStripID()
         << " startADC=" << hist->GetBinCenter(startBin)
         << " peakADC=" << hist->GetBinCenter(peakBin)
         << " peakCounts=" << peakCounts
         << endl; */


    // -------------------------------------------------------------
    // Detect pedestal-like noise distributions
    //
    // Some noisy channels do not have a well-defined noise peak.
    // Instead, they rise sharply from ~0 counts into a broad, flat
    // pedestal. A pedestal-like spectrum must show both:
    //
    //   1. A rapid rise to near the maximum noise height
    //   2. A sustained flat top for several bins
    //
    // This prevents conventional noise peaks with a sharp leading
    // edge and a falling tail from being misidentified as pedestals.
    // -------------------------------------------------------------

    bool pedestalLike = false;
    int pedestalThresholdBin = -1;

    // Save the measured plateau height so it can also be used
    // to determine the slow hardware threshold.
    double pedestalPlateauCounts = -1.0;

    // Look only a few bins beyond the first significant bin
    const int pedestalLookAheadBins = 8;
    int pedestalEndBin =
      min(startBin + pedestalLookAheadBins, maxSearchBin);

    // Number of consecutive bins required to establish a flat top
    const int pedestalPlateauBins = 6;

    // Maximum allowed variation from the first plateau bin
    const double pedestalFlatTolerance = 0.10; // +/- 10%

    const int pedestalTailCheckBins = 3;
    const double pedestalTailMinFraction = 0.85;

    // Search for a rapid rise to near the measured noise height
    for (int b = startBin; b <= pedestalEndBin; ++b) {

      if (hist->GetBinContent(b) >= 0.80 * peakCounts &&
          b - startBin <= 4) {

        // ---------------------------------------------------------
        // We have found a rapid rise. Now determine whether the
        // spectrum remains approximately flat for several bins.
        // ---------------------------------------------------------

        bool flatTop = true;
        double plateauReference = hist->GetBinContent(b);

        int plateauEndBin =
          min(b + pedestalPlateauBins - 1, maxSearchBin);

        // Require the full number of plateau bins to be available
        if (plateauEndBin - b + 1 < pedestalPlateauBins) {
          flatTop = false;
        }

        if (flatTop == true) {

          for (int p = b; p <= plateauEndBin; ++p) {

            double counts = hist->GetBinContent(p);

            double lowerLimit =
              plateauReference * (1.0 - pedestalFlatTolerance);

            double upperLimit =
              plateauReference * (1.0 + pedestalFlatTolerance);

            if (counts < lowerLimit || counts > upperLimit) {
              flatTop = false;
              break;
            }
          }
        }

		// ---------------------------------------------------------
        // A broad conventional peak can appear approximately flat
        // for several bins near its maximum. A pedestal should
        // remain high after the candidate flat region rather than
        // immediately entering a sustained falling tail.
        // ---------------------------------------------------------

        if (flatTop == true) {

          int tailEndBin =
            min(plateauEndBin + pedestalTailCheckBins, maxSearchBin);

          // Require enough bins beyond the candidate plateau to
          // determine whether the spectrum remains pedestal-like.
          if (tailEndBin - plateauEndBin < pedestalTailCheckBins) {
            flatTop = false;
          } else {

            for (int p = plateauEndBin + 1; p <= tailEndBin; ++p) {

              double counts = hist->GetBinContent(p);

              if (counts < pedestalTailMinFraction * plateauReference) {
                flatTop = false;
                break;
              }
            }
          }
        }

        if (flatTop == false) {
          continue;
        }

        // ---------------------------------------------------------
        // Rapid rise + sustained flat top:
        // classify this channel as pedestal-like.
        // ---------------------------------------------------------

        pedestalLike = true;
		pedestalPlateauCounts = plateauReference;

        // Continue forward from the upper part of the rising edge
        // and find where the spectrum stops increasing significantly.
        // Put the threshold one bin beyond that point.
        for (int p = b; p < pedestalEndBin; ++p) {

          double current = hist->GetBinContent(p);
          double next = hist->GetBinContent(p + 1);

          // The leading edge has reached the plateau when the next
          // bin is no more than 5% higher than the current bin.
          if (next <= current * 1.05) {
            // The leading edge has reached the plateau.
            // Move the threshold a few bins farther into the plateau
            // to provide margin above the rising noise edge.
            const int pedestalThresholdOffsetBins = 2;

            pedestalThresholdBin =
              min(p + 1 + pedestalThresholdOffsetBins, maxSearchBin);

            break;
          }
        }

        // Safety fallback: if no flattening point was found within
        // the look-ahead region, use the end of that region.
        if (pedestalThresholdBin < 0) {
          pedestalThresholdBin = pedestalEndBin;
        }

        break;
      }
    }

    /*
    // Print the GR result to the terminal for debugging
    if (R.GetStripID() == 64) {
        cout << "GR FINAL DECISION:"
             << " pedestalLike=" << pedestalLike
             << " startADC=" << hist->GetBinCenter(startBin)
             << " peakADC=" << hist->GetBinCenter(peakBin)
             << " peakCounts=" << peakCounts
             << " thresholdADC=" << hist->GetBinCenter(thresholdBin)
             << " thresholdCounts=" << hist->GetBinContent(thresholdBin)
             << endl;
    } */


    // -------------------------------------------------------------
    // Determine the slow HARDWARE threshold
    //
    // The hardware threshold is defined as the 50% point on the
    // rising edge of the identified low-energy feature.
    //
    // For a conventional peak, use 50% of the peak height.
    // For a pedestal-like feature, use 50% of the plateau height.
    // -------------------------------------------------------------

    double hardwareReferenceCounts = peakCounts;

    if (pedestalLike == true && pedestalPlateauCounts > 0.0) {
      hardwareReferenceCounts = pedestalPlateauCounts;
    }

    double hardwareHalfHeight = 0.50 * hardwareReferenceCounts;

    int hardwareThresholdBin = -1;

    // Search from the beginning of the identified feature toward
    // the peak/plateau for the first crossing of the 50% level.
    for (int b = startBin; b <= peakBin; ++b) {

      if (hist->GetBinContent(b) >= hardwareHalfHeight) {
        hardwareThresholdBin = b;
        break;
      }
    }

    // If the 50% crossing could not be found, mark this hardware
    // threshold as invalid rather than altering the software result.
    if (hardwareThresholdBin >= 1) {

      double hardwareThresholdADC =
        hist->GetBinCenter(hardwareThresholdBin);

      double hardwareThresholdKeV =
        m_EnergyCalibration.GetEnergy(R, hardwareThresholdADC);

      m_SlowHardwareThresholdsADC[R] = hardwareThresholdADC;
      m_SlowHardwareThresholds[R] = hardwareThresholdKeV;

      // -------------------------------------------------------------
      // TEMPORARY DIAGNOSTIC:
      // Print the slow hardware threshold so we can verify that the
      // 50% rising-edge calculation is behaving as expected.
      // -------------------------------------------------------------

      /* cout << "SLOW HARDWARE threshold: Det "
           << R.GetDetectorID()
           << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
           << " Strip " << R.GetStripID()
           << " | pedestalLike=" << pedestalLike
           << " | startADC=" << hist->GetBinCenter(startBin)
           << " | peakADC=" << hist->GetBinCenter(peakBin)
           << " | referenceCounts=" << hardwareReferenceCounts
           << " | halfHeight=" << hardwareHalfHeight
           << " | hardwareADC=" << hardwareThresholdADC
           << " | hardwareKeV=" << hardwareThresholdKeV
           << endl; */


    } else {

      m_SlowHardwareThresholdsADC[R] = -1.0;
      m_SlowHardwareThresholds[R] = -1.0;
    }



    // -------------------------------------------------------------
    // Determine the slow threshold
    // -------------------------------------------------------------

    int thresholdBin = peakBin;

    if (pedestalLike == true) {

      // Pedestal-like distribution:
      // pedestalThresholdBin already includes the offset that moves
      // the threshold a few bins into the flat top.
      thresholdBin = pedestalThresholdBin;

    } else if (R.GetStripID() == 64) {

      // -----------------------------------------------------------
      // Guard ring:
      // Find the first bin on the falling side of the noise peak
      // where the counts have dropped to 50% of the peak height.
      // -----------------------------------------------------------

      for (int b = peakBin + 1; b <= maxSearchBin; ++b) {

        if (hist->GetBinContent(b) <= 0.50 * peakCounts) {
          thresholdBin = b;
          break;
        }
      }

    } else {

      // -----------------------------------------------------------
      // Normal central strip:
      // Search for the trough to the right of the noise peak.
      //
      // Keep track of the lowest bin while moving away from the
      // noise peak. Stop once the spectrum begins rising into the
      // physical-event distribution.
      // -----------------------------------------------------------

      double minCounts = hist->GetBinContent(peakBin);

      for (int b = peakBin + 1; b <= maxSearchBin; ++b) {

        double counts = hist->GetBinContent(b);

        if (counts < minCounts) {
          minCounts = counts;
          thresholdBin = b;
        }

        // Once the spectrum has risen substantially again, we have
        // passed the trough and should stop searching.
        if (counts >= 0.50 * peakCounts) {
          break;
        }
      }
    }


    // -------------------------------------------------------------
    // Shift conventional thresholds slightly to the right
    //
    // Pedestal thresholds are NOT shifted here because the pedestal
    // logic above already moves them a few bins into the flat top.
    // -------------------------------------------------------------

    if (pedestalLike == false) {
      const int shiftBins = 2;
      thresholdBin =
        min(thresholdBin + shiftBins, hist->GetNbinsX());
    }

    double thresholdADC = hist->GetBinCenter(thresholdBin);

    // Convert ADC → keV using energy calibration
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

  if (m_TimingCounts.empty() == true) {
    cout << "Warning: No timing data available for fast threshold calculation." << endl;
  }

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

    // -------------------------------------------------------------
    // Count total dt0 and dt1 populations
    // -------------------------------------------------------------

    int totalCounts = 0;
    int totalDt0Counts = 0;
    int totalDt1Counts = 0;

    for (auto& a : ADCMap) {

      int n0 = a.second.first;
      int n1 = a.second.second;

      totalDt0Counts += n0;
      totalDt1Counts += n1;
      totalCounts += n0 + n1;
    }

    double dt1Fraction = 0.0;

    if (totalCounts > 0) {
      dt1Fraction =
        static_cast<double>(totalDt1Counts) /
        static_cast<double>(totalCounts);
    }


    // -------------------------------------------------------------
    // FAST threshold fallback
    //
    // A physical dt0/dt1 crossover cannot be determined if there
    // are too few total events, or if either timing population is
    // essentially absent.
    // -------------------------------------------------------------

    if (totalCounts < m_MinEntries) {

      m_FastThresholds[R] = m_FastFallbackThresholdKeV;
      // If a fast fallback value is used, we cannot calculate the ADC value from keV
      // We set it to -1 so that it's clearly not real
      m_FastThresholdsADC[R] = -1.0;

      cout << "WARNING: FAST threshold could not be determined for Det "
           << R.GetDetectorID()
           << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
           << " Strip " << R.GetStripID()
           << ": only " << totalCounts
           << " total timing counts. "
           << "Using fallback threshold = "
           << m_FastFallbackThresholdKeV << " keV."
           << endl;

      continue;
    }


    // A valid FAST threshold requires a meaningful population
    // of both dt0 and dt1 events for every strip. A handful of sparse dt1 counts
    // distributed across the spectrum is not sufficient to define
    // a physical crossover.
    const double minDt1Fraction = 0.05; // 5%

    if (totalDt0Counts < m_MinEntries ||
        totalDt1Counts < m_MinEntries ||
        dt1Fraction < minDt1Fraction) {

      m_FastThresholds[R] = m_FastFallbackThresholdKeV;
      m_FastThresholdsADC[R] = -1.0;

      cout << "WARNING: FAST threshold could not be determined for Det "
           << R.GetDetectorID()
           << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
           << " Strip " << R.GetStripID()
           << ": insufficient timing populations "
           << "(dt0=" << totalDt0Counts
           << ", dt1=" << totalDt1Counts
           << ", dt1 fraction=" << 100.0 * dt1Fraction << "%). "
           << "Using fallback threshold = "
           << m_FastFallbackThresholdKeV << " keV."
           << endl;

      continue;
    }

    // Print out every fast timing population fraction
    /* cout << "FAST timing populations: Det "
         << R.GetDetectorID()
         << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
         << " Strip " << R.GetStripID()
         << " dt0=" << totalDt0Counts
         << " dt1=" << totalDt1Counts
         << " dt1_fraction=" << 100.0 * dt1Fraction << "%"
         << endl; */

    // Find first nonzero ADC
    int first_nonzero = -1;
    if (kv.second.empty() == true) {
      continue;
    }

    for (auto& a : ADCMap) {
      if (a.second.first + a.second.second > 0) {
        first_nonzero = a.first + 10;
        break;
      }
    }

    if (first_nonzero < 0) {

      m_FastThresholds[R] = m_FastFallbackThresholdKeV;
      m_FastThresholdsADC[R] = -1.0;

      cout << "WARNING: No valid FAST timing data found for Det "
           << R.GetDetectorID()
           << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
           << " Strip " << R.GetStripID()
           << ". Using fallback threshold = "
           << m_FastFallbackThresholdKeV << " keV."
           << endl;

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
      if (n0 + n1 < 50) {
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

      m_FastThresholds[R] = m_FastFallbackThresholdKeV;
      m_FastThresholdsADC[R] = -1.0;

      cout << "WARNING: No valid FAST threshold solution found for Det "
           << R.GetDetectorID()
           << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
           << " Strip " << R.GetStripID()
           << ". Using fallback threshold = "
           << m_FastFallbackThresholdKeV << " keV."
           << endl;

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

	  // TEMPORARY DIAGNOSTIC:
    // Print the channel immediately before attempting energy calibration
    // cout << "FAST calibration request: Det "
    //     << R.GetDetectorID()
    //     << " Side " << (R.IsLowVoltageStrip() ? "LV" : "HV")
    //     << " Strip " << R.GetStripID()
    //     << " ADC " << fast_thresh_ADC
    //     << endl;
 
    double fast_thresh_keV = m_EnergyCalibration.GetEnergy(R, fast_thresh_ADC);

    m_FastThresholds[R] = fast_thresh_keV;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void MStripThresholdFinder::WriteCSV()
{

  if (m_SlowThresholds.empty() == true && m_FastThresholds.empty() == true) {
    cout << "Warning: No thresholds available to write to CSV." << endl;
  }

  // -------------------------------------------------------------
  // Write CSV threshold tables (HV and LV)
  // -------------------------------------------------------------

  MString HVOutputCSVFileName = m_OutputPrefix + "_Slow_HV_thresholds.csv";
  MString LVOutputCSVFileName = m_OutputPrefix + "_Slow_LV_thresholds.csv";

  ofstream csv_HV(HVOutputCSVFileName);
  ofstream csv_LV(LVOutputCSVFileName);

  if (csv_HV.is_open() == false) {
    cerr << "Error: Failed to open CSV output file for HV thresholds: "
         << HVOutputCSVFileName << endl;
    return;
  }

  if (csv_LV.is_open() == false) {
    cerr << "Error: Failed to open CSV output file for LV thresholds: "
         << LVOutputCSVFileName << endl;
    return;
  }

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
  // Write SLOW hardware threshold CSV tables
  // -------------------------------------------------------------

  MString HardwareHVOutputCSVFileName =
    m_OutputPrefix + "_Slow_Hardware_HV_thresholds.csv";

  MString HardwareLVOutputCSVFileName =
    m_OutputPrefix + "_Slow_Hardware_LV_thresholds.csv";

  ofstream csv_Hardware_HV(HardwareHVOutputCSVFileName);
  ofstream csv_Hardware_LV(HardwareLVOutputCSVFileName);

  if (csv_Hardware_HV.is_open() == false) {
    cerr << "Error: Failed to open CSV output file for HV hardware thresholds: "
         << HardwareHVOutputCSVFileName << endl;
    return;
  }

  if (csv_Hardware_LV.is_open() == false) {
    cerr << "Error: Failed to open CSV output file for LV hardware thresholds: "
         << HardwareLVOutputCSVFileName << endl;
    return;
  }

  // CSV headers
  csv_Hardware_HV
    << "detector_side,Strip,threshold_ADC,threshold_keV\n";

  csv_Hardware_LV
    << "detector_side,Strip,threshold_ADC,threshold_keV\n";

  // Write rows
  for (const auto& kv : m_SlowHardwareThresholds) {

    MReadOutElementDoubleStrip R = kv.first;
    int Strip = R.GetStripID();

    double thr_keV = kv.second;
    double thr_ADC = m_SlowHardwareThresholdsADC[R];

    if (R.IsLowVoltageStrip() == true) {

      csv_Hardware_LV
        << "l,"
        << Strip << ","
        << thr_ADC << ","
        << thr_keV << "\n";

    } else {

      csv_Hardware_HV
        << "h,"
        << Strip << ","
        << thr_ADC << ","
        << thr_keV << "\n";
    }
  }


  // -------------------------------------------------------------
  // Write Fast CSV threshold tables
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

  csv_Hardware_HV.close();
  csv_Hardware_LV.close();

  csv_TAC_HV.close();
  csv_TAC_LV.close();
}


void MStripThresholdFinder::WriteDiagnostics()
{

  if (m_SlowThresholds.empty() == true && m_FastThresholds.empty() == true) {
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
  // SLOW HARDWARE threshold per Strip (HV vs LV scatter)
  // -------------------------------------------------------------

  TGraph* gSlowHardwareThresh_LV = new TGraph();
  TGraph* gSlowHardwareThresh_HV = new TGraph();

  gSlowHardwareThresh_LV->SetName("SlowHardwareThresh_LV");
  gSlowHardwareThresh_HV->SetName("SlowHardwareThresh_HV");

  gSlowHardwareThresh_LV->SetTitle(
    "Slow Hardware Threshold per Strip;Strip;Hardware Threshold (keV)");

  gSlowHardwareThresh_LV->SetMarkerStyle(20);
  gSlowHardwareThresh_LV->SetMarkerSize(1.0);
  gSlowHardwareThresh_LV->SetMarkerColor(kBlue);

  gSlowHardwareThresh_HV->SetMarkerStyle(20);
  gSlowHardwareThresh_HV->SetMarkerSize(1.0);
  gSlowHardwareThresh_HV->SetMarkerColor(kRed);


  // Fill graphs
  for (const auto& kv : m_SlowHardwareThresholds) {

    MReadOutElementDoubleStrip R = kv.first;
    int Strip = R.GetStripID();
    double Threshold = kv.second;

    // Ignore invalid hardware thresholds
    if (Threshold < 0.0) {
      continue;
    }

    if (R.IsLowVoltageStrip() == true) {

      gSlowHardwareThresh_LV->SetPoint(
        gSlowHardwareThresh_LV->GetN(),
        Strip,
        Threshold);

    } else {

      gSlowHardwareThresh_HV->SetPoint(
        gSlowHardwareThresh_HV->GetN(),
        Strip,
        Threshold);
    }
  }


  // -------------------------------------------------------------
  // Canvas with LV and HV overlaid
  // -------------------------------------------------------------

  TCanvas* cSlowHardwareThresh =
    new TCanvas(
      "cSlowHardwareThresh",
      "Slow Hardware Threshold per Strip",
      800,
      600);

  cSlowHardwareThresh->cd();

  gSlowHardwareThresh_LV->Draw("AP");


  // Determine plotting range
  double hardwareYmin = 1e9;
  double hardwareYmax = -1e9;

  for (const auto& kv : m_SlowHardwareThresholds) {

    double v = kv.second;

    if (v < 0.0) {
      continue;
    }

    if (v < hardwareYmin) {
      hardwareYmin = v;
    }

    if (v > hardwareYmax) {
      hardwareYmax = v;
    }
  }

  if (hardwareYmax > hardwareYmin) {

    double hardwarePad =
      0.10 * (hardwareYmax - hardwareYmin);

    gSlowHardwareThresh_LV->GetYaxis()->SetRangeUser(
      hardwareYmin - hardwarePad,
      hardwareYmax + hardwarePad);
  }

  gSlowHardwareThresh_HV->Draw("P SAME");

  TLegend* legSlowHardwareScatter =
    new TLegend(0.70, 0.72, 0.80, 0.80);

  legSlowHardwareScatter->AddEntry(
    gSlowHardwareThresh_LV, "LV", "p");

  legSlowHardwareScatter->AddEntry(
    gSlowHardwareThresh_HV, "HV", "p");

  legSlowHardwareScatter->SetTextSize(0.02);
  legSlowHardwareScatter->SetBorderSize(1);
  legSlowHardwareScatter->SetFillStyle(0);

  legSlowHardwareScatter->Draw();

  cSlowHardwareThresh->Modified();
  cSlowHardwareThresh->Update();

  cSlowHardwareThresh->Write();
  gSlowHardwareThresh_LV->Write();
  gSlowHardwareThresh_HV->Write();

  // -------------------------------------------------------------
  // SLOW HARDWARE threshold distribution (HV vs LV separated)
  // -------------------------------------------------------------

  TH1D* hSlowHardwareDist_LV = new TH1D(
    "SlowHardwareThresholdDistribution_LV",
    "Slow Hardware Threshold Distribution;Hardware Threshold (keV);Counts",
    100, 0, 50);

  TH1D* hSlowHardwareDist_HV = new TH1D(
    "SlowHardwareThresholdDistribution_HV",
    "Slow Hardware Threshold Distribution;Hardware Threshold (keV);Counts",
    100, 0, 50);


  // Styling
  hSlowHardwareDist_LV->SetLineColor(kBlue);
  hSlowHardwareDist_LV->SetLineWidth(2);

  hSlowHardwareDist_HV->SetLineColor(kRed);
  hSlowHardwareDist_HV->SetLineWidth(2);


  // Fill histograms
  for (const auto& kv : m_SlowHardwareThresholds) {

    MReadOutElementDoubleStrip R = kv.first;
    double Threshold = kv.second;

    if (Threshold < 0.0) {
      continue;
    }

    if (R.IsLowVoltageStrip() == true) {
      hSlowHardwareDist_LV->Fill(Threshold);
    } else {
      hSlowHardwareDist_HV->Fill(Threshold);
    }
  }


  // -------------------------------------------------------------
  // Canvas with legend
  // -------------------------------------------------------------

  TCanvas* cSlowHardwareDist =
    new TCanvas(
      "cSlowHardwareDist",
      "Slow Hardware Threshold Distribution",
      800,
      600);

  cSlowHardwareDist->cd();

  hSlowHardwareDist_LV->Draw("HIST");
  hSlowHardwareDist_HV->Draw("HIST SAME");

  TLegend* legSlowHardware =
    new TLegend(0.70, 0.72, 0.80, 0.80);

  legSlowHardware->AddEntry(
    hSlowHardwareDist_LV, "LV", "l");

  legSlowHardware->AddEntry(
    hSlowHardwareDist_HV, "HV", "l");

  legSlowHardware->Draw();

  cSlowHardwareDist->Modified();
  cSlowHardwareDist->Update();

  cSlowHardwareDist->Write();
  hSlowHardwareDist_LV->Write();
  hSlowHardwareDist_HV->Write();


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

  if (m_StripIndex_LV.empty() == false) {
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

  if (m_StripIndex_HV.empty() == false) {
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
      leg->AddEntry(line, "Fast Threshold", "l");
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
      "Slow Software and Hardware Thresholds (det=%d side=%c Strip=%d);Energy (keV);Counts",
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

    // -------------------------------------------------------------
    // Software slow threshold
    // -------------------------------------------------------------

    if (m_SlowThresholds.count(R) == 1) {

      double softwareThreshold =
        m_SlowThresholds[R];

      TLine* softwareLine =
        new TLine(
          softwareThreshold,
          0,
          softwareThreshold,
          energyHist->GetMaximum());

      softwareLine->SetLineColor(kRed);
      softwareLine->SetLineWidth(2);

      energyHist->GetListOfFunctions()->Add(softwareLine);
    }


    // -------------------------------------------------------------
    // Hardware slow threshold
    // -------------------------------------------------------------

    if (m_SlowHardwareThresholds.count(R) == 1 &&
        m_SlowHardwareThresholds[R] >= 0.0) {

      double hardwareThreshold =
        m_SlowHardwareThresholds[R];

      TLine* hardwareLine =
        new TLine(
          hardwareThreshold,
          0,
          hardwareThreshold,
          energyHist->GetMaximum());

      hardwareLine->SetLineColor(kBlue);
      hardwareLine->SetLineWidth(2);
      hardwareLine->SetLineStyle(2);

      energyHist->GetListOfFunctions()->Add(hardwareLine);
    }
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
