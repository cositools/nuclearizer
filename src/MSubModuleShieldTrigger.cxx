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
  // Main data analysis routine, which updates the event to a new level 

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
