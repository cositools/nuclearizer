/*
 * MSubModuleDEEOutput.cxx
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
// MSubModuleDEEOutput
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleDEEOutput.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleDEEOutput)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleDEEOutput::MSubModuleDEEOutput() : MSubModule()
{
  // Construct an instance of MSubModuleDEEOutput

  m_Name = "DEE output module";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleDEEOutput::~MSubModuleDEEOutput()
{
  // Delete this instance of MSubModuleDEEOutput
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDEEOutput::Initialize()
{
  // Initialize the module

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDEEOutput::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDEEOutput::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  // Convert the DEE strip hits to standard strip hits
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    Event->AddStripHit(SH.Convert());
  }
  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    Event->AddStripHit(SH.Convert());
  }
  list<MDEECrystalHit>& CHits = Event->GetDEECrystalHitListReference();
  for (MDEECrystalHit& CH: CHits) {
    Event->AddCrystalHit(CH.Convert());
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDEEOutput::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDEEOutput::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MSubModuleDEEOutput::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration

  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleDEEOutput.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
