/*
 * MGUIExpo.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpo__
#define __MGUIExpo__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TString.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGUIERBList.h"
#include "MTimer.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExpo : public TGCompositeFrame
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpo();
  //! Default destructor
  virtual ~MGUIExpo();

  //! Close the window
  void CloseWindow();
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  
  //! The creation part which gets overwritten
  virtual void Create() {};

  //! Update the frame - should be overwritten to refresh the histograms, etc.
  virtual void Update() { };

  //! Reset the data in the UI - should be overwritten to clear the histograms, etc.
  virtual void Reset() { };

  //! Get the title
  TString GetTabTitle() { return m_TabTitle; }

  //! Return true if we need an update
  bool NeedsUpdate() { return m_NeedsUpdate; }
  
  // protected methods:
 protected:


  // protected members:
 protected:
  //! Tab Title of the GUI element
  TString m_TabTitle;
  
  //! Flag to indicate we need an update
  bool m_NeedsUpdate;
  
  // private members:
 private:

#ifdef ___CINT___
 public:
  ClassDef(MGUIExpo, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
