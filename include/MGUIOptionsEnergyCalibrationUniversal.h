/*
 * MGUIOptionsEnergyCalibrationUniversal.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.:q
 *
 */


#ifndef __MGUIOptionsEnergyCalibrationUniversal__
#define __MGUIOptionsEnergyCalibrationUniversal__


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
#include "MGUIOptions.h"
#include "MGUIERBList.h"
#include "MGUIEEntry.h"

// Nuclearizer libs:
#include "MModule.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The user interface for the universal energy calibration
class MGUIOptionsEnergyCalibrationUniversal : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsEnergyCalibrationUniversal(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsEnergyCalibrationUniversal();

  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  //! The creation part which gets overwritten
  virtual void Create();

  // protected methods:
 protected:

  //! Actions after the Apply or OK button has been pressed
  virtual bool OnApply();

  //! Toggle the radio buttons
  void ToggleRadioButtons(int WidgetID);


  // protected members:
 protected:

  // private members:
 private:

  //! Select which file to load
  MGUIEFileSelector* m_FileSelector;

  //! Radio button for ignoring the slow threshold cut
  TGRadioButton* m_SlowThresholdCutRBIgnore;

  //! Radio button for using a fixed slow threshold cut
  TGRadioButton* m_SlowThresholdCutRBFixed;
  //! The slow threshold cut entry box
  MGUIEEntry* m_SlowThresholdCutFixedValue;

  //! Radio button for reading slow threshold cuts from a file
  TGRadioButton* m_SlowThresholdCutRBFile;
  //! The slow threshold cut file selector
  MGUIEFileSelector* m_SlowThresholdCutFileSelector;


  //! IDs of radio buttons
  //! The numbers should be identical to MSlowThresholdCutModes
  enum RBButtonIDs { c_SlowThresholdIgnore = 0, c_SlowThresholdFixed, c_SlowThresholdFile };

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsEnergyCalibrationUniversal, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
