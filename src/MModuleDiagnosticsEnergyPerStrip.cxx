/*
 * MModuleDiagnosticsEnergyPerStrip.cxx
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
// MModuleDiagnosticsEnergyPerStrip
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleDiagnosticsEnergyPerStrip.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MModule.h"
#include "MGUIOptionsTemplate.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleDiagnosticsEnergyPerStrip)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleDiagnosticsEnergyPerStrip::MModuleDiagnosticsEnergyPerStrip() : MModule()
{
  // Construct an instance of MModuleDiagnosticsEnergyPerStrip

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Diagnostics: Energy Per Strip";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagDiagnosticsEnergyPerStrip";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_Diagnostics);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  // Set all modules, which can follow this module

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options

  // Can the program be run multi-threaded
  m_AllowMultiThreading = true;

  // Can we use multiple instances of this class
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleDiagnosticsEnergyPerStrip::~MModuleDiagnosticsEnergyPerStrip()
{
  // Delete this instance of MModuleDiagnosticsEnergyPerStrip
}


////////////////////////////////////////////////////////////////////////////////


void MModuleDiagnosticsEnergyPerStrip::CreateExpos()
{
  // Create all expos

  if (HasExpos() == true) return;

  // Set the histogram display
  m_ExpoDiagnosticsEnergyPerStrip = new MGUIExpoDiagnosticsEnergyPerStrip(this);
  m_ExpoDiagnosticsEnergyPerStrip->SetStripHitHistogramParameters(64, -0.5, 63.5, 200, 0, 2000);
  m_Expos.push_back(m_ExpoDiagnosticsEnergyPerStrip);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDiagnosticsEnergyPerStrip::Initialize()
{
  // Initialize the module 

  m_HasEnergies = false;
  m_HasHits = false;
  m_NSetupEvents = 10;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDiagnosticsEnergyPerStrip::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 


  if (HasExpos() == true) {
    if (m_NSetupEvents > 0) {
      for (unsigned int sh = 0; sh < Event->GetNStripHits(); ++sh) {
        if (Event->GetStripHit(sh)->GetEnergy() > 0) {
          m_HasEnergies = true;
          break;
        }
      }
      if (Event->GetNHits() > 0) {
        m_HasHits = true;
      }
      m_NSetupEvents--;

      if (m_NSetupEvents == 0 && m_HasEnergies == false) {
        m_ExpoDiagnosticsEnergyPerStrip->SetStripHitHistogramParameters(64, -0.5, 63.5, 256, 0, 16384);
      }
    } else {
      if (m_HasHits == true) {
        for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
          m_ExpoDiagnosticsEnergyPerStrip->AddHit(Event->GetHit(h));
        }
      } else {
        for (unsigned int sh = 0; sh < Event->GetNStripHits(); ++sh) {
          m_ExpoDiagnosticsEnergyPerStrip->AddStripHit(Event->GetStripHit(sh), m_HasEnergies);
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleDiagnosticsEnergyPerStrip::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MModule::Finalize();
}


// MModuleDiagnosticsEnergyPerStrip.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
