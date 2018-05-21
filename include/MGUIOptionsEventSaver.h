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

  //! Checkbutton to add a time tag
  TGCheckButton* m_AddTimeTag;

  //! Checkbutton to split or not to split the file 
  TGCheckButton* m_SplitFile;
  //! Entry field for the time after which to split the file
  MGUIEEntry* m_SplitFileTime;

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsEventSaver, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
