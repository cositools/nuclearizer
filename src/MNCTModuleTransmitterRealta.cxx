/*
 * MNCTModuleTransmitterRealta.cxx
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
// MNCTModuleTransmitterRealta
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleTransmitterRealta.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"
#include "MGUIOptionsTransmitterRealta.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleTransmitterRealta)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleTransmitterRealta::MNCTModuleTransmitterRealta() : MNCTModule()
{
  // Construct an instance of MNCTModuleTransmitterRealta

  // Set all modules, which *have to be* done before this module
  AddPreceedingModuleType(c_EventLoader);
  //AddPreceedingModuleType(c_EventOrdering);
  //AddPreceedingModuleType(c_Aspect);
  AddPreceedingModuleType(c_EnergyCalibration);
  AddPreceedingModuleType(c_StripPairing);
  AddPreceedingModuleType(c_DepthCorrection);

  
  // Set all types this modules handles
  //AddModuleType(c_EventLoader);
  AddModuleType(c_EventTransmitter);

  // Set all modules, which *can* follow this module
  AddSucceedingModuleType(c_NoRestriction);
  
  // Set the module name --- has to be unique
  m_Name = "Transmitter for fully calibrated events to Realta";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagTransmitterRealta";  
  
  m_HasOptionsGUI = true;
  
  m_Transmitter = 0;
  m_HostName = "localhost";
  m_HostPort = 12354;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleTransmitterRealta::~MNCTModuleTransmitterRealta()
{
  // Delete this instance of MNCTModuleTransmitterRealta
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleTransmitterRealta::Initialize()
{
  // Initialize the module 

  // Set up the transceiver and connect:
  delete m_Transmitter;
  m_Transmitter = new MTransceiverTcpIp("Realta-Transmitter", m_HostName, m_HostPort, MTransceiverTcpIp::c_ModeRawEventList);
  //m_Transmitter->SetVerbosity(3);
  m_Transmitter->Connect();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleTransmitterRealta::AnalyzeEvent(MNCTEvent* Event) 
{
  // Convert the event to text and transmit it
  
  /*
  // For testing: randomly fill an event
  static int ID = 0;
  Event->SetID(++ID);
  Event->SetTime(0.01*ID);
  MNCTHit* H1 = new MNCTHit();
  H1->SetPosition(MVector(0.1, 0.2, 0.3));
  H1->SetEnergy(500);
  Event->AddHit(H1);
  MNCTHit* H2 = new MNCTHit();
  H2->SetPosition(MVector(1, 2, 3));
  H2->SetEnergy(1500);
  Event->AddHit(H2);
  Event->SetDataRead(true);
  */
  
  if (Event->IsBad() == true) return true;

  static int ID = 0;
  static double Time = 0.0;
  Event->SetTime(Time += 0.01);
  Event->SetID(++ID);
  
  ostringstream estream;
  Event->StreamEvta(estream);
  estream<<"EN"<<endl;
  m_Transmitter->Send(estream.str());
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleTransmitterRealta::Finalize()
{
  // Initialize the module
  
  delete m_Transmitter;
  m_Transmitter = 0;

  return;
}


///////////////////////////////////////////////////////////////////////////////


void MNCTModuleTransmitterRealta::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsTransmitterRealta* Options = new MGUIOptionsTransmitterRealta(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleTransmitterRealta::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* HostNameNode = Node->GetNode("HostName");
  if (HostNameNode != 0) {
    m_HostName = HostNameNode->GetValue();
  }
  MXmlNode* HostPortNode = Node->GetNode("HostPort");
  if (HostPortNode != 0) {
    m_HostPort = HostPortNode->GetValueAsInt();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleTransmitterRealta::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "HostName", m_HostName);
  new MXmlNode(Node, "HostPort", m_HostPort);
  
  return Node;
}


// MNCTModuleTransmitterRealta.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
