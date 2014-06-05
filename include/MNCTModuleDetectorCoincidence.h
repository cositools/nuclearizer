/*
 * MNCTModuleDetectorCoincidence.h
 *
 * Copyright (C) 2009-2009 by Mark Bandstra
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleDetectorCoincidence__
#define __MNCTModuleDetectorCoincidence__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MNCTModule.h"
#include "MNCTEvent.h"
#include "MNCTEventBuffer.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleDetectorCoincidence : public MNCTModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleDetectorCoincidence();
  //! Default destructor
  virtual ~MNCTModuleDetectorCoincidence();

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MNCTEvent* Event);

  //! Show the options GUI
  //virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  //virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  //virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:

  // Updates event statistics with given event
  void UpdateEventStatistics(MNCTEvent* Event);

  // Prints a summary of all event statistics
  string EventStatisticsString();


  // private members:
 private:
  // Event statistics
  unsigned long m_NGoodEventsInFile;
  unsigned long m_NStartEventsInFile;
  unsigned long m_NEventsByNDetectors[10];
  unsigned long m_NSingleDetectorEvents[10];
  unsigned long m_NMultipleDetectorEvents[10];




#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleDetectorCoincidence, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
