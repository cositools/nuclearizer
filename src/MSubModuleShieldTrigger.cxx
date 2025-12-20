// /*
//  * MSubModuleShieldTrigger.cxx
//  *
//  *
//  * Copyright (C) by Andreas Zoglauer.
//  * All rights reserved.
//  *
//  *
//  * This code implementation is the intellectual property of
//  * Andreas Zoglauer.
//  *
//  * By copying, distributing or modifying the Program (or any work
//  * based on the Program) you indicate your acceptance of this statement,
//  * and all its terms.
//  *
//  */


// ////////////////////////////////////////////////////////////////////////////////
// //
// // MSubModuleShieldTrigger
// //
// ////////////////////////////////////////////////////////////////////////////////


// // Include the header:
// #include "MSubModuleShieldTrigger.h"

// // Standard libs:

// // ROOT libs:

// // MEGAlib libs:
// #include "MSubModule.h"


// ////////////////////////////////////////////////////////////////////////////////


// #ifdef ___CLING___
// ClassImp(MSubModuleShieldTrigger)
// #endif


// ////////////////////////////////////////////////////////////////////////////////


// MSubModuleShieldTrigger::MSubModuleShieldTrigger() : MSubModule()
// {
//   // Construct an instance of MSubModuleShieldTrigger

//   m_Name = "DEE shield trigger module";

//   m_HasTrigger = false;
//   m_HasVeto = false;
// }


// ////////////////////////////////////////////////////////////////////////////////


// MSubModuleShieldTrigger::~MSubModuleShieldTrigger()
// {
//   // Delete this instance of MSubModuleShieldTrigger
// }


// ////////////////////////////////////////////////////////////////////////////////


// bool MSubModuleShieldTrigger::Initialize()
// {
//   // Initialize the module

//   return MSubModule::Initialize();
// }


// ////////////////////////////////////////////////////////////////////////////////


// void MSubModuleShieldTrigger::Clear()
// {
//   // Clear for the next event

//   m_HasTrigger = false;
//   m_HasVeto = false;

//   MSubModule::Clear();
// }


// ////////////////////////////////////////////////////////////////////////////////


// bool MSubModuleShieldTrigger::AnalyzeEvent(MReadOutAssembly* Event)
// {
//   // Main data analysis routine, which updates the event to a new level 

//   m_HasTrigger = false;

//   return true;
// }


// ////////////////////////////////////////////////////////////////////////////////


// void MSubModuleShieldTrigger::Finalize()
// {
//   // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

//   MSubModule::Finalize();
// }


// ////////////////////////////////////////////////////////////////////////////////


// bool MSubModuleShieldTrigger::ReadXmlConfiguration(MXmlNode* Node)
// {
//   //! Read the configuration data from an XML node

//   /*
//   MXmlNode* SomeTagNode = Node->GetNode("SomeTag");
//   if (SomeTagNode != 0) {
//     m_SomeTagValue = SomeTagNode->GetValue();
//   }
//   */

//   return true;
// }


// ////////////////////////////////////////////////////////////////////////////////


// MXmlNode* MSubModuleShieldTrigger::CreateXmlConfiguration(MXmlNode* Node)
// {
//   //! Create an XML node tree from the configuration
  
//   /*
//   MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
//   */

//   return Node;
// }


// // MSubModuleShieldTrigger.cxx: the end...
// ////////////////////////////////////////////////////////////////////////////////


