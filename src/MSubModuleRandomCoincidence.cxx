/*
 * MSubModuleRandomCoincidence.cxx
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
// MSubModuleRandomCoincidence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleRandomCoincidence.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleRandomCoincidence)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleRandomCoincidence::MSubModuleRandomCoincidence() : MSubModule()
{
  // Construct an instance of MSubModuleRandomCoincidence

  m_Name = "DEE random coincidence module";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleRandomCoincidence::~MSubModuleRandomCoincidence()
{
  // Delete this instance of MSubModuleRandomCoincidence
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleRandomCoincidence::Initialize()
{
  // Initialize the module

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleRandomCoincidence::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleRandomCoincidence::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleRandomCoincidence::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleRandomCoincidence::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MSubModuleRandomCoincidence::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleRandomCoincidence.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
