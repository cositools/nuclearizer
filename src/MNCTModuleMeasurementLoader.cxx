/*
 * MNCTModuleMeasurementLoader.cxx
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
// MNCTModuleMeasurementLoader
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleMeasurementLoader.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsMeasurementLoader.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleMeasurementLoader)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleMeasurementLoader::MNCTModuleMeasurementLoader() : MModule()
{
  // Construct an instance of MNCTModuleMeasurementLoader

  // Set all modules, which have to be done before this module
  // None
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventLoader);
  AddModuleType(MAssembly::c_EventLoaderMeasurement);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  m_HasOptionsGUI = true;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleMeasurementLoader::~MNCTModuleMeasurementLoader()
{
  // Delete this instance of MNCTModuleMeasurementLoader
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoader::Initialize()
{
  // Initialize the module 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoader::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


///////////////////////////////////////////////////////////////////////////////


void MNCTModuleMeasurementLoader::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsMeasurementLoader* Options = new MGUIOptionsMeasurementLoader(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


// MNCTModuleMeasurementLoader.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
