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
#include <algorithm>

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
  AddSucceedingModuleType(c_EnergyCalibration);
  AddSucceedingModuleType(c_CrosstalkCorrection);
  AddSucceedingModuleType(c_ChargeSharingCorrection);
  AddSucceedingModuleType(c_DepthCorrection);
  AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_Else);
  AddSucceedingModuleType(c_EventReconstruction);
  AddSucceedingModuleType(c_EventSaver);
  
  // Set the module name --- has to be unique
  m_Name.AppendInPlace("Data packet receiver, sorter, and aspect reconstructor for COSI 2014");
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagReceiverCOSI2014";  
  
  m_HasOptionsGUI = true;
  
  m_Receiver = 0;

  m_UseComptonDataframes = true;
  m_UseRawDataframes = true;
  m_NumRawDataframes = 0;
  m_NumComptonDataframes = 0;
  m_NumAspectPackets = 0;
  m_NumOtherPackets = 0;
  MAX_TRIGS = 80;
  LastTimestamps.resize(12);

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
      gSystem->Sleep(10);
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
    
    Wait = 1000;
    bool Restart = false;
    while (Handshaker->IsConnected() == true && Restart == false && Interrupt == false) {
      gSystem->Sleep(100); 
      cout<<"CONNECTION ESTABLISHED"<<endl;
      // Need a timeout here
      if (--Wait == 0) {
        cout<<"Connected but didn't receive anything -- timeout & restarting"<<endl;
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
  m_Receiver = new MTransceiverTcpIpBinary("Final receiver", m_LocalReceivingHostName, m_LocalReceivingPort);
  m_Receiver->SetVerbosity(3);
  m_Receiver->Connect(true, 10);
  m_Receiver->SetMaximumBufferSize(100000000);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleReceiverCOSI2014::Initialize()
{
  // Initialize the module 

  m_LocalReceivingHostName = "192.168.37.18";
  m_LocalReceivingPort = 12345;

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

	SyncWord.push_back(0xEB);
	SyncWord.push_back(0x90);

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
	m_Receiver->SyncedReceive(Received, SyncWord, 1);

	//where do I set whether m_Receiver will sync to eb90?
	//don't use SyncReceived(), do the syncing manually here

	if( Received.size() >= 10 ){
		if( (Received[0] == 0xeb) && (Received[1] == 0x90) ){
			//received data is synced, get the size
			Len = 0;
			Len = ((uint16_t)Received[8]<<8) | ((uint16_t)Received[9]);
			Type = Received[2];

			if( Received.size() >= Len ){
				if( Received.size() != Len ){
					cout<<"packet length mismatch, got "<<Received.size()<<"/"<<Len<<" bytes"<<endl;
				}
				vector<uint8_t> TruncReceived = Received;
				TruncReceived.erase(TruncReceived.begin(), TruncReceived.begin()+10);


				switch( Type ){
					case 0x00:
						//raw dataframe
						if( m_UseRawDataframes ){
							//convert dataframe to class
							//clear out first ten bytes 
							Dataframe = new dataframe();
							ParseErr = RawDataframe2Struct( Received, Dataframe );
							if( ParseErr >= 0 ){
								ConvertToMNCTEvents( Dataframe, &NewEvents );
								CCId = Dataframe->CCId;
							}
							delete Dataframe;
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
			} else {
				cout<<"not enough bytes, got "<<Received.size()<<"/"<<Len<<endl;
				cout<<std::hex<<(int)Received[0]<<" "<<(int)Received[1]<<std::dec<<endl;
			}
		} else {
			cout<<"out of sync [0]="<<std::hex<<Received[0]<<std::dec<<" [1]="<<Received[1]<<endl;
		}
	}

	for( auto E: NewEvents ){
		//push the events onto the deque
		//before we push the event into m_EventsBuf, check if we have a sync problem
		if( E->GetCL() > LastTimestamps[CCId] ) LastTimestamps[CCId] = E->GetCL(); else {
			//sync problem detected...
			cout<<"sync error on CC "<<CCId<<", det "<<m_CCMap[CCId]<<", flushing m_EventsBuf"<<endl;
			FlushEventsBuf();
			LastTimestamps[CCId] = E->GetCL();
		}

		m_EventsBuf.push_back(E);
		//printf("unix = %d  CL = %llx\n", E->GetTI(), E->GetCL());
	}

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

	return true;
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
					EventList.push_back( m_EventsBuf.front() ); m_Events.pop_front();
				} else {
					break;
				}
			}
			//at this point, EventList contains all of the events to be merged, merge them
			MNCTEvent * NewMergedEvent = MergeEvents( &EventList );
			//now push this merged event onto the internal events deque
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
		for( int j = 0; j < E->GetNStripHits(); ++j ){
			BaseEvent->AddStripHit( E->GetStripHit(j) );
			//need to remove the strip hit from the old event so that when we delete 
			//non-base event, the memory pointed to by strip hit is not also deleted
			E->RemoveStripHit(j);
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
					TrigBuf[i].ADCBytes = (Buf[Ax] << 8) | Buf[Ax + 1];
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
	CEvents->clear(); //
	bool RolloverOccurred, EndRollover, MiddleRollover;
	uint64_t Clk;

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
			if( T.Board >= 4 && T.Board < 8 ) PosSide = false; else if( T.Board >= 0 && T.Board < 4 ) PosSide = true; else {cout<<"bad trigger board = "<<T.Board<<endl; delete StripHit; continue;} 
			StripHit->IsPositiveStrip(PosSide);
			if( T.Channel >= 0 && T.Channel < 10 ) StripHit->SetStripID(m_StripMap[T.Board][T.Channel]); else {cout<<"bad trigger channel = "<<T.Channel<<endl; delete StripHit; continue;}
			if( T.HasADC ) StripHit->SetUncorrectedADCUnits((double)T.ADCBytes);
			if( T.HasTiming ) StripHit->SetTiming((double)T.TimingByte);
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

	m_CCMap[0] = 4;
	m_CCMap[1] = 1;
	m_CCMap[2] = 2;
	m_CCMap[3] = 3;
	m_CCMap[4] = 6;
	m_CCMap[5] = 7;
	m_CCMap[6] = 5;
	m_CCMap[7] = 8;
	m_CCMap[8] = 11;
	m_CCMap[9] = 12;
	m_CCMap[10] = 9;
	m_CCMap[11] = 8;

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
