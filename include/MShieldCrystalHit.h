/*
 * MShieldCrystalHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MShieldCrystalHit__
#define __MShieldCrystalHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MReadOutElement.h"
#include "MShieldCrystalHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MShieldCrystalHit
{
  // public interface:
 public:
  //! Default constructor
  MShieldCrystalHit();
  //! Default destructor
  virtual ~MShieldCrystalHit();

  //! Reset all data
  void Clear();

  //! Get the read-out element
  MReadOutElement* GetReadOutElement() const { return m_ReadOutElement; }

  //! Set the Crystal ID
  void SetCrystalID(int StripID) { m_ReadOutElement->SetDetectorID(StripID); }
  //! Return the Crystal ID
  int GetCrystalID() const { return m_ReadOutElement->GetDetectorID(); }

  //! Set the Detector ID - this derived from the crystal ID
  void SetDetectorID(int DetectorID) { m_ReadOutElement->SetDetectorID(DetectorID); }
  //! Return the Detector ID- this derived from the crystal ID
  int GetDetectorID() const { return m_ReadOutElement->GetDetectorID(); }

  //! Set whether the crystal has triggered
  void HasTriggered(bool HasTriggered) { m_HasTriggered = HasTriggered; }
  //! Return whether the crystal has triggered
  bool HasTriggered() const { return m_HasTriggered; }

  //! Set whether the crystal has triggered
  void HasVetoed(bool HasVetoed) { m_HasVetoed = HasVetoed; }
  //! Return whether the crystal has triggered
  bool HasVetoed() const { return m_HasVetoed; }

  //! Set the ADCUnits of the crystal
  void SetADCUnits(double ADCUnits) { m_ADCUnits = ADCUnits; }
  //! Return the ADCUnits of the crystal
  double GetADCUnits() const { return m_ADCUnits; }

  //! Set the calibrated energy
  void SetEnergy(double Energy) { m_Energy = Energy; }
  //! Return the calibrated energy
  double GetEnergy() const { return m_Energy; }

  //! Set the energy resolution (sigma)
  void SetEnergyResolution(double EnergyResolution) { m_EnergyResolution = EnergyResolution; }
  //! Return the calibrated energy
  double GetEnergyResolution() const { return m_EnergyResolution; }

  //! Set the origins from the simulations (take care of duplicates)
  void AddOrigins(vector<int> Origins);
  //! Get the origins from the simulation
  vector<int> GetOrigins() const { return m_Origins; }

  //! Produce an unsigned int with bitwise values representing flags
  unsigned int MakeFlags();
  //! Read in unsigned int with bitwise values representing flags and update boolean flags
  void ParseFlags(unsigned int Flags);

  //! Parse some content from a line
  bool Parse(MString& Line, int Version = 1);
  //! Dump the content into a file stream
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the content in MEGAlib's roa format 
  void StreamRoa(ostream& S, bool WithADC = true, bool WithEnergy = false, bool WithFlags = false, bool WithOrigins = false);
  
  
  // protected methods:
 protected:

   // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The read-out element with a single ID (representing the crystal ID)
  MReadOutElement* m_ReadOutElement;
  //! Crystal has triggered
  bool m_HasTriggered;
  //! Crystal has vetoed
  bool m_HasVetoed;
  //! ADCUnits before all corrections
  double m_UncorrectedADCUnits;
  //! ADCUnits after any correction
  double m_ADCUnits;
  //! The calibrated energy
  double m_Energy;
  //! The energy resolution
  double m_EnergyResolution;

  //! Origin IAs from simulations
  vector<int> m_Origins;

#ifdef ___CLING___
 public:
  ClassDef(MShieldCrystalHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
