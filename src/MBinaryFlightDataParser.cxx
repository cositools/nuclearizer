/*
 * MBinaryFlightDataParser.cxx
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
// MBinaryFlightDataParser
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBinaryFlightDataParser.h"

// Standard libs:
#include <algorithm>
#include <cstdio>
#include <time.h>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MStripHit.h"
#include "MFile.h"
#include "MModuleEventSaver.h"
#include "MQuaternion.h"

//Pipeline Tools:
#include "GCUSettingsParser.h"
#include "GCUHousekeepingParser.h"
#include "LivetimeParser.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBinaryFlightDataParser)
#endif





////////////////////////////////////////////////////////////////////////////////


bool MReadOutAssemblyTimeCompare(MReadOutAssembly* E1, MReadOutAssembly* E2);


////////////////////////////////////////////////////////////////////////////////


MBinaryFlightDataParser::MBinaryFlightDataParser() : TIRecord(1000)
{
	// Construct an instance of MBinaryFlightDataParser

	m_DataSelectionMode = MBinaryFlightDataParserDataModes::c_All;
	m_UseComptonDataframes = false;
	m_UseRawDataframes = true;
	m_NumRawDataframes = 0;
	m_NumComptonDataframes = 0;
	m_NumAspectPackets = 0;
	m_NumSettingsPackets = 0;
	m_NumGCUHkpPackets = 0;
	m_NumLivetimePackets = 0;
	m_NumOtherPackets = 0;
	MAX_TRIGS = 80;
	LastTimestamps.clear();
	LastTimestamps.resize(12, 0);
	dx = 0;
	m_EventTimeWindow = 60 * 10000000;
	m_ComptonWindow = 2;
	LoadStripMap();
	LoadCCMap();
	m_EventIDCounter = 0;
	m_LastCorrectedClk = 0xffffffffffffffff;
	m_UseGPSDSO = true; //simply sets whether or not we add these frames to the Aspect deque
	m_UseMagnetometer = true; //^^^^
	m_NumDSOReceived = 0;
	m_NumComptonBytes = 0;
	m_NumBytesReceived = 0;
	m_LostBytes = 0;
	m_IgnoreAspect = false;
	m_LastDSOUnixTime = 0xffffffff;
	m_LastAspectID = 0xffff;
	m_AspectReconstructor = nullptr;
	m_CoincidenceEnabled = true;
	m_HousekeepingFileName = "Housekeeping.hkp";
}


////////////////////////////////////////////////////////////////////////////////


MBinaryFlightDataParser::~MBinaryFlightDataParser()
{
	// Delete this instance of MBinaryFlightDataParser

	for (auto E: m_Events) {
		delete E;
	}
	m_Events.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MBinaryFlightDataParser::Initialize()
{
	// Initialize the module 

  m_NumRawDataframes = 0;
  m_NumComptonDataframes = 0;
  m_NumAspectPackets = 0;
  m_NumSettingsPackets = 0;
  m_NumGCUHkpPackets = 0;
  m_NumLivetimePackets = 0;
  m_NumOtherPackets = 0;


  //LastTimestamps.clear();
  //LastTimestamps.resize(12);
  dx = 0;

  m_EventIDCounter = 0;
  m_LastCorrectedClk = 0xffffffffffffffff;
  
  m_NumDSOReceived = 0;
  m_NumComptonBytes = 0;
  m_NumBytesReceived = 0;
  m_LostBytes = 0;
  
  m_LastDSOUnixTime = 0xffffffff;
  m_LastAspectID = 0xffff;

  for (auto E: m_Events) {
    delete E;
  }
  m_Events.clear();
  for (auto E: m_EventsBuf) {
    delete E;
  }
  m_EventsBuf.clear();
  
  m_SBuf.clear();
  
  m_LastDateTimeString = "";
  m_LastCorrectedClk = 0;
  m_LastLatitude = 0;
  m_LastLongitude = 0;
  m_LastAltitude = 0;
  m_LastGPSWeek = 0;
  m_LastAspectID = 0;
  LastComptonTimestamp = 0;
  m_NumComptonBytes = 0;
  m_NumRawDataBytes = 0;
  m_NumBytesReceived = 0;
 
  m_PreampTemps.reserve(24);
 
  // Load aspect reconstruction module
  delete m_AspectReconstructor;
  m_AspectReconstructor = new MAspectReconstruction();
  
  if(m_DataSelectionMode == MBinaryFlightDataParserDataModes::c_Compton){
	  m_EventTimeWindow = 0;
	  cout << "Receiver is using Compton mode -> Events might come in out of order over Openport! Enable coincidence search in Realta..." << endl;
  } else {
     m_EventTimeWindow = 60 * 10000000;
  }


 
	//AWL: this block of code prevents using the nuclearizer library outside of the nuclearizer gui when the event saver module isn't instantiated.  
  //AWL: changing this so that if it fails, we continue with the analysis and 
//Turns out the EventSaver modeule is always open, even if it's not selected in Nuclearizer, so this will make a .hkp file with the prefix with the name of the last saved file in Nuclearizer... for now.
  /*
  MSupervisor* S = MSupervisor::GetSupervisor();
  m_EventSaver = (MModuleEventSaver*) S->GetAvailableModuleByXmlTag("XmlTagEventSaver");
  if (m_EventSaver == nullptr) {
	cout<<"MBinaryFlightDataParser: Could not find file name for Housekeeping file"<<endl;
	return false;
  } else {   
	m_HkpOutFile = m_EventSaver->GetFileName();
    m_HkpOutFile.RemoveInPlace(m_HkpOutFile.Last('.'));
    m_HkpOutFile += ".hkp";
    MFile::ExpandFileName(m_HkpOutFile);
  	if (Housekeeping.is_open() == true) Housekeeping.close();
    Housekeeping.clear();
    Housekeeping.open(m_HkpOutFile, std::ofstream::out);
    if (Housekeeping.is_open() == false) {
      cout<<"MBinaryFlightDataParser: Unable to open housekeeping data file \""<<m_HkpOutFile<<"\""<<endl;
      return false;
    }
    
  }
  */
  
  // Handle the housekeeping file
  
  if (m_Housekeeping.is_open() == true) {
    m_Housekeeping.close();
    m_Housekeeping.clear();    
  }
  m_Housekeeping.open(m_HousekeepingFileName);
  if (m_Housekeeping.is_open() == false) {
    cout<<"Error: Unable to open housekeeping file for writing: "<<m_HousekeepingFileName<<endl;
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////

bool MReadOutAssemblyReverseSort(MReadOutAssembly* E1, MReadOutAssembly* E2) {

	//sort based on 48 bit clock value
	if( E1->GetCL() < E2->GetCL() ) return true; else return false;

}

////////////////////////////////////////////////////////////////////////////////


bool MBinaryFlightDataParser::ParseData(vector<uint8_t> Received) 
{
	uint8_t Type;
	vector <MReadOutAssembly*> NewEvents;
	vector<unsigned char> SyncWord;
	dataframe * Dataframe;
	int ParseErr;
	//unsigned int CCId = 0;

	struct GCUSettingsPacket* SettingsPacket;
 	struct GCUHousekeepingPacket* GCUHkpPacket;
	struct LivetimePacket CCLivetimePacket;
	uint8_t GCUUnixTimeMSB;
	double ShieldNumCounts;
	double ShieldTimeInterval;
	double ShieldCountRate;
	MAspect* LatestAspect;

	SyncWord.push_back(0xEB);
	SyncWord.push_back(0x90);
	m_NumBytesReceived += Received.size();
	if (g_Verbosity >= c_Info) cout<<"BinaryFlightDataParser: NumBytesReceived "<<m_NumBytesReceived<<endl;

	//apend the received data to m_SBuf
	//might want to reserve space here
	m_SBuf.insert( m_SBuf.end(), Received.begin(), Received.end() );
	//FindNextPacket handles all the resyncing etc...

	vector<uint8_t> NextPacket;
	while (FindNextPacket( NextPacket )) {
		Type = NextPacket[2] & 0x0f;
		
		uint64_t PacketKey = ((uint64_t)NextPacket[3] << 40) | 
			                  ((uint64_t)NextPacket[4] << 32) | 
									((uint64_t)NextPacket[5] << 24) | 
									((uint64_t)NextPacket[6] << 16) | 
									((uint64_t)NextPacket[7] << 8) | 
									Type;

		if(m_PacketRecord.count(PacketKey) == 1){
			//cout << "duplicate compton packet, ID:" << Dataframe->PacketCounter << " UNIXT:" << Dataframe->UnixTime << endl;
			continue;
		} else {
			m_PacketRecord[PacketKey] = (NextPacket[2] & 0x80) >> 7; //mapped value of zero -> normal data, or openport A. mapped value of one -> openport B
		}

		//trim the packet record
		while(m_PacketRecord.size() > 10000){
			m_PacketRecord.erase(m_PacketRecord.begin());
		}

		if (g_Verbosity >= c_Info) {
			//printf("FNP: %u - %lu, dx = %d, bufsize = %lu\n",Type, NextPacket.size(), dx, m_SBuf.size());
			cout<<"FNP: "<<hex<<Type<<" - "<<NextPacket.size()<<", dx = "<<dx<<", bufsize = "<<m_SBuf.size()<<endl;
		}


		switch( Type ){
			case 0x00:
				//raw dataframe
				if( m_DataSelectionMode == MBinaryFlightDataParserDataModes::c_Raw ){
					Dataframe = new dataframe();
					ParseErr = RawDataframe2Struct( NextPacket, Dataframe );
					if( ParseErr >= 0 ){
						ConvertToMReadOutAssemblys( Dataframe, &NewEvents );
						//CCId = Dataframe->CCId;
					} else {
						if (g_Verbosity >= c_Error) cout<<"BinaryFlightDataParser: ParseERR"<<endl;
					}
					//cout<<"made "<<NewEvents.size()<<" MReadOutAssemblys"<<endl;
					delete Dataframe;
					m_NumRawDataBytes += NextPacket.size();
					//cout<<"NumRawDataBytes "<<m_NumRawDataBytes<<endl;
					m_NumRawDataframes++;

				}
				break;
			case 0x01:
				//compton dataframe
				if( m_DataSelectionMode == MBinaryFlightDataParserDataModes::c_Compton ){
					Dataframe = new dataframe();
					if( ComptonDataframe2Struct( NextPacket, Dataframe ) ){
						ConvertToMReadOutAssemblys( Dataframe, &NewEvents );
					} else {
						if (g_Verbosity >= c_Error) cout<<"BinaryFlightDataParser: Parsing error"<<endl;
					}
					size_t NEvents = Dataframe->Events.size();
					
					/*
					printf("!@# ID:%u UNIXT:%u (%u,%f) <---> (%u,%f)\n",Dataframe->PacketCounter,
							                                              Dataframe->UnixTime,
																						 Dataframe->Events[0].EventID,
																						 ((double)NewEvents[0]->GetCL())*1E-7,
																						 Dataframe->Events[NEvents-1].EventID,
																						 ((double)NewEvents[NEvents-1]->GetCL())*1E-7);
																						 */

					delete Dataframe;
					m_NumComptonDataframes++;
					m_NumComptonBytes += NextPacket.size();

				}
				break;
			case 0x05:
				//aspect packet
				if (g_Verbosity >= c_Info) cout<<"got aspect packet!"<<endl;
				if (m_AspectMode != MBinaryFlightDataParserAspectModes::c_Neither) {
	
					ProcessAspect( NextPacket );

					//Print info into housekeeping file 
        	                      if (m_Housekeeping.is_open() == true) {
						if (m_AspectReconstructor->GetLastAspectInDeque() != 0) { 
							LatestAspect = m_AspectReconstructor->GetLastAspectInDeque();
							if (((m_AspectMode == MBinaryFlightDataParserAspectModes::c_GPS || m_AspectMode == MBinaryFlightDataParserAspectModes::c_Interpolate) && (LatestAspect->GetGPS_or_magnetometer() == 0)) || ((m_AspectMode == MBinaryFlightDataParserAspectModes::c_Magnetometer) && (LatestAspect->GetGPS_or_magnetometer() == 1))) {
								m_Housekeeping<<"ASP\nTI "<<LatestAspect->GetUTCTime()<<"\nMD "<<LatestAspect->GetGPS_or_magnetometer()<<"\nGX "<<LatestAspect->GetGalacticPointingXAxisLongitude()<<" "<<LatestAspect->GetGalacticPointingXAxisLatitude()<<"\nGZ "<<LatestAspect->GetGalacticPointingZAxisLongitude()<<" "<<LatestAspect->GetGalacticPointingZAxisLatitude()<<"\nCO "<<LatestAspect->GetLatitude()<<" "<<LatestAspect->GetLongitude()<<" "<<LatestAspect->GetAltitude()<<"\nGPS "<<LatestAspect->GetHeading()<<" "<<LatestAspect->GetPitch()<<" "<<LatestAspect->GetRoll()<<"\n\n";
							}
						}
					}
					//cout<<"GZ: "<<LatestAspect->GetGalacticPointingZAxisLongitude()<<" "<<LatestAspect->GetGalacticPointingZAxisLatitude()<<endl;
				
				}
				m_NumAspectPackets++;
				break;
			case 0x06:
				//livetime packet
				if (g_Verbosity >= c_Info) cout<<"got livetime packet!"<<endl;
				//wait to get a gcu_hkp packet to use the unix time most sig bit
				if (m_NumGCUHkpPackets > 0) {
					ParseLivetime(&CCLivetimePacket,&NextPacket[0]);
					//Print CC livetime info into housekeeping file
					if (m_Housekeeping.is_open() == true) {
						m_Housekeeping<<"LT\nTI "<<((GCUUnixTimeMSB << 24) | CCLivetimePacket.UnixTime)<<"\nID "<<CCLivetimePacket.PacketCounter<<"\nDU 1";;
						for (int i = 0; i < 12; ++i) {
							if (CCLivetimePacket.CCHasLivetime[i] == 1) {
								m_Housekeeping<<"\nCC"<<i<<" "<<(CCLivetimePacket.TotalLivetime[i])/3051.;
							} else {
								m_Housekeeping<<"\nCC"<<i<<" -1";
							}
						}
						m_Housekeeping<<"\n\n";
					}
				}
				m_NumLivetimePackets++;
				break;
			case 0x07:
				//gcu hkp packet

				if (g_Verbosity >= c_Info) cout<<"got GCU housekeeping packet!"<<endl;
				GCUHkpPacket = ParseGCUHousekeepingPacket(&NextPacket[0]);
				GCUUnixTimeMSB = GCUHkpPacket->UnixTimeMSB;

				//Calculate shield rate
				ShieldNumCounts = static_cast <double> (GCUHkpPacket->NumCounts[0]);
				ShieldTimeInterval = (static_cast <double> (GCUHkpPacket->TimeInterval[0]))*1e-7;
				ShieldCountRate = ShieldNumCounts/ShieldTimeInterval;

				//Print info into housekeeping file
				if (m_Housekeeping.is_open() == true) {
					m_Housekeeping<<"HKP\nTI "<<((GCUUnixTimeMSB << 24) | GCUHkpPacket->UnixTime)<<"\nID "<<GCUHkpPacket->PacketCounter<<"\nDU 5"<<"\nSR "<<ShieldCountRate<<"\n\n";
				}
				m_NumGCUHkpPackets++;
				break;
			case 0x0b:
				//preamp temperatures
				if (g_Verbosity >= c_Info) cout<<"got settings packet!"<<endl;
				SettingsPacket = ParseGCUSettingsPacket(&NextPacket[0]);
				//Order of PreampTemps are defined as Det0 DC, Det0 AC, Det1 DC, Det1, AC...etc
				m_PreampTemps[0] = SettingsPacket->RpiTemp_Brd2_Ch0;
				m_PreampTemps[1] = SettingsPacket->RpiTemp_Brd2_Ch3;
				m_PreampTemps[2] = SettingsPacket->RpiTemp_Brd0_Ch6;
				m_PreampTemps[3] = SettingsPacket->RpiTemp_Brd2_Ch2;
				m_PreampTemps[4] = SettingsPacket->RpiTemp_Brd0_Ch7;
				m_PreampTemps[5] = SettingsPacket->RpiTemp_Brd2_Ch1;
				m_PreampTemps[6] = SettingsPacket->RpiTemp_Brd1_Ch6;
				m_PreampTemps[7] = SettingsPacket->RpiTemp_Brd2_Ch4;
				m_PreampTemps[8] = SettingsPacket->RpiTemp_Brd1_Ch7;
				m_PreampTemps[9] = SettingsPacket->RpiTemp_Brd2_Ch5;
				m_PreampTemps[10] = SettingsPacket->RpiTemp_Brd2_Ch7;
				m_PreampTemps[11] = SettingsPacket->RpiTemp_Brd2_Ch6;
				m_PreampTemps[12] = SettingsPacket->RpiTemp_Brd1_Ch5;
				m_PreampTemps[13] = SettingsPacket->RpiTemp_Brd1_Ch2;
				m_PreampTemps[14] = SettingsPacket->RpiTemp_Brd1_Ch4;
				m_PreampTemps[15] = SettingsPacket->RpiTemp_Brd1_Ch1;
				m_PreampTemps[16] = SettingsPacket->RpiTemp_Brd1_Ch3;
				m_PreampTemps[17] = SettingsPacket->RpiTemp_Brd1_Ch0;
				m_PreampTemps[18] = SettingsPacket->RpiTemp_Brd0_Ch3;
				m_PreampTemps[19] = SettingsPacket->RpiTemp_Brd0_Ch0;
				m_PreampTemps[20] = SettingsPacket->RpiTemp_Brd0_Ch4;
				m_PreampTemps[21] = SettingsPacket->RpiTemp_Brd0_Ch1;
				m_PreampTemps[22] = SettingsPacket->RpiTemp_Brd0_Ch5;
				m_PreampTemps[23] = SettingsPacket->RpiTemp_Brd0_Ch2;
				m_NumSettingsPackets++;
				break;
			default:
				//don't care
				m_NumOtherPackets++;
		}
		if( NewEvents.size() > 0 ){
			for( auto E: NewEvents ){
				/*
				int CCId = E->GetStripHit(0)>GetDetectorID(); //this line might be an issue since events from compton packets can have SHs from more than one detector
				uint64_t Lower;
				if(m_EventTimeWindow > LastTimestamps[CCId]){
					Lower = LastTimestamps[CCId] >> 1;
				} else {
					Lower = LastTimestamps[CCId] - m_EventTimeWindow;
				}
				bool DeleteEvent = false;
				if(E->GetCL() < Lower){ //timestamp went back in time too far
					cout << CCId << " past: current CL = " << E->GetCL() <<", LastCL = " << LastTimestamps[CCId];
					if(m_EventsBuf.size() > 0) cout << ", frontCL = " << m_EventsBuf.front()->GetCL() << ", backCL = " << m_EventsBuf.back()->GetCL() << endl; else cout << endl;
					while(m_EventsBuf.size() > 0){
						MReadOutAssembly* E = m_EventsBuf[0]; m_EventsBuf.pop_front();
						delete E;
					}
					deque<MReadOutAssembly*>::iterator I = lower_bound(m_EventsBuf.begin(), m_EventsBuf.end(), E, MReadOutAssemblyReverseSort);
					m_EventsBuf.insert(I, E);
				} else if(E->GetCL() > (LastTimestamps[CCId] + (m_EventTimeWindow << 1))){ //timestamp is too far in the future
					cout << CCId << " future: current CL = " << E->GetCL() <<", LastCL = " << LastTimestamps[CCId];
					if(m_EventsBuf.size() > 0) cout << ", frontCL = " << m_EventsBuf.front()->GetCL() << ", backCL = " << m_EventsBuf.back()->GetCL() << endl; else cout << endl;
					DeleteEvent = true;
				} else {//timestamp is OK, insert the event into m_EventsBuf
					deque<MReadOutAssembly*>::iterator I = lower_bound(m_EventsBuf.begin(), m_EventsBuf.end(), E, MReadOutAssemblyReverseSort);
					m_EventsBuf.insert(I, E);
				}
				LastTimestamps[CCId] = E->GetCL();
				if(DeleteEvent){
					delete E;
				}*/

				/*
				if(m_EventsBuf.size() > 0){
					if(m_EventsBuf.front()->GetCL() >= m_EventTimeWindow){
						if(E->GetCL() < (m_EventsBuf.front()->GetCL() - m_EventTimeWindow)){ //event time jumped back too far
							cout << "event time back-skip: this CL = " << E->GetCL() << ", front CL = " << m_EventsBuf.front()->GetCL() << ", back CL = " << m_EventsBuf.back()->GetCL() << endl;
							while(m_EventsBuf.size() > 0){
								MReadOutAssembly* Ev = m_EventsBuf.front(); m_EventsBuf.pop_front();
								delete Ev;
							}
							deque<MReadOutAssembly*>::iterator I = lower_bound(m_EventsBuf.begin(), m_EventsBuf.end(), E, MReadOutAssemblyReverseSort);
							m_EventsBuf.insert(I, E);
						} else if(E->GetCL() > (m_EventsBuf.back()->GetCL() + (4*m_EventTimeWindow))){ //event time jumped forward too far
							cout << "event time forward-skip: this CL = " << E->GetCL() << ", front CL = " << m_EventsBuf.front()->GetCL() << ", back CL = " << m_EventsBuf.back()->GetCL() << endl;
							delete E;
						} else {
							deque<MReadOutAssembly*>::iterator I = lower_bound(m_EventsBuf.begin(), m_EventsBuf.end(), E, MReadOutAssemblyReverseSort);
							m_EventsBuf.insert(I, E);
						}
					} else {
							deque<MReadOutAssembly*>::iterator I = lower_bound(m_EventsBuf.begin(), m_EventsBuf.end(), E, MReadOutAssemblyReverseSort);
							m_EventsBuf.insert(I, E);
					}
				} else {
					m_EventsBuf.push_back(E);
				}
				*/
				if(m_EventsBuf.size() > 0){
					if(E->GetCL() < (m_EventsBuf.front()->GetCL() >> 1)){ //event time jumped back too far
						cout << "event time back-skip: this CL = " << E->GetCL() << ", front CL = " << m_EventsBuf.front()->GetCL() << ", back CL = " << m_EventsBuf.back()->GetCL() << endl;
						while(m_EventsBuf.size() > 0){
							MReadOutAssembly* Ev = m_EventsBuf.front(); m_EventsBuf.pop_front();
							delete Ev;
						}
						m_EventsBuf.push_back(E);
					} else {
						deque<MReadOutAssembly*>::iterator I = lower_bound(m_EventsBuf.begin(), m_EventsBuf.end(), E, MReadOutAssemblyReverseSort);
						m_EventsBuf.insert(I, E);
					}
				} else {
					m_EventsBuf.push_back(E);
				}

			}
			NewEvents.clear();
			if( m_UseRawDataframes ){
				if (g_Verbosity >= c_Info) cout<<"BinaryFlightDataParser: T ::: ";;
				for( auto E: LastTimestamps ){
					if (g_Verbosity >= c_Info) cout<<std::hex<<E<<" ";
				}
				if (g_Verbosity >= c_Info) cout<<std::dec<<endl;
			} else if( m_UseComptonDataframes ){
				if (g_Verbosity >= c_Info) cout<<"BinaryFlightDataParser: T_compton ::: "<<LastComptonTimestamp<<endl;
			}
		}
	}

	CheckEventsBuf();

	if( m_AspectMode != MBinaryFlightDataParserAspectModes::c_Neither){
		for( auto E: m_Events ){
			if( E->GetAspect() == 0 ){
				int gps_or_mag;
				if( m_AspectMode == MBinaryFlightDataParserAspectModes::c_GPS ){
					gps_or_mag = 0;
				} else if ( m_AspectMode == MBinaryFlightDataParserAspectModes::c_Magnetometer) {
					gps_or_mag = 1;
				} else { //Interpolation
					gps_or_mag = 2; 
				}
				MAspect* A = m_AspectReconstructor->GetAspect(E->GetTime(), gps_or_mag);
				if( A != 0 ){
					E->SetAspect(new MAspect(*A));
				}
			}
		}
	}



	//Preamp Temp Allocation
	int striphits;
	int det;
	int side;
	double temp;
	for( auto E: m_Events ){
		striphits = E->GetNStripHits();
		for(int s = 0; s < striphits; s++) {
			det = E->GetStripHit(s)->GetDetectorID();
			side = E->GetStripHit(s)->IsLowVoltageStrip() == true;
			temp = (m_PreampTemps[det*2 + side]*0.0005/0.5)*2.471*100 - 273.0;
			E->GetStripHit(s)->SetPreampTemp(temp);
		}
	}	

	if (m_Events.size() > 0) {
		//if (m_IgnoreAspect == true) {
		if (m_AspectMode == MBinaryFlightDataParserAspectModes::c_Neither) {
			return true;
		} else {
			if (m_Events[0]->GetAspect() != 0) {
				return true;
			} else {
				return false;
			}
		}
	} else {
		return false;
	}

}

////////////////////////////////////////////////////////////////////////////////


bool MBinaryFlightDataParser::FindNextPacket(vector<uint8_t>& NextPacket , unsigned int * idx){

	//return true if a complete packet was found, return packet in NextPacket
	//return false if a complete packet was not found.  also copy the leftover bytes
	//back to the beginning

	//idx is the value of dx that points to the beginning of the packet in m_SBuf

	//assert: search buf is either synced or empty

	uint16_t Len;
	//bool FoundPacket;
	//FoundPacket = false;
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
	// AZ: Found a case with Len == 0 which screwed up everything...
	// Skip ahead beyond syncword and resync
	if (Len == 0) {
		dx += 2; 
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

	//FoundPacket = true;

	//we have a complete packet, extract it
	NextPacket.assign( m_SBuf.begin() + dx, m_SBuf.begin() + dx + Len);
	//store the location of beginning of this packet
	if( idx != NULL ){
		*idx = dx;
	}
	//increment the index...we should be pointing at the next 0xeb 
	dx += Len;

	if( dx == m_SBuf.size() ){
		//no leftover bytes, just clear the buffer
		dx = 0;
		m_SBuf.clear();
	}

	if( m_SBuf.size() > 10000000 ){
		//clear out buffer
		cout<<"lost !!! dx = "<<dx<<" size = "<<m_SBuf.size()<<endl;
		m_LostBytes += m_SBuf.size() - dx;
		dx = 0;
		m_SBuf.clear();
	}

	return true;

}


////////////////////////////////////////////////////////////////////////////////


bool MBinaryFlightDataParser::ResyncSBuf(void){


	//this method makes sure that either m_SBuf[dx] = 0xeb and m_SBuf[dx+1] = 0x90
	//or that the buffer is empty and dx = 0;

	//start from the +1th element when searching for 0xeb, or check if the current
	//Buf[dx] is eb, and if it is then + 1

	if (g_Verbosity >= c_Info) cout<<"BinaryFlightDataParser: Resyncing input stream! Buffer size: "<<m_SBuf.size()<<", position in buffer: "<<dx<<endl;

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
	for(unsigned i = dx; i < m_SBuf.size()-1; ++i) { // m_SBuf is not allowed to be 0 so we are OK
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


////////////////////////////////////////////////////////////////////////////////


bool MBinaryFlightDataParser::FlushEventsBuf(void){

	//int MergedEventCounter = 0;

	//don't check m_EventTimeWindow, we are flushing the buffer
	while( m_EventsBuf.size() > 0){
		MReadOutAssembly * FirstEvent = m_EventsBuf.front(); m_EventsBuf.pop_front();
		deque<MReadOutAssembly*> EventList;
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
		MReadOutAssembly * NewMergedEvent = MergeEvents( &EventList );
		//now push this merged event onto the internal events deque
		//set the ID of the event and increment the ID counter
		NewMergedEvent->SetID( ++m_EventIDCounter );
		m_Events.push_back( NewMergedEvent );
	}

	if( m_EventsBuf.size() == 0 ) return true; else return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MBinaryFlightDataParser::CheckEventsBuf(void){

	int MergedEventCounter = 0;
	unsigned long long Window;

	//in file mode we need a way to clear out the queue, do this by setting the search buffer time to zero
	if( m_IsDone ){
		Window = 0;
	} else {
		Window = m_EventTimeWindow;
	}

	if( m_EventsBuf.size() > 0 ){
		if (m_EventsBuf.back()->GetCL() - m_EventsBuf.front()->GetCL() < 100000 && 
				m_EventsBuf.size() > 500) {
			cout<<"Something is strange: I have more than 500 events and all are within the time window of 10 milli-seconds"<<endl;
		}    
	}

	//pop good events
	while(m_EventsBuf.size() > 0){
		if(m_EventsBuf.back()->GetCL() - m_EventsBuf.front()->GetCL() >= Window ){
			MReadOutAssembly * FirstEvent = m_EventsBuf.front(); m_EventsBuf.pop_front();
			deque<MReadOutAssembly*> EventList;
			EventList.push_back(FirstEvent);

			if( m_CoincidenceEnabled ){
				//now check if the next events are within the compton window
				while( m_EventsBuf.size() > 0 ){
					if( (m_EventsBuf[0]->GetCL() - FirstEvent->GetCL()) <= m_ComptonWindow ){
						EventList.push_back( m_EventsBuf.front() ); m_EventsBuf.pop_front();
					} else {
						break;
					}
				}
			}
			//at this point, EventList contains all of the events to be merged, merge them
			MReadOutAssembly * NewMergedEvent = MergeEvents( &EventList );
			//now push this merged event onto the internal events deque
			NewMergedEvent->SetID( ++m_EventIDCounter );
			m_Events.push_back(NewMergedEvent);
			//if( m_EventsBuf.size() == 0 ) break;
		} else {
			break;
		}
	}

	if( MergedEventCounter > 0 ) return true; else return false;
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly * MBinaryFlightDataParser::MergeEvents( deque<MReadOutAssembly*> * EventList ){

	//assert: there is at least one event in event list
	MReadOutAssembly * BaseEvent;

	//take the first event, and then merge hits from all other coincident
	//events into this base event
	BaseEvent = EventList->front(); EventList->pop_front();
	for( auto E: *EventList ){
		while( E->GetNStripHits() > 0){
			BaseEvent->AddStripHit( E->GetStripHit(0) );
			E->RemoveStripHit(0);
		}
		while( E->GetNStripHitsTOnly() > 0){
			BaseEvent->AddStripHitTOnly( E->GetStripHitTOnly(0) );
			E->RemoveStripHitTOnly(0);
		}
		//now we should free the memory for the MReadOutAssembly that we just copied the 
		//strip hit from
		delete E;
	}

	return BaseEvent;

}


////////////////////////////////////////////////////////////////////////////////


void MBinaryFlightDataParser::Finalize()
{
	// Close the tranceiver 
  
  while (m_EventsBuf.begin() != m_EventsBuf.end()) {
    delete m_EventsBuf.front();
    m_EventsBuf.pop_front();
  }
	while (m_Events.begin() != m_Events.end()) {
		delete m_Events.front();
		m_Events.pop_front();
	}

	m_Housekeeping.close();
	cout<<"HOUSEKEEPING FILE CLOSED"<<endl;
	return;
}


////////////////////////////////////////////////////////////////////////////////


int MBinaryFlightDataParser::RawDataframe2Struct( vector<uint8_t> Buf, dataframe * DataOut)
{
	//return a dataframe struct
	//a subsequent funtion should dtake the returned dataframe and return a vector of MReadOutAssemblys

	size_t x; //index for looping through Buf
	trigger TrigBuf[MAX_TRIGS];
	unsigned int tx;
	int EventCounter;
	int NumPayLoadBytes;
	unsigned int Tx, Ax;
	int NumADCTrigs, NumTimingTrigs;
	int NumTimingBytes[8];
	unsigned int j;
	event Event;
	uint8_t mask_or[10];
	uint8_t Masks[8] = {1,2,4,8,16,32,64,128};
	int NumEvents;	
	unsigned int Length;
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

			unsigned int k;
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
						//might want to copy below code regarding NumPayloadBytes since it will get skipped on the goto
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


			for(unsigned int i = 0; i < tx; ++i ){ //loop over triggers

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
					//TrigBuf[i].ADCBytes &= 0x1fff;
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
				for(unsigned int i = 0; i < tx; ++i ){ //loop over all triggers...
					//at some point it would be cool to look into timing only triggers for better positioning
					//in that case, don't throw out the timing only triggers here
					//then below, in ConvertToMReadOutAssemblys, put the timing only strip hits in a separate buffer so that they don't interfere
					//with all of the mainstream analysis.
					NewTrig = TrigBuf[i];
					NewTrig.CCId = DataOut->CCId;
					Event.Triggers.push_back(NewTrig);
					++N;

					/*
					if( TrigBuf[i].HasADC == true ){ //... but only copy over triggers that have ADC
						NewTrig = TrigBuf[i];
						NewTrig.CCId = DataOut->CCId;
						Event.Triggers.push_back(NewTrig);
						++N;
					}
					*/
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



///////////////////////////////////////////////////////////////////////


bool MBinaryFlightDataParser::ConvertToMReadOutAssemblys( dataframe * DataIn, vector<MReadOutAssembly*> * CEvents)
{
	bool PosSide;
	MReadOutAssembly * NewEvent;
	MStripHit * StripHit;
	bool RolloverOccurred;
	//bool EndRollover  // az: not used, thus commented out
	bool MiddleRollover;
	uint64_t Clk;
	static MTime LastTime = 0;

	CEvents->clear(); //

	//make sure we have some events
	if( DataIn->Events.size() == 0 ){
		return false;
	}

	//since the MReadOutAssemblys are going to be pushed into a deque for the coincedence search, we want to call 
	//new and delete so that the pointers to these MReadOutAssemblys will be valid until the MReadOutAssembly is popped
	//out of the deque.

	//check for rollovers in this dataframe.  need this info in order to properly shift bits 48..33 of the 
	//systime into the individual events times (which are only 32 bits)
	RolloverOccurred = false;
	if( (DataIn->SysTime & 0xffffffff) < DataIn->Events[0].EventTime ){
		//there was a rollover
		RolloverOccurred = true; 
		//EndRollover = false; 
		MiddleRollover = false;
		if( DataIn->Events.back().EventTime < DataIn->Events.front().EventTime ){
			MiddleRollover = true;
		} else {
			// EndRollover = true; // az: not used, thus commented out
		}
	}

	//negative side -> DC -> boards 4-7 -> X
	//positive side -> AC -> boards 0-3 -> Y

	for( auto E: DataIn->Events ){
		NewEvent = new MReadOutAssembly();
		for( auto T: E.Triggers ){
			StripHit = new MStripHit();
			StripHit->SetDetectorID(m_CCMap[T.CCId]);
			//go from board channel, to side strip
			if( T.Board >= 4 && T.Board < 8 ) PosSide = false; else if( T.Board >= 0 && T.Board < 4 ) PosSide = true; else {cout<<"bad trigger board = "<<T.Board<<endl; delete StripHit; continue;} 
			StripHit->IsLowVoltageStrip(PosSide);
			if( T.Channel >= 0 && T.Channel < 10 ) StripHit->SetStripID(m_StripMap[T.Board][T.Channel]+1); else {cout<<"bad trigger channel = "<<T.Channel<<endl; delete StripHit; continue;}
			if( T.HasADC ) StripHit->SetADCUnits((double)((uint16_t)T.ADCBytes & 0x1fff)); else StripHit->SetADCUnits(0.0);
			if( T.HasTiming ) {
				unsigned int Val = 0;
				//bit 7 is 1 if first edge is a falling edge
				if( T.TimingByte & 0x80 ){
					//IS falling edge
					Val = ((T.TimingByte & 0x3f) << 1) + ((T.TimingByte & 0x1) ^ ((T.TimingByte >> 6) & 0x1));
				} else {
					//IS rising edge
					Val = ((T.TimingByte & 0x3f) << 1) - ((T.TimingByte & 0x1) ^ ((T.TimingByte >> 6) & 0x1));
				}
				StripHit->SetTiming(Val * 5.0); //timing is set in ns with 5 ns resolution
			} else {
				StripHit->SetTiming(0.0);
			}
			if( T.HasADC ){
				NewEvent->AddStripHit( StripHit );
			} else {
				NewEvent->AddStripHitTOnly( StripHit );
			}
		}
		//now need to set parameters for the MReadOutAssembly
		NewEvent->SetID(E.EventID);
		NewEvent->SetFC(DataIn->PacketCounter);
		NewEvent->SetTI(DataIn->UnixTime);
		Clk = 0;
		if( RolloverOccurred ){
			if( MiddleRollover ){
				//cout << "middle rollover for CC " << DataIn->CCId << endl;
				if( E.EventTime >= DataIn->Events.front().EventTime ){
					Clk = E.EventTime | ((DataIn->SysTime - 0x0000000100000000) & 0x0000ffff00000000);
				} else {
					Clk = E.EventTime | (DataIn->SysTime & 0x0000ffff00000000);
				}
			} else {
				//cout << "end rollover for CC " << DataIn->CCId << endl;
				//the rollover happened between the last event timestamp and the DataIn systime
				//NOTE the systime in the dataframe header is always latched AFTER the last event timestamp
				Clk = E.EventTime | ((DataIn->SysTime - 0x0000000100000000) & 0x0000ffff00000000);
			}
		} else {
			//no rollover, just shift in the upper two bytes of DataIn->SysTime
			Clk = E.EventTime | (DataIn->SysTime & 0x0000ffff00000000);
		}
		NewEvent->SetCL( Clk );
		uint64_t ClkModulo = Clk % 10000000;
		uint64_t int_ClkSeconds = Clk - ClkModulo;
		double ClkSeconds = (double) int_ClkSeconds/10000000.;
		double ClkNanoseconds = (double) ClkModulo*100.0;
		MTime NewTime = MTime();
		NewTime.Set( ClkSeconds, ClkNanoseconds );
		NewEvent->SetTime( NewTime );

		if( E.TrigAndVetoInfo & 0x70 ){
			NewEvent->SetVeto();
		}

		if( E.TrigAndVetoInfo & 0x10 ){
			//had a guard ring 0 veto
			NewEvent->SetGR0Veto(true);
		}

		if( E.TrigAndVetoInfo & 0x20 ){
			NewEvent->SetGR1Veto(true);
		}

		if( E.TrigAndVetoInfo & 0x40 ){
			NewEvent->SetShieldVeto(true);
		}

		CEvents->push_back(NewEvent);

		//set the MJD only when there is aspect info

	}

	return true;

}

//////////////////////////////////////////////////////////////////

void MBinaryFlightDataParser::LoadCCMap(void){

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

//////////////////////////////////////////////////////////////////////

void MBinaryFlightDataParser::LoadStripMap(void){

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

///////////////////////////////////////////////////////////////////

bool MBinaryFlightDataParser::SortEventsBuf(void){

	std::sort(m_EventsBuf.begin(), m_EventsBuf.end(), MReadOutAssemblyTimeCompare);
	return true;

}

///////////////////////////////////////////////////////////////////

bool MReadOutAssemblyTimeCompare(MReadOutAssembly * E1, MReadOutAssembly * E2){

	//sort based on 48 bit clock value
	if( E1->GetCL() < E2->GetCL() ) return true; else return false;

}

///////////////////////////////////////////////////////////////////

bool MBinaryFlightDataParser::ProcessAspect( vector<uint8_t> & NextPacket ){

	//look for '$'

	int Len = NextPacket.size();
	int wx = 0; // skip to first byte
	bool NotEnoughBytes = false;
	int i;
	//uint32_t LastClkSample;
	uint64_t UpperClkBytes;

	int DSOLen = 84; //length of the DSO message including the last 5 bytes for the clock
	int MagLen = 24; //length of the magnetometer message including my "$M" header

	uint16_t AspectID = 0;
	AspectID |= ((uint16_t) NextPacket[6]) << 8;
	AspectID |= ((uint16_t) NextPacket[7]);

	int UnixBytes = 0;
	UnixBytes |= ((int) NextPacket[3] << 16);
	UnixBytes |= ((int) NextPacket[4] << 8);
	UnixBytes |= ((int) NextPacket[5]);

	/* this was for COSI 14

	if( UnixBytes > 0x0094C5B1 ){
		UnixBytes |= 0x54000000;
	} else {
		UnixBytes |= 0x55000000;
	}

	*/

	//COSI 16... need to replace this with GPS time eventually
	if( UnixBytes > 0x00f7494c ){
		UnixBytes |= 0x56000000;
	} else {
		UnixBytes |= 0x57000000;
	}

	time_t UnixTime = (time_t) UnixBytes;
	UpperClkBytes = 0; UpperClkBytes = ((uint64_t) NextPacket[10] << 40) | ((uint64_t) NextPacket[11] << 32);
	wx = 12;
	while( wx < Len ){

		if( NextPacket[wx] == '$' ){
			//check that we have enough bytes to determine the message type
			if( (wx + 10) <= Len ){
				//determine the type:
				string Header ;
				for( i = 0; i < 10; ++i ) Header += (char) NextPacket[wx + i];
				////////////////////// DSO Msg //////////////////////////////////
				if( Header.find("$PASHR,DSO") == 0 ){
					//check that we have enough bytes in the buffer for this
					if( (wx + DSOLen) <= Len ){
						vector<uint8_t> DSOMsg;
						DSOMsg.assign( NextPacket.begin() + wx, NextPacket.begin() + wx + DSOLen );
						MAspectPacket DSOPacket;
						DecodeDSO( DSOMsg, DSOPacket );//transfer info from DSO msg into an MAspectPacket
						DSOPacket.PPSClk |= UpperClkBytes;
						long int SecondsSinceGPSEpoch = (UnixTime - 315964800) + 17; //17 seconds is number of leapseconds introduced since 1980 GPS epoch
						long int GPSWeek = SecondsSinceGPSEpoch/(60*60*24*7);
						int64_t GPSms = ((uint64_t)GPSWeek*(7*24*60*60*1000)) + DSOPacket.GPSMilliseconds; //absolute GPS time down to the millisecond 
						MTime GPSTime((long int)(GPSms/1000),(long int)((GPSms % 1000)*10000));
						MTime UTCTime((unsigned int)(GPSTime.GetAsSystemSeconds() -17 + 315964800), GPSTime.GetNanoSeconds());
						long int UTCSec = UTCTime.GetAsSystemSeconds();
						int64_t PPS = DSOPacket.PPSClk;
						bool FoundPPS = TIRecord.AddCorrect(UTCSec,PPS);
						if(FoundPPS){
							DSOPacket.PPSClk = PPS;
							DSOPacket.UnixTime = UTCSec;
							DSOPacket.GPSms = GPSms;
							m_LastDSOPacket = DSOPacket; 
							m_LastDSOUnixTime = UTCSec;
							m_LastAspectID = AspectID;
							if( m_UseGPSDSO ){
								m_AspectReconstructor->AddAspectFrame( DSOPacket );
								m_NumDSOReceived++;
							}
						}
						if(m_NumDSOReceived < 20){
							cout << "GPSms = " << GPSms << endl;
						}

						wx += DSOLen;
					} else {
						NotEnoughBytes = true;
					}

					////////////////////// Mag Msg //////////////////////////////////
				} else if( Header.find("$M") == 0){
					if( (wx + MagLen) <= Len ){
						if( m_NumDSOReceived > 0 ){ //only use magnetometer if we have at least one dso msg
							if( (UnixTime >= m_LastDSOUnixTime) ){ //don't need a packet counter check because aspect packets are slower than once a second
								//the above check on unix time and aspect ID are so that if we get misordered packets,
								//and the first subpacket is a magnetometer packet, we won't use the DSO info 
								//from the last DSO message processed, since this will have happened in the future.
								vector<uint8_t> MagMsg;
								MAspectPacket MagPacket;
								MagMsg.assign( NextPacket.begin() + wx, NextPacket.begin() + wx + MagLen );
								DecodeMag( MagMsg, MagPacket );

								//copy over all necessary parameters to MagPacket from m_LastDSOPacket
								MagPacket.geographic_longitude = m_LastDSOPacket.geographic_longitude;
								MagPacket.geographic_latitude = m_LastDSOPacket.geographic_latitude;
								MagPacket.elevation = m_LastDSOPacket.elevation;
								MagPacket.date_and_time = m_LastDSOPacket.date_and_time;
								MagPacket.CorrectedClk = m_LastDSOPacket.CorrectedClk;
								MagPacket.UnixTime = UnixTime;
								MagPacket.GPSms = m_LastDSOPacket.GPSms;
								MagPacket.PPSClk = m_LastDSOPacket.PPSClk;

								if( m_UseMagnetometer ){
									m_AspectReconstructor->AddAspectFrame( MagPacket );
								}
							}
						}

						wx += MagLen;
					} else {
						NotEnoughBytes = true;
					}
				} else {
					wx += 1;
				}
			} else {
				NotEnoughBytes = true;
			}
		} else {
			wx += 1;
		}

		if( NotEnoughBytes ){
			break;
		}
	}

	return true;

}


/////////////////////////////////////////////////////////////////////////////////

/*
bool MBinaryFlightDataParser::ProcessAspect_works( vector<uint8_t> & NextPacket ){

	//look for '$'

	int Len = NextPacket.size();
	int wx = 0; // skip to first byte
	bool NotEnoughBytes = false;
	int i;
	time_t UnixTime;
	struct tm * timeinfo;
	char DateString[32];
	string cpp_DateString;
	//uint32_t LastClkSample;
	uint64_t UpperClkBytes;


	int DSOLen = 84; //length of the DSO message including the last 5 bytes for the clock
	int MagLen = 24; //length of the magnetometer message including my "$M" header


	int UnixBytes = 0;
	UnixBytes |= ((int) NextPacket[3] << 16);
	UnixBytes |= ((int) NextPacket[4] << 8);
	UnixBytes |= ((int) NextPacket[5]);

	//checked the GCU unix time before launch on 12/20/14-00:32:00 UTC
	//and got 0x5494C5B1, which will rollover the lower three bytes in ~81 days
	//so if we have a flight longer than ~81 days, use 0x55 for the upper byte
	//if the lower three bytes is less than 0x0094C5B1
	if( UnixBytes > 0x0094C5B1 ){
		UnixBytes |= 0x54000000;
	} else {
		UnixBytes |= 0x55000000;
	}

	UnixTime = (time_t) UnixBytes;

	timeinfo = gmtime(&UnixTime);
	//date format has to be year/month/day for Ares' thing to work
	strftime( DateString, sizeof(DateString), "%Y/%m/%d", timeinfo);
	//should figure out what day it is, but then use the GPS millisecond to figure out exactly what time it is
	//then use the GPS ms time to figure out exactly what time it is

	//now use strftime to get a date string using this time
	//according to Ares the GPS week starts at 23:59:44 on saturday night DOUBLE CHECK THIS
	cpp_DateString = string( DateString ); 

	//get the upper two 10 MHz clock byte from the beginning of the packet
	UpperClkBytes = 0; UpperClkBytes = ((uint64_t) NextPacket[10] << 40) | ((uint64_t) NextPacket[11] << 32);


	while( wx < Len ){

		if( NextPacket[wx] == '$' ){
			//check that we have enough bytes to determine the message type
			if( (wx + 10) < Len ){
				//determine the type:
				string Header ;
				for( i = 0; i < 10; ++i ) Header += (char) NextPacket[wx + i];

				////////////////////// DSO Msg //////////////////////////////////
				if( Header.find("$PASHR,DSO") == 0 ){
					//check that we have enough bytes in the buffer for this
					if( (wx + DSOLen) < Len ){

						//from unix time, get the GPS week, and include GPSWeek in the MAspectPAcket
						

						m_NumDSOReceived++;
						vector<uint8_t> DSOMsg;
						DSOMsg.assign( NextPacket.begin() + wx, NextPacket.begin() + wx + DSOLen );
						MAspectPacket DSOPacket;
						DecodeDSO( DSOMsg, DSOPacket );//transfer info from DSO msg into an MAspectPacket
						string TempString = cpp_DateString + DSOPacket.date_and_time; //prepend the date, date computed above using unix time
						DSOPacket.date_and_time = TempString;
						DSOPacket.PPSClk |= UpperClkBytes;
						DSOPacket.CorrectedClk = DSOPacket.PPSClk + ((DSOPacket.GPSMilliseconds % 1000)*10000);//estimate the clock board value at the time of the DSO message
						
						//get the GPS week
						int SecondsSinceGPSEpoch = (UnixTime - 315964800) + 16; //16 seconds is number of leapseconds introduced since 1980 GPS epoch
						int GPSWeek = SecondsSinceGPSEpoch/(60*60*24*7);

						DSOPacket.GPSWeek = GPSWeek;
						m_LastGPSWeek = GPSWeek; //keep this around for the magnetometer
						m_LastCorrectedClk = DSOPacket.CorrectedClk; // keep this around for the magnetometer
						m_LastDateTimeString = DSOPacket.date_and_time; //keep this around for the magnetometer
						m_LastLatitude = DSOPacket.geographic_latitude;
						m_LastLongitude = DSOPacket.geographic_longitude;
						m_LastAltitude = DSOPacket.elevation;
						//DSOPacket.CorrectedClk is converted to MTime in AddAspectFrame.
						if( m_UseGPSDSO ){
							m_AspectReconstructor->AddAspectFrame( DSOPacket );
						}
						wx += DSOLen;
					} else {
						NotEnoughBytes = true;
					}

					////////////////////// Mag Msg //////////////////////////////////
				} else if( Header.find("$M") == 0){
					if( (wx + MagLen) < Len ){
						if( m_NumDSOReceived > 0 ){ //only use magnetometer if we have at least one dso msg
							vector<uint8_t> MagMsg;
							MAspectPacket MagPacket;
							MagMsg.assign( NextPacket.begin() + wx, NextPacket.begin() + wx + MagLen );
							DecodeMag( MagMsg, MagPacket );
							//check if the LastDateTimeString is valid! if not 
							if( m_LastDateTimeString.size() > 0 ){
								MagPacket.date_and_time = m_LastDateTimeString;
							} else {
								//we don't have have a GPS time string for this event, and we need one for Ares to compute the
								//aspect stuff in pyephem
								//make a date/time string using the unix second
								//need to use Ares' format.
								//for now, skip
								wx += MagLen; continue;
							}

							if( m_LastCorrectedClk == 0xffffffffffffffff ){
								//we havent received a clock sample yet, so there is no way for us to look up
								//a timestamp and we can't use this for the aspect reconstruction.  this could happen if:
								//you just started nuclearizer, and the first aspect packet you received started with a 
								//magnetometer sample.  

							} else {
								MagPacket.CorrectedClk = m_LastCorrectedClk;
							}
							//MagPacket.CorrectedClk is converted to MTime in AddAspectFrame
							if( m_UseMagnetometer ){
								m_AspectReconstructor->AddAspectFrame( MagPacket );
							}
						}

						wx += MagLen;
					} else {
						NotEnoughBytes = true;
					}
				} else {
					wx += 1;
				}
			} else {
				NotEnoughBytes = true;
			}
		} else {
			wx += 1;
		}

		if( NotEnoughBytes ){
			break;
		}
	}

	return true;

}

*/

////////////////////////////////////////////////////////////////////////////////

bool MBinaryFlightDataParser::DecodeDSO(vector<uint8_t> & DSOString, MAspectPacket& GPS_Packet){

	uint32_t MySeconds;
	MySeconds = 0;
	MySeconds = (((uint32_t)DSOString[11] & 0xFF) << 24) | (((uint32_t)DSOString[12] & 0xFF) << 16) | (((uint32_t)DSOString[13] & 0xFF)  << 8) | ((uint32_t)DSOString[14] & 0xFF);  
	//printf("%02x %02x %02x %02x - ", (DSOString[11] & 0xFF),(DSOString[12] & 0xFF),(DSOString[13] & 0xFF),(DSOString[14] & 0xFF));
	if (g_Verbosity >= c_Info) printf("DSO Packet: Milliseconds = %u, ",MySeconds);  //Again these are Carolyn's packets, not MAspectPacket objects
	long intermediate_seconds = MySeconds;
	GPS_Packet.GPSMilliseconds = MySeconds;


	uint64_t MyHeading_int = 0;
	double MyHeading = 0.0;
	MyHeading_int = (((uint64_t)DSOString[15] & 0xFF) << 56) | (((uint64_t)DSOString[16] & 0xFF) << 48) | (((uint64_t)DSOString[17] & 0xFF) << 40) |  (((uint64_t)DSOString[18] & 0xFF) << 32) |  (((uint64_t)DSOString[19] & 0xFF) << 24) | (((uint64_t)DSOString[20] & 0xFF) << 20) |  (((uint64_t)DSOString[21] & 0xFF) << 8) | ((uint64_t)DSOString[22] & 0xFF);
	MyHeading = *(double *) &MyHeading_int;
	if (g_Verbosity >= c_Info) printf("Heading = %4.2f, ", MyHeading); 
	GPS_Packet.heading =  MyHeading;


	uint64_t MyPitch_int = 0;
	double MyPitch = 0.0;
	MyPitch_int = (((uint64_t)DSOString[23] & 0xFF) << 56) | (((uint64_t)DSOString[24] & 0xFF) << 48) | (((uint64_t)DSOString[25] & 0xFF) << 40) |  (((uint64_t)DSOString[26] & 0xFF) << 32) |  (((uint64_t)DSOString[27] & 0xFF) << 24) | (((uint64_t)DSOString[28] & 0xFF) << 16) |  (((uint64_t)DSOString[29] & 0xFF) << 8) | ((uint64_t)DSOString[30] & 0xFF);
	MyPitch = *(double *) &MyPitch_int;
	if (g_Verbosity >= c_Info) printf("Pitch = %4.2f, ", MyPitch);  
	GPS_Packet.pitch =  MyPitch;


	uint64_t MyRoll_int = 0;
	double MyRoll = 0.0;
	MyRoll_int = (((uint64_t)DSOString[31] & 0xFF) << 56) | (((uint64_t)DSOString[32] & 0xFF) << 48) | (((uint64_t)DSOString[33] & 0xFF) << 40) |  (((uint64_t)DSOString[34] & 0xFF) << 32) |  (((uint64_t)DSOString[35] & 0xFF) << 24) | (((uint64_t)DSOString[36] & 0xFF) << 16) |  (((uint64_t)DSOString[37] & 0xFF) << 8) | ((uint64_t)DSOString[38] & 0xFF);
	MyRoll = *(double *) &MyRoll_int;
	if (g_Verbosity >= c_Info) printf("Roll = %4.2f, ", MyRoll);  
	GPS_Packet.roll =  MyRoll;


	uint64_t MyBRMS_int = 0;
	double MyBRMS = 0.0;
	MyBRMS_int = (((uint64_t)DSOString[39] & 0xFF) << 56) | (((uint64_t)DSOString[40] & 0xFF) << 48) | (((uint64_t)DSOString[41] & 0xFF) << 40) |  (((uint64_t)DSOString[42] & 0xFF) << 32) |  (((uint64_t)DSOString[43] & 0xFF) << 24) | (((uint64_t)DSOString[44] & 0xFF) << 16) |  (((uint64_t)DSOString[45] & 0xFF) << 8) | ((uint64_t)DSOString[46] & 0xFF);
	MyBRMS = *(double *) &MyBRMS_int;
	if (g_Verbosity >= c_Info) printf("BRMS = %f, ", MyBRMS);  
	GPS_Packet.BRMS = MyBRMS;


	uint8_t MyAtt_flag_1;
	MyAtt_flag_1 = (uint8_t)DSOString[47] & 0xFF;
	if (g_Verbosity >= c_Info) printf("Attitude Flag #1 = %u, ", MyAtt_flag_1);

	uint8_t MyAtt_flag_2;
	MyAtt_flag_2 = (uint8_t)DSOString[48] & 0xFF;
	if (g_Verbosity >= c_Info) printf("Attitude Flag #2 = %u, ", MyAtt_flag_2);

	GPS_Packet.AttFlag = ((uint16_t) MyAtt_flag_1 << 8) | ((uint16_t) MyAtt_flag_2);


	uint64_t MyLat_int = 0;
	double MyLat;
	MyLat_int = (((uint64_t)DSOString[49] & 0xFF) << 56) | (((uint64_t)DSOString[50] & 0xFF) << 48) | (((uint64_t)DSOString[51] & 0xFF) << 40) |  (((uint64_t)DSOString[52] & 0xFF) << 32) |  (((uint64_t)DSOString[53] & 0xFF) << 24) | (((uint64_t)DSOString[54] & 0xFF) << 16) |  (((uint64_t)DSOString[55] & 0xFF) << 8) | ((uint64_t)DSOString[56] & 0xFF);
	MyLat = *(double *) &MyLat_int;
	if (g_Verbosity >= c_Info) printf("Latitude = %4.2f, ", MyLat);  
	GPS_Packet.geographic_latitude =  MyLat;


	uint64_t MyLong_int = 0;
	double MyLong = 0.0;
	MyLong_int = (((uint64_t)DSOString[57] & 0xFF) << 56) | (((uint64_t)DSOString[58] & 0xFF) << 48) | (((uint64_t)DSOString[59] & 0xFF) << 40) | (((uint64_t)DSOString[60] & 0xFF) << 32) | (((uint64_t)DSOString[61] & 0xFF) << 24) | (((uint64_t)DSOString[62] & 0xFF) << 16) | (((uint64_t)DSOString[63] & 0xFF) << 8) | ((uint64_t)DSOString[64] & 0xFF);
	MyLong = *(double *) &MyLong_int;
	if (g_Verbosity >= c_Info) printf("Longitude = %4.2f, ", MyLong); 
	GPS_Packet.geographic_longitude =  MyLong;


	uint64_t MyAlt_int = 0;
	double MyAlt = 0.0;
	MyAlt_int = (((uint64_t)DSOString[65] & 0xFF) << 56) | (((uint64_t)DSOString[66] & 0xFF) << 48) | (((uint64_t)DSOString[67] & 0xFF) << 40) |  (((uint64_t)DSOString[68] & 0xFF) << 32) |  (((uint64_t)DSOString[69] & 0xFF) << 24) | (((uint64_t)DSOString[70] & 0xFF) << 16) |  (((uint64_t)DSOString[71] & 0xFF) << 8) | ((uint64_t)DSOString[72] & 0xFF);
	MyAlt = *(double *) &MyAlt_int;
	if (g_Verbosity >= c_Info) printf("Altitude = %4.2f, ", MyAlt); 
	GPS_Packet.elevation =  MyAlt; 


	//NEEDS WORK!
	uint16_t MyChecksum;
	MyChecksum = (((uint16_t)DSOString[73] & 0xFF) << 8) | ((uint16_t)DSOString[74] & 0xFF);
	//printf("Hex Checksum = %c %c %c %c %c- ", (char)(DSOString [72] & 0xFF), (char)(DSOString[73] & 0xFF), (char)(DSOString[74] & 0xFF), (char)(DSOString[75] & 0xFF), (char)(DSOString[76] & 0xFF));
	if (g_Verbosity >= c_Info) printf("MyChecksum? = %u, ", MyChecksum);

	uint32_t MyClock = 0;  //counting 10 MHz clock signal.
	MyClock = (((uint32_t)DSOString[80] & 0xFF) << 24) | (((uint32_t)DSOString[81] & 0xFF) << 16) | (((uint32_t)DSOString[82] & 0xFF)  << 8) | ((uint32_t)DSOString[83] & 0xFF);  
	if (g_Verbosity >= c_Info) printf("Clock =  %u. \n",MyClock);  
	GPS_Packet.PPSClk = MyClock;

  GPS_Packet.GPS_or_magnetometer = 0;
  
  if (g_Verbosity >= c_Info) {
    printf("\n");  
    printf("Now, here is what's from GPS_Packet: \n");
    printf("Heading: \n"); 
    printf("%f\n",GPS_Packet.heading); 
    printf("Pitch: \n"); 
    printf("%f\n",GPS_Packet.pitch); 
    printf("Roll: \n"); 
    printf("%f\n",GPS_Packet.roll); 
    printf("Geographic Latitude: \n"); 
    printf("%f\n",GPS_Packet.geographic_latitude); 
    printf("Geographic Longitude: \n"); 
    printf("%f\n",GPS_Packet.geographic_longitude);  
    printf("Elevation: \n"); 
    printf("%f\n",GPS_Packet.elevation); 
  }

	string slash = "/";
	string colon = ":";
	string twenty = "20";
	string space = " ";
	string zero = "0";


	long milliseconds = intermediate_seconds - 16000;  //this will only work for one day (date collected must be same date file with data was made because this program will trust that that date is correct)


	int hours = 0;
	int minutes = 0;
	int seconds = 0;
	unsigned int nanoseconds = 0;



	while(milliseconds > 86400000){
		milliseconds = milliseconds - 86400000;
	}
	while(milliseconds > 3600000){
		hours = hours +1;
		milliseconds = milliseconds - 3600000;
	}
	while(milliseconds > 60000){
		minutes = minutes +1;
		milliseconds = milliseconds - 60000;
	}
	while(milliseconds > 1000){
		seconds = seconds +1;
		milliseconds = milliseconds - 1000;
	} 



	string Hours = to_string(hours);
	string Minutes = to_string(minutes);
	string Seconds = to_string(seconds);



	if(hours < 10){
		Hours = zero + Hours;
	}
	if(minutes < 10){
		Minutes = zero + Minutes;
	}
	if(seconds < 10){
		Seconds = zero + Seconds;
	} 



	//string date_and_time = date + space + Hours + colon + Minutes + colon + Seconds; 
	string date_and_time = space + Hours + colon + Minutes + colon + Seconds; 
	GPS_Packet.date_and_time = date_and_time;  


	nanoseconds = milliseconds * 1000000;
	GPS_Packet.nanoseconds = nanoseconds;


  if (g_Verbosity >= c_Info) {
    printf("Date_and_Time: \n"); 

    cout << GPS_Packet.date_and_time << endl; 

    printf("Nanoseconds: \n"); 
    printf("%u\n",GPS_Packet.nanoseconds); 

    printf("\n"); 
  }

	return true;

}

/////////////////////////////////////////////////////////////////////


bool MBinaryFlightDataParser::DecodeMag(vector<uint8_t>& MagString, MAspectPacket& M_Packet){

	//  printf(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n", MagString[0] & 0xFF, MagString[1] & 0xFF, MagString[2] & 0xFF, MagString[3] & 0xFF, MagString[4] & 0xFF, MagString[5] & 0xFF, MagString[6] & 0xFF, MagString[7] & 0xFF, MagString[8] & 0xFF, MagString[9] & 0xFF, MagString[10] & 0xFF, MagString[11] & 0xFF, MagString[12] & 0xFF, MagString[13] & 0xFF, MagString[14] & 0xFF, MagString[15] & 0xFF, MagString[16] & 0xFF, MagString[17] & 0xFF, MagString[18] & 0xFF, MagString[19] & 0xFF, MagString[20] & 0xFF, MagString[21] & 0xFF, MagString[22] & 0xFF, MagString[23] & 0xFF);

	int16_t MyRoll_int;
	float MyRoll = 0.0;
	MyRoll_int = (((int16_t)MagString[4] & 0xFF) << 8) | ((int16_t)MagString[5] & 0xFF);  
	MyRoll = MyRoll_int/10.0;
	if (g_Verbosity >= c_Info) printf("Magnetometer Packet: Roll = %4.2f, ",MyRoll); //Once again, let me be clear, these are Carolyn's packets, not MAspectPacket objects
	M_Packet.roll =  MyRoll; 


	int16_t MyMagRoll_int;
	float MyMagRoll = 0.0;
	MyMagRoll_int = (((int16_t)MagString[6] & 0xFF) << 8) | ((int16_t)MagString[7] & 0xFF);  
	MyMagRoll = MyMagRoll_int/10.0;
	if (g_Verbosity >= c_Info) printf("Mag Roll = %4.2f, ",MyMagRoll);  


	int16_t MyInclination_int;
	float MyInclination = 0.0;
	MyInclination_int = (((int16_t)MagString[8] & 0xFF) << 8) | ((int16_t)MagString[9] & 0xFF);  
	MyInclination = MyInclination_int/10.0;
	if (g_Verbosity >= c_Info) printf("Inclination = %4.2f, ",MyInclination); 
	M_Packet.pitch =  MyInclination; 


	int16_t MyMagTot_int;
	float MyMagTot = 0.0;
	MyMagTot_int = (((int16_t)MagString[10] & 0xFF) << 8) | ((int16_t)MagString[11] & 0xFF);  
	MyMagTot = MyMagTot_int/10000.0;
	if (g_Verbosity >= c_Info) printf("Mag Total = %4.2f, ",MyMagTot);  


	int16_t MyAzi_int;
	float MyAzi = 0.0;
	MyAzi_int = (((int16_t)MagString[12] & 0xFF) << 8) | ((int16_t)MagString[13] & 0xFF);  
	MyAzi = MyAzi_int/10.0;
	if (g_Verbosity >= c_Info) printf("Azimuth = %4.2f, ",MyAzi);  
	M_Packet.heading =  MyAzi; 


	int16_t MyAccel_int;
	float MyAccel = 0.0;
	MyAccel_int = (((int16_t)MagString[14] & 0xFF) << 8) | ((int16_t)MagString[15] & 0xFF);  
	MyAccel = MyAccel_int/10000.0;
	if (g_Verbosity >= c_Info) printf("Acceleration = %4.2f, ",MyAccel);  


	int16_t MyTemp_int;
	float MyTemp = 0.0;
	MyTemp_int = (((int16_t)MagString[16] & 0xFF) << 8) | ((int16_t)MagString[17] & 0xFF);  
	MyTemp = MyTemp_int/100.0;
	if (g_Verbosity >= c_Info) printf("Temperature = %4.2f, ",MyTemp);  


	int16_t MyVolt_int;
	float MyVolt = 0.0;
	MyVolt_int = (((int16_t)MagString[18] & 0xFF) << 8) | ((int16_t)MagString[19] & 0xFF);  
	MyVolt = MyVolt_int/100.0;
	if (g_Verbosity >= c_Info) printf("Voltage = %4.2f, ",MyVolt);  


	//CHECKSUM!


	//the following commented block assumes that the timestamp is always right before the mag data
	//don't assume this! just use the last read PPSClk in the calling thread
	/*

		uint32_t MyClock = 0;  //counting 10 MHz clock signal.
		MyClock = (((uint32_t)MagString[-4] & 0xFF) << 24) | (((uint32_t)MagString[-3] & 0xFF) << 16) | (((uint32_t)MagString[-2] & 0xFF)  << 8) | ((uint32_t)MagString[-1] & 0xFF);  
		printf("Clock =  %u. \n",MyClock);  
		MyClock = MyMag->Clock;
	 */


  M_Packet.GPS_or_magnetometer = 1;

  if (g_Verbosity >= c_Info) {
    printf("\n"); 
    printf("Now, here is what's from M_Packet: \n");
    printf("Heading: \n"); 
    printf("%f\n",M_Packet.heading); 
    printf("Pitch: \n"); 
    printf("%f\n",M_Packet.pitch); 
    printf("Roll: \n"); 
    printf("%f\n",M_Packet.roll); 

    // M_Packet.geographic_latitude = GPS_Packet.geographic_latitude;
    // M_Packet.geographic_longitude = GPS_Packet.geographic_longitude;
    // M_Packet.elevation = GPS_Packet.elevation;

    printf("This is GPS stuff we put in the M_Packet: \n");
    printf("Geographic Latitude: \n"); 
    printf("%f\n",M_Packet.geographic_latitude); 
    printf("Geographic Longitude: \n"); 
    printf("%f\n",M_Packet.geographic_longitude);  
    printf("Elevation: \n"); 
    printf("%f\n",M_Packet.elevation); 

    // M_Packet.date_and_time = GPS_Packet.date_and_time;
    // M_Packet.nanoseconds = GPS_Packet.nanoseconds;  

    printf("Date_and_Time: \n");
    cout << M_Packet.date_and_time << endl;  
    printf("Nanoseconds: \n"); 
    printf("%u\n",M_Packet.nanoseconds);  
    printf("\n"); 
  }

	//AR->AddAspectFrame(M_Packet);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////


bool MBinaryFlightDataParser::ComptonDataframe2Struct( vector<uint8_t>& Buf, dataframe * DataOut ){

	size_t wx = 0;
	size_t BufSize = Buf.size();
	int EvCnt = 0;

	if( DataOut == NULL ){
		return false;
	}

	if( BufSize < 16 ){
		return false;
	}

	int CalculatedLen = ((int) Buf[8] << 8) | ((int) Buf[9]);

	int UnixInt = ((int) Buf[3] << 16) | ((int) Buf[4] << 8) | ((int) Buf[5]);
	DataOut->UnixTime = (time_t) UnixInt;
	DataOut->PacketCounter = ((int) Buf[6] << 8) | ((int) Buf[7]);
	DataOut->Length = CalculatedLen;
	DataOut->SysTime = 0;
	DataOut->SysTime = ((uint64_t) Buf[10] << 40) | ((uint64_t) Buf[11] << 32) | ((uint64_t) Buf[12] << 24) | ((uint64_t) Buf[13] << 16) | ((uint64_t) Buf[14] << 8) | ((uint64_t) Buf[15]);
	wx = 16;

	while( wx < BufSize ){
		if( Buf[wx] == 0xae ){
			//we are at the beginning of an event, read it in

			wx += 7; if( wx > BufSize ) { DataOut->ParseError = true; return false; }
			event NewEvent;
			EvCnt++;
			NewEvent.EventID = Buf[wx - 6];
			NewEvent.NumCCsInvolved = Buf[wx - 5] & 0x0f;
			int N = NewEvent.NumCCsInvolved; if( N > 12 ){ DataOut->ParseError = true; return false; }
			NewEvent.EventTime = 0;
			NewEvent.EventTime = ((uint64_t) Buf[wx-4] << 24) | ((uint64_t) Buf[wx-3] << 16) | ((uint64_t) Buf[wx-2] << 8) | ((uint64_t) Buf[wx-1]); 

			//loop over triggered card cages
			//loop over triggers

			for( int i = 0; i < N; ++i ){
				wx += 2; if( wx > BufSize ) { DataOut->ParseError = true; return false; }
				int CurrentCC = (int) Buf[wx - 2];
				int NumTriggers = (int) Buf[wx - 1];
				for( int j = 0; j < NumTriggers; ++j ){

					//at this point, wx points at the trigger byte

					wx += 3; if( wx > BufSize ) { DataOut->ParseError = true; return false; }
					trigger NewTrig;
					NewTrig.HasADC = true;
					NewTrig.Channel = Buf[wx-3] & 0x0f;
					NewTrig.Board = (Buf[wx-3] & 0x70) >> 4;
					if( Buf[wx-3] & 0x80 ){
						NewTrig.HasTiming = true;
					} else {
						NewTrig.HasTiming = false;
					}

					NewTrig.ADCBytes = ((uint16_t) Buf[wx - 2] << 8) | ((uint16_t) Buf[wx - 1]);
					if( NewTrig.HasTiming ){
						wx += 1; if( wx > BufSize ) { DataOut->ParseError = true; return false; }
						NewTrig.TimingByte = Buf[wx - 1];
					}

					NewTrig.CCId = CurrentCC;
					NewEvent.Triggers.push_back(NewTrig);

				}
			}

			DataOut->Events.push_back(NewEvent);

		} else { DataOut->ParseError = true; return false; }
	}

	//	cout<<"compton ::: pktcnt="<<DataOut->PacketCounter<<" evcnt "<<EvCnt<<endl;

	if( wx != BufSize ) return false; else return true;

}


///////////////////////////////////////////////////////////////////////////////

void MBinaryFlightDataParser::SetIsDone(bool IsDone)
{
	m_IsDone = IsDone;
	m_AspectReconstructor->SetIsDone(IsDone);
}

// MBinaryFlightDataParser.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
