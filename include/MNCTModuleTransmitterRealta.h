/*
 * MNCTModuleTransmitterRealta.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleTransmitterRealta__
#define __MNCTModuleTransmitterRealta__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MNCTModule.h"
#include "MTransceiverTcpIp.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleTransmitterRealta : public MNCTModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleTransmitterRealta();
  //! Default destructor
  virtual ~MNCTModuleTransmitterRealta();

  //! Return the name of the host computer
  MString GetHostName() const { return m_HostName; }
  //! Set the name of the host computer
  void SetHostName(MString HostName) { m_HostName = HostName; }
  
  //! Return the port of the host computer
  int GetHostPort() const { return m_HostPort; }
  //! Set the port of the host computer
  void SetHostPort(int HostPort) { m_HostPort = HostPort; }
  
  //! Initialize the module - open the connection
  virtual bool Initialize();

  //! Main data analysis routine, which hand the data over to the transmitter
  virtual bool AnalyzeEvent(MNCTEvent* Event);

  //! Finalize the module - close the connection
  virtual void Finalize();

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The name of the computer to which we send the data
  MString m_HostName;
  //! The port on the computer to which we send the data
  int m_HostPort;

  //! The transmitter
  MTransceiverTcpIp* m_Transmitter;




#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleTransmitterRealta, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
