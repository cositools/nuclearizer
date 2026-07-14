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
  void AddStripHit(MStripHit* StripHit);
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

  //! Set the cross-talk flag
  void SetCrossTalkFlag(bool CrossTalk) { m_CrossTalk = CrossTalk; }
  //! Return the cross-talk flag
  bool GetCrossTalkFlag() const { return m_CrossTalk; }

  //! Set the guard-ring hit flag
  void SetGuardRingHitFlag(bool GuardRingHit) { m_GuardRingHit = GuardRingHit; }
  //! Return the guard-ring hit flag
  bool GetGuardRingHitFlag() const { return m_GuardRingHit; }

  //! Set the charge-loss flag
  void SetChargeLossFlag(bool ChargeLoss) { m_ChargeLoss = ChargeLoss; }
  //! Return the charge-loss flag
  bool GetChargeLossFlag() const { return m_ChargeLoss; }

  //! Set the flag indicating that a low-voltage strip was hit multiple times
  void SetStripHitMultipleTimesLV(bool StripHitMultipleTimesLV) { m_StripHitMultipleTimesLV = StripHitMultipleTimesLV; }
  //! Return the flag indicating that a low-voltage strip was hit multiple times
  bool GetStripHitMultipleTimesLV() const { return m_StripHitMultipleTimesLV; }
  //! Set the flag indicating that a high-voltage strip was hit multiple times
  void SetStripHitMultipleTimesHV(bool StripHitMultipleTimesHV) { m_StripHitMultipleTimesHV = StripHitMultipleTimesHV; }
  //! Return the flag indicating that a high-voltage strip was hit multiple times
  bool GetStripHitMultipleTimesHV() const { return m_StripHitMultipleTimesHV; }

  //! Set the charge sharing flag for the low-voltage side
  void SetChargeSharingLV(bool ChargeSharingLV) { m_ChargeSharingLV = ChargeSharingLV; }
  //! Return the charge sharing flag for the low-voltage side
  bool GetChargeSharingLV() const { return m_ChargeSharingLV; }
  //! Set the charge sharing flag for the high-voltage side
  void SetChargeSharingHV(bool ChargeSharingHV) { m_ChargeSharingHV = ChargeSharingHV; }
  //! Return the charge sharing flag for the high-voltage side
  bool GetChargeSharingHV() const { return m_ChargeSharingHV; }
  //! Return the general charge sharing flag
  bool GetChargeSharing() const { return m_ChargeSharingLV == true || m_ChargeSharingHV == true; }

  //! Set the no-depth flag
  void SetNoDepth(bool NoDepth = true) { m_NoDepth = NoDepth; }
  //! Return the no-depth flag
  bool GetNoDepth() const { return m_NoDepth; }

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
  //! List of strip hits contributing to this hit
  //! Ownership stays elsewhere
  vector<MStripHit*> m_StripHits;

  //! Position of the hit
  MVector m_Position;
  //! Position resolution of the hit
  MVector m_PositionResolution;

  //! Energy of the hit
  double m_Energy;
  //! Energy resolution of the hit
  double m_EnergyResolution;
  //! Low-voltage energy of the hit
  double m_LVEnergy;
  //! High-voltage energy of the hit
  double m_HVEnergy;

  //! Flag indicating cross talk
  bool m_CrossTalk;
  //! Flag indicating that this hit contains a guard ring strip
  bool m_GuardRingHit;
  //! Flag indicating charge loss
  bool m_ChargeLoss;

  //! Flag indicating that this hit contains a low-voltage strip hit multiple times
  bool m_StripHitMultipleTimesLV;
  //! Flag indicating that this hit contains a high-voltage strip hit multiple times
  bool m_StripHitMultipleTimesHV;

  //! Flag indicating that this hit contains charge sharing on the low-voltage side
  bool m_ChargeSharingLV;
  //! Flag indicating that this hit contains charge sharing on the high-voltage side
  bool m_ChargeSharingHV;

  //! Flag indicating that the depth is invalid
  //! This can happen when the pixel depth was uncalibrated, the hit was mapped too far out of the detector, or timing data was missing
  bool m_NoDepth;

  //! Origin interaction IDs from simulations
  vector<int> m_Origins;


#ifdef ___CLING___
 public:
  ClassDef(MHit, 0) // a hit
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
