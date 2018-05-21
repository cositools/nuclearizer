/*
 * MGUIOptionsAspect.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsAspect__
#define __MGUIOptionsAspect__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGButtonGroup.h>
#include <MString.h>
#include <TGClient.h>
#include <TGNumberEntry.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIERBList.h"
#include "MModule.h"
#include "MGUIOptions.h"

// Forward declarations:
class MGUIEFileSelector;

////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsAspect : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsAspect(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsAspect();

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
  //
  MGUIEFileSelector* m_FileSelector;
  TGNumberEntry* m_TimeZeroNumEntry;
  TGNumberEntry* m_AspectDelayNumEntry;
  TGButtonGroup* m_CoordinateButtonGroup;
  TGCheckButton* m_RunTimeCorrectionButton;
  TGCheckButton* m_VerboseButton;
	
  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsAspect, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
