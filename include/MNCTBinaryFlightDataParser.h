/*
 * MNCTBinaryFlightDataParser.h
 *
 * Copyright (C) by Alex Lowell & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTBinaryFlightDataParser__
#define __MNCTBinaryFlightDataParser__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <list>
#include <fstream>
#include <map>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTransceiverTcpIpBinary.h"

// Nuclearizer libs
#include "MNCTAspectReconstruction.h"
#include "MReadOutAssembly.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The data modes: analyze raw events, Compton events, or all events
enum class MNCTBinaryFlightDataParserDataModes : unsigned int { c_Raw = 0, c_Compton = 1, c_All = 2 };    

//! Aspect modes... use the GPS, the magnetometer, or neither
enum class MNCTBinaryFlightDataParserAspectModes : unsigned int {c_GPS = 0, c_Magnetometer, c_Neither};

  
////////////////////////////////////////////////////////////////////////////////


class MNCTBinaryFlightDataParser
{
  // public interface:
 public:
  //! Default constructor
  MNCTBinaryFlightDataParser();
  //! Default destructor
  virtual ~MNCTBinaryFlightDataParser();
  
  //! Get the data selection mode
  MNCTBinaryFlightDataParserDataModes GetDataSelectionMode() const { return m_DataSelectionMode; }
  //! Set the data selection mode
  void SetDataSelectionMode(MNCTBinaryFlightDataParserDataModes Mode) { m_DataSelectionMode = Mode; } 

  //! Get the aspect mode
  MNCTBinaryFlightDataParserAspectModes GetAspectMode() const { return m_AspectMode; }
  //! Set the aspect mode
  void SetAspectMode(MNCTBinaryFlightDataParserAspectModes Mode) { m_AspectMode = Mode; }

  //! Enable/Disable Coincidence merging
  void EnableCoincidenceMerging(bool X) {m_CoincidenceEnabled = X;}
  //! Get coincidence merging true/false
  bool GetCoincidenceMerging() const { return m_CoincidenceEnabled; }
 
  //! Parse some data, return true if the module is ready to analyze events
  virtual bool ParseData(vector<uint8_t> Received) ;
  
  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  // protected methods:
 protected:


  // private methods:
 private:
  void LoadStripMap(void);
  void LoadCCMap(void);



  // protected members:
 protected:
  //! The data selection mode (raw, Compton, all)
  MNCTBinaryFlightDataParserDataModes m_DataSelectionMode;

  //! The aspect mode (c_GPS, c_Magnetometer, c_Neither)
  MNCTBinaryFlightDataParserAspectModes m_AspectMode;

  //! Controls whether or not coincident events are merged
  bool m_CoincidenceEnabled;

  //! internal event list - sorted but unmerged events
  deque<MReadOutAssembly*> m_EventsBuf;//sorted, unmerged events

  //! The internal event list - final merged events
  deque<MReadOutAssembly*> m_Events;

  //! If true ignore aspect information if not ready
  bool m_IgnoreAspect;

  //! Flag that tells CheckEventsBuf to ignore the buffer search time, used in file mode when the file is over
  bool m_IgnoreBufTime;

  // private members:
 private:
  
  //! 
  
  //! The aspect reconstructor
  MNCTAspectReconstruction* m_AspectReconstructor;

  
  //added by AWL
  bool m_UseComptonDataframes;
  bool m_UseRawDataframes;
  uint32_t m_NumRawDataframes;
  uint32_t m_NumComptonDataframes;
  uint32_t m_NumAspectPackets;
  uint32_t m_NumOtherPackets;
  unsigned int MAX_TRIGS;
  unsigned long long m_EventTimeWindow;
  vector<uint64_t> LastTimestamps;
  uint64_t m_ComptonWindow;
  vector<uint8_t> m_SBuf;//search buffer for the incoming TCP data stream
  unsigned int dx; //index into search buffer
  unsigned int m_EventIDCounter;
  string m_LastDateTimeString;
  uint64_t m_LastCorrectedClk;
  bool m_UseGPSDSO;
  bool m_UseMagnetometer;
  double m_LastLatitude;
  double m_LastLongitude;
  double m_LastAltitude;
  int m_LastGPSWeek;
  time_t m_LastDSOUnixTime;
  uint16_t m_LastAspectID;
  MNCTAspectPacket m_LastDSOPacket;
  uint32_t m_NumDSOReceived;
  uint64_t LastComptonTimestamp;
  uint32_t m_NumComptonBytes;
  uint32_t m_NumRawDataBytes;
  uint32_t m_NumBytesReceived;
  uint32_t m_LostBytes;
  map<uint64_t,int> m_PacketRecord;
	


  int m_StripMap[8][10];
  int m_CCMap[12];

  class trigger{

	  public:
		  uint8_t Board;
		  int8_t Channel;
		  bool HasTiming;
		  bool HasADC;
		  uint16_t ADCBytes;
		  uint8_t TimingByte;
		  int CCId;

  };

  class event{

	  public:
		  uint64_t EventTime; 
		  uint8_t ErrorBoardList;
		  uint8_t ErrorInfo;
		  uint8_t EventID;
		  uint8_t TrigAndVetoInfo;
		  uint8_t FTPattern;
		  uint8_t LTPattern;
		  bool ParseError;
		  uint8_t CCId; 
		  uint8_t InternalCompton; 
		  uint8_t Touchable; 

		  uint32_t NumTriggers;
		  vector<trigger> Triggers;


		  //below is stuff that only applies to compton events
		  uint8_t NumCCsInvolved;
		  uint8_t TDiff;

  };

  class dataframe {

	  public:
		  uint16_t PacketCounter;
		  uint32_t UnixTime;
		  int Length;
		  uint8_t PacketType;
		  uint8_t CCId;
		  string RawOrCompton; //0 if this struct came from a raw data frame, or 1 if from compton
		  uint32_t ReportedNumEvents;
		  uint64_t SysTime;
		  uint32_t LifetimeBits;
		  uint8_t NumNormal;
		  uint8_t NumLLD;
		  uint8_t NumTOnly;
		  uint8_t NumNoData;
		  bool HasSysErr;

		  uint32_t NumEvents; //the number of events which is <= 41
		  vector<event> Events;

		  bool ParseError;

  };

  
 public:
  int RawDataframe2Struct( vector<uint8_t> Buf, dataframe * DataOut);
  bool ComptonDataframe2Struct( vector<uint8_t>& Buf, dataframe * DataOut); 
  bool ConvertToMReadOutAssemblys( dataframe * DataIn, vector<MReadOutAssembly*> * CEvents);
  bool SortEventsBuf(void);
  bool FlushEventsBuf(void);
  bool CheckEventsBuf(void);
  MReadOutAssembly * MergeEvents( deque<MReadOutAssembly*> * EventList );
  bool FindNextPacket( vector<uint8_t> & NextPacket, unsigned int * idx = NULL );
  bool ResyncSBuf(void);
  bool ProcessAspect( vector<uint8_t> & NextPacket );
  bool ProcessAspect_works( vector<uint8_t> & NextPacket );
  bool DecodeDSO( vector<uint8_t> & DSOString, MNCTAspectPacket & DSO_Packet);
  bool DecodeMag( vector<uint8_t> & MagString, MNCTAspectPacket & Mag_Packet);



  
  
#ifdef ___CINT___
 public:
  ClassDef(MNCTBinaryFlightDataParser, 0) // no description
#endif

};

#endif



////////////////////////////////////////////////////////////////////////////////
