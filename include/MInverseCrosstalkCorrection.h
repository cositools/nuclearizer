/*
 * MInverseCrosstalkCorrection.h
 *
 * Copyright (C) 2009-2009 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

#define MAXNSKIP 3

#ifndef __MInverseCrosstalkCorrection__
#define __MInverseCrosstalkCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MInverseCrosstalkCorrection
{
  // public interface:
 public:
  //! Default constructor
  MInverseCrosstalkCorrection();
  //! Default destructor
  virtual ~MInverseCrosstalkCorrection();

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  //virtual bool AnalyzeEvent(MReadOutAssembly* Event);
  
  // Method to make the cross-talk correction on a vector of strip hits
  virtual void ApplyCrosstalk(vector<MStripHit*> StripHits, int det, unsigned int side);

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

#ifdef ___CLING___
 public:
  ClassDef(MInverseCrosstalkCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
