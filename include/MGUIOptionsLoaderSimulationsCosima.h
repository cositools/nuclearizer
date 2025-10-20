/*
 * MGUIOptionsLoaderSimulationsCosima.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsLoaderSimulationsCosima__
#define __MGUIOptionsLoaderSimulationsCosima__


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
#include "MGUIEFileSelector.h"
#include "MGUIEEntry.h"
#include "MGUIOptions.h"

// Nuclearizer libs:
#include "MModule.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsLoaderSimulationsCosima : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsLoaderSimulationsCosima(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsLoaderSimulationsCosima();

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
  //! Select which file to load
  MGUIEFileSelector* m_SimulationFileSelector;
  //! Use stop after a maximum number of events
  TGCheckButton* m_StopAfter;
  //! Entry field for the maximum number of accepted events
  MGUIEEntry* m_MaximumAcceptedEvents;
  
  
  
#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsLoaderSimulationsCosima, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
