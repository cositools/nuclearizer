/*
 * MGUIOptionsEventSaver.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsEventSaver__
#define __MGUIOptionsEventSaver__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"
#include "MGUIERBList.h"
#include "MGUIOptions.h"

// Nuclearizer libs:
#include "MModule.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! UI for the event saver
class MGUIOptionsEventSaver : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsEventSaver(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsEventSaver();

  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  //! The creation part which gets overwritten
  virtual void Create();

  // protected methods:
 protected:

  //! Actions after the Apply or OK button has been pressed
	virtual bool OnApply();


  // protected members:
 protected:

  // private members:
 private:
  //! Select the mode
  MGUIERBList* m_Mode;
  //! Select which file to load
  MGUIEFileSelector* m_FileSelector;

  //! Checkbutton to save or reject bad events
  TGCheckButton* m_SaveBadEvents;

  //! Checkbutton to save veto events
  TGCheckButton* m_SaveVetoEvents;

  //! Checkbutton to add a time tag
  TGCheckButton* m_AddTimeTag;

  //! Checkbutton to split or not to split the file 
  TGCheckButton* m_SplitFile;
  //! Entry field for the time after which to split the file
  MGUIEEntry* m_SplitFileTime;
    
  //! Checkbutton to include or exclude ADCs in the roa file
  TGCheckButton* m_RoaWithADCs;
  //! Checkbutton to include or exclude TACs in the roa file
  TGCheckButton* m_RoaWithTACs;
  //! Checkbutton to include or exclude energies in the roa file
  TGCheckButton* m_RoaWithEnergies;
  //! Checkbutton to include or exclude timings in the roa file
  TGCheckButton* m_RoaWithTimings;
  //! Checkbutton to include or exclude temperatures in the roa file
  TGCheckButton* m_RoaWithTemperatures;
  //! Checkbutton to include or exclude flags in the roa file
  TGCheckButton* m_RoaWithFlags;
  //! Checkbutton to include or exclude origins in the roa file
  TGCheckButton* m_RoaWithOrigins;
  //! Checkbutton to include or exclude nearest neighbor hits in the roa file
  TGCheckButton* m_RoaWithNearestNeighbors;

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsEventSaver, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
