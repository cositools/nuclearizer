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

  if (LoadSplinesFile() == false) {
    return false;
  }

  if (LoadCoeffsFile() == false) {
    return false;
  }

  if (LoadTACCalFile() == false) {
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
    if (m_DepthGrid.count(DetID) == 1) {
      if (SH.m_IsGuardRing == false) {

        // Determine the hole drift times (accounting for electronics)
        // TODO: This only applies to main strips, find a better implementation for nearest neighbors
        vector<double> DepthGrid = m_DepthGrid[DetID];
        vector<double> HoleDriftTimes = m_HoleDriftTimes[DetID];
        TSpline3 HoleSpline = TSpline3("", &DepthGrid[0], &HoleDriftTimes[0], DepthGrid.size());

        // Apply stretch and offset based on Eq. (3) in https://doi.org/10.1016/j.nima.2026.171332
        int PixelCode = 10000*SH.m_ROE.GetDetectorID() + 100*SH.m_ROE.GetStripID() + SH.m_OppositeStripID;
        if (m_Coeffs.count(PixelCode) == 1){
          vector<double> Coeffs = m_Coeffs[PixelCode];
          double Stretch = Coeffs[0];
          double Offset = Coeffs[1];
          double CTD_Sigma = Coeffs[2] * m_Coeffs_Energy / SH.m_Energy;
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
          int StripCode = SH.m_ROE.GetDetectorID() + 100 * SH.m_ROE.GetStripID();
          if (m_TACCal.count(StripCode) == 1) {
            vector<double> TACCal = m_TACCal[StripCode];
            double TACCalSlope = TACCal[0];
            double TACCalOffset = TACCal[1];
            SH.m_TAC = (SH.m_Timing - TACCalOffset) / TACCalSlope;
          } else {
            if (g_Verbosity >= c_Error) {
              cout<<"ERROR in MSubModuleDepthReadout::AnalyzeEvent: No TAC calibration found for LV strip "<<SH.m_ROE.GetStripID()<<endl;
            }
            SH.m_TAC = 0;
          }
          if (SH.m_TAC > 16383) {
            SH.m_TAC = 16383;
          }
        } else {
          if (g_Verbosity >= c_Info) {
            cout<<"MSubModuleDepthReadout::AnalyzeEvent: No depth coefficient found for pixel with code "<<PixelCode<<"."<<endl;
            SH.m_TAC = 0;
            SH.m_HasTriggered = false;
          }
        }
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

        // Apply stretch based on Eq. (3) in https://doi.org/10.1016/j.nima.2026.171332
        // Apply no offset to the electron drift time --> add it fully to the hole signal
        int PixelCode = 10000*SH.m_ROE.GetDetectorID() + 100*SH.m_OppositeStripID + SH.m_ROE.GetStripID();
        if (m_Coeffs.count(PixelCode) == 1){
          vector<double> Coeffs = m_Coeffs[PixelCode];
          double Stretch = Coeffs[0];
          double CTD_Sigma = Coeffs[2] * m_Coeffs_Energy / SH.m_Energy;
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
          int StripCode = SH.m_ROE.GetDetectorID() + SH.m_ROE.GetStripID();
          if (m_TACCal.count(StripCode) == 1) {
            vector<double> TACCal = m_TACCal[StripCode];
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
            SH.m_TAC = 0;
            SH.m_HasTriggered = false;
          }
        }
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


// Copied from MModuleDepthCalibration.cxx, 
// adjusted for filename and for storing CTD_Sigma instead of CTD_FWHM
bool MSubModuleDepthReadout::LoadCoeffsFile() {
  // Read in the stretch and offset file, which should have a header line with information on the measurements:
  // ### 800 V 80 K 59.5 keV
  // And which should contain for each pixel:
  // Pixel code (10000*det + 100*LVStrip + HVStrip), Stretch, Offset, Timing/CTD noise, Chi2 for the CTD fit (for diagnostics mainly)

  MFile CoeffsFile;
  if (CoeffsFile.Open(m_DepthCoefficientsFile) == false) {
    cout << "ERROR in MSubModuleDepthReadout::LoadCoeffsFile: failed to open coefficients file." << endl;
    return false;
  }

  MString Line;
  while (CoeffsFile.ReadLine(Line)) {
    if (Line.BeginsWith('#') == true) {
      std::vector<MString> Tokens = Line.Tokenize(" ");
      m_Coeffs_Energy = Tokens[5].ToDouble();
      if (g_Verbosity >= c_Info) cout << "MSubModuleDepthReadout: The stretch and offset were calculated for " << m_Coeffs_Energy << " keV." << endl;
    } else {
      std::vector<MString> Tokens = Line.Tokenize(",");
      if (Tokens.size() == 5) {
        int PixelCode = Tokens[0].ToInt();
        double Stretch = Tokens[1].ToDouble();
        double Offset = Tokens[2].ToDouble();
        double CTD_Sigma = Tokens[3].ToDouble();
        double Chi2 = Tokens[4].ToDouble();
        // Previous iteration of depth calibration read in "Scale" instead of ctd resolution.
        vector<double> coeffs;
        coeffs.push_back(Stretch); coeffs.push_back(Offset); coeffs.push_back(CTD_Sigma); coeffs.push_back(Chi2);
        m_Coeffs[PixelCode] = coeffs;
      }
    }
  }

  CoeffsFile.Close();

  return true;

}


////////////////////////////////////////////////////////////////////////////////


// Copied from MModuleTACcut.cxx
bool MSubModuleDepthReadout::LoadTACCalFile()
{
  // Read in the TAC Calibration file, which should contain for each strip:
  // DetID, Side (h or l for high or low voltage), TAC cal, TAC cal error, TAC cal offset, TAC offset error
  // OR:
  // ReadOutID, Detector, Side, Strip, TAC cal, TAC cal error, TAC offset, TAC offset error
  MFile F;
  if (F.Open(m_TACCalFile) == false) {
    cout<<"ERROR in MSubModuleDepthReadout::LoadTACCalFile: Failed to open TAC Calibration file."<<endl;
    return false;
  } else {
    MString Line;
    while (F.ReadLine(Line)) {
      if (!Line.BeginsWith("#")) {
        std::vector<MString> Tokens = Line.Tokenize(",");
        if ((Tokens.size() == 7) || (Tokens.size() == 8)) {
          int IndexOffset = Tokens.size() % 7;
          int DetID = Tokens[0+IndexOffset].ToInt();
          MString SideString = Tokens[1+IndexOffset].Trim();
          char Side;
          if (SideString.Length()!=1) {
            cout<<"ERROR in MSubModuleDepthReadout::LoadTACCalFile: Expected 1 character Side, got string \""<<SideString<<"\" in TAC calibration file."<<endl;
            return false;
          }
          else {
            Side = SideString[0];
          }
          int StripID = Tokens[2+IndexOffset].ToInt();
          double TACCal = Tokens[3+IndexOffset].ToDouble();
          double TACCalError = Tokens[4+IndexOffset].ToDouble();
          double Offset = Tokens[5+IndexOffset].ToDouble();
          double OffsetError = Tokens[6+IndexOffset].ToDouble();
          vector<double> CalValues;
          CalValues.push_back(TACCal); CalValues.push_back(Offset); CalValues.push_back(TACCalError); CalValues.push_back(OffsetError);

          unordered_map<char, int> SideToIndex = {{'l', 100}, {'h', 1}, {'0', 100}, {'1', 1}, {'p', 100}, {'n', 1}};
          
          if (SideToIndex.find(Side) != SideToIndex.end()) {
            int StripCode = 10000 * DetID + SideToIndex[Side] * StripID;
            m_TACCal[StripCode] = CalValues;
          } else {
            cout<<"ERROR in MSubModuleDepthReadout::LoadTACCalFile: Unable to identify Side \""<<Side<<"\" in TAC calibration file."<<endl;
            return false;
          }
        }
      }
    }
    F.Close();
  }

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
    m_DepthSplinesFile = DepthSplineFile->GetValue();
  }

  MXmlNode* DepthCoefficientsFile = Node->GetNode("DepthCoefficientsFileName");
  if (DepthCoefficientsFile != nullptr) {
    m_DepthCoefficientsFile = DepthCoefficientsFile->GetValue();
  }

  MXmlNode* TACCalFile = Node->GetNode("TACCalFileName");
  if (TACCalFile != nullptr) {
    m_TACCalFile = TACCalFile->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleDepthReadout::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  new MXmlNode(Node, "DepthSplineFileName", m_DepthSplinesFile);
  new MXmlNode(Node, "DepthCoefficientsFileName", m_DepthCoefficientsFile);
  new MXmlNode(Node, "TACCalFileName", m_TACCalFile);

  return Node;
}


// MSubModuleDepthReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
