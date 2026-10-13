/*
 * MCOSIPayloadLollipopCut.cxx
 *
 *
 * Copyright (C) by Robin Anthony-Petersen.
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

#include "MCOSIPayloadLollipopCut.h"
#include "MStripHit.h"
#include <limits>

////////////////////////////////////////////////////////////////////////////////
//
// MCOSIPayloadLollipopCut
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MCOSIPayloadLollipopCut.h"
#include "MStripHit.h"
#include "MGUIOptionsCOSIPayloadLollipopCut.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MCOSIPayloadLollipopCut)
#endif


////////////////////////////////////////////////////////////////////////////////


MCOSIPayloadLollipopCut::MCOSIPayloadLollipopCut()
{
  // Construct an instance of MCOSIPayloadLollipopCut
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "COSI Payload Lollipop Cut";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagCOSIPayloadLollipopCut";
  
  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_TACCalibration);
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_COSIPayloadLollipopCut);
  
  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_EnergyCalibration);
  
  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  
  // Can the program be run multi-threaded
  m_AllowMultiThreading = true;
  
  // Can we use multiple instances of this class
  m_AllowMultipleInstances = true;

  // Applying lollipo cuts by default
  m_ApplyKeepSidesIlluminatedByLollipops = true;
  
  // Set default source positions
  m_SourcePosition = 0;
  
  m_ExpoEnergySpectrum = nullptr;
  
  // Clear keep list so it starts cutting 100% of events
  m_DetectorsToKeep.clear();
  
}


////////////////////////////////////////////////////////////////////////////////


MCOSIPayloadLollipopCut::~MCOSIPayloadLollipopCut()
{
  // Delete this instance of MCOSIPayloadLollipopCut
}

bool MCOSIPayloadLollipopCut::Initialize()
{
  // Initialize the module
  
  // Refresh m_DetectorsToKeep based on current m_SourcePosition
  UpdateKeptDetectors();
  
  return MModule::Initialize();
}

////////////////////////////////////////////////////////////////////////////////


void MCOSIPayloadLollipopCut::CreateExpos()
{
  // it will use the energy spectrum expo so we can see the before and after
  if (HasExpos() == true) return;

  m_ExpoEnergySpectrum = new MGUIExpoPlotSpectrum(this);
  m_Expos.push_back(m_ExpoEnergySpectrum);
}


////////////////////////////////////////////////////////////////////////////////


void MCOSIPayloadLollipopCut::ShowOptionsGUI()
{
  MGUIOptionsCOSIPayloadLollipopCut* Options = new MGUIOptionsCOSIPayloadLollipopCut(this);
  Options->Create();
}


////////////////////////////////////////////////////////////////////////////////

void MCOSIPayloadLollipopCut::UpdateKeptDetectors()
{
  // Map integer selection (1-3) to Source 1 boolean flags
  m_Source_L0L1 = (m_SourcePosition == 1);
  m_Source_L1L2 = (m_SourcePosition == 2);
  m_Source_L2L3 = (m_SourcePosition == 3);

  // Re-run mapping setup to refresh m_DetectorsToKeep
  SetupLollipopMapping();
}

void MCOSIPayloadLollipopCut::SetupLollipopMapping()
{
  // Set the values of everything in the detector mapping to false
  // False means, this detector side is not being illuminated we will cut it
  
  DetectorSideIlluminated["Q0L0HVIlluminated"] = false;
  DetectorSideIlluminated["Q0L0LVIlluminated"] = false;
  DetectorSideIlluminated["Q1L0HVIlluminated"] = false;
  DetectorSideIlluminated["Q1L0LVIlluminated"] = false;
  DetectorSideIlluminated["Q2L0HVIlluminated"] = false;
  DetectorSideIlluminated["Q2L0LVIlluminated"] = false;
  DetectorSideIlluminated["Q3L0HVIlluminated"] = false;
  DetectorSideIlluminated["Q3L0LVIlluminated"] = false;
  
  DetectorSideIlluminated["Q0L1HVIlluminated"] = false;
  DetectorSideIlluminated["Q0L1LVIlluminated"] = false;
  DetectorSideIlluminated["Q1L1HVIlluminated"] = false;
  DetectorSideIlluminated["Q1L1LVIlluminated"] = false;
  DetectorSideIlluminated["Q2L1HVIlluminated"] = false;
  DetectorSideIlluminated["Q2L1LVIlluminated"] = false;
  DetectorSideIlluminated["Q3L1HVIlluminated"] = false;
  DetectorSideIlluminated["Q3L1LVIlluminated"] = false;
  
  DetectorSideIlluminated["Q0L2HVIlluminated"] = false;
  DetectorSideIlluminated["Q0L2LVIlluminated"] = false;
  DetectorSideIlluminated["Q1L2HVIlluminated"] = false;
  DetectorSideIlluminated["Q1L2LVIlluminated"] = false;
  DetectorSideIlluminated["Q2L2HVIlluminated"] = false;
  DetectorSideIlluminated["Q2L2LVIlluminated"] = false;
  DetectorSideIlluminated["Q3L2HVIlluminated"] = false;
  DetectorSideIlluminated["Q3L2LVIlluminated"] = false;
  
  DetectorSideIlluminated["Q0L3HVIlluminated"] = false;
  DetectorSideIlluminated["Q0L3LVIlluminated"] = false;
  DetectorSideIlluminated["Q1L3HVIlluminated"] = false;
  DetectorSideIlluminated["Q1L3LVIlluminated"] = false;
  DetectorSideIlluminated["Q2L3HVIlluminated"] = false;
  DetectorSideIlluminated["Q2L3LVIlluminated"] = false;
  DetectorSideIlluminated["Q3L3HVIlluminated"] = false;
  DetectorSideIlluminated["Q3L3LVIlluminated"] = false;
  
  // Set the specific faces to true based on the active GUI button
  if (m_Source_L0L1 == true) {
    DetectorSideIlluminated["Q0L0HVIlluminated"] = true;
    DetectorSideIlluminated["Q1L0LVIlluminated"] = true;
    //DetectorSideIlluminated["Q2L0HVIlluminated"] = true;
    // apple sauced!!
    DetectorSideIlluminated["Q2L0LVIlluminated"] = true;
    DetectorSideIlluminated["Q3L0LVIlluminated"] = true;
    //DetectorSideIlluminated["Q0L1LVIlluminated"] = true;
    // apple sauced!!
    DetectorSideIlluminated["Q0L1HVIlluminated"] = true;
    DetectorSideIlluminated["Q1L1HVIlluminated"] = true;
    DetectorSideIlluminated["Q2L1LVIlluminated"] = true;
    DetectorSideIlluminated["Q3L1HVIlluminated"] = true;
  }
  
  if (m_Source_L1L2 == true) {
    //DetectorSideIlluminated["Q0L1HVIlluminated"] = true;
    //appled sauced!! change once felix's fix is merged
    DetectorSideIlluminated["Q0L1LVIlluminated"] = true;
    DetectorSideIlluminated["Q1L1LVIlluminated"] = true;
    // apple sauced!!
    //DetectorSideIlluminated["Q2L1HVIlluminated"] = true;
    DetectorSideIlluminated["Q2L1LVIlluminated"] = true;
    DetectorSideIlluminated["Q3L1LVIlluminated"] = true;
    DetectorSideIlluminated["Q0L2LVIlluminated"] = true;
    DetectorSideIlluminated["Q1L2HVIlluminated"] = true;
    DetectorSideIlluminated["Q2L2LVIlluminated"] = true;
    DetectorSideIlluminated["Q3L2HVIlluminated"] = true;
  }
  
  if (m_Source_L2L3 == true) {
    DetectorSideIlluminated["Q0L2HVIlluminated"] = true;
    DetectorSideIlluminated["Q1L2LVIlluminated"] = true;
    DetectorSideIlluminated["Q2L2HVIlluminated"] = true;
    DetectorSideIlluminated["Q3L2LVIlluminated"] = true;
    DetectorSideIlluminated["Q0L3LVIlluminated"] = true;
    DetectorSideIlluminated["Q1L3HVIlluminated"] = true;
    DetectorSideIlluminated["Q2L3LVIlluminated"] = true;
    DetectorSideIlluminated["Q3L3HVIlluminated"] = true;
  }
  
  // Clear the list first
  m_DetectorsToKeep.clear();
  
  // Loop through the DetectorSideIlluminated map
  for (auto entry : DetectorSideIlluminated) {
    
    // Get the first and second entry of the map
    std::string key = entry.first;       // --> "Q0L0HVIlluminated"
    bool isIlluminated = entry.second;  // --> true or false
    
    // if a side is illuminated we want to keep it
    if (isIlluminated == true) {
      
      // Extract Q and L numbers to get a list of detector IDs to keep
      int Q = key[1] - '0';
      int L = key[3] - '0';
      
      // Calculate detector ID
      int DetectorIlluminatedID = (Q * 4) + L;
      
      // Check if this is LV or HV (index 4 is 'L' or 'H')
      bool isLV = (key[4] == 'L');
      
      // Create a kept side object and add the sides to keep to this list :)
      KeptDetectorSide sideToKeep;
      sideToKeep.DetectorIlluminatedID = DetectorIlluminatedID;
      sideToKeep.isLV = isLV;
      
      m_DetectorsToKeep.push_back(sideToKeep);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////



bool MCOSIPayloadLollipopCut::ApplyKeepSidesIlluminatedByLollipops(MReadOutAssembly* Event)
{
  // If the event is empty, pass it through
  if (Event->GetNStripHits() == 0) {
    return false;
  }

  // Make sure all hits belong to the exact same detector
  int eventDetectorID = Event->GetStripHit(0)->GetDetectorID();

  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
    MStripHit* stripHit = Event->GetStripHit(i);
    
    // Skip GRs hits
    if (stripHit == nullptr || stripHit->IsGuardRing() == true) {
      continue;
    }

    if (stripHit->GetDetectorID() != eventDetectorID) {
      return false; // Multi-detector event --> cut for now
    }
  }

  // Check if this detector is in our keep list AND get the EXPECTED side
  bool isDetectorFound = false;
  bool expectedIsLV = false;

  for (const auto& sideToKeep : m_DetectorsToKeep) {
    if (sideToKeep.DetectorIlluminatedID == eventDetectorID) {
      isDetectorFound = true;
      expectedIsLV = sideToKeep.isLV; // Where the config said the source was pointing
      break;
    }
  }

  // If this detector ID was not in m_DetectorsToKeep --> cut
  if (isDetectorFound == false) {
    return false;
  }

  // Find the highest energy hit on each face
  // TODO: Replace this with more sophisticated cuts based on charztiation in python
  
  MStripHit* maxLVHit = nullptr;
  MStripHit* maxHVHit = nullptr;
  double maxEnergyADCLV = -1.0;
  double maxEnergyADCHV = -1.0;

  
  // Loop through the stip hits and find the one with the highest energy
  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
    MStripHit* stripHit = Event->GetStripHit(i);

    // If the strip hit doesn't exist or if it's a GR, then we skip
    if (stripHit == nullptr || stripHit->IsGuardRing() == true) {
      continue;
    }
    
    // For the list of hits, find the hit with the most energy (in ADC)
    // So that we don't have to apply the ecal before this module, get the enegry in terms of ADC
    double energyADC = stripHit->GetADCUnits();

    // By the end of this, we should have the largest HV and largest LV strip hits
    if (stripHit->IsLowVoltageStrip() == true) {
      if (energyADC > maxEnergyADCLV) {
        maxEnergyADCLV = energyADC;
        maxLVHit = stripHit;
      }
    } else {
      if (energyADC > maxEnergyADCHV) {
        maxEnergyADCHV = energyADC;
        maxHVHit = stripHit;
      }
    }
  }

  // Must have a hit on both sides to measure timing
  if (maxLVHit == nullptr || maxHVHit == nullptr) {
    // Check, does returning false here cut the event??
    // Yes, because in AnalyzeEvent(), it calls this function and if this function returns false, it cuts the event
    return false;
  }

  // Determine which side the event occurred on
  double lvTiming = maxLVHit->GetTiming();
  double hvTiming = maxHVHit->GetTiming();

  // Chose the below timing values from the depth calibation module:
  // else if ((LVTiming < 1.0E-6) || (HVTiming < 1.0E-6)) {
  // ++m_Error3;
  // H->SetNoDepth();
  // Event->SetDepthCalibrationError("No timing");
  if (lvTiming < 1.0E-6 || hvTiming < 1.0E-6) {
    return false; // Invalid timing calibration
  }

  double ctd = hvTiming - lvTiming;
  
  // Make sure both timing types match with this cut from Field
  if (ctd > 1000 || ctd < -1000) {
    
    // Negative CTD means the hit physically occurred on the LV side
    bool lvSideIlluminated = (ctd < 0.0);
    
    // Keep the event ONLY if actual side matches expected side
    if (lvSideIlluminated == expectedIsLV) {
      return true;  // Keep event!
    } else {
      return false; // Cut event
    }
  } else {
    return false; // ctd doesn't fall within the allowed range (mix of fast and slow timing)
  }
}



////////////////////////////////////////////////////////////////////////////////



bool MCOSIPayloadLollipopCut::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Log pre-cut spectrum exposure
  if (HasExpos()) {
    for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
      MStripHit* SH = Event->GetStripHit(i);
      if (SH == nullptr) continue;
      m_ExpoEnergySpectrum->AddEnergyInitial(SH->GetEnergy(), SH->IsNearestNeighbor(), SH->IsLowVoltageStrip());
    }
  }

  if (m_ApplyKeepSidesIlluminatedByLollipops == true) {
    bool TACError = Event->HasTACCalibrationError();
    bool Keep = ApplyKeepSidesIlluminatedByLollipops(Event);
    
    if (TACError == true || Keep == false) {
      Event->SetFilteredOut(true);
      return true;
    }
  }
  
  // Log post-cut spectrum exposure for surviving events
  if (HasExpos()) {
    for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
      MStripHit* SH = Event->GetStripHit(i);
      if (SH == nullptr) continue;
      m_ExpoEnergySpectrum->AddEnergyFinal(SH->GetEnergy(), SH->IsNearestNeighbor(), SH->IsLowVoltageStrip());
    }
  }

  return true;
}



////////////////////////////////////////////////////////////////////////////////




bool MCOSIPayloadLollipopCut::ReadXmlConfiguration(MXmlNode* Node)
{
  MXmlNode* ApplyNode = Node->GetNode("ApplyKeepSidesIlluminatedByLollipops");
  if (ApplyNode != nullptr) {
    m_ApplyKeepSidesIlluminatedByLollipops = ApplyNode->GetValueAsBoolean();
  }

  MXmlNode* SourceNode = Node->GetNode("SourcePosition");
  if (SourceNode != nullptr) {
    m_SourcePosition = SourceNode->GetValueAsInt();
  }

  UpdateKeptDetectors(); // Refresh detector array from newly read positions!

  return true;
}

MXmlNode* MCOSIPayloadLollipopCut::CreateXmlConfiguration()
{
  MXmlNode* Node = new MXmlNode(nullptr, m_XmlTag);

  new MXmlNode(Node, "ApplyKeepSidesIlluminatedByLollipops", m_ApplyKeepSidesIlluminatedByLollipops);
  new MXmlNode(Node, "SourcePosition", m_SourcePosition);

  return Node;
}




////////////////////////////////////////////////////////////////////////////////




MModule* MCOSIPayloadLollipopCut::Clone()
{
  MCOSIPayloadLollipopCut* Module = new MCOSIPayloadLollipopCut();

  // Copy parameters from master instance to clone
  Module->m_ApplyKeepSidesIlluminatedByLollipops = m_ApplyKeepSidesIlluminatedByLollipops;
  Module->m_SourcePosition = m_SourcePosition;

  // Populate m_DetectorsToKeep on the clone
  Module->UpdateKeptDetectors();

  return Module;
}





// MCOSIPayloadLollipopCut.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
