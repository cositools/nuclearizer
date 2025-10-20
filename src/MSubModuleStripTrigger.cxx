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

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleStripTrigger)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripTrigger::MSubModuleStripTrigger() : MSubModule()
{
  // Construct an instance of MSubModuleStripTrigger

  m_Name = "DEE strip trigger module";
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

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripTrigger::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripTrigger::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripTrigger::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MSubModuleStripTrigger::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleStripTrigger.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
