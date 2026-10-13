/*
 * MModuleTrappingCorrection.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer and Sophie Haight.				
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
  AddPreceedingModuleType(MAssembly::c_TACCalibration, true);
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
  m_TrappingCorrectionFileIsLoaded = LoadTrappingCorrectionFile(m_TrappingCorrectionFileName);
  if (m_TrappingCorrectionFileIsLoaded == false) {
    return false;
  }
  
  MSupervisor* S = MSupervisor::GetSupervisor();
  
  m_DepthCalibration = (MModuleDepthCalibration*) S->GetAvailableModuleByXmlTag("XmlTagDepthCalibration");
  if (m_DepthCalibration == nullptr) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR in MModuleTrappingCorrection::Initialize: couldn't resolve pointer to Depth Calibration Module... need access to this module for depth resolution lookup!" << endl;
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
    return false;
  } 

  for (unsigned int i = 0; i < Event->GetNHits(); ++i) {
    MHit* H = Event->GetHit(i);

    // Skip hits that don't have a depth value
    if (H->GetNoDepth() == true) {
      continue;
    }

    // Local Z depth position (in cm) from the hit
    double DepthVal = H->GetLocalPosition().GetZ();

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
        double MinParamDepth = std::min(detData.m_Depths.front(), detData.m_Depths.back());
        double MaxParamDepth = std::max(detData.m_Depths.front(), detData.m_Depths.back());
        bool IsDepthOutofBounds = (DepthVal < MinParamDepth || DepthVal > MaxParamDepth);

        double rawLVEnergy = LVSH->GetEnergy(); 

        if (HasExpos() == true) {
          m_ExpoSpectrum->AddEnergyInitial(rawLVEnergy, LVSH->IsNearestNeighbor(), LVSH->IsLowVoltageStrip());
        }

        double correctedLVEnergy = rawLVEnergy;
        if (IsDepthOutofBounds == false) {
          correctedLVEnergy = GetSimBasedCorrectedEnergy(DepthVal, rawLVEnergy, detData.m_Depths, detData.m_CCEs_LV_e, detData.m_CCEs_LV_h, detData.m_ParamB, detData.m_ParamC);
        } else {
          if (DepthVal < MinParamDepth) {
            correctedLVEnergy = GetSimBasedCorrectedEnergy(MinParamDepth, rawLVEnergy, detData.m_Depths, detData.m_CCEs_LV_e, detData.m_CCEs_LV_h, detData.m_ParamB, detData.m_ParamC);
          } else if (DepthVal > MaxParamDepth) {
            correctedLVEnergy = GetSimBasedCorrectedEnergy(MaxParamDepth, rawLVEnergy, detData.m_Depths, detData.m_CCEs_LV_e, detData.m_CCEs_LV_h, detData.m_ParamB, detData.m_ParamC); 
          }
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
        double MinParamDepth = std::min(detData.m_Depths.front(), detData.m_Depths.back());
        double MaxParamDepth = std::max(detData.m_Depths.front(), detData.m_Depths.back());
        bool IsDepthOutofBounds = (DepthVal < MinParamDepth || DepthVal > MaxParamDepth);
    

        double rawHVEnergy = HVSH->GetEnergy(); 
        
        if (HasExpos() == true) {
          m_ExpoSpectrum->AddEnergyInitial(rawHVEnergy, HVSH->IsNearestNeighbor(), HVSH->IsLowVoltageStrip());
        }
        double correctedHVEnergy = rawHVEnergy;
        if (IsDepthOutofBounds == false) {
          correctedHVEnergy = GetSimBasedCorrectedEnergy( DepthVal, rawHVEnergy, detData.m_Depths, detData.m_CCEs_HV_e, detData.m_CCEs_HV_h, detData.m_ParamB, detData.m_ParamC);
        } else {
          if (DepthVal < MinParamDepth) {
            correctedHVEnergy = GetSimBasedCorrectedEnergy(MinParamDepth, rawHVEnergy, detData.m_Depths, detData.m_CCEs_HV_e, detData.m_CCEs_HV_h, detData.m_ParamB, detData.m_ParamC);
          } else if (DepthVal > MaxParamDepth) {
            correctedHVEnergy = GetSimBasedCorrectedEnergy(MaxParamDepth, rawHVEnergy, detData.m_Depths, detData.m_CCEs_HV_e, detData.m_CCEs_HV_h, detData.m_ParamB, detData.m_ParamC); 
          }
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

  return; 
}
/////////////////////////////////////////////////////////////////////////////////


bool MModuleTrappingCorrection::LoadTrappingCorrectionFile(MString FileName)
{
  MFile TrappingCorrectionFile;
  if (TrappingCorrectionFile.Open(FileName) == false) {
    if (g_Verbosity >= c_Error) {
      cout << m_XmlTag << ": ERROR: Could not open CCE file " << FileName << endl;
    }
    return false;
  }

  m_DetectorParamMap.clear();
  MString Line;
  int currentDetID = -1;
  std::set<MString> SeenTokens; // track detector IDs to avoid duplicates

  while (TrappingCorrectionFile.ReadLine(Line)) {
    Line = Line.Strip();
    if (Line.IsEmpty() == true) continue;

    // Detect section header: "### <Detector ID>"
    if (Line.BeginsWith("###")) {
      std::vector<MString> Tokens = Line.Tokenize(" ");
      if (Tokens.size() >= 2) {
        currentDetID = Tokens.back().Strip().ToInt(); 

        MString TargetToken = Tokens[1]; 
        if (SeenTokens.find(TargetToken) != SeenTokens.end()) {
          if (g_Verbosity >= c_Error) {
            cout << m_XmlTag << ": ERROR: Duplicate token '" << TargetToken << "'" << endl;
          }
          return false; 
        } 
        SeenTokens.insert(TargetToken);
      }
      continue;
    }

    // Skip comment lines or lines before the first detector section
    if (currentDetID < 0 || Line.BeginsWith('#')) continue;

    // Tokenize using only commas as delimiters 
    std::vector<MString> Tokens = Line.Tokenize(",", false);
    DetectorTrappingData& det = m_DetectorParamMap[currentDetID];

    // Read parameters (A_HV, A_LV, B, C)
    if (det.m_Depths.empty() && Tokens.size() == 4) {
      det.m_ParamA_HV = Tokens[0].Strip().ToDouble();
      det.m_ParamA_LV = Tokens[1].Strip().ToDouble();
      det.m_ParamB    = Tokens[2].Strip().ToDouble();
      det.m_ParamC    = Tokens[3].Strip().ToDouble();
    }
    // Read CCE depth curves
    else if (Tokens.size() == 5) {
      det.m_Depths.push_back(Tokens[0].Strip().ToDouble());
      det.m_CCEs_HV_e.push_back(Tokens[1].Strip().ToDouble());
      det.m_CCEs_HV_h.push_back(Tokens[2].Strip().ToDouble());
      det.m_CCEs_LV_e.push_back(Tokens[3].Strip().ToDouble());
      det.m_CCEs_LV_h.push_back(Tokens[4].Strip().ToDouble());
    }
  }

  TrappingCorrectionFile.Close();
  if (m_DetectorParamMap.empty() == true) {
    if (g_Verbosity >= c_Error) {
      cout << m_XmlTag << ": ERROR: Obtained an empty charge trapping parameter map when reading CCE file " << FileName << endl;
    }
    return false;
  }

  return true;
}

/////////////////////////////////////////////////////////////////////////////////

double MModuleTrappingCorrection::GetSimBasedCorrectedEnergy(double DepthVal, double UncorrectedEnergy, const std::vector<double>& depths, const std::vector<double>& SimCCESortedE, const std::vector<double>& SimCCESortedH, double paramB, double paramC)
{
  if (SimCCESortedE.empty() == true || SimCCESortedH.empty() == true) {
    return UncorrectedEnergy;
  }
  double CCEBaseE = Interpolate(DepthVal, depths, SimCCESortedE);
  double CCEBaseH = Interpolate(DepthVal, depths, SimCCESortedH);
  
  // Evaluate the physical trapping function model using class global popt variables
  double ExpectedCentroidScaled =  (1.0 - paramB * (1.0 - CCEBaseE)) * (1.0 - paramC * (1.0 - CCEBaseH));

  // Prevent division-by-zero or non-physical negative values
  if (ExpectedCentroidScaled <= 0.0) {
      return UncorrectedEnergy;
  }

  return UncorrectedEnergy / ExpectedCentroidScaled;
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

  MXmlNode* TrappingCorrectionFileNameNode = Node->GetNode("TrappingCorrectionFileName");
  if (TrappingCorrectionFileNameNode != nullptr) {
    m_TrappingCorrectionFileName = TrappingCorrectionFileNameNode->GetValue();
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////////////

MXmlNode* MModuleTrappingCorrection::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0,m_XmlTag);
  new MXmlNode(Node, "TrappingCorrectionFileName", m_TrappingCorrectionFileName);
  
  return Node;
}

// MModuleTrappingCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
