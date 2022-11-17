/*
 * MGUIOptionsDepthCalibrationB.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsDepthCalibrationB__
#define __MGUIOptionsDepthCalibrationB__


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


class MGUIOptionsDepthCalibrationB : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsDepthCalibrationB(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsDepthCalibrationB();

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
  //! Select which file to load the lookup tables from
  MGUIEFileSelector* m_FileSelector;



#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsDepthCalibrationB, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
