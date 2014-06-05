/*
 * MGUIOptionsEventReconstruction.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsEventReconstruction__
#define __MGUIOptionsEventReconstruction__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <MString.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIERBList.h"
#include "MNCTModule.h"
#include "MGUIOptions.h"

// Forward declarations:
class MGUIEFileSelector;


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsEventReconstruction : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsEventReconstruction(MNCTModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsEventReconstruction();

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
  //! GUI element storing the current load file name
  MGUIEFileSelector* m_FileSelector;


#ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsEventReconstruction, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
