/*
 * MCalibratorEnergy.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibratorEnergy__
#define __MCalibratorEnergy__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer
#include "MReadOutElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MCalibratorEnergy
{
  // public interface:
 public:
  //! Base class for energy calibration
  MCalibratorEnergy();
  //! Default destructor
  virtual ~MCalibratorEnergy();

  //! Set the read out element
  //! Attention this class will be the new owner of the new element and will delete it on destruction
  void SetReadOutElement(MReadOutElement* ReadOut) { delete m_ReadOut; m_ReadOut = ReadOut; }
  //! Get the read out element
  //! Attention is remains property of this class and will get deleted with this class!
  MReadOutElement* GetReadOutElement() const { return m_ReadOut; } 

  //! Return true if this calibrator can have multiple entries
  //! i.e. the CP command appears seberal times with the same read-out element
  bool HasMultipleEntries() const { return m_HasMultipleEntries; }
  
  //! Return the energy associated with the given ADC units
  virtual double GetEnergy(double ADCUnits) { return 0; } // not const because derived funtion might save intermediate values to speed the analysis up

  //! Dump a string about the class
  virtual MString ToString() const;
  
  //! Enable/Disable Preamp Temp Correction
  void EnablePreampTempCorrection(bool X) {m_TemperatureEnabled = X;}
  //! Get coincidence merging true/false
  bool GetPreampTempCorrection() const { return m_TemperatureEnabled; }


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  //! The associated read out element
  MReadOutElement* m_ReadOut;
  //! Check if this calibrator has multiple entries
  bool m_HasMultipleEntries;
  //! Preamp Temperature Correction
  bool m_TemperatureEnabled;

  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MCalibratorEnergy, 0) // no description
#endif

};

//! Streamify the calibrator
std::ostream& operator<<(std::ostream& os, const MCalibratorEnergy& C);

#endif


////////////////////////////////////////////////////////////////////////////////
