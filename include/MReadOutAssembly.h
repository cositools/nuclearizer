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
#include <array>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MPhysicalEvent.h"
#include "MSimEvent.h"
#include "MSimIA.h"

// Nuclearizer libs:
#include "MStripHit.h"
#include "MDEEStripHit.h"
#include "MCrystalHit.h"
#include "MDEECrystalHit.h"
#include "MGuardringHit.h"
#include "MHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The read-out assembly: A container for all read-out data, reconstructed events, and derived event flags
class MReadOutAssembly : public MReadOutSequence
{
  // public interface:
 public:
  //! Default constructor
  MReadOutAssembly();
  //! Default destructor
  virtual ~MReadOutAssembly();

  //! Copying is disabled - the assembly owns raw pointers
  MReadOutAssembly(const MReadOutAssembly&) = delete;
  //! Copy assignment is disabled - the assembly owns raw pointers
  MReadOutAssembly& operator=(const MReadOutAssembly&) = delete;

  //! Reset all data
  virtual void Clear();

  //! Delete all hits
  void DeleteHits();

  //! Set the Reference Time System (RTS) time for this event
  //! The RTS is mission time in seconds since January 1, 2025 in TT (terrestrial time)
  void SetTimeRTS(const MTime& TimeRTS) { m_EventTimeRTS = TimeRTS; }
  //! Return the Reference Time System (RTS) time for this event
  MTime GetTimeRTS() const { return m_EventTimeRTS; }

  //! Set the UTC time of this event
  void SetTimeUTC(const MTime& TimeUTC) { m_EventTimeUTC = TimeUTC; }
  //! Return the UTC time of this event
  MTime GetTimeUTC() const { return m_EventTimeUTC; }

  //! Set and get simulation aspect information
  //! Set the galactic pointing X-axis theta
  void SetGalacticPointingXAxisTheta(double Theta) { m_GalacticPointingXAxisTheta = Theta; }
  //! Set the galactic pointing X-axis phi
  void SetGalacticPointingXAxisPhi(double Phi) { m_GalacticPointingXAxisPhi = Phi; }
  //! Set the galactic pointing Z-axis theta
  void SetGalacticPointingZAxisTheta(double Theta) { m_GalacticPointingZAxisTheta = Theta; }
  //! Set the galactic pointing Z-axis phi
  void SetGalacticPointingZAxisPhi(double Phi) { m_GalacticPointingZAxisPhi = Phi; }

  //! Return the galactic pointing X-axis theta
  double GetGalacticPointingXAxisTheta() const { return m_HasSimAspectInfo ? m_GalacticPointingXAxisTheta : 0; }
  //! Return the galactic pointing X-axis phi
  double GetGalacticPointingXAxisPhi() const { return m_HasSimAspectInfo ? m_GalacticPointingXAxisPhi : 0; }
  //! Return the galactic pointing Z-axis theta
  double GetGalacticPointingZAxisTheta() const { return m_HasSimAspectInfo ? m_GalacticPointingZAxisTheta : 0; }
  //! Return the galactic pointing Z-axis phi
  double GetGalacticPointingZAxisPhi() const { return m_HasSimAspectInfo ? m_GalacticPointingZAxisPhi : 0; }

  //! Set whether simulation aspect information is available
  void SetSimAspectInfo(bool Flag) { m_HasSimAspectInfo = Flag; }
  //! Return true if simulation aspect information is available
  bool HasSimAspectInfo() const { return m_HasSimAspectInfo; }


  //! Find out if the event contains strip hits in a given detector
  bool InDetector(int DetectorID) const;

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
  //! Ownership stays with this object
  MStripHit* GetStripHit(unsigned int i);
  //! Add a strip hit
  //! Ownership is transferred to this object
  void AddStripHit(MStripHit* StripHit);
  //! Remove strip hit i without deleting it
  //! The assembly relinquishes ownership of the removed pointer
  void RemoveStripHit(unsigned int i);

