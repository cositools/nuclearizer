/*
 * MNCTStripHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
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

// Nuclearizer libs
#include "MReadOutElement.h"
#include "MReadOutElementDoubleStrip.h"

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
  void IsXStrip(bool PositiveStrip) { m_ReadOutElement->IsPositiveStrip(PositiveStrip); }
  //! Return the strip type (x/y)
  bool IsXStrip() const { return m_ReadOutElement->IsPositiveStrip(); }

  //! Set the strip type (positive or negative)
  void IsPositiveStrip(bool PositiveStrip) { m_ReadOutElement->IsPositiveStrip(PositiveStrip); }
  //! Return the strip type (positive or negative)
  bool IsPositiveStrip() const { return m_ReadOutElement->IsPositiveStrip(); }

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


#ifdef ___CINT___
 public:
  ClassDef(MNCTStripHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
