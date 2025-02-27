/*
 * MModuleLoaderSimulationsSingleDet.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleLoaderSimulationsSingleDet__
#define __MModuleLoaderSimulationsSingleDet__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MModule.h"
#include "MDetectorEffectsEngineSingleDet.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleLoaderSimulationsSingleDet : public MModule, public MDetectorEffectsEngineSingleDet
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderSimulationsSingleDet();
  //! Default destructor
  virtual ~MModuleLoaderSimulationsSingleDet();
  
  //! Create a new object of this class 
  virtual MModuleLoaderSimulationsSingleDet* Clone() { return new MModuleLoaderSimulationsSingleDet(); }
 
  //! Set the geometry
  virtual void SetGeometry(MDGeometryQuest* Geometry) { MModule::SetGeometry(Geometry); }
  
  //! Set geometry file name
  void SetGeometryFileName(const MString& FileName) { cout<<"Use SetGeometry instead"<<endl; abort(); }
 
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
  unsigned long m_AcceptedEvents;

  //! Stop after a certain amount of accpetd events
  bool m_UseStopAfter;
  //! Stop after this amount of accepted events
  unsigned long m_MaximumAcceptedEvents;
  

#ifdef ___CLING___
 public:
  ClassDef(MModuleLoaderSimulationsSingleDet, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
