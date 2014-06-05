/*
 * MNCTStripHit.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTStripHit__
#define __MNCTStripHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTStripHit
{
  // public interface:
 public:
  //! Default constructor
  MNCTStripHit();
  //! Default destructor
  virtual ~MNCTStripHit();

  //! Reset all data
  void Clear();

  //! Set the Detector ID
  void SetDetectorID(int DetectorID) { m_DetectorID = DetectorID; }
  //! Return the Detector ID
  int GetDetectorID() const { return m_DetectorID; }

  //! Set the Strip ID
  void SetStripID(int StripID) { m_StripID = StripID; }
  //! Return the Strip ID
  int GetStripID() const { return m_StripID; }

  //! Set the strip type (x/y)
  void IsXStrip(bool IsXStrip) { m_IsXStrip = IsXStrip; }
  //! Return the strip type (x/y)
  bool IsXStrip() const { return m_IsXStrip; }

  //! Set the ADCUnits of the strip
  void SetADCUnits(double ADCUnits) { m_ADCUnits = ADCUnits; }
  //! Return the ADCUnits of the strip
  double GetADCUnits() const { return m_ADCUnits; }

  //! Set the calibrated energy
  void SetEnergy(double Energy) { m_Energy = Energy; }
  //! Return the calibrated energy
  double GetEnergy() const { return m_Energy; }

  //! Set the energy resolution (sigma)
  void SetEnergyResolution(double EnergyResolution) { m_EnergyResolution = EnergyResolution; }
  //! Return the calibrated energy
  double GetEnergyResolution() const { return m_EnergyResolution; }

  //! Set the Timing of the top side
  void SetTiming(double Timing) { m_Timing = Timing; }
  //! Return the Timing of the top side
  double GetTiming() const { return m_Timing; }

  // protected methods:
 protected:
  //MNCTStripHit() {};
  //MNCTStripHit(const MNCTStripHit& NCTStripHit) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Detector ID
  int m_DetectorID;
  //! Strip ID
  int m_StripID;
  //! Strip type
  bool m_IsXStrip;
  //! ADCUnits of the top side
  double m_ADCUnits;
  //! The calibrated energy
  double m_Energy;
  //! The energy resolution
  double m_EnergyResolution;
  //! Timing of the top side
  double m_Timing;


#ifdef ___CINT___
 public:
  ClassDef(MNCTStripHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
