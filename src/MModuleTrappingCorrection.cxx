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
  m_XmlTag = "TrappingCorrection";

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


  m_DetectorIDs = m_DepthCalibration-> GetDetectorIDs();

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
  if (Event->GetGuardRingVeto() == true) {
    // Right now we cannot use events w GR veto 
    return false;
  } else {
    for (unsigned int i = 0; i < Event->GetNHits(); ++i) {
      MHit* H = Event->GetHit(i);

      int Grade = m_DepthCalibration->GetHitGrade(H);

      if (Grade < 0 || Grade > 4) {
        H->SetNoDepth();
      } else { // If Grade is 0-4, proceed with analysis

        vector<MStripHit*> LVStrips;
        vector<MStripHit*> HVStrips;

        for (unsigned int j = 0; j < H->GetNStripHits(); ++j) {
          MStripHit* SH = H->GetStripHit(j);
          if (SH->IsLowVoltageStrip()) LVStrips.push_back(SH); else HVStrips.push_back(SH);
        }

        double LVEnergyFraction;
        double HVEnergyFraction;
        MStripHit* LVSH = m_DepthCalibration->GetDominantStrip(LVStrips, LVEnergyFraction); 
        MStripHit* HVSH = m_DepthCalibration->GetDominantStrip(HVStrips, HVEnergyFraction); 

        // Local Z depth position
        double depth_val = H->GetLocalPosition().GetZ();

        // --- Low Voltage Side ---
        if (LVSH != nullptr) {
          double rawLVEnergy = LVSH->GetEnergy(); 
      
          // --- 1. FILL UNCORRECTED (RAW) ---
          if (HasExpos() == true) {
            m_ExpoSpectrum->AddEnergyInitial(rawLVEnergy, LVSH->IsNearestNeighbor(), LVSH->IsLowVoltageStrip());
          }
      
          // --- 2. CALCULATE CORRECTION ---
          double correctedLVEnergy = GetSimBasedCorrectedEnergy(depth_val, rawLVEnergy, m_CCEs_LV_e, m_CCEs_LV_h, m_ParamA_LV, m_ParamB, m_ParamC);
          LVSH->SetEnergy(correctedLVEnergy);    
      
          // --- 3. FILL CORRECTED (FINAL) ---
          if (HasExpos() == true) {
            m_ExpoSpectrum->AddEnergyFinal(correctedLVEnergy, LVSH->IsNearestNeighbor(), LVSH->IsLowVoltageStrip());
          }
      }

        // --- High Voltage Side ---
        if (HVSH != nullptr) {
            double rawHVEnergy = HVSH->GetEnergy(); 

            // 1. Record UNCORRECTED (raw) HV energy to expo spectrum
            if (HasExpos() == true) {
              m_ExpoSpectrum->AddEnergyInitial(rawHVEnergy, HVSH->IsNearestNeighbor(), HVSH->IsLowVoltageStrip());
            }

            // 2. Compute trapping correction
            double correctedHVEnergy = GetSimBasedCorrectedEnergy(depth_val, rawHVEnergy, m_CCEs_HV_e, m_CCEs_HV_h, m_ParamA_HV, m_ParamB, m_ParamC);
            HVSH->SetEnergy(correctedHVEnergy);    

            // 3. Record CORRECTED (final) HV energy to expo spectrum
            if (HasExpos() == true) {
              m_ExpoSpectrum->AddEnergyFinal(correctedHVEnergy, HVSH->IsNearestNeighbor(), HVSH->IsLowVoltageStrip());
            }
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
    cout << "ERROR in MModuleTrappingCorrection::Finalize: Expo plot spectrum is null." << endl;
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

  // Helper lambda to perform fit, output results, and return {mu, fwhm}
  auto FitAndPrintSpectrum = [&](TH1D* hist, const string& titleLabel) -> std::pair<double, double> {
    if (hist == nullptr || hist->GetEntries() <= 0) {
      cout << "WARNING: " << titleLabel << " histogram is null or has 0 entries." << endl;
      return std::make_pair(0.0, 0.0);
    }

    double directFWHM = CalculateDirectFWHM(hist);

    TF1* fitFunc = GeneratePhotopeakFunction();
    fitFunc->SetParameter("Amplitude", hist->GetBinContent(hist->GetMaximumBin()));
    hist->Fit(fitFunc, "RQ");

    double mu   = fitFunc->GetParameter("x0 (Mu)");
    double fwhm = 2.35482 * fitFunc->GetParameter("Sigma Gauss");

    cout << "\n" << m_XmlTag << " --- " << titleLabel << " ---" << endl;
    cout << "  Centroid (Mu)        : " << mu << " keV" << endl;
    cout << "  Fitted Gaussian FWHM : " << fwhm << " keV" << endl;
    cout << "  Direct Histogram FWHM: " << directFWHM << " keV" << endl;

    delete fitFunc;
    return std::make_pair(mu, fwhm);
  };

  cout << "\n========================================================" << endl;
  cout << "      TRAPPING CORRECTION SPECTRUM FIT RESULTS          " << endl;
  cout << "========================================================" << endl;

  // --- Fit Uncorrected and Corrected Spectra ---
  std::pair<double, double> lv_raw   = FitAndPrintSpectrum(histLVInit,  "LV UNCORRECTED (RAW) SPECTRUM");
  std::pair<double, double> lv_corr  = FitAndPrintSpectrum(histLVFinal, "LV CORRECTED SPECTRUM");

  std::pair<double, double> hv_raw   = FitAndPrintSpectrum(histHVInit,  "HV UNCORRECTED (RAW) SPECTRUM");
  std::pair<double, double> hv_corr  = FitAndPrintSpectrum(histHVFinal, "HV CORRECTED SPECTRUM");

  // Summary Comparison Output
  cout << "\n========================================================" << endl;
  cout << "                 SUMMARY COMPARISON                     " << endl;
  cout << "========================================================" << endl;
  cout << " LV Side: " << endl;
  cout << "   Raw Centroid: " << lv_raw.first << " keV | Corrected Centroid: " << lv_corr.first << " keV" << endl;
  cout << "   Raw FWHM    : " << lv_raw.second << " keV | Corrected FWHM    : " << lv_corr.second << " keV" << endl;
  cout << " HV Side: " << endl;
  cout << "   Raw Centroid: " << hv_raw.first << " keV | Corrected Centroid: " << hv_corr.first << " keV" << endl;
  cout << "   Raw FWHM    : " << hv_raw.second << " keV | Corrected FWHM    : " << hv_corr.second << " keV" << endl;
  cout << "========================================================\n" << endl;

  return; 
}

/////////////////////////////////////////////////////////////////////////////////


bool MModuleTrappingCorrection::LoadSimCCEFile(MString FileName)
{
  MFile SimCCEFile;
  if (SimCCEFile.Open(FileName) == false) {
    cout << "ERROR in MModuleTrappingCorrection::LoadSimCCEFile: failed to open file." << endl;
    return false;
  }

  m_Depths.clear();
  m_CCEs_HV_e.clear();
  m_CCEs_HV_h.clear();
  m_CCEs_LV_e.clear();
  m_CCEs_LV_h.clear();

  MString Line;
  int ValidLineCount = 0;

  while (SimCCEFile.ReadLine(Line)) {
    // Skip comment lines
    if (Line.BeginsWith('#') == true) {
      continue;
    }

    std::vector<MString> Tokens = Line.Tokenize(",");
    
    // Skip empty lines safely
    if (Tokens.size() == 0) {
      continue;
    }

    if (ValidLineCount == 0) {
      //Read parameters A_HV, A_LV, B, and C from the first line
      if (Tokens.size() == 4) {
        m_ParamA_HV = Tokens[0].ToDouble();
        m_ParamA_LV = Tokens[1].ToDouble();
        m_ParamB    = Tokens[2].ToDouble();
        m_ParamC    = Tokens[3].ToDouble();

        ValidLineCount++;
      } else {
        if (g_Verbosity >= c_Error) {
          cout << "ERROR in LoadSimCCEFile: Expected 4 parameters (A_HV, A_LV, B, and C) on the first line." << endl;
        }
        SimCCEFile.Close();
        return false;
      }
    }  
    else {
      //Read the 5 data columns into depth and CCE arrays
      if (Tokens.size() == 5) {
        m_Depths.push_back(Tokens[0].ToDouble());
        m_CCEs_HV_e.push_back(Tokens[1].ToDouble());
        m_CCEs_HV_h.push_back(Tokens[2].ToDouble());
        m_CCEs_LV_e.push_back(Tokens[3].ToDouble());
        m_CCEs_LV_h.push_back(Tokens[4].ToDouble());
        
        ValidLineCount++;
      } else {
        if (g_Verbosity >= c_Error) {
          cout << "ERROR in LoadSimCCEFile: Expected 5 columns on line " << (ValidLineCount + 1) << endl;
        }  
        SimCCEFile.Close();
        return false;
      }
    }
  }

  SimCCEFile.Close();

  // Print summary to console if verbose logging is enabled
  if (g_Verbosity >= c_Info) {
    cout << m_XmlTag << "Loaded parameters: A_HV=" << m_ParamA_HV << ", A_LV=" << m_ParamA_LV
         << ", B=" << m_ParamB << ", C=" << m_ParamC << endl;
    cout << m_XmlTag << "Loaded " << m_Depths.size() << " data points into arrays." << endl;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////

double MModuleTrappingCorrection::GetSimBasedCorrectedEnergy(double depth_val, double uncorrected_energy, const std::vector<double>& sim_cce_sorted_e, const std::vector<double>& sim_cce_sorted_h, double paramA, double paramB, double paramC) {

  // Look up the simulation CCE baseline using the interpolate function
  
  double cce_base_e = Interpolate(depth_val, m_Depths, sim_cce_sorted_e);
  double cce_base_h = Interpolate(depth_val, m_Depths, sim_cce_sorted_h);
  

  // Evaluate the physical trapping function model using class global popt variables
  double expected_centroid_scaled = paramA * (1.0 - paramB * (1.0 - cce_base_e)) * (1.0 - paramC * (1.0 - cce_base_h));

  // Prevent division-by-zero or non-physical negative values
  if (expected_centroid_scaled <= 0.0) {
      return uncorrected_energy;
  }

  //Reconstruct the true un-trapped energy 
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


////////////////////////////////////////////////////////////////////////////////
double MModuleTrappingCorrection::CalculateDirectFWHM(TH1D* hist)
{
  if (hist == nullptr || hist->GetEntries() == 0) return 0.0;

  //Locate the peak bin
  int maxBin = hist->GetMaximumBin();

  if (g_Verbosity >= c_Info){
    cout << "INFO: Maximum bin located at: " << maxBin << " with content: " << hist->GetBinContent(maxBin) << endl;
  }
  
  double peakHeight = hist->GetBinContent(maxBin);
  if (peakHeight <= 0.0) return 0.0;

  //Define a local search window (depends on binning you set on the GUI)
  // Set the binning in the GUI to 0.1 keV before finalize function is called
  const int searchWindowBins = 150; 
  
  int minSearchBin = std::max(1, maxBin - searchWindowBins);
  int maxSearchBin = std::min(hist->GetNbinsX(), maxBin + searchWindowBins);

  //Estimate local background level from the window edges
  double bgLeft  = hist->GetBinContent(minSearchBin);
  double bgRight = hist->GetBinContent(maxSearchBin);
  double localBG =  0.0; // assume the background is  zero for now
  //(bgLeft + bgRight) / 2.0;

  // Calculate net peak height above background
  double netPeakHeight = peakHeight - localBG;
  if (netPeakHeight <= 0.0) return 0.0;

  // Target level is half-maximum relative to local background
  double targetHalfMax = localBG + (netPeakHeight / 2.0);

  //Search left within the restricted window
  double xLeft = -1.0;
  for (int b = maxBin; b >= minSearchBin; --b) {
    if (hist->GetBinContent(b) <= targetHalfMax) {
      double x1 = hist->GetBinCenter(b);
      double y1 = hist->GetBinContent(b);
      double x2 = hist->GetBinCenter(b + 1);
      double y2 = hist->GetBinContent(b + 1);

      // Linear interpolation between adjacent bins
      xLeft = (y2 != y1) ? x1 + (targetHalfMax - y1) * (x2 - x1) / (y2 - y1) : x1;
      break;
    }
  }

  //Search right within the restricted window
  double xRight = -1.0;
  for (int b = maxBin; b <= maxSearchBin; ++b) {
    if (hist->GetBinContent(b) <= targetHalfMax) {
      double x1 = hist->GetBinCenter(b - 1);
      double y1 = hist->GetBinContent(b - 1);
      double x2 = hist->GetBinCenter(b);
      double y2 = hist->GetBinContent(b);

      // Linear interpolation between adjacent bins
      xRight = (y2 != y1) ? x1 + (targetHalfMax - y1) * (x2 - x1) / (y2 - y1) : x2;
      break;
    }
  }

  // Ensure valid crossing points were found on both sides
  if (xLeft < 0.0 || xRight < 0.0) {
    if (g_Verbosity >= c_Warning) {
      cout << "WARNING in CalculateDirectFWHM: Peak did not cross half-maximum inside local window." << endl;
    }
    return 0.0;
  }

  return (xRight - xLeft);
}
////////////////////////////////////////////////////////////////////////////////


// MModuleTrappingCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
