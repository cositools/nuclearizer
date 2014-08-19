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
bool MNCTEventTimeCompare(MNCTEvent * E1, MNCTEvent * E2);


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
  AddSucceedingModuleType(c_NoRestriction);
  
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

  m_DataSelectionMode = MNCTModuleReceiverCOSI2014DataModes::c_All;
  
  
  m_Receiver = 0;

  m_UseComptonDataframes = true;
  m_UseRawDataframes = true;
  m_NumRawDataframes = 0;
  m_NumComptonDataframes = 0;
  m_NumAspectPackets = 0;
  m_NumOtherPackets = 0;
  MAX_TRIGS = 80;
  LastTimestamps.resize(12);
  dx = 0;
  m_EventTimeWindow = 5 * 10000000;
  m_ComptonWindow = 2;

  LoadStripMap();
  LoadCCMap();

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

  for (auto E: m_Events) {
    delete E;
  }
  m_Events.clear();
  for (auto E: m_EventsBuf) {
    delete E;
  }
  m_EventsBuf.clear();
  
  // Do handshake and open transceiver
  if (DoHandshake() == false) {
    if (m_Interrupt == true) return false;
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

	uint8_t Type;
	uint16_t Len;
	vector <MNCTEvent*> NewEvents;
	vector<unsigned char> SyncWord;
	dataframe * Dataframe;
	int ParseErr;
	bool SyncError;
	int CCId;

	if( m_Events.size() > 0 ){
		return true;
	}

	SyncWord.push_back(0xEB);
	SyncWord.push_back(0x90);

	// Do the actual work here, not in analyze event!
	/*
	for( auto E: m_Events ){
     //m_Events.pop_front();
	  delete E;
	}
	m_Events.clear();
	*/

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
	vector<uint8_t> Received ;
	m_Receiver->Receive(Received);

	//apend the received data to m_SBuf
	m_SBuf.insert( m_SBuf.end(), Received.begin(), Received.end() );
	//FindNextPacket handles all the resyncing etc...

	vector<uint8_t> NextPacket;
	//dx = 0;
  int Rounds = 100;
	while( FindNextPacket( NextPacket ) && m_Interrupt == false && --Rounds > 0 ){ //loop until there are no more complete packets

		Type = NextPacket[2];

		switch( Type ){
			case 0x00:
				//raw dataframe
				if( m_UseRawDataframes ){
					//convert dataframe to class
					//clear out first ten bytes 
					//LEAK
					Dataframe = new dataframe();
					ParseErr = RawDataframe2Struct( NextPacket, Dataframe );
					if( ParseErr >= 0 ){
						ConvertToMNCTEvents( Dataframe, &NewEvents );
						CCId = Dataframe->CCId;
					}
					//cout<<"made "<<NewEvents.size()<<" MNCTEvents"<<endl;
					delete Dataframe;
					//LEAK
					m_NumRawDataframes++;
				}
				break;
			case 0x01:
				//compton dataframe
				if( m_UseComptonDataframes ){
					//convert to dataframe class
					//cout<<"got compton dataframe!"<<endl;
					m_NumComptonDataframes++;
				}
				break;
			case 0x05:
				//aspect packet
				cout<<"got aspect packet!"<<endl;
				m_NumAspectPackets++;
				break;
			default:
				//don't care
				m_NumOtherPackets++;

		}
		if( NewEvents.size() > 0 ){
			for( auto E: NewEvents ){
				//push the events onto the deque
				//before we push the event into m_EventsBuf, check if we have a sync problem
				if( E->GetCL() > LastTimestamps[CCId] ) LastTimestamps[CCId] = E->GetCL(); else {
					//sync problem detected...
					//cout<<"sync error on CC "<<CCId<<", det "<<m_CCMap[CCId]<<", flushing m_EventsBuf"<<endl;
					FlushEventsBuf();
					LastTimestamps[CCId] = E->GetCL();
				}

				m_EventsBuf.push_back(E);
			}
			NewEvents.clear();

			cout<<"T ::: ";;
			for( auto E: LastTimestamps ){
				cout<<std::hex<<E<<" ";
			}
			cout<<endl;

			//now sort m_EventsBuf
			SortEventsBuf();
			//look thru m_EventsBuf for multi-detector events
			CheckEventsBuf();
			/*
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
			 */
		}

	} 


	if( m_Events.size() > 0 ){
		return true;
	} else {
		return false;
	}

}

