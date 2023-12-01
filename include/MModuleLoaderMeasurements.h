/*
 * MModuleLoaderMeasurements.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleLoaderMeasurements__
#define __MModuleLoaderMeasurements__


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


class MModuleLoaderMeasurements : public MModule, public MFileEvents
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderMeasurements();
  //! Default destructor
  virtual ~MModuleLoaderMeasurements();
  
  //! Create a new object of this class 
  virtual MModuleLoaderMeasurements* Clone() = 0;
  
  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

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
  ClassDef(MModuleLoaderMeasurements, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
