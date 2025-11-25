/*
 * MSubModuleChargeTransport.cxx
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
// MSubModuleChargeTransport
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleChargeTransport.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleChargeTransport)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleChargeTransport::MSubModuleChargeTransport() : MSubModule()
{
  // Construct an instance of MSubModuleChargeTransport


}


////////////////////////////////////////////////////////////////////////////////


MSubModuleChargeTransport::~MSubModuleChargeTransport()
{
  // Delete this instance of MSubModuleChargeTransport
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeTransport::Initialize()
{
  // Initialize the module

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleChargeTransport::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeTransport::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  // Dummy code:

  // Create strip hits
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    MVector Pos = SH.m_SimulatedPositionInDetector;

    // Calculate LV strip ID by rounding down intentionally to avoid truncation towards zero
    // TODO: Confirm the correct strip pitch based on SMEX detector models
    int ID = static_cast<int>(std::floor((Pos.X() + 7.4/2.0) / (7.4/64.0)));

    // Check for strip ID and if the position is within the allowed strip length or on the guard ring
    // TODO: Confirm the correct boundary of the guard ring based on SMEX detector models
    if (ID >= 0 && ID < 64 && std::abs(Pos.Y()) <= 7.4/2.0 && std::hypot(Pos.X(), Pos.Y()) <= 4.712) {
      SH.m_ROE.SetStripID(ID);
      SH.m_IsGuardRing = false;
    } else {
      SH.m_ROE.SetStripID(64);
      SH.m_IsGuardRing = true;
    }
    SH.m_Energy = SH.m_SimulatedEnergy;
  }

  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    MVector Pos = SH.m_SimulatedPositionInDetector;

    // Calculate HV strip ID by rounding down intentionally to avoid truncation towards zero
    // TODO: Confirm the correct strip pitch based on SMEX detector models
    int ID = static_cast<int>(std::floor((Pos.Y() + 7.4/2.0) / (7.4/64.0)));

    // Check for strip ID and if the position is within the allowed strip length or on the guard ring
    // TODO: Confirm the correct boundary of the guard ring based on SMEX detector models
    if (ID >= 0 && ID < 64 && std::abs(Pos.X()) <= 7.4/2.0 && std::hypot(Pos.X(), Pos.Y()) <= 4.712) {
      SH.m_ROE.SetStripID(ID);
      SH.m_IsGuardRing = false;
    } else {
      SH.m_ROE.SetStripID(64);
      SH.m_IsGuardRing = true;
    }
    SH.m_Energy = SH.m_SimulatedEnergy;
  }

  // Merge hits:
  for (auto IterLV1 = LVHits.begin(); IterLV1 != LVHits.end(); ++IterLV1) {
    auto IterLV2 = std::next(IterLV1);
    while (IterLV2 != LVHits.end()) {
      if (IterLV1->m_ROE == IterLV2->m_ROE) {
        IterLV1->m_Energy += IterLV2->m_Energy;
        IterLV2 = LVHits.erase(IterLV2);
      } else {
        ++IterLV2;
      }
    }
  }
  for (auto IterHV1 = HVHits.begin(); IterHV1 != HVHits.end(); ++IterHV1) {
    auto IterHV2 = std::next(IterHV1);
    while (IterHV2 != HVHits.end()) {
      if (IterHV1->m_ROE == IterHV2->m_ROE) {
        IterHV1->m_Energy += IterHV2->m_Energy;
        IterHV2 = HVHits.erase(IterHV2);
      } else {
        ++IterHV2;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleChargeTransport::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeTransport::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MSubModuleChargeTransport::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration

  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleChargeTransport.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