bool MNCTModuleReceiverCOSI2014::FindNextPacket(vector<uint8_t>& NextPacket , int * idx){

	//return true if a complete packet was found, return packet in NextPacket
	//return false if a complete packet was not found.  also copy the leftover bytes
	//back to the beginning

	//idx is the value of dx that points to the beginning of the packet in m_SBuf

	//assert: search buf is either synced or empty

	uint16_t Len;
	bool FoundPacket;

	FoundPacket = false;
	NextPacket.clear();

	if( (dx + 2) > m_SBuf.size() ){
		//not enough bytes to check for sync
		return false;
	}

	while( !(m_SBuf[dx] == 0xeb && m_SBuf[dx+1] == 0x90) ){
		ResyncSBuf();
		if( m_SBuf.size() == 0 ){
			return false;
		}
	}

	//we are synced and the buffer is not empty
	if( (dx + 10) > m_SBuf.size() ){
		//not enough bytes to compute len
		return false;
	}

	Len = ((uint16_t)m_SBuf[dx+8]<<8) | ((uint16_t)m_SBuf[dx+9]);
	if( Len > 1360 ){
		//got a weird value, could be a spurious eb90, Resync() and exit
		ResyncSBuf();
		return false;
	}

	if( (dx + Len) > m_SBuf.size() ){
		//we don't have the complete packet
		//this should happen often since TCP will give us a bunch of bytes w/o boundaries 
		//move the data up so as to clear out the packets we have already processed
		m_SBuf.assign( m_SBuf.begin() + dx, m_SBuf.end() );
		dx = 0;
		return false;
	}

	FoundPacket = true;

	//we have a complete packet, extract it
	NextPacket.assign( m_SBuf.begin() + dx, m_SBuf.begin() + dx + Len);
	//store the location of beginning of this packet
	if( idx != NULL ){
		*idx = dx;
	}
	//increment the index...we should be pointing at the next 0xeb 
	dx += Len;

	//check if our buffer is too big, move it up if it is
	if( m_SBuf.size() > 1000000 ){
		//a mega byte
		m_SBuf.assign( m_SBuf.begin() + dx, m_SBuf.end() );
		dx = 0;
	}

	return true;

}

bool MNCTModuleReceiverCOSI2014::ResyncSBuf(void){


	//this method makes sure that either m_SBuf[dx] = 0xeb and m_SBuf[dx+1] = 0x90
	//or that the buffer is empty and dx = 0;

	//start from the +1th element when searching for 0xeb, or check if the current
	//Buf[dx] is eb, and if it is then + 1

	cout<<"resyncing input stream!"<<endl;

	int i;
	bool FoundSync;

	if( m_SBuf.size() == 0 ){
		dx = 0;
		FoundSync = false;
		return FoundSync;
	}

	//we might be pointing at a spurious 0xeb 0x90, rare case... add 1 so that
	//the for loop below doesn't think its on a valid sync word
	if (m_SBuf[dx] == 0xeb) ++dx;

	FoundSync = false;
	for( i = dx; i > m_SBuf.size()-1; ++i ){
		if( m_SBuf[i] == 0xeb ){
			if( m_SBuf[i+1] == 0x90 ){
				FoundSync = true;
				dx = i;
				break;
			}
		}
	}

	if( !FoundSync ){
		m_SBuf.clear();
		dx = 0;
	} 

	return FoundSync;

}

