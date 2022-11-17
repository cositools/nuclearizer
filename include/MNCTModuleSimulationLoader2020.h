/*
 * MNCTModuleSimulationLoader2020.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleSimulationLoader2020__
#define __MNCTModuleSimulationLoader2020__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MModule.h"
#include "MNCTDetectorEffectsEngineCOSI2020.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleSimulationLoader2020 : public MModule, public MNCTDetectorEffectsEngineCOSI2020
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleSimulationLoader2020();
  //! Default destructor
  virtual ~MNCTModuleSimulationLoader2020();
  
  //! Create a new object of this class 
  virtual MNCTModuleSimulationLoader2020* Clone() { return new MNCTModuleSimulationLoader2020(); }
 
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
  ClassDef(MNCTModuleSimulationLoader2020, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
