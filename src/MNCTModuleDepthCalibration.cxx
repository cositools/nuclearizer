/*
 * MNCTModuleDepthCalibration.cxx
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
// MNCTModuleDepthCalibration
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDepthCalibration.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleDepthCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibration::MNCTModuleDepthCalibration() : MNCTModule()
{
  // Construct an instance of MNCTModuleDepthCalibration

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Depth calibration";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DepthCalibration";

  // Set all modules, which have to be done before this module
  //AddPreceedingModuleType(c_DetectorEffectsEngine);
  AddPreceedingModuleType(c_EnergyCalibration);
  AddPreceedingModuleType(c_ChargeSharingCorrection);
  //AddPreceedingModuleType(c_DepthCorrection);
  //AddPreceedingModuleType(c_StripPairing);
  //AddPreceedingModuleType(c_EventReconstruction);

  // Set all types this modules handles
  //AddModuleType(c_DetectorEffectsEngine);
  //AddModuleType(c_EnergyCalibration);
  //AddModuleType(c_ChargeSharingCorrection);
  AddModuleType(c_DepthCorrection);
  //AddModuleType(c_StripPairing);
  //AddModuleType(c_EventReconstruction);

  // Set all modules, which can follow this module
  //AddSucceedingModuleType(c_DetectorEffectsEngine);
  //AddSucceedingModuleType(c_EnergyCalibration);
  //AddSucceedingModuleType(c_ChargeSharingCorrection);
  //AddSucceedingModuleType(c_DepthCorrection);
  AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_Aspect);
  AddSucceedingModuleType(c_EventReconstruction);
  AddSucceedingModuleType(c_Else);

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibration::~MNCTModuleDepthCalibration()
{
  // Delete this instance of MNCTModuleDepthCalibration
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibration::Initialize()
{
  // Initialize the module 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibration::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleDepthCalibration::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleDepthCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
