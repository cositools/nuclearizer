/*
 * MModuleRevan.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleRevan__
#define __MModuleRevan__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MFile.h"
#include "MSettingsRevan.h"
#include "MGeometryRevan.h"
#include "MRawEventAnalyzer.h"

// Nuclearizer libs:
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleRevan : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleRevan();
  //! Default destructor
  virtual ~MModuleRevan();
  
  //! Create a new object of this class 
  virtual MModuleRevan* Clone() { return new MModuleRevan(); }

  //! Get the revan configuration file name
  MString GetRevanConfigurationFileName() const { return m_RevanConfigurationFileName; }
  //! Set the revan configuration file name
  void SetRevanConfigurationFileName(MString RevanConfigurationFileName) { m_RevanConfigurationFileName = RevanConfigurationFileName; }
  
  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
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

  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The revan configuration file name
  MString m_RevanConfigurationFileName;

  //! The settings for revan
  MSettingsRevan* m_Settings;

  //! The revan geometry
  MGeometryRevan* m_ReconstructionGeometry;

  //! The raw event analyzer
  MRawEventAnalyzer* m_RawEventAnalyzer;
  
#ifdef ___CLING___
 public:
  ClassDef(MModuleRevan, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
