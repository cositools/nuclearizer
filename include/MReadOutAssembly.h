/*
 * MReadOutAssembly.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNuclearizerReadOutAssembly__
#define __MNuclearizerReadOutAssembly__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <atomic>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MStripHit.h"
#include "MDEEStripHit.h"
#include "MCrystalHit.h"
#include "MDEECrystalHit.h"
#include "MGuardringHit.h"
#include "MHit.h"
#include "MPhysicalEvent.h"
#include "MSimEvent.h"
#include "MSimIA.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MReadOutAssembly : public MReadOutSequence
{
  // public interface:
 public:
  //! Default constructor
  MReadOutAssembly();
  //! Default destructor
  virtual ~MReadOutAssembly();

  //! Reset all data
  virtual void Clear();

  //! Delete Hits
  void DeleteHits();

  //! Set and get the Reference Time System for this event
  //! The RTS is mission time in seconds since Jan 1, 2025 in TT
  void SetTimeRTS(const MTime& TimeRTS) { m_EventTimeRTS = TimeRTS; }
  MTime GetTimeRTS() const { return m_EventTimeRTS; }
  
  //! Set and get the UTC time of this event
  void SetTimeUTC(const MTime& TimeUTC) { m_EventTimeUTC = TimeUTC; }
  MTime GetTimeUTC() const { return m_EventTimeUTC; }
  
  //! Set and get simulation aspect information
  void SetGalacticPointingXAxisTheta(double theta){ m_GalacticPointingXAxisTheta = theta; }
  void SetGalacticPointingXAxisPhi(double phi){ m_GalacticPointingXAxisPhi = phi; }
  void SetGalacticPointingZAxisTheta(double theta){ m_GalacticPointingZAxisTheta = theta; }
  void SetGalacticPointingZAxisPhi(double phi){ m_GalacticPointingZAxisPhi = phi; }

  double GetGalacticPointingXAxisTheta(){ if (m_HasSimAspectInfo){return m_GalacticPointingXAxisTheta;} else{return 0;}}
  double GetGalacticPointingXAxisPhi(){ if (m_HasSimAspectInfo){return m_GalacticPointingXAxisPhi;} else{return 0;}}
  double GetGalacticPointingZAxisTheta(){ if (m_HasSimAspectInfo){return m_GalacticPointingZAxisTheta;} else{return 0;}}
  double GetGalacticPointingZAxisPhi(){ if (m_HasSimAspectInfo){return m_GalacticPointingZAxisPhi;} else{return 0;}}

  void SetSimAspectInfo(bool TF){ m_HasSimAspectInfo = TF; }
  bool HasSimAspectInfo(){ return m_HasSimAspectInfo; }


  //! Find out if the event contains strip hits in a given detector
  bool InDetector(int DetectorID);

  //! Set the guard ring veto flag
  void SetGuardRingVeto(bool Veto = true) { m_GuardRingVeto = Veto; }
  //! Get the guard ring veto flag
  bool GetGuardRingVeto() const { return m_GuardRingVeto; }

  //! Set the shield veto flag
  void SetShieldVeto(bool Veto = true) { m_ShieldVeto = Veto; }
  //! Get the shield veto flag
  bool GetShieldVeto() const { return m_ShieldVeto; }

  //! Set the triggered flag
  void SetTrigger(bool Trigger = true) { m_Trigger = Trigger; }
  //! Return the trigger flag
  bool GetTrigger() const { return m_Trigger; }

  //! Return the number of strip hits
  unsigned int GetNStripHits() const { return m_StripHits.size(); }
  //! Return strip hit i
  MStripHit* GetStripHit(unsigned int i);
  //! Add a strip hit
  void AddStripHit(MStripHit* StripHit);
  //! Remove a strip hit
  void RemoveStripHit(unsigned int i);

  //! Return the number of T Only strip hits
  //! TODO Is this a hold-over from balloon days?
  unsigned int GetNStripHitsTOnly() const { return m_StripHitsTOnly.size(); }
  //! Return strip hit i
  MStripHit* GetStripHitTOnly(unsigned int i);
  //! Adda T Only strip hit
  void AddStripHitTOnly(MStripHit*);
  //! Remove a strip hit
  void RemoveStripHitTOnly(unsigned int i);

  //! Return the number of crystal hits
  unsigned int GetNCrystalHits() const { return m_CrystalHits.size(); }
  //! Return crystal hit i
  MCrystalHit* GetCrystalHit(unsigned int i);
  //! Add a crystal hit
  void AddCrystalHit(MCrystalHit* CrystalHit);
  //! Remove a crystal hit
  void RemoveCrystalHit(unsigned int i);

  //! Return the number of guardring hits
  unsigned int GetNGuardringHits() const { return m_GuardringHits.size(); }
  //! Return guardring hit i
  MGuardringHit* GetGuardringHit(unsigned int i);
  //! Add a guardring hit
  void AddGuardringHit(MGuardringHit* GuardringHit) { return m_GuardringHits.push_back(GuardringHit); }

  //! Return the number of hits
  unsigned int GetNHits() const { return m_Hits.size(); }
  //! Return hit i
  MHit* GetHit(unsigned int i);
  //! Add a hit
  void AddHit(MHit* Hit) { return m_Hits.push_back(Hit); }
  //! Remove a hit
  void RemoveHit(unsigned int i);

  //! Return the number of simulation hits
  // TODO: Remove - part of m_SimEvent
//  unsigned int GetNHitsSim() const { return m_HitsSim.size(); }
  //! Return simulation hit i
  // TODO: Remove - part of m_SimEvent
//  MHit* GetHitSim(unsigned int i);
  //! Move hits to simulation hits list
  // TODO: Why ??
//  void MoveHitsToSim() {m_HitsSim = m_Hits; m_Hits.clear();}

  /*
  //! Return the number of simulation interactions
  unsigned int GetNSimIAs() const { return m_IAs.size(); }
  //! Return simulation hit i
  MSimIA* GetSimIA(unsigned int i);
  */

  //! Set the physical event from event reconstruction
  void SetPhysicalEvent(MPhysicalEvent* Event);
  //! Return the physical event
  MPhysicalEvent* GetPhysicalEvent() { return m_PhysicalEvent; }

  //! Set the physical event from event reconstruction
  void SetSimulatedEvent(MSimEvent* Event) { m_SimEvent = Event; }
  //! Return the simulated event
  MSimEvent* GetSimulatedEvent() { return m_SimEvent; }

  //! Return the number of low-voltage DEE strip hits
  unsigned int GetNDEEStripHitsLV() const { return m_DEEStripHitsLV.size(); }
  //! Return low-voltage DEE Strip hit at position i
  void AddDEEStripHitLV(MDEEStripHit& DEEStripHit) { return m_DEEStripHitsLV.push_back(DEEStripHit); }
  //! Get a reference to the list of strip hits for direct manipulation
  list<MDEEStripHit>& GetDEEStripHitLVListReference() { return m_DEEStripHitsLV; }

  //! Return the number of high-voltage DEE strip hits
  unsigned int GetNDEEStripHitsHV() const { return m_DEEStripHitsHV.size(); }
  //! Add a high-voltage DEE Strip hit
  void AddDEEStripHitHV(MDEEStripHit DEEStripHit) { return m_DEEStripHitsHV.push_back(DEEStripHit); }
  //! Get a reference to the list of strip hits for direct manipulation
  list<MDEEStripHit>& GetDEEStripHitHVListReference() { return m_DEEStripHitsHV; }

  //! Return the number of crystal hits
  unsigned int GetNDEECrystalHits() const { return m_DEECrystalHits.size(); }
  //! Add a crystal hit
  void AddDEECrystalHit(MDEECrystalHit DEECrystalHit) { return m_DEECrystalHits.push_back(DEECrystalHit); }
  //! Get a reference to the list of crystal hits for direct manipulation
  list<MDEECrystalHit>& GetDEECrystalHitListReference() { return m_DEECrystalHits; }


  //Track BD Flags

  //! Set the energy calibration error flag
  void SetEnergyCalibrationError(MString Text = "") { m_EnergyCalibrationError = true; if (Text != "") { m_EnergyCalibrationErrorString.push_back(Text); }}
  //! Get the energy calibration error flag
  bool HasEnergyCalibrationError() const { return m_EnergyCalibrationError; }
 
 //! Set the strip pairing error flag
  void SetStripPairingError(MString Text = "") { m_StripPairingError = true; if (Text != "") { m_StripPairingErrorString.push_back(Text); }}
  //! Get the strip pairing error flag
  bool HasStripPairingError() const { return m_StripPairingError; }

  //! Set the depth calibration error flag
  void SetDepthCalibrationError(MString Text = "") { m_DepthCalibrationError = true; if (Text != "") { m_DepthCalibrationErrorString.push_back(Text); }}
  //! Get the depth calibration error flag
  bool HasDepthCalibrationError() const { return m_DepthCalibrationError; }

  //! Set the event reconstruction error flag
  void SetEventReconstructionError(MString Text = "") { m_EventReconstructionError = true; if (Text != "") { m_EventReconstructionErrorString.push_back(Text); }}
  //! Get the event reconstruction error flag
  bool HasEventReconstructionError() const { return m_EventReconstructionError; }

  // Track Quality Flags

  //! Set the Strip Hit Below Threshold quality flag
  void SetStripHitBelowThreshold_QualityFlag(MString Text = ""){ m_StripHitBelowThreshold_QualityFlag = true; if (Text != "") { m_StripHitBelowThresholdString_QualityFlag.push_back(Text); }}
  //! Get the Strip Hit Below Threshold quality flag
  bool HasStripHitBelowThreshold_QualityFlag() const { return m_StripHitBelowThreshold_QualityFlag; }
    
  //! Set the Strip Pairing quality flag
  void SetStripPairing_QualityFlag(MString Text = ""){ m_StripPairing_QualityFlag = true;
      if (Text != "") { m_StripPairingString_QualityFlag.push_back(Text); }}
  //! Get the Strip Pairing quality flag
  bool HasStripPairing_QualityFlag() const { return m_StripPairing_QualityFlag; }

  //! Set the Reduced Chi^2 used in MultiRoundChiSquare module (one for each detector)
  void SetStripPairingReducedChiSquare(double StripPairingReducedChiSquare) { m_StripPairingReducedChiSquare.push_back(StripPairingReducedChiSquare); }
  //! Return all the Reduced Chi^2 (for each detector)
  vector<double> GetStripPairingReducedChiSquare() const { return m_StripPairingReducedChiSquare; }


  // Track Vetos

  //! Returns true if any of the "veto" flags have been set
  bool IsVeto() const;


  //! Set the filtered-out flag
  void SetFilteredOut(bool Flag = true) { m_FilteredOut = Flag; }
  //! Get the filgtered-out flag
  bool IsFilteredOut() const { return m_FilteredOut; }
  //! Return the unique assembly identifier
  unsigned long GetAssemblyID() const { return m_AssemblyID; }

  //! Returns true if none of the "bad" or "Error" flags has been set and the event has not been filtered out or rejected
  bool IsGood() const;
  //! Returns true if any of the "bad" or "Error" flags has been set
  bool IsBad() const;

  //! Set a specific analysis progress
  void SetAnalysisProgress(uint64_t Progress) { m_AnalysisProgress |= Progress; }
  //! Check if we have a certain progress
  bool HasAnalysisProgress(uint64_t Progress) const { return (m_AnalysisProgress & Progress) == Progress ? true : false; }
  //! Return the analysis progress flag
  uint64_t GetAnalysisProgress() const { return m_AnalysisProgress; }

  //! Parse some content from a line
  bool Parse(MString& Line, int Version = 1);

  //! Steam the content in a way Nuclearizer can read it in again
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the content in MEGAlib's evta format 
  void StreamEvta(ostream& S);
  //! Stream the content in MEGAlib's evta format
  void StreamTra(ostream& S);
  //! Stream the content in MEGAlib's roa format 
  void StreamRoa(ostream& S, bool WithADCs = true, bool WithTACs = true, bool WithEnergies = false, bool WithTimings = false, bool WithTemperatures = false, bool WithFlags = false, bool WithOrigins = false, bool WithNearestNeighbors = false);

  //! Steam the BD flags
  void StreamBDFlags(ostream& S);

  //! Build the next MReadoutAssemply from a .dat file
  bool GetNextFromDatFile(MFile &F);

  //! Compute the RTS time from known UTC time
  MTime ComputeRTSfromUTCTime(MTime UTCTime);
  //! Compute the UTC time from known RTS
  MTime ComputeUTCfromRTSTime(MTime RTSTime);

  // protected methods:
 protected:
  //MReadOutAssembly() {};
  //MReadOutAssembly(const MReadOutAssembly& ReadOutAssembly) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  //! Unique assembly identifier counter
  static atomic<unsigned long> s_NextAssemblyID;

  //! Unique assembly identifier
  unsigned long m_AssemblyID;

  //! The time of the event in COSI Reference Time System (seconds since Jan 1, 2025) in TT
  MTime m_EventTimeRTS;

  //! The time of the event in absolute UTC time
  MTime m_EventTimeUTC;

  //! The aspect information from the simulation, only used in DEE
  double m_GalacticPointingXAxisTheta;
  double m_GalacticPointingXAxisPhi;
  double m_GalacticPointingZAxisTheta;
  double m_GalacticPointingZAxisPhi;
  bool m_HasSimAspectInfo;

  //! Guard ring veto flag
  bool m_GuardRingVeto;

  //! Shield veto flag
  bool m_ShieldVeto;

  //! Trigger flag of this event
  bool m_Trigger;

  //! Whether event contains strip hits in given detector
  bool m_InDetector[16];

  //! List of strip hits
  vector<MStripHit*> m_StripHits;

  //! List of strip hits with timing only
  vector<MStripHit*> m_StripHitsTOnly;

  //! List of crystal hits
  vector<MCrystalHit*> m_CrystalHits;

  //! List of guardring hits
  vector<MGuardringHit*> m_GuardringHits;

  //! List of real hits
  vector<MHit*> m_Hits;

  //! The simulated event (nullptr if there is none)
  MSimEvent* m_SimEvent;

  //! List of simulation hits
  //! TODO: Remove: Part of m_SimEvent
  vector<MHit*> m_HitsSim;

  //! A list of low voltage DEE strips hit - i.e. normal strip hits in the making from the simulated hits sorted by side
  list<MDEEStripHit> m_DEEStripHitsLV;
  //! A list of high voltage DEE strips hit - i.e. normal strip hits in the making from the simulated hits sorted by side
  list<MDEEStripHit> m_DEEStripHitsHV;
  //! A list of crystal hit - i.e. normal crystal hits in the making from the simulated hits
  list<MDEECrystalHit> m_DEECrystalHits;

  //! The physical event from event reconstruction
  MPhysicalEvent* m_PhysicalEvent;
    
  // Flags indicating bad events:

  //! Energy calibration error flag
  bool m_EnergyCalibrationError;
  //! Energy calibration error string
  vector<MString> m_EnergyCalibrationErrorString;

  //! String pairing error flag
  bool m_StripPairingError;
  //! Strip pairing error string
  vector<MString> m_StripPairingErrorString;

  //! Depth calibration error flag
  bool m_DepthCalibrationError;
  //! Depth calibration error string
  vector<MString> m_DepthCalibrationErrorString;
 
  //! Event reconstruction error flag
  bool m_EventReconstructionError;
  //! Event reconstruction error string
  vector<MString> m_EventReconstructionErrorString;
 
  // Flags indicating the quality of the event: quality warning, but not to be filtered out:

  //! Strip hit below threshold quality flag
  bool m_StripHitBelowThreshold_QualityFlag;
  //! Strip hit below threshold quality string
  vector<MString> m_StripHitBelowThresholdString_QualityFlag;
    
  //! Strip pairing quality flag
  bool m_StripPairing_QualityFlag;
  //! Strip pairing quality string
  vector<MString> m_StripPairingString_QualityFlag;

  //! Reduced Chi^2 of the Strip Paired Event
  vector<double> m_StripPairingReducedChiSquare;

 //! True if event has been filtered out
  bool m_FilteredOut;

  //! The analysis progress 
  uint64_t m_AnalysisProgress;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MReadOutAssembly, 0) // no description
#endif

};

#endif


///////////////////////////////////////////////////////////////////////////////
