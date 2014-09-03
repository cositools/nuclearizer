/*
 * MNCTModuleReceiverCOSI2014.cxx
 *
 *
 * Copyright (C) by Alex Lowell & Andreas Zoglauer.
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
#include <algorithm>
#include <cstdio>
using namespace std;
#include <time.h>

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsReceiverCOSI2014.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleReceiverCOSI2014)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleReceiverCOSI2014::MNCTModuleReceiverCOSI2014() : MModule(), MNCTBinaryFlightDataParser()
{
  // Construct an instance of MNCTModuleReceiverCOSI2014

  // Set all modules, which have to be done before this module
  // None
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventLoader);
  AddModuleType(MAssembly::c_EventLoaderMeasurement);
  AddModuleType(MAssembly::c_EventOrdering);
  AddModuleType(MAssembly::c_Aspect);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // Set the module name --- has to be unique
  m_Name = "Data packet receiver, sorter, and aspect reconstructor for COSI 2014";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagReceiverCOSI2014";  
  
  m_HasOptionsGUI = true;
  
  m_DistributorName = "localhost";
  m_DistributorPort = 9091;
  m_DistributorStreamID = "OP";
  
  m_LocalReceivingHostName = "localhost";
  m_LocalReceivingPort = 12345;  
  
  m_Receiver = 0;
  
  m_NAllowedWorkerThreads = 1;
  m_IsStartModule = true;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleReceiverCOSI2014::~MNCTModuleReceiverCOSI2014()
{
  // Delete this instance of MNCTModuleReceiverCOSI2014
}

////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::DoHandshake()
{
  // Perform a handshake with the distributor
  
  // TODO: Add a timeout!

  bool HandshakeSuccessful = false;
  MTransceiverTcpIpBinary* Handshaker = 0;
  
  while (HandshakeSuccessful == false && m_Interrupt == false) {
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
    while (Handshaker->IsConnected() == false && --Wait > 0 && m_Interrupt == false) {
      gSystem->Sleep(10);
      gSystem->ProcessEvents();
      continue;
    }
    if (Handshaker->IsConnected() == false && Wait == 0) {
      cout<<"Never connected - disconnecting"<<endl;
      Handshaker->Disconnect(true);
      delete Handshaker;
      Handshaker = 0;
      gSystem->ProcessEvents();
      gSystem->Sleep(1000*gRandom->Rndm());
      gSystem->ProcessEvents();
      cout<<"Done sleeping"<<endl;
      continue;
    }
    
    ostringstream msg;
    if (m_DistributorStreamID == "ALL" || m_DistributorStreamID == "") { 
      msg<<"START:"<<m_LocalReceivingHostName<<":"<<m_LocalReceivingPort;
    } else {
      msg<<"START:"<<m_LocalReceivingHostName<<":"<<m_LocalReceivingPort<<":"<<m_DistributorStreamID;        
    }
    vector<uint8_t> ToSend;
    for (char c: msg.str()) {
      ToSend.push_back(static_cast<uint8_t>(c));
    }
    Handshaker->Send(ToSend);
    cout<<"Sent connection request: "<<msg.str()<<endl;
    
    MTimer Waiting;
    Wait = 0;
    bool Restart = false;
    while (Handshaker->IsConnected() == true && Restart == false && m_Interrupt == false) {
      cout<<"Waiting for a reply since "<<Waiting.GetElapsed()<<" sec (up to 60 sec).."<<endl;
      ++Wait;
      gSystem->ProcessEvents();
      if (Wait < 10) {
        gSystem->Sleep(100);
      } else {
        gSystem->Sleep(1000);
      }
      // Need a timeout here
      if (Waiting.GetElapsed() > 60) {
        cout<<"Connected but didn't receive anything -- timeout & restarting"<<endl;
        Handshaker->Disconnect();
        delete Handshaker;
        Handshaker = 0;
        gSystem->ProcessEvents();
        gSystem->Sleep(1000*gRandom->Rndm());
        gSystem->ProcessEvents();
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
          gSystem->ProcessEvents();
          gSystem->Sleep(1000*gRandom->Rndm());
          gSystem->ProcessEvents();
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
      gSystem->ProcessEvents();
      gSystem->Sleep(1000*gRandom->Rndm());
      gSystem->ProcessEvents();
    }
  }
  
  if (m_Interrupt == true) return false;
  
  // Set up the transceiver and connect:
  delete m_Receiver;
  m_Receiver = new MTransceiverTcpIpBinary("Final receiver", m_LocalReceivingHostName, m_LocalReceivingPort);
  m_Receiver->SetVerbosity(3);
  m_Receiver->SetMaximumBufferSize(100000000);
  m_Receiver->Connect(true, 10);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::Initialize()
{
  // Initialize the module 

  // Do handshake and open transceiver
  if (DoHandshake() == false) {
    if (m_Interrupt == true) return false;
    merr<<"Failed to connect to distributor"<<endl;
    return false;
  }
    
  if (m_RoaFileName != "") {
    MTime Now;
    MString TimeStamp(".");
    TimeStamp += Now.GetShortString();
    TimeStamp += ".roa";
    MString FileName = m_RoaFileName;
    if (m_RoaFileName.EndsWith(".roa")) {
      FileName.ReplaceAllInPlace(".roa", TimeStamp);
    } else {
      FileName += TimeStamp; 
    }
    m_Out.open(FileName);
    m_Out<<"TYPE ROA"<<endl;
    m_Out<<"UF doublesidedstrip adcwithtiming"<<endl;
    m_Out<<endl;
  }
  
  if (MNCTBinaryFlightDataParser::Initialize() == false) return false;
  
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::IsReady() 
{
  if (m_Events.size() > 0) {
    if (m_IgnoreAspect == true) {
      return true;
    } else {
      MNCTAspect* A = m_Events[0]->GetAspect();
      if( A != 0 ){
        return true;
      }
    }
  }
  
  vector<uint8_t> Received ;
  m_Receiver->Receive(Received);
  
  return ParseData(Received);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // IsReady() ensured that the oldest event in the list has a reconstructed aspect
  MReadOutAssembly * NewEvent;
  
  if (m_Events.size() == 0) {
    cout<<"ERROR in MNCTModuleReceiverCOSI2014::AnalyzeEvent: No events"<<endl;
    cout<<"This function should have never been called when we have no events"<<endl;
    return false;
  }

  NewEvent = m_Events[0];
  m_Events.pop_front();

  //this checks if the event's aspect data was within the range of the retrieved aspect info
  if( NewEvent->GetAspect()->GetOutOfRange() ){
    delete NewEvent;
    return false;
  }

  while( NewEvent->GetNStripHits() > 0 ){
    Event->AddStripHit( NewEvent->GetStripHit(0) );
    NewEvent->RemoveStripHit(0);
  }

  Event->SetID( NewEvent->GetID() );
  Event->SetFC( NewEvent->GetFC() );
  Event->SetTI( NewEvent->GetTI() );
  Event->SetCL( NewEvent->GetCL() );
  Event->SetTime( NewEvent->GetTime() );
  Event->SetMJD( NewEvent->GetMJD() );
  if (NewEvent->GetAspect() != 0) {
    Event->SetAspect(new MNCTAspect(*(NewEvent->GetAspect())) );
    Event->SetAnalysisProgress(MAssembly::c_Aspect);
  }
  Event->SetAnalysisProgress(MAssembly::c_EventLoader | 
                             MAssembly::c_EventLoaderMeasurement | 
                             MAssembly::c_EventOrdering);

  if (m_RoaFileName != "") {
    Event->StreamRoa(m_Out);
  }
  
  delete NewEvent;
  

  // TODO: Just *copy* the data from the OLDEST event in the list to this event  

  // TODO: Remove the oldest events from the list
  //delete m_Events.front();
  //m_Events.pop_front();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleReceiverCOSI2014::Finalize()
{
  // Close the tranceiver 

  MModule::Finalize();
  MNCTBinaryFlightDataParser::Finalize();
  
  if (m_RoaFileName != "") {
    m_Out.close();
  }
  
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

  MXmlNode* LocalReceivingHostNameNode = Node->GetNode("LocalReceivingHostName");
  if (LocalReceivingHostNameNode != 0) {
    m_LocalReceivingHostName = LocalReceivingHostNameNode->GetValue();
  }
  MXmlNode* LocalReceivingPortNode = Node->GetNode("LocalReceivingPort");
  if (LocalReceivingPortNode != 0) {
    m_LocalReceivingPort = LocalReceivingPortNode->GetValueAsInt();
  }
  

  MXmlNode* DataSelectionModeNode = Node->GetNode("DataSelectionMode");
  if (DataSelectionModeNode != 0) {
    m_DataSelectionMode = (MNCTBinaryFlightDataParserDataModes) LocalReceivingHostNameNode->GetValueAsInt();
  }
  

  MXmlNode* RoaFileNameNode = Node->GetNode("RoaFileName");
  if (RoaFileNameNode != 0) {
    m_RoaFileName = RoaFileNameNode->GetValueAsString();
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

  new MXmlNode(Node, "LocalReceivingHostName", m_LocalReceivingHostName);
  new MXmlNode(Node, "LocalReceivingPort", m_LocalReceivingPort);

  new MXmlNode(Node, "DataSelectionMode", (unsigned int) m_DataSelectionMode);

  new MXmlNode(Node, "RoaFileName", m_RoaFileName);
  
  return Node;
}


// MNCTModuleReceiverCOSI2014.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
