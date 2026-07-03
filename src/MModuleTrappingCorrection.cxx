/*
 * MModuleTrappingCorrection.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer, Alex Lowell, 
 * 				Sean Pike, Carolyn Kierans.
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
// #include "MGUIOptionsTrappingCorrection.h"

// Standard libs:

// ROOT libs:
#include "TMath.h"
#include "TGClient.h"
#include "TH1.h"

// MEGAlib libs:


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
  AddPreceedingModuleType(MAssembly::c_DepthCalibration, true);
//  AddPreceedingModuleType(MAssembly::c_CrosstalkCorrection, false); // Soft requirement

  // Set all types this modules handles
  AddModuleType(MAssembly::c_TrappingCorrection);
  AddModuleType(MAssembly::c_PositionDetermiation);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
  
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

  // The detectors need to be in the same order as DetIDs.
  // ie DetID=0 should be the 0th detector in m_Detectors, DetID=1 should the 1st, etc.
  vector<MDDetector*> DetList = m_Geometry->GetDetectorList();

  // Look through the Geometry and get the names and thicknesses of all the detectors.
  for (unsigned int i = 0; i < DetList.size(); ++i) {
    // For now, DetID is in order of detectors, which puts contraints on how the geometry file should be written.
    // If using the card cage at UCSD, default to DetID=11.
    unsigned int DetID = i;
    if (m_UCSDOverride == true) {
      DetID = 11;
    }

    MDDetector* det = DetList[i];
    vector<string> DetectorNames;
    if (det->GetTypeName() == "Strip3D") {
      if (det->GetNSensitiveVolumes() == 1) {
        MDVolume* vol = det->GetSensitiveVolume(0);
        string det_name = vol->GetName().GetString();
        if (find(DetectorNames.begin(), DetectorNames.end(), det_name) == DetectorNames.end()) {
          DetectorNames.push_back(det_name);
          m_Thicknesses[DetID] = 2 * (det->GetStructuralSize().GetZ());
          MDStrip3D* strip = dynamic_cast<MDStrip3D*>(det);
          m_XPitches[DetID] = strip->GetPitchX();
          m_YPitches[DetID] = strip->GetPitchY();
          m_NXStrips[DetID] = strip->GetNStripsX();
          m_NYStrips[DetID] = strip->GetNStripsY();

          if (g_Verbosity >= c_Info) {
            cout << "Found detector " << det_name << " corresponding to DetID=" << DetID << "." << endl;
            cout << "Detector thickness: " << m_Thicknesses[DetID] << endl;
            cout << "Number of X strips: " << m_NXStrips[DetID] << endl;
            cout << "Number of Y strips: " << m_NYStrips[DetID] << endl;
            cout << "X strip pitch: " << m_XPitches[DetID] << endl;
            cout << "Y strip pitch: " << m_YPitches[DetID] << endl;
          }
          m_DetectorIDs.push_back(DetID);
          m_Detectors[DetID] = det;
        } else {
          if (g_Verbosity >= c_Error) {
            cout<<"ERROR in MModuleTrappingCorrection::Initialize: Found a duplicate detector: "<<det_name<<endl;
          }
        }
      } else {
        if (g_Verbosity >= c_Error) {
          cout<<"ERROR in MModuleTrappingCorrection::Initialize: Found a Strip3D detector with "<<det->GetNSensitiveVolumes()<<" Sensitive Volumes."<<endl;
        }
      }
    }
  }

  if (m_DetectorIDs.size() == 0) {
    cout<<"No Strip3D detectors were found."<<endl;
    return false; 
  }

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

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////

// void MModuleTrappingCorrection::CreateExpos()
// {
//   // Create all expos

//   if (HasExpos() == true) return;

//   // Set the histogram display
//   m_ExpoTrappingCorrection = new MGUIExpoTrappingCorrection(this);
//   m_ExpoTrappingCorrection->SetDepthHistogramArrangement(&m_DetectorIDs);
//   for (unsigned int i = 0; i < m_DetectorIDs.size(); ++i){
//     unsigned int DetID = m_DetectorIDs[i];
//     double thickness = m_Thicknesses[DetID];
//     m_ExpoTrappingCorrection->SetDepthHistogramParameters(DetID, 120, -thickness/2.0,thickness/2.0);
//   }
//   m_Expos.push_back(m_ExpoTrappingCorrection);
// }


/////////////////////////////////////////////////////////////////////////////////


bool MModuleTrappingCorrection::AnalyzeEvent(MReadOutAssembly* Event) 
{
  
  if (Event->GetGuardRingVeto() == true) {
    //TODO: Handle events with GR vetos    
    
    Event->SetTrappingCorrectionError("GR Veto");
    return false;
  
  } else {
    
    for (unsigned int i = 0; i < Event->GetNHits(); ++i ){
      // Each event represents one photon. It contains Hits, representing interaction sites.
      // H is a pointer to an instance of the MHit class. Each Hit has activated strips, represented by
      // instances of the MStripHit class.
      MHit* H = Event->GetHit(i);

      int Grade = GetHitGrade(H);

      // Handle different grades differently    
      // GRADE=-1 is an error. Break from the loop and continue.
      if (Grade < 0){
        H->SetNoDepth();
        Event->SetTrappingCorrectionError("Error in Trapping Correction");
        if (Grade == -1) {
          ++m_ErrorSH;
        } else if (Grade == -2) {
          ++m_ErrorNullSH;
        } else if (Grade == -3) {
          ++m_ErrorNoE;
        }
      } else if (Grade > 4) { // GRADE=5 is some complicated geometry with multiple hits on a single strip. GRADE=6 means not all strips are adjacent.
        H->SetNoDepth();
        Event->SetTrappingCorrectionError("Multiple hits on single strip");
        if (Grade==5) {
          ++m_Error5;
        } else if (Grade==6) {
          ++m_Error6;
        }
      } else { // If the Grade is 0-4, we can handle it.

        // Get the position from the depth cal. If error is thrown, record and no depth.
        // Take a Hit and separate its activated X- and Y-strips into separate vectors.
        vector<MStripHit*> LVStrips;
        vector<MStripHit*> HVStrips;

        for (unsigned int j = 0; j < H->GetNStripHits(); ++j) {
          MStripHit* SH = H->GetStripHit(j);
          if (SH->IsLowVoltageStrip()) LVStrips.push_back(SH); else HVStrips.push_back(SH);
        }

        double LVEnergyFraction;
        double HVEnergyFraction;
        MStripHit* LVSH = GetDominantStrip(LVStrips, LVEnergyFraction); 
        MStripHit* HVSH = GetDominantStrip(HVStrips, HVEnergyFraction); 

        double CTD_s = 0.0;

        //now try and get z position
        int DetID = LVSH->GetDetectorID();
        int LVStripID = LVSH->GetStripID();
        int HVStripID = HVSH->GetStripID();
        // int PixelCode = 10000*DetID + 100*LVStripID + HVStripID;

      

        // Get the position value (assumed from event/hit context H)
        int Zpos = H->GetPosition(); 
        double ctd_val = static_cast<double>(Zpos);

        // Correct the Low Voltage side energy if the hit pointer exists
        if (LVSH != nullptr) {
            double rawLVEnergy = LVSH->GetEnergy(); // Replace with actual getter method for your hit class
            double correctedLVEnergy = GetSimBasedCorrectedEnergy(ctd_val, rawLVEnergy, m_CCEs_LV);
            LVSH->SetEnergy(correctedLVEnergy);     // Replace with actual setter method for your hit class
        }

        // Correct the High Voltage side energy if the hit pointer exists
        if (HVSH != nullptr) {
            double rawHVEnergy = HVSH->GetEnergy(); // Replace with actual getter method for your hit class
            double correctedHVEnergy = GetSimBasedCorrectedEnergy(ctd_val, rawHVEnergy, m_CCEs_HV);
            HVSH->SetEnergy(correctedHVEnergy);     // Replace with actual setter method for your hit class
        }


      }
    }
  }

  Event->SetAnalysisProgress(MAssembly::c_TrappingCorrection);

  return true;
}


/////////////////////////////////////////////////////////////////////////////////


MStripHit* MModuleTrappingCorrection::GetDominantStrip(vector<MStripHit*>& Strips, double& EnergyFraction)
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
  m_CCE_HVs.clear();
  m_CCEs_LVs.clear();

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
      // Step 1: Read parameters A, B, and C from the first line
      if (Tokens.size() == 3) {
        m_ParamA = Tokens[0].ToDouble();
        m_ParamB = Tokens[1].ToDouble();
        m_ParamC = Tokens[2].ToDouble();
        ValidLineCount++;
      } else {
        cout << "ERROR in LoadSimCCEFile: Expected 3 parameters (A,B,C) on the first line." << endl;
        SimCCEFile.Close();
        return false;
      }
    } 
    else if (ValidLineCount == 1) {
      // Step 2: Skip the second line which is the column header (z_depth_mm,CCE_HV)
      ValidLineCount++;
    } 
    else {
      // Step 3: Read the rest of the rows into your depth and CCE HV arrays
      if (Tokens.size() == 3) {
        m_Depths.push_back(Tokens[0].ToDouble());
        m_CCE_HVs.push_back(Tokens[1].ToDouble());
        m_CCEs_LVs.push_back(Tokens[2].ToDouble())
        ValidLineCount++;
      }
    }
  }

  SimCCEFile.Close();

  // Print summary to console if verbose logging is enabled
  if (g_Verbosity >= c_Info) {
    cout << m_XmlTag << "Loaded parameters: A=" << m_ParamA 
         << ", B=" << m_ParamB << ", C=" << m_ParamC << endl;
    cout << m_XmlTag << "Loaded " << m_Depths.size() << " data points into arrays." << endl;
  }

  return true;
}


/////////////////////////////////////////////////////////////////////////////////

double MModuleTrappingCorrection::GetSimBasedCorrectedEnergy(double ctd_val, double uncorrected_energy, const std::vector<double>& sim_cce_sorted) {

  // 2. Look up the simulation CCE baseline using the helper
  double cce_base = Interpolate(ctd_val, m_Depths, sim_cce_sorted);

  // 3. Evaluate the physical trapping function model using class global popt variables
  double expected_centroid_scaled = m_ParamA * (1.0 - m_ParamB * (1.0 - cce_base)) * (1.0 - m_ParamC * (1.0 - cce_base));

  // 4. Prevent division-by-zero or non-physical negative values
  if (expected_centroid_scaled <= 0.0) {
      return uncorrected_energy;
  }

  // 5. Reconstruct the true un-trapped energy deposition
  return uncorrected_energy / expected_centroid_scaled;
}


/////////////////////////////////////////////////////////////////////////////////


double MModuleTrappingCorrection::Interpolate(double x, const std::vector<double>& xp, const std::vector<double>& fp) {
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


int MModuleTrappingCorrection::GetHitGrade(MHit* H){
  // Function for choosing which Depth-to-CTD relation to use for a given event.
  // At time of writing, intention is to choose a CTD based on sub-pixel region determined via charge sharing (Event "grade").
  // 5 possible grades, and one Error Grade, -1. GRADE 4 is as yet uncategorized complicated geometry. GRADE 5 means multiple, presumably separated strip hits.

  //organize x and y strips into vectors
  if (H == nullptr) {
    return -1;
  }
  if (H->GetNStripHits() == 0) {
    // Error if no strip hits listed. Bad grade is returned
    if (g_Verbosity >= c_Error) cout << m_XmlTag << "ERROR in MModuleTrappingCorrection: HIT WITH NO STRIP HITS" << endl;
    return -1;
  }
   
  // Take a Hit and separate its activated p and n strips into separate vectors.
  std::vector<MStripHit*> LVStrips;
  std::vector<MStripHit*> HVStrips;
  vector<int> LVStripIDs;
  vector<int> HVStripIDs;
  for (unsigned int j = 0; j < H->GetNStripHits(); ++j) {
    MStripHit* SH = H->GetStripHit(j);
    if (SH == nullptr ) { 
      if (g_Verbosity >= c_Error) cout << m_XmlTag << "ERROR in MModuleTrappingCorrection: Trapping Correction: got NULL strip hit :( " << endl;
      return -1;
    }
    if (SH->GetEnergy() == 0 ) { 
      if (g_Verbosity >= c_Error) cout << m_XmlTag << "ERROR in MModuleTrappingCorrection: Trapping Correction: got strip without energy :( " << endl; 
      return -1;
    }
    if (SH->IsLowVoltageStrip()) {
      LVStrips.push_back(SH); 
      LVStripIDs.push_back(SH->GetStripID());
    }
    else {
      HVStrips.push_back(SH);
      HVStripIDs.push_back(SH->GetStripID());
    }
  }

  // If the same strip has multiple hits, this is a bad grade.
  bool MultiHitX = H->GetStripHitMultipleTimesX();
  bool MultiHitY = H->GetStripHitMultipleTimesY();
  if (MultiHitX || MultiHitY) {
    return 5;  
  }

  if (LVStrips.size()>0 && HVStrips.size()>0) {
    int HVmin = * std::min_element(HVStripIDs.begin(), HVStripIDs.end());
    int HVmax = * std::max_element(HVStripIDs.begin(), HVStripIDs.end());

    int LVmin = * std::min_element(LVStripIDs.begin(), LVStripIDs.end());
    int LVmax = * std::max_element(LVStripIDs.begin(), LVStripIDs.end());

    // If the strip hits are not all adjacent, it's a bad grade.
    if ( ((HVmax - HVmin) >= (HVStrips.size())) || ((LVmax - LVmin) >= (LVStrips.size())) ) {
      return 6;
    }
  }
  else{
    return -1;
  }

  int return_value;
  // If 1 strip on each side, GRADE=0
  // This represents the center of the pixel
  if ( ((LVStrips.size() == 1) && (HVStrips.size() == 1)) || ((LVStrips.size() == 3) && (HVStrips.size() == 3)) ) {
    return_value = 0;
  } 
  // If 2 hits on N side and 1 on P, GRADE=1
  // This represents the middle of the edges of the pixel
  else if ( (LVStrips.size() == 1) && (HVStrips.size() == 2) ) {
    return_value = 1;
  } 

  // If 2 hits on P and 1 on N, GRADE=2
  // This represents the middle of the edges of the pixel
  else if ( (LVStrips.size() == 2) && (HVStrips.size() == 1) ) {
    return_value = 2;
  } 
  
  // If 2 strip hits on both sides, GRADE=3
  // This represents the corners the pixel
  else if ( (LVStrips.size() == 2) && (HVStrips.size() == 2) ) {
    return_value = 3;
  } 

  // If 3 hits on N side and 1 on P, GRADE=0
  // This represents the middle of the pixel, near the p (LV) side of the detector.
  else if ( (LVStrips.size() == 1) && (HVStrips.size() == 3) ) {
    return_value = 0;
  } 

  // If 3 hits on P and 1 on N, GRADE=0
  // This represents the middle of the pixel, near the n (HV) side of the detector.
  else if ( (LVStrips.size() == 3) && (HVStrips.size() == 1) ) {
    return_value = 0;
  } 

  // If 3 hits on N side and 2 on P, GRADE=0
  // This represents the middle of the edge of the pixel, near the p (LV) side of the detector.
  else if ( (LVStrips.size() == 2) && (HVStrips.size() == 3) ) {
    return_value = 2;
  } 

  // If 3 hits on P and 2 on N, GRADE=0
  // This represents the middle of the edge of the pixel, near the n (HV) side of the detector.
  else if ( (LVStrips.size() == 3) && (HVStrips.size() == 2) ) {
    return_value = 1;
  } 

  else {
    // If more complicated than the above cases, return 4 for now.
    // TODO: Handle more complicated charge distributions.
    return_value = 4;
  }

  return return_value;
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

void MModuleTrappingCorrection::Finalize()
{

  MModule::Finalize();
  cout << "finalize is working" << endl;

  // Clean up maps and vectors
  m_Detectors.clear();
  m_DetectorIDs.clear();

}



// MModuleTrappingCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
