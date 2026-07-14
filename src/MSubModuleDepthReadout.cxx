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
#include "TRandom.h"
#include "TMath.h"

// MEGAlib libs:
#include "MModuleDepthCalibration.h"
#include "MModuleTACcut.h"


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

  m_DepthGrid.clear();
  m_CTDMap.clear();
  m_ElectronDriftTimes.clear();
  m_HoleDriftTimes.clear();
  m_Coeffs.clear();

  // Load depth-related files using the parsers in MModuleDepthCalibration
  MModuleDepthCalibration DepthCalibration;

  // Determine the thicknesses of the individual detectors from the geometry
  DepthCalibration.SetUCSDOverride(false);
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
          cout<<"ERROR in MSubModuleDepthReadout::Initialize: Expected (at least) 4 columns for detector "<<DetID<<" in "<<m_DepthSplinesFileName<<endl;
        }
        return false;
      }
      m_CTDMap[DetID] = Column[0];
      m_ElectronDriftTimes[DetID] = Column[1];
      m_HoleDriftTimes[DetID] = Column[2];
    }

  } else {
    return false;
  }

  // Load depth calibration coefficients
  DepthCalibration.SetCoeffsFileName(m_DepthCoefficientsFileName);
  if (DepthCalibration.LoadCoeffsFile(m_DepthCoefficientsFileName) == true) {
    // Copy depth calibration coefficients
    m_Coeffs = DepthCalibration.GetCoeffs();
    m_Coeffs_Energy = DepthCalibration.GetCoeffsEnergy();

    // The reference energy for the timing noise should be in the file header of the depth coefficients file
    // Throw a warning if it was not retrieved and m_Coeffs_Energy is still at its default value of 0
    if (m_ApplyTimingResolutionCalibration == true && m_Coeffs_Energy == 0) {
      if (g_Verbosity >= c_Warning) {
        cout << "Timing values will not be smeared as no reference energy found in depth spline file "<<m_DepthCoefficientsFileName<<endl;
      }
    }
  } else {
    return false;
  }

  // Load TAC calibration parameters
  MModuleTACcut TACcut;
  TACcut.SetTACCalFileName(m_TACCalFileName);
  if (TACcut.LoadTACCalFile(m_TACCalFileName) == true) {
    // Copy TAC cal parameters
    m_TACCal = TACcut.GetTACCalParameters();
  } else {
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

  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    int DetID = SH.m_ROE.GetDetectorID();
    double Z = SH.m_SimulatedPositionInDetector.Z();
    if (m_DepthGrid.count(DetID) == 1) {
      if (SH.m_IsGuardRing == false) {

        // Determine the hole drift times (accounting for electronics)
        // TODO: This only applies to main strips, find a better implementation for nearest neighbors
        vector<double> DepthGrid = m_DepthGrid[DetID];
        vector<double> HoleDriftTimes = m_HoleDriftTimes[DetID];
        TSpline3 HoleSpline = TSpline3("", &DepthGrid[0], &HoleDriftTimes[0], DepthGrid.size());

        // Apply stretch and offset based on Eq. (3) in https://doi.org/10.1016/j.nima.2026.171332
        unsigned int StripID = SH.m_ROE.GetStripID();
        int PixelCode = 10000*DetID + 100*StripID + SH.m_OppositeStripID;
        if (m_Coeffs.count(PixelCode) == 1){
          vector<double> Coeffs = m_Coeffs[PixelCode];
          double Stretch = Coeffs[0];
          double Offset = Coeffs[1];
          double CTD_FWHM = Coeffs[2] * m_Coeffs_Energy / SH.m_Energy;
          double CTD_Sigma = CTD_FWHM / 2.355;
          double HoleDriftTime = (HoleSpline.Eval(Z) + Offset) * Stretch;

          // Convert drift time to timing by subtracting 4200ns (for now)
          // TODO: Improve determining the timing from drift times
          SH.m_Timing = 4200.0 - HoleDriftTime;

          // Smear the timing value based on the given CTD resolution
          // --> divide by √2 to obtain TAC resolution from CTD resolution
          if (m_ApplyTimingResolutionCalibration == true) {
            SH.m_Timing = gRandom->Gaus(SH.m_Timing, CTD_Sigma / TMath::Sqrt(2.0));
          }

          // Apply the inverse TAC cal to obtain TAC in ADC units
          if (m_TACCal.count(DetID) == 1 && m_TACCal[DetID].size() >= 1 && StripID < m_TACCal[DetID][0].size()) {
            vector<double> TACCal = m_TACCal[DetID][0][StripID];
            double TACCalSlope = TACCal[0];
            double TACCalOffset = TACCal[1];
            if ((SH.m_Timing - TACCalOffset) / TACCalSlope >= 0) {
              SH.m_TAC = (SH.m_Timing - TACCalOffset) / TACCalSlope;
            } else {
              if (g_Verbosity >= c_Info) {
                cout<<"MSubModuleDepthReadout::AnalyzeEvent: Simulated TAC value would be negative, setting it to zero."<<endl;
              }
              SH.m_TAC = 0;
            }
          } else {
            if (g_Verbosity >= c_Error) {
              cout<<"ERROR in MSubModuleDepthReadout::AnalyzeEvent: No TAC calibration found for LV strip "<<StripID<<endl;
            }
            SH.m_TAC = 0;
          }
          if (SH.m_TAC > 16383) {
            SH.m_TAC = 16383;
          }
        } else {
          if (g_Verbosity >= c_Info) {
            cout<<"MSubModuleDepthReadout::AnalyzeEvent: No depth coefficient found for pixel with code "<<PixelCode<<"."<<endl;
          }
          SH.m_TAC = 0;
          SH.m_HasTriggered = false;
        }
      }
    } else {
      if (g_Verbosity >= c_Warning) {
        cout << "No Depth Spline for Event with DetID " << DetID << endl;
      }
    }
  }
  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    int DetID = SH.m_ROE.GetDetectorID();
    double Z = SH.m_SimulatedPositionInDetector.Z();
    if (m_DepthGrid.count(DetID) == 1) {
      if (SH.m_IsGuardRing == false) {

        // Determine the electron drift times (accounting for electronics)
        // TODO: This only applies to main strips, find a better implementation for nearest neighbors
        vector<double> DepthGrid = m_DepthGrid[DetID];
        vector<double> ElectronDriftTimes = m_ElectronDriftTimes[DetID];
        TSpline3 ElectronSpline = TSpline3("", &DepthGrid[0], &ElectronDriftTimes[0], DepthGrid.size());

        // Apply stretch based on Eq. (3) in https://doi.org/10.1016/j.nima.2026.171332
        // Apply no offset to the electron drift time --> add it fully to the hole signal
        unsigned int StripID = SH.m_ROE.GetStripID();
        int PixelCode = 10000*DetID + 100*SH.m_OppositeStripID + StripID;
        if (m_Coeffs.count(PixelCode) == 1){

          vector<double> Coeffs = m_Coeffs[PixelCode];
          double Stretch = Coeffs[0];
          double CTD_FWHM = Coeffs[2] * m_Coeffs_Energy / SH.m_Energy;
          double CTD_Sigma = CTD_FWHM / 2.355;
          double ElectronDriftTime = ElectronSpline.Eval(Z) * Stretch;

          // Convert drift time to timing by subtracting 4200ns (for now)
          // TODO: Improve determining the timing from drift times
          SH.m_Timing = 4200.0 - ElectronDriftTime;

          // Smear the timing value based on the given CTD resolution
          // --> divide by √2 to obtain TAC resolution from CTD resolution
          if (m_ApplyTimingResolutionCalibration == true) {
            SH.m_Timing = gRandom->Gaus(SH.m_Timing, CTD_Sigma / TMath::Sqrt(2.0));
          }

          // Apply the inverse TAC cal to obtain TAC in ADC units
          if (m_TACCal.count(DetID) == 1 && m_TACCal[DetID].size() >= 2 && StripID < m_TACCal[DetID][1].size()) {
            vector<double> TACCal = m_TACCal[DetID][1][StripID];
            double TACCalSlope = TACCal[0];
            double TACCalOffset = TACCal[1];
            SH.m_TAC = (SH.m_Timing - TACCalOffset) / TACCalSlope;
          } else {
            if (g_Verbosity >= c_Error) {
              cout<<"ERROR in MSubModuleDepthReadout::AnalyzeEvent: No TAC calibration found for HV strip "<<SH.m_ROE.GetStripID()<<endl;
            }
            SH.m_TAC = 0;
          }
          if (SH.m_TAC > 16383) {
            SH.m_TAC = 16383;
          }
        } else {
          if (g_Verbosity >= c_Info) {
            cout<<"MSubModuleDepthReadout::AnalyzeEvent: No depth coefficient found for pixel with code "<<PixelCode<<endl;
          }
          SH.m_TAC = 0;
          SH.m_HasTriggered = false;
        }
      }
    } else {
      if (g_Verbosity >= c_Info) {
        cout << "No Depth Spline for Event with DetID " << DetID << endl;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDepthReadout::LoadSplinesFile(MString FileName)
{
  // Input spline files should have the following format:
  // ### DetID, HV, Temperature, Photopeak Energy
  // depth, ctd, electron_drift_time, hole_drift_time

  MFile SplineFile; 
  if (SplineFile.Open(FileName) == false) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR in MSubModuleDepthReadout::LoadSplinesFile: failed to open depth splines file." << endl;
    }
    return false;
  }

  // Populate these vectors for each detector
  int DetID = -1;
  vector<double> DepthVector;
  vector<double> CTDVector;
  vector<double> ElectronDriftTimeVector;
  vector<double> HoleDriftTimeVector;

  MString Line;
  while (SplineFile.ReadLine(Line) == true) {
    if (Line.Length() != 0) {
      if (Line.BeginsWith("#") == true) {

        if (DetID != -1 && DepthVector.size() > 0) {
          m_DepthGrid[DetID] = DepthVector;
          m_CTDMap[DetID] = CTDVector;
          m_ElectronDriftTimes[DetID] = ElectronDriftTimeVector;
          m_HoleDriftTimes[DetID] = HoleDriftTimeVector;
        }

        // Get the Detector ID from the commented lines
        vector<MString> Tokens = Line.Tokenize(" ");
        DetID = Tokens[1].ToInt();
        DepthVector.clear();
        CTDVector.clear();
        ElectronDriftTimeVector.clear();
        HoleDriftTimeVector.clear();

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

  if (DetID == -1 || m_DepthGrid.size() == 0) {
    if (g_Verbosity >= c_Error) {
      cout << "ERROR in MSubModuleDepthReadout::LoadSplinesFile: No depth splines recovered from the file." << endl;
    }
    return false;
  }

  SplineFile.Close();
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDepthReadout::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize()
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
  MXmlNode* DepthSplineFile = Node->GetNode("DepthSplineFileName");
  if (DepthSplineFile != nullptr) {
    m_DepthSplinesFileName = DepthSplineFile->GetValue();
  }

  MXmlNode* DepthCoefficientsFileName = Node->GetNode("DepthCoefficientsFileName");
  if (DepthCoefficientsFileName != nullptr) {
    m_DepthCoefficientsFileName = DepthCoefficientsFileName->GetValue();
  }

  MXmlNode* TACCalFileName = Node->GetNode("TACCalFileName");
  if (TACCalFileName != nullptr) {
    m_TACCalFileName = TACCalFileName->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleDepthReadout::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  new MXmlNode(Node, "DepthSplineFileName", m_DepthSplinesFileName);
  new MXmlNode(Node, "DepthCoefficientsFileName", m_DepthCoefficientsFileName);
  new MXmlNode(Node, "TACCalFileName", m_TACCalFileName);

  return Node;
}


// MSubModuleDepthReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
