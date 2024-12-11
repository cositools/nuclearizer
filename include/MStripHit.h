/*
 * MStripHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MStripHit__
#define __MStripHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MReadOutElement.h"
#include "MReadOutElementDoubleStrip.h"
#include "MStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MStripHit
{
  // public interface:
 public:
  //! Default constructor
  MStripHit();
  //! Default destructor
  virtual ~MStripHit();

  //! Reset all data
  void Clear();

  //! Get the read-out element
  MReadOutElement* GetReadOutElement() const { return m_ReadOutElement; }
  
  //! Set the Detector ID
  void SetDetectorID(int DetectorID) { m_ReadOutElement->SetDetectorID(DetectorID); }
  //! Return the Detector ID
  int GetDetectorID() const { return m_ReadOutElement->GetDetectorID(); }

  //! Set the Strip ID
  void SetStripID(int StripID) { m_ReadOutElement->SetStripID(StripID); }
  //! Return the Strip ID
  int GetStripID() const { return m_ReadOutElement->GetStripID(); }

  //! Set the strip type (x/y)
  void IsXStrip(bool PositiveStrip) { m_ReadOutElement->IsLowVoltageStrip(PositiveStrip); }
  //! Return the strip type (x/y)
  bool IsXStrip() const { return m_ReadOutElement->IsLowVoltageStrip(); }

  //! Set the strip type (positive or negative)
  void IsLowVoltageStrip(bool PositiveStrip) { m_ReadOutElement->IsLowVoltageStrip(PositiveStrip); }
  //! Return the strip type (positive or negative)
  bool IsLowVoltageStrip() const { return m_ReadOutElement->IsLowVoltageStrip(); }

  //! Set whether the strip has triggered
  void HasTriggered(bool HasTriggered) { m_HasTriggered = HasTriggered; }
  //! Return whether the strip has triggered
  bool HasTriggered() const { return m_HasTriggered; }

  //! Set the uncorrected ADCUnits of the strip (before common-mode correction)
  void SetUncorrectedADCUnits(double UncorrectedADCUnits) { m_UncorrectedADCUnits = UncorrectedADCUnits; }
  //! Return the uncorrected ADCUnits of the strip (before common-mode correction)
  double GetUncorrectedADCUnits() const { return m_UncorrectedADCUnits; }

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

  //! Set the Temperature of the relavent preamp (in degrees C)
  void SetPreampTemp(double PreampTemp) { m_PreampTemp = PreampTemp; }
  //! Return the Temperature of the relavent preamp (in degrees C)
  double GetPreampTemp() const { return m_PreampTemp; }

  //! Set the origins from the simulations (take care of duplicates)
  void AddOrigins(vector<int> Origins);
  //! Get the origins from the simulation
  vector<int> GetOrigins() const { return m_Origins; }
  
  
  
  //! Parse some content from a line
  bool Parse(MString& Line, int Version = 1);
  //! Dump the content into a file stream
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the content in MEGAlib's roa format 
  void StreamRoa(ostream& S);
  
  
  // protected methods:
 protected:

   // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The read-out element
  MReadOutElementDoubleStrip* m_ReadOutElement;
  //! Strp has triggered
  bool m_HasTriggered;
  //! ADCUnits before all corrections
  double m_UncorrectedADCUnits;
  //! ADCUnits after any correction
  double m_ADCUnits;
  //! The calibrated energy
  double m_Energy;
  //! The energy resolution
  double m_EnergyResolution;
  //! Timing of the top side
  double m_Timing;
  //! Temperature of Preamp
  double m_PreampTemp;
  
  //! Origin IAs from simulations
  vector<int> m_Origins;

#ifdef ___CLING___
 public:
  ClassDef(MStripHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
