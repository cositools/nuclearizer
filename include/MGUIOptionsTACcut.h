/*
 * MGUIOptionsTACcut.h
 *
 * Copyright (C) 2008-2010 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsTACcut__
#define __MGUIOptionsTACcut__


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
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIERBList.h"
#include "MModule.h"
#include "MGUIOptions.h"

// Forward declarations:
class MGUIEFileSelector;
class MGUIEMinMaxEntry;

////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsTACcut : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsTACcut(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsTACcut();

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
  //! The detector IDs as a string
  TGTextEntry* m_Detectors;
  //! The total TAC selection
  MGUIEMinMaxEntry* m_TAC;

  //! Select TAC Calibration file to load, converts readout timing to nanoseconds
  MGUIEFileSelector* m_TACCalFileSelector;

  TGCheckButton* m_SingleSiteOnly;
	
  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsTACcut, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
