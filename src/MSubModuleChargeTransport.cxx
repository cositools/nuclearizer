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

// Nuclearizer libs:
#include "MModuleDepthCalibration.h"


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

  // Look through the Geometry and get the names and thicknesses of all Strip3D detectors.
  vector<string> DetectorNames;
  unsigned int DetID = 0;
  
  for(unsigned int i = 0; i < DetList.size(); ++i){
    MDDetector* det = DetList[i];
    if (det->GetTypeName() == "Strip3D") {
      if (det->GetNSensitiveVolumes() == 1) {
        MDVolume* vol = det->GetSensitiveVolume(0);
        MString DetectorName = det->GetName();
        string DetName = DetectorName.GetString();
        
        // Check that the DetID agrees with the naming scheme GeD_X
        if (DetectorName.BeginsWith("GeD_") == true) {
          DetectorName.RemoveAllInPlace("GeD_"); // The number after GeD is the COSI detector ID
          if (DetID != DetectorName.ToUnsignedInt()) {
            if (g_Verbosity >= c_Error) {
              cout << "ERROR in MModuleDepthCalibration::Initialize: Non-matching DetID="<<DetID<<" for detector "<<DetName<<endl;
            }
            // Return false if this is running with the COSI SMEX payload mass model
            if (m_Geometry->GetName() == "COSI-SMEX-Payload"){
              return false;
            }
          }
        } else if (m_Geometry->GetName() == "COSI-SMEX-Payload") {
          if (g_Verbosity >= c_Error) {
            cout << "ERROR in MModuleDepthCalibration::Initialize: COSI-SMEX-Payload expects all Strip3D detectors to follow the name scheme GeD_X"<<endl;
          }
          return false;
        }

        if (find(DetectorNames.begin(), DetectorNames.end(), DetName) == DetectorNames.end()) {
          DetectorNames.push_back(DetName);
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
            cout << "Found detector " << DetName << " corresponding to DetID=" << DetID << "." << endl;
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
          DetID += 1;
        } else {
          cout << "ERROR in MSubModuleChargeTransport::Initialize: Found a duplicate detector: " << DetName << endl;
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

  m_Coeffs.clear();
  m_DepthGrid.clear();
  m_ElectronDriftTimes.clear();
  m_HoleDriftTimes.clear();

  // Load depth-related files using the parsers in MModuleDepthCalibration
  MModuleDepthCalibration DepthCalibration;
  DepthCalibration.SetUCSDOverride(false);

  // Determine the thicknesses of the individual detectors from the geometry
  if (DepthCalibration.LoadDetectorDimensions(m_Geometry) == false) {
    return false;
  }

  // Load CTD-to-depth splines
  DepthCalibration.SetSplinesFileName(m_DepthSplinesFileName);
  if (DepthCalibration.LoadSplinesFile(m_DepthSplinesFileName) == true) {

    // There should be at least three more columns with 1. the CTDmap, 2. the electron drift times and 3. the hole drift times
    unordered_map<int, vector<vector<double>>> Columns = DepthCalibration.GetCTDMap();

    // Copy the depth grid, CTD splines and the charge carrier drift times
    m_DepthGrid = DepthCalibration.GetDepthGrid();
    for (auto const& [DetID, Column] : Columns) {
      if (Column.size() < 3) {
        if (g_Verbosity >= c_Error) {
          cout<<"ERROR in MSubModuleChargeTransport::Initialize: Expected (at least) 4 columns for detector "<<DetID<<" in "<<m_DepthSplinesFileName<<endl;
        }
        return false;
      }
      m_ElectronDriftTimes[DetID] = Column[1];
      m_HoleDriftTimes[DetID] = Column[2];

      // Build and store the splines here
      m_ElectronDriftSplines[DetID] = new TSpline3("", &m_DepthGrid[DetID][0], &m_ElectronDriftTimes[DetID][0], m_DepthGrid[DetID].size());
      m_HoleDriftSplines[DetID] = new TSpline3("", &m_DepthGrid[DetID][0], &m_HoleDriftTimes[DetID][0], m_DepthGrid[DetID].size());
    }

  } else {
    return false;
  }

  // Load depth calibration coefficients
  DepthCalibration.SetCoeffsFileName(m_DepthCoefficientsFileName);
  if (DepthCalibration.LoadCoeffsFile(m_DepthCoefficientsFileName) == true) {
    // Copy depth calibration coefficients
    m_Coeffs = DepthCalibration.GetCoeffs();

  } else {
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

  m_ChargeTransportHits.clear();

  // Create strip hits
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    RunChargeTransportForHit(SH, true);
  }
    
  // replace old list by new list
  Event->GetDEEStripHitLVListReference().clear();
  for (MDEEStripHit& SH: m_ChargeTransportHits) {
    Event->AddDEEStripHitLV(SH);
  }

  // empty list
  m_ChargeTransportHits.clear();

  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    RunChargeTransportForHit(SH, false);
  }

  // replace old list by new list
  Event->GetDEEStripHitHVListReference().clear();
  for (MDEEStripHit& SH: m_ChargeTransportHits) {
    Event->AddDEEStripHitHV(SH);
  }

  m_ChargeTransportHits.clear();

  // Merge hits:
  // TODO: how to deal with flags like "m_IsNearestNeighbor" etc. ?
  // TODO: how to properly combine m_FastPeakTime, maybe merge strip hits only in MSubModuleDepthReadout ?
  for (auto IterLV1 = LVHits.begin(); IterLV1 != LVHits.end(); ++IterLV1) {
    auto IterLV2 = std::next(IterLV1);
    while (IterLV2 != LVHits.end()) {
      if (IterLV1->m_ROE == IterLV2->m_ROE) {
        IterLV1->m_Energy += IterLV2->m_Energy;
        IterLV1->m_FastPeakTime = IterLV1->m_Energy > IterLV2->m_Energy ? IterLV1->m_FastPeakTime : IterLV2->m_FastPeakTime;
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
        IterHV1->m_FastPeakTime = IterHV1->m_Energy > IterHV2->m_Energy ? IterHV1->m_FastPeakTime : IterHV2->m_FastPeakTime;
        IterHV2 = HVHits.erase(IterHV2);
      } else {
        ++IterHV2;
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////


void MSubModuleChargeTransport::RunChargeTransportForHit(MDEEStripHit& SH, bool isLV) {

  // Define physical constants
  constexpr double kB = TMath::K(); // unit: J/K
  constexpr double ElementaryCharge = TMath::Qe(); // unit: C
  constexpr double IonizationEnergy = 0.00295; // unit: keV
  constexpr double Epsilon0 = 8.85418781762039e-14; // unit: F/cm
  constexpr double EpsilonR = 16.0; // in germanium, unitless

  // TODO: Read bias voltage and temperature of the detector from a database
  constexpr double BiasVoltage = 1050.0; // unit: V
  constexpr double Temperature = 87.0; // unit: K

  double N = SH.m_SimulatedEnergy / IonizationEnergy;

  // TODO: Implement energy-dependent initial charge-cloud sizes
  constexpr double InitialChargeCloudSize = 0.; // zero for now, could be set to the default cut range ?



  // This function uses strip coordinates (P and Q) instead of X and Y
  // (P = perpendicular to strip length, Q = along strip length):
  //   ╔═════════════════════════════════════════════════╗ ↑
  // P ║                  STRIP CONTACT                  ║ │ Pitch
  // ↑ ╚═════════════════════════════════════════════════╝ ↓
  // └→ Q 
  // On the LV side: P = X, Q = Y
  // On the HV side: P = Y, Q = X

  // Get detector and strip dimensions
  unsigned int DetID = SH.m_ROE.GetDetectorID();
  double Thickness   = m_Thicknesses[DetID];
  double Radius      = m_Radii[DetID];
  double PWidth      = isLV ? m_XWidths[DetID] : m_YWidths[DetID];
  double QWidth      = isLV ? m_YWidths[DetID] : m_XWidths[DetID];
  double PPitch      = isLV ? m_XPitches[DetID] : m_YPitches[DetID];
  double QPitch      = isLV ? m_YPitches[DetID] : m_XPitches[DetID];
  int NStrips        = isLV ? m_NXStrips[DetID] : m_NYStrips[DetID];

  // Express coordinates of the hit in local strip coordinates
  MVector Pos = SH.m_SimulatedPositionInDetector;
  double P    = isLV ? Pos.X() : Pos.Y();
  double Q    = isLV ? Pos.Y() : Pos.X();
  double Z    = Pos.Z();
  double DeltaZ = isLV ? Z + Thickness / 2.0 : Thickness / 2.0 - Z;

  double MeanElectricField = BiasVoltage / Thickness; // unit: V/cm

  // Calculate strip ID by rounding down intentionally to avoid truncation towards zero
  // TODO: Include mask metrology information when calculating the strip ID from the position.
  int ID = static_cast<int>(std::floor((P + PWidth/2.0) / PPitch));

  // Calculate the strip ID for the opposite side of the detector (and explicitly check for guard ring)
  int OppositeStripID = static_cast<int>(std::floor((Q + QWidth/2.0) / QPitch));
  if (std::abs(P) > PWidth/2.0 && std::abs(Q) > QWidth/2.0 && std::hypot(P, Q) > Radius) {
    OppositeStripID = NStrips;
  }

  // Check for strip ID and if the position is within the allowed strip length or on the guard ring
  // TODO: Confirm the correct boundary of the guard ring based on SMEX detector models
  if (ID >= 0 && ID < NStrips && std::abs(Q) <= QWidth/2.0 && std::hypot(P, Q) <= Radius) {

    // Determine the charge drift times in nanoseconds from simulations + stretch/offset from the depth calibration
    // Set the default to a large number (here: 1e10 ns) in case no depth calibration coefficients exist
    double FastPeakTime = 1e10;

    TSpline3* DriftTimeSpline = isLV ? m_HoleDriftSplines[DetID] : m_ElectronDriftSplines[DetID];
    int PixelCode = 10000*DetID + 100*(isLV ? ID : OppositeStripID) + (isLV ? OppositeStripID : ID);

    // Apply stretch based on Eq. (3) in https://doi.org/10.1016/j.nima.2026.171332
    // Apply no offset to the electron drift time --> add it fully to the hole (LV) signal
    auto it = m_Coeffs.find(PixelCode);
    if (it != m_Coeffs.end()) {
      const vector<double>& Coeffs = it->second;
      double Stretch = Coeffs[0];
      double Offset = isLV ? Coeffs[1] : 0.0;
      FastPeakTime = (DriftTimeSpline->Eval(Z) + Offset) * Stretch;
    } else {
      if (g_Verbosity >= c_Warning) {
        cout << "No depth calibration coefficients for pixel in DetID " << DetID << " HV " << (isLV ? OppositeStripID : ID) << " LV " << (isLV ? ID : OppositeStripID) << endl;
      }
    }

    // Apply charge sharing based on relative coordinate to the gap of that strip (0 <= X < XPitch)
    double FromGap = std::fmod(P + PWidth/2.0, PPitch);

    // Charge transport based on Eq. (7) in https://doi.org/10.1016/j.nima.2023.168310
    // calculate σ and η, assuming t = z / v = z / (µ * E)
    // TODO: Reevaluate whether we want to use t = z / v = z / (µ * E), or use the simulated drift times here instead
    double Sigma = std::sqrt(2.0 * kB * Temperature * DeltaZ / (ElementaryCharge * MeanElectricField)); // in cm
    double Eta   = std::cbrt(std::pow(InitialChargeCloudSize, 3) + 3.0 * N * ElementaryCharge * DeltaZ / (4.0 * TMath::Pi() * Epsilon0 * EpsilonR * MeanElectricField)); // in cm
    auto Lambda = [&](double x) -> double { 
      double a = (x - Eta) / (TMath::Sqrt2() * Sigma);
      double b = (x + Eta) / (TMath::Sqrt2() * Sigma);
      return SH.m_SimulatedEnergy / (8.0 * std::pow(Eta, 3)) * (
        std::erf(b) * (2.0 * std::pow(Eta, 3) + x * (3.0 * std::pow(Eta, 2) - 3.0 * std::pow(Sigma, 2) - std::pow(x, 2))) + 
        std::erf(a) * (2.0 * std::pow(Eta, 3) - x * (3.0 * std::pow(Eta, 2) - 3.0 * std::pow(Sigma, 2) - std::pow(x, 2))) + 
        std::exp(- std::pow(b,2)) * std::sqrt(2 / TMath::Pi()) * Sigma * (Eta * x + (2.0 * std::pow(Eta, 2) - 2.0 * std::pow(Sigma, 2) - std::pow(x, 2))) + 
        std::exp(- std::pow(a,2)) * std::sqrt(2 / TMath::Pi()) * Sigma * (Eta * x - (2.0 * std::pow(Eta, 2) - 2.0 * std::pow(Sigma, 2) - std::pow(x, 2))) 
      );
    };

    double MainStripEnergy    = Lambda(PPitch - FromGap) - Lambda(-FromGap);
    double NNLeftStripEnergy  = Lambda(-FromGap) - Lambda(-PPitch - FromGap);
    double NNRightStripEnergy = Lambda(2.0*PPitch - FromGap) - Lambda(PPitch - FromGap);

    // create entry for the main hit
    MDEEStripHit MainSH = SH;
    MainSH.m_ROE.SetStripID(ID);
    MainSH.m_OppositeStripID = OppositeStripID;
    MainSH.m_Energy = MainStripEnergy;
    // TODO: Implement a more realistic parameterization to determine nearest-neighbor timing values
    MainSH.m_FastPeakTime = FastPeakTime - 50 * (1 - MainStripEnergy / SH.m_SimulatedEnergy);
    MainSH.m_IsGuardRing = false;
    m_ChargeTransportHits.push_back(MainSH);

    // create MDEEStripHit for the left NN
    MDEEStripHit NNLeftSH = SH;
    NNLeftSH.m_Energy = std::max(NNLeftStripEnergy, 0.0);
    NNLeftSH.m_FastPeakTime = FastPeakTime - 50 * (1 - NNLeftStripEnergy / SH.m_SimulatedEnergy);
    NNLeftSH.m_OppositeStripID = OppositeStripID;
    if (ID > 0) {
      NNLeftSH.m_ROE.SetStripID(ID - 1);
      NNLeftSH.m_IsGuardRing = false;
    } else {
      NNLeftSH.m_ROE.SetStripID(NStrips);
      NNLeftSH.m_IsGuardRing = true;
    }
    m_ChargeTransportHits.push_back(NNLeftSH);
    
    // create MDEEStripHit for the right NN
    MDEEStripHit NNRightSH = SH;
    NNRightSH.m_Energy = std::max(NNRightStripEnergy, 0.0);
    NNRightSH.m_FastPeakTime = FastPeakTime - 50 * (1 - NNRightStripEnergy / SH.m_SimulatedEnergy);
    NNRightSH.m_OppositeStripID = OppositeStripID;
    if (ID < NStrips - 1) {
      NNRightSH.m_ROE.SetStripID(ID + 1);
      NNRightSH.m_IsGuardRing = false;
    } else {
      NNRightSH.m_ROE.SetStripID(NStrips);
      NNRightSH.m_IsGuardRing = true;
    }
    m_ChargeTransportHits.push_back(NNRightSH);

  } else {
    // TODO: implement charge sharing also for GR events
    SH.m_Energy = SH.m_SimulatedEnergy;
    SH.m_ROE.SetStripID(NStrips);
    SH.m_IsGuardRing = true;
    m_ChargeTransportHits.push_back(SH);
  }
}
  

////////////////////////////////////////////////////////////////////////////////


void MSubModuleChargeTransport::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  m_Coeffs.clear();
  m_DepthGrid.clear();
  m_ElectronDriftTimes.clear();
  m_HoleDriftTimes.clear();

  // Delete the allocated spline objects
  for (auto& [DetID, spline] : m_ElectronDriftSplines) {
    delete spline;
  }
  for (auto& [DetID, spline] : m_HoleDriftSplines) {
    delete spline;
  }
  m_ElectronDriftSplines.clear();
  m_HoleDriftSplines.clear();

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeTransport::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleChargeTransport::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  return Node;
}

// MSubModuleChargeTransport.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
