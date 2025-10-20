/*
 * MSubModuleStripReadoutNoise.cxx
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
// MSubModuleStripReadoutNoise
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleStripReadoutNoise.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleStripReadoutNoise)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadoutNoise::MSubModuleStripReadoutNoise() : MSubModule()
{
  // Construct an instance of MSubModuleStripReadoutNoise

  m_Name = "DEE strip readout module";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadoutNoise::~MSubModuleStripReadoutNoise()
{
  // Delete this instance of MSubModuleStripReadoutNoise
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadoutNoise::Initialize()
{
  // Initialize the module

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripReadoutNoise::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadoutNoise::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  // Dummy code:
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    SH.m_ADC = 2000 + 4*SH.m_Energy;
    if (SH.m_ADC > 16383) SH.m_ADC = 16383;
  }
  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    SH.m_ADC = 2000 + 4*SH.m_Energy;
    if (SH.m_ADC > 16383) SH.m_ADC = 16383;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripReadoutNoise::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadoutNoise::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MSubModuleStripReadoutNoise::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleStripReadoutNoise.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
