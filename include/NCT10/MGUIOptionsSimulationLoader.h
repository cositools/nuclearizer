/*
 * MGUIOptionsSimulationLoader.h
 *
 * Copyright (C) by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsSimulationLoader__
#define __MGUIOptionsSimulationLoader__


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
#include <TGNumberEntry.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIERBList.h"
#include "MModule.h"
#include "MGUIOptions.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsSimulationLoader : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsSimulationLoader(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsSimulationLoader();

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
  //! Select which file to load
  MGUIEFileSelector* m_FileSelector;

  // DEE options
  TGNumberEntry* m_TimeOffset0NumEntry;
  TGNumberEntry* m_TimeOffsetNumEntry;
  TGCheckButton* m_DeadStripButton;
  TGCheckButton* m_CoincidenceButton;
  TGCheckButton* m_AntiCoincidenceButton;
  TGCheckButton* m_ChargeSharingButton;
  TGCheckButton* m_CrosstalkButton;
  TGCheckButton* m_NonlinearGainButton;
  TGCheckButton* m_KeepLLDOnlyButton;
  TGCheckButton* m_VerboseButton;
	
  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsSimulationLoader, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
