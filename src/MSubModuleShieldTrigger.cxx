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

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::AnalyzeEvent(MReadOutAssembly* Event)
{
  MSimEvent* SimEvent = nullptr;
  SimEvent = Event->GetSimulatedEvent();

  MTime evt_time = SimEvent->GetTime();

  int ShieldDetNum = 0;
  double energy = 0;
  int ShieldDetGroup;
  m_HasVeto = false;

  bool m_IsShieldDead = false;

  for (unsigned int h=0; h<SimEvent->GetNHTs(); h++) {
    MSimHT* HT = SimEvent->GetHTAt(h);

    MDVolumeSequence* VS = HT->GetVolumeSequence();  // VF: to remove?
    MDDetector* Detector = VS->GetDetector(); // VF: to remove?
    MString DetName = Detector->GetName(); // VF: to remove?

    if (HT->GetDetectorType() == 8) {
      cout << "Shield Hit Detected" << endl;
    }
  }
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldTrigger::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldTrigger::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  /*
  MXmlNode* SomeTagNode = Node->GetNode("SomeTag");
  if (SomeTagNode != 0) {
    m_SomeTagValue = SomeTagNode->GetValue();
  }
  */

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleShieldTrigger::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleShieldTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
