/*
 * MGUIOptionsTACCalibration.h
 *
 * Copyright (C) 2008-2010 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsTACCalibration__
#define __MGUIOptionsTACCalibration__


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
class MGUIEEntry;

////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsTACCalibration : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsTACCalibration(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsTACCalibration();

  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  //! The creation part which gets overwritten
  virtual void Create();


  // protected methods:
 protected:

  //! Actions after the Apply or OK button has been pressed
  virtual bool OnApply();

  //! Toggle TAC cut radio buttons and coincidence window entry
  void ToggleRadioButtons(int WidgetID);

  //! Widget IDs
  enum {
  c_TACCutIgnore = 120,
  c_TACCutApply = 121
  };
  
  // protected members:
 protected:
  //! The detector IDs as a string
  TGTextEntry* m_Detectors;

  //! Select TAC Calibration file to load, converts readout timing to nanoseconds
  MGUIEFileSelector* m_TACCalFileSelector;

  //! Do not apply TAC cuts
  TGRadioButton* m_TACCutRBIgnore;

  //! Apply TAC cuts
  TGRadioButton* m_TACCutRBApply;

  //! TAC coincidence window in ns
  MGUIEEntry* m_CoincidenceWindow;
	
  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsTACCalibration, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
