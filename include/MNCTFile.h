/*
 * MNCTFile.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTFile__
#define __MNCTFile__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
using namespace std;

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"
#include "MDGeometryQuest.h"
#include "MFileEventsSim.h"
#include "MNCTFileEventsDat.h"
#include "MNCTEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTFile : public MFileEvents
{
  // public interface:
 public:
  //! Default constructor
  MNCTFile(MString GeometryFileName = "");
  //! Default destructor
  virtual ~MNCTFile();

  //! Open the file 
  virtual bool Open(MString FileName, unsigned int Way, int HighestAnalysisLevel = 0);
  //! Close the file
  virtual bool Close();

  //! 
  bool IsReadSim(){return m_ReadSimFile;}

  //! Return the next event or zero is ther isn't any  
  MNCTEvent* GetNextEvent();

  //! Write an event
  bool Write(MNCTEvent* Event, int AnalysisLevel);

  //! Returns true if the progress dialog is shown
  bool IsShowProgress();
  //! Show the progress dialog GUI
  virtual void ShowProgress(bool Show = true);
  //! Update the progress dialog GUI
  virtual bool UpdateProgress();
  //! Set the titles of the progress dialog GUI
  void SetProgressTitle(MString Main, MString Sub);
  //! Use another progress dialog GUI instead of this one
  void SetProgress(MGUIProgressBar* ProgressBar, int Level);

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The geometry file name
  MString m_GeometryFileName;
  //! The MEGAlib geometry description
  MDGeometryQuest* m_Geometry;

  //! Flag to indicate that the sim file is in use
  bool m_ReadSimFile;
  //! The sim file
  MFileEventsSim m_SimFile;

  //! Flag to indicate that the dat file is in use
  bool m_ReadDatFile;
  //! The dat file
  MNCTFileEventsDat m_DatFile;

  

#ifdef ___CINT___
 public:
  ClassDef(MNCTFile, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
