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
#include "MNCTAspectReconstruction.h"

// Forward declarations:


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
  MString m_LocalReceivingHost;
  //! Port to send the data to
  int m_LocalReceivingPort;

  //! The transceiver
  MTransceiverTcpIpBinary* m_Receiver;
  
  //! The aspect reconstructor
  MNCTAspectReconstruction* m_AspectReconstructor;

  //! The internal event list
  deque<MNCTEvent*> m_Events;
  
  
  
#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleReceiverCOSI2014, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
