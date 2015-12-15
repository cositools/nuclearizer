/*
 * MNCTFileEventsDat.h
 *
 * Copyright (C) 2008-2008 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTFileEventsDat__
#define __MNCTFileEventsDat__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileEvents.h"
#include "MReadOutAssembly.h"
#include "MNCTEventBuffer.h"
#include "MDGeometryQuest.h"

// Standard libs
#include <deque>

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTFileEventsDat : public MFileEvents
{
  // public interface:
 public:
  MNCTFileEventsDat(MString GeometryFileName = "");
  virtual ~MNCTFileEventsDat();

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way);

  // The main code: return the next event in buffer
  MReadOutAssembly* GetNextEvent();
  MReadOutAssembly* GetNextEvent_buffers();
  MReadOutAssembly* GetNextEvent_singlebuffer();
  MReadOutAssembly* GetNextEvent_deque();

  // protected methods:
 protected:
  void Init();

  // private methods:
 private:
  //! The geometry file name
  MString m_GeometryFileName;

  // reads a single-detector event from file
  MReadOutAssembly* ReadNextEvent();

  // Updates event statistics with given event
  void UpdateEventStatistics(MReadOutAssembly* Event);

  // Prints a summary of all event statistics
  string EventStatisticsString();

  // load segment correction file (for syncing cardcages)
  bool LoadSegmentCorrectionFile();

  // find segment for an event
  bool FindSegment(MReadOutAssembly* E);

  // protected members:
 protected:

  // private members:
 private:
  int m_EventId;
  bool m_IsFirstEvent;
  bool m_FindCoincidences;
  unsigned long m_NGoodEventsInFile;
  unsigned long m_NStartEventsInFile;
  unsigned long m_NEventsByNDetectors[10];
  unsigned long m_NSingleDetectorEvents[10];
  unsigned long m_NMultipleDetectorEvents[10];

  // Event buffers
  // The following two are temporary until the multiple buffers are done being tested --MSB
  deque<MReadOutAssembly*> m_EventBuffer_deque;
  MNCTEventBuffer m_EventBuffer_single;
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

  // segment correction parameters (for syncing carcages mostly)
  vector< vector<double> > m_SegmentCorrectionTable;
  int m_Segment;
 
#ifdef ___CINT___
 public:
  ClassDef(MNCTFileEventsDat, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////