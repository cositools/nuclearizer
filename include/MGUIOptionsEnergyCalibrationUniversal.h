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


  // protected members:
 protected:

  // private members:
 private:


  //! Select which file to load
  MGUIEFileSelector* m_FileSelector;

  //! Select Threshold File to use
  bool m_UseThresholdFile;
  TGCheckButton* m_ThresholdFileCB;
  MGUIEFileSelector* m_ThresholdFile;

  //! Use Threshold cut
  bool m_UseThresholdValue;
  //! Check button to decide to use a threshold cut
  TGCheckButton* m_ThresholdValueCB;
  //! The threshold entry box
  MGUIEEntry* m_SetThresholdValue;

  //! IDs of check buttons
  enum ButtonIDs { c_ThresholdFile = 1, c_Threshold };

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsEnergyCalibrationUniversal, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
