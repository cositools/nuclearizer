/*
 * MModuleLoaderMeasurementsTRA.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleLoaderMeasurementsTRA__
#define __MModuleLoaderMeasurementsTRA__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEventsTra.h"
#include "MPhysicalEvent.h"

// Nuclearizer libs:
#include "MModuleLoaderMeasurements.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Loader for reconstructed events stored in a tra file
//! Fills each read-out assembly with the physical event, its ID, time, error and veto flags, and hits,
//! as if the event had passed the whole analysis up to and including event reconstruction
class MModuleLoaderMeasurementsTRA : public MModuleLoaderMeasurements
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderMeasurementsTRA();
  //! Default destructor
  virtual ~MModuleLoaderMeasurementsTRA();

  //! Create a new object of this class
  virtual MModuleLoaderMeasurementsTRA* Clone() { return new MModuleLoaderMeasurementsTRA(); }

  //! Open the tra file
  virtual bool Open(MString FileName, unsigned int Way);

  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  //! Main data analysis routine, which fills the read-out assembly with the next event from file
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

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
  //! The tra file
  MFileEventsTra m_TraFile;


#ifdef ___CLING___
 public:
  ClassDef(MModuleLoaderMeasurementsTRA, 0) // Loader for reconstructed events from tra files
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
