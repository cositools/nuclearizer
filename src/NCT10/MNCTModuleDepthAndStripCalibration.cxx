/*
 * MNCTModuleDepthAndStripCalibration.cxx
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
// MNCTModuleDepthAndStripCalibration
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDepthAndStripCalibration.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleDepthAndStripCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthAndStripCalibration::MNCTModuleDepthAndStripCalibration() : MModule()
{
  // Construct an instance of MNCTModuleDepthAndStripCalibration

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Combined depth calibration and strip pairing";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "CombinedDepthCalibrationAndStripPairing";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_ChargeSharingCorrection);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_DepthCorrection);
  AddModuleType(MAssembly::c_StripPairing);

  // Set all modules, which can follow this module

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthAndStripCalibration::~MNCTModuleDepthAndStripCalibration()
{
  // Delete this instance of MNCTModuleDepthAndStripCalibration
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthAndStripCalibration::Initialize()
{
  // Initialize the module 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthAndStripCalibration::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleDepthAndStripCalibration::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleDepthAndStripCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
