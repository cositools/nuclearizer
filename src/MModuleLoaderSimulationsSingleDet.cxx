/*
 * MModuleLoaderSimulationsSingleDet.cxx
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
// MModuleLoaderSimulationsSingleDet
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleLoaderSimulationsSingleDet.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsLoaderSimulations.h"
#include "MModuleEventSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleLoaderSimulationsSingleDet)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderSimulationsSingleDet::MModuleLoaderSimulationsSingleDet()
    : MModule()
{
  // Construct an instance of MModuleLoaderSimulationsSingleDet

  // Set the module name --- has to be unique
  m_Name = "Simulation loader and detector effects engine for COSI SingleDet";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagLoaderSimulationsSingleDet";

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

  m_UseStopAfter = false;
  m_MaximumAcceptedEvents = 10000000;
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderSimulationsSingleDet::~MModuleLoaderSimulationsSingleDet()
{
  // Delete this instance of MModuleLoaderSimulationsSingleDet
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderSimulationsSingleDet::Initialize()
{
  // Initialize the module

  MDetectorEffectsEngineSingleDet::SetGeometry(MModule::m_Geometry);
  if (MDetectorEffectsEngineSingleDet::Initialize() == false) {
    return false;
  }

  MSupervisor* S = MSupervisor::GetSupervisor();
  MModuleEventSaver* Saver = dynamic_cast<MModuleEventSaver*>(S->GetAvailableModuleByXmlTag("XmlTagEventSaver"));
  if (Saver != nullptr) {
    Saver->SetStartAreaFarField(m_StartAreaFarField);
  }

  m_AcceptedEvents = 0;

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderSimulationsSingleDet::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level

  if (m_UseStopAfter == true) {
    if (m_AcceptedEvents >= m_MaximumAcceptedEvents) {
      m_IsFinished = true;
      return false;
    }
  }

  if (MDetectorEffectsEngineSingleDet::GetNextEvent(Event) == false) {
    m_IsFinished = true;
    return false;
  }

  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderSimulation | MAssembly::c_DetectorEffectsEngine);

  m_AcceptedEvents++;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderSimulationsSingleDet::Finalize()
{
  // Initialize the module

  MSupervisor* S = MSupervisor::GetSupervisor();
  MModuleEventSaver* Saver = dynamic_cast<MModuleEventSaver*>(S->GetAvailableModuleByXmlTag("XmlTagEventSaver"));
  if (Saver != nullptr) {
    // Saver->SetSimulatedEvents(m_Reader->GetSimulatedEvents());
    Saver->SetSimulatedEvents(m_NumberOfSimulatedEvents);
  }

  MDetectorEffectsEngineSingleDet::Finalize();
}


///////////////////////////////////////////////////////////////////////////////


void MModuleLoaderSimulationsSingleDet::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsLoaderSimulations* Options = new MGUIOptionsLoaderSimulations(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderSimulationsSingleDet::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* SimulationFileNameNode = Node->GetNode("SimulationFileName");
  if (SimulationFileNameNode != 0) {
    SetSimulationFileName(SimulationFileNameNode->GetValue());
  }
  MXmlNode* DeadtimeFileNameNode = Node->GetNode("DeadtimeFileName");
  if (DeadtimeFileNameNode != 0) {
    SetDeadtimeFileName(DeadtimeFileNameNode->GetValue());
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
  MXmlNode* GuardRingThresholdFileNameNode = Node->GetNode("GuardRingThresholdFileName");
  if (GuardRingThresholdFileNameNode != 0) {
    SetGuardRingThresholdFileName(GuardRingThresholdFileNameNode->GetValue());
  }
  MXmlNode* ChargeSharingFileNameNode = Node->GetNode("ChargeSharingFileName");
  if (ChargeSharingFileNameNode != 0) {
    SetChargeSharingFileName(ChargeSharingFileNameNode->GetValue());
  }
  MXmlNode* CrosstalkFileNameNode = Node->GetNode("CrosstalkFileName");
  if (CrosstalkFileNameNode != 0) {
    SetCrosstalkFileName(CrosstalkFileNameNode->GetValue());
  }
  MXmlNode* ChargeLossFileNameNode = Node->GetNode("ChargeLossFileName");
  if (ChargeLossFileNameNode != 0) {
    SetChargeLossFileName(ChargeLossFileNameNode->GetValue());
  }
  MXmlNode* DepthCalibrationCoeffsFileNameNode = Node->GetNode("DepthCalibrationCoeffsFileName");
  if (DepthCalibrationCoeffsFileNameNode != 0) {
    SetDepthCalibrationCoeffsFileName(DepthCalibrationCoeffsFileNameNode->GetValue());
  }
  MXmlNode* DepthCalibrationTACCalFileNameNode = Node->GetNode("DepthCalibrationTACCalFileName");
  if (DepthCalibrationTACCalFileNameNode != 0) {
    SetDepthCalibrationTACCalFileName(DepthCalibrationTACCalFileNameNode->GetValue());
  }
  MXmlNode* DepthCalibrationSplinesFileNameNode = Node->GetNode("DepthCalibrationSplinesFileName");
  if (DepthCalibrationSplinesFileNameNode != 0) {
    SetDepthCalibrationSplinesFileName(DepthCalibrationSplinesFileNameNode->GetValue());
  }
  MXmlNode* ApplyFudgeFactorNode = Node->GetNode("ApplyFudgeFactor");
  if (ApplyFudgeFactorNode != 0) {
    m_ApplyFudgeFactor = ApplyFudgeFactorNode->GetValueAsBoolean();
  }
  MXmlNode* UseStopAfterNode = Node->GetNode("UseStopAfter");
  if (UseStopAfterNode != 0) {
    m_UseStopAfter = UseStopAfterNode->GetValueAsBoolean();
  }
  MXmlNode* MaximumAcceptedEventsNode = Node->GetNode("MaximumAcceptedEvents");
  if (MaximumAcceptedEventsNode != 0) {
    m_MaximumAcceptedEvents = MaximumAcceptedEventsNode->GetValueAsLong();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleLoaderSimulationsSingleDet::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "SimulationFileName", m_SimulationFileName);
  new MXmlNode(Node, "DeadtimeFileName", m_DeadtimeFileName);
  new MXmlNode(Node, "EnergyCalibrationFileName", m_EnergyCalibrationFileName);
  new MXmlNode(Node, "DeadStripFileName", m_DeadStripFileName);
  new MXmlNode(Node, "ThresholdFileName", m_ThresholdFileName);
  new MXmlNode(Node, "GuardRingThresholdFileName", m_GuardRingThresholdFileName);
  new MXmlNode(Node, "ChargeSharingFileName", m_ChargeSharingFileName);
  new MXmlNode(Node, "CrosstalkFileName", m_CrosstalkFileName);
  new MXmlNode(Node, "ChargeLossFileName", m_ChargeLossFileName);
  new MXmlNode(Node, "DepthCalibrationCoeffsFileName", m_DepthCalibrationCoeffsFileName);
  new MXmlNode(Node, "DepthCalibrationTACCalFileName", m_DepthCalibrationTACCalFileName);
  new MXmlNode(Node, "DepthCalibrationSplinesFileName", m_DepthCalibrationSplinesFileName);
  new MXmlNode(Node, "ApplyFudgeFactor", m_ApplyFudgeFactor);
  new MXmlNode(Node, "UseStopAfter", m_UseStopAfter);
  new MXmlNode(Node, "MaximumAcceptedEvents", m_MaximumAcceptedEvents);

  return Node;
}


// MModuleLoaderSimulationsSingleDet.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
