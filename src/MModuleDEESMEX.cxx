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

  // Set the module name --- has to be unique
  m_Name = "Detector effects engine for COSI SMEX";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagDEESMEX";
  
  // This is a special start module which can generate its own events
  m_IsStartModule = false;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = false;
  m_AllowMultipleInstances = false;

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoaderSimulation);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_DetectorEffectsEngine);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  m_HasOptionsGUI = true;
}


////////////////////////////////////////////////////////////////////////////////


MModuleDEESMEX::~MModuleDEESMEX()
{
  // Delete this instance of MModuleDEESMEX
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDEESMEX::Initialize()
{
  // Initialize the module 

  // Each Initialize() should handle its own error messaging
  if (m_Intake.Initialize() == false) return false;
  if (m_RandomCoincidence.Initialize() == false) return false;
  if (m_ShieldEnergyCorrection.Initialize() == false) return false;
  if (m_ShieldReadout.Initialize() == false) return false;
  if (m_ShieldTrigger.Initialize() == false) return false;
  if (m_ChargeTransport.Initialize() == false) return false;
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

  if (Event->GetTime() < m_DeadTimeEnd) {
    // Flag event for deletion
    return true;
  }

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


  // Step (6): the shield veto / trigger, handle pre-scalers, calculate dead-time, calculate random coincidence time
  m_ShieldTrigger.Clear();
  m_ShieldTrigger.AnalyzeEvent(Event);
  if (m_ShieldTrigger.HasVeto() == true) {
    if (m_ShieldTrigger.GetDeadTimeEnd() > m_DeadTimeEnd) {
      m_DeadTimeEnd = m_ShieldTrigger.GetDeadTimeEnd();
    }

    // Clean up

    Event->SetAnalysisProgress(MAssembly::c_DetectorEffectsEngine);
    return true;
  } else if (m_ShieldTrigger.HasTrigger() == true) { // = energy read out
    if (m_ShieldTrigger.GetDeadTimeEnd() > m_DeadTimeEnd) {
      m_DeadTimeEnd = m_ShieldTrigger.GetDeadTimeEnd();
    }
  }

  // Charge trapping as input to the charge transport or as "correction afterwards"?

  // Step (7): Handle GeD charge transport to grid and voxelation into strips
  m_ChargeTransport.Clear();
  m_ChargeTransport.AnalyzeEvent(Event);

  // Step (8): Handle the strip readout: energy -> ADCs
  m_StripReadout.Clear();
  m_StripReadout.AnalyzeEvent(Event);


  // Step (9)): Simulate micro-phonics random noise for triggered strips & next neighbors
  m_StripReadoutNoise.Clear();
  m_StripReadoutNoise.AnalyzeEvent(Event);

  // Step (10): Handles triggers and guard ring vetoes, pre-scalers, calculate dead-time, add nearest neighbor noise, calculate random coincidence time
  m_StripTrigger.Clear();
  m_StripTrigger.AnalyzeEvent(Event);
  if (m_StripTrigger.HasVeto() == true) {
    if (m_StripTrigger.GetDeadTimeEnd() > m_DeadTimeEnd) {
      m_DeadTimeEnd = m_StripTrigger.GetDeadTimeEnd();
    }
    // Clean up

    Event->SetAnalysisProgress(MAssembly::c_DetectorEffectsEngine);
    return true;
  }
  if (m_StripTrigger.HasTrigger() == true) {
    if (m_StripTrigger.GetDeadTimeEnd() > m_DeadTimeEnd) {
      m_DeadTimeEnd = m_StripTrigger.GetDeadTimeEnd();
    }
  }

  // Step (11): Handle depth and timing noise
  m_DepthReadout.Clear();
  m_DepthReadout.AnalyzeEvent(Event);
  cout<<"(11): # LV strips: "<<Event->GetNDEEStripHitsLV()<<endl;


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
  m_StripTrigger.ReadXmlConfiguration(Node);
  m_DepthReadout.ReadXmlConfiguration(Node);
  m_Output.ReadXmlConfiguration(Node);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleDEESMEX::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  
  return Node;
}


// MModuleDEESMEX.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
