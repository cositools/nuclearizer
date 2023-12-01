/*
 * MModuleRevan.cxx
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
// MModuleRevan
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleRevan.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGUIOptionsRevan.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleRevan)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleRevan::MModuleRevan() : MModule()
{
  // Construct an instance of MModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Event reconstruction (Revan)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagRevan";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader, true);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration, true);
  AddPreceedingModuleType(MAssembly::c_StripPairing, true);
  AddPreceedingModuleType(MAssembly::c_DepthCorrection, true);
  AddPreceedingModuleType(MAssembly::c_PositionDetermiation, true);
 
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventReconstruction);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // Set if this module has an options GUI
  m_HasOptionsGUI = true;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleRevan::~MModuleRevan()
{
  delete m_RawEventAnalyzer;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleRevan::Initialize()
{
  // Initialize the module
  
  m_Settings = new MSettingsRevan();
  m_Settings->Read(m_RevanConfigurationFileName);

  m_ReconstructionGeometry = new MGeometryRevan();
  if (m_ReconstructionGeometry->ScanSetupFile(m_Geometry->GetFileName(), false) == false) {
    cout<<"Loading of geometry "<<m_ReconstructionGeometry->GetName()<<" failed!!"<<endl;
    return false;
  }

   // Initialize the raw event analyzer
  m_RawEventAnalyzer = new MRawEventAnalyzer();
  m_RawEventAnalyzer->SetGeometry(m_ReconstructionGeometry);
  m_RawEventAnalyzer->SetSettings(m_Settings);

  m_RawEventAnalyzer->SetSaveOI(true);
  m_RawEventAnalyzer->SetCoincidenceAlgorithm(MRawEventAnalyzer::c_CoincidenceAlgoNone);
  m_RawEventAnalyzer->SetEventClusteringAlgorithm(MRawEventAnalyzer::c_EventClusteringAlgoNone);
  m_RawEventAnalyzer->SetTrackingAlgorithm(MRawEventAnalyzer::c_TrackingAlgoNone);

  if (m_RawEventAnalyzer->PreAnalysis() == false) {
    cout<<"Revan pre-analysis failed!"<<endl;
    return false;
  }

  return MModule::Initialize();
}
  

////////////////////////////////////////////////////////////////////////////////


void MModuleRevan::Finalize()
{
  // Initialize the module 
  
  MModule::Finalize();
  
  delete m_RawEventAnalyzer;
  m_RawEventAnalyzer = nullptr;
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleRevan::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Write the event to disk
  
  if (Event->IsBad() == true) return true;
  
  MRERawEvent* RawEvent = new MRERawEvent();
  // --> will be deleted by the RawEventAnalyzer

  ostringstream Out;
  Event->StreamEvta(Out);

  RawEvent->Parse(Out.str(), 200);

  // Reconstruct
  m_RawEventAnalyzer->AddRawEvent(RawEvent);
  unsigned int ReturnCode = m_RawEventAnalyzer->AnalyzeEvent();

  if (ReturnCode == MRawEventAnalyzer::c_AnalysisSucess) {

    MRERawEvent* BestRawEvent = nullptr;
    if (m_RawEventAnalyzer->GetSingleOptimumEvent() != nullptr) {
      BestRawEvent = m_RawEventAnalyzer->GetSingleOptimumEvent();
    } else if (m_RawEventAnalyzer->GetSingleBestTryEvent() != nullptr) {
      BestRawEvent = m_RawEventAnalyzer->GetSingleBestTryEvent();
    }

    if (BestRawEvent != nullptr) {
      MPhysicalEvent* PE = BestRawEvent->GetPhysicalEvent();
      Event->SetPhysicalEvent(PE);
    } else {
      Event->SetEventReconstructionIncomplete(true);
    }
  } else {
    Event->SetEventReconstructionIncomplete(true);
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleRevan::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsRevan* Options = new MGUIOptionsRevan(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleRevan::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* RevanConfigurationFileNameNode = Node->GetNode("RevanConfigurationFileName");
  if (RevanConfigurationFileNameNode != nullptr) {
    m_RevanConfigurationFileName = RevanConfigurationFileNameNode->GetValueAsString();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleRevan::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(nullptr, m_XmlTag);  
  new MXmlNode(Node, "RevanConfigurationFileName", m_RevanConfigurationFileName);

  return Node;
}


// MModuleRevan.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
