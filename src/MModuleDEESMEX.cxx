/*
 * MModuleDEESMEX.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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
// MModuleDEESMEX
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleDEESMEX.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsDEESMEX.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleDEESMEX)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleDEESMEX::MModuleDEESMEX() : MModule()
{
  // Construct an instance of MModuleDEESMEX

  // Set geometry to nullptr to explicitly check that it was set in Initialize
  m_Geometry = nullptr;

  // Set the module name --- has to be unique
  m_Name = "Detector effects engine for COSI SMEX";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagDEESMEX";
  
  // This is a special start module which can generate its own events
  m_IsStartModule = false;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoaderSimulation);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_DetectorEffectsEngine);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  m_HasOptionsGUI = true;
  
  // Default to adding noise to the simulated energies
  m_ApplyResolutionCalibration = true;
  m_EnableShieldVeto = true;
  m_EnableGuardRingVeto = true;

  // Default to adding noise to the simulated timing values
  m_ApplyTimingResolutionCalibration = true;
}


////////////////////////////////////////////////////////////////////////////////


MModuleDEESMEX::~MModuleDEESMEX()
{
  // Delete this instance of MModuleDEESMEX
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDEESMEX::Initialize()
{

  if (m_Geometry == nullptr) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR in MModuleDEESMEX::Initialize: m_Geometry is a nullptr" << endl;
    }
    return false;
  }

  // Set the geometry to the SubModules using it
  m_ChargeTransport.SetGeometry(m_Geometry);
  
  m_StripReadout.SetApplyResolutionCalibration(m_ApplyResolutionCalibration);
  m_DepthReadout.SetApplyTimingResolutionCalibration(m_ApplyTimingResolutionCalibration);

  // Pass the depth-calibration-related files to the SubModules using it
  m_ChargeTransport.SetDepthSplinesFileName(m_DepthSplinesFileName);
  m_ChargeTransport.SetDepthCoefficientsFileName(m_DepthCoefficientsFileName);
  m_DepthReadout.SetDepthCoefficientsFileName(m_DepthCoefficientsFileName);

  // Each Initialize() should handle its own error messaging
  if (m_Intake.Initialize() == false) return false;
  if (m_RandomCoincidence.Initialize() == false) return false;
  if (m_ShieldEnergyCorrection.Initialize() == false) return false;
  if (m_ShieldReadout.Initialize() == false) return false;
  if (m_ShieldTrigger.Initialize() == false) return false;
  if (m_ChargeTransport.Initialize() == false) return false;
  if (m_StripReadoutNoise.Initialize() == false) return false;
  if (m_StripReadout.Initialize() == false) return false;
  if (m_StripTrigger.Initialize() == false) return false;
  if (m_DepthReadout.Initialize() == false) return false;
  if (m_Output.Initialize() == false) return false;

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDEESMEX::AnalyzeEvent(MReadOutAssembly* Event)
{
  // The main DEE loop

  // Step (1):
  // Handle dead times - needs to happen in main DEE class

  // Step (2): Fill the MDEEStripHits of the event
  m_Intake.Clear();
  m_Intake.AnalyzeEvent(Event);

  // Step (3): Merge coincident events

  // Don't know how to handle random coincidences yet
  // We do have an input queue thus I might just look if we have enough events in there
  // If not flag the Supervisor to push more in

  m_RandomCoincidence.Clear();
  // m_RandomCoincidence.AddEventToMerge();
  m_RandomCoincidence.AnalyzeEvent(Event);

  // Need to check if we are still in random coincidence time

  // Step (4): Handle the effect of the light transport to the SiPMs as an energy correction
  m_ShieldEnergyCorrection.Clear();
  m_ShieldEnergyCorrection.AnalyzeEvent(Event);


  // Step (5): Handle the shield readout: Energy to ADCs and thresholds
  m_ShieldReadout.Clear();
  m_ShieldReadout.AnalyzeEvent(Event);


  bool HasShieldHit = Event->GetDEECrystalHitListReference().empty() == false;
  if (HasShieldHit == true && Event->GetTimeUTC() < m_ShieldTrigger.GetShieldDeadTimeEnd()) {
    Event->GetDEECrystalHitListReference().clear();
  }


  // Step (6): the shield veto / trigger, handle pre-scalers, calculate dead-time, calculate random coincidence time
  m_ShieldTrigger.Clear();
  m_ShieldTrigger.AnalyzeEvent(Event);
  if (m_EnableShieldVeto == true && m_ShieldTrigger.HasShieldVeto() == true) {
    Event->SetShieldVeto(true);
    m_StripTrigger.ApplyFastClearDeadtime(m_ShieldTrigger.GetShieldVetoTime());

    // Clean up
    
    Event->SetAnalysisProgress(MAssembly::c_DetectorEffectsEngine);
    return true;
  }

  bool HasStripHit = Event->GetDEEStripHitLVListReference().empty() == false ||
                     Event->GetDEEStripHitHVListReference().empty() == false;
  if (HasStripHit == true && Event->GetTimeUTC() < m_StripTrigger.GetStripDeadTimeEnd()) {
    // Strip/GeD deadtime only gates the strip path. Shield processing above still runs.
    return true;
  }

  // Charge trapping as input to the charge transport or as "correction afterwards"?

  // Step (7): Handle GeD charge transport to grid and voxelation into strips
  m_ChargeTransport.Clear();
  m_ChargeTransport.AnalyzeEvent(Event);

  // Step (8): Handle the strip readout: energy -> ADCs
  // Also includes user selected energy resolution with the FWHM values from the ecal 
  m_StripReadout.Clear();
  m_StripReadout.AnalyzeEvent(Event);
  
  // Step (9): Simulate micro-phonics random noise
  m_StripReadoutNoise.Clear();
  m_StripReadoutNoise.AnalyzeEvent(Event);

  // Step (10): Handles triggers and guard ring vetoes, pre-scalers, calculate dead-time, add nearest neighbor noise, calculate random coincidence time
  m_StripTrigger.Clear();
  m_StripTrigger.AnalyzeEvent(Event);
  if (m_EnableGuardRingVeto == true && m_StripTrigger.HasGuardRingVeto() == true) {
    Event->SetGuardRingVeto(true);   // <-- mark the event so EventSaver can filter it
    Event->SetAnalysisProgress(MAssembly::c_DetectorEffectsEngine);
    return true;
  }

  // Step (11): Handle depth and timing noise
  m_DepthReadout.Clear();
  m_DepthReadout.AnalyzeEvent(Event);


  // Step (12): Global event time?


  // Step (13): Fill strip hit structures of the event
  m_Output.Clear();
  m_Output.AnalyzeEvent(Event);

  // Step (14): Handle Aspect and other auxillary data

  Event->SetAnalysisProgress(MAssembly::c_DetectorEffectsEngine);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleDEESMEX::Finalize()
{
  // Initialize the module 
  
  m_Intake.Finalize();
  m_RandomCoincidence.Finalize();
  m_ShieldEnergyCorrection.Finalize();
  m_ShieldReadout.Finalize();
  m_ShieldTrigger.Finalize();
  m_ChargeTransport.Finalize();
  m_StripReadout.Finalize();
  m_StripReadoutNoise.Finalize();
  m_StripTrigger.Finalize();
  m_DepthReadout.Finalize();
  m_Output.Finalize();

  MModule::Finalize();
}


///////////////////////////////////////////////////////////////////////////////


void MModuleDEESMEX::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsDEESMEX* Options = new MGUIOptionsDEESMEX(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDEESMEX::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  m_Intake.ReadXmlConfiguration(Node);
  m_RandomCoincidence.ReadXmlConfiguration(Node);
  m_ShieldEnergyCorrection.ReadXmlConfiguration(Node);
  m_ShieldReadout.ReadXmlConfiguration(Node);
  m_ShieldTrigger.ReadXmlConfiguration(Node);
  m_ChargeTransport.ReadXmlConfiguration(Node);
  m_StripReadout.ReadXmlConfiguration(Node);
  m_StripReadoutNoise.ReadXmlConfiguration(Node);
  m_StripTrigger.ReadXmlConfiguration(Node);
  m_DepthReadout.ReadXmlConfiguration(Node);
  m_Output.ReadXmlConfiguration(Node);

  // Add depth-calibration-related file names (used by several submodules)
  MXmlNode* DepthSplineFile = Node->GetNode("DepthSplineFileName");
  if (DepthSplineFile != nullptr) {
    m_DepthSplinesFileName = DepthSplineFile->GetValue();
  }
  MXmlNode* DepthCoefficientsFileName = Node->GetNode("DepthCoefficientsFileName");
  if (DepthCoefficientsFileName != nullptr) {
    m_DepthCoefficientsFileName = DepthCoefficientsFileName->GetValue();
  }
  
  // Add noise button for energies
  MXmlNode* ResolutionCalibrationNode = Node->GetNode("ApplyResolutionCalibration");
  if (ResolutionCalibrationNode != nullptr) {
    m_ApplyResolutionCalibration  = ResolutionCalibrationNode->GetValueAsBoolean();
  }
  MXmlNode* EnableShieldVetoNode = Node->GetNode("EnableShieldVeto");
  if (EnableShieldVetoNode != nullptr) {
    m_EnableShieldVeto = EnableShieldVetoNode->GetValueAsBoolean();
  }
  MXmlNode* EnableGuardRingVetoNode = Node->GetNode("EnableGuardRingVeto");
  if (EnableGuardRingVetoNode != nullptr) {
    m_EnableGuardRingVeto = EnableGuardRingVetoNode->GetValueAsBoolean();
  }

  // Add noise button for timing values
  MXmlNode* TimingResolutionCalibrationNode = Node->GetNode("ApplyTimingResolutionCalibration");
  if (TimingResolutionCalibrationNode != nullptr) {
    m_ApplyTimingResolutionCalibration  = TimingResolutionCalibrationNode->GetValueAsBoolean();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleDEESMEX::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  m_Intake.CreateXmlConfiguration(Node);
  m_RandomCoincidence.CreateXmlConfiguration(Node);
  m_ShieldEnergyCorrection.CreateXmlConfiguration(Node);
  m_ShieldReadout.CreateXmlConfiguration(Node);
  m_ShieldTrigger.CreateXmlConfiguration(Node);
  m_ChargeTransport.CreateXmlConfiguration(Node);
  m_StripReadout.CreateXmlConfiguration(Node);
  m_StripReadoutNoise.CreateXmlConfiguration(Node);
  m_StripTrigger.CreateXmlConfiguration(Node);
  m_DepthReadout.CreateXmlConfiguration(Node);
  m_Output.CreateXmlConfiguration(Node);
  
  // Add depth-calibration-related file names (used by several submodules)
  new MXmlNode(Node, "DepthSplineFileName", m_DepthSplinesFileName);
  new MXmlNode(Node, "DepthCoefficientsFileName", m_DepthCoefficientsFileName);

  // Add noise button for energies
  new MXmlNode(Node, "ApplyResolutionCalibration", m_ApplyResolutionCalibration);
  // Add shield veto effects button
  new MXmlNode(Node, "EnableShieldVeto", m_EnableShieldVeto);
  // Add guard ring veto effects button
  new MXmlNode(Node, "EnableGuardRingVeto", m_EnableGuardRingVeto);

  // Add noise button for timing values
  new MXmlNode(Node, "ApplyTimingResolutionCalibration", m_ApplyTimingResolutionCalibration);

  return Node;
}


// MModuleDEESMEX.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
