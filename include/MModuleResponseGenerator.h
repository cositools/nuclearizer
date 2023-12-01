/*
 * MModuleResponseGenerator.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleResponseGenerator__
#define __MModuleResponseGenerator__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MFile.h"
#include "MResponseBuilder.h"

// Nuclearizer libs:
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleResponseGenerator : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleResponseGenerator();
  //! Default destructor
  virtual ~MModuleResponseGenerator();
  
  //! Create a new object of this class 
  virtual MModuleResponseGenerator* Clone() { return new MModuleResponseGenerator(); }

  //! Get the mode
  unsigned int GetMode() const { return m_Mode; }
  //! Set the mode
  void SetMode(unsigned int Mode) { m_Mode = Mode; }

  //! Get the response name
  MString GetResponseName() const { return m_ResponseName; }
  //! Set the response name
  void SetResponseName(MString ResponseName) { m_ResponseName = ResponseName; }

  //! Get the revan configuration file name
  MString GetRevanConfigurationFileName() const { return m_RevanConfigurationFileName; }
  //! Set the revan configuration file name
  void SetRevanConfigurationFileName(MString RevanConfigurationFileName) { m_RevanConfigurationFileName = RevanConfigurationFileName; }

  //! Get the mimrec configuration file name
  MString GetMimrecConfigurationFileName() const { return m_MimrecConfigurationFileName; }
  //! Set the mimrec configuration file name
  void SetMimrecConfigurationFileName(MString MimrecConfigurationFileName) { m_MimrecConfigurationFileName = MimrecConfigurationFileName; }
  
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

  static const unsigned int c_Spectrum             = 0;
  static const unsigned int c_Clustering           = 1;
  static const unsigned int c_BayesianER           = 2;
  static const unsigned int c_Efficiency           = 3;
  static const unsigned int c_Imaging              = 4;
  
  // protected methods:
 protected:

  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The operation mode
  unsigned int m_Mode;
  
  //! The response name
  MString m_ResponseName;
  
  //! The revan configuration file name
  MString m_RevanConfigurationFileName;
  //! The mimrec configuration file name
  MString m_MimrecConfigurationFileName;

  //! The response
  MResponseBuilder* m_Response;
  
#ifdef ___CLING___
 public:
  ClassDef(MModuleResponseGenerator, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
