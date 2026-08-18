/*
 * MSubModuleStripReadout.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Robin Anthony-Petersen.
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
// MSubModuleStripReadout
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleStripReadout.h"

// Standard libs:

// ROOT libs:
#include "TRandom.h"

// MEGAlib libs:
#include "MParser.h"

// Nuclearizer libs:
#include "MModuleEnergyCalibration.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleStripReadout)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadout::MSubModuleStripReadout() : MSubModule()
{
  // Construct an instance of MSubModuleStripReadout

  m_Name = "DEE strip readout module";

  m_EnergyCalibrationFileName = "";

}


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadout::~MSubModuleStripReadout()
{
  // Delete this instance of MSubModuleStripReadout
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadout::Initialize()
{
  // Initialize the module

  // Clear the maps before reading
  m_Calibration.clear();
  m_ResolutionCalibration.clear();

  // Check if we have a file
  if (m_EnergyCalibrationFileName == "") {
    if (g_Verbosity >= c_Error) {
      cout << m_Name << ": No energy calibration file specified." << endl;
    }
    return false;
  }

  // Read energy calibration file
  MModuleEnergyCalibration EnergyCalibration;
  if (EnergyCalibration.ReadEnergyCalibrationFile(m_EnergyCalibrationFileName) == true) {
    // Copy the energy calibration function maps
    m_Calibration = EnergyCalibration.GetCalibration();
    m_ResolutionCalibration = EnergyCalibration.GetResolutionCalibration();
  } else {
    return false;
  }

  if (EnergyCalibration.ReadSlowThresholdCutFile(m_HardwareThresholdFileName) == true) {
    // Copy the hardware threshold map
    m_HardwareThresholdMap = EnergyCalibration.GetHardwareThresholdMap();
  } else {
    return false;
  }
  
  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripReadout::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadout::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level
  
  // Get low-voltage and high-voltage hits
  for (auto* Hits : { &Event->GetDEEStripHitLVListReference(), &Event->GetDEEStripHitHVListReference() }) {

    set<MReadOutElementDoubleStrip> TriggeredStripHits;
    set<MReadOutElementDoubleStrip> NeighborCandidateStripHits;
    set<MReadOutElementDoubleStrip> NeighborStripHits;
    set<MReadOutElementDoubleStrip> DeadStrips;
    
    for (MDEEStripHit& SH : *Hits) {
      
      // If the user wants it applied, apply the FWHM Guassian energy resolution
      if (m_ApplyResolutionCalibration == true) {
        // Look up the FWHM fit for this strip
        if (m_ResolutionCalibration.count(SH.m_ROE) == 1) {
          
          double Sigma = m_ResolutionCalibration[SH.m_ROE]->Eval(SH.m_Energy);
          
          // Smear the hit energy using a Gaussian distribution
          SH.m_Energy = gRandom->Gaus(SH.m_Energy, Sigma);
          
          // If energy is lower than zero now, floor it to zero
          if (SH.m_Energy < 0) {
            SH.m_Energy = 0;
          }
          
        } else {
          // The fit wasn't found! Handle the error
          if (g_Verbosity >= c_Warning) {
            cout << m_Name << ": Warning - No resolution calibration fit found for strip ID " << SH.m_ROE.GetStripID() << endl;
          }
          // Note, if no resolution calibration is found then the energy remains unsmeared
        }
      }
      
      // Apply the inverse energy calibration
      // Look up the fit using the ecal
      TF1* Fit = m_Calibration[SH.m_ROE];

      if (Fit != nullptr) {
        // Apply the inverse energy calibration using ROOT's poly inverter (keV -> ADC) in the allowed ADC range
        double calculatedADC = Fit->GetX(SH.m_Energy, 0., m_MaxADCRange);
        
        // Apply hardware limits
        if (calculatedADC > m_MaxADCRange) calculatedADC = m_MaxADCRange;
        if (calculatedADC < 0) calculatedADC = 0;
        
        SH.m_ADC = static_cast<unsigned int>(calculatedADC);

        // Apply the hardware threshold to determine if a strip hit is a nearest-neighbor strip or not
        SH.m_IsNearestNeighbor = SH.m_ADC < m_HardwareThresholdMap[SH.m_ROE];
        SH.m_HasTriggered = true;

        // Keep track of all triggered strips and their nearest neighbors
        if (SH.m_IsNearestNeighbor == false) {
          TriggeredStripHits.insert(SH.m_ROE);
          if (SH.m_ROE.GetStripID() > 0) {
            MReadOutElementDoubleStrip Left = SH.m_ROE;
            Left.SetStripID(Left.GetStripID() - 1);
            NeighborCandidateStripHits.insert(Left);
          }
          if (SH.m_ROE.GetStripID() < 63) {
            MReadOutElementDoubleStrip Right = SH.m_ROE;
            Right.SetStripID(Right.GetStripID() + 1);
            NeighborCandidateStripHits.insert(Right);
          }
        }
        
      } else {
        // If no calibration exists in the .ecal file for this strip set it to ADC value of 0
        if (g_Verbosity >= c_Warning) cout << m_Name << ": No inverse calibration found for element " << SH.m_ROE << endl;
        SH.m_ADC = 0;
        SH.m_HasTriggered = false;
        DeadStrips.insert(SH.m_ROE);
      }
    }

    // Iterate through the list once more to remove sub-threshold hits and add 
    for (auto SH = Hits->begin(); SH != Hits->end(); ) {

      // Iterate only through nearest neighbor candidates
      if (SH->m_HasTriggered && SH->m_IsNearestNeighbor == true) {

        bool HasTriggeredNeighbor = false;

        // Check that one of its neighbors has triggered ...
        NeighborCandidateStripHits.erase(SH->m_ROE);
        if (HasTriggeredNeighbor == false && SH->m_ROE.GetStripID() > 0) {
          MReadOutElementDoubleStrip Left = SH->m_ROE;
          Left.SetStripID(Left.GetStripID() - 1);
          HasTriggeredNeighbor = TriggeredStripHits.count(Left) > 0;
        }
        if (HasTriggeredNeighbor == false && SH->m_ROE.GetStripID() < 63) {
          MReadOutElementDoubleStrip Right = SH->m_ROE;
          Right.SetStripID(Right.GetStripID() + 1);
          HasTriggeredNeighbor = TriggeredStripHits.count(Right) > 0;
        }

        if (HasTriggeredNeighbor == true) {
          NeighborStripHits.insert(SH->m_ROE);
          ++SH;
        } else {
          // ... or remove it otherwise
          SH = Hits->erase(SH);
        }
      }
      else {
        ++SH;
      }
    }

    // Add "empty" strip hits for neighbors that did not trigger
    // Assume an empty baseline (Energy == 0) and no timing (Timing == 0)
    for (const auto& NSH : NeighborCandidateStripHits) {

      // Skip dead strips and triggered strips
      if (DeadStrips.count(NSH) > 0) continue;
      if (TriggeredStripHits.count(NSH) > 0) continue;
      if (NeighborStripHits.count(NSH) > 0) continue;

      // Skip strips with no energy calibration, they are most probably dead or shorted
      TF1* Fit = m_Calibration[NSH];
      if (Fit == nullptr) {
        DeadStrips.insert(NSH);
        continue;
      }

      if (g_Verbosity >= c_Info) {
        cout << "Event ID " << Event->GetID() << ": Adding nearest neighbor candidate: " << (NSH.IsLowVoltageStrip() ? "LV" : "HV") << NSH.GetStripID() << endl;
      }

      MDEEStripHit SH;
      SH.m_ROE = NSH;
      SH.m_Energy = 0;
      SH.m_FastPeakTime = 3200; // some value that will result in m_Timing of 1000ns, to pass TAC cuts ...
      SH.m_Timing = 0;
      SH.m_ADC = 0;
      SH.m_TAC = 0;
      SH.m_HasTriggered = true;
      SH.m_HasFastTiming = false;
      SH.m_IsNearestNeighbor = true;
      SH.m_IsGuardRing = false;

      // Note, if no resolution calibration is found then the energy remains unsmeared
      if (m_ApplyResolutionCalibration == true) {
        if (m_ResolutionCalibration.count(NSH) == 1) {
          double Sigma = m_ResolutionCalibration[NSH]->Eval(SH.m_Energy);
          SH.m_Energy = max(gRandom->Gaus(SH.m_Energy, Sigma), 0.0);
        }
      }
      
      // Apply the inverse energy calibration using ROOT's poly inverter (keV -> ADC) in the allowed ADC range
      double calculatedADC = Fit->GetX(SH.m_Energy, 0., m_MaxADCRange);
      SH.m_ADC = static_cast<unsigned int>(clamp(calculatedADC, 0.0, m_MaxADCRange));

      if (NSH.IsLowVoltageStrip() == true) {
        Event->AddDEEStripHitLV(SH);
      } else {
        Event->AddDEEStripHitHV(SH);
      }
      NeighborStripHits.insert(NSH);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripReadout::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize()

  // Clean up the memory 
  for (auto& F : m_Calibration) {
    delete F.second;
  }
  m_Calibration.clear();
  
  // Clean up the resolution calibration memory
  for (auto& F : m_ResolutionCalibration) {
    delete F.second;
  }
  m_ResolutionCalibration.clear();
  m_HardwareThresholdMap.clear();

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadout::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* EnergyCalibrationFileNameNode = Node->GetNode("EnergyCalibrationFileName");
  if (EnergyCalibrationFileNameNode != nullptr) {
    m_EnergyCalibrationFileName = EnergyCalibrationFileNameNode->GetValue();
  }

  MXmlNode* HardwareThresholdFileNameNode = Node->GetNode("HardwareThresholdFileName");
  if (HardwareThresholdFileNameNode != nullptr) {
    m_HardwareThresholdFileName = HardwareThresholdFileNameNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleStripReadout::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  new MXmlNode(Node, "EnergyCalibrationFileName", m_EnergyCalibrationFileName);
  new MXmlNode(Node, "HardwareThresholdFileName", m_HardwareThresholdFileName);

  return Node;
}


// MSubModuleStripReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
