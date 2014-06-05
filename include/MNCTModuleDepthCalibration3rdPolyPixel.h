/*
 * MNCTModuleDepthCalibrationLinearPixel.h
 *
 * Copyright (C) 2008-2008 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleDepthCalibration3rdPolyPixel__
#define __MNCTModuleDepthCalibration3rdPolyPixel__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MNCTModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleDepthCalibration3rdPolyPixel : public MNCTModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleDepthCalibration3rdPolyPixel();
  //! Default destructor
  virtual ~MNCTModuleDepthCalibration3rdPolyPixel();

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MNCTEvent* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  bool m_IsCalibrationLoaded[10];
  bool m_IsCalibrationLoadedPixel[10][37][37];
  double m_Pixel_CTD2Depth[10][37][37][4];
  double m_Pixel_CTD_FWHM_Positive[10][37][37];
  double m_Pixel_CTD_FWHM_Negative[10][37][37];
  double m_Default_CTD2Depth[4];
  double m_Default_CTD_FWHM;
  unsigned long ShareHitNumber0,ShareHitNumber1;
  unsigned long SingleHitNumber,OtherHitNumber;  
  unsigned long ShareEventNumber0,ShareEventNumber1, SingleEventNumber,OtherEventNumber;

#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleDepthCalibration3rdPolyPixel, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
