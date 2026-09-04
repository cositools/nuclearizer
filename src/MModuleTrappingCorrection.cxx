/*
 * MModuleTrappingCorrection.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer, Alex Lowell, 
 * 				Sophie Haight, Carolyn Kierans.
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
// MModuleTrappingCorrection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleTrappingCorrection.h"

// Standard libs:

// ROOT libs:
#include "TMath.h"
#include "TGClient.h"
#include "TH1.h"

// MEGAlib libs:
#include "MString.h"

// Nuclearizer libs:
#include "MGUIOptionsTrappingCorrection.h"
#include "MGUIExpoTrappingCorrection.h"
#include "MGUIExpoPlotSpectrum.h"
#include "MModuleEnergyCalibration.h"
#include "MModuleDepthCalibration.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleTrappingCorrection)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleTrappingCorrection::MModuleTrappingCorrection() : MModule()
{
  // Construct an instance of MModuleTrappingCorrection

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Trapping Correction"; // - correcting energies for charge trapping (by Sophie);

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagTrappingCorrection";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration, true);
  AddPreceedingModuleType(MAssembly::c_StripPairing, true);
  AddPreceedingModuleType(MAssembly::c_TACcut, true);
  AddPreceedingModuleType(MAssembly::c_DepthCorrection, true);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_TrappingCorrection);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;


}


////////////////////////////////////////////////////////////////////////////////


MModuleTrappingCorrection::~MModuleTrappingCorrection()
{
  // Delete this instance of MModuleTrappingCorrection
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTrappingCorrection::Initialize()
{
  m_SimCCEFileIsLoaded = LoadSimCCEFile(m_SimCCEFileName);
  if (m_SimCCEFileIsLoaded == false) {
    return false;
  }
  
  MSupervisor* S = MSupervisor::GetSupervisor();
  m_EnergyCalibration = (MModuleEnergyCalibration*) S->GetAvailableModuleByXmlTag("EnergyCalibration");
  if (m_EnergyCalibration == nullptr) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR in MModuleTrappingCorrection::Initialize: couldn't resolve pointer to Energy Calibration Module... need access to this module for energy resolution lookup!" << endl;
    }
       return false;
  }

  m_DepthCalibration = (MModuleDepthCalibration*) S->GetAvailableModuleByXmlTag("DepthCalibration");
  if (m_DepthCalibration == nullptr) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR in MModuleTrappingCorrection::Initialize: couldn't resolve pointer to Depth Calibration Module... need access to this module for depth resolution lookup!" << endl;
    }
       return false;
  }

// TO DO: remove this check once we successfully process multiple detectors
  m_DetectorIDs = m_DepthCalibration->GetDetectorIDs();

  if (m_DetectorIDs.empty()) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR in MModuleTrappingCorrection::Initialize: Depth Calibration has no registered detector IDs!" << endl;
    }
    return false;
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////

void MModuleTrappingCorrection::CreateExpos()
{
  // Create all expos

  if (HasExpos() == true) return;

  // Set the histogram display using the new double-canvas GUI class
  m_ExpoSpectrum = new MGUIExpoTrappingCorrection(this); 
  m_ExpoSpectrum->SetEnergyHistogramParameters(200, 0, 2000);
  m_Expos.push_back(m_ExpoSpectrum);
}


/////////////////////////////////////////////////////////////////////////////////


bool MModuleTrappingCorrection::AnalyzeEvent(MReadOutAssembly* Event) 
{

  if (Event->GetGuardRingVeto() || m_DetectorParamMap.empty()) {
  } 

  for (unsigned int i = 0; i < Event->GetNHits(); ++i) {
    MHit* H = Event->GetHit(i);


    int Grade = m_DepthCalibration->GetHitGrade(H);
    if (Grade < 0 || Grade > 4) {
      H->SetNoDepth();
      continue; 
    } 

    // Local Z depth position (in cm) from the hit
    double depth_val = H->GetLocalPosition().GetZ();

    // Separate strip hits into LV and HV lists
    vector<MStripHit*> LVStrips;
    vector<MStripHit*> HVStrips;


    for (unsigned int j = 0; j < H->GetNStripHits(); ++j) {
      MStripHit* SH = H->GetStripHit(j);
      if (SH->IsLowVoltageStrip()) LVStrips.push_back(SH);
      else HVStrips.push_back(SH);
    }

    // To account for charge sharing, get the dominant strip in each category and its energy fraction
    double LVEnergyFraction = 0.0;
    double HVEnergyFraction = 0.0;
    MStripHit* LVSH = m_DepthCalibration->GetDominantStrip(LVStrips, LVEnergyFraction); 
    MStripHit* HVSH = m_DepthCalibration->GetDominantStrip(HVStrips, HVEnergyFraction); 
  
    // --- Low Voltage (LV) Side ---
    if (LVSH != nullptr) {
      int detID = LVSH->GetDetectorID();
      // Iterate through detector param map to look up pre-loaded parameters for this detector
      auto it = m_DetectorParamMap.find(detID);

      if (it != m_DetectorParamMap.end()) {
        // Define a reference to the detector param struct 
        // Access the parameter data once the iterator is fixed
        const DetectorTrappingData& detData = it->second;

        // Check depth bounds using this specific detector's CCE grid limits
        double min_param_depth = std::min(detData.m_Depths.front(), detData.m_Depths.back());
        double max_param_depth = std::max(detData.m_Depths.front(), detData.m_Depths.back());
        bool is_depth_out_of_bounds = (depth_val < min_param_depth || depth_val > max_param_depth);

        double rawLVEnergy = LVSH->GetEnergy(); 

        if (HasExpos() == true) {
          m_ExpoSpectrum->AddEnergyInitial(rawLVEnergy, LVSH->IsNearestNeighbor(), LVSH->IsLowVoltageStrip());
        }

        double correctedLVEnergy = rawLVEnergy;
        if (!is_depth_out_of_bounds) {
          correctedLVEnergy = GetSimBasedCorrectedEnergy(depth_val, rawLVEnergy, detData.m_Depths, detData.m_CCEs_LV_e, detData.m_CCEs_LV_h, detData.m_ParamB, detData.m_ParamC);
        }

        LVSH->SetEnergy(correctedLVEnergy);    

        if (HasExpos() == true) {
          m_ExpoSpectrum->AddEnergyFinal(correctedLVEnergy, LVSH->IsNearestNeighbor(), LVSH->IsLowVoltageStrip());
        }
      } else {
        if (g_Verbosity >= c_Warning) {
          cout << "WARNING [TrappingCorrection]: LV Strip Detector ID " << detID << " not found in map!" << endl;
        }
      }
    }

    // --- High Voltage (HV) Side ---
    if (HVSH != nullptr) {
            int detID = HVSH->GetDetectorID();
      // Iterate through detector param map to look up pre-loaded parameters for this detector
      auto it = m_DetectorParamMap.find(detID);


      if (it != m_DetectorParamMap.end()) {
        // Define a reference to the detector param struct 
        // Access the parameter data once the iterator is fixed
        const DetectorTrappingData& detData = it->second;

        // Check depth bounds using this specific detector's CCE grid limits
        double min_param_depth = std::min(detData.m_Depths.front(), detData.m_Depths.back());
        double max_param_depth = std::max(detData.m_Depths.front(), detData.m_Depths.back());
        bool is_depth_out_of_bounds = (depth_val < min_param_depth || depth_val > max_param_depth);

        double rawHVEnergy = HVSH->GetEnergy(); 
        
        if (HasExpos() == true) {
          m_ExpoSpectrum->AddEnergyInitial(rawHVEnergy, HVSH->IsNearestNeighbor(), HVSH->IsLowVoltageStrip());
        }
        double correctedHVEnergy = rawHVEnergy;
        if (!is_depth_out_of_bounds) {
          correctedHVEnergy = GetSimBasedCorrectedEnergy( depth_val, rawHVEnergy, detData.m_Depths, detData.m_CCEs_HV_e, detData.m_CCEs_HV_h, detData.m_ParamB, detData.m_ParamC);
        }

        HVSH->SetEnergy(correctedHVEnergy);    

        if (HasExpos() == true) {
          m_ExpoSpectrum->AddEnergyFinal(correctedHVEnergy, HVSH->IsNearestNeighbor(), HVSH->IsLowVoltageStrip());
        }
      } else {
        if (g_Verbosity >= c_Warning) {
          cout << "WARNING [TrappingCorrection]: HV Strip Detector ID " << detID << " not found in map!" << endl;

        }
      }
    }
  }

  Event->SetAnalysisProgress(MAssembly::c_TrappingCorrection);
  return true;
}
    
/////////////////////////////////////////////////////////////////////////////////

void MModuleTrappingCorrection::Finalize()
{
  MModule::Finalize();

  if (m_ExpoSpectrum == nullptr) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR in MModuleTrappingCorrection::Finalize: Expo plot spectrum is null." << endl;
    }
    return;
  }

  if (g_Verbosity >= c_Info) {
    cout << "INFO: Finalizing Trapping Correction Module..." << endl;
  } 

  // Retrieve both uncorrected and corrected histograms
  TH1D* histLVInit  = m_ExpoSpectrum->GetEnergyHistogramLVInitial();
  TH1D* histLVFinal = m_ExpoSpectrum->GetEnergyHistogramLVFinal();

  TH1D* histHVInit  = m_ExpoSpectrum->GetEnergyHistogramHVInitial();
  TH1D* histHVFinal = m_ExpoSpectrum->GetEnergyHistogramHVFinal();

  // Helper lambda to calculate FWHM directly from the full continuous TF1 function curve
  auto CalculateFunctionFWHM = [](TF1* func, double xMin, double xMax, double step = 0.01) -> double {
    if (func == nullptr) return 0.0;

    double xPeak = func->GetMaximumX(xMin, xMax);
    double yPeak = func->Eval(xPeak);
    double halfMax = yPeak / 2.0;

    // Search left for half-max crossing point
    double xLeft = xPeak;
    for (double x = xPeak; x >= xMin; x -= step) {
      if (func->Eval(x) <= halfMax) {
        // Refine with linear interpolation between step bounds
        double x1 = x, y1 = func->Eval(x1);
        double x2 = x + step, y2 = func->Eval(x2);
        xLeft = (y2 != y1) ? x1 + (halfMax - y1) * (x2 - x1) / (y2 - y1) : x1;
        break;
      }
    }

    // Search right for half-max crossing point
    double xRight = xPeak;
    for (double x = xPeak; x <= xMax; x += step) {
      if (func->Eval(x) <= halfMax) {
        // Refine with linear interpolation between step bounds
        double x1 = x - step, y1 = func->Eval(x1);
        double x2 = x, y2 = func->Eval(x2);
        xRight = (y2 != y1) ? x1 + (halfMax - y1) * (x2 - x1) / (y2 - y1) : x2;
        break;
      }
    }

    return (xRight > xLeft) ? (xRight - xLeft) : 0.0;
  };

  // Helper lambda to perform fit, output results
  // returns : mu, gauss_fwhm, full_fit_fwhm
  auto FitAndPrintSpectrum = [&](TH1D* hist, const string& titleLabel) -> std::tuple<double, double, double, double> {
    if (hist == nullptr || hist->GetEntries() <= 0) {
      cout << "WARNING: " << titleLabel << " histogram is null or has 0 entries." << endl;
      return std::make_tuple(0.0, 0.0, 0.0, 0.0);
    }

    TF1* fitFunc = GeneratePhotopeakFunction();
    fitFunc->SetParameter("Amplitude", hist->GetBinContent(hist->GetMaximumBin()));
    hist->Fit(fitFunc, "RQ");

    double mu         = fitFunc->GetParameter("x0 (Mu)");
    double gaussFWHM  = 2.35482 * fitFunc->GetParameter("Sigma Gauss");
    
    // Evaluate full function FWHM over the fit window (645 keV to 675 keV)
    double xMinFit = 645.0;
    double xMaxFit = 675.0;
    fitFunc->GetRange(xMinFit, xMaxFit);
    double fullFitFWHM = CalculateFunctionFWHM(fitFunc, xMinFit, xMaxFit);
     

    cout << "\n" << " --- " << titleLabel << " ---" << endl;
    cout << "  Centroid (Mu)        : " << mu << " keV" << endl;
    cout << "  Fitted Gaussian FWHM : " << gaussFWHM << " keV" << endl;
    cout << "  Full Fit Function FWHM: " << fullFitFWHM << " keV" << endl;


    delete fitFunc;
    return std::make_tuple(mu, gaussFWHM, fullFitFWHM);
  };
  
  // --- EXECUTE FITS ---
  std::tuple<double, double, double, double> lv_raw  = FitAndPrintSpectrum(histLVInit,  "LV UNCORRECTED (RAW) SPECTRUM");
  std::tuple<double, double, double, double> lv_corr = FitAndPrintSpectrum(histLVFinal, "LV CORRECTED SPECTRUM");

  std::tuple<double, double, double, double> hv_raw  = FitAndPrintSpectrum(histHVInit,  "HV UNCORRECTED (RAW) SPECTRUM");
  std::tuple<double, double, double, double> hv_corr = FitAndPrintSpectrum(histHVFinal, "HV CORRECTED SPECTRUM");

  // Helper lambda to print formatted delta comparison between raw and corrected results
  auto PrintTrappingCorrectionSummary = [](const string& channelLabel, 
                                           const std::tuple<double, double, double, double>& raw, 
                                           const std::tuple<double, double, double, double>& corr) 
  {
    double mu_raw          = std::get<0>(raw);
    double gauss_fwhm_raw  = std::get<1>(raw);
    double full_fwhm_raw   = std::get<2>(raw);

    double mu_corr          = std::get<0>(corr);
    double gauss_fwhm_corr  = std::get<1>(corr);
    double full_fwhm_corr   = std::get<2>(corr);

    // Calculate changes: (Raw - Corrected)
    double lineShift        = mu_corr - mu_raw;
    double deltaGaussFWHM   = std::sqrt(std::pow(gauss_fwhm_raw,2) - std::pow(gauss_fwhm_corr,2));
    double deltaFullFitFWHM = std::sqrt(std::pow(full_fwhm_raw,2) - std::pow(full_fwhm_corr,2));

    cout << "\n=======================================================" << endl;
    cout << "   TRAPPING CORRECTION SUMMARY: " << channelLabel << endl;
    cout << "=======================================================" << endl;
    cout << " Centroid Shift (Delta Mu)      : " << lineShift << " keV (" << mu_raw << " -> " << mu_corr << ")" << endl;
    cout << " Gaussian FWHM Change           : " << deltaGaussFWHM << " keV (" << gauss_fwhm_raw << " -> " << gauss_fwhm_corr << ")" << endl;
    cout << " Full Fit Function FWHM Change  : " << deltaFullFitFWHM << " keV (" << full_fwhm_raw << " -> " << full_fwhm_corr << ")" << endl;
    cout << "=======================================================\n" << endl;
  };

  // --- OUTPUT DELTA COMPARISONS ---
  PrintTrappingCorrectionSummary("LOW VOLTAGE (LV) STRIPS", lv_raw, lv_corr);
  PrintTrappingCorrectionSummary("HIGH VOLTAGE (HV) STRIPS", hv_raw, hv_corr);

  return; 
}
/////////////////////////////////////////////////////////////////////////////////


bool MModuleTrappingCorrection::LoadSimCCEFile(MString FileName)
{
  MFile SimCCEFile;
  if (!SimCCEFile.Open(FileName)) return false;

  m_DetectorParamMap.clear();
  MString Line;
  int currentDetID = -1;

  while (SimCCEFile.ReadLine(Line)) {
    Line = Line.Strip();
    if (Line.IsEmpty()) continue;

    // Detect section header: "### <Detector ID>"
    if (Line.BeginsWith("###")) {
      std::vector<MString> Tokens = Line.Tokenize(" ");
      if (Tokens.size() >= 1) {
        currentDetID = Tokens.back().Strip().ToInt();
      }
      continue;
    }

    if (currentDetID < 0 || Line.BeginsWith('#')) continue;

    std::vector<MString> Tokens = Line.Tokenize(",");
    DetectorTrappingData& det = m_DetectorParamMap[currentDetID];

    // Read parameters (A_HV, A_LV, B, C)
    if (det.m_Depths.empty() && Tokens.size() == 4) {
      det.m_ParamA_HV = Tokens[0].Strip().ToDouble();
      det.m_ParamA_LV = Tokens[1].Strip().ToDouble();
      det.m_ParamB    = Tokens[2].Strip().ToDouble();
      det.m_ParamC    = Tokens[3].Strip().ToDouble();
    }
    // Read CCE depth curves
    else if (Tokens.size() == 5 && !Line.Contains("z_depth")) {
      det.m_Depths.push_back(Tokens[0].Strip().ToDouble());
      det.m_CCEs_HV_e.push_back(Tokens[1].Strip().ToDouble());
      det.m_CCEs_HV_h.push_back(Tokens[2].Strip().ToDouble());
      det.m_CCEs_LV_e.push_back(Tokens[3].Strip().ToDouble());
      det.m_CCEs_LV_h.push_back(Tokens[4].Strip().ToDouble());
    }
  }

  SimCCEFile.Close();
  return !m_DetectorParamMap.empty();
}

/////////////////////////////////////////////////////////////////////////////////

double MModuleTrappingCorrection::GetSimBasedCorrectedEnergy(double depth_val, double uncorrected_energy, const std::vector<double>& depths, const std::vector<double>& sim_cce_sorted_e, const std::vector<double>& sim_cce_sorted_h, double paramB, double paramC)
{
  if (sim_cce_sorted_e.empty() || sim_cce_sorted_h.empty()) {
    return uncorrected_energy;
  }
  double cce_base_e = Interpolate(depth_val, m_Depths, sim_cce_sorted_e);
  double cce_base_h = Interpolate(depth_val, m_Depths, sim_cce_sorted_h);
  
  // Evaluate the physical trapping function model using class global popt variables
  double expected_centroid_scaled =  (1.0 - paramB * (1.0 - cce_base_e)) * (1.0 - paramC * (1.0 - cce_base_h));

  // Prevent division-by-zero or non-physical negative values
  if (expected_centroid_scaled <= 0.0) {
      return uncorrected_energy;
  }

  return uncorrected_energy / expected_centroid_scaled;
}

/////////////////////////////////////////////////////////////////////////////////


double MModuleTrappingCorrection::Interpolate(double x, const std::vector<double>& xp, const std::vector<double>& fp) {
  // need an interpolation function to get continuous CCE values from the discrete simulation data
  if (xp.empty()) return 0.0;
  if (x <= xp.front()) return fp.front();
  if (x >= xp.back()) return fp.back();

  // Find the first element which is greater than or equal to x
  auto it = std::lower_bound(xp.begin(), xp.end(), x);
  size_t idx = std::distance(xp.begin(), it);

  // Linear interpolation formula
  double x0 = xp[idx - 1];
  double x1 = xp[idx];
  double y0 = fp[idx - 1];
  double y1 = fp[idx];

  return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

/////////////////////////////////////////////////////////////////////////////////


void MModuleTrappingCorrection::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
  MGUIOptionsTrappingCorrection* Options = new MGUIOptionsTrappingCorrection(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


/////////////////////////////////////////////////////////////////////////////////


bool MModuleTrappingCorrection::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* SimCCEFileNameNode = Node->GetNode("SimCCEFileName");
  if (SimCCEFileNameNode != nullptr) {
  m_SimCCEFileName = SimCCEFileNameNode->GetValue();
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////////////

MXmlNode* MModuleTrappingCorrection::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0,m_XmlTag);
  new MXmlNode(Node, "SimCCEFileName", m_SimCCEFileName);
  
  return Node;
}

//////////////////////////////////////////////////////////////////////////////


TF1* MModuleTrappingCorrection::GeneratePhotopeakFunction()
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

  return PhotopeakFunction;
}

// MModuleTrappingCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
