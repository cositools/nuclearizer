/*
 * MGUIExpoStripPairing.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoStripPairing__
#define __MGUIExpoStripPairing__


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


class MGUIExpoStripPairing : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoStripPairing(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoStripPairing();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Print the data in the UI
  virtual void Print(const MString& FileName);

  //! Set the energy histogram parameters 
  void SetEnergiesHistogramParameters(int NBins, double Min, double Max);

  //! Add data to the energy histogram
  void AddEnergies(double pEnergy, double nEnergy);

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! Energy canvas
  TRootEmbeddedCanvas* m_EnergiesCanvas;
  //! Energy histogram
  TH2D* m_Energies;



#ifdef ___CINT___
 public:
  ClassDef(MGUIExpoStripPairing, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
