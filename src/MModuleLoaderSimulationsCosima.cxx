/*
 * MModuleLoaderSimulationsCosima.cxx
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
// MModuleLoaderSimulationsCosima
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleLoaderSimulationsCosima.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsLoaderSimulationsCosima.h"
#include "MModuleEventSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleLoaderSimulationsCosima)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderSimulationsCosima::MModuleLoaderSimulationsCosima() : MModule()
{
  // Construct an instance of MModuleLoaderSimulationsCosima

  // Set the module name --- has to be unique
  m_Name = "Cosima simulation loader";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagLoaderSimulations";
  
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

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  m_HasOptionsGUI = true;
  
  m_UseStopAfter = false;
  m_MaximumAcceptedEvents = 10000000;
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderSimulationsCosima::~MModuleLoaderSimulationsCosima()
{
  // Delete this instance of MModuleLoaderSimulationsCosima
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderSimulationsCosima::Initialize()
{
  // Initialize the module

  m_Reader = new MFileEventsSim(m_Geometry);
  if (m_Reader->Open(m_SimulationFileName) == false) {
    cout<<"Unable to open sim file "<<m_SimulationFileName<<" - Aborting!"<<endl;
    return false;
  }
  m_StartAreaFarField = m_Reader->GetSimulationStartAreaFarField();

  
  MSupervisor* S = MSupervisor::GetSupervisor();
  MModuleEventSaver* Saver = dynamic_cast<MModuleEventSaver*>(S->GetAvailableModuleByXmlTag("XmlTagEventSaver"));
  if (Saver != nullptr) {
    Saver->SetStartAreaFarField(m_StartAreaFarField);
  }
  
  m_AcceptedEvents = 0;
  
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderSimulationsCosima::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  if (m_UseStopAfter == true) {
    if (m_AcceptedEvents >= m_MaximumAcceptedEvents) {
      m_IsFinished = true;
      return false;
    }
  }
  
  MSimEvent* SimEvent = m_Reader->GetNextEvent(false);

  if (SimEvent == nullptr) {
    m_IsFinished = true;
    return false;
  }

  Event->SetSimulatedEvent(SimEvent); // event becomes owner
  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderSimulation);
  
  m_AcceptedEvents++;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderSimulationsCosima::Finalize()
{
  // Initialize the module 

  MModule::Finalize();

  MSupervisor* S = MSupervisor::GetSupervisor();
  MModuleEventSaver* Saver = dynamic_cast<MModuleEventSaver*>(S->GetAvailableModuleByXmlTag("XmlTagEventSaver"));
  if (Saver != nullptr) {
    // Saver->SetSimulatedEvents(m_Reader->GetSimulatedEvents());
    Saver->SetSimulatedEvents(m_NumberOfSimulatedEvents);
  }    
  
}


///////////////////////////////////////////////////////////////////////////////


void MModuleLoaderSimulationsCosima::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsLoaderSimulationsCosima* Options = new MGUIOptionsLoaderSimulationsCosima(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderSimulationsCosima::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* SimulationFileNameNode = Node->GetNode("SimulationFileName");
  if (SimulationFileNameNode != 0) {
    SetSimulationFileName(SimulationFileNameNode->GetValue());
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleLoaderSimulationsCosima::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "SimulationFileName", m_SimulationFileName);
  
  return Node;
}


// MModuleLoaderSimulationsCosima.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
