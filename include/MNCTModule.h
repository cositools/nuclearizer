/*
 * MNCTModule.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModule__
#define __MNCTModule__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDGeometryQuest.h"
#include "MNCTEvent.h"
#include "MXmlNode.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModule();
  //! Default destructor
  virtual ~MNCTModule();

  //! Return the name of this module:
  MString GetName() { return m_Name; }

  //! Return the XML tag of this module:
  MString GetXmlTag() { return m_XmlTag; }

  //! Set the geometry
  void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }

  // Module types:
  static const int c_EventLoader              = 14;
  static const int c_EventLoaderSimulation    = 11;
  static const int c_EventLoaderMeasurement   = 12;
  static const int c_EventOrdering            = 15;
  static const int c_DetectorEffectsEngine    = 1;
  static const int c_EventFilter              = 2;
  static const int c_EnergyCalibration        = 3;
  static const int c_ChargeSharingCorrection  = 4;
  static const int c_DepthCorrection          = 5;
  static const int c_StripPairing             = 6;
  static const int c_Aspect                   = 7;
  static const int c_CrosstalkCorrection      = 8;
  static const int c_EventReconstruction      = 9;
  static const int c_Else                     = 10;
  static const int c_EventSaver               = 13;
  
  // IMPORTANT:
  // If you add one analysis level, make sure you also handle it in:
  // -> ALL module constructors!
  // -> MNCTData::GetHighestAnalysislevel()

  //! Return the number of the preceeding modules
  unsigned int GetNPreceedingModuleTypes() { return m_PreceedingModules.size(); }
  //! Return the preceeding module at position i (no error checks performed)
  int GetPreceedingModuleType(unsigned int i) { return m_PreceedingModules.at(i); }

  //! Return the number of module types
  unsigned int GetNModuleTypes() { return m_Modules.size(); }
  //! Return the module type at position i (no error checks performed)
  int GetModuleType(unsigned int i) { return m_Modules.at(i); }

  //! Return the number of the succeeding modules
  unsigned int GetNSucceedingModuleTypes() { return m_SucceedingModules.size(); }
  //! Return the succeeding module at position i (no error checks performed)
  int GetSucceedingModuleType(unsigned int i) { return m_SucceedingModules.at(i); }

  //! Initialize the module --- has to be overwritten
  virtual bool Initialize() = 0;

  //! Finalize the module --- can be overwritten
  virtual void Finalize() { return; }

  //! Report anything what we want after analsis
  virtual MString Report();

  //! Main data analysis routine, which updates the event to a new level 
  //! Has to be overwritten in derived class
  virtual bool AnalyzeEvent(MNCTEvent* Event) = 0;

  //! True if this module has an associated options GUI
  bool HasOptionsGUI() { return m_HasOptionsGUI; }
  //! Show the options GUI --- has to be overwritten!
  virtual void ShowOptionsGUI() {};

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  //! Return if the module is ready to analyze events
  virtual bool IsReady() { return m_IsReady; }

  //! Return if the status of the module is OK
  virtual bool IsOK() { return m_IsOK; }

  // protected methods:
 protected:
  //! Set the name of this module
  void SetName(MString Name) { m_Name = Name; }

  //! Set which modules are assumed to be already performed
  void AddPreceedingModuleType(int Type) { m_PreceedingModules.push_back(Type); }
  //! Add which type of module this is, e.g. c_EnergyCalibration
  //! This option ca be called twice to set two tasks of this modules!
  void AddModuleType(int Type) { m_Modules.push_back(Type); };
  //! Set which modules are expected to follow this one
  void AddSucceedingModuleType(int Type) { m_SucceedingModules.push_back(Type); };


  // private methods:
 private:



  // protected members:
 protected:
  //! Name of this module
  MString m_Name;
  //! Name of the XML tag --- has to be uniquie
  MString m_XmlTag;

  //! List of preceeding modules
  vector<int> m_PreceedingModules;
  //! List of succeeding modules
  vector<int> m_SucceedingModules;
  //! List of types of this modules
  vector<int> m_Modules;

  //! The Geometry description
  MDGeometryQuest* m_Geometry;

  //! True, if this module has an options GUI
  bool m_HasOptionsGUI;

  //! True, if the module is ready to analyze events
  bool m_IsReady;
  
  //! True, if the status of the module is OK
  bool m_IsOK;
  
  // private members:
 private:




#ifdef ___CINT___
 public:
  ClassDef(MNCTModule, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
