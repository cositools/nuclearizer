/*
 * MSubModuleStripReadout.cxx
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
// MSubModuleStripReadout
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleStripReadout.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleStripReadout)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadout::MSubModuleStripReadout() : MSubModule()
{
  // Construct an instance of MSubModuleStripReadout

  m_Name = "DEE strip readout module";

  m_EnergyCalibrationFileName = "";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadout::~MSubModuleStripReadout()
{
  // Delete this instance of MSubModuleStripReadout
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadout::Initialize()
{
  // Initialize the module

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripReadout::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadout::AnalyzeEvent(MReadOutAssembly* Event)
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


void MSubModuleStripReadout::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadout::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* N = Node->GetNode("EnergyCalibrationFileName");
  if (N != nullptr) {
    m_EnergyCalibrationFileName = N->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleStripReadout::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  new MXmlNode(Node, "EnergyCalibrationFileName", m_EnergyCalibrationFileName);

  return Node;
}


// MSubModuleStripReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
