// /*
//  * MSubModuleStripTrigger.cxx
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
// // MSubModuleStripTrigger
// //
// ////////////////////////////////////////////////////////////////////////////////


// // Include the header:
// #include "MSubModuleStripTrigger.h"

// // Standard libs:

// // ROOT libs:

// // MEGAlib libs:
// #include "MSubModule.h"


// ////////////////////////////////////////////////////////////////////////////////


// #ifdef ___CLING___
// ClassImp(MSubModuleStripTrigger)
// #endif


// ////////////////////////////////////////////////////////////////////////////////


// MSubModuleStripTrigger::MSubModuleStripTrigger() : MSubModule()
// {
//   // Construct an instance of MSubModuleStripTrigger

//   m_Name = "DEE strip trigger module";

//   m_HasTrigger = false;
//   m_HasVeto = false;
// }


// ////////////////////////////////////////////////////////////////////////////////


// MSubModuleStripTrigger::~MSubModuleStripTrigger()
// {
//   // Delete this instance of MSubModuleStripTrigger
// }


// ////////////////////////////////////////////////////////////////////////////////


// bool MSubModuleStripTrigger::Initialize()
// {
//   // Initialize the module

//   return MSubModule::Initialize();
// }


// ////////////////////////////////////////////////////////////////////////////////


// void MSubModuleStripTrigger::Clear()
// {
//   // Clear for the next event

//   m_HasTrigger = false;
//   m_HasVeto = false;

//   MSubModule::Clear();
// }


// ////////////////////////////////////////////////////////////////////////////////


// bool MSubModuleStripTrigger::AnalyzeEvent(MReadOutAssembly* Event)
// {
//   // Main data analysis routine, which updates the event to a new level 

//   m_HasTrigger = true;

//   return true;
// }


// ////////////////////////////////////////////////////////////////////////////////


// void MSubModuleStripTrigger::Finalize()
// {
//   // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

//   MSubModule::Finalize();
// }


// ////////////////////////////////////////////////////////////////////////////////


// bool MSubModuleStripTrigger::ReadXmlConfiguration(MXmlNode* Node)
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


// MXmlNode* MSubModuleStripTrigger::CreateXmlConfiguration(MXmlNode* Node)
// {
//   //! Create an XML node tree from the configuration
  
//   /*
//   MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
//   */

//   return Node;
// }


// // MSubModuleStripTrigger.cxx: the end...
// ////////////////////////////////////////////////////////////////////////////////


/*
 * MSubModuleStripTrigger.cxx
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
// MSubModuleStripTrigger
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleStripTrigger.h"

// Standard libs:
#include <algorithm>

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
  m_StripHitsErased = 0;

  // Initialize vectors
  m_ASICDeadTime = vector<vector<double>>(nDets, vector<double>(nASICs, 0.0));
  m_ASICHitStripID = vector<vector<vector<int>>>(nDets, vector<vector<int>>(nASICs));
  m_ASICHitStripID_noDT = vector<vector<vector<int>>>(nDets, vector<vector<int>>(nASICs));
  m_TempEvtTimes = vector<vector<vector<double>>>(nDets, vector<vector<double>>(nASICs));
  m_TriggerRates = vector<int>(nDets, 0);
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

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


double MSubModuleStripTrigger::dTimeASICs(vector<int> ASICChannels)
{
  // Calculate deadtime for GeD ASICs
  double deadtime = 0;
  int countUnique = 0;

  if (ASICChannels.empty()) {
    return 0.0;
  }

  unordered_set<int> ASICChannelsSet;

  // Sort ASICChannels to process channels in ascending order
  sort(ASICChannels.begin(), ASICChannels.end());

  // Loop through each channel ID in the sorted list
  for (int ID : ASICChannels) {
    if (ID == 64) {
      cout << "Strip ID is 64; should not happen" << endl;
      continue;
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
    int temp_size = ASICChannelsSet.size();

    if (ID == 64) {
      cout << "Strip ID is 64; should not happen" << endl;
      continue;
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
    
    int new_size = ASICChannelsSet.size();
    for (size_t j = 0; j < (new_size - temp_size); j++) {
      CountTimeVec.push_back(CountTime[i]);
    }
  }

  int h = 0;
  for (int k : ASICChannelsSet) {
    m_EventStripIDs.push_back(k);
    m_EventTimes.push_back(CountTimeVec[h]);
    h++;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripTrigger::UpdateDeadtime(int det, int ASIC, vector<int> stripIDs, bool isFirstHitAfterDead)
{
  // Update deadtime tracking for a detector/ASIC
  
  if (det < 0 || det >= nDets || ASIC < 0 || ASIC >= nASICs) {
    return;
  }

  if (isFirstHitAfterDead) {
    // Clear the lists after deadtime period
    for (int d = 0; d < nDets; d++) {
      for (int a = 0; a < nASICs; a++) {
        CountRate(m_ASICHitStripID[d][a], m_TempEvtTimes[d][a]);
        m_ASICHitStripID_noDT[d][a].clear();
        m_ASICHitStripID[d][a].clear();
        m_TempEvtTimes[d][a].clear();
      }
    }
    
    m_StripsTotalDeadtime += m_StripsCurrentDeadtime;
    m_StripsCurrentDeadtime = 0.0;
    m_ASICLastHitTime = m_EventTime;
  }

  // Add strip IDs to tracking vectors
  for (int stripID : stripIDs) {
    m_ASICHitStripID[det][ASIC].push_back(stripID);
    m_ASICHitStripID_noDT[det][ASIC].push_back(stripID);
    m_TempEvtTimes[det][ASIC].push_back(m_EventTime);
  }

  // Calculate new deadtime
  if (!m_IsGeDDead) {
    double newDeadtime = dTimeASICs(m_ASICHitStripID[det][ASIC]);
    if (newDeadtime > m_StripsCurrentDeadtime) {
      m_StripsCurrentDeadtime = newDeadtime;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine for strip trigger
  
  // For now, we just set the trigger flag to true
  // More complex logic will be implemented when integrating with DEE
  m_HasTrigger = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripTrigger::Finalize()
{
  // Finalize the analysis

  cout << "###################" << endl
       << "STRIP TRIGGER MODULE STATISTICS" << endl
       << "###################" << endl;
  
  cout << "Total dead time of the instrument: " << m_StripsTotalDeadtime << endl;
  cout << "Hits erased due to detector being dead: " << m_StripHitsErased << endl;
  
  if (m_StripsTotalDeadtime > 0) {
    cout << "Avg deadtime per strip hit: " << m_StripsTotalDeadtime << endl;
  }
  
  cout << "Trigger rates (events per detector):" << endl;
  for (int i = 0; i < nDets; i++) {
    cout << "  Detector " << i << ": " << m_TriggerRates[i] << endl;
  }

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::ParseDeadtimeFile()
{
  // Read in deadtime parameters file

  MParser Parser;
  if (Parser.Open(m_DeadtimeFileName) == false) {
    cout << "Unable to open deadtime parameters file: " << m_DeadtimeFileName << endl;
    return false;
  }

  if (Parser.GetNLines() < 2) {
    cout << "Deadtime file does not have enough data" << endl;
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
  if (DeadtimeFileNode != 0) {
    m_DeadtimeFileName = DeadtimeFileNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleStripTrigger::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* DeadtimeFileNode = new MXmlNode(Node, "DeadtimeFileName", m_DeadtimeFileName);
  return Node;
  
}


// MSubModuleStripTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////