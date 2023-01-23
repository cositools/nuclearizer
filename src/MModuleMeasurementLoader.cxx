/*
 * MModuleMeasurementLoader.cxx
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
// MModuleMeasurementLoader
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleMeasurementLoader.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsMeasurementLoader.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleMeasurementLoader)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleMeasurementLoader::MModuleMeasurementLoader() : MModule()
{
  // Construct an instance of MModuleMeasurementLoader

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


MModuleMeasurementLoader::~MModuleMeasurementLoader()
{
  // Delete this instance of MModuleMeasurementLoader
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleMeasurementLoader::Initialize()
{
  // Initialize the module 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleMeasurementLoader::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


///////////////////////////////////////////////////////////////////////////////


void MModuleMeasurementLoader::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsMeasurementLoader* Options = new MGUIOptionsMeasurementLoader(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


// MModuleMeasurementLoader.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
