/*
 * MNCTHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTHit__
#define __MNCTHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MNCTStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTHit
{
  // public interface:
 public:
  //! Standard constructor
  MNCTHit();
  //! Default destructor
  virtual ~MNCTHit();

  //! Reset all data
  void Clear();

  //! Set the position of the hit
  void SetPosition(const MVector& Position) { m_Position = Position; }
  //! Return the position of the hit
  MVector GetPosition() const { return m_Position; }

  //! Set the position of the hit
  void SetPositionResolution(const MVector& PositionResolution) { m_PositionResolution = PositionResolution; }
  //! Return the position of the hit
  MVector GetPositionResolution() const { return m_PositionResolution; }

  //! Set the energy
  void SetEnergy(double Energy) { m_Energy = Energy; }
  //! Return the energy
  double GetEnergy() const { return m_Energy; }

  //! Set the energy resolution
  void SetEnergyResolution(double EnergyResolution) { m_EnergyResolution = EnergyResolution; }
  //! Return the energy resolution
  double GetEnergyResolution() const { return m_EnergyResolution; }

  //! Set the Quality of the Hit
  void SetHitQuality(double HitQuality) { m_HitQuality = HitQuality; }
  //! Return the Quality of the Hit
  double GetHitQuality() const { return m_HitQuality; }

  //! Return the number of strip hits
  unsigned int GetNStripHits() const { return m_StripHits.size(); }
  //! Return strip hit i
  MNCTStripHit* GetStripHit(unsigned int i);
  //! Add a strip hit
  void AddStripHit(MNCTStripHit* StripHit) { return m_StripHits.push_back(StripHit); }
 
	//! set cross talk flag
	void SetCrossTalkFlag(bool PossibleCrossTalk) {m_PossibleCrossTalk = PossibleCrossTalk;}
	//! get cross talk flag value
	bool GetCrossTalkFlag() const { return m_PossibleCrossTalk; }

	//! set charge loss flag
	void SetChargeLossFlag(bool PossibleChargeLoss) {m_PossibleChargeLoss = PossibleChargeLoss;}
	//! get charge loss flag value
	bool GetChargeLossFlag() const { return m_PossibleChargeLoss; }
 
  //! Dump the content into a file stream
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the content in MEGAlib's evta format 
  void StreamEvta(ostream& S);
  
  // protected methods:
 protected:
  //MNCTHit() {};
  //MNCTHit(const MNCTHit& NCTHit) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Position of the hit
  MVector m_Position;
  //! Position resolutionof the hit
  MVector m_PositionResolution;

  //! Energy of the hit
  double m_Energy;
  //! Energy resolution of the hit
  double m_EnergyResolution;

  //! Quality of the Hit
  double m_HitQuality; 

  //! List of strip hits
  vector<MNCTStripHit*> m_StripHits;

	//! Flag: possible cross talk
	bool m_PossibleCrossTalk;
	//! Flag: possible charge loss
	bool m_PossibleChargeLoss;

#ifdef ___CINT___
 public:
  ClassDef(MNCTHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
