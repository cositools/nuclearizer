/*
 * MModuleLoaderMeasurementsROA.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleLoaderMeasurementsROA__
#define __MModuleLoaderMeasurementsROA__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileReadOuts.h"

// Nuclearizer libs:
#include "MModuleLoaderMeasurements.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleLoaderMeasurementsROA : public MModuleLoaderMeasurements
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderMeasurementsROA();
  //! Default destructor
  virtual ~MModuleLoaderMeasurementsROA();
  
  //! Create a new object of this class 
  virtual MModuleLoaderMeasurementsROA* Clone() { return new MModuleLoaderMeasurementsROA(); }

  //! The Open method has to be derived from MFileEvents to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way);

  //! Initialize the module
  virtual bool Initialize();

  //! Initialize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:
  //! Reads one event from file - return zero in case of no more events present or an Error occured
  bool ReadNextEvent(MReadOutAssembly* Event);

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Start of the observation time
  MTime m_StartObservationTime;
  //! Clock time belonging to the start of the observation time
  unsigned long m_StartClock; 
  //! End of the observation time
  MTime m_EndObservationTime;
  //! Clock time belonging to the end of the observation time
  unsigned long m_EndClock;

  //! The read-out file
  MFileReadOuts m_ROAFile;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MModuleLoaderMeasurementsROA, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