bool MNCTModuleReceiverCOSI2014::FlushEventsBuf(void){

	int MergedEventCounter = 0;

	//don't check m_EventTimeWindow, we are flushing the buffer
	while( m_EventsBuf.size() > 0){
		MNCTEvent * FirstEvent = m_EventsBuf.front(); m_EventsBuf.pop_front();
		deque<MNCTEvent*> EventList;
		EventList.push_back(FirstEvent);
		//now check if the next events are within the compton window
		while( m_EventsBuf.size() > 0 ){
			if( (m_EventsBuf[0]->GetCL() - FirstEvent->GetCL()) <= m_ComptonWindow ){
				EventList.push_back( m_EventsBuf.front() ); m_EventsBuf.pop_front();
			} else {
				break;
			}
		}
		//at this point, EventList contains all of the events to be merged, merge them
		MNCTEvent * NewMergedEvent = MergeEvents( &EventList );
		//now push this merged event onto the internal events deque
		m_Events.push_back( NewMergedEvent );
		++MergedEventCounter;
	}

	if( m_EventsBuf.size() == 0 ) return true; else return false;
}

bool MNCTModuleReceiverCOSI2014::CheckEventsBuf(void){

	int MergedEventCounter = 0;

	if( m_EventsBuf.size() > 0 ){
		while(m_EventsBuf.back()->GetCL() - m_EventsBuf.front()->GetCL() >= m_EventTimeWindow ){
			MNCTEvent * FirstEvent = m_EventsBuf.front(); m_EventsBuf.pop_front();
			deque<MNCTEvent*> EventList;
			EventList.push_back(FirstEvent);
			//now check if the next events are within the compton window
			while( m_EventsBuf.size() > 0 ){
				if( (m_EventsBuf[0]->GetCL() - FirstEvent->GetCL()) <= m_ComptonWindow ){
					EventList.push_back( m_EventsBuf.front() ); m_EventsBuf.pop_front();
				} else {
					break;
				}
			}
			//at this point, EventList contains all of the events to be merged, merge them
			MNCTEvent * NewMergedEvent = MergeEvents( &EventList );
			//now push this merged event onto the internal events deque
			NewMergedEvent->SetDataRead(true);
			m_Events.push_back( NewMergedEvent );
			++MergedEventCounter;
			if( m_EventsBuf.size() == 0 ) break;
		}

	}

	if( MergedEventCounter > 0 ) return true; else return false;
}

MNCTEvent * MNCTModuleReceiverCOSI2014::MergeEvents( deque<MNCTEvent*> * EventList ){

	//assert: there is at least one event in event list
	MNCTEvent * BaseEvent;

	//take the first event, and then merge hits from all other coincident
	//events into this base event
	BaseEvent = EventList->front(); EventList->pop_front();
	for( auto E: *EventList ){
		while( E->GetNStripHits() > 0){
			BaseEvent->AddStripHit( E->GetStripHit(0) );
			E->RemoveStripHit(0);
		}
		//now we should free the memory for the MNCTEvent that we just copied the 
		//strip hit from
		delete E;
	}

	return BaseEvent;

}

////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::AnalyzeEvent(MNCTEvent* Event) 
{
  // IsReady() ensured that the oldest event in the list has a reconstructed aspect
	MNCTEvent * NewEvent;
  
  if (m_Events.size() == 0) {
    cout<<"ERROR in MNCTModuleReceiverCOSI2014::AnalyzeEvent: No events"<<endl;
    cout<<"This function should have never been called when we have no events"<<endl;
    return false;
  }

  NewEvent = m_Events[0];
  m_Events.pop_front();

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
  Event->SetDataRead();

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
  
  // TODO: Clear all lists
  delete m_Receiver;
  m_Receiver = 0;
  
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
    m_DataSelectionMode = (MNCTModuleReceiverCOSI2014DataModes) LocalReceivingHostNameNode->GetValueAsInt();
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
  
  return Node;
}

