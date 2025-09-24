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

  //! Set the strip type (x/y). Note that x strips run parallel to the x-axis!!
  //void IsLowVoltageStrip(bool PositiveStrip) { m_ReadOutElement->IsLowVoltageStrip(PositiveStrip); }
  //! Return the strip type (x/y). Note that x strips run parallel to the x-axis!!
  //bool IsLowVoltageStrip() const { return m_ReadOutElement->IsLowVoltageStrip(); }

  //! Set the strip type (positive or negative)
  //void IsLowVoltageStrip(bool PositiveStrip) { m_ReadOutElement->IsLowVoltageStrip(PositiveStrip); }
  //! Return the strip type (positive or negative)
  //bool IsLowVoltageStrip() const { return m_ReadOutElement->IsLowVoltageStrip(); }

  //! Set the strip type (LV or HV)
  //! Remark:  HV = negative = Y strip in old nomenclature)
  //! Remark:  LV = positive = X strip in old nomenclature)
  void IsLowVoltageStrip(bool LowVoltageStrip) { m_ReadOutElement->IsLowVoltageStrip(LowVoltageStrip); }
  //! Return the strip type (LV or HV)
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

  //! Set the TAC
  void SetTAC(double TAC) { m_TAC = TAC; }
  //! Return the TAC
  double GetTAC() const { return m_TAC; }

  //! Set the TAC resolution
  void SetTACResolution(double TACResolution) { m_TACResolution = TACResolution; }
  //! Return the TAC resolution
  double GetTACResolution() const { return m_TACResolution; }

  //! Set the Timing in nanoseconds
  void SetTiming(double Timing) { m_Timing = Timing; }
  //! Return the Timing in nanoseconds
  double GetTiming() const { return m_Timing; }

  //! Set the Timing resolution
  void SetTimingResolution(double TimingResolution) { m_TimingResolution = TimingResolution; }
  //! Return the Timing resolution
  double GetTimingResolution() const { return m_TimingResolution; }

  //! Set the Temperature of the relavent preamp (in degrees C)
  void SetPreampTemp(double PreampTemp) { m_PreampTemp = PreampTemp; }
  //! Return the Temperature of the relavent preamp (in degrees C)
  double GetPreampTemp() const { return m_PreampTemp; }

  //! Set the origins from the simulations (take care of duplicates)
  void AddOrigins(vector<int> Origins);
  //! Get the origins from the simulation
  vector<int> GetOrigins() const { return m_Origins; }

  //! Set the Guard Ring flag
  void IsGuardRing(bool GuardRing) { m_IsGuardRing = GuardRing; }
  //! Return a boolean indicating whether the strip is a Guard Ring
  bool IsGuardRing() const { return m_IsGuardRing; }  
  //! Set the Nearest Neighbor flag
  void IsNearestNeighbor(bool NearestNeighbor) { m_IsNearestNeighbor = NearestNeighbor; }
  //! Return a boolean indicating whether the strip is a Nearest Neighbor
  bool IsNearestNeighbor() const { return m_IsNearestNeighbor; }
    
  //! Set the Fast Timing flag
  void HasFastTiming(bool FastTiming) { m_HasFastTiming = FastTiming; }
  //! Return a boolean indicating whether the strip timing is fast;
  bool HasFastTiming() const { return m_HasFastTiming; }

  //! Set the Calibrated Timing flag
  void HasCalibratedTiming(bool CalibratedTiming) { m_HasCalibratedTiming = CalibratedTiming; }
  //! Return a boolean indicating whether the strip timing has been calibrated;
  bool HasCalibratedTiming() const { return m_HasCalibratedTiming; }

  //! Produce an unsigned int with bitwise values representing flags
  unsigned int MakeFlags();
  //! Read in unsigned int with bitwise values representing flags and update boolean flags
  void ParseFlags(unsigned int Flags);

  //! Parse some content from a line
  bool Parse(MString& Line, int Version = 1);
  //! Dump the content into a file stream
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the content in MEGAlib's roa format 
  void StreamRoa(ostream& S, bool WithADC = true, bool WithTAC = true, bool WithEnergy = false, bool WithTiming = false, bool WithTemperature = false, bool WithFlags = false, bool WithOrigins = false);
  
  
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
  //! TAC timing
  double m_TAC;
  //! TAC timing resolution
  double m_TACResolution;
  //! Timing in ns
  double m_Timing;
  //! Timing resolution in ns
  double m_TimingResolution;
  //! Temperature of Preamp
  double m_PreampTemp;

  //! Flags denoting the type of strip hit
  bool m_IsGuardRing;
  bool m_IsNearestNeighbor;

  //! Flag indicating whether the hit has fast timing
  bool m_HasFastTiming;
  //! Flag indicating whether the hit has calibrated timing
  bool m_HasCalibratedTiming;

  //! Origin IAs from simulations
  vector<int> m_Origins;

#ifdef ___CLING___
 public:
  ClassDef(MStripHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
