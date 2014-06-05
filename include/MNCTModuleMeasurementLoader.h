/*
 * MNCTModuleMeasurementLoader.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleMeasurementLoader__
#define __MNCTModuleMeasurementLoader__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"

// Nuclearizer libs
#include "MNCTModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleMeasurementLoader : public MNCTModule, public MFileEvents
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleMeasurementLoader();
  //! Default destructor
  virtual ~MNCTModuleMeasurementLoader();
  
  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MNCTEvent* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node) = 0;
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration() = 0;


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! Name of the detector which we are reading
  MString m_Detector;
  //! The number of events in the file
  unsigned int m_NEventsInFile;
  //! The number of good events in file
  unsigned int m_NGoodEventsInFile;

  // private members:
 private:




#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleMeasurementLoader, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
