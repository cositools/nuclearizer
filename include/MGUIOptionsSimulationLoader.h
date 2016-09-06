/*
 * MGUIOptionsSimulationLoader.h
 *
 * Copyright (C) by Andreas Zoglauer.
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

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIEFileSelector.h"
#include "MGUIOptions.h"

// Nuclearizer libs:
#include "MModule.h"


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

  // private members:
 private:
  //! Select which file to load
  MGUIEFileSelector* m_SimulationFileSelector;
  //! Energy calibration file name
  MGUIEFileSelector* m_EnergyCalibrationFileSelector;
  //! Dead strip file name
  MGUIEFileSelector* m_DeadStripFileSelector;
  //! Thresholds file name
  MGUIEFileSelector* m_ThresholdFileSelector;
  //! Depth calibration coefficients file name
  MGUIEFileSelector* m_DepthCalibrationCoeffsFileSelector;
  //! Depth calibration splines file name
  MGUIEFileSelector* m_DepthCalibrationSplinesFileSelector;

  
#ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsSimulationLoader, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////