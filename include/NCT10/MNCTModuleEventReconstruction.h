/*
 * MNCTModuleEventReconstruction.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleEventReconstruction__
#define __MNCTModuleEventReconstruction__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MRawEventAnalyzer.h"
#include "MGeometryRevan.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleEventReconstruction : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleEventReconstruction();
  //! Default destructor
  virtual ~MNCTModuleEventReconstruction();
  
  //! Create a new object of this class 
  virtual MNCTModuleEventReconstruction* Clone() { return new MNCTModuleEventReconstruction(); }

  //! Initialize the module --- has to be overwritten
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  // Get/Set for the data
  //! Set the file name of the revan configuration
  void SetRevanConfigurationFileName(MString FileName) { m_RevanConfigurationFileName = FileName; }
  //! Get the file name of the revan configuration
  MString GetRevanConfigurationFileName() { return m_RevanConfigurationFileName; }

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
  //! File name of the revan geometry description
  MGeometryRevan* m_GeometryRevan;

  //! File name of the revan configuration file
  MString m_RevanConfigurationFileName;

  //! The analyzer
  MRawEventAnalyzer* m_Analyzer;


#ifdef ___CLING___
 public:
  ClassDef(MNCTModuleEventReconstruction, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
