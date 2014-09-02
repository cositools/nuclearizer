/*
 * MGUIExpoDepthCalibration.h
 *    
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoDepthCalibration__
#define __MGUIExpoDepthCalibration__


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


class MGUIExpoDepthCalibration : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoDepthCalibration(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoDepthCalibration();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Print the data in the UI
  virtual void Print(const MString& FileName);

  //! Set the arrangment of the depth histogram
  //!  0    1    2    3 
  //!  4    5    6    7
  //!  8    9   10   11
  void SetDepthHistogramArrangement(unsigned int NDetectorsX, unsigned int NDetectorsY);
  
  //! Set the energy histogram parameters 
  void SetDepthHistogramParameters(unsigned int NBins, double DepthMin, double DepthMax);
  
  //! Set the energy histogram parameters 
  void SetDepthHistogramName(unsigned int Detector, MString Name);

  //! Add data to the depth histogram
  //!  0    1    2    3 
  //!  4    5    6    7
  //!  8    9   10   11
  void AddDepth(unsigned int Detector, double Depth);

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! Depth canvas
  vector<TRootEmbeddedCanvas*> m_DepthCanvases;
  //! Depth vs detector ID histogram
  vector<TH1D*> m_DepthHistograms;

  //! Detectors in x direction
  unsigned int m_NDetectorsX;
  //! Detectors in y direction
  unsigned int m_NDetectorsY;
  
  //! The number of bins of the histogram
  int m_NBins;
  //! The minimum depth
  double m_Min;
  //! The maximum depth
  double m_Max;
  

#ifdef ___CINT___
 public:
  ClassDef(MGUIExpoDepthCalibration, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
