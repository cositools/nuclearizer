/*
 * MModuleDiagnostics.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer
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
// MModuleDiagnostics
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleDiagnostics.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MTime.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleDiagnostics)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleDiagnostics::MModuleDiagnostics() : MModule()
{
  // Construct an instance of MModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Diagnostics (can be run at any spot after reading)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagDiagnostics";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_Diagnostics);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // Set if this module has an options GUI
  m_HasOptionsGUI = false;

  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleDiagnostics::~MModuleDiagnostics()
{
  // Destructor
}


////////////////////////////////////////////////////////////////////////////////


void MModuleDiagnostics::CreateExpos()
{
  // Create all expos

  if (HasExpos() == true) return;

  // Set the histogram display
  m_ExpoDiagnostics = new MGUIExpoDiagnostics(this);
  m_ExpoDiagnostics->SetStripHitHistogramParameters(64, -0.5, 63.5, 200, 0, 2000);
  m_Expos.push_back(m_ExpoDiagnostics);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDiagnostics::Initialize()
{
  // Initialize the module - nothing to be done
  
  return MModule::Initialize();
}

  

////////////////////////////////////////////////////////////////////////////////


void MModuleDiagnostics::Finalize()
{
  // Finalize the module - nothing to be done

  MModule::Finalize();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleDiagnostics::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Write the event to disk
 
  if (HasExpos() == true) {
    for (unsigned int sh = 0; sh < Event->GetNStripHits(); ++sh) {
      m_ExpoDiagnostics->AddStripHit(Event->GetStripHit(sh));
    }
  }

  return true;
}


// MModuleDiagnostics.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
