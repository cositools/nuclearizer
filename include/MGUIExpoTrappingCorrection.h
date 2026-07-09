/*
 * MGUIExpoTrappingCorrection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoTrappingCorrection__
#define __MGUIExpoTrappingCorrection__


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


class MGUIExpoTrappingCorrection : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoTrappingCorrection(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoTrappingCorrection();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Export the data in the UI
  virtual void Export(const MString& FileName);

  //! Set the energy histogram parameters 
  void SetEnergyHistogramParameters(int NBins, double Min, double Max);

  //! Add data to the energy histogram
  void AddEnergy(double Energy);

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! Energy canvas
  TRootEmbeddedCanvas* m_EnergyCanvas;
  //! Energy histogram
  TH1D* m_Energy;



#ifdef ___CLING___
 public:
  ClassDef(MGUIExpoTrappingCorrection, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
