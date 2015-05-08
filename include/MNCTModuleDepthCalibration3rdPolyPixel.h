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

// Forward declarations:
#include "MModule.h"
#include "MGUIExpoDepthCalibration.h"

////////////////////////////////////////////////////////////////////////////////


class MNCTModuleDepthCalibration3rdPolyPixel : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleDepthCalibration3rdPolyPixel();
  //! Default destructor
  virtual ~MNCTModuleDepthCalibration3rdPolyPixel();
  
  //! Create a new object of this class 
  virtual MNCTModuleDepthCalibration3rdPolyPixel* Clone() { return new MNCTModuleDepthCalibration3rdPolyPixel(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

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
  //! The depth calibration UI 
  MGUIExpoDepthCalibration* m_ExpoDepthCalibration;

  bool m_IsCalibrationLoaded[12];
  bool m_IsCalibrationLoadedPixel[12][37][37];
  double m_Pixel_CTD2Depth[12][37][37][4];
  double m_Pixel_CTD_FWHM_Positive[12][37][37];
  double m_Pixel_CTD_FWHM_Negative[12][37][37];
  double m_Default_CTD2Depth[4];
  double m_Default_CTD_FWHM;
  unsigned long ShareHitNumber0,ShareHitNumber1, NotValidStripNumber, OutofBoundsDepth;
  unsigned long SingleHitNumber,OtherHitNumber, LLDNumber, InvalidEventNumber;
  unsigned long ShareEventNumber0,ShareEventNumber1, SingleEventNumber,OtherEventNumber;

#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleDepthCalibration3rdPolyPixel, 0) // no description
#endif

};

#endif


///////////////////////////////////////////////////////////////////////////////