int MNCTModuleReceiverCOSI2014::RawDataframe2Struct( vector<uint8_t> Buf, dataframe * DataOut)
{
		//return a dataframe struct
		//a subsequent funtion should dtake the returned dataframe and return a vector of MNCTEvents

	size_t x; //index for looping through Buf
	trigger TrigBuf[MAX_TRIGS];
	int tx;
	int EventCounter;
	int NumPayLoadBytes;
	int Tx, Ax;
	int NumADCTrigs, NumTimingTrigs;
	int NumTimingBytes[8];
	int j;
	event Event;
	uint8_t mask_or[10];
	uint8_t Masks[8] = {1,2,4,8,16,32,64,128};
	int NumEvents;	
	int Length;
	trigger NewTrig;


	if( DataOut == NULL ){
		cout<<"DataOut is NULL, returning -1..."<<endl;
		return -1;
	}

	if( Buf.size() != 1360 ){
		cout<<"dataframe must be 1360 bytes! returning -1..."<<endl;
		return -1;
	} else {
		Length = Buf.size();
	}

	//check that we get the first 0xae 0xe0 in the right place

	if( Buf[22] == 0xae && Buf[23] == 0xe0 ){
		//great
	} else {
		return -3;
	}

	//add the telem header stuff here!
	DataOut->PacketType = Buf[2];
	DataOut->UnixTime = ((uint32_t)Buf[3]<<16) | ((uint32_t)Buf[4]<<8) | ((uint32_t)Buf[5]);
	DataOut->PacketCounter = ((uint16_t)Buf[6]<<8) | ((uint16_t)Buf[7]);
	DataOut->CCId = Buf[10] & 0x0f;
	DataOut->ReportedNumEvents = Buf[11];
	DataOut->SysTime = ((uint64_t)Buf[17] << 40) | ((uint64_t)Buf[16] << 32) | ((uint64_t)Buf[15] << 24) | ((uint64_t)Buf[14] << 16) | ((uint64_t)Buf[13] << 8) | Buf[12];
	DataOut->SysTime = DataOut->SysTime & 0xffffffffffff;
	DataOut->LifetimeBits = (((((uint32_t)Buf[21] << 24) | ((uint32_t)Buf[20] << 16)) | ((uint32_t)Buf[19] << 8)) | ((uint32_t)Buf[18]));
	DataOut->RawOrCompton = "raw";
	DataOut->HasSysErr = false;

	//x = 12; //jump to index of first 0xAE
	x = 22; //now the input is a full 1360 packet
	NumEvents = Buf[11];
	Tx = 0; Ax = 0;
	EventCounter = 0;

	for(int z = 0; z < NumEvents; ++z){

		if( !((Buf[x] == 0xAE) && (Buf[x+1] == 0xE0)) ){ //check that we have 0xAE in the right place, if not, find it

			int k;
			k = x;
			while( k < (Length-1)){
				if( Buf[k] == 0xAE ){
					if( Buf[k + 1] == 0xE0 ){
						x = k;
						break;
					}
				} 
				++k;
			}

			if( k >= (Length - 1) ){
				return -100;
			} 

		}

		NumPayLoadBytes = 0;
		tx = 0;
		NumADCTrigs = 0;
		NumTimingTrigs = 0;

		//need to check here if there are enough bytes in the package to loop over the bitmasks
		if( (x + 32) >= Length ){
			//overran the end of the packet, return
			return -1;

		}


		//pre-compute the bitmask or's
		for(int i = 0; i < 10; ++i ){

			mask_or[i] = Buf[x+22+i] | Buf[x+12+i];

		}

		for( int j = 0; j < 8; ++j ){ //loop over boards

			NumTimingBytes[j] = 0;

			for( int i = 0; i < 10; ++i ){ //loop over channels

				if( mask_or[i] & Masks[j] ){

					//got something on this brd/chan

					TrigBuf[tx].Board = j;
					TrigBuf[tx].Channel = i;


					if( Buf[x+22+i] & Masks[j] ){
						//we have ADC on this brd/chan
						++NumADCTrigs;
						TrigBuf[tx].HasADC = true;
						NumPayLoadBytes += 2;
					} else{
						TrigBuf[tx].HasADC = false;
					}

					if( Buf[x+12+i] & Masks[j] ){
						//we have timing on this brd/chan
						++NumTimingTrigs; 
						TrigBuf[tx].HasTiming = true;
						NumTimingBytes[j] += 1;
						NumPayLoadBytes += 1;

					} else {
						TrigBuf[tx].HasTiming = false;
					}

					++tx;
					if( tx > MAX_TRIGS ){
						goto loop_exit; //legitimate usage of goto... to break out of nested for loops
					}


				}

			}

			if( NumPayLoadBytes & 1 ){
				//Num payload bytes is odd -> there are an odd number of timing bytes on this board -> add 1

				++NumPayLoadBytes;
			}

		}

loop_exit:

		if( (tx <= MAX_TRIGS) && ( tx > 0) ){

			Tx = x + 32; //jump to first timing byte

			if( Tx + NumPayLoadBytes >= Length ){
				//something went wrong, we overran the packet
				return -5;
			}

			j = TrigBuf[0].Board; //setup first board

			//setup Tx (timing byte index) and Ax (ADC byte index) for the first trigger
			if( NumTimingBytes[j] & 1 ){ //check if num timing bytes is odd is on this board
				Ax = Tx + NumTimingBytes[j] + 1;
			} else {
				Ax = Tx + NumTimingBytes[j];
			}


			for( int i = 0; i < tx; ++i ){ //loop over triggers

				if( TrigBuf[i].Board != j ){
					j = TrigBuf[i].Board;
					Tx = Ax;

					if( NumTimingBytes[j] & 1 ){
						Ax = Tx + NumTimingBytes[j] + 1;
					} else {
						Ax = Tx + NumTimingBytes[j];
					}

				} 

				if( TrigBuf[i].HasTiming ){
					TrigBuf[i].TimingByte = Buf[Tx];
					++Tx;
				}

				if( TrigBuf[i].HasADC ){
					TrigBuf[i].ADCBytes = (Buf[Ax+1] << 8) | Buf[Ax];
					TrigBuf[i].ADCBytes &= 0x1fff;
					Ax += 2;
				}
			}


			if( NumADCTrigs > 0 ){ //allocate mem for a new event if there are ADC trigs

				//fill in the event header info 
				Event.EventTime = (Buf[x+5]<<24)|(Buf[x+4]<<16)|(Buf[x+3]<<8)|(Buf[x+2]);
				Event.EventTime = Event.EventTime & 0x00000000ffffffff;//make sure we clear out the upper 4 bytes
				Event.ErrorBoardList = Buf[x+6];
				Event.ErrorInfo = Buf[x+7];
				Event.EventID = Buf[x+8];
				Event.TrigAndVetoInfo = Buf[x+9];
				Event.FTPattern = Buf[x+10];
				Event.LTPattern = Buf[x+11];
				Event.CCId = DataOut->CCId;

				if( Event.ErrorBoardList != 0){
					//we have a system error, set the flag in dataframe struct
					DataOut->HasSysErr = true;
				}

				//clear out triggers
				Event.Triggers.clear();

				int N;
				N = 0;
				//copy over triggers
				for( int i = 0; i < tx; ++i ){ //loop over all triggers...
					if( TrigBuf[i].HasADC == true ){ //... but only copy over triggers that have ADC
						NewTrig = TrigBuf[i];
						Event.Triggers.push_back(NewTrig);
						++N;
					}
				}

				Event.NumTriggers = N;
				DataOut->Events.push_back(Event);
				++EventCounter;
				++DataOut->NumEvents;

			}
			//done reading in the event. 

			x = Ax;
		} else {
			//too many trigs, or a no data event.  
			if( (x + 32) < Length ){
				x = x + 32;
			} else {
				//reached the end of the packet, return normally 
				return -100;
			}
		}
		//now check if we find the 0xAE in the right spot.
	}

	return 0;

}

