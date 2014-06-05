/*
 * MNCTEvent.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTEvent__
#define __MNCTEvent__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MNCTStripHit.h"
#include "MNCTGuardringHit.h"
#include "MNCTHit.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTEvent
{
  // public interface:
 public:
  //! Default constructor
  MNCTEvent();
  //! Default destructor
  virtual ~MNCTEvent();

  //! Reset all data
  void Clear();

  //! Delete Hits
  void DeleteHits();
  
  //! Set the ID of this event
  void SetID(unsigned int ID) { m_ID = ID; }
  //! Return the ID of this event
  unsigned int GetID() const { return m_ID; }

  //! Set the Frame Counter of this event
  void SetFC(unsigned int FC) { m_FC = FC; }
  //! Return the Frame Counter of this event
  unsigned int GetFC() const { return m_FC; }

  //! set and get Unix clock time
  void SetTI(unsigned long long TI) { m_TI = TI;}
  unsigned long long GetTI() const { return m_TI;}

  //! set and get clock tick
  void SetCL(unsigned long CL) { m_CL = CL;}
  unsigned long GetCL() const { return m_CL;}

  //! Set and get the Time of this event
  void SetTime(double Time) { m_Time = Time; }
  double GetTime() const { return m_Time; }

  //! Set and get the Modified Julian Date of this event
  void SetMJD(double MJD) { m_MJD = MJD; }
  double GetMJD() const { return m_MJD; }

  //! Set and get the latitude, longitude, and altitude of this event
  void SetLatitude(double lat) { m_Latitude = lat; }
  double GetLatitude() const { return m_Latitude; }
  void SetLongitude(double lon) { m_Longitude = lon; }
  double GetLongitude() const { return m_Longitude; }
  void SetAltitude(double alt) { m_Altitude = alt; }
  double GetAltitude() const { return m_Altitude; }

  //! Set Aspect of this event (galactic coordinates)
  void SetGX(vector<double> GX) {m_GX = GX;}
  void SetGZ(vector<double> GZ) {m_GZ = GZ;}

  //! Get Aspect of this event (galactic coordinates)
  vector<double> GetGX() const { return m_GX;}
  vector<double> GetGZ() const { return m_GZ;}

  //! Set Aspect of this event (local horizon coordinates)
  void SetHX(vector<double> HX) {m_HX = HX;}
  void SetHZ(vector<double> HZ) {m_HZ = HZ;}

  //! Get Aspect of this event (local horizon coordinates)
  vector<double> GetHX() const { return m_HX;}
  vector<double> GetHZ() const { return m_HZ;}

  //! Find out if the event contains strip hits in a given detector
  bool InDetector(int DetectorID);

  //! Set the vetoed flag
  void SetVeto(bool Veto = true) { m_Veto = Veto; }
  //! Return the veto flag
  bool GetVeto() const { return m_Veto; }

  //! Set the triggered flag
  void SetTrigger(bool Trigger = true) { m_Trigger = Trigger; }
  //! Return the trigger flag
  bool GetTrigger() const { return m_Trigger; }

  //! Return the number of strip hits
  unsigned int GetNStripHits() const { return m_StripHits.size(); }
  //! Return strip hit i
  MNCTStripHit* GetStripHit(unsigned int i);
  //! Add a strip hit
  void AddStripHit(MNCTStripHit* StripHit);
  //! Remove a strip hit
  void RemoveStripHit(unsigned int i);

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


  //! Set the data read flag
  void SetDataRead(bool Flag = true) { m_DataRead = Flag; }
  //! Return the data read flag
  bool IsDataRead() const { return m_DataRead; }

  //! Set the data read flag
  void SetEnergyCalibrated(bool Flag = true) { m_EnergyCalibrated = Flag; }
  //! Return the data read flag
  bool IsEnergyCalibrated() const { return m_EnergyCalibrated; }

  //! Set the cross-talk corrected flag
  void SetCrosstalkCorrected(bool Flag = true) { m_CrosstalkCorrected = Flag; }
  //! Return the cross-talk corrected flag
  bool IsCrosstalkCorrected() const { return m_CrosstalkCorrected; }

  //! Set the charge sharing corrected flag
  void SetChargeSharingCorrected(bool Flag = true) { m_ChargeSharingCorrected = Flag; }
  //! Return the charge sharing corrected flag
  bool IsChargeSharingCorrected() const { return m_ChargeSharingCorrected; }

  //! Set the depth calibrated flag
  void SetDepthCalibrated(bool Flag = true) { m_DepthCalibrated = Flag; }
  //! Return the depth calibrated flag
  bool IsDepthCalibrated() const { return m_DepthCalibrated; }

  //! Set the strips paired flag
  void SetStripsPaired(bool Flag = true) { m_StripsPaired = Flag; }
  //! Return the strips paired flag
  bool IsStripsPaired() const { return m_StripsPaired; }

  //! 
  void SetAspectAdded(bool Flag = true) { m_AspectAdded = Flag; }
  //!
  bool IsAspectAdded() {return m_AspectAdded; }

  //! Set the reconstructed flag
  void SetReconstructed(bool Flag = true) { m_Reconstructed = Flag; }
  //! Return the reconstructed flag
  bool IsReconstructed() const { return m_Reconstructed; }

  //! Set the Quality of this Event
  void SetEventQuality(double EventQuality){ m_EventQuality = EventQuality; }
  //!Return the Quality of this Event
  double GetEventQuality() const { return m_EventQuality; }




  // protected methods:
 protected:
  //MNCTEvent() {};
  //MNCTEvent(const MNCTEvent& NCTEvent) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! ID of this event
  unsigned int m_ID;

  //! Frame Counter of this event
  unsigned int m_FC;

  //! Clock tick (Unix and UHF)
  unsigned long long m_TI;
  unsigned long m_CL;

  //! Time and MJD of this event
  double m_Time;
  double m_MJD;

  //! Location of this event
  double m_Latitude;
  double m_Longitude;
  double m_Altitude;

  //! Aspect information (horizon, celestial, galactic coordinates)
  vector<double> m_GX;
  vector<double> m_GZ;
  vector<double> m_HX;
  vector<double> m_HZ;

  //! Quality of this event
  double m_EventQuality;

  //! Veto flag of this event
  bool m_Veto;

  //! Trigger flag of this event
  bool m_Trigger;

  //! Whether event contains strip hits in given detector
  bool m_InDetector[10];

  //! List of strip hits
  vector<MNCTStripHit*> m_StripHits;

  //! List of guardring hits
  vector<MNCTGuardringHit*> m_GuardringHits;

  //! List of real hits
  vector<MNCTHit*> m_Hits;

  //! List of simulation hits
  vector<MNCTHit*> m_HitsSim;

  //! The physical event from event reconstruction
  MPhysicalEvent* m_PhysicalEvent; 

  // Flags indicating the analysis level of the event
  bool m_DataRead;
  bool m_EnergyCalibrated;
  bool m_CrosstalkCorrected;
  bool m_ChargeSharingCorrected;
  bool m_DepthCalibrated;
  bool m_StripsPaired;
  bool m_AspectAdded;
  bool m_Reconstructed;


#ifdef ___CINT___
 public:
  ClassDef(MNCTEvent, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
