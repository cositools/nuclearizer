/*
 * MGUIOptionsRevan.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsRevan__
#define __MGUIOptionsRevan__


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


class MGUIOptionsRevan : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsRevan(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsRevan();

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
  //! Select the revan configuration file
  MGUIEFileSelector* m_RevanCfgFileSelector;


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsRevan, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
