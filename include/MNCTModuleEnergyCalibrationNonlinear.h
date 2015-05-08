/*
 * MNCTModuleEnergyCalibrationNonlinear.h
 *
 * Copyright (C) 2009-2009 by Mark Bandstra
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleEnergyCalibrationNonlinear__
#define __MNCTModuleEnergyCalibrationNonlinear__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleEnergyCalibrationNonlinear : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleEnergyCalibrationNonlinear();
  //! Default destructor
  virtual ~MNCTModuleEnergyCalibrationNonlinear();
  
  //! Create a new object of this class 
  virtual MNCTModuleEnergyCalibrationNonlinear* Clone() { return new MNCTModuleEnergyCalibrationNonlinear(); }

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
  double m_CalibrationCoefficients[10][2][37][5];
  double m_FwhmCoefficients[10][2][37][3];
  double m_DefaultGain;
  double m_DefaultFwhm;

#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleEnergyCalibrationNonlinear, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
