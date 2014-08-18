/*
 * MNCTModuleReceiverCOSI2014.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleReceiverCOSI2014__
#define __MNCTModuleReceiverCOSI2014__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <list>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTransceiverTcpIpBinary.h"

// Nuclearizer libs
#include "MNCTModule.h"
//#include "MNCTAspectReconstruction.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The data modes: analyze raw events, Compton events, or all events
enum class MNCTModuleReceiverCOSI2014DataModes : unsigned int { c_Raw = 0, c_Compton = 1, c_All = 2 };    

  
////////////////////////////////////////////////////////////////////////////////


class MNCTModuleReceiverCOSI2014 : public MNCTModule
{
  // public interface:
 public:
   
  //! Default constructor
  MNCTModuleReceiverCOSI2014();
  //! Default destructor
  virtual ~MNCTModuleReceiverCOSI2014();
  
  //! Return the name of the transmitting computer
  MString GetDistributorName() const { return m_DistributorName; }
  //! Set the name of the transmitting computer
  void SetDistributorName(MString DistributorName) { m_DistributorName = DistributorName; }
  
  //! Return the port of the transmitting computer
  int GetDistributorPort() const { return m_DistributorPort; }
  //! Set the port of the transmitting computer
  void SetDistributorPort(int DistributorPort) { m_DistributorPort = DistributorPort; }

  //! Return the stream ID to be transmitted
  MString GetDistributorStreamID() const { return m_DistributorStreamID; }
  //! Set the ID of the stream which should be transmitted
  void SetDistributorStreamID(MString DistributorStreamID) { m_DistributorStreamID = DistributorStreamID; }

  //! Return the local receiving host name
  MString GetLocalReceivingHostName() const { return m_LocalReceivingHostName; }
  //! Set the name of the local receiveing host
  void SetLocalReceivingHostName(MString LocalReceivingHostName) { m_LocalReceivingHostName = LocalReceivingHostName; }
  
  //! Return the port of the local receiving host computer
  int GetLocalReceivingPort() const { return m_LocalReceivingPort; }
  //! Set the port of the local receiving host computer
  void SetLocalReceivingPort(int LocalReceivingPort) { m_LocalReceivingPort = LocalReceivingPort; }
 
  //! Get the data selection mode
  MNCTModuleReceiverCOSI2014DataModes GetDataSelectionMode() const { return m_DataSelectionMode; }
  //! Set the data selection mode
  void SetDataSelectionMode(MNCTModuleReceiverCOSI2014DataModes Mode) { m_DataSelectionMode = Mode; } 
 
 
  //! Return if the module is ready to analyze events
  virtual bool IsReady();
  
  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module --- can be overwritten
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MNCTEvent* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  // protected methods:
 protected:
  //! Perform Handshake
  bool DoHandshake();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The name of the computer from which we receive the data
  MString m_DistributorName;
  //! The port on the computer from which we receive the data
  int m_DistributorPort;
  //! The stream ID which we want to receive
  MString m_DistributorStreamID;

  //! Where to send the data to
  MString m_LocalReceivingHostName;
  //! Port to send the data to
  int m_LocalReceivingPort;

  //! The data selection mode (raw, Compton, all)
  MNCTModuleReceiverCOSI2014DataModes m_DataSelectionMode;
  
  //! The transceiver
  MTransceiverTcpIpBinary* m_Receiver;
  
  //! The aspect reconstructor
  //MNCTAspectReconstruction* m_AspectReconstructor;

  //! The internal event list
  deque<MNCTEvent*> m_Events;

  //added by AWL
  bool m_UseComptonDataframes;
  bool m_UseRawDataframes;
  uint32_t m_NumRawDataframes;
  uint32_t m_NumComptonDataframes;
  uint32_t m_NumAspectPackets;
  uint32_t m_NumOtherPackets;
  
  
  
#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleReceiverCOSI2014, 0) // no description
#endif

};

#endif

//AWL adding my struct definitions for the dataframe parsing


struct trigger{

	uint8_t Board;
	int8_t Channel;
	uint8_t HasTiming;
	uint8_t HasADC;
	uint16_t ADCBytes;
	uint8_t TimingByte;

};

struct event{

	uint64_t EventTime; 
	uint8_t ErrorBoardList;
	uint8_t ErrorInfo;
	uint8_t EventID;
	uint8_t TrigAndVetoInfo;
	uint8_t FTPattern;
	uint8_t LTPattern;
	uint8_t ParseError;
	uint8_t CCId; 
	uint8_t InternalCompton; 
	uint8_t Touchable; 

	uint32_t NumTriggers;
	struct trigger * Triggers; 

};

struct dataframe {

	uint8_t Valid; 
	uint8_t CCId;
	uint8_t RawOrCompton; //0 if this struct came from a raw data frame, or 1 if from compton
	uint32_t ReportedNumEvents;
	uint64_t SysTime;
	uint32_t LifetimeBits;
	uint8_t NumNormal;
	uint8_t NumLLD;
	uint8_t NumTOnly;
	uint8_t NumNoData;
	uint8_t HasSysErr;

	uint32_t NumEvents; //the number of events which is <= 41
	struct event * Events[41]; //an array of pointers to dynamically allocated event structures

};


////////////////////////////////////////////////////////////////////////////////