bool MNCTModuleReceiverCOSI2014::ConvertToMNCTEvents( dataframe * DataIn, vector<MNCTEvent*> * CEvents)
{
	bool PosSide;
	MNCTEvent * NewEvent;
	MNCTStripHit * StripHit;
	bool RolloverOccurred, EndRollover, MiddleRollover;
	uint64_t Clk;

	CEvents->clear(); //

	//make sure we have some events
	if( DataIn->Events.size() == 0 ){
		return false;
	}

	//since the MNCTEvents are going to be pushed into a deque for the coincedence search, we want to call 
	//new and delete so that the pointers to these MNCTEvents will be valid until the MNCTEvent is popped
	//out of the deque.

	//check for rollovers in this dataframe.  need this info in order to properly shift bits 48..33 of the 
	//systime into the individual events times (which are only 32 bits)
	RolloverOccurred = false;
	if( (DataIn->SysTime & 0xffffffff) < DataIn->Events[0].EventTime ){
		//there was a rollover
		RolloverOccurred = true; EndRollover = false; MiddleRollover = false;
		if( DataIn->Events.back().EventTime < DataIn->Events.front().EventTime ){
			MiddleRollover = true;
		} else {
			EndRollover = true;
		}
	}

	//negative side -> DC -> boards 4-7 -> X
	//positive side -> AC -> boards 0-3 -> Y

	for( auto E: DataIn->Events ){
		NewEvent = new MNCTEvent();
		for( auto T: E.Triggers ){
			StripHit = new MNCTStripHit();
			StripHit->SetDetectorID(m_CCMap[DataIn->CCId]);
			//go from board channel, to side strip
			if( T.Board >= 4 && T.Board < 8 ) PosSide = true; else if( T.Board >= 0 && T.Board < 4 ) PosSide = false; else {cout<<"bad trigger board = "<<T.Board<<endl; delete StripHit; continue;} 
			StripHit->IsPositiveStrip(PosSide);
			if( T.Channel >= 0 && T.Channel < 10 ) StripHit->SetStripID(m_StripMap[T.Board][T.Channel]+1); else {cout<<"bad trigger channel = "<<T.Channel<<endl; delete StripHit; continue;}
			if( T.HasADC ) StripHit->SetADCUnits((double)T.ADCBytes);
			if( T.HasTiming ) StripHit->SetTiming((double) (T.TimingByte & 0x3f));
			NewEvent->AddStripHit( StripHit );
		}
		//now need to set parameters for the MNCTEvent
		NewEvent->SetID(E.EventID);
		NewEvent->SetFC(DataIn->PacketCounter);
		NewEvent->SetTI(DataIn->UnixTime);
		Clk = 0;
		if( RolloverOccurred ){
			if( MiddleRollover ){
				if( E.EventTime >= DataIn->Events.front().EventTime ){
					Clk = E.EventTime | ((DataIn->SysTime - 0x0000000100000000) & 0x0000ffff00000000);
				} else {
					Clk = E.EventTime | (DataIn->SysTime & 0x0000ffff00000000);
				}
			} else {
				//the rollover happened between the last event timestamp and the DataIn systime
				//NOTE the systime in the dataframe header is always latched AFTER the last event timestamp
				Clk = E.EventTime | ((DataIn->SysTime - 0x0000000100000000) & 0x0000ffff00000000);
			}
		} else {
			//no rollover, just shift in the upper two bytes of DataIn->SysTime
			Clk = E.EventTime | (DataIn->SysTime & 0x0000ffff00000000);
		}
		NewEvent->SetCL( Clk );

		//set the MJD only when there is aspect info

		CEvents->push_back(NewEvent);
	}

	return true;

}

