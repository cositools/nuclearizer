/*
 * MSubModuleShieldTrigger.h
 *
 * Copyright (C) by Andreas Zoglauer, Parshad Patel.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleShieldTrigger__
#define __MSubModuleShieldTrigger__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <unordered_set>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"
#include "MSimEvent.h"
#include "MSimHT.h"
#include "MDVolumeSequence.h"
#include "MDDetector.h"

// Nuclearizer libs:
#include "MDEECrystalHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleShieldTrigger : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleShieldTrigger();

  //! No copy constructor
  MSubModuleShieldTrigger(const MSubModuleShieldTrigger&) = delete;
  //! No copy assignment
  MSubModuleShieldTrigger& operator=(const MSubModuleShieldTrigger&) = delete;
  //! No move constructors
  MSubModuleShieldTrigger(MSubModuleShieldTrigger&&) = delete;
  //! No move operators
  MSubModuleShieldTrigger& operator=(MSubModuleShieldTrigger&&) = delete;

  //! Set deadtime parameters file name
  void SetDeadtimeFileName(const MString& FileName)
  {
    m_DeadtimeFileName = FileName;
  }
  //! Get deadtime parameters file name
  MString GetDeadtimeFileName() const
  {
    return m_DeadtimeFileName;
  }

  //! Default destructor
  virtual ~MSubModuleShieldTrigger();

  //! Initialize the module
  virtual bool Initialize();

  //! Clear event data from the module
  virtual void Clear();

  //! Main data analysis routine
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Return true if we have a trigger - filled after AnalyzeEvent
  bool HasTrigger() const { return m_HasTrigger; }

  //! Return true if we have a veto - filled after AnalyzeEvent
  bool HasVeto() const { return m_HasVeto; }

  //! Return the time when the dead time ends - filled after AnalyzeEvent
  MTime GetDeadTimeEnd() const { return m_DeadTimeEnd; }

  //! Get total shield deadtime for a panel
  double GetTotalShieldDeadtime(int panel) const { 
    if (panel >= 0 && panel < nShieldPanels) return m_TotalShieldDeadtime[panel];
    return 0.0;
  }

  //! Get shield hit counts
  int GetShieldHitCounts() const { return m_NumShieldHitCounts; }

  //! Get BGO hits erased
  int GetBGOHitsErased() const { return m_NumBGOHitsErased; }

  //! Check if shield is currently dead
  bool IsShieldDead() const { return m_IsShieldDead; }

  //! Finalize the module
  virtual void Finalize();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration(MXmlNode* Node);

  // protected methods:
 protected:
  //! Parse deadtime file
  bool ParseDeadtimeFile();

  //! Calculate ASIC deadtime for shield
  double CalculateASICDeadtime(vector<int> CrystalIDs);

  //! Process shield hits and determine veto status
  bool ProcessShieldHits(MReadOutAssembly* Event);

  // private members:
 private:
  //! Deadtime parameters file name
  MString m_DeadtimeFileName;

  //! Current event time
  double m_EventTime;

  //! Flag indicating that a trigger has been raised
  bool m_HasTrigger;
  //! Flag indicating that a veto has been raised
  bool m_HasVeto;
  //! Time when the shield dead time ends
  MTime m_DeadTimeEnd;

  //! Shield threshold in keV
  double m_ShieldThreshold;
  //! Shield pulse duration in seconds
  double m_ShieldPulseDuration;
  //! Shield delay 1 before trigger in seconds
  double m_ShieldDelayBefore;
  //! Shield delay 2 before trigger in seconds
  double m_ShieldDelayAfter;
  //! Shield veto window size in seconds
  double m_ShieldVetoWindowSize;
  //! Shield deadtime per channel read out in seconds
  double m_ASICDeadTimePerChannel;
  
  //! Number of shield hits before deadtime
  unsigned long m_NumShieldHitCounts;
  //! Number of BGO hits erased due to deadtime
  unsigned long m_NumBGOHitsErased;
  //! Bool to store if corresponding shield ASIC is dead or not
  bool m_IsShieldDead;

  //! First event time in seconds
  double m_FirstTime;
  //! Last event time in seconds
  double m_LastTime;

  //! Number of shield panels
  static const int nShieldPanels = 6;
  
  //! Last hit time for each shield panel
  vector<double> m_ShieldLastHitTime;
  //! Current Deadtime for each shield panel
  vector<double> m_ShieldDeadtime;
  //! Total Deadtime (added up over time) for each shield panel
  vector<double> m_TotalShieldDeadtime;
  
  //! Group of shield numbers per panel
  vector<vector<int>> m_ShieldPanelGroups;
  //! Shield crystal IDs for particular hits
  vector<vector<int>> m_ShieldHitCrystalID;

  //! GeD detectors hit (for deadtime calculation)
  vector<int> m_DetectorsHitForShieldVeto;
  
  //! Number of GeD detectors
  static const int nDets = 12;

#ifdef ___CLING___
 public:
  ClassDef(MSubModuleShieldTrigger, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
