/*
 * MSubModuleShieldReadout.cxx
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
// MSubModuleShieldReadout
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleShieldReadout.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleShieldReadout)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldReadout::MSubModuleShieldReadout() : MSubModule()
{
  // Construct an instance of MSubModuleShieldReadout

  m_Name = "DEE shield readout module";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldReadout::~MSubModuleShieldReadout()
{
  // Delete this instance of MSubModuleShieldReadout
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldReadout::Initialize()
{
  // Initialize the module

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldReadout::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldReadout::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  // Dummy code:
  list<MDEECrystalHit>& Hits = Event->GetDEECrystalHitListReference();
  for (MDEECrystalHit& CH: Hits) {
    CH.m_ADC = 2000 + 4*CH.m_Energy;
    if (CH.m_ADC > 16383) CH.m_ADC = 16383;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldReadout::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldReadout::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MSubModuleShieldReadout::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration

  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleShieldReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
