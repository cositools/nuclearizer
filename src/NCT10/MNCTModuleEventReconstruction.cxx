/*
 * MNCTModuleEventReconstruction.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
// MNCTModuleEventReconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleEventReconstruction.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"
#include "MREHit.h"
#include "MRERawEvent.h"
#include "MGUIDataRevan.h"
#include "MGUIOptionsEventReconstruction.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleEventReconstruction)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEventReconstruction::MNCTModuleEventReconstruction() : MNCTModule()
{
  // Construct an instance of MNCTModuleEventReconstruction

  // Set all module relevant information

  // Set the module name
  m_Name = "Event reconstruction";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "EventReconstruction";

  // Set all modules, which have to be done before this module
  //AddPreceedingModuleType(c_DetectorEffectsEngine);
  AddPreceedingModuleType(c_EnergyCalibration);
  //AddPreceedingModuleType(c_ChargeSharingCorrection);
  AddPreceedingModuleType(c_DepthCorrection);
  AddPreceedingModuleType(c_StripPairing);
  //AddPreceedingModuleType(c_EventReconstruction);

  // Set all types this modules handles
  //AddModuleType(c_DetectorEffectsEngine);
  //AddModuleType(c_EnergyCalibration);
  //AddModuleType(c_ChargeSharingCorrection);
  //AddModuleType(c_DepthCorrection);
  //AddModuleType(c_StripPairing);
  AddModuleType(c_EventReconstruction);

  // Set all modules, which can follow this module
  //AddSucceedingModuleType(c_DetectorEffectsEngine);
  //AddSucceedingModuleType(c_EnergyCalibration);
  //AddSucceedingModuleType(c_ChargeSharingCorrection);
  //AddSucceedingModuleType(c_DepthCorrection);
  //AddSucceedingModuleType(c_StripPairing);
  //AddSucceedingModuleType(c_EventReconstruction);

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options

  // Local initializations follow here:
  m_RevanConfigurationFileName = "";
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEventReconstruction::~MNCTModuleEventReconstruction()
{
  // Delete this instance of MNCTModuleEventReconstruction
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventReconstruction::Initialize()
{
  // Initialize the module --- has to be overwritten

  if (m_RevanConfigurationFileName == "") {
    mgui<<"Event reconstruction need a configuration file name"<<show;
    return false;
  }

  if (MFile::Exists(m_RevanConfigurationFileName) == false) {
    mgui<<"Event reconstruction: Cannot load file name \""<<m_RevanConfigurationFileName<<"\""<<show;
    return false;
  }

  // We have to re-initialize our own geometry here:
  m_GeometryRevan = new MGeometryRevan();
  m_GeometryRevan->ScanSetupFile(m_Geometry->GetFileName());

  m_Analyzer = new MRawEventAnalyzer("", "", m_GeometryRevan);
  
  MGUIDataRevan Data;
  Data.ReadData(m_RevanConfigurationFileName);
  Data.UpdateRawEventAnalyzer(*m_Analyzer);
  m_Analyzer->SetBatch(true);

  if (m_Analyzer->PreAnalysis() == false) {
    mgui<<"Pre-analysis of your input failed. Please check the input file as well as the revan configuration file!"<<show;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventReconstruction::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  // Create a raw event:
  MRERawEvent* RE = new MRERawEvent();
  RE->SetEventId(Event->GetID());
  RE->SetEventTime(Event->GetTime());

  for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
    MNCTHit* H = Event->GetHit(h);
    MREHit* Hit = new MREHit(H->GetPosition(), 
                             H->GetEnergy(), 0, 3,
                             H->GetPositionResolution(), 
                             H->GetEnergyResolution(), 
                             0.0);
    RE->AddRESE(Hit);
  }

  if (m_Analyzer->AnalyzeOneEvent(RE) == true) {
    Event->SetPhysicalEvent(m_Analyzer->GetBestTryEvent()->GetPhysicalEvent());
  }
  
  // The analyzer is deleting the event and all hits!

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleEventReconstruction::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing

  MGUIOptionsEventReconstruction* Options = new MGUIOptionsEventReconstruction(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventReconstruction::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* RevanConfigurationFileNameNode = Node->GetNode("RevanConfigurationFileName");
  if (RevanConfigurationFileNameNode != 0) {
    m_RevanConfigurationFileName = RevanConfigurationFileNameNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleEventReconstruction::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  new MXmlNode(Node, "RevanConfigurationFileName", m_RevanConfigurationFileName);

  return Node;
}


// MNCTModuleEventReconstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
