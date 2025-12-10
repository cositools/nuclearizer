/*
 * MGUIExpoDiagnosticsEnergyPerStrip.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoDiagnosticsEnergyPerStrip__
#define __MGUIExpoDiagnosticsEnergyPerStrip__


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
#include "MHit.h"
#include "MStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExpoDiagnosticsEnergyPerStrip : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoDiagnosticsEnergyPerStrip(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoDiagnosticsEnergyPerStrip();

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

  //! Add hit data
  void AddHit(MHit* Hit);

  //! Add strip data
  void AddStripHit(MStripHit* StripHit, bool UseEnergy);


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
  ClassDef(MGUIExpoDiagnosticsEnergyPerStrip, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
