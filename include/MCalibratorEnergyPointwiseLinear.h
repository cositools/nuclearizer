/*
 * MCalibratorEnergyPointwiseLinear.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibratorEnergyPointwiseLinear__
#define __MCalibratorEnergyPointwiseLinear__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFunction.h"

// Nuclearizer libs:
#include "MCalibratorEnergy.h"

// Forward declarations:
class MReadOutElement;

////////////////////////////////////////////////////////////////////////////////


class MCalibratorEnergyPointwiseLinear : public MCalibratorEnergy
{
  // public interface:
 public:
   //! An energy calibrator where
  MCalibratorEnergyPointwiseLinear();
  //! Default destructor
  virtual ~MCalibratorEnergyPointwiseLinear();

  //! Return the energy associated with the given ADC units
  double GetEnergy(double ADCUnits);

  //! Add a new data point
  bool Add(double ADCUnits, double Energy);

  //! Dump a string about the class
  virtual MString ToString() const;
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! The function which converts from ADC units to energy
  MFunction m_ADCToEnergy;

  // private members:
 private:

#ifdef ___CINT___
 public:
  ClassDef(MCalibratorEnergyPointwiseLinear, 0) // no description
#endif

};

//! Streamify the calibrator
ostream& operator<<(ostream& os, const MCalibratorEnergyPointwiseLinear& C);

#endif


////////////////////////////////////////////////////////////////////////////////
