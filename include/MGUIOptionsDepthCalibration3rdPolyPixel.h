/*
 * MGUIOptionsDepthCalibration3rdPolyPixel.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsDepthCalibration3rdPolyPixel__
#define __MGUIOptionsDepthCalibration3rdPolyPixel__


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


class MGUIOptionsDepthCalibration3rdPolyPixel : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsDepthCalibration3rdPolyPixel(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsDepthCalibration3rdPolyPixel();

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


#ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsDepthCalibration3rdPolyPixel, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
