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
#include <string>
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
#include <TGLabel.h>

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
  void AddHeading(MTime Time, double Heading, int GPS_or_magnetometer, double BRMS, uint16_t AttFlag);

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! Label
  TGLabel* m_Label;
  //! Energy canvas
  TRootEmbeddedCanvas* m_HeadingCanvas;
  //! GPS graph
  TGraph* m_Heading_GPS;
  // Magnetometer graph
  TGraph* m_Heading_Mag;

  //! The maximum time intervall to be kept (max-min) in seconds
  double m_TimeCutOff;
  
  //! Tracks if it is time to update the label
  int m_Update;

  //! Stored Time&Heading (GPS)
  list<MTime> m_Times_GPS;
  //! 
  list<double> m_Headings_GPS;
  //! Stored Time&Heading (Mag)
  list<MTime> m_Times_Mag;
  //!
  list<double> m_Headings_Mag;
  //! Stored Labels
  list<string> m_Labels;
  



#ifdef ___CINT___
 public:
  ClassDef(MGUIExpoAspectViewer, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
