/*
 * MModuleMeasurementLoader.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleMeasurementLoader__
#define __MModuleMeasurementLoader__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"

// Nuclearizer libs
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleMeasurementLoader : public MModule, public MFileEvents
{
  // public interface:
 public:
  //! Default constructor
  MModuleMeasurementLoader();
  //! Default destructor
  virtual ~MModuleMeasurementLoader();
  
  //! Create a new object of this class 
  virtual MModuleMeasurementLoader* Clone() = 0;
  
  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

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




#ifdef ___CLING___
 public:
  ClassDef(MModuleMeasurementLoader, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
