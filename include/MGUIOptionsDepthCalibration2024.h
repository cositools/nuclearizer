/*
 * MGUIOptionsDepthCalibration.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsDepthCalibration2024__
#define __MGUIOptionsDepthCalibration2024__


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


class MGUIOptionsDepthCalibration2024 : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsDepthCalibration2024(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsDepthCalibration2024();

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
  //! Select which coefficients file (stretching factors and offsets) to load
  MGUIEFileSelector* m_CoeffsFileSelector;

  //! Select spline file to load, splines will convert CTD->Depth
  MGUIEFileSelector* m_SplinesFileSelector;

  //! Check button if working with the Card Cage at UCSD
  TGCheckButton* m_UCSDOverride;


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsDepthCalibration2024, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
