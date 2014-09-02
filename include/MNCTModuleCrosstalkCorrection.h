/*
 * MNCTModuleCrosstalkCorrection.h
 *
 * Copyright (C) 2009-2009 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

#define MAXNSKIP 3

#ifndef __MNCTModuleCrosstalkCorrection__
#define __MNCTModuleCrosstalkCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleCrosstalkCorrection : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleCrosstalkCorrection();
  //! Default destructor
  virtual ~MNCTModuleCrosstalkCorrection();

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
  // Method to make the cross-talk correction on a vector of strip hits
  virtual void CorrectCrosstalk(vector<MNCTStripHit*> StripHits, int det, unsigned int side);


  // protected members:
 protected:


  // private members:
 private:
  bool m_IsCalibrationLoadedDet[10];
  bool m_IsCalibrationLoaded[10][2][MAXNSKIP+1];
  double m_CrosstalkCoeffs[10][2][MAXNSKIP+1][2];

#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleCrosstalkCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
