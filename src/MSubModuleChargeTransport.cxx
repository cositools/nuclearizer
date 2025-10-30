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
#include <cmath>

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MSubModule.h"
#include "MDShapeIntersection.h"
#include "MDShapeTUBS.h"


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

          // Read the detector radius from the geometry (assuming it is the second shape of an intersection)
          if (vol->GetShape()->GetType() == "Intersection" && dynamic_cast<MDShapeIntersection*>(vol->GetShape())->GetShapeB()->GetType() == "TUBS") {
            m_Radii[DetID] = dynamic_cast<MDShapeTUBS*>(dynamic_cast<MDShapeIntersection*>(vol->GetShape())->GetShapeB())->GetRmax();
          } else {
            // If that does not exist, set the detector radius to a value high enough to not have an impact
            m_Radii[DetID] = m_XWidths[DetID] + m_YWidths[DetID];
            if (g_Verbosity >= c_Info) {
              cout << m_Name << ": No bounding tube volume found for this detector" << endl;
            }
          }
          
          if (g_Verbosity >= c_Info) {
            cout << "Found detector " << det_name << " corresponding to DetID=" << DetID << "." << endl;
            cout << "Detector width (X): " << m_XWidths[DetID] << endl;
            cout << "Detector width (Y): " << m_YWidths[DetID] << endl;
            cout << "Detector radius (R): " << m_Radii[DetID] << endl;
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

  // Define physical constants
  constexpr double kB = TMath::K(); // unit: J/K
  constexpr double ElementaryCharge = TMath::Qe(); // unit: C
  constexpr double IonizationEnergy = 0.00295; // unit: keV
  constexpr double Epsilon0 = 8.85418781762039e-14; // unit: F/cm
  constexpr double EpsilonR = 16.0; // in germanium, unitless

  // TODO: Read bias voltage and temperature of the detector from a file
  constexpr double BiasVoltage = 600.0; // unit: V
  constexpr double Temperature = 87.0; // unit: K

  // TODO: Implement energy-dependent initial charge-cloud sizes
  constexpr double InitialChargeCloudSize = 0.02; // 200µm in cm

  // Create strip hits
  list<MDEEStripHit> ChargeTransportLVHits;

  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    MVector Pos = SH.m_SimulatedPositionInDetector;

    // Determine detector and strip dimensions from the geometry
    unsigned int DetID = SH.m_ROE.GetDetectorID();
    double XWidth = m_XWidths[DetID];
    double YWidth = m_YWidths[DetID];
    double Thickness = m_Thicknesses[DetID];
    double XPitch = m_XPitches[DetID];
    double Radius = m_Radii[DetID];
    int NXStrips = m_NXStrips[DetID];

    double MeanElectricField = BiasVoltage / Thickness; // unit: V/cm
    double N = SH.m_SimulatedEnergy / IonizationEnergy;

    // Calculate LV strip ID by rounding down intentionally to avoid truncation towards zero
    int ID = static_cast<int>(std::floor((Pos.X() + XWidth/2.0) / XPitch));

    // Check for strip ID and if the position is within the allowed strip length or on the guard ring
    // TODO: Confirm the correct boundary of the guard ring based on SMEX detector models
    if (ID >= 0 && ID < NXStrips && std::abs(Pos.Y()) <= YWidth/2.0 && std::hypot(Pos.X(), Pos.Y()) <= Radius) {

      // Apply charge sharing based on relative coordinate to the gap of that strip (0 <= X < XPitch)
      double XFromGap = std::fmod(Pos.X() + XWidth/2.0, XPitch);

      // calculate σ and µ, assuming t = z / v = z / (µ * E)
      double SigmaX = std::sqrt(2.0 * kB * Temperature * (Pos.Z() + Thickness / 2.0) / (ElementaryCharge * MeanElectricField)); // in cm
      double EtaX   = std::cbrt(std::pow(InitialChargeCloudSize, 3) + 3.0 * N * ElementaryCharge * (Pos.Z() + Thickness / 2.0) / (4.0 * TMath::Pi() * Epsilon0 * EpsilonR * MeanElectricField)); // in cm
      cout << "X: " << Pos.X() << ", Y: " << Pos.Y() << ", Z: " << Pos.Z() << ", E: " << SH.m_SimulatedEnergy << endl;
      cout << "Sigma X: " << SigmaX << ", EtaX: " << EtaX << endl;

      auto Lambda = [&](double x) -> double {
        double a = (x - EtaX) / (TMath::Sqrt2() * SigmaX);
        double b = (x + EtaX) / (TMath::Sqrt2() * SigmaX);
        return SH.m_SimulatedEnergy / (8.0 * std::pow(EtaX, 3)) * (
          std::erf(b) * (2.0 * std::pow(EtaX, 3) + x * (3.0 * std::pow(EtaX, 2) - 3.0 * std::pow(SigmaX, 2) - std::pow(x, 2))) + 
          std::erf(a) * (2.0 * std::pow(EtaX, 3) - x * (3.0 * std::pow(EtaX, 2) - 3.0 * std::pow(SigmaX, 2) - std::pow(x, 2))) + 
          std::exp(- std::pow(b,2)) * std::sqrt(2.0 / TMath::Pi()) * SigmaX * (EtaX * x + (2.0 * std::pow(EtaX, 2) - 2.0 * std::pow(SigmaX, 2) - std::pow(x, 2))) + 
          std::exp(- std::pow(a,2)) * std::sqrt(2.0 / TMath::Pi()) * SigmaX * (EtaX * x - (2.0 * std::pow(EtaX, 2) - 2.0 * std::pow(SigmaX, 2) - std::pow(x, 2))) 
        );
      };

      double MainStripEnergy    = Lambda(XPitch - XFromGap) - Lambda(-XFromGap);
      double NNLeftStripEnergy  = Lambda(-XFromGap) - Lambda(-XPitch - XFromGap);
      double NNRightStripEnergy = Lambda(2.0*XPitch - XFromGap) - Lambda(XPitch - XFromGap);

      cout << "Energy: " << SH.m_SimulatedEnergy << ", split into " << NNLeftStripEnergy << ", " << MainStripEnergy << " and " << NNRightStripEnergy << endl;

      // create entry for the main hit
      MDEEStripHit MainSH = SH;
      MainSH.m_ROE.SetStripID(ID);
      MainSH.m_Energy = MainStripEnergy;
      MainSH.m_IsGuardRing = false;
      ChargeTransportLVHits.push_back(MainSH);

      // create MDEEStripHit for the left NN
      if (NNLeftStripEnergy > IonizationEnergy) {
        MDEEStripHit NNLeftSH = SH;
        NNLeftSH.m_Energy = NNLeftStripEnergy;
        if (ID > 0) {
          NNLeftSH.m_ROE.SetStripID(ID - 1);
          NNLeftSH.m_IsGuardRing = false;
          // NNLeftSH.m_IsNearestNeighbor = true;
        } else {
          NNLeftSH.m_ROE.SetStripID(NXStrips);
          NNLeftSH.m_IsGuardRing = true;
        }
        ChargeTransportLVHits.push_back(NNLeftSH);
      }
      
      // create MDEEStripHit for the right NN
      if (NNRightStripEnergy > IonizationEnergy) {
        MDEEStripHit NNRightSH = SH;
        NNRightSH.m_Energy = NNRightStripEnergy;
        if (ID < NXStrips - 1) {
          NNRightSH.m_ROE.SetStripID(ID + 1);
          NNRightSH.m_IsGuardRing = false;
          // NNRightSH.m_IsNearestNeighbor = true;
        } else {
          NNRightSH.m_ROE.SetStripID(NXStrips);
          NNRightSH.m_IsGuardRing = true;
        }
        ChargeTransportLVHits.push_back(NNRightSH);
      }

    } else {
      // TODO: implement charge sharing also for GR events
      SH.m_Energy = SH.m_SimulatedEnergy;
      SH.m_ROE.SetStripID(NXStrips);
      SH.m_IsGuardRing = true;
      ChargeTransportLVHits.push_back(SH);
    }
  }

  // replace old list by new list
  Event->GetDEEStripHitLVListReference().clear();
  for (MDEEStripHit& SH: ChargeTransportLVHits) {
    Event->AddDEEStripHitLV(SH);
  }

  list<MDEEStripHit> ChargeTransportHVHits;

  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    MVector Pos = SH.m_SimulatedPositionInDetector;

    // Determine detector and strip dimensions from the geometry
    unsigned int DetID = SH.m_ROE.GetDetectorID();
    double XWidth = m_XWidths[DetID];
    double YWidth = m_YWidths[DetID];
    double Thickness = m_Thicknesses[DetID];
    double YPitch = m_YPitches[DetID];
    double Radius = m_Radii[DetID];
    int NYStrips = m_NYStrips[DetID];

    double MeanElectricField = BiasVoltage / Thickness; // unit: V/cm
    double N = SH.m_SimulatedEnergy / IonizationEnergy;

    // Calculate HV strip ID by rounding down intentionally to avoid truncation towards zero
    // TODO: Confirm the correct strip pitch based on SMEX detector models
    int ID = static_cast<int>(std::floor((Pos.Y() + YWidth/2.0) / YPitch));

    // Check for strip ID and if the position is within the allowed strip length or on the guard ring
    // TODO: Confirm the correct boundary of the guard ring based on SMEX detector models
    if (ID >= 0 && ID < NYStrips && std::abs(Pos.X()) <= XWidth/2.0 && std::hypot(Pos.X(), Pos.Y()) <= Radius) {

      // Apply charge sharing based on relative coordinate to the gap of that strip (0 <= Y < YPitch)
      double YFromGap = std::fmod(Pos.Y() + YWidth/2.0, YPitch);

      // calculate σ and µ, assuming t = z / v = z / (µ * E)
      double SigmaY = std::sqrt(2.0 * kB * Temperature * (Thickness / 2.0 - Pos.Z()) / (ElementaryCharge * MeanElectricField)); // in cm
      double EtaY   = std::cbrt(std::pow(InitialChargeCloudSize, 3) + 3.0 * N * ElementaryCharge * (Thickness / 2.0 - Pos.Z()) / (4.0 * TMath::Pi() * Epsilon0 * EpsilonR * MeanElectricField)); // in cm

      auto Lambda = [&](double y) -> double {
        double a = (y - EtaY) / (TMath::Sqrt2() * SigmaY);
        double b = (y + EtaY) / (TMath::Sqrt2() * SigmaY);
        return SH.m_SimulatedEnergy / (8.0 * std::pow(EtaY, 3)) * (
          std::erf(b) * (2.0 * std::pow(EtaY, 3) + y * (3.0 * std::pow(EtaY, 2) - 3.0 * std::pow(SigmaY, 2) - std::pow(y, 2))) + 
          std::erf(a) * (2.0 * std::pow(EtaY, 3) - y * (3.0 * std::pow(EtaY, 2) - 3.0 * std::pow(SigmaY, 2) - std::pow(y, 2))) + 
          std::exp(- std::pow(b,2)) * std::sqrt(2 / TMath::Pi()) * SigmaY * (EtaY * y + (2.0 * std::pow(EtaY, 2) - 2.0 * std::pow(SigmaY, 2) - std::pow(y, 2))) + 
          std::exp(- std::pow(a,2)) * std::sqrt(2 / TMath::Pi()) * SigmaY * (EtaY * y - (2.0 * std::pow(EtaY, 2) - 2.0 * std::pow(SigmaY, 2) - std::pow(y, 2))) 
        );
      };

      double MainStripEnergy    = Lambda(YPitch - YFromGap) - Lambda(-YFromGap);
      double NNLeftStripEnergy  = Lambda(-YFromGap) - Lambda(-YPitch - YFromGap);
      double NNRightStripEnergy = Lambda(2.0*YPitch - YFromGap) - Lambda(YPitch - YFromGap);

      // create entry for the main hit
      MDEEStripHit MainSH = SH;
      MainSH.m_ROE.SetStripID(ID);
      MainSH.m_Energy = MainStripEnergy;
      MainSH.m_IsGuardRing = false;
      ChargeTransportHVHits.push_back(MainSH);

      // create MDEEStripHit for the left NN
      if (NNLeftStripEnergy > IonizationEnergy) {
        MDEEStripHit NNLeftSH = SH;
        NNLeftSH.m_Energy = NNLeftStripEnergy;
        if (ID > 0) {
          NNLeftSH.m_ROE.SetStripID(ID - 1);
          NNLeftSH.m_IsGuardRing = false;
          // NNLeftSH.m_IsNearestNeighbor = true;
        } else {
          NNLeftSH.m_ROE.SetStripID(NYStrips);
          NNLeftSH.m_IsGuardRing = true;
        }
        ChargeTransportHVHits.push_back(NNLeftSH);
      }
        
      // create MDEEStripHit for the right NN
      if (NNRightStripEnergy > IonizationEnergy) {
        MDEEStripHit NNRightSH = SH;
        NNRightSH.m_Energy = NNRightStripEnergy;
        if (ID < NYStrips - 1) {
          NNRightSH.m_ROE.SetStripID(ID + 1);
          NNRightSH.m_IsGuardRing = false;
          // NNRightSH.m_IsNearestNeighbor = true;
        } else {
          NNRightSH.m_ROE.SetStripID(NYStrips);
          NNRightSH.m_IsGuardRing = true;
        }
        ChargeTransportHVHits.push_back(NNRightSH);
      }

    } else {
      // TODO: implement charge sharing also for GR events
      SH.m_Energy = SH.m_SimulatedEnergy;
      SH.m_ROE.SetStripID(NYStrips);
      SH.m_IsGuardRing = true;
      ChargeTransportHVHits.push_back(SH);
    }
  }

  // replace old list by new list
  Event->GetDEEStripHitHVListReference().clear();
  for (MDEEStripHit& SH: ChargeTransportHVHits) {
    Event->AddDEEStripHitHV(SH);
  }

  // Merge hits:
  // TODO: how to deal with flags like "m_IsNearestNeighbor" etc. ?
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
