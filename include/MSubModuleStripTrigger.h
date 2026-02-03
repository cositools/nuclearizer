/*
 * MSubModuleStripTrigger.h
 *
 * Copyright (C) by Andreas Zoglauer, Parshad Patel.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleStripTrigger__
#define __MSubModuleStripTrigger__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <unordered_set>
#include <set>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Nuclearizer libs:
#include "MDEEStripHit.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleStripTrigger : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleStripTrigger();

  //! No copy constructor
  MSubModuleStripTrigger(const MSubModuleStripTrigger&) = delete;
  //! No copy assignment
  MSubModuleStripTrigger& operator=(const MSubModuleStripTrigger&) = delete;
  //! No move constructors
  MSubModuleStripTrigger(MSubModuleStripTrigger&&) = delete;
  //! No move operators
  MSubModuleStripTrigger& operator=(MSubModuleStripTrigger&&) = delete;

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
  virtual ~MSubModuleStripTrigger();

  //! Initialize the module
  virtual bool Initialize();

  //! Clear event data from the module
  virtual void Clear();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Return true if we have a trigger - filled after AnalyzeEvent
  bool HasTrigger() const { return m_HasTrigger; }

  //! Return true if we have a veto - filled after AnalyzeEvent
  bool HasVeto() const { return m_HasVeto; }

  //! Return the time when the dead time ends - filled after AnalyzeEvent
  MTime GetDeadTimeEnd() const { return m_DeadTimeEnd; }

  //! Check if GeD is currently dead
  bool IsGeDDead() const { return m_IsGeDDead; }

  //! Get total strips deadtime
  double GetStripsTotalDeadtime() const { return m_StripsTotalDeadtime; }

  //! Get number of strip hits erased
  int GetStripHitsErased() const { return m_StripHitsErased; }

  //! Get trigger rate for detector
  int GetTriggerRate(int det) const {
    if (det >= 0 && det < nDets) return m_NumStripTriggers[det];
    return 0;
  }

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

  //! Calculate GeD ASIC deadtime
  double CalculateASICDeadtime(vector<int> ASICChannels);

  //! Process strip hits for deadtime and trigger determination
  bool ProcessStripHits(MReadOutAssembly* Event);

  //! Helper function for getting count rate (including nearest neighbor)
  bool CountRate(vector<int> ASICChannels, vector<double> CountTime);

  //! Check if at least one strip exists on each side of each detector
  bool CheckTriggerConditions(MReadOutAssembly* Event);

  // protected members:
 protected:
  //! Deadtime parameters file name
  MString m_DeadtimeFileName;

  // private members:
 private:
  //! Current event time
  double m_EventTime;

  //! Flag indicating that a trigger has been raised
  bool m_HasTrigger;

  //! Flag indicating that a veto has been raised
  bool m_HasVeto;

  //! Time when the strip dead time ends
  MTime m_DeadTimeEnd;

  //! Bool to store if ASIC is dead or not
  bool m_IsGeDDead;

  //! Strip readout coincidence window
  double m_StripCoincidenceWindow;
  //! ADC deadtime per channel read out
  double m_ASICDeadTimePerChannel;
  //! Strip delay before readout
  double m_StripDelayAfter1;
  //! Strip delay for output to settle
  double m_StripDelayAfter2;
  //! Total strip delay after readout
  double m_StripDelayAfter;

  //! Strip coincidence window in seconds as set in file
  double m_StripCoincidenceWindowFromFile;
  //! ASIC deadtime per channel read out in seconds as set in file
  double m_ASICDeadTimePerChannelFromFile;
  //! Strip delay 1 before readout in seconds as set in file
  double m_StripDelayAfter1FromFile;
  //! Strip delay 2 for output to settle in seconds as set in file
  double m_StripDelayAfter2FromFile;

  //! Stores current dead time of the instrument
  double m_StripsCurrentDeadtime;
  //! Stores last good event time
  double m_ASICLastHitTime;
  //! Stores total dead time of the instrument
  double m_StripsTotalDeadtime;
  //! Hits erased due to deadtime
  int m_StripHitsErased;
  //! Total strip hits counter
  int m_TotalStripHitsCounter;

  //! First event time for statistics
  double m_FirstTime;
  //! Last event time for statistics
  double m_LastTime;

  //! Number of detectors
  static const int nDets = 16;
  //! Number of ASICs per detector
  static const int nASICs = 4;

  //! Stores dead time for each ASIC
  vector<vector<double>> m_ASICDeadTime;
  //! Strip ID for particular hit in ASIC
  vector<vector<vector<int>>> m_ASICHitStripID;
  //! Helper Strip ID vector to count for hits without deadtime
  vector<vector<vector<int>>> m_ASICHitStripID_noDT;
  //! Event times for particular hit in ASIC
  vector<vector<vector<double>>> m_TempEvtTimes;

  //! Stores trigger counts for each detector
  vector<int> m_NumStripTriggers;

  //! Event strip times for counting (should be removed later)
  vector<double> m_EventStripTimes;
  //! Event strip IDs for counting (should be removed later)
  vector<double> m_EventStripIDs;

#ifdef ___CLING___
 public:
  ClassDef(MSubModuleStripTrigger, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////