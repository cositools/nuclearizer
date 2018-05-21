/*
 * MNCTModuleStripPairing.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
// MNCTModuleStripPairing
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleStripPairing.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:



////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNCTModuleStripPairing)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleStripPairing::MNCTModuleStripPairing() : MModule()
{
  // Construct an instance of MNCTModuleStripPairing

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Strip pairing";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "StripPairing";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_StripPairing);

  // Set all modules, which can follow this module

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleStripPairing::~MNCTModuleStripPairing()
{
  // Delete this instance of MNCTModuleStripPairing
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleStripPairing::Initialize()
{
  // Initialize the module 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleStripPairing::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleStripPairing::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleStripPairing.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
