/*
 * MGUIOptionsLoaderMeasurementsFITS.h
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsLoaderMeasurementsFITS__
#define __MGUIOptionsLoaderMeasurementsFITS__


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


//! UI settings for the FITS measurements loader
class MGUIOptionsLoaderMeasurementsFITS : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsLoaderMeasurementsFITS(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsLoaderMeasurementsFITS();

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
  //! Select which FITS file to load
  MGUIEFileSelector* m_FileSelectorFITS;



#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsLoaderMeasurementsFITS, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