  //! Return the number of crystal hits
  unsigned int GetNCrystalHits() const { return m_CrystalHits.size(); }
  //! Return crystal hit i
  //! Ownership stays with this object
  MCrystalHit* GetCrystalHit(unsigned int i);
  //! Add a crystal hit
  //! Ownership is transferred to this object
  void AddCrystalHit(MCrystalHit* CrystalHit);
  //! Remove crystal hit i without deleting it
  //! The assembly relinquishes ownership of the removed pointer
  void RemoveCrystalHit(unsigned int i);

  //! Return the number of guardring hits
  unsigned int GetNGuardringHits() const { return m_GuardringHits.size(); }
  //! Return guardring hit i
  //! Ownership stays with this object
  MGuardringHit* GetGuardringHit(unsigned int i);
  //! Add a guardring hit
  //! Ownership is transferred to this object
  void AddGuardringHit(MGuardringHit* GuardringHit) { if (GuardringHit != nullptr) m_GuardringHits.push_back(GuardringHit); }

  //! Return the number of hits
  unsigned int GetNHits() const { return m_Hits.size(); }
  //! Return hit i
  //! Ownership stays with this object
  MHit* GetHit(unsigned int i);
  //! Add a hit
  //! Ownership is transferred to this object
  void AddHit(MHit* Hit) { if (Hit != nullptr) m_Hits.push_back(Hit); }
  //! Remove hit i without deleting it
  //! The assembly relinquishes ownership of the removed pointer
  void RemoveHit(unsigned int i);

  //! Set the physical event from event reconstruction by storing a duplicate
  //! Ownership of the supplied pointer stays with the caller
  void SetPhysicalEvent(MPhysicalEvent* Event);
  //! Return the physical event
  //! Ownership stays with this object
  MPhysicalEvent* GetPhysicalEvent() { return m_PhysicalEvent; }

  //! Set the simulated event
  //! Ownership of the supplied pointer is transferred to this object
  //! Any previously stored simulated event is deleted
  void SetSimulatedEvent(MSimEvent* Event) { if (Event != m_SimEvent) { delete m_SimEvent; m_SimEvent = Event; } }
  //! Return the simulated event; returns nullptr if none is set
  //! Ownership stays with this object
  MSimEvent* GetSimulatedEvent() { return m_SimEvent; }

  //! Return the number of low-voltage DEE strip hits
  unsigned int GetNDEEStripHitsLV() const { return m_DEEStripHitsLV.size(); }
  //! Add a low-voltage DEE strip hit
  void AddDEEStripHitLV(const MDEEStripHit& DEEStripHit) { m_DEEStripHitsLV.push_back(DEEStripHit); }
  //! Get a reference to the list of strip hits for direct manipulation
  list<MDEEStripHit>& GetDEEStripHitLVListReference() { return m_DEEStripHitsLV; }

  //! Return the number of high-voltage DEE strip hits
  unsigned int GetNDEEStripHitsHV() const { return m_DEEStripHitsHV.size(); }
  //! Add a high-voltage DEE strip hit
  void AddDEEStripHitHV(const MDEEStripHit& DEEStripHit) { m_DEEStripHitsHV.push_back(DEEStripHit); }
  //! Get a reference to the list of strip hits for direct manipulation
  list<MDEEStripHit>& GetDEEStripHitHVListReference() { return m_DEEStripHitsHV; }

  //! Return the number of crystal hits
  unsigned int GetNDEECrystalHits() const { return m_DEECrystalHits.size(); }
  //! Add a DEE crystal hit
  void AddDEECrystalHit(const MDEECrystalHit& DEECrystalHit) { m_DEECrystalHits.push_back(DEECrystalHit); }
  //! Get a reference to the list of crystal hits for direct manipulation
  list<MDEECrystalHit>& GetDEECrystalHitListReference() { return m_DEECrystalHits; }


  // Track BD flags

  //! Set the energy calibration error flag
  void SetEnergyCalibrationError(const MString& Text = "") { m_EnergyCalibrationError = true; if (Text != "") m_EnergyCalibrationErrorString.push_back(Text); }
  //! Get the energy calibration error flag
  bool HasEnergyCalibrationError() const { return m_EnergyCalibrationError; }

