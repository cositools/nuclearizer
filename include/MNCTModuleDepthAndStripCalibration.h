/*
 * MNCTModuleDepthAndStripCalibration.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleDepthAndStripCalibration__
#define __MNCTModuleDepthAndStripCalibration__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MNCTModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleDepthAndStripCalibration : public MNCTModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleDepthAndStripCalibration();
  //! Default destructor
  virtual ~MNCTModuleDepthAndStripCalibration();

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




#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleDepthAndStripCalibration, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
