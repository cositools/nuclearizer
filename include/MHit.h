/*
 * MHit.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MHit__
#define __MHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MHit
{
  // public interface:
 public:
  //! Standard constructor
  MHit();
  //! Default destructor
  virtual ~MHit();

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
    
    //! Set the LVenergy
    void SetLVEnergy(double LVEnergy) { m_LVEnergy = LVEnergy; }
    //! Return the LVenergy
    double GetLVEnergy() const { return m_LVEnergy; }

    //! Set the HVenergy
    void SetHVEnergy(double HVEnergy) { m_HVEnergy = HVEnergy; }
    //! Return the HVenergy
    double GetHVEnergy() const { return m_HVEnergy; }

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
  MStripHit* GetStripHit(unsigned int i);
  //! Add a strip hit
  void AddStripHit(MStripHit* StripHit) { return m_StripHits.push_back(StripHit); }
  //! Remove a strip hit
  void RemoveStripHit(unsigned int i); 
  //! Remove a strip hit
  void RemoveStripHit(MStripHit* StripHit); 
  
	//! set cross talk flag
	void SetCrossTalkFlag(bool PossibleCrossTalk) {m_PossibleCrossTalk = PossibleCrossTalk;}
	//! get cross talk flag value
	bool GetCrossTalkFlag() const { return m_PossibleCrossTalk; }

	//! set charge loss flag
	void SetChargeLossFlag(bool PossibleChargeLoss) {m_PossibleChargeLoss = PossibleChargeLoss;}
	//! get charge loss flag value
	bool GetChargeLossFlag() const { return m_PossibleChargeLoss; }

	//! set x strip hit multiple times flag
	void SetStripHitMultipleTimesX(bool stripHitMultipleTimesX) {m_StripHitMultipleTimesX = stripHitMultipleTimesX;}
	//! get m_StripHitMultipleTimesX
	bool GetStripHitMultipleTimesX() const { return m_StripHitMultipleTimesX; }
	//! set y strip hit multiple times flag
	void SetStripHitMultipleTimesY(bool stripHitMultipleTimesY) {m_StripHitMultipleTimesY = stripHitMultipleTimesY;}
	//! get m_StripHitMultipleTimesY
	bool GetStripHitMultipleTimesY() const { return m_StripHitMultipleTimesY; }

	//! set charge sharing flag
	void SetChargeSharing(bool chargeSharing) {m_ChargeSharing = chargeSharing; }
	//! get m_ChargeSharing
	bool GetChargeSharing() const { return m_ChargeSharing; }
	//! set m_NoDepth
	void SetNoDepth(bool X = true) { m_NoDepth = X;}
	//! get m_NoDepth
	bool GetNoDepth(void) const { return m_NoDepth; }
	//! set m_IsNonDominantNeighborStrip
	void SetIsNondominantNeighborStrip(bool X = true) {m_IsNonDominantNeighborStrip = X;}
	//! get m_IsNonDominantNeighborStrip
	bool GetIsNondominantNeighborStrip(void) const {return m_IsNonDominantNeighborStrip;}
	
	//! Set the origins from the simulations (take care of duplicates)
	void AddOrigins(vector<int> Origins);
  //! Get the origins from the simulation
  vector<int> GetOrigins() const { return m_Origins; }
  
  //! Dump the content into a file stream
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the content in MEGAlib's evta format 
  void StreamEvta(ostream& S);
  
  //! Parse some content from a line
  bool Parse(MString &Line, int Version = 1);

  // protected methods:
 protected:
  //MHit() {};
  //MHit(const MHit& NCTHit) {};

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
    
  //! LV Energy of the hit
  double m_LVEnergy;
    
  //! HV Energy of the hit
  double m_HVEnergy;
    
  //! Energy resolution of the hit
  double m_EnergyResolution;

  //! Quality of the Hit
  double m_HitQuality; 

  //! List of strip hits
  vector<MStripHit*> m_StripHits;

	//! Flag: possible cross talk
	bool m_PossibleCrossTalk;
	//! Flag: possible charge loss
	bool m_PossibleChargeLoss;

	//! true if hit contains strip that was hit multiple times on X
	bool m_StripHitMultipleTimesX = false;
    //! true if hit contains strip that was hit multiple times on Y
	bool m_StripHitMultipleTimesY = false;

	//! true if hit contains charge sharing
	bool m_ChargeSharing;

	//! true if depth is invalid, either because the pixel depth was uncalibrated, the hit was mapped too far out of the detector,or there was no timing data
	bool m_NoDepth;

	//! true if hit was made from a charge sharing event using a neighbor strip that had the lowere energy fraction
	bool m_IsNonDominantNeighborStrip;
  
  //! Origin IAs from simulations
  vector<int> m_Origins;
  
  
  
#ifdef ___CLING___
 public:
  ClassDef(MHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
