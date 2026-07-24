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
  m_SimCCEFileIsLoaded = LoadSimCCEFile(m_SimCCEFile);
  if (m_SimCCEFileIsLoaded == false) {
    return false;
  }
  
  MSupervisor* S = MSupervisor::GetSupervisor();
  m_EnergyCalibration = (MModuleEnergyCalibration*) S->GetAvailableModuleByXmlTag("EnergyCalibration");
  if (m_EnergyCalibration == nullptr) {
    cout << "MModuleTrappingCorrection: couldn't resolve pointer to Energy Calibration Module... need access to this module for energy resolution lookup!" << endl;
    return false;
  }

  m_DepthCalibration = (MModuleDepthCalibration*) S->GetAvailableModuleByXmlTag("DepthCalibration");
  if (m_DepthCalibration == nullptr) {
    cout << "MModuleTrappingCorrection: couldn't resolve pointer to Depth Calibration Module... need access to this module for depth resolution lookup!" << endl;
    return false;
  }


  m_DetectorIDs = m_DepthCalibration-> GetDetectorIDs();

  if (m_DetectorIDs.empty()) {
    cout << "ERROR in MModuleTrappingCorrection::Initialize: Depth Calibration has no registered detector IDs!" << endl;
    return false;
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////

void MModuleTrappingCorrection::CreateExpos()
{
  // Create all expos

  if (HasExpos() == true) return;

  // Set the histogram display
  m_ExpoSpectrum = new MGUIExpoPlotSpectrum(this);
  m_ExpoSpectrum->SetEnergyHistogramParameters(200, 0, 2000);
  m_Expos.push_back(m_ExpoSpectrum);


}


/////////////////////////////////////////////////////////////////////////////////


bool MModuleTrappingCorrection::AnalyzeEvent(MReadOutAssembly* Event) 
{
  
  if (Event->GetGuardRingVeto() == true) {
    //Right now we cannot use events w GR veto 
    
    // Event->SetTrappingCorrectionError("GR Veto");
    return false;
  
  } else {
    
    for (unsigned int i = 0; i < Event->GetNHits(); ++i ){
      // Each event represents one photon. It contains Hits, representing interaction sites.
      // H is a pointer to an instance of the MHit class. Each Hit has activated strips, represented by
      // instances of the MStripHit class.
      MHit* H = Event->GetHit(i);

      int Grade = m_DepthCalibration->GetHitGrade(H);

      // Handle different grades differently  
      // Get the position from the depth cal. If error is thrown, record and no depth.

      // GRADE=-1 is an error. Break from the loop and continue.
      if (Grade < 0){
        H->SetNoDepth();
      } else if (Grade > 4) { // GRADE=5 is some complicated geometry with multiple hits on a single strip. GRADE=6 means not all strips are adjacent.
        H->SetNoDepth();
      } else { // If the Grade is 0-4, we can handle it.

        // Take a Hit and separate its activated X- and Y-strips into separate vectors.
        vector<MStripHit*> LVStrips;
        vector<MStripHit*> HVStrips;

        for (unsigned int j = 0; j < H->GetNStripHits(); ++j) {
          MStripHit* SH = H->GetStripHit(j);
          if (SH->IsLowVoltageStrip()) LVStrips.push_back(SH); else HVStrips.push_back(SH);
        }

        // Get the dominant strip for the hit and its energy fraction for both LV and HV sides
        //s.t we cna determine the depth anf energy to correct
        double LVEnergyFraction;
        double HVEnergyFraction;
        MStripHit* LVSH = m_DepthCalibration->GetDominantStrip(LVStrips, LVEnergyFraction); 
        MStripHit* HVSH = m_DepthCalibration->GetDominantStrip(HVStrips, HVEnergyFraction); 

        // Get the position value (assumed from event/hit context H)
        double depth_val = H->GetPosition().GetZ();
        // cout<<"Depth value: "<<depth_val<<endl;
        // double depth_val = static_cast<double>(Zpos);

        // Correct the Low Voltage side energy if the hit pointer exists
        if (LVSH != nullptr) {
            double rawLVEnergy = LVSH->GetEnergy(); 
            double correctedLVEnergy = GetSimBasedCorrectedEnergy(depth_val, rawLVEnergy, m_CCEs_LV, m_ParamA_LV, m_ParamB_LV, m_ParamC_LV);
            LVSH->SetEnergy(correctedLVEnergy);    

            if (HasExpos() == true) {
              m_ExpoSpectrum->AddEnergyFinal(correctedLVEnergy, LVSH->IsNearestNeighbor(), LVSH->IsLowVoltageStrip());
            }
        }

        // Correct the High Voltage side energy if the hit pointer exists
        if (HVSH != nullptr) {
            double rawHVEnergy = HVSH->GetEnergy(); 
            double correctedHVEnergy = GetSimBasedCorrectedEnergy(depth_val, rawHVEnergy, m_CCEs_HV, m_ParamA_HV, m_ParamB_HV, m_ParamC_HV);
            HVSH->SetEnergy(correctedHVEnergy);    

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

  // Locate the histograms dynamically from ROOT's global memory map
  TH1D* histLV = (TH1D*) gDirectory->Get("EnergyHistogramLVFinal");
  TH1D* histHV = (TH1D*) gDirectory->Get("EnergyHistogramHVFinal");

  if (histLV == nullptr) histLV = (TH1D*) gDirectory->Get("m_EnergyHistogramLVFinal");
  if (histHV == nullptr) histHV = (TH1D*) gDirectory->Get("m_EnergyHistogramHVFinal");

  // Perform the photopeak fit and direct calculation for the LV peak
  if (histLV != nullptr && histLV->GetEntries() > 0) {
    // Calculate raw FWHM directly from the histogram
    m_DirectFWHM_LV = CalculateDirectFWHM(histLV);

    TF1* fitFuncLV = GeneratePhotopeakFunction();
    fitFuncLV->SetParameter("Amplitude", histLV->GetBinContent(histLV->GetMaximumBin()));
    histLV->Fit(fitFuncLV, "RQ");
    
    double mu = fitFuncLV->GetParameter("x0 (Mu)");
    double fwhm_fit = 2.35482 * fitFuncLV->GetParameter("Sigma Gauss");
    
    if (g_Verbosity >= c_Info) {
      cout << m_XmlTag << " --- LV FINAL SPECTRUM RESULTS ---" << endl;
      cout << "  Centroid (Mu)        : " << mu << " keV" << endl;
      cout << "  Fitted Gaussian FWHM : " << fwhm_fit << " keV" << endl;
      cout << "  Direct Histogram FWHM: " << m_DirectFWHM_LV << " keV" << endl;
    }
    delete fitFuncLV;
  }

  // Perform the photopeak fit and direct calculation for the HV peak
  if (histHV != nullptr && histHV->GetEntries() > 0) {
    // Calculate raw FWHM directly from the histogram
    m_DirectFWHM_HV = CalculateDirectFWHM(histHV);

    TF1* fitFuncHV = GeneratePhotopeakFunction();
    fitFuncHV->SetParameter("Amplitude", histHV->GetBinContent(histHV->GetMaximumBin()));
    histHV->Fit(fitFuncHV, "RQ");
    
    double mu = fitFuncHV->GetParameter("x0 (Mu)");
    double fwhm_fit = 2.35482 * fitFuncHV->GetParameter("Sigma Gauss");
    
    if (g_Verbosity >= c_Info) {
      cout << m_XmlTag << " --- HV FINAL SPECTRUM RESULTS ---" << endl;
      cout << "  Centroid (Mu)        : " << mu << " keV" << endl;
      cout << "  Fitted Gaussian FWHM : " << fwhm_fit << " keV" << endl;
      cout << "  Direct Histogram FWHM: " << m_DirectFWHM_HV << " keV" << endl;
    }
    delete fitFuncHV;
  }

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

  // Clear existing array data before loading new files
  m_Depths.clear();
  m_CCEs_HV.clear();
  m_CCEs_LV.clear();

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
      // Read parameters A, B, and C from the first line
      if (Tokens.size() == 6) {
        m_ParamA_HV = Tokens[0].ToDouble();
        m_ParamB_HV = Tokens[1].ToDouble();
        m_ParamC_HV = Tokens[2].ToDouble();
        m_ParamA_LV = Tokens[3].ToDouble();
        m_ParamB_LV = Tokens[4].ToDouble();
        m_ParamC_LV = Tokens[5].ToDouble();
        ValidLineCount++;
      } else {
        cout << "ERROR in LoadSimCCEFile: Expected 6 parameters (A,B,C for HV, LV) on the first line." << endl;
        SimCCEFile.Close();
        return false;
      }
    } 
    else if (ValidLineCount == 1) {
      // Skip the second line which is the column header (z_depth_mm,CCE_HV)
      ValidLineCount++;
    } 
    else {
      // Read the rest of the rows into your depth and CCE HV arrays
      if (Tokens.size() == 3) {
        m_Depths.push_back(Tokens[0].ToDouble());
        m_CCEs_HV.push_back(Tokens[1].ToDouble());
        m_CCEs_LV.push_back(Tokens[2].ToDouble());
        ValidLineCount++;
      }
    }
  }

  SimCCEFile.Close();

  // Print summary to console if verbose logging is enabled
  if (g_Verbosity >= c_Info) {
    cout << m_XmlTag << "Loaded HV parameters: A=" << m_ParamA_HV 
         << ", B=" << m_ParamB_HV << ", C=" << m_ParamC_HV << endl;
    cout << m_XmlTag << "Loaded " << m_Depths.size() << " data points into arrays." << endl;
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////////////

double MModuleTrappingCorrection::GetSimBasedCorrectedEnergy(double depth_val, double uncorrected_energy, const std::vector<double>& sim_cce_sorted, double paramA, double paramB, double paramC) {

  // Look up the simulation CCE baseline using the interpolate function
  double cce_base = Interpolate(depth_val, m_Depths, sim_cce_sorted);

  // Evaluate the physical trapping function model using class global popt variables
  double expected_centroid_scaled = paramA * (1.0 - paramB * (1.0 - cce_base)) * (1.0 - paramC * (1.0 - cce_base));

  // Prevent division-by-zero or non-physical negative values
  if (expected_centroid_scaled <= 0.0) {
      return uncorrected_energy;
  }

  //Reconstruct the true un-trapped energy 
  return uncorrected_energy / expected_centroid_scaled;
}


/////////////////////////////////////////////////////////////////////////////////


double MModuleTrappingCorrection::Interpolate(double x, const std::vector<double>& xp, const std::vector<double>& fp) {
  // need an interpolation function to get continuour CCE values from the discrete simulation data
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
  m_SimCCEFile = SimCCEFileNameNode->GetValue();
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////////////

MXmlNode* MModuleTrappingCorrection::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0,m_XmlTag);
  new MXmlNode(Node, "SimCCEFileName", m_SimCCEFile);
  
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

  // 1. Find the maximum bin and its half-maximum height
  int maxBin = hist->GetMaximumBin();
  double halfMax = hist->GetBinContent(maxBin) / 2.0;

  if (halfMax <= 0.0) return 0.0;

  // 2. Search LEFT for the half-maximum crossing point
  double xLeft = hist->GetBinCenter(1);
  for (int b = maxBin; b >= 1; --b) {
    if (hist->GetBinContent(b) <= halfMax) {
      double x1 = hist->GetBinCenter(b);
      double y1 = hist->GetBinContent(b);
      double x2 = hist->GetBinCenter(b + 1);
      double y2 = hist->GetBinContent(b + 1);

      // Linear interpolation between bins
      xLeft = (y2 != y1) ? x1 + (halfMax - y1) * (x2 - x1) / (y2 - y1) : x1;
      break;
    }
  }

  // 3. Search RIGHT for the half-maximum crossing point
  double xRight = hist->GetBinCenter(hist->GetNbinsX());
  for (int b = maxBin; b <= hist->GetNbinsX(); ++b) {
    if (hist->GetBinContent(b) <= halfMax) {
      double x1 = hist->GetBinCenter(b - 1);
      double y1 = hist->GetBinContent(b - 1);
      double x2 = hist->GetBinCenter(b);
      double y2 = hist->GetBinContent(b);

      // Linear interpolation between bins
      xRight = (y2 != y1) ? x1 + (halfMax - y1) * (x2 - x1) / (y2 - y1) : x2;
      break;
    }
  }

  // 4. Return total width at half maximum
  return (xRight - xLeft);
}

////////////////////////////////////////////////////////////////////////////////


// MModuleTrappingCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
