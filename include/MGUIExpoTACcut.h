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

  //! Set the arrangment of the TAC histogram
  //!  0    1    2    3 
  //!  4    5    6    7
  //!  8    9   10   11
  void SetTACHistogramArrangement(const vector<unsigned int> DetIDs);
  
  //! Set the energy histogram parameters 
  void SetTACHistogramParameters(unsigned int DetID, unsigned int NBins, double TACMin, double TACMax);
  
  //! Set the energy histogram parameters 
  void SetTACHistogramName(unsigned int DetID, MString Name);

  //! Add data to the TAC histogram
  //!  0    1    2    3 
  //!  4    5    6    7
  //!  8    9   10   11
  void AddTAC(unsigned int DetID, double TAC);

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! TAC canvas
  unordered_map<unsigned int, TRootEmbeddedCanvas*> m_TACCanvases;
  //! TAC vs detector ID histogram
  unordered_map<unsigned int, TH1D*> m_TACHistograms;

  //! Detectors in x direction
  unsigned int m_NColumns;
  //! Detectors in y direction
  unsigned int m_NRows;

  // Map the detector ID to the x,y position of histograms.
  vector<vector<unsigned int>> m_DetectorMap;
  
  //! The number of bins of the histogram
  unordered_map<unsigned int, unsigned int> m_NBins;
  //! The minimum TAC
  unordered_map<unsigned int, double> m_Min;
  //! The maximum TAC
  unordered_map<unsigned int, double> m_Max;
  

#ifdef ___CLING___
 public:
  ClassDef(MGUIExpoTACcut, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
