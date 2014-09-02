/*
 * MNCTModuleMeasurementLoaderBinary.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleMeasurementLoaderBinary__
#define __MNCTModuleMeasurementLoaderBinary__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <list>
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MModule.h"
#include "MNCTBinaryFlightDataParser.h"

// Forward declarations:

  
////////////////////////////////////////////////////////////////////////////////


class MNCTModuleMeasurementLoaderBinary : public MModule, public MNCTBinaryFlightDataParser
{
  // public interface:
 public:
   
  //! Default constructor
  MNCTModuleMeasurementLoaderBinary();
  //! Default destructor
  virtual ~MNCTModuleMeasurementLoaderBinary();
   
  //! Get the file name
  MString GetFileName() const { return m_FileName; }
  //! Set the file name
  void SetFileName(const MString& Name) { m_FileName = Name; }
 
  //! Return if the module is ready to analyze events
  virtual bool IsReady();
  
  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module --- can be overwritten
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
  //! Perform Handshake
  bool DoHandshake();

  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! The file name
  MString m_FileName;
  //! The file stream
  ifstream m_In;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleMeasurementLoaderBinary, 0) // no description
#endif

};

#endif



////////////////////////////////////////////////////////////////////////////////
