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


//! This class represents a hit
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


  // Strip hits:

  //! Return the number of strip hits
  unsigned int GetNStripHits() const { return m_StripHits.size(); }
  //! Return strip hit i or nullptr if i is out of bounds
  //! Ownership stays elsewhere
  MStripHit* GetStripHit(unsigned int i);
  //! Add a strip hit
  //! Ownership stays elsewhere
  void AddStripHit(MStripHit* StripHit) { m_StripHits.push_back(StripHit); }
  //! Remove strip hit i without deleting it
  void RemoveStripHit(unsigned int i);
  //! Remove a strip hit without deleting it
  void RemoveStripHit(MStripHit* StripHit);


  // Position:

  //! Set the position of the hit
  void SetPosition(const MVector& Position) { m_Position = Position; }
  //! Return the position of the hit
  MVector GetPosition() const { return m_Position; }

  //! Set the position resolution of the hit
  void SetPositionResolution(const MVector& PositionResolution) { m_PositionResolution = PositionResolution; }
  //! Return the position resolution of the hit
  MVector GetPositionResolution() const { return m_PositionResolution; }


  // Energy:

  //! Set the energy
  void SetEnergy(double Energy) { m_Energy = Energy; }
  //! Return the energy
  double GetEnergy() const { return m_Energy; }

  //! Set the energy resolution
  void SetEnergyResolution(double EnergyResolution) { m_EnergyResolution = EnergyResolution; }
  //! Return the energy resolution
  double GetEnergyResolution() const { return m_EnergyResolution; }

  //! Set the low-voltage energy
  void SetLVEnergy(double LVEnergy) { m_LVEnergy = LVEnergy; }
  //! Return the low-voltage energy
  double GetLVEnergy() const { return m_LVEnergy; }

  //! Set the high-voltage energy
  void SetHVEnergy(double HVEnergy) { m_HVEnergy = HVEnergy; }
  //! Return the high-voltage energy
  double GetHVEnergy() const { return m_HVEnergy; }


  // Flags:

  //! REVIEW: Currently unused
  //! Set the cross talk flag
  void SetCrossTalkFlag(bool PossibleCrossTalk) { m_PossibleCrossTalk = PossibleCrossTalk; }
  //! Return the cross talk flag
  bool GetCrossTalkFlag() const { return m_PossibleCrossTalk; }

  //! Set the guard ring hit flag
  void SetGuardRingHitFlag(bool GuardRingHit) { m_GuardRingHit = GuardRingHit; }
  //! Return the guard ring hit flag
  bool GetGuardRingHitFlag() const { return m_GuardRingHit; }

  //! REVIEW: Currently unused
  //! Set the charge loss flag
  void SetChargeLossFlag(bool PossibleChargeLoss) { m_PossibleChargeLoss = PossibleChargeLoss; }
  //! Return the charge loss flag
  bool GetChargeLossFlag() const { return m_PossibleChargeLoss; }

  //! REVIEW: These should use low-voltage and high-voltage naming
  //! Set the flag indicating that an x strip was hit multiple times
  void SetStripHitMultipleTimesX(bool StripHitMultipleTimesX) { m_StripHitMultipleTimesX = StripHitMultipleTimesX; }
  //! Return the flag indicating that an x strip was hit multiple times
  bool GetStripHitMultipleTimesX() const { return m_StripHitMultipleTimesX; }
  //! Set the flag indicating that a y strip was hit multiple times
  void SetStripHitMultipleTimesY(bool StripHitMultipleTimesY) { m_StripHitMultipleTimesY = StripHitMultipleTimesY; }
  //! Return the flag indicating that a y strip was hit multiple times
  bool GetStripHitMultipleTimesY() const { return m_StripHitMultipleTimesY; }

  //! Set the charge sharing flag for the low-voltage side
  void SetChargeSharingLV(bool ChargeSharingLV) { m_ChargeSharingLV = ChargeSharingLV; }
  //! Return the charge sharing flag for the low-voltage side
  bool GetChargeSharingLV() const { return m_ChargeSharingLV; }
  //! Set the charge sharing flag for the high-voltage side
  void SetChargeSharingHV(bool ChargeSharingHV) { m_ChargeSharingHV = ChargeSharingHV; }
  //! Return the charge sharing flag for the high-voltage side
  bool GetChargeSharingHV() const { return m_ChargeSharingHV; }

  //! REVIEW: Only SetChargeSharingLV and SetChargeSharingHV are used
  //! Set the general charge sharing flag
  void SetChargeSharing(bool ChargeSharing) { m_ChargeSharing = ChargeSharing; }
  //! Return the general charge sharing flag
  bool GetChargeSharing() const { return m_ChargeSharing; }

  //! Set the no-depth flag
  void SetNoDepth(bool NoDepth = true) { m_NoDepth = NoDepth; }
  //! Return the no-depth flag
  bool GetNoDepth() const { return m_NoDepth; }

  //! REVIEW: This might be left over from greedy strip pairing
  //! Set the flag indicating that this hit uses a non-dominant neighbor strip
  void SetIsNondominantNeighborStrip(bool IsNonDominantNeighborStrip = true) { m_IsNonDominantNeighborStrip = IsNonDominantNeighborStrip; }
  //! Return the flag indicating that this hit uses a non-dominant neighbor strip
  bool GetIsNondominantNeighborStrip() const { return m_IsNonDominantNeighborStrip; }

  //! REVIEW: Currently unused
  //! Set the quality of the hit
  void SetHitQuality(double HitQuality) { m_HitQuality = HitQuality; }
  //! Return the quality of the hit
  double GetHitQuality() const { return m_HitQuality; }


  // Parsing / Streaming:

  //! Parse a hit in Nuclearizer's DAT format
  bool Parse(MString& Line, int Version = 1);
  //! Stream the hit in Nuclearizer's DAT format
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the hit in MEGAlib's EVTA format
  void StreamEvta(ostream& S);


  // Simulation:

  //! Set the origins from the simulations (take care of duplicates)
  void AddOrigins(const vector<int>& Origins);
  //! Get the origins from the simulation
  vector<int> GetOrigins() const { return m_Origins; }



  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Position of the hit
  MVector m_Position;
  //! Position resolution of the hit
  MVector m_PositionResolution;

  //! Energy of the hit
  double m_Energy;

  //! Low-voltage energy of the hit
  double m_LVEnergy;

  //! High-voltage energy of the hit
  double m_HVEnergy;

  //! Energy resolution of the hit
  double m_EnergyResolution;

  //! Quality of the hit
  double m_HitQuality;

  //! List of strip hits contributing to this hit
  //! Ownership stays with the caller
  vector<MStripHit*> m_StripHits;

  //! Flag indicating possible cross talk
  bool m_PossibleCrossTalk;
  //! Flag indicating possible charge loss
  bool m_PossibleChargeLoss;
  //! Flag indicating that this hit contains a guard ring strip
  bool m_GuardRingHit;

  //! Flag indicating that this hit contains an x strip hit multiple times
  bool m_StripHitMultipleTimesX;
  //! Flag indicating that this hit contains a y strip hit multiple times
  bool m_StripHitMultipleTimesY;

  //! Flag indicating that this hit contains charge sharing
  bool m_ChargeSharing;
  //! Flag indicating that this hit contains charge sharing on the low-voltage side
  bool m_ChargeSharingLV;
  //! Flag indicating that this hit contains charge sharing on the high-voltage side
  bool m_ChargeSharingHV;

  //! Flag indicating that the depth is invalid
  //! This can happen when the pixel depth was uncalibrated, the hit was mapped too far out of the detector, or timing data was missing
  bool m_NoDepth;

  //! Flag indicating that this hit was made from a charge sharing event using a neighbor strip with the lower energy fraction
  bool m_IsNonDominantNeighborStrip;

  //! Origin interaction IDs from simulations
  vector<int> m_Origins;



#ifdef ___CLING___
 public:
  ClassDef(MHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
