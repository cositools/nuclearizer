/*
 * MNCTModuleSimulationLoader.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleSimulationLoader__
#define __MNCTModuleSimulationLoader__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MModule.h"
#include "MNCTDetectorEffectsEngineCOSI.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleSimulationLoader : public MModule, public MNCTDetectorEffectsEngineCOSI
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleSimulationLoader();
  //! Default destructor
  virtual ~MNCTModuleSimulationLoader();
  
  //! Create a new object of this class 
  virtual MNCTModuleSimulationLoader* Clone() { return new MNCTModuleSimulationLoader(); }
 
  //! Set the geometry
  virtual void SetGeometry(MDGeometryQuest* Geometry) { MModule::SetGeometry(Geometry); }
  
  //! Set geometry file name
  void SetGeometryFileName(const MString& FileName) { cout<<"Use SetGeometry instead"<<endl; abort(); }
 
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



#ifdef ___CLING___
 public:
  ClassDef(MNCTModuleSimulationLoader, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
