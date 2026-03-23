/*
 * MSubModuleStripTrigger.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Parshad Patel.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MSubModuleStripTrigger
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleStripTrigger.h"

// Standard libs:
#include <algorithm>
#include <limits>

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"
#include "MParser.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleStripTrigger)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripTrigger::MSubModuleStripTrigger() : MSubModule()
{
  // Construct an instance of MSubModuleStripTrigger

  m_Name = "DEE strip trigger module";

  m_EventTime = 0.0;
  m_HasTrigger = false;
  m_HasVeto = false;
  m_IsGeDDead = false;

  // Initialize deadtime parameters
  m_StripCoincidenceWindow = 0.0;
  m_ASICDeadTimePerChannel = 0.0;
  m_StripDelayAfter1 = 0.0;
  m_StripDelayAfter2 = 0.0;
  m_StripDelayAfter = 0.0;

  m_StripsCurrentDeadtime = 0.0;
  m_ASICLastHitTime = -10.0;
  m_StripsTotalDeadtime = 0.0;
  // m_StripHitsErased = 0;
  m_TotalStripHitsCounter = 0;
  m_TotalGRHitsCounter = 0;

  m_FirstTime = std::numeric_limits<double>::max();
  m_LastTime = 0.0;

  // Initialize vectors
  m_ASICDeadTime = vector<vector<double>>(nDets, vector<double>(nASICs, 0.0));
  m_ASICHitStripID = vector<vector<vector<int>>>(nDets, vector<vector<int>>(nASICs));
  m_ASICHitStripID_noDT = vector<vector<vector<int>>>(nDets, vector<vector<int>>(nASICs));
  m_TempEvtTimes = vector<vector<vector<double>>>(nDets, vector<vector<double>>(nASICs));
  m_NumStripTriggers = vector<int>(nDets, 0);
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripTrigger::~MSubModuleStripTrigger()
{
  // Delete this instance of MSubModuleStripTrigger
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::Initialize()
{
  // Initialize the module

  // Read deadtime parameters from file
  if (m_DeadtimeFileName != "" && ParseDeadtimeFile() == false) {
    if (g_Verbosity >= c_Error) cout<<"Error: Deadtime parameters file not found"<<endl;
    return false;
  }

  // Set deadtime parameters
  m_StripCoincidenceWindow = m_StripCoincidenceWindowFromFile;
  m_ASICDeadTimePerChannel = m_ASICDeadTimePerChannelFromFile;
  m_StripDelayAfter1 = m_StripDelayAfter1FromFile;
  m_StripDelayAfter2 = m_StripDelayAfter2FromFile;
  m_StripDelayAfter = m_StripDelayAfter1 + m_StripDelayAfter2;

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripTrigger::Clear()
{
  // Clear for the next event

  m_HasTrigger = false;
  m_HasVeto = false;
  m_DeadTimeEnd = MTime(0.0);

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


double MSubModuleStripTrigger::CalculateASICDeadtime(vector<int> ASICChannels)
{
  // Calculate deadtime for GeD ASICs including nearest neighbor readout

  double deadtime = 0; // temporary deadtime variable
  int countUnique = 0; // temporary unique channel counter

  if (ASICChannels.empty()) {
    return 0.0;
  }

  unordered_set<int> ASICChannelsSet;

  // Sort ASICChannels to process channels in ascending order
  sort(ASICChannels.begin(), ASICChannels.end());

  // Loop through each channel ID and add nearest neighbors
  for (int ID : ASICChannels) {
    if (ID == 64) {
      ASICChannelsSet.insert(ID);
    } else if (ID == 0 || ID == 32) {
      // Edge case: If ID is 0 or 32, add the channel and the next channel
      ASICChannelsSet.insert(ID);
      ASICChannelsSet.insert(ID + 1);
    } else if (ID == 31 || ID == 63) {
      // Edge case: If ID is 31 or 63, add the previous channel and the channel itself
      ASICChannelsSet.insert(ID - 1);
      ASICChannelsSet.insert(ID);
    } else {
      // General case: Add the previous channel, the channel itself, and the next channel
      ASICChannelsSet.insert(ID - 1);
      ASICChannelsSet.insert(ID);
      ASICChannelsSet.insert(ID + 1);
    }
  }

  // Count the number of unique channels read out
  countUnique = ASICChannelsSet.size();

  // Calculate the total deadtime based on unique channels
  deadtime = m_StripCoincidenceWindow + (m_ASICDeadTimePerChannel * countUnique) + m_StripDelayAfter;

  return deadtime;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::CountRate(vector<int> ASICChannels, vector<double> CountTime)
{
  // Helper function for getting count rate (including nearest neighbor)
  
  if (ASICChannels.empty()) {
    return false;
  }

  unordered_set<int> ASICChannelsSet;
  vector<double> CountTimeVec;

  // Sort ASICChannels to process channels in ascending order
  sort(ASICChannels.begin(), ASICChannels.end());

  // Loop through each channel ID
  for (size_t i = 0; i < ASICChannels.size(); i++) {
    int ID = ASICChannels[i];
    size_t temp_size = ASICChannelsSet.size();

    if (ID == 64) {
      // ASICChannelsSet.insert(ID);
      continue; // Do not include GR hits in count rate calculation as it has its own readout and does not cause nearest neighbor readout
    } else if (ID == 0 || ID == 32) {
      ASICChannelsSet.insert(ID);
      ASICChannelsSet.insert(ID + 1);
    } else if (ID == 31 || ID == 63) {
      ASICChannelsSet.insert(ID - 1);
      ASICChannelsSet.insert(ID);
    } else {
      ASICChannelsSet.insert(ID - 1);
      ASICChannelsSet.insert(ID);
      ASICChannelsSet.insert(ID + 1);
    }
    
    size_t new_size = ASICChannelsSet.size();
    for (size_t j = 0; j < (new_size - temp_size); j++) {
      CountTimeVec.push_back(CountTime[i]);
    }
  }

  int h = 0;
  for (int k : ASICChannelsSet) {
    m_EventStripIDs.push_back(k);
    m_EventStripTimes.push_back(CountTimeVec[h]);
    h++;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::CheckTriggerConditions(MReadOutAssembly* Event)
{
  // Check if at least one strip exists on each side of each detector
  // If not, remove remaining strips because they won't trigger detector

  vector<bool> xExists(nDets, false);
  vector<bool> yExists(nDets, false);

  // Check LV strips (x-direction)
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (const MDEEStripHit& Hit : LVHits) {
    if (!Hit.m_IsGuardRing) {
      int DetID = Hit.m_ROE.GetDetectorID();
      if (DetID >= 0 && DetID < nDets) {
        xExists[DetID] = true;
      }
    }
  }

  // Check HV strips (y-direction)
  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (const MDEEStripHit& Hit : HVHits) {
    if (!Hit.m_IsGuardRing) {
      int DetID = Hit.m_ROE.GetDetectorID();
      if (DetID >= 0 && DetID < nDets) {
        yExists[DetID] = true;
      }
    }
  }

  // Remove hits that won't trigger detector
  auto LVIter = LVHits.begin();
  while (LVIter != LVHits.end()) {
    int DetID = LVIter->m_ROE.GetDetectorID();
    if (DetID >= 0 && DetID < nDets && (xExists[DetID] == false || yExists[DetID] == false)) {
      LVIter = LVHits.erase(LVIter);
    } else {
      ++LVIter;
    }
  }

  auto HVIter = HVHits.begin();
  while (HVIter != HVHits.end()) {
    int DetID = HVIter->m_ROE.GetDetectorID();
    if (DetID >= 0 && DetID < nDets && (xExists[DetID] == false || yExists[DetID] == false)) {
      HVIter = HVHits.erase(HVIter);
    } else {
      ++HVIter;
    }
  }

  // Check if any valid hits remain
  return (!LVHits.empty() || !HVHits.empty());
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::ProcessStripHits(MReadOutAssembly* Event)
{
  // Process strip hits for deadtime calculation and trigger determination

  m_EventTime = Event->GetTime().GetAsSeconds();
  bool ASICFirstHitAfterDead = false;
  m_IsGeDDead = false;

  // Get merged strip hits from the event
  // TODO: check if the strip merging should happen here instead of MSubModuleChargeTransport
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();

  // Process all hits and track deadtime. Split accordingly between LV and HV hits, as they belong to different ASICs. Create a m_ASICHitStripID vector that is passed in to calculate deadtime for each ASIC later in the code. 3 cases for this, depending on whether the event is after deadtime, within coincidence window, or within deadtime.
  auto ProcessHits = [&](list<MDEEStripHit>& Hits) {
    auto HitIter = Hits.begin();
    while (HitIter != Hits.end()) {
      m_TotalStripHitsCounter++;
      
      int det = HitIter->m_ROE.GetDetectorID();
      if (det < 0 || det >= nDets) {
        ++HitIter;
        continue;
      }

      // Determine which ASIC this strip belongs to
      int ASICofDet = -1;
      int StripID = HitIter->m_ROE.GetStripID();
      bool IsLV = HitIter->m_ROE.IsLowVoltageStrip();

      if (IsLV && StripID >= 0 && StripID <= 31) {
        ASICofDet = 0;
      } else if (IsLV && StripID >= 32 && StripID <= 63) {
        ASICofDet = 1;
      } else if (!IsLV && StripID >= 0 && StripID <= 31) {
        ASICofDet = 2;
      } else if (!IsLV && StripID >= 32 && StripID <= 63) {
        ASICofDet = 3;
      } else if (!IsLV && StripID == 64) {
        m_HasVeto = true;
        m_TotalGRHitsCounter++;
        ASICofDet = 4;
      } else if (IsLV && StripID == 64) {
        m_HasVeto = true;
        m_TotalGRHitsCounter++;
        ASICofDet = 5;
      } else {
        if (g_Verbosity >= c_Warning) {
          cout << m_Name << ": Warning - Strip not associated with any ASIC" << endl;
        }
        ++HitIter;
        continue;
      }

      // Check deadtime status
      if (m_EventTime > (m_ASICLastHitTime + m_StripsCurrentDeadtime)) {
        // Event occurred after deadtime - clear old data
        if (!ASICFirstHitAfterDead) {
          for (int d = 0; d < nDets; d++) {
            for (int a = 0; a < nASICs; a++) {
              CountRate(m_ASICHitStripID[d][a], m_TempEvtTimes[d][a]);
              m_ASICHitStripID_noDT[d][a].clear();
              m_ASICHitStripID[d][a].clear();
              m_TempEvtTimes[d][a].clear();
            }
          }
          ASICFirstHitAfterDead = true;
          m_ASICLastHitTime = m_EventTime;
        }

        m_ASICHitStripID[det][ASICofDet].push_back(StripID);
        m_ASICHitStripID_noDT[det][ASICofDet].push_back(StripID);
        m_TempEvtTimes[det][ASICofDet].push_back(m_EventTime);
      }
      else if (m_EventTime <= (m_ASICLastHitTime + m_StripCoincidenceWindow)) {
        // Event occurred within coincidence window
        m_ASICHitStripID[det][ASICofDet].push_back(StripID);
        m_ASICHitStripID_noDT[det][ASICofDet].push_back(StripID);
        m_TempEvtTimes[det][ASICofDet].push_back(m_EventTime);
      }
      else {
        // Event occurred within deadtime - erase hit
        m_ASICHitStripID_noDT[det][ASICofDet].push_back(StripID);
        m_TempEvtTimes[det][ASICofDet].push_back(m_EventTime);
        m_IsGeDDead = true;
        // m_StripHitsErased++;
        HitIter = Hits.erase(HitIter);
        continue;
      }

      ++HitIter;
    }
  };

  // Process both LV and HV hits
  ProcessHits(LVHits);
  ProcessHits(HVHits);

  // Update total deadtime if this was first hit after dead period
  if (ASICFirstHitAfterDead) {
    m_StripsTotalDeadtime += m_StripsCurrentDeadtime;
    m_StripsCurrentDeadtime = 0.0;
  }

  // Calculate new deadtime for each ASIC
  for (int det = 0; det < nDets; det++) {
    for (int ASIC = 0; ASIC < nASICs; ASIC++) {
      if (!m_IsGeDDead && !m_ASICHitStripID[det][ASIC].empty()) {
        m_ASICDeadTime[det][ASIC] = CalculateASICDeadtime(m_ASICHitStripID[det][ASIC]);
        if (m_ASICDeadTime[det][ASIC] > m_StripsCurrentDeadtime) {
          m_StripsCurrentDeadtime = m_ASICDeadTime[det][ASIC];
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine for strip trigger

  m_HasTrigger = false;
  m_HasVeto = false;

  // Process strip hits and calculate deadtime
  ProcessStripHits(Event);

  // Check if we have valid trigger conditions (at least one strip on each side)
  if (!CheckTriggerConditions(Event)) {
    // No valid trigger - all strips removed
    return true;
  }

  // Update trigger rates
  set<int> detectorsHit;
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (const MDEEStripHit& Hit : LVHits) {
    if (!Hit.m_IsGuardRing) {
      int DetID = Hit.m_ROE.GetDetectorID();
      if (DetID >= 0 && DetID < nDets) {
        detectorsHit.insert(DetID);
      }
    }
  }

  for (int detID : detectorsHit) {
    m_NumStripTriggers[detID]++;
  }

  // We have a valid trigger
  m_HasTrigger = true;

  // Update time tracking
  if (m_EventTime < m_FirstTime) {
    m_FirstTime = m_EventTime;
  }
  if (m_EventTime > m_LastTime) {
    m_LastTime = m_EventTime;
  }

  // Set dead time end
  if (m_StripsCurrentDeadtime > 0) {
    m_DeadTimeEnd = MTime(m_ASICLastHitTime + m_StripsCurrentDeadtime);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripTrigger::Finalize()
{
  // Finalize the analysis

  cout << "###################" << endl
       << "STRIP TRIGGER MODULE STATISTICS" << endl
       << "###################" << endl;
  
  double simTime = m_LastTime - m_FirstTime;
  if (simTime > 0) {
    cout << "Simulation time: " << simTime << " seconds" << endl;
  }
  
  cout << "Total strip hits after charge sharing (before deadtime): " << m_TotalStripHitsCounter << endl;
  cout << "Total GR hits (before deadtime): " << m_TotalGRHitsCounter << endl;
  cout << "Total dead time of the instrument: " << m_StripsTotalDeadtime << " seconds" << endl;
  
  if (simTime > 0) {
    double liveFraction = 1.0 - (m_StripsTotalDeadtime / simTime);
    cout << "Livetime fraction: " << liveFraction << endl;
  }
  
  // cout << "Hits erased due to detector being dead: " << m_StripHitsErased << endl;
  
  if (m_TotalStripHitsCounter > 0) {
    cout << "Avg deadtime per strip hit: " << m_StripsTotalDeadtime / m_TotalStripHitsCounter << " seconds" << endl;
  }
  
  cout << "Trigger rates (events per detector):" << endl;
  for (int i = 0; i < nDets; i++) {
    cout << "  Detector " << i << ": " << m_NumStripTriggers[i] << " events";
    if (simTime > 0) {
      cout << " (" << (m_NumStripTriggers[i] / simTime) << " Hz)";
    }
    cout << endl;
  }

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::ParseDeadtimeFile()
{
  // Read in deadtime parameters file

  MParser Parser;
  if (Parser.Open(m_DeadtimeFileName) == false) {
    cout << m_Name << ": Unable to open deadtime parameters file: " << m_DeadtimeFileName << endl;
    return false;
  }

  if (Parser.GetNLines() < 2) {
    cout << m_Name << ": Deadtime file does not have enough data" << endl;
    return false;
  }

  m_StripCoincidenceWindowFromFile = Parser.GetTokenizerAt(1)->GetTokenAtAsDouble(0);
  m_ASICDeadTimePerChannelFromFile = Parser.GetTokenizerAt(1)->GetTokenAtAsDouble(1);
  m_StripDelayAfter1FromFile = Parser.GetTokenizerAt(1)->GetTokenAtAsDouble(2);
  m_StripDelayAfter2FromFile = Parser.GetTokenizerAt(1)->GetTokenAtAsDouble(3);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* DeadtimeFileNode = Node->GetNode("DeadtimeFileName");
  if (DeadtimeFileNode != nullptr) {
    m_DeadtimeFileName = DeadtimeFileNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleStripTrigger::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  new MXmlNode(Node, "DeadtimeFileName", m_DeadtimeFileName);

  return Node;
}


// MSubModuleStripTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////