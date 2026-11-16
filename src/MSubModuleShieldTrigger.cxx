/*
 * MSubModuleShieldTrigger.cxx
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
// MSubModuleShieldTrigger
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleShieldTrigger.h"

// Standard libs:
#include <algorithm>
#include <limits>

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"
#include "MParser.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleShieldTrigger)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldTrigger::MSubModuleShieldTrigger() : MSubModule()
{
  // Construct an instance of MSubModuleShieldTrigger

  m_Name = "DEE shield trigger module";

  m_EventTime = 0.0;
  m_HasShieldVeto = false;
  m_IsShieldDead = false;
  m_ShieldVetoTime = 0.0;
  m_ShieldVetoWindowDelta = 0.0;
  m_DeadTimeEnd = MTime(0.0);

  // Initialize shield parameters with default values
  m_ShieldThreshold = -1.0; // Need to change this value at some point
  
  m_NumShieldHitCounts = 0;
  m_NumBGOHitsErased = 0;

  m_FirstTime = std::numeric_limits<double>::max();
  m_LastTime = 0.0;

  // Initialize vectors
  m_ShieldLastHitTime = vector<double>(nShieldPanels, -10.0);
  m_ShieldDeadtime = vector<double>(nShieldPanels, 0.0);
  m_TotalShieldDeadtime = vector<double>(nShieldPanels, 0.0);
  m_ShieldHitCrystalID = vector<vector<int>>(nShieldPanels);
  m_DetectorsHitForShieldVeto = vector<int>(nDets, 0);
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldTrigger::~MSubModuleShieldTrigger()
{
  // Delete this instance of MSubModuleShieldTrigger
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::Initialize()
{
  // Initialize the module

  // Read deadtime parameters from file if provided
  if (m_DeadtimeFileName != "" && ParseDeadtimeFile() == false) {
    if (g_Verbosity >= c_Error) cout<<"Error: Deadtime parameters file not found"<<endl;
    return false;
  }

  // A new run must not inherit a previous run's windows or statistics.
  m_ShieldLastHitTime.assign(nShieldPanels, -10.0);
  m_ShieldDeadtime.assign(nShieldPanels, 0.0);
  m_ShieldHitCrystalID.assign(nShieldPanels, vector<int>());
  m_TotalShieldDeadtime.assign(nShieldPanels, 0.0);
  m_NumShieldHitCounts = 0;
  m_NumBGOHitsErased = 0;
  m_FirstTime = std::numeric_limits<double>::max();
  m_LastTime = 0.0;
  m_DeadTimeEnd = MTime(0.0);
  Clear();

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldTrigger::Clear()
{
  // Clear for the next event

  m_HasShieldVeto = false;
  m_IsShieldDead = false;
  m_ShieldVetoTime = 0.0;
  // Keep panel windows and their accepted channels until they expire.
  
  m_DetectorsHitForShieldVeto = vector<int>(nDets, 0);

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


double MSubModuleShieldTrigger::CalculateASICDeadtime(vector<int> CrystalIDs)
{
  // Calculate deadtime for shield ASICs
  double deadtime = 0;
  int countUnique = 0;

  if (CrystalIDs.empty()) {
    return 0.0;
  }

  unordered_set<int> BGOCrystalsSet;
  for (int ID : CrystalIDs) {
    BGOCrystalsSet.insert(ID);
  }

  // Count the number of unique channels read out (2 for each hit in the BGO)
  countUnique = BGOCrystalsSet.size() * 2;
  deadtime = m_ShieldDelayBefore + (m_ASICDeadTimePerChannel * countUnique) + m_ShieldDelayAfter;
  
  if (deadtime < m_ShieldPulseDuration) {
    deadtime = m_ShieldPulseDuration;
  }

  return deadtime;
}

////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::ProcessShieldHits(MReadOutAssembly* Event)
{
  // Process shield crystal hits and update shield deadtime state

  // // Track which GeD detectors got hit (for later deadtime update)
  // list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  // for (const MDEEStripHit& Hit : LVHits) {
  //   int DetID = Hit.m_ROE.GetDetectorID();
  //   if (DetID >= 0 && DetID < nDets) {
  //     m_DetectorsHitForShieldVeto[DetID] = 1;
  //   }
  // }

  // Process shield crystal hits
  list<MDEECrystalHit>& CrystalHits = Event->GetDEECrystalHitListReference();
  
  for (auto Hit = CrystalHits.begin(); Hit != CrystalHits.end(); ) {
    MDEECrystalHit& CHit = *Hit;
    m_NumShieldHitCounts += 1;
    
    MString DetectorID = CHit.m_DetectorID;
    int CrystalID = CHit.m_CrystalID;
    double energy = CHit.m_Energy;

    if (energy > m_ShieldThreshold) {
      // Find which panel group this detector belongs to
      int ShieldDetGroup = -1;
      int ReadoutCrystalID = CrystalID;

      if (DetectorID == "X0") {
        ShieldDetGroup = 0;
      }
      else if (DetectorID == "X1") {
        ShieldDetGroup = 1;
      }
      else if (DetectorID == "Y0" || (DetectorID == "Z0" && CrystalID == 4)) {
        ShieldDetGroup = 2;
      }
      else if (DetectorID == "Y1" || (DetectorID == "Z1" && CrystalID == 0)) {
        ShieldDetGroup = 3;
        // Y1-0 and Z1-0 are distinct crystals on this group's electronics.
        if (DetectorID == "Z1") ReadoutCrystalID = 3;
      }
      else if (DetectorID == "Z0" && CrystalID >= 0 && CrystalID <= 3) {
        ShieldDetGroup = 4;
      }
      else if (DetectorID == "Z1" && CrystalID >= 1 && CrystalID <= 4) {
        ShieldDetGroup = 5;
      }
      else {
        if (g_Verbosity >= c_Warning) {
          cout << m_Name << ": Warning - shield detector " << DetectorID 
                << " not found in any panel group" << endl;
        }
        ++Hit;
        continue;
      }

      // Check deadtime conditions
      if (m_ShieldHitCrystalID[ShieldDetGroup].empty() ||
          m_EventTime > (m_ShieldLastHitTime[ShieldDetGroup] + m_ShieldDeadtime[ShieldDetGroup])) {
        // Event occurred after deadtime - start new veto window
        m_ShieldHitCrystalID[ShieldDetGroup].clear();
        m_ShieldLastHitTime[ShieldDetGroup] = m_EventTime;
        m_ShieldHitCrystalID[ShieldDetGroup].push_back(ReadoutCrystalID);
        m_TotalShieldDeadtime[ShieldDetGroup] += m_ShieldDeadtime[ShieldDetGroup];
      }
      else if (m_EventTime <= (m_ShieldLastHitTime[ShieldDetGroup] + m_ShieldDelayBefore)) {
        // Event occurred within coincidence window - add to existing veto
        m_ShieldHitCrystalID[ShieldDetGroup].push_back(ReadoutCrystalID);
      }
      else {
        // Event occurred within deadtime
        m_IsShieldDead = true;
        m_NumBGOHitsErased += 1;
        Hit = CrystalHits.erase(Hit);
        continue;
      }

      // Update immediately: subsequent hits can extend this same window.
      m_ShieldDeadtime[ShieldDetGroup] = CalculateASICDeadtime(m_ShieldHitCrystalID[ShieldDetGroup]);
    }
    ++Hit;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::ParseDeadtimeFile()
{
  // Read in deadtime parameters file
  // Format:
  //   Row 1: strip trigger header
  //   Row 2: strip trigger parameters
  //   Row 3: shield trigger header
  //   Row 4: shield trigger parameters

  MParser Parser;
  if (Parser.Open(m_DeadtimeFileName) == false) {
    if (g_Verbosity >= c_Error) cout << m_Name << ": Unable to open deadtime parameters file: " << m_DeadtimeFileName << endl;
    return false;
  }

  if (Parser.GetNLines() < 4) {
    if (g_Verbosity >= c_Error) cout << m_Name << ": Deadtime file does not have enough data" << endl;
    return false;
  }

  MTokenizer* ShieldTokenizer = Parser.GetTokenizerAt(3);
  if (ShieldTokenizer->GetNTokens() != 5) {
    if (g_Verbosity >= c_Error) cout << m_Name << ": Shield deadtime row must contain exactly 5 values" << endl;
    return false;
  }

  m_ASICDeadTimePerChannel = ShieldTokenizer->GetTokenAtAsDouble(0);
  m_ShieldPulseDuration = ShieldTokenizer->GetTokenAtAsDouble(1);
  m_ShieldDelayBefore = ShieldTokenizer->GetTokenAtAsDouble(2);
  m_ShieldDelayAfter = ShieldTokenizer->GetTokenAtAsDouble(3);
  m_ShieldVetoWindowDelta = ShieldTokenizer->GetTokenAtAsDouble(4);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine for shield trigger and veto

  m_HasShieldVeto = false;
  m_IsShieldDead = false;
  m_ShieldVetoTime = 0.0;

  m_EventTime = Event->GetTimeUTC().GetAsSeconds();

  // Retain a panel's channels through its complete readout and veto window.
  // Once expired, account for its dead time exactly once and discard its state.
  for (int group = 0; group < nShieldPanels; ++group) {
    if (!m_ShieldHitCrystalID[group].empty() &&
        m_EventTime > m_ShieldLastHitTime[group] + m_ShieldDeadtime[group] +
          std::max(0.0, m_ShieldVetoWindowDelta)) {
      m_TotalShieldDeadtime[group] += m_ShieldDeadtime[group];
      m_ShieldHitCrystalID[group].clear();
      m_ShieldDeadtime[group] = 0.0;
      m_ShieldLastHitTime[group] = -10.0;
    }
  }

  ProcessShieldHits(Event);

  // The global veto lasts until the latest active panel window ends. Keep each
  // duration paired with its own start, including any coincident-hit extension.
  double maxDeadTimeEnd = 0.0;
  double maxVetoEnd = -std::numeric_limits<double>::infinity();
  for (int group = 0; group < nShieldPanels; ++group) {
    if (m_ShieldHitCrystalID[group].empty()) continue;

    double DeadTimeEnd = m_ShieldLastHitTime[group] + m_ShieldDeadtime[group];
    maxDeadTimeEnd = std::max(maxDeadTimeEnd, DeadTimeEnd);
    double VetoEnd = DeadTimeEnd + m_ShieldVetoWindowDelta;
    if (m_EventTime >= m_ShieldLastHitTime[group] && m_EventTime <= VetoEnd &&
        VetoEnd > maxVetoEnd) {
      maxVetoEnd = VetoEnd;
      m_ShieldVetoTime = m_ShieldLastHitTime[group];
      m_HasShieldVeto = true;
    }
  }
  m_DeadTimeEnd = MTime(maxDeadTimeEnd);

  if (m_EventTime < m_FirstTime) {
    m_FirstTime = m_EventTime;
  }
  if (m_EventTime > m_LastTime) {
    m_LastTime = m_EventTime;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldTrigger::Finalize()
{
  // Finalize the analysis - do all cleanup

  // Print shield trigger module diagnostics
  if (g_Verbosity >= c_Info) {
    cout << "###################" << endl
        << "SHIELD TRIGGER MODULE STATISTICS" << endl
        << "###################" << endl;
    
    double simTime = m_LastTime - m_FirstTime;
    if (simTime > 0) {
      cout << "Simulation time: " << simTime << " seconds" << endl;
    }
    
    cout << "Total BGO hits before BGO deadtime: " << m_NumShieldHitCounts << endl;
    
    for (int i = 0; i < nShieldPanels; i++) {
      cout << "Shield Panel " << i << " dead time: " << m_TotalShieldDeadtime[i] << " seconds" << endl;
      if (simTime > 0) {
        double liveFraction = 1.0 - (m_TotalShieldDeadtime[i] / simTime);
        cout << "  Livetime fraction: " << liveFraction << endl;
      }
    }
    
    cout << "BGO hits erased due to BGO being dead: " << m_NumBGOHitsErased << endl;
    
    if (simTime > 0) {
      double rateAfterDT = (m_NumShieldHitCounts - m_NumBGOHitsErased) / simTime;
      cout << "Shield rate after deadtime: " << rateAfterDT << " cps" << endl;
    }
  }

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleShieldTrigger::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  return Node;
}


// MSubModuleShieldTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
