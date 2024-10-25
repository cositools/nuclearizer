/*
 * MGUIExpoTACcut.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoTACcut__
#define __MGUIExpoTACcut__


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


class MGUIExpoTACcut : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoTACcut(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoTACcut();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Export the data in the UI
  virtual void Export(const MString& FileName);

  //! Set the TAC histogram parameters 
  void SetTACHistogramParameters(int NBins, double Min, double Max);

  //! Add data to the TAC histogram
  void AddTAC(double TAC);

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! TAC canvas
  TRootEmbeddedCanvas* m_TACCanvas;
  //! TAC histogram
  TH1D* m_TAC;



#ifdef ___CLING___
 public:
  ClassDef(MGUIExpoTACcut, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
