/*
 * MNCTInverseCrosstalkCorrection.h
 *
 * Copyright (C) 2009-2009 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

#define MAXNSKIP 3

#ifndef __MNCTInverseCrosstalkCorrection__
#define __MNCTInverseCrosstalkCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MNCTEvent.h"
#include "MNCTStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTInverseCrosstalkCorrection
{
  // public interface:
 public:
  //! Default constructor
  MNCTInverseCrosstalkCorrection();
  //! Default destructor
  virtual ~MNCTInverseCrosstalkCorrection();

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  //virtual bool AnalyzeEvent(MNCTEvent* Event);
  
  // Method to make the cross-talk correction on a vector of strip hits
  virtual void ApplyCrosstalk(vector<MNCTStripHit*> StripHits, int det, unsigned int side);

  // protected methods:
 protected:

  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  bool m_IsCalibrationLoadedDet[10];
  bool m_IsCalibrationLoaded[10][2][MAXNSKIP+1];
  double m_CrosstalkCoeffs[10][2][MAXNSKIP+1][2];

#ifdef ___CINT___
 public:
  ClassDef(MNCTInverseCrosstalkCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
