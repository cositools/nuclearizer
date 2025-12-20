// /*
//  * MSubModuleStripTrigger.h
//  *
//  * Copyright (C) by Andreas Zoglauer.
//  * All rights reserved.
//  *
//  * Please see the source-file for the copyright-notice.
//  *
//  */


// #ifndef __MSubModuleStripTrigger__
// #define __MSubModuleStripTrigger__


// ////////////////////////////////////////////////////////////////////////////////


// // Standard libs:

// // ROOT libs:

// // MEGAlib libs:
// #include "MGlobal.h"
// #include "MSubModule.h"

// // Forward declarations:


// ////////////////////////////////////////////////////////////////////////////////


// class MSubModuleStripTrigger : public MSubModule
// {
//   // public interface:
//  public:
//   //! Default constructor
//   MSubModuleStripTrigger();

//   //! No copy constructor
//   MSubModuleStripTrigger(const MSubModuleStripTrigger&) = delete;
//   //! No copy assignment
//   MSubModuleStripTrigger& operator=(const MSubModuleStripTrigger&) = delete;
//   //! No move constructors
//   MSubModuleStripTrigger(MSubModuleStripTrigger&&) = delete;
//   //! No move operators
//   MSubModuleStripTrigger& operator=(MSubModuleStripTrigger&&) = delete;

//   //! Set deadime parameters file name
//   void SetDeadtimeFileName(const MString& FileName)
//   {
//     m_DeadtimeFileName = FileName;
//   }
//   //! Get deadime parameters file name
//   MString GetDeadtimeFileName() const
//   {
//     return m_DeadtimeFileName;
//   }

//   //! Default destructor
//   virtual ~MSubModuleStripTrigger();

//   //! Initialize the module
//   virtual bool Initialize();

//   //! Clear event data from the module
//   virtual void Clear();

//   //! Main data analysis routine, which updates the event to a new level 
//   virtual bool AnalyzeEvent(MReadOutAssembly* Event);

//   //! Return true if we have a trigger - filled after AnalyzeEvent
//   bool HasTrigger() const { return m_HasTrigger; }

//   //! Return true if we have a veto - filled after AnalyzeEvent
//   bool HasVeto() const { return m_HasVeto; }

//   //! Return the time when the dead time ends - filled after AnalyzeEvent
//   MTime GetDeadTimeEnd() const { return m_DeadTimeEnd; }

//   //! Finalize the module
//   virtual void Finalize();

//   //! Read the configuration data from an XML node
//   virtual bool ReadXmlConfiguration(MXmlNode* Node);
//   //! Create an XML node tree from the configuration
//   virtual MXmlNode* CreateXmlConfiguration(MXmlNode* Node);

//   // protected methods:
//  protected:

//   // private methods:
//  private:



//   // protected members:
//  protected:
//   //! Deadtime parameters file name
//   MString m_DeadtimeFileName;

//   // private members:
//  private:
//   //! Flag indicating that a trigger has been raised
//   bool m_HasTrigger;

//   //! Flag indicating that a veto has been raised
//   bool m_HasVeto;

//   //! Time when the shield dead time ends
//   MTime m_DeadTimeEnd;



// #ifdef ___CLING___
//  public:
//   ClassDef(MSubModuleStripTrigger, 0) // no description
// #endif

// };

// #endif


// ////////////////////////////////////////////////////////////////////////////////


/*
 * MSubModuleStripTrigger.h
 *
 * Copyright (C) by Andreas Zoglauer.
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
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

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

  //! Set the event time for deadtime calculations
  void SetEventTime(double Time) { m_EventTime = Time; }

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

  //! Get current strips deadtime
  double GetStripsCurrentDeadtime() const { return m_StripsCurrentDeadtime; }

  //! Get number of strip hits erased
  int GetStripHitsErased() const { return m_StripHitsErased; }

  //! Get trigger rates for detector
  int GetTriggerRate(int det) const {
    if (det >= 0 && det < nDets) return m_TriggerRates[det];
    return 0;
  }

  //! Increment trigger rate for detector
  void IncrementTriggerRate(int det) {
    if (det >= 0 && det < nDets) m_TriggerRates[det]++;
  }

  //! Update deadtime tracking
  void UpdateDeadtime(int det, int ASIC, vector<int> stripIDs, bool isFirstHitAfterDead);

  //! Calculate GeD deadtime
  double dTimeASICs(vector<int> ASICChannels);

  //! Helper function for getting count rate
  bool CountRate(vector<int> ASICChannels, vector<double> CountTime);

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

  //! Strip deadtime parameters
  double m_StripCoincidenceWindow;
  double m_ASICDeadTimePerChannel;
  double m_StripDelayAfter1;
  double m_StripDelayAfter2;
  double m_StripDelayAfter;

  double m_StripCoincidenceWindowFromFile;
  double m_ASICDeadTimePerChannelFromFile;
  double m_StripDelayAfter1FromFile;
  double m_StripDelayAfter2FromFile;

  //! Stores current dead time of the instrument
  double m_StripsCurrentDeadtime;
  //! Stores last good event time
  double m_ASICLastHitTime;
  //! Stores total dead time of the instrument
  double m_StripsTotalDeadtime;
  //! Hits erased due to deadtime
  int m_StripHitsErased;

  //! Number of detectors
  static const int nDets = 1;
  //! Number of ASICs for 1 det
  static const int nASICs = 4;

  //! Stores dead time for each ASIC
  vector<vector<double>> m_ASICDeadTime;
  //! Strip ID for particular hit in ASIC
  vector<vector<vector<int>>> m_ASICHitStripID;
  //! Helper Strip ID vector to count for hits without deadtime
  vector<vector<vector<int>>> m_ASICHitStripID_noDT;
  //! Strip ID for particular hit in ASIC
  vector<vector<vector<double>>> m_TempEvtTimes;

  //! Stores trigger rates (number of events) for each detector
  vector<int> m_TriggerRates;

  //! Event times and strip IDs for counting
  vector<double> m_EventTimes;
  vector<double> m_EventStripIDs;

#ifdef ___CLING___
 public:
  ClassDef(MSubModuleStripTrigger, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////