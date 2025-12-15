/*
 * MGUIExpoStripPairingHits.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoStripPairingHits__
#define __MGUIExpoStripPairingHits__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TString.h>
#include <TGClient.h>
#include <TRootEmbeddedCanvas.h>
#include <TH1.h>
#include <TH2.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIERBList.h"

// NuSTAR libs
#include "MGUIExpo.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExpoStripPairingHits : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoStripPairingHits(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoStripPairingHits();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Export the data in the UI
  virtual void Export(const MString& FileName);

  //! Set the hits histogram parameters 
  void SetHitsHistogramParameters(int NBins, double Min, double Max);

  //! Add data to the hits histogram
  void AddHits(int NHits);

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! Hits canvas
  TRootEmbeddedCanvas* m_HitsCanvas;
  //! Hits histogram
  TH1D* m_Hits;


#ifdef ___CLING___
 public:
  ClassDef(MGUIExpoStripPairingHits, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
