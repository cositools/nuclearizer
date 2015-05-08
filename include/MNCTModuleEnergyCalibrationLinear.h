/*
 * MNCTModuleEnergyCalibrationLinear.h
 *
 * Copyright (C) 2008-2008 by Mark Bandstra
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleEnergyCalibrationLinear__
#define __MNCTModuleEnergyCalibrationLinear__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleEnergyCalibrationLinear : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleEnergyCalibrationLinear();
  //! Default destructor
  virtual ~MNCTModuleEnergyCalibrationLinear();
  
  //! Create a new object of this class 
  virtual MNCTModuleEnergyCalibrationLinear* Clone() { return new MNCTModuleEnergyCalibrationLinear(); }

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
  bool m_IsCalibrationLoadedStrip[10][2][37];
  double m_CalibrationCoefficients[10][2][37][2];
  double m_FwhmCoefficients[10][2][37][3];
  double m_DefaultGain;
  double m_DefaultFwhm;

#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleEnergyCalibrationLinear, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
