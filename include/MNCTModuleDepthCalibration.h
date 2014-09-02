/*
 * MNCTModuleDepthCalibration.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleDepthCalibration__
#define __MNCTModuleDepthCalibration__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleDepthCalibration : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleDepthCalibration();
  //! Default destructor
  virtual ~MNCTModuleDepthCalibration();

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
  ClassDef(MNCTModuleDepthCalibration, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
