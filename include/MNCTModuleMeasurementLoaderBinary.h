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
#include "zlib.h"

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MModule.h"
#include "MNCTBinaryFlightDataParser.h"
#include "MGUIExpoAspectViewer.h"

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
  
  //! Create a new object of this class 
  virtual MNCTModuleMeasurementLoaderBinary* Clone() { return new MNCTModuleMeasurementLoaderBinary(); }
   
  //! Get the file name
  MString GetFileName() const { return m_FileName; }
  //! Set the file name
  void SetFileName(const MString& Name) { m_FileName = Name; }
 
  //! Return if the module is ready to analyze events
  virtual bool IsReady();
  
  //! Create the expos
  virtual void CreateExpos();
  
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

  //! Open next file, return false on error
  bool OpenNextFile();
  
  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! A GUI to display the aspect data 
  MGUIExpoAspectViewer* m_ExpoAspectViewer;

  //! The file name
  MString m_FileName;
  //! True if the current fiule is gzip'ed
  bool m_IsZipped;
  //! The current binary data stream uncompressed
  ifstream m_In;
  //! The basic file stream for zlib
  gzFile m_ZipFile;
  //! A list of all binary data files
  vector<MString> m_BinaryFileNames;
  //! The currently open binary file name (-1 none is open)
  int m_OpenFileID;
  //! Flag indicating that file read is over
  bool m_FileIsDone;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MNCTModuleMeasurementLoaderBinary, 0) // no description
#endif

};

#endif



////////////////////////////////////////////////////////////////////////////////
