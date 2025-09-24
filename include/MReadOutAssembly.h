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

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MAspect.h"
#include "MStripHit.h"
#include "MGuardringHit.h"
#include "MHit.h"
#include "MPhysicalEvent.h"
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
  
  /*
  //! Set the ID of this event
  void SetID(unsigned long ID) { m_ID = ID; }
  //! Return the ID of this event
  unsigned long GetID() const { return m_ID; }
  */
  
  //! Set the Frame Counter of this event
  void SetFC(unsigned int FC) { m_FC = FC; }
  //! Return the Frame Counter of this event
  unsigned int GetFC() const { return m_FC; }

  //! set and get Unix clock time
  void SetTI(unsigned long long TI) { m_TI = TI;}
  unsigned long long GetTI() const { return m_TI;}

  //! set and get clock tick
  void SetCL(uint64_t CL) { m_CL = CL;}
  uint64_t GetCL() const { return m_CL;}

  /*
  //! Set and get the Time of this event
  void SetTime(MTime Time) { m_Time = Time; }
  MTime GetTime() const { return m_Time; }
  */
  
  //! Set and get the Modified Julian Date of this event
  void SetMJD(double MJD) { m_MJD = MJD; }
  double GetMJD() const { return m_MJD; }
  
  //! Set and get the UTC time of this event
  void SetTimeUTC(const MTime& TimeUTC) { m_EventTimeUTC = TimeUTC; }
  MTime GetTimeUTC() const { return m_EventTimeUTC; }
  
  //! Set the aspect
  void SetAspect(MAspect* Aspect) { if (m_Aspect != 0) delete m_Aspect;  m_Aspect = Aspect; }
  //! Get the aspect - will be zero if the aspect has not been set!
  MAspect* GetAspect() { return m_Aspect; }
  
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

  //! Set the aspect good flag
  void SetAspectGood(bool AspectGood = true) { m_AspectGood = AspectGood; }
  //! Return the aspect good flag
  bool GetAspectGood() const { return m_AspectGood; }

  //! Return the number of strip hits
  unsigned int GetNStripHits() const { return m_StripHits.size(); }
  //! Return strip hit i
  MStripHit* GetStripHit(unsigned int i);
  //! Add a strip hit
  void AddStripHit(MStripHit* StripHit);
  //! Remove a strip hit
  void RemoveStripHit(unsigned int i);

  //! Return the number of T Only strip hits
  unsigned int GetNStripHitsTOnly() const { return m_StripHitsTOnly.size(); }
  //! Return strip hit i
  MStripHit* GetStripHitTOnly(unsigned int i);
  //! Adda T Only strip hit
  void AddStripHitTOnly(MStripHit*);
  //! Remove a strip hit
  void RemoveStripHitTOnly(unsigned int i);


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
  unsigned int GetNHitsSim() const { return m_HitsSim.size(); }
  //! Return simulation hit i
  MHit* GetHitSim(unsigned int i);
  //! Move hits to simulation hits list
  void MoveHitsToSim() {m_HitsSim = m_Hits; m_Hits.clear();}

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

  //! Return the number of read outs
  //unsigned int GetNReadOuts() const { return m_ReadOuts.size(); }
  //! Return read out i - throws an exception of the index is not found
  //MReadOut& GetReadOut(unsigned int i);
  //! Add a read out
  // void AddReadOut(MReadOut& ReadOut) {}
  //! Remove a read out - does do nothing if the index is not found
  //void RemoveReadOut(unsigned int i);



  //! Set the aspect-incomplete flag
  void SetAspectIncomplete(bool Flag = true, MString Text = "") { m_AspectIncomplete = Flag; m_AspectIncompleteString = Text; }
  //! Get the aspect-incomplete flag
  bool IsAspectIncomplete() const { return m_AspectIncomplete; }

  //! Set the time-incomplete flag
  void SetTimeIncomplete(bool Flag = true, MString Text = "") { m_TimeIncomplete = Flag;  m_TimeIncompleteString = Text; }
  //! Get the time-incomplete flag
  bool IsTimeIncomplete() const { return m_TimeIncomplete; }
  
   //! Set the energy-calibration-incomplete flag for strips without a calibration
  void SetEnergyCalibrationIncomplete_BadStrip(bool Flag = true, MString Text = "") { m_EnergyCalibrationIncomplete_BadStrip = Flag;  m_EnergyCalibrationIncomplete_BadStripString = Text; }
  //! Get the energy-calibration-incomplete flag for strips without a calibration
  bool IsEnergyCalibrationIncomplete_BadStrip() const { return m_EnergyCalibrationIncomplete_BadStrip; }

  //! Set the energy-calibration-incomplete flag
  void SetEnergyCalibrationIncomplete(bool Flag = true, MString Text = "") { m_EnergyCalibrationIncomplete = Flag;  m_EnergyCalibrationIncompleteString = Text; }
  //! Get the energy-calibration-incomplete flag
  bool IsEnergyCalibrationIncomplete() const { return m_EnergyCalibrationIncomplete; }

  //! Set the energy resolution calibration incomplete flag
  void SetEnergyResolutionCalibrationIncomplete(bool Flag = true, MString Text = "") { m_EnergyResolutionCalibrationIncomplete = Flag; m_EnergyResolutionCalibrationIncompleteString = Text;}
  //! Get the energy resolution calibration incomplete flag
  bool IsEnergyResolutionCalibrationIncomplete() const { return m_EnergyResolutionCalibrationIncomplete; }

 //! Set the strip-pairing-incomplete flag
  void SetStripPairingIncomplete(bool Flag = true, MString Text = "") { m_StripPairingIncomplete = Flag;  m_StripPairingIncompleteString = Text; }
  //! Get the strip-pairing-incomplete flag
  bool IsStripPairingIncomplete() const { return m_StripPairingIncomplete; }

  //! Set the LLD Event flag
  void SetLLDEvent(bool Flag = true, MString Text = "") { m_LLDEvent = Flag; m_LLDEventString = Text; }
  //! Get the LLD Event flag
  bool IsLLDEvent() const {return m_LLDEvent;}

  //! Set the depth-calibration-incomplete flag
  void SetDepthCalibrationIncomplete(bool Flag = true, MString Text = "") { m_DepthCalibrationIncomplete = Flag;  m_DepthCalibrationIncompleteString = Text; }
  //! Get the depth-calibration-incomplete flag
  bool IsDepthCalibrationIncomplete() const { return m_DepthCalibrationIncomplete; }

  //! Set the depth-calibration out of range flag
  void SetDepthCalibration_OutofRange(bool Flag = true, MString Text = "") {m_DepthCalibration_OutofRange = Flag; m_DepthCalibration_OutofRangeString = Text; }
  //! Get the depth calibration out of range flag
  bool IsDepthCalibration_OutofRange() const { return m_DepthCalibration_OutofRange; }

  //! Set the filtered-out flag
  void SetFilteredOut(bool Flag = true) { m_FilteredOut = Flag; }
  //! Get the filgtered-out flag
  bool IsFilteredOut() const { return m_FilteredOut; }

  //! Returns true if any of the "veto" flags have been set
  bool IsVeto() const;

  //! Returns true if none of the "bad" or "incomplete" flags has been set and the event has not been filtered out or rejected
  bool IsGood() const;
  //! Returns true if any of the "bad" or "incomplete" flags has been set
  bool IsBad() const;

  //! Set a specific analysis progress
  void SetAnalysisProgress(uint64_t Progress) { m_AnalysisProgress |= Progress; }
  //! Check if we have a certain progress
  bool HasAnalysisProgress(uint64_t Progress) const { return (m_AnalysisProgress & Progress) == Progress ? true : false; }
  //! Return the analysis progress flag
  uint64_t GetAnalysisProgress() const { return m_AnalysisProgress; }
  //! Set the Quality of this Event
  void SetEventQuality(double EventQuality){ m_EventQuality = EventQuality; }
  //!Return the Quality of this Event
  double GetEventQuality() const { return m_EventQuality; }
  //! Parse some content from a line
  bool Parse(MString& Line, int Version = 1);
  //! Steam the content in a way Nuclearizer can read it in again
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the content in MEGAlib's evta format 
  void StreamEvta(ostream& S);
  //! Stream the content in MEGAlib's roa format 
  void StreamRoa(ostream& S, bool WithADCs = true, bool WithTACs = true, bool WithEnergies = false, bool WithTimings = false, bool WithTemperatures = false, bool WithFlags = false, bool WithOrigins = false, bool WithNearestNeighbors = false);
  //! Build the next MReadoutAssemply from a .dat file
  bool GetNextFromDatFile(MFile &F);
  //! Use the info in m_Aspect to turn m_CL into an absolute UTC time
  bool ComputeAbsoluteTime();
  //! Set the MTime corresponding to absolute UTC time
  void SetAbsoluteTime(MTime T) {m_EventTimeUTC = T;}
  //! Get the MTime corresponding to absolute UTC time
  MTime GetAbsoluteTime() const {return m_EventTimeUTC; }


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
  //! ID of this event
  // unsigned long m_ID; // in base class

  //! Frame Counter of this event
  unsigned int m_FC;

  //! Clock tick (Unix and UHF)
  unsigned long long m_TI;
  uint64_t m_CL;

  //! Time and MJD of this event
  double m_MJD;
  // MTime m_Time; // in base class

  //! The time of the event in absolute UTC time
  MTime m_EventTimeUTC;

  //! The aspect information - will be zero if not set!
  MAspect* m_Aspect;

	//Added by Clio:
	//! The aspect information from the simulation, only used in DEE
	// (Simulation aspect information doesn't have everything in Aspect packet)
	double m_GalacticPointingXAxisTheta;
	double m_GalacticPointingXAxisPhi;
	double m_GalacticPointingZAxisTheta;
	double m_GalacticPointingZAxisPhi;
	bool m_HasSimAspectInfo;
 
  //! Quality of this event
  double m_EventQuality;

  //! Guard ring veto flag
  bool m_GuardRingVeto;

  //! Shield veto flag
  bool m_ShieldVeto;

  //! Trigger flag of this event
  bool m_Trigger;

  //! True if the aspect data of the event is good
  bool m_AspectGood;

  //! Whether event contains strip hits in given detector
  bool m_InDetector[12];

  //! List of strip hits
  vector<MStripHit*> m_StripHits;

  //! List of strip hits with timing only
  vector<MStripHit*> m_StripHitsTOnly;

  //! List of guardring hits
  vector<MGuardringHit*> m_GuardringHits;

  //! List of real hits
  vector<MHit*> m_Hits;

  //! List of simulation hits
  vector<MHit*> m_HitsSim;

  //! The physical event from event reconstruction
  MPhysicalEvent* m_PhysicalEvent; 

  // Flags indicating the quality of the event
  bool m_AspectIncomplete;
  MString m_AspectIncompleteString;
  bool m_TimeIncomplete;
  MString m_TimeIncompleteString;
  bool m_EnergyCalibrationIncomplete_BadStrip;
  MString m_EnergyCalibrationIncomplete_BadStripString;
  bool m_EnergyCalibrationIncomplete;
  MString m_EnergyCalibrationIncompleteString;
  bool m_EnergyResolutionCalibrationIncomplete;
  MString m_EnergyResolutionCalibrationIncompleteString;
  bool m_StripPairingIncomplete;
  MString m_StripPairingIncompleteString;
  bool m_LLDEvent;
  MString m_LLDEventString;
  bool m_DepthCalibrationIncomplete;
  MString m_DepthCalibrationIncompleteString;
  bool m_DepthCalibration_OutofRange;
  MString m_DepthCalibration_OutofRangeString;  



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
