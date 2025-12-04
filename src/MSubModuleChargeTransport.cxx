/*
 * MSubModuleChargeTransport.cxx
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
// MSubModuleChargeTransport
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleChargeTransport.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleChargeTransport)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleChargeTransport::MSubModuleChargeTransport() : MSubModule()
{
  // Construct an instance of MSubModuleChargeTransport


}


////////////////////////////////////////////////////////////////////////////////


MSubModuleChargeTransport::~MSubModuleChargeTransport()
{
  // Delete this instance of MSubModuleChargeTransport
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeTransport::Initialize()
{

  // The detectors need to be in the same order as DetIDs.
  // ie DetID=0 should be the 0th detector in m_Detectors, DetID=1 should the 1st, etc.
  vector<MDDetector*> DetList = m_Geometry->GetDetectorList();

  // Look through the Geometry and get the names and thicknesses of all the detectors.
  for(unsigned int i = 0; i < DetList.size(); ++i){

    unsigned int DetID = i;

    MDDetector* det = DetList[i];
    vector<string> DetectorNames;
    if (det->GetTypeName() == "Strip3D") {
      if (det->GetNSensitiveVolumes() == 1) {
        MDVolume* vol = det->GetSensitiveVolume(0);
        string det_name = vol->GetName().GetString();
        if (find(DetectorNames.begin(), DetectorNames.end(), det_name) == DetectorNames.end()) {
          DetectorNames.push_back(det_name);
          m_Thicknesses[DetID] = 2*(det->GetStructuralSize().GetZ());
          MDStrip3D* strip = dynamic_cast<MDStrip3D*>(det);
          m_XPitches[DetID] = strip->GetPitchX();
          m_YPitches[DetID] = strip->GetPitchY();
          m_NXStrips[DetID] = strip->GetNStripsX();
          m_NYStrips[DetID] = strip->GetNStripsY();
          m_XWidths[DetID] = strip->GetWidthX();
          m_YWidths[DetID] = strip->GetWidthY();
          if (g_Verbosity >= c_Info) {
            cout << "Found detector " << det_name << " corresponding to DetID=" << DetID << "." << endl;
            cout << "Detector width (X): " << m_XWidths[DetID] << endl;
            cout << "Detector width (Y): " << m_YWidths[DetID] << endl;
            cout << "Detector thickness: " << m_Thicknesses[DetID] << endl;
            cout << "Number of X strips: " << m_NXStrips[DetID] << endl;
            cout << "Number of Y strips: " << m_NYStrips[DetID] << endl;
            cout << "X strip pitch: " << m_XPitches[DetID] << endl;
            cout << "Y strip pitch: " << m_YPitches[DetID] << endl;
          }
          m_DetectorIDs.push_back(DetID);
          m_Detectors[DetID] = det;
        } else {
          cout << "ERROR in MSubModuleChargeTransport::Initialize: Found a duplicate detector: " << det_name << endl;
        }
      } else {
        cout << "ERROR in MSubModuleChargeTransport::Initialize: Found a Strip3D detector with " << det->GetNSensitiveVolumes() << " Sensitive Volumes." << endl;
      }
    }
  }

  if (m_DetectorIDs.size() == 0) {
    cout<<"No Strip3D detectors were found."<<endl;
    return false; 
  }

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleChargeTransport::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeTransport::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  // Dummy code:

  // Create strip hits
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    MVector Pos = SH.m_SimulatedPositionInDetector;

    // Determine detector and strip dimensions from the geometry
    unsigned int DetID = SH.m_ROE.GetDetectorID();
    double XWidth = m_XWidths[DetID];
    double YWidth = m_YWidths[DetID];
    double XPitch = m_XPitches[DetID];
    int NXStrips = m_NXStrips[DetID];

    // Calculate LV strip ID by rounding down intentionally to avoid truncation towards zero
    int ID = static_cast<int>(std::floor((Pos.X() + XWidth/2.0) / XPitch));

    // Check for strip ID and if the position is within the allowed strip length or on the guard ring
    // TODO: Confirm the correct boundary of the guard ring based on SMEX detector models
    if (ID >= 0 && ID < NXStrips && std::abs(Pos.Y()) <= YWidth/2.0 && std::hypot(Pos.X(), Pos.Y()) <= 4.712) {
      SH.m_ROE.SetStripID(ID);
      SH.m_IsGuardRing = false;
    } else {
      SH.m_ROE.SetStripID(NXStrips);
      SH.m_IsGuardRing = true;
    }
    SH.m_Energy = SH.m_SimulatedEnergy;
  }

  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    MVector Pos = SH.m_SimulatedPositionInDetector;

    // Determine detector and strip dimensions from the geometry
    unsigned int DetID = SH.m_ROE.GetDetectorID();
    double XWidth = m_XWidths[DetID];
    double YWidth = m_YWidths[DetID];
    double YPitch = m_YPitches[DetID];
    int NYStrips = m_NYStrips[DetID];

    // Calculate HV strip ID by rounding down intentionally to avoid truncation towards zero
    // TODO: Confirm the correct strip pitch based on SMEX detector models
    int ID = static_cast<int>(std::floor((Pos.Y() + YWidth/2.0) / YPitch));

    // Check for strip ID and if the position is within the allowed strip length or on the guard ring
    // TODO: Confirm the correct boundary of the guard ring based on SMEX detector models
    if (ID >= 0 && ID < NYStrips && std::abs(Pos.X()) <= XWidth/2.0 && std::hypot(Pos.X(), Pos.Y()) <= 4.712) {
      SH.m_ROE.SetStripID(ID);
      SH.m_IsGuardRing = false;
    } else {
      SH.m_ROE.SetStripID(NYStrips);
      SH.m_IsGuardRing = true;
    }
    SH.m_Energy = SH.m_SimulatedEnergy;
  }

  // Merge hits:
  for (auto IterLV1 = LVHits.begin(); IterLV1 != LVHits.end(); ++IterLV1) {
    auto IterLV2 = std::next(IterLV1);
    while (IterLV2 != LVHits.end()) {
      if (IterLV1->m_ROE == IterLV2->m_ROE) {
        IterLV1->m_Energy += IterLV2->m_Energy;
        IterLV2 = LVHits.erase(IterLV2);
      } else {
        ++IterLV2;
      }
    }
  }
  for (auto IterHV1 = HVHits.begin(); IterHV1 != HVHits.end(); ++IterHV1) {
    auto IterHV2 = std::next(IterHV1);
    while (IterHV2 != HVHits.end()) {
      if (IterHV1->m_ROE == IterHV2->m_ROE) {
        IterHV1->m_Energy += IterHV2->m_Energy;
        IterHV2 = HVHits.erase(IterHV2);
      } else {
        ++IterHV2;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleChargeTransport::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeTransport::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MSubModuleChargeTransport::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration

  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MSubModuleChargeTransport.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