void MNCTModuleReceiverCOSI2014::LoadCCMap(void){

	//takes you from CC Id to det ID
m_CCMap[0] = 0;
  m_CCMap[1] = 1;
  m_CCMap[2] = 2;
  m_CCMap[3] = 3;
  m_CCMap[4] = 4;
  m_CCMap[5] = 5;
  m_CCMap[6] = 6;
  m_CCMap[7] = 7;
  m_CCMap[8] = 8;
  m_CCMap[9] = 9;
  m_CCMap[10] = 10;
  m_CCMap[11] = 11;
/*
	m_CCMap[0] = 3;
	m_CCMap[1] = 0;
	m_CCMap[2] = 1;
	m_CCMap[3] = 2;
	m_CCMap[4] = 5;
	m_CCMap[5] = 6;
	m_CCMap[6] = 4;
	m_CCMap[7] = 7;
	m_CCMap[8] = 10;
	m_CCMap[9] = 11;
	m_CCMap[10] = 8;
	m_CCMap[11] = 9;
  */
}

void MNCTModuleReceiverCOSI2014::LoadStripMap(void){

	//takes you from board/chan to strip ID

	m_StripMap[0][0]=m_StripMap[4][0]=16;
	m_StripMap[0][1]=m_StripMap[4][1]=12;
	m_StripMap[0][2]=m_StripMap[4][2]= 8;
	m_StripMap[0][3]=m_StripMap[4][3]=37;  
	m_StripMap[0][4]=m_StripMap[4][4]= 4;
	m_StripMap[0][5]=m_StripMap[4][5]=10;
	m_StripMap[0][6]=m_StripMap[4][6]= 6;
	m_StripMap[0][7]=m_StripMap[4][7]= 2;
	m_StripMap[0][8]=m_StripMap[4][8]=14;
	m_StripMap[0][9]=m_StripMap[4][9]=17;  
	m_StripMap[1][0]=m_StripMap[5][0]=26;
	m_StripMap[1][1]=m_StripMap[5][1]= 0;
	m_StripMap[1][2]=m_StripMap[5][2]=34;
	m_StripMap[1][3]=m_StripMap[5][3]=36;
	m_StripMap[1][4]=m_StripMap[5][4]=32;
	m_StripMap[1][5]=m_StripMap[5][5]=30;
	m_StripMap[1][6]=m_StripMap[5][6]=28;
	m_StripMap[1][7]=m_StripMap[5][7]=24;
	m_StripMap[1][8]=m_StripMap[5][8]=18; 
	m_StripMap[1][9]=m_StripMap[5][9]=22;
	m_StripMap[2][0]=m_StripMap[6][0]=19;
	m_StripMap[2][1]=m_StripMap[6][1]=13;
	m_StripMap[2][2]=m_StripMap[6][2]= 9;
	m_StripMap[2][3]=m_StripMap[6][3]= 7;
	m_StripMap[2][4]=m_StripMap[6][4]= 5;
	m_StripMap[2][5]=m_StripMap[6][5]= 1;
	m_StripMap[2][6]=m_StripMap[6][6]= 3;
	m_StripMap[2][7]=m_StripMap[6][7]=38;
	m_StripMap[2][8]=m_StripMap[6][8]=11;
	m_StripMap[2][9]=m_StripMap[6][9]=15;
	m_StripMap[3][0]=m_StripMap[7][0]=23;
	m_StripMap[3][1]=m_StripMap[7][1]=35;
	m_StripMap[3][2]=m_StripMap[7][2]=31;
	m_StripMap[3][3]=m_StripMap[7][3]=39;
	m_StripMap[3][4]=m_StripMap[7][4]=33;
	m_StripMap[3][5]=m_StripMap[7][5]=27;
	m_StripMap[3][6]=m_StripMap[7][6]=29;
	m_StripMap[3][7]=m_StripMap[7][7]=25;
	m_StripMap[3][8]=m_StripMap[7][8]=21;
	m_StripMap[3][9]=m_StripMap[7][9]=20;

	return;
}

bool MNCTModuleReceiverCOSI2014::SortEventsBuf(void){

	std::sort(m_EventsBuf.begin(), m_EventsBuf.end(), MNCTEventTimeCompare);
	return true;

}

bool MNCTEventTimeCompare(MNCTEvent * E1, MNCTEvent * E2){

	//new events are added to the queue using push_back(), so older events are closer to [0]
	//older stuff comes before newer stuff, 

	/*
	if( E1->GetTI() == E2->GetTI() ){
		if( E1->GetCL() < E2->GetCL() ) return true; else return false;
	} else {
		if( E1->GetTI() < E2->GetTI() ) return true; else return false;
	}*/

	//sort based on 48 bit clock value
	if( E1->GetCL() < E2->GetCL() ) return true; else return false;


}






// MNCTModuleReceiverCOSI2014.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
