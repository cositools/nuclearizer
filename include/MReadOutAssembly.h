/*
 * MReadOutAssembly.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutAssembly__
#define __MReadOutAssembly__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOut.h"
#include "MNCTAspect.h"
#include "MNCTStripHit.h"
#include "MNCTGuardringHit.h"
#include "MNCTHit.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MReadOutAssembly
{
  // public interface:
 public:
  //! Default constructor
  MReadOutAssembly();
  //! Default destructor
  virtual ~MReadOutAssembly();

  //! Reset all data
  void Clear();

  //! Delete Hits
  void DeleteHits();
  
  //! Set the ID of this event
  void SetID(unsigned long ID) { m_ID = ID; }
  //! Return the ID of this event
  unsigned long GetID() const { return m_ID; }

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

  //! Set and get the Time of this event
  void SetTime(MTime Time) { m_Time = Time; }
  MTime GetTime() const { return m_Time; }

  //! Set and get the Modified Julian Date of this event
  void SetMJD(double MJD) { m_MJD = MJD; }
  double GetMJD() const { return m_MJD; }
  
  //! Set the aspect
  void SetAspect(MNCTAspect* Aspect) { if (m_Aspect != 0) delete m_Aspect;  m_Aspect = Aspect; }
  //! Get the aspect - will be zero if the aspect has not been set!
  MNCTAspect* GetAspect() { return m_Aspect; }
  
  //! Find out if the event contains strip hits in a given detector
  bool InDetector(int DetectorID);

  //! Set the vetoed flag
  void SetVeto(bool Veto = true) { m_Veto = Veto; }
  //! Return the veto flag
  bool GetVeto() const { return m_Veto; }

  //! Set the guard ring 0 veto flag
  void SetGR0Veto(bool Veto = true) { m_VetoGR0 = Veto; }
  //! Get the guard ring 0 veto flag
  bool GetGR0Veto() const { return m_VetoGR0; }

  //! Set the guard ring 1 veto flag
  void SetGR1Veto(bool Veto = true) { m_VetoGR1 = Veto; }
  //! Get the guard ring 1 veto flag
  bool GetGR1Veto() const { return m_VetoGR1; }

  //! Set the shield veto flag
  void SetShieldVeto(bool Veto = true) { m_VetoShield = Veto; }
  //! Get the shield veto flag
  bool GetShieldVeto() const { return m_VetoShield; }

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
  MNCTStripHit* GetStripHit(unsigned int i);
  //! Add a strip hit
  void AddStripHit(MNCTStripHit* StripHit);
  //! Remove a strip hit
  void RemoveStripHit(unsigned int i);

  //! Return the number of T Only strip hits
  unsigned int GetNStripHitsTOnly() const { return m_StripHitsTOnly.size(); }
  //! Return strip hit i
  MNCTStripHit* GetStripHitTOnly(unsigned int i);
  //! Adda T Only strip hit
  void AddStripHitTOnly(MNCTStripHit*);
  //! Remove a strip hit
  void RemoveStripHitTOnly(unsigned int i);


  //! Return the number of guardring hits
  unsigned int GetNGuardringHits() const { return m_GuardringHits.size(); }
  //! Return guardring hit i
  MNCTGuardringHit* GetGuardringHit(unsigned int i);
  //! Add a guardring hit
  void AddGuardringHit(MNCTGuardringHit* GuardringHit) { return m_GuardringHits.push_back(GuardringHit); }

  //! Return the number of hits
  unsigned int GetNHits() const { return m_Hits.size(); }
  //! Return hit i
  MNCTHit* GetHit(unsigned int i);
  //! Add a hit
  void AddHit(MNCTHit* Hit) { return m_Hits.push_back(Hit); }

  //! Return the number of simulation hits
  unsigned int GetNHitsSim() const { return m_HitsSim.size(); }
  //! Return simulation hit i
  MNCTHit* GetHitSim(unsigned int i);
  //! Move hits to simulation hits list
  void MoveHitsToSim() {m_HitsSim = m_Hits; m_Hits.clear();}

  //! Set the physical event from event reconstruction
  void SetPhysicalEvent(MPhysicalEvent* Event);
  //! Return the physical event 
  MPhysicalEvent* GetPhysicalEvent() { return m_PhysicalEvent; }

  //! Return the number of read outs
  //unsigned int GetNReadOuts() const { return m_ReadOuts.size(); }
  //! Return read out i - throws an exception of the index is not found
  //MReadOut& GetReadOut(unsigned int i);
  //! Add a read out
  void AddReadOut(MReadOut& ReadOut) {}
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
  void StreamRoa(ostream& S, bool WithDescriptor = true);

  //! Build the next MReadoutAssemply from a .dat file
  bool GetNextFromDatFile(MFile &F);


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
  unsigned long m_ID;

  //! Frame Counter of this event
  unsigned int m_FC;

  //! Clock tick (Unix and UHF)
  unsigned long long m_TI;
  uint64_t m_CL;

  //! Time and MJD of this event
  MTime m_Time;
  double m_MJD;

  //! The aspect information - will be zero if not set!
  MNCTAspect* m_Aspect;
  
  //! Quality of this event
  double m_EventQuality;

  //! Veto flag of this event
  bool m_Veto;

  //! Guard ring 0 veto flag
  bool m_VetoGR0;

  //! Guard ring 1 veto flag
  bool m_VetoGR1;

  //! Shield veto flag
  bool m_VetoShield;

  //! Trigger flag of this event
  bool m_Trigger;

  //! True if the aspect data of the event is good
  bool m_AspectGood;

  //! Whether event contains strip hits in given detector
  bool m_InDetector[12];

  //! List of strip hits
  vector<MNCTStripHit*> m_StripHits;

  //! List of strip hits with timing only
  vector<MNCTStripHit*> m_StripHitsTOnly;

  //! List of guardring hits
  vector<MNCTGuardringHit*> m_GuardringHits;

  //! List of real hits
  vector<MNCTHit*> m_Hits;

  //! List of simulation hits
  vector<MNCTHit*> m_HitsSim;

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
  
  
#ifdef ___CINT___
 public:
  ClassDef(MReadOutAssembly, 0) // no description
#endif

};

#endif


///////////////////////////////////////////////////////////////////////////////
