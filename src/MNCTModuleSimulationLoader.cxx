/*
 * MNCTModuleSimulationLoader.cxx
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
// MNCTModuleSimulationLoader
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleSimulationLoader.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsSimulationLoader.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleSimulationLoader)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleSimulationLoader::MNCTModuleSimulationLoader() : MModule()
{
  // Construct an instance of MNCTModuleSimulationLoader

  // Set the module name --- has to be unique
  m_Name = "Simulation loader and detector effects engine for COSI 2016";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagSimulationLoader";
  
  // This is a special start module which can generate its own events
  m_IsStartModule = true;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;

  // Set all modules, which have to be done before this module
  // None
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventLoader);
  AddModuleType(MAssembly::c_EventLoaderSimulation);
  AddModuleType(MAssembly::c_DetectorEffectsEngine);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  m_HasOptionsGUI = true;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleSimulationLoader::~MNCTModuleSimulationLoader()
{
  // Delete this instance of MNCTModuleSimulationLoader
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleSimulationLoader::Initialize()
{
  // Initialize the module 

  MNCTDetectorEffectsEngineCOSI::SetGeometry(MModule::m_Geometry);
  return MNCTDetectorEffectsEngineCOSI::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleSimulationLoader::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  if (MNCTDetectorEffectsEngineCOSI::GetNextEvent(Event) == false) {
    m_IsFinished = true;
    return false;
  }
    
  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderSimulation | MAssembly::c_DetectorEffectsEngine);
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleSimulationLoader::Finalize()
{
  // Initialize the module 

  MNCTDetectorEffectsEngineCOSI::Finalize();
}


///////////////////////////////////////////////////////////////////////////////


void MNCTModuleSimulationLoader::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsSimulationLoader* Options = new MGUIOptionsSimulationLoader(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleSimulationLoader::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* SimulationFileNameNode = Node->GetNode("SimulationFileName");
  if (SimulationFileNameNode != 0) {
    SetSimulationFileName(SimulationFileNameNode->GetValue());
  }
  MXmlNode* EnergyCalibrationFileNameNode = Node->GetNode("EnergyCalibrationFileName");
  if (EnergyCalibrationFileNameNode != 0) {
    SetEnergyCalibrationFileName(EnergyCalibrationFileNameNode->GetValue());
  }
  MXmlNode* DeadStripFileNameNode = Node->GetNode("DeadStripFileName");
  if (DeadStripFileNameNode != 0) {
    SetDeadStripFileName(DeadStripFileNameNode->GetValue());
  }
  MXmlNode* ThresholdFileNameNode = Node->GetNode("ThresholdFileName");
  if (ThresholdFileNameNode != 0) {
    SetThresholdFileName(ThresholdFileNameNode->GetValue());
  }
  MXmlNode* DepthCalibrationCoeffsFileNameNode = Node->GetNode("DepthCalibrationCoeffsFileName");
  if (DepthCalibrationCoeffsFileNameNode != 0) {
    SetDepthCalibrationCoeffsFileName(DepthCalibrationCoeffsFileNameNode->GetValue());
  }
  MXmlNode* DepthCalibrationSplinesFileNameNode = Node->GetNode("DepthCalibrationSplinesFileName");
  if (DepthCalibrationSplinesFileNameNode != 0) {
    SetDepthCalibrationSplinesFileName(DepthCalibrationSplinesFileNameNode->GetValue());
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleSimulationLoader::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "SimulationFileName", m_SimulationFileName);
  new MXmlNode(Node, "EnergyCalibrationFileName", m_EnergyCalibrationFileName);
  new MXmlNode(Node, "DeadStripFileName", m_DeadStripFileName);
  new MXmlNode(Node, "ThresholdFileName", m_ThresholdFileName);
  new MXmlNode(Node, "DepthCalibrationCoeffsFileName", m_DepthCalibrationCoeffsFileName);
  new MXmlNode(Node, "DepthCalibrationSplinesFileName", m_DepthCalibrationSplinesFileName);
  
  return Node;
}


// MNCTModuleSimulationLoader.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
