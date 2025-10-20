/*
 * MSubModuleDepthReadout.cxx
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
// MSubModuleDepthReadout
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleDepthReadout.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleDepthReadout)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleDepthReadout::MSubModuleDepthReadout() : MSubModule()
{
  // Construct an instance of MSubModuleDepthReadout

  m_Name = "DEE depth readout module";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleDepthReadout::~MSubModuleDepthReadout()
{
  // Delete this instance of MSubModuleDepthReadout
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDepthReadout::Initialize()
{
  // Initialize the module

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDepthReadout::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDepthReadout::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  // Dummy code
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    if (SH.m_IsGuardRing == false) {
      SH.m_TAC = 8000 + 2000*SH.m_SimulatedRelativeDepth;
      if (SH.m_TAC > 16383) SH.m_TAC = 16383;
    }
  }
  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    if (SH.m_IsGuardRing == false) {
      SH.m_TAC = 8000 + 2000*(1.0-SH.m_SimulatedRelativeDepth);
      if (SH.m_TAC > 16383) SH.m_TAC = 16383;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDepthReadout::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDepthReadout::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MSubModuleDepthReadout::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleDepthReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
