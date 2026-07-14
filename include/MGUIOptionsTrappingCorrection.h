/*
 * MGUIOptionsTrappingCorrection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.:q
 *
 */


#ifndef __MGUIOptionsTrappingCorrection__
#define __MGUIOptionsTrappingCorrection__


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
class MGUIOptionsTrappingCorrection : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsTrappingCorrection(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsTrappingCorrection();

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
  MGUIEFileSelector* m_SimCCEFileSelector;


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsTrappingCorrection, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
