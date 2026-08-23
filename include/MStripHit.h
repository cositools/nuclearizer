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

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This class represents a hit in a strip
class MStripHit
{
  // public interface:
 public:
  //! Default constructor
  MStripHit();
  //! Copy constructor is disabled because this class owns a m_ReadOutElement pointer
  MStripHit(const MStripHit&) = delete;
  //! Assignment is disabled because this class owns a m_ReadOutElement pointer
  MStripHit& operator=(const MStripHit&) = delete;
  //! Default destructor
  virtual ~MStripHit();

  //! Reset all data
  void Clear();


  // The read-out element

  //! Return the read-out element
  //! Ownership stays with this object
  MReadOutElement* GetReadOutElement() const { return m_ReadOutElement; }

  //! Set the detector ID
  void SetDetectorID(unsigned int DetectorID) { m_ReadOutElement->SetDetectorID(DetectorID); }
  //! Return the detector ID
  unsigned int GetDetectorID() const { return m_ReadOutElement->GetDetectorID(); }

  //! Set the strip ID
  void SetStripID(unsigned int StripID) { m_ReadOutElement->SetStripID(StripID); }
  //! Return the strip ID
  unsigned int GetStripID() const { return m_ReadOutElement->GetStripID(); }

  //! DEPRECATED: Remove, but we need to remove it from remaining classes that use it first: Issue #172
  void IsXStrip(bool LowVoltageSide) { m_ReadOutElement->IsLowVoltageStrip(LowVoltageSide); }
  //! DEPRECATED: Remove, but we need to remove it from remaining classes that use it first: Issue #172
  bool IsXStrip() const { return m_ReadOutElement->IsLowVoltageStrip(); }

  //! Set whether the strip is on the low-voltage side
  void IsLowVoltageStrip(bool LowVoltageSide) { m_ReadOutElement->IsLowVoltageStrip(LowVoltageSide); }
  //! Return whether the strip is on the low-voltage side
  bool IsLowVoltageStrip() const { return m_ReadOutElement->IsLowVoltageStrip(); }


  // Energy section:

  //! TODO: Switch to unsigned int: Issue #176
  //! Set the measured ADC units of the strip
  void SetADCUnits(double ADCUnits) { m_ADCUnits = ADCUnits; }
  //! Return the measured ADC units of the strip
  double GetADCUnits() const { return m_ADCUnits; }

  //! Set the calibrated energy (= calibrated ADC's) in keV
  void SetEnergy(double Energy) { m_Energy = Energy; }
  //! Return the calibrated energy (= calibrated ADC's) in keV
  double GetEnergy() const { return m_Energy; }

  //! Set the energy resolution (1 sigma)
  void SetEnergyResolution(double EnergyResolution) { m_EnergyResolution = EnergyResolution; }
  //! Return the energy resolution (1 sigma)
  double GetEnergyResolution() const { return m_EnergyResolution; }


  // Timing:

  //! TODO: Switch to unsigned int: Issue #176
  //! Set the measured TAC value of the strip (arrival timing)
  void SetTAC(double TAC) { m_TAC = TAC; }
  //! Return the measured TAC value of the strip (arrival timing)
  double GetTAC() const { return m_TAC; }

  //! Set the timing (= calibrated TAC) in nanoseconds
  void SetTiming(double Timing) { m_Timing = Timing; }
  //! Return the timing (= calibrated TAC) in nanoseconds
  double GetTiming() const { return m_Timing; }

  //! Set the timing resolution
  void SetTimingResolution(double TimingResolution) { m_TimingResolution = TimingResolution; }
  //! Return the timing resolution
  double GetTimingResolution() const { return m_TimingResolution; }



  // Flags:

  //! Set the guard ring flag
  void IsGuardRing(bool GuardRing) { m_IsGuardRing = GuardRing; }
  //! Return whether the strip is a guard-ring hit
  bool IsGuardRing() const { return m_IsGuardRing; }

  //! Set the nearest-neighbor flag
  void IsNearestNeighbor(bool NearestNeighbor) { m_IsNearestNeighbor = NearestNeighbor; }
  //! Return whether the strip is a nearest-neighbor hit
  bool IsNearestNeighbor() const { return m_IsNearestNeighbor; }

  //! Set whether the strip has passed the fast threshold
  void HasFastTiming(bool FastTiming) { m_HasFastTiming = FastTiming; }
  //! Return whether the strip has passed the fast threshold
  bool HasFastTiming() const { return m_HasFastTiming; }

  //! Set whether the strip has triggered (ADC values above slow threshold)
  void HasTriggered(bool HasTriggered) { m_HasTriggered = HasTriggered; }
  //! Return whether the strip has triggered (ADC values above slow threshold)
  bool HasTriggered() const { return m_HasTriggered; }

  //! TODO: Rename to HasTiming()
  //! Set the calibrated-timing flag
  void HasCalibratedTiming(bool CalibratedTiming) { m_HasCalibratedTiming = CalibratedTiming; }
  //! Return whether the strip timing has been calibrated
  bool HasCalibratedTiming() const { return m_HasCalibratedTiming; }


  // Parsing:

  //! Return the bitwise strip-hit flags
  unsigned int MakeFlags();
  //! Update the strip-hit flags from a bit mask
  void ParseFlags(unsigned int Flags);

  //! Parse a strip hit from a line in DAT format
  bool Parse(const MString& Line, int Version = 1);
  //! Stream the strip hit in Nuclearizer's DAT format
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the strip hit in MEGAlib's ROA format
  void StreamRoa(ostream& S, bool WithADC = true, bool WithTAC = true, bool WithEnergy = false, bool WithTiming = false, bool WithFlags = false, bool WithOrigins = false);


  // Simulation:

  //! Add origins from the simulation and remove duplicates
  void AddOrigins(const vector<int>& Origins);
  //! Return the origins from the simulation
  vector<int> GetOrigins() const { return m_Origins; }


  // protected methods:
 protected:

   // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Read-out element
  MReadOutElementDoubleStrip* m_ReadOutElement;

  //! Measured ADC units
  double m_ADCUnits;
  //! Calibrated energy (= calibrated ADC's) in keV
  double m_Energy;
  //! Energy resolution in keV (1 sigma)
  double m_EnergyResolution;

  //! The measured TAC timing values
  double m_TAC;
  //! Timing (= calibrated TAC) in nanoseconds
  double m_Timing;
  //! Timing resolution in nanoseconds
  double m_TimingResolution;


  //! True if the hit is a guard ring hit
  bool m_IsGuardRing;
  //! True if the hit is a nearest neighbor hit
  bool m_IsNearestNeighbor;
  //! True if the strip has triggered
  bool m_HasTriggered;
  //! True if the hit has fast timing
  bool m_HasFastTiming;
  //! True if the hit has calibrated timing
  bool m_HasCalibratedTiming;

  //! Origin interaction IDs from the simulation
  vector<int> m_Origins;


#ifdef ___CLING___
 public:
  ClassDef(MStripHit, 0) // A strip hit
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
