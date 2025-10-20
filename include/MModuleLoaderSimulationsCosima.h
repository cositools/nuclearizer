/*
 * MModuleLoaderSimulationsCosima.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleLoaderSimulationsCosima__
#define __MModuleLoaderSimulationsCosima__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MModule.h"
#include "MFileEventsSim.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleLoaderSimulationsCosima : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderSimulationsCosima();
  //! Default destructor
  virtual ~MModuleLoaderSimulationsCosima();
  
  //! Create a new object of this class 
  virtual MModuleLoaderSimulationsCosima* Clone() { return new MModuleLoaderSimulationsCosima(); }
 
  //! Set the geometry
  virtual void SetGeometry(MDGeometryQuest* Geometry) { MModule::SetGeometry(Geometry); }

  //! Set the simulation file name
  void SetSimulationFileName(const MString& FileName) { m_SimulationFileName = FileName; }
  //! Get the simulation file name
  MString GetSimulationFileName() const { return m_SimulationFileName; }

  //! Set if the stop after X accepted events flag is used
  void SetUseStopAfter(bool Flag) { m_UseStopAfter = Flag; }
  //! Return if the stop after X accepted events option is used
  bool UseStopAfter() const { return m_UseStopAfter; }
 
  //! Set the maximum number of accepted events
  void SetMaximumAcceptedEvents(unsigned long MaximumAcceptedEvents) { m_MaximumAcceptedEvents = MaximumAcceptedEvents; }
  //! Return the maximum number of accepted events
  unsigned long GetMaximumAcceptedEvents() const { return m_MaximumAcceptedEvents; }
     
 
  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);
  
  //! Finalize the module
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

  //! Stop after a certain amount of accpetd events
  bool m_UseStopAfter;
  //! The currently accepted events
  unsigned long m_AcceptedEvents;
  //! Stop after this amount of accepted events
  unsigned long m_MaximumAcceptedEvents;
  
  //! Simulation file name
  MString m_SimulationFileName;

  //! The event reader
  MFileEventsSim* m_Reader;

  //! The far field start area
  double m_StartAreaFarField;

  //! The number of simulated events
  unsigned long m_NumberOfSimulatedEvents;



#ifdef ___CLING___
 public:
  ClassDef(MModuleLoaderSimulationsCosima, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
