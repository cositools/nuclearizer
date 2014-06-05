/*
 * MNCTPreprocessor.h
 *
 * Copyright (C) 2008-2009 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTPreprocessor__
#define __MNCTPreprocessor__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

#include "MNCTFile.h"
#include "MNCTEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTPreprocessor
{
  // public interface:
 public:
  //! Default constructor
  MNCTPreprocessor(MNCTFile*);
  //! Default destructor
  ~MNCTPreprocessor();

  //! Initialize the module
  bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  MNCTEvent* GetNextEvent();

  // Updates event statistics with given event
  void UpdateEventStatistics(MNCTEvent* Event);

  // Prints a summary of all event statistics
  string EventStatisticsString();

  //! Show the options GUI
  void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  //bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  //MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:

  // private methods:
 private:
  // break apart event into events by detector
  vector<MNCTEvent*> EventByDetectors(MNCTEvent* Event);

  // protected members:
 protected:


  // private members:
 private:
  MNCTFile* m_File;

  bool m_EndOfFile;

  bool m_FindCoincidences;
  unsigned long m_NGoodEventsInFile;
  unsigned long m_NStartEventsInFile;
  unsigned long m_NEventsByNDetectors[10];
  unsigned long m_NSingleDetectorEvents[10];
  unsigned long m_NMultipleDetectorEvents[10];

  // Event buffers
  // Event buffer for multiple detectors
  MNCTEventBuffer m_EventBuffer[10];
  // Flags to tell whether we are doing the initial buffer fill:
  bool m_InitialRead;
  unsigned long m_InitialReadNEvents;
  bool m_DetectorActive[10];
  // Maximum size of buffers
  unsigned long m_EventBufferMaxSize;
  // Tolerance (in clock ticks, one tick is 100 ns) of coincidence matching
  double m_CoincidenceTolerance;

  // deal with the overflow problem (added by Jau-Shian Liang)
  unsigned long long m_tempCL;

#ifdef ___CINT___
 public:
  ClassDef(MNCTPreprocessor, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
