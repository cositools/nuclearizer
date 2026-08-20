/*
 * MModuleEnergyCalibration.cxx
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
// MModuleEnergyCalibration
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleEnergyCalibration.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleEnergyCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleEnergyCalibration::MModuleEnergyCalibration() : MModule()
{
  // Construct an instance of MModuleEnergyCalibration

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Combined energy calibration and charge sharing correction";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "CombinedEnergyCalibrationAndChargeSharingCorrection";

  // Set all modules, which have to be done before this module
  AddModuleType(MAssembly::c_EventLoader);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_EnergyCalibration);
  AddModuleType(MAssembly::c_ChargeSharingCorrection);

  // Set all modules, which can follow this module

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MModuleEnergyCalibration::~MModuleEnergyCalibration()
{
  // Delete this instance of MModuleEnergyCalibration
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibration::Initialize()
{
  // Initialize the module 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibration::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEnergyCalibration::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MModuleEnergyCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
