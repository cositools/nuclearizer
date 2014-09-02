/*
 * MNCTModuleDepthCalibrationLinearPixel.h
 *
 * Copyright (C) 2008-2008 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleDepthCalibrationLinearPixel__
#define __MNCTModuleDepthCalibrationLinearPixel__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleDepthCalibrationLinearPixel : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleDepthCalibrationLinearPixel();
  //! Default destructor
  virtual ~MNCTModuleDepthCalibrationLinearPixel();

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
  bool m_IsCalibrationLoaded[10];
  bool m_IsCalibrationLoadedPixel[10][37][37];
  double m_Pixel_CTD_Front[10][37][37];
  double m_Pixel_CTD_Back[10][37][37];
  double m_Pixel_CTD_FWHM_Front[10][37][37];
  double m_Pixel_CTD_FWHM_Back[10][37][37];
  double m_Default_CTD_Negative;
  double m_Default_CTD_Positive;
  double m_Default_CTD_FWHM;


#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleDepthCalibrationLinearPixel, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
