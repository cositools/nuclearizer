/*
 * MNCTModuleEnergyCalibration.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleEnergyCalibration__
#define __MNCTModuleEnergyCalibration__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleEnergyCalibration : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleEnergyCalibration();
  //! Default destructor
  virtual ~MNCTModuleEnergyCalibration();

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




#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleEnergyCalibration, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
