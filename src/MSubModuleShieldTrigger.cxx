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
  m_HasTrigger = false;
  m_HasVeto = false;
  m_IsShieldDead = false;

  // Initialize shield parameters with default values
  m_ShieldThreshold = -1.0; // Need to change this value at some point
  m_ShieldPulseDuration = 1.7e-6;
  m_ShieldDelayBefore = 0.1e-6;
  m_ShieldDelayAfter = 0.4e-6;
  m_ShieldVetoWindowSize = 1.5e-6;
  m_ASICDeadTimePerChannel = 0.0;
  m_ShieldVetoTime = 0.0;
  
  m_NumShieldHitCounts = 0;
  m_ShieldVetoCounter = 0;
  m_NumBGOHitsErased = 0;

  m_FirstTime = std::numeric_limits<double>::max();
  m_LastTime = 0.0;

  // Initialize vectors
  m_ShieldLastHitTime = vector<double>(nShieldPanels, -10.0);
  m_ShieldDeadtime = vector<double>(nShieldPanels, 0.0);
  m_TotalShieldDeadtime = vector<double>(nShieldPanels, 0.0);
  m_ShieldHitCrystalID = vector<vector<int>>(nShieldPanels);
  m_DetectorsHitForShieldVeto = vector<int>(nDets, 0);

  // Initialize shield panel groups (based on detector IDs)
  // These map crystal IDs to panel groups
  m_ShieldPanelGroups = {
    { 0, 1, 2},      // Panel 0: Detectors 0-3
    { 0, 1, 2},      // Panel 1: Detectors 4-7
    { 0, 1, 2, 4 },    // Panel 2: Detectors 8-11
    { 0, 1, 2, 4 },  // Panel 3: Detectors 12-15
    { 0, 1, 2, 3 },      // Panel 4: Detectors 16-18
    { 0, 1, 2, 3 }       // Panel 5: Detectors 19-21
  };
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
    return false;
  }

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldTrigger::Clear()
{
  // Clear for the next event

  m_HasTrigger = false;
  m_HasVeto = false;
  m_IsShieldDead = false;
  m_DeadTimeEnd = MTime(0.0);
  
  // // Clear per-event data
  // for (int i = 0; i < nShieldPanels; i++) {
  //   m_ShieldHitCrystalID[i].clear();
  // }
  
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
  // Process shield crystal hits to determine veto status

  m_EventTime = Event->GetTime().GetAsSeconds();

  // Track which GeD detectors got hit (for later deadtime update)
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (const MDEEStripHit& Hit : LVHits) {
    int DetID = Hit.m_ROE.GetDetectorID();
    if (DetID >= 0 && DetID < nDets) {
      m_DetectorsHitForShieldVeto[DetID] = 1;
    }
  }

  // Process shield crystal hits
  list<MDEECrystalHit>& CrystalHits = Event->GetDEECrystalHitListReference();
  
  for (MDEECrystalHit& CHit : CrystalHits) {
    m_NumShieldHitCounts += 1;
    
    MString DetectorID = CHit.m_DetectorID;
    int CrystalID = CHit.m_CrystalID;
    double energy = CHit.m_Energy;

    cout << energy << endl;

    if (energy > m_ShieldThreshold) {
      // Find which panel group this detector belongs to
      int ShieldDetGroup = -1;

      if (DetectorID == "Z0" && CrystalID == 4) {
        ShieldDetGroup = 2;
      }
      else if (DetectorID == "Z1" && CrystalID == 4) {
        ShieldDetGroup = 3;
      }
      else if (DetectorID == "X0") {
        ShieldDetGroup = 0;
      }
      else if(DetectorID == "X1") {
        ShieldDetGroup = 1;
      }
      else if (DetectorID == "Y0") {
        ShieldDetGroup = 2;
      }
      else if(DetectorID == "Y1") {
        ShieldDetGroup = 3;
      }
      else {
        if (g_Verbosity >= c_Warning) {
          cout << m_Name << ": Warning - shield detector " << DetectorID 
                << " not found in any panel group" << endl;
        }
      continue;
      }

      // Check deadtime conditions
      if (m_ShieldLastHitTime[ShieldDetGroup] + m_ShieldDeadtime[ShieldDetGroup] < m_EventTime) {
        // Event occurred after deadtime - start new veto window
        // for (int group = 0; group < nShieldPanels; group++) {
        //   m_ShieldHitCrystalID[group].clear();
        // }
        m_ShieldHitCrystalID[ShieldDetGroup].clear();
        m_ShieldLastHitTime[ShieldDetGroup] = m_EventTime;
        m_ShieldVetoTime = m_EventTime;
        m_ShieldHitCrystalID[ShieldDetGroup].push_back(CrystalID);
        m_HasVeto = true;
        m_TotalShieldDeadtime[ShieldDetGroup] += m_ShieldDeadtime[ShieldDetGroup];
      }
      else if (m_ShieldLastHitTime[ShieldDetGroup] + m_ShieldDelayBefore > m_EventTime) {
        // Event occurred within coincidence window - add to existing veto
        m_ShieldVetoTime = m_EventTime;
        m_ShieldHitCrystalID[ShieldDetGroup].push_back(CrystalID);
        m_HasVeto = true;
      }
      else if (m_ShieldLastHitTime[ShieldDetGroup] + m_ShieldDeadtime[ShieldDetGroup] > m_EventTime) {
        // Event occurred within deadtime
        m_IsShieldDead = true;
        m_NumBGOHitsErased += 1;
      }
    }
  }

  // Calculate deadtime for each panel group after processing all hits
  for (int group = 0; group < nShieldPanels; group++) {
    if (!m_IsShieldDead) {
      m_ShieldDeadtime[group] = CalculateASICDeadtime(m_ShieldHitCrystalID[group]);
      // cout << "m_ShieldDeadtime[group]" << m_ShieldDeadtime[group] << endl;
    }
  }

  // Check if event is within veto window
  if (((m_ShieldVetoTime + m_ShieldVetoWindowSize) >= m_EventTime) && 
      (m_EventTime >= m_ShieldVetoTime)) {
    m_HasVeto = true;
    if (Event->GetSimulatedEvent() != nullptr) {
      m_ShieldVetoCounter += Event->GetSimulatedEvent()->GetNHTs();
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::ParseDeadtimeFile()
{
  // Read in deadtime parameters file
  // Format: StripCoincidenceWindow ASICDeadTimePerChannel StripDelayAfter1 StripDelayAfter2

  MParser Parser;
  if (Parser.Open(m_DeadtimeFileName) == false) {
    cout << m_Name << ": Unable to open deadtime parameters file: " << m_DeadtimeFileName << endl;
    return false;
  }

  if (Parser.GetNLines() < 2) {
    cout << m_Name << ": Deadtime file does not have enough data" << endl;
    return false;
  }

  // We only need the ASICDeadTimePerChannel (second value) for shield
  m_ASICDeadTimePerChannel = Parser.GetTokenizerAt(1)->GetTokenAtAsDouble(1);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine for shield trigger and veto

  m_HasTrigger = false;
  m_HasVeto = false;
  m_IsShieldDead = false;

  // Process shield hits and check for veto conditions
  ProcessShieldHits(Event);

  // Update time tracking for statistics
  double eventTime = Event->GetTime().GetAsSeconds();
  if (eventTime < m_FirstTime) {
    m_FirstTime = eventTime;
  }
  if (eventTime > m_LastTime) {
    m_LastTime = eventTime;
  }

  // If vetoed, set the dead time end
  if (m_HasVeto) {
    // Calculate the maximum deadtime end across all panels
    double maxDeadTimeEnd = 0.0;
    for (int i = 0; i < nShieldPanels; i++) {
      double thisEnd = m_ShieldLastHitTime[i] + m_ShieldDeadtime[i];
      if (thisEnd > maxDeadTimeEnd) {
        maxDeadTimeEnd = thisEnd;
      }
    }
    m_DeadTimeEnd = MTime(maxDeadTimeEnd);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldTrigger::Finalize()
{
  // Finalize the analysis - do all cleanup

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
  cout << "Shield vetoes: " << m_ShieldVetoCounter << endl;
  
  if (simTime > 0) {
    double rateAfterDT = (m_NumShieldHitCounts - m_NumBGOHitsErased) / simTime;
    cout << "Shield rate after deadtime: " << rateAfterDT << " cps" << endl;
  }

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* DeadtimeFileNode = Node->GetNode("DeadtimeFileName");
  if (DeadtimeFileNode != 0) {
    m_DeadtimeFileName = DeadtimeFileNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleShieldTrigger::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  new MXmlNode(Node, "DeadtimeFileName", m_DeadtimeFileName);

  return Node;
}


// MSubModuleShieldTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////