  //! Set the strip pairing error flag
  void SetStripPairingError(const MString& Text = "") { m_StripPairingError = true; if (Text != "") m_StripPairingErrorString.push_back(Text); }
  //! Get the strip pairing error flag
  bool HasStripPairingError() const { return m_StripPairingError; }

  //! Set the depth calibration error flag
  void SetDepthCalibrationError(const MString& Text = "") { m_DepthCalibrationError = true; if (Text != "") m_DepthCalibrationErrorString.push_back(Text); }
  //! Get the depth calibration error flag
  bool HasDepthCalibrationError() const { return m_DepthCalibrationError; }

  //! Set the event reconstruction error flag
  void SetEventReconstructionError(const MString& Text = "") { m_EventReconstructionError = true; if (Text != "") m_EventReconstructionErrorString.push_back(Text); }
  //! Get the event reconstruction error flag
  bool HasEventReconstructionError() const { return m_EventReconstructionError; }

  // Track Quality Flags

  //! Set the Strip Hit Below Threshold quality flag
  void SetStripHitBelowThreshold_QualityFlag(const MString& Text = "") { m_StripHitBelowThreshold_QualityFlag = true; if (Text != "") m_StripHitBelowThresholdString_QualityFlag.push_back(Text); }
  //! Get the Strip Hit Below Threshold quality flag
  bool HasStripHitBelowThreshold_QualityFlag() const { return m_StripHitBelowThreshold_QualityFlag; }

  //! Set the Strip Pairing quality flag
  void SetStripPairing_QualityFlag(const MString& Text = "") { m_StripPairing_QualityFlag = true; if (Text != "") m_StripPairingString_QualityFlag.push_back(Text); }
  //! Get the Strip Pairing quality flag
  bool HasStripPairing_QualityFlag() const { return m_StripPairing_QualityFlag; }

  //! Set the reduced chi^2 used in the MultiRoundChiSquare module (one for each detector)
  void SetStripPairingReducedChiSquare(double StripPairingReducedChiSquare) { m_StripPairingReducedChiSquare.push_back(StripPairingReducedChiSquare); }
  //! Return all the reduced chi^2 values (one for each detector)
  vector<double> GetStripPairingReducedChiSquare() const { return m_StripPairingReducedChiSquare; }


  // Track vetoes

  //! Returns true if any of the "veto" flags have been set
  bool IsVeto() const;


  //! Set the filtered-out flag
  void SetFilteredOut(bool Flag = true) { m_FilteredOut = Flag; }
  //! Get the filtered-out flag
  bool IsFilteredOut() const { return m_FilteredOut; }
  //! Return the unique assembly identifier
  unsigned long GetAssemblyID() const { return m_AssemblyID; }

  //! Return true if no error flag is set and the event has not been filtered out
  //! Veto and quality flags do not affect this result
  bool IsGood() const;
  //! Return true if any error flag is set or the event has been filtered out
  //! Veto and quality flags do not affect this result
  bool IsBad() const;
  //! Returns true if any of the Quality flags have been set
  bool IsPoorQuality() const;

  //! Set a specific analysis progress
  void SetAnalysisProgress(uint64_t Progress) { m_AnalysisProgress |= Progress; }
  //! Check if we have a certain progress
  bool HasAnalysisProgress(uint64_t Progress) const { return (m_AnalysisProgress & Progress) == Progress ? true : false; }
  //! Return the analysis progress flag
  uint64_t GetAnalysisProgress() const { return m_AnalysisProgress; }

  //! Parse a read-out assembly from a line
  bool Parse(MString& Line, int Version = 1);

  //! Stream the read-out assembly in a way Nuclearizer can read it in again
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the read-out assembly in MEGAlib's EVTA format
  void StreamEvta(ostream& S);
  //! Stream the read-out assembly in MEGAlib's TRA format
  void StreamTra(ostream& S);
  //! Stream the read-out assembly in MEGAlib's ROA format
  void StreamRoa(ostream& S, bool WithADCs = true, bool WithTACs = true, bool WithEnergies = false, bool WithTimings = false, bool WithTemperatures = false, bool WithFlags = false, bool WithOrigins = false, bool WithNearestNeighbors = false);

