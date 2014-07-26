/*
 * MNCTModuleReceiverCOSI2014.cxx
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
// MNCTModuleReceiverCOSI2014
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleReceiverCOSI2014.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"
#include "MGUIOptionsReceiverCOSI2014.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleReceiverCOSI2014)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleReceiverCOSI2014::MNCTModuleReceiverCOSI2014() : MNCTModule()
{
  // Construct an instance of MNCTModuleReceiverCOSI2014

  // Set all modules, which have to be done before this module
  // None
  
  // Set all types this modules handles
  AddModuleType(c_EventLoader);
  AddModuleType(c_EventLoaderMeasurement);
  AddModuleType(c_EventOrdering);
  AddModuleType(c_Aspect);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(c_EnergyCalibration);
  AddSucceedingModuleType(c_CrosstalkCorrection);
  AddSucceedingModuleType(c_ChargeSharingCorrection);
  AddSucceedingModuleType(c_DepthCorrection);
  AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_Else);
  AddSucceedingModuleType(c_EventReconstruction);
  AddSucceedingModuleType(c_EventSaver);
  
  // Set the module name --- has to be unique
  m_Name = "Data packet receiver, sorter, and aspect reconstructor for COSI 2014";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagReceiverCOSI2014";  
  
  m_HasOptionsGUI = true;
  
  m_Receiver = 0;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleReceiverCOSI2014::~MNCTModuleReceiverCOSI2014()
{
  // Delete this instance of MNCTModuleReceiverCOSI2014
  
  for (auto E: m_Events) {
    delete E;
  }
  m_Events.clear();
}

////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::DoHandshake()
{
  // Perform a handshake with the distributor
  
  // TODO: Add a timeout!
  
  bool Interrupt = false;
  
  bool HandshakeSuccessful = false;
  MTransceiverTcpIpBinary* Handshaker = 0;
  
  while (HandshakeSuccessful == false && Interrupt == false) {
    if (Handshaker != 0) {
      cout<<"HANDSHAKER NOT DELETED!"<<endl;
      delete Handshaker;
      Handshaker = 0;
    }
    cout<<"Trying to connect to: "<<m_DistributorName<<":"<<m_DistributorPort<<endl;
    Handshaker = new MTransceiverTcpIpBinary("HandShaker", m_DistributorName, m_DistributorPort);
    Handshaker->SetVerbosity(3);
    Handshaker->RequestClient(true);
    Handshaker->Connect(false);
    int Wait = 2000;
    while (Handshaker->IsConnected() == false && --Wait > 0 && Interrupt == false) {
      gSystem->Sleep(1);
      continue;
    }
    if (Handshaker->IsConnected() == false && Wait == 0) {
      cout<<"Never connected - disconnecting"<<endl;
      Handshaker->Disconnect(true);
      delete Handshaker;
      Handshaker = 0;
      gSystem->Sleep(1000*gRandom->Rndm());
      cout<<"Done sleeping"<<endl;
      continue;
    }
    
    ostringstream msg;
    if (m_DistributorStreamID == "ALL" || m_DistributorStreamID == "") { 
      msg<<"START:"<<m_LocalReceivingHost<<":"<<m_LocalReceivingPort;
    } else {
      msg<<"START:"<<m_LocalReceivingHost<<":"<<m_LocalReceivingPort<<":"<<m_DistributorStreamID;        
    }
    vector<uint8_t> ToSend;
    for (char c: msg.str()) {
      ToSend.push_back(static_cast<uint8_t>(c));
    }
    Handshaker->Send(ToSend);
    cout<<"Sent connection request: "<<msg.str()<<endl;
    
    Wait = 10000;
    bool Restart = false;
    while (Handshaker->IsConnected() == true && Restart == false && Interrupt == false) {
      gSystem->Sleep(1);
      // Need a timeout here
      if (--Wait == 0) {
        Handshaker->Disconnect();
        delete Handshaker;
        Handshaker = 0;
        gSystem->Sleep(1000*gRandom->Rndm());
        Restart = true;
        break;
      }
      vector<uint8_t> ToReceive;
      Handshaker->Receive(ToReceive);
      if (ToReceive.size() > 0) {
        string ReceivedString(ToReceive.begin(), ToReceive.end());
        
        string Expected(msg.str());
        Expected += ":ACK";
        if (ReceivedString == Expected) {
          cout<<"Handshake successfull"<<endl;
          HandshakeSuccessful = true;
          Handshaker->Disconnect();
          delete Handshaker;
          break;
        } else {
          cout<<"Error performing handshake: "<<ReceivedString<<endl;
          Handshaker->Disconnect();
          delete Handshaker;
          Handshaker = 0;
          gSystem->Sleep(1000*gRandom->Rndm());
          Restart = true;
          break;
        }
      } else {
        //cout<<"Nothing received..."<<endl; 
      }
    }
    if (HandshakeSuccessful == false && Handshaker != 0) {
      Handshaker->Disconnect();
      delete Handshaker;
      Handshaker = 0;
      gSystem->Sleep(1000*gRandom->Rndm());
    }
  }
  
  // Set up the transceiver and connect:
  delete m_Receiver;
  m_Receiver = new MTransceiverTcpIpBinary("Final receiver", m_LocalReceivingHost, m_LocalReceivingPort);
  m_Receiver->SetVerbosity(3);
  m_Receiver->Connect(true, 10);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::Initialize()
{
  // Initialize the module 

  m_LocalReceivingHost = "localhost";
  m_LocalReceivingPort = 12345;

  for (auto E: m_Events) {
    delete E;
  }
  m_Events.clear();
  
  // Do handshake and open transceiver
  if (DoHandshake() == false) {
    merr<<"Failed to connect to distributor"<<endl;
    return false;
  }
  
  // Load aspect reconstruction module
  delete m_AspectReconstructor;
  m_AspectReconstructor = new MNCTAspectReconstruction();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::IsReady() 
{
  // Do the actual work here, not in analyze event!
  
  // Check if the receiver is still up and running, if not reconnect
  // TODO: Add time out to DoHandshake
  if (m_Receiver == 0 || m_Receiver->IsConnected() == false) {
    if (DoHandshake() == false) {
      merr<<"Failed to connect to distributor"<<endl;
      return false;
    }
    if (m_Receiver == 0 || m_Receiver->IsConnected() == false) {
      merr<<"Unable to establish connection!"<<endl;
      return false;
    }
  }

  // Retrieve the latest data from the transceiver
  vector<uint8_t> Received;
  m_Receiver->Receive(Received);

  // TODO: Split it into packets & determine what they are and where they go
  
  // TODO: Parse the events into MNCTEvent*'s and store the
  
  // TODO: Hand over the aspects to the aspect reconstructor
  //m_AspectReconstructor->AddFrame(...);
  
  // TODO: If we have events at least N seconds in the future from all card cages
  // from the oldest event, we are good to *sort* the events until a certain time
  // and do coincidence search
  // Otherwise, we are not yet ready and have to return false
  
  // TODO: Add to as many events as possible reconstructed aspect information
  for (auto E: m_Events) {
    if (E->GetAspect() == 0) {
      MNCTAspect* A = m_AspectReconstructor->GetAspect(E->GetTime());
      if (A != 0) {
        E->SetAspect(A);
      }
    }
  }
  
  // TODO: If the oldest event has a reconstructed event, we are ready for the analyze function,
  // thus we return true otehrwise false
  if (m_Events.begin() != m_Events.end()) {
    if (m_Events.front()->GetAspect() != 0) {
      return true;
    }
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::AnalyzeEvent(MNCTEvent* Event) 
{
  // IsReady() ensured that the oldest event in the list has a reconstructed aspect
  
  // TODO: Just *copy* the data from the OLDEST event in the list to this event  

  // TODO: Remove the oldest events from the list
  delete m_Events.front();
  m_Events.pop_front();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleReceiverCOSI2014::Finalize()
{
  // Close the tranceiver 
  
  // TODO: Clear all lists

  return;
}


///////////////////////////////////////////////////////////////////////////////


void MNCTModuleReceiverCOSI2014::ShowOptionsGUI()
{
  // Show the options GUI

  MGUIOptionsReceiverCOSI2014* Options = new MGUIOptionsReceiverCOSI2014(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* DistributorNameNode = Node->GetNode("DistributorName");
  if (DistributorNameNode != 0) {
    m_DistributorName = DistributorNameNode->GetValue();
  }
  MXmlNode* DistributorPortNode = Node->GetNode("DistributorPort");
  if (DistributorPortNode != 0) {
    m_DistributorPort = DistributorPortNode->GetValueAsInt();
  }
  MXmlNode* DistributorStreamIDNode = Node->GetNode("DistributorStreamID");
  if (DistributorStreamIDNode != 0) {
    m_DistributorStreamID = DistributorStreamIDNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleReceiverCOSI2014::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "DistributorName", m_DistributorName);
  new MXmlNode(Node, "DistributorPort", m_DistributorPort);
  new MXmlNode(Node, "DistributorStreamID", m_DistributorStreamID);
  
  return Node;
}


// MNCTModuleReceiverCOSI2014.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
