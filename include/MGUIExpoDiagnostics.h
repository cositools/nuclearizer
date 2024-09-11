/*
 * MGUIExpoDiagnostics.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoDiagnostics__
#define __MGUIExpoDiagnostics__


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

// Nuclearizer libs
#include "MGUIExpo.h"
#include "MStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExpoDiagnostics : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoDiagnostics(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoDiagnostics();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Export the data in the UI
  virtual void Export(const MString& FileName);

  //! Set the energy histogram parameters 
  void SetStripHitHistogramParameters(int NBinsIDs, double MinIDs, double MaxIDs, int NBinsEnergy, double MinEnergy, double MaxEnergy);

  //! Add strip data
  void AddStripHit(MStripHit* SH);


  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! Energy canvas
  TRootEmbeddedCanvas* m_DiagnosticsCanvas;
  //! Low-voltage strip hit histogram
  TH2D* m_StripHitsLV;
  //! High-voltage strip hit histogram
  TH2D* m_StripHitsHV;



#ifdef ___CLING___
 public:
  ClassDef(MGUIExpoDiagnostics, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
