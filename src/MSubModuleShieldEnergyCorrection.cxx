/*
 * MSubModuleShieldEnergyCorrection.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Valentina Fioretti.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer, Valentina Fioretti.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MSubModuleShieldEnergyCorrection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleShieldEnergyCorrection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleShieldEnergyCorrection)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldEnergyCorrection::MSubModuleShieldEnergyCorrection()
    : MSubModule()
{
  // Construct an instance of MSubModuleShieldEnergyCorrection

  m_Name = "DEE shield energy correction module";
  m_ShieldEnergyCorrectionFileName = "";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldEnergyCorrection::~MSubModuleShieldEnergyCorrection()
{
  // Delete this instance of MSubModuleShieldEnergyCorrection
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldEnergyCorrection::Initialize()
{
  // Initialize the module

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldEnergyCorrection::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldEnergyCorrection::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level

  // Set the energy
  list<MDEECrystalHit>& Hits = Event->GetDEECrystalHitListReference();
  for (MDEECrystalHit& CH : Hits) {
    CH.m_Energy = CH.m_SimulatedEnergy;
  }

  // Merge hits:
  for (auto IterLV1 = Hits.begin(); IterLV1 != Hits.end(); ++IterLV1) {
    auto IterLV2 = std::next(IterLV1);
    while (IterLV2 != Hits.end()) {
      if (IterLV1->m_ROE == IterLV2->m_ROE) {
        IterLV1->m_Energy += IterLV2->m_Energy;
        IterLV2 = Hits.erase(IterLV2);
      } else {
        ++IterLV2;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldEnergyCorrection::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize()

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldEnergyCorrection::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* ShieldEnergyCorrectionFileName = Node->GetNode("ShieldEnergyCorrectionFileName");
  if (ShieldEnergyCorrectionFileName != 0) {
    m_ShieldEnergyCorrectionFileName = ShieldEnergyCorrectionFileName->GetValue();
  }


  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleShieldEnergyCorrection::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration

  new MXmlNode(Node, "ShieldEnergyCorrectionFileName", m_ShieldEnergyCorrectionFileName);


  return Node;
}


// MSubModuleShieldEnergyCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
