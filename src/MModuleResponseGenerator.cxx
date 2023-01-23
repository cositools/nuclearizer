/*
 * MModuleResponseGenerator.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Jau-Shian Liang.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MModuleResponseGenerator
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleResponseGenerator.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MTime.h"
#include "MGUIOptionsResponseGenerator.h"
#include "MResponseSpectral.h"
#include "MResponseClusteringDSS.h"
#include "MResponseImagingEfficiency.h"
#include "MResponseMultipleCompton.h"
#include "MResponseImagingARM.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleResponseGenerator)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleResponseGenerator::MModuleResponseGenerator() : MModule()
{
  // Construct an instance of MModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Response creator";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagResponseGenerator";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader, true);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration, true);
  AddPreceedingModuleType(MAssembly::c_StripPairing, true);
  AddPreceedingModuleType(MAssembly::c_DepthCorrection, true);
  AddPreceedingModuleType(MAssembly::c_PositionDetermiation, true);
 
  // Set all types this modules handles
  AddModuleType(MAssembly::c_ResponseGeneration);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // Set if this module has an options GUI
  m_HasOptionsGUI = true;
  
  m_Mode = c_Spectrum;
  m_ResponseName = "Response";
  m_Response = nullptr;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleResponseGenerator::~MModuleResponseGenerator()
{
  delete m_Response;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleResponseGenerator::Initialize()
{
  // Initialize the module
  
  delete m_Response;
  
  if (m_Mode == c_Spectrum) {
    MResponseSpectral* Response = new MResponseSpectral();
    m_Response = Response; 
  } else if (m_Mode == c_Clustering) {
    MResponseClusteringDSS* Response = new MResponseClusteringDSS();
    m_Response = Response; 
  } else if (m_Mode == c_Efficiency) {
    MResponseImagingEfficiency* Response = new MResponseImagingEfficiency();
    m_Response = Response; 
  } else if (m_Mode == c_BayesianER) {
    MResponseMultipleCompton* Response = new MResponseMultipleCompton();
    m_Response = Response; 
  } else if (m_Mode == c_Imaging) {
    MResponseImagingARM* Response = new MResponseImagingARM();
    m_Response = Response; 
  }
  
  m_Response->SetGeometryFileName(m_Geometry->GetFileName());
  m_Response->SetResponseName(MString(gSystem->WorkingDirectory()) + "/" + m_ResponseName);
    
  m_Response->SetCompression(true);
  m_Response->SetSaveAfterNumberOfEvents(100000);
    
  m_Response->SetRevanSettingsFileName(m_RevanConfigurationFileName);
  m_Response->SetMimrecSettingsFileName(m_MimrecConfigurationFileName);
 
  if (m_Response->Initialize() == false) return false;
  
  return MModule::Initialize();
}
  

////////////////////////////////////////////////////////////////////////////////


void MModuleResponseGenerator::Finalize()
{
  // Initialize the module 
  
  MModule::Finalize();
  
  m_Response->Finalize();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleResponseGenerator::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Write the event to disk
  
  if (Event->IsBad() == true) return true;
  
  ostringstream Out;
  Event->StreamEvta(Out);
  
  if (m_Response->SetEvent(MString(Out.str()), false, 25) == false) {
    cout<<"Unable to set event"<<endl;
    return true;
  }
  
  if (m_Response->Analyze() == false) {
    cout<<"Analysis failed"<<endl; 
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleResponseGenerator::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsResponseGenerator* Options = new MGUIOptionsResponseGenerator(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleResponseGenerator::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* ModeNode = Node->GetNode("Mode");
  if (ModeNode != nullptr) {
    m_Mode = ModeNode->GetValueAsUnsignedInt();
  }
  MXmlNode* ResponseNameNode = Node->GetNode("Name");
  if (ResponseNameNode != nullptr) {
    m_ResponseName = ResponseNameNode->GetValueAsString();
  }  
  MXmlNode* MimrecConfigurationFileNameNode = Node->GetNode("MimrecConfigurationFileName");
  if (MimrecConfigurationFileNameNode != nullptr) {
    m_MimrecConfigurationFileName = MimrecConfigurationFileNameNode->GetValueAsString();
  }
  MXmlNode* RevanConfigurationFileNameNode = Node->GetNode("RevanConfigurationFileName");
  if (RevanConfigurationFileNameNode != nullptr) {
    m_RevanConfigurationFileName = RevanConfigurationFileNameNode->GetValueAsString();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleResponseGenerator::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(nullptr, m_XmlTag);  
  new MXmlNode(Node, "Mode", m_Mode);
  new MXmlNode(Node, "Name", m_ResponseName);
  new MXmlNode(Node, "MimrecConfigurationFileName", m_MimrecConfigurationFileName);
  new MXmlNode(Node, "RevanConfigurationFileName", m_RevanConfigurationFileName);

  return Node;
}


// MModuleResponseGenerator.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