  //! Stream the BD flags
  void StreamBDFlags(ostream& S);

  //! Build the next MReadOutAssembly from a `.dat` file
  bool GetNextFromDatFile(MFile& F);

  //! Compute the RTS time from known UTC time
  //! BUG: Fix leap-second issue and move to MTimeConversions class
  MTime ComputeRTSfromUTCTime(MTime UTCTime) const;
  //! Compute the UTC time from known RTS
  //! BUG: Fix leap-second issue and move to MTimeConversions class
  MTime ComputeUTCfromRTSTime(MTime RTSTime) const;
  //! Compute the RTS time from GPS time
  //! BUG: Fix leap-second issue and move to MTimeConversions class
  MTime ComputeRTSfromGPSTime(MTime GPSTime) const;
  //! Compute GPS time from known RTS
  //! BUG: Fix leap-second issue and move to MTimeConversions class
  MTime ComputeGPSfromRTSTime(MTime RTSTime) const;


  // protected methods:
 protected:

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

  //! The time of the event in COSI Reference Time System (seconds since January 1, 2025) in TT (terrestrial time)
  MTime m_EventTimeRTS;

  //! The time of the event in absolute UTC time
  MTime m_EventTimeUTC;

  //! The galactic pointing X-axis theta from simulation aspect information, used only in DEE
  double m_GalacticPointingXAxisTheta;
  //! The galactic pointing X-axis phi from simulation aspect information, used only in DEE
  double m_GalacticPointingXAxisPhi;
  //! The galactic pointing Z-axis theta from simulation aspect information, used only in DEE
  double m_GalacticPointingZAxisTheta;
  //! The galactic pointing Z-axis phi from simulation aspect information, used only in DEE
  double m_GalacticPointingZAxisPhi;
  //! True if simulation aspect information is available
  bool m_HasSimAspectInfo;

  //! Guard ring veto flag
  bool m_GuardRingVeto;

  //! Shield veto flag
  bool m_ShieldVeto;

  //! Trigger flag of this event
  bool m_Trigger;

  //! Whether event contains strip hits in given detector
  array<bool, 16> m_InDetector;

  //! List of strip hits
  //! Ownership stays with this object
  vector<MStripHit*> m_StripHits;

  //! List of crystal hits
  //! Ownership stays with this object
  vector<MCrystalHit*> m_CrystalHits;

  //! List of guardring hits
  //! Ownership stays with this object
  vector<MGuardringHit*> m_GuardringHits;

  //! List of real hits
  //! Ownership stays with this object
  vector<MHit*> m_Hits;

  //! The simulated event (nullptr if there is none)
  //! Ownership stays with this object
  MSimEvent* m_SimEvent;

  //! A list of low-voltage DEE strip hits, i.e. normal strip hits in the making from the simulated hits sorted by side
  list<MDEEStripHit> m_DEEStripHitsLV;
  //! A list of high-voltage DEE strip hits, i.e. normal strip hits in the making from the simulated hits sorted by side
  list<MDEEStripHit> m_DEEStripHitsHV;
  //! A list of crystal hits, i.e. normal crystal hits in the making from the simulated hits
  list<MDEECrystalHit> m_DEECrystalHits;

  //! The physical event from event reconstruction
  //! Ownership stays with this object
  MPhysicalEvent* m_PhysicalEvent;

  // Flags indicating bad events:

  //! Energy calibration error flag
  bool m_EnergyCalibrationError;
  //! Energy calibration error string
  vector<MString> m_EnergyCalibrationErrorString;

  //! Strip pairing error flag
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

  //! The reduced chi^2 values of the strip-paired event
  vector<double> m_StripPairingReducedChiSquare;

  //! True if the event has been filtered out
  bool m_FilteredOut;

  //! The analysis progress
  uint64_t m_AnalysisProgress;


#ifdef ___CLING___
 public:
  ClassDef(MReadOutAssembly, 0) // a read-out assembly
#endif

};

#endif


///////////////////////////////////////////////////////////////////////////////
