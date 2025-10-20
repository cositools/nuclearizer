/*
 * MSubModuleDEEIntake.cxx
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
// MSubModuleDEEIntake
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleDEEIntake.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"
#include "MDVolume.h"
#include "MDShapeBRIK.h"
#include "MDShapeIntersection.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleDEEIntake)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleDEEIntake::MSubModuleDEEIntake() : MSubModule()
{
  // Construct an instance of MSubModuleDEEIntake

  m_Name = "DEE intake module";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleDEEIntake::~MSubModuleDEEIntake()
{
  // Delete this instance of MSubModuleDEEIntake
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDEEIntake::Initialize()
{
  // Initialize the module

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDEEIntake::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDEEIntake::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  Event->SetID(Event->GetSimulatedEvent()->GetID());
  Event->SetTime(Event->GetSimulatedEvent()->GetTime());

  for (unsigned int h = 0; h < Event->GetSimulatedEvent()->GetNHTs(); ++h) {
    MSimHT* HT = Event->GetSimulatedEvent()->GetHTAt(h);

    MDVolumeSequence* VS = HT->GetVolumeSequence();
    MDDetector* Detector = VS->GetDetector();
    if (Detector == nullptr) {
      if (g_Verbosity >= c_Error) cout<<m_Name<<": No detector found for this simulated hit"<<endl;
      continue;
    }
    MDVolume* SensitiveVolume = VS->GetSensitiveVolume();
    if (SensitiveVolume == nullptr) {
      if (g_Verbosity >= c_Error) cout<<m_Name<<": No sensitive volume found for this simulated hit"<<endl;
      continue;
    }
    MDShapeBRIK* Shape = nullptr;
    if (SensitiveVolume->GetShape()->GetType() == "BRIK") {
      Shape = dynamic_cast<MDShapeBRIK*>(SensitiveVolume->GetShape());
    } else if (SensitiveVolume->GetShape()->GetType() == "Intersection") {
      if (dynamic_cast<MDShapeIntersection*>(SensitiveVolume->GetShape())->GetShapeA()->GetType() == "BRIK") {
        Shape = dynamic_cast<MDShapeBRIK*>(dynamic_cast<MDShapeIntersection*>(SensitiveVolume->GetShape())->GetShapeA());
      } else {
        if (g_Verbosity >= c_Error) cout<<m_Name<<": No box-shaped sensitive volume found for this simulated hit"<<endl;
        continue;
      }
    } else {
      if (g_Verbosity >= c_Error) cout<<m_Name<<": No box-shaped sensitive volume found for this simulated hit"<<endl;
      continue;
    }
    double DetectorDepth = Shape->GetSizeZ();

    MString DetectorName = Detector->GetName();
    if (DetectorName.BeginsWith("GeD") == true) {
      DetectorName.RemoveAllInPlace("GeD_"); // The number after GeD is the COSI detector ID
      int DetectorID = DetectorName.ToInt();

      MDEEStripHit LVHit;
      MDEEStripHit HVHit;

      // Fill out what we can - only some can be done now,

      // Sim info
      LVHit.m_SimulatedEventID = Event->GetSimulatedEvent()->GetID();
      HVHit.m_SimulatedEventID = Event->GetSimulatedEvent()->GetID();

      LVHit.m_SimulatedHitIndex = h;
      HVHit.m_SimulatedHitIndex = h;

      // Sivan provides a vector and we want a list her (why??)
      auto HTOrigins = HT->GetOrigins();
      vector<int> Origins(HTOrigins.begin(), HTOrigins.end());
      LVHit.m_SimulatedOrigins = list<int>(Origins.begin(), Origins.end());
      HVHit.m_SimulatedOrigins = list<int>(Origins.begin(), Origins.end());

      LVHit.m_SimulatedPosition = HT->GetPosition();
      HVHit.m_SimulatedPosition = HT->GetPosition();
      LVHit.m_SimulatedEnergy = HT->GetEnergy();
      HVHit.m_SimulatedEnergy = HT->GetEnergy();

      //! The simulated depth in the detector
      MVector PositionInDetector = VS->GetPositionInSensitiveVolume();
      LVHit.m_SimulatedPositionInDetector = PositionInDetector;
      HVHit.m_SimulatedPositionInDetector = PositionInDetector;

      LVHit.m_SimulatedRelativeDepth = (LVHit.m_SimulatedPositionInDetector.Z() + 0.5*DetectorDepth)/DetectorDepth;
      HVHit.m_SimulatedRelativeDepth = (HVHit.m_SimulatedPositionInDetector.Z() + 0.5*DetectorDepth)/DetectorDepth;

      // Assume not a guard ring - we only know after the charge transport
      LVHit.m_SimulatedIsGuardRing = false;
      HVHit.m_SimulatedIsGuardRing = false;

      // Add a unique identifiers
      LVHit.m_ID = 1000+(2*h);
      HVHit.m_ID = 1000+(2*h+1);

      LVHit.m_OppositeSideID = HVHit.m_ID;
      HVHit.m_OppositeSideID = LVHit.m_ID;

      LVHit.m_ROE.IsLowVoltageStrip(true);
      HVHit.m_ROE.IsLowVoltageStrip(false);

      LVHit.m_ROE.SetDetectorID(DetectorID);
      HVHit.m_ROE.SetDetectorID(DetectorID);

      // The rest will be filled in later

      // Event will be responsible for deleting the event
      cout<<"Adding LV hit"<<endl;
      Event->AddDEEStripHitLV(LVHit);
      Event->AddDEEStripHitHV(HVHit);

    } else if (DetectorName.BeginsWith("ACS_Crystal_") == true) {
      // later
    } else {
      if (g_Verbosity >= c_Error) cout<<m_Name<<": No GeD_ volumes found"<<endl;
      continue;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDEEIntake::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDEEIntake::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MSubModuleDEEIntake::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration

  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleDEEIntake.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
