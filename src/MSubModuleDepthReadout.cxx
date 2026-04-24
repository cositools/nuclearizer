/*
 * MSubModuleDepthReadout.cxx
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
// MSubModuleDepthReadout
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleDepthReadout.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleDepthReadout)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleDepthReadout::MSubModuleDepthReadout() : MSubModule()
{
  // Construct an instance of MSubModuleDepthReadout

  m_Name = "DEE depth readout module";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleDepthReadout::~MSubModuleDepthReadout()
{
  // Delete this instance of MSubModuleDepthReadout
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDepthReadout::Initialize()
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
        } else {
          cout << "ERROR in MSubModuleDepthReadout::Initialize: Found a duplicate detector: " << det_name << endl;
        }
      } else {
        cout << "ERROR in MSubModuleDepthReadout::Initialize: Found a Strip3D detector with " << det->GetNSensitiveVolumes() << " Sensitive Volumes." << endl;
      }
    }
  }

  if (m_Thicknesses.size() == 0) {
    cout<<"No Strip3D detectors were found."<<endl;
    return false; 
  }

  m_DepthGrid.clear();
  m_CTDMap.clear();
  m_ElectronDriftTimes.clear();
  m_HoleDriftTimes.clear();

  if (LoadSplinesFile() == false) {
    return false;
  }

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDepthReadout::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDepthReadout::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  // Dummy code
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    int DetID = SH.m_ROE.GetDetectorID();
    double Z = SH.m_SimulatedPositionInDetector.Z();
    if (m_DepthGrid.count(DetID) > 0) {
      if (SH.m_IsGuardRing == false) {

        // Determine the hole drift times (accounting for electronics)
        // TODO: This only applies to main strips, find a better implementation for nearest neighbors
        vector<double> DepthGrid = m_DepthGrid[DetID];
        vector<double> HoleDriftTimes = m_HoleDriftTimes[DetID];
        TSpline3 HoleSpline = TSpline3("", &DepthGrid[0], &HoleDriftTimes[0], DepthGrid.size());
        // TODO: Apply stretch and offset
        double HoleDriftTime = HoleSpline.Eval(Z);

        // Convert drift time to timing by subtracting 4500ns (for now)
        // TODO: Improve determining the timing from drift times
        SH.m_Timing = 4500.0 - HoleDriftTime;
        // TODO: Apply the inverse TAC cal to obtain TAC in ADC units
        SH.m_TAC = SH.m_Timing;
        if (SH.m_TAC > 16383) SH.m_TAC = 16383;
      }
    } else {
      cout << "No Depth Spline for Event with DetID " << DetID << endl;
    }
  }
  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    int DetID = SH.m_ROE.GetDetectorID();
    double Z = SH.m_SimulatedPositionInDetector.Z();
    if (m_DepthGrid.count(DetID) > 0) {
      if (SH.m_IsGuardRing == false) {

        // Determine the electron drift times (accounting for electronics)
        // TODO: This only applies to main strips, find a better implementation for nearest neighbors
        vector<double> DepthGrid = m_DepthGrid[DetID];
        vector<double> ElectronDriftTimes = m_ElectronDriftTimes[DetID];
        TSpline3 ElectronSpline = TSpline3("", &DepthGrid[0], &ElectronDriftTimes[0], DepthGrid.size());
        // TODO: Apply stretch and offset
        double ElectronDriftTime = ElectronSpline.Eval(Z);

        // Convert drift time to timing by subtracting 4500ns (for now)
        // TODO: Improve determining the timing from drift times
        SH.m_Timing = 4500.0 - ElectronDriftTime;
        // TODO: Apply the inverse TAC cal to obtain TAC in ADC units
        SH.m_TAC = SH.m_Timing;
        if (SH.m_TAC > 16383) SH.m_TAC = 16383;
      }
    } else {
      cout << "No Depth Spline for Event with DetID " << DetID << endl;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////

bool MSubModuleDepthReadout::LoadSplinesFile()
{
  // Input spline files should have the following format:
  // ### DetID, HV, Temperature, Photopeak Energy
  // depth, ctd, electron_drift_time, hole_drift_time

  MFile SplineFile; 
  if (SplineFile.Open(m_DepthSplinesFile) == false) {
    cout << "ERROR in MSubModuleDepthReadout::LoadSplinesFile: failed to open depth splines file." << endl;
    return false;
  }

  // For now assume just one detector
  // TODO: Extend this logic to multiple detectors
  int DetID = 0;
  vector<double> DepthVector;
  vector<double> CTDVector;
  vector<double> ElectronDriftTimeVector;
  vector<double> HoleDriftTimeVector;
  
  m_DepthGrid[DetID] = DepthVector;
  m_CTDMap[DetID] = CTDVector;
  m_ElectronDriftTimes[DetID] = ElectronDriftTimeVector;
  m_HoleDriftTimes[DetID] = HoleDriftTimeVector;

  MString Line;
  while (SplineFile.ReadLine(Line) == true) {
    if (Line.Length() != 0) {
      if (Line.BeginsWith("#") == true) {
        // Get the Detector ID from the first commented line in that file
        // vector<MString> Tokens = Line.Tokenize(" ");
        // DetID = Tokens[1].ToInt();
        continue;

      } else {
        vector<MString> Tokens = Line.Tokenize(",");
        if (Tokens.size() >= 4) {
          m_DepthGrid[DetID].push_back(Tokens[0].ToDouble());
          m_CTDMap[DetID].push_back(Tokens[1].ToDouble());
          m_ElectronDriftTimes[DetID].push_back(Tokens[2].ToDouble());
          m_HoleDriftTimes[DetID].push_back(Tokens[3].ToDouble());
        } else {
          if (g_Verbosity >= c_Error) {
            cout << "ERROR in MSubModuleDepthReadout::LoadSplinesFile: Empty line in depth splines file." << endl;
          }
          return false;
        }
      }
    }
  }

  SplineFile.Close();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDepthReadout::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize()
  m_Thicknesses.clear();

  m_DepthGrid.clear();
  m_CTDMap.clear();
  m_ElectronDriftTimes.clear();
  m_HoleDriftTimes.clear();

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDepthReadout::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  MXmlNode* N = Node->GetNode("DepthSplineFileName");
  if (N != nullptr) {
    m_DepthSplinesFile = N->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleDepthReadout::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  new MXmlNode(Node, "DepthSplineFileName", m_DepthSplinesFile);

  return Node;
}


// MSubModuleDepthReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