/*
 * MSubModuleShieldTrigger.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleShieldTrigger)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldTrigger::MSubModuleShieldTrigger() : MSubModule()
{
  // Construct an instance of MSubModuleShieldTrigger

  m_Name = "DEE shield trigger module";

  m_SimEvent = nullptr;
  m_EventTime = 0.0;
  m_HasTrigger = false;
  m_HasVeto = false;
  m_IsShieldDead = false;

  // Initialize shield parameters with default values
  m_ShieldThreshold = 80.0;
  m_ShieldPulseDuration = 1.7e-6;
  m_ShieldDelayBefore = 0.1e-6;
  m_ShieldDelayAfter = 0.4e-6;
  m_ShieldVetoWindowSize = 1.5e-6;
  m_ASICDeadTimePerChannel = 0.0;
  m_ShieldVetoTime = 0.0;
  
  m_NumShieldHitCounts = 0;
  m_ShieldVetoCounter = 0;
  m_NumBGOHitsErased = 0;

  // Initialize vectors
  m_ShieldLastHitTime = vector<double>(nShieldPanels, -10.0);
  m_ShieldDeadtime = vector<double>(nShieldPanels, 0.0);
  m_TotalShieldDeadtime = vector<double>(nShieldPanels, 0.0);
  m_ShieldHitID = vector<vector<int>>(nShieldPanels);
  m_DetectorsHitForShieldVeto = vector<int>(nDets, 0);

  // Initialize shield panel groups
  m_ShieldPanelGroups = {
    { 0, 1, 2, 3 },
    { 4, 5, 6, 7 },
    { 8, 9, 10, 11 },
    { 12, 13, 14, 15 },
    { 16, 17, 18 },
    { 19, 20, 21 }
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

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldTrigger::Clear()
{
  // Clear for the next event

  m_HasTrigger = false;
  m_HasVeto = false;
  m_IsShieldDead = false;
  
  // Clear per-event data
  for (int i = 0; i < nShieldPanels; i++) {
    m_ShieldHitID[i].clear();
  }
  
  m_DetectorsHitForShieldVeto = vector<int>(nDets, 0);

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


double MSubModuleShieldTrigger::dTimeASICs(vector<int> ASICChannels)
{
  // Calculate deadtime for shield ASICs
  double deadtime = 0;
  int countUnique = 0;

  if (ASICChannels.empty()) {
    return 0.0;
  }

  unordered_set<int> BGOChannelsSet;
  for (int ID : ASICChannels) {
    BGOChannelsSet.insert(ID);
  }

  // Count the number of unique channels read out (2 for each hit in the BGO)
  countUnique = BGOChannelsSet.size() * 2;
  deadtime = m_ShieldDelayBefore + (m_ASICDeadTimePerChannel * countUnique) + m_ShieldDelayAfter;
  
  if (deadtime < m_ShieldPulseDuration) {
    deadtime = m_ShieldPulseDuration;
  }

  return deadtime;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine for shield trigger and veto

  m_HasTrigger = false;
  m_HasVeto = false;
  m_IsShieldDead = false;

  if (m_SimEvent == nullptr) {
    return true;
  }

  int ShieldDetNum = 0;
  double energy = 0;
  int ShieldDetGroup;

  // Process shield hits
  for (unsigned int h = 0; h < m_SimEvent->GetNHTs(); h++) {
    MSimHT* HT = m_SimEvent->GetHTAt(h);

    if (HT->GetDetectorType() == 8) { // Shield hit
      m_NumShieldHitCounts += 1;
      
      MDVolumeSequence* VS = HT->GetVolumeSequence();
      MDDetector* Detector = VS->GetDetector();
      MString FullDetName = Detector->GetName();
      MString DetName = FullDetName;

      // Parse detector number
      DetName.RemoveAllInPlace("BGO_X0_");
      ShieldDetNum = DetName.ToInt();
      ShieldDetNum = ShieldDetNum - 1;
      
      energy = HT->GetOriginalEnergy();
      HT->SetEnergy(energy);

      ShieldDetGroup = 0;

      if (energy > m_ShieldThreshold) {
        // Find which panel group this detector belongs to
        bool found = false;
        for (size_t i = 0; i < m_ShieldPanelGroups.size(); ++i) {
          for (size_t j = 0; j < m_ShieldPanelGroups[i].size(); ++j) {
            if (m_ShieldPanelGroups[i][j] == ShieldDetNum) {
              ShieldDetGroup = i;
              found = true;
              break;
            }
          }
          if (found) break;
        }

        // Check deadtime conditions
        if (m_ShieldLastHitTime[ShieldDetGroup] + m_ShieldDeadtime[ShieldDetGroup] < m_EventTime) {
          // Event occurred after deadtime
          for (int group = 0; group < nShieldPanels; group++) {
            m_ShieldHitID[group].clear();
          }
          m_ShieldLastHitTime[ShieldDetGroup] = m_EventTime;
          m_ShieldVetoTime = m_EventTime;
          m_ShieldHitID[ShieldDetGroup].push_back(ShieldDetNum);
          m_HasVeto = true;
          m_TotalShieldDeadtime[ShieldDetGroup] += m_ShieldDeadtime[ShieldDetGroup];
        }
        else if (m_ShieldLastHitTime[ShieldDetGroup] + m_ShieldDelayBefore > m_EventTime) {
          // Event occurred within coincidence window
          m_ShieldVetoTime = m_EventTime;
          m_ShieldHitID[ShieldDetGroup].push_back(ShieldDetNum);
          m_HasVeto = true;
        }
        else if (m_ShieldLastHitTime[ShieldDetGroup] + m_ShieldDeadtime[ShieldDetGroup] > m_EventTime) {
          // Event occurred within deadtime
          m_IsShieldDead = true;
          m_NumBGOHitsErased += 1;
        }
      }
    }
  }

  // Calculate deadtime for each panel group
  for (int group = 0; group < nShieldPanels; group++) {
    if (!m_IsShieldDead) {
      m_ShieldDeadtime[group] = dTimeASICs(m_ShieldHitID[group]);
    }
  }

  // Check if event is within veto window
  if (((m_ShieldVetoTime + m_ShieldVetoWindowSize) >= m_EventTime) && 
      (m_EventTime >= m_ShieldVetoTime)) {
    m_HasVeto = true;
    m_ShieldVetoCounter += m_SimEvent->GetNHTs();
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
  cout << "Total BGO hits before BGO deadtime: " << m_NumShieldHitCounts << endl;
  
  for (int i = 0; i < nShieldPanels; i++) {
    cout << "Shield Panel " << i << " dead time: " << m_TotalShieldDeadtime[i] << endl;
  }
  
  cout << "BGO hits erased due to BGO being dead: " << m_NumBGOHitsErased << endl;
  cout << "Shield vetoes: " << m_ShieldVetoCounter << endl;

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