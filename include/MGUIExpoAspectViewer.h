/*
 * MGUIExpoAspectViewer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoAspectViewer__
#define __MGUIExpoAspectViewer__


////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <list>
using namespace std;

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
#include <TGraph.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MGUIERBList.h"

// NuSTAR libs
#include "MGUIExpo.h"

//Nuclearizer libs


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExpoAspectViewer : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoAspectViewer(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoAspectViewer();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Export the data in the UI
  virtual void Export(const MString& FileName);

  //! Add data to the heading histogram
  void AddHeading(MTime Time, double Heading);

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! Energy canvas
  TRootEmbeddedCanvas* m_HeadingCanvas;
  //! Energy histogram
  TGraph* m_Heading;

  //! The maximum time intervall to be kept (max-min) in seconds
  double m_TimeCutOff;

  //! Stored Times
  list<MTime> m_Times;
  //! Stored Headings
  list<double> m_Headings;


#ifdef ___CINT___
 public:
  ClassDef(MGUIExpoAspectViewer, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
