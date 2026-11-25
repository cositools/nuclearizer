/*
 * MModuleLoaderMeasurements.cxx
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
// MModuleLoaderMeasurements
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleLoaderMeasurements.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MDGeometryQuest.h"
#include "MDDetector.h"
#include "MGUIOptionsLoaderMeasurements.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleLoaderMeasurements)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurements::MModuleLoaderMeasurements() : MModule()
{
  // Construct an instance of MModuleLoaderMeasurements

  // Set all modules, which have to be done before this module
  // None
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventLoader);
  AddModuleType(MAssembly::c_EventLoaderMeasurement);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  m_XmlTag = "XmlTagMeasurementLoader";

  m_HasOptionsGUI = true;
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurements::~MModuleLoaderMeasurements()
{
  // Delete this instance of MModuleLoaderMeasurements
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurements::Initialize()
{
  // Initialize the module 

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurements::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurements::LoadDetectorIDsFromGeometry()
{
  // Collect the detector IDs of all GeDs in the geometry

  m_DetectorIDsInGeometry.clear();

  if (m_Geometry == nullptr) return;

  for (MDDetector* Detector : m_Geometry->GetDetectorList()) {
    MString Name = Detector->GetName();
    if (Detector->GetTypeName() == "Strip3D" && Name.BeginsWith("GeD_") == true) {
      Name.RemoveAllInPlace("GeD_");
      m_DetectorIDsInGeometry.insert(Name.ToUnsignedInt());
    }
  }

  // Keep all strip hits if the detector names don't follow convention
  if (m_DetectorIDsInGeometry.empty() == true && g_Verbosity >= c_Warning) {
    cout<<m_XmlTag<<": No GeD_X detectors found in the geometry - keeping strip hits from all detectors"<<endl;
  }

  if (g_Verbosity >= c_Info) {
    cout<<m_XmlTag<<": Detector IDs in the geometry:";
    for (unsigned int DetID : m_DetectorIDsInGeometry) cout<<" "<<DetID;
    cout<<endl;
  }
}


// MModuleLoaderMeasurements.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
