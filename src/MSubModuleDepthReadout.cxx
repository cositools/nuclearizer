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

// Nuclearizer libs:
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

  m_Coeffs.clear();

  // Load depth-related files using the parsers in MModuleDepthCalibration
  MModuleDepthCalibration DepthCalibration;
  DepthCalibration.SetUCSDOverride(false);

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
    if (SH.m_IsGuardRing == false) {
      unsigned int StripID = SH.m_ROE.GetStripID();
      // Only compute timing values for strip hits with reasonable timing values
      // Note: m_FastPeakTime can be negative if the charge drift time is shorter
      //       than the timing offset between main strips and non-collecting adjacent strips 
      // Note: The upper limit of 10000.0 is chosen to remove strip hits with no depth calibration,
      //       where the default value is set to 1e10
      if (SH.m_FastPeakTime > -200.0 && SH.m_FastPeakTime < 10000.0) {
        SH.m_Timing = 4200.0 - SH.m_FastPeakTime;

        // TODO: apply fast threshold
        SH.m_HasFastTiming = true;

        if (m_ApplyTimingResolutionCalibration == true){
          int PixelCode = 10000*DetID + 100*StripID + SH.m_OppositeStripID;
          if (m_Coeffs.count(PixelCode) == 1){
            vector<double> Coeffs = m_Coeffs[PixelCode];
            double CTD_FWHM = Coeffs[2] * m_Coeffs_Energy / SH.m_Energy;
            double CTD_Sigma = CTD_FWHM / 2.355;
            // Smear the timing value based on the given CTD resolution
            // --> divide by √2 to obtain TAC resolution from CTD resolution
            SH.m_Timing = gRandom->Gaus(SH.m_Timing, CTD_Sigma / TMath::Sqrt(2.0));
          } else {
            if (g_Verbosity >= c_Info) {
              cout<<"MSubModuleDepthReadout::AnalyzeEvent: No depth coefficient found for pixel with code "<<PixelCode<<"."<<endl;
            }
          }
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
          cout<<"MSubModuleDepthReadout::AnalyzeEvent: Unphysical drift time."<<endl;
        }
        SH.m_TAC = 0;
        SH.m_HasFastTiming = false;
      }
    }
  }
  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    int DetID = SH.m_ROE.GetDetectorID();
    if (SH.m_IsGuardRing == false) {
      unsigned int StripID = SH.m_ROE.GetStripID();
      // Only compute timing values for strip hits with reasonable timing values
      // Note: m_FastPeakTime can be negative if the charge drift time is shorter
      //       than the timing offset between main strips and non-collecting adjacent strips 
      // Note: The upper limit of 10000.0 is chosen to remove strip hits with no depth calibration,
      //       where the default value is set to 1e10
      if (SH.m_FastPeakTime > -200.0 && SH.m_FastPeakTime < 10000.0){
        SH.m_Timing = 4200.0 - SH.m_FastPeakTime;

        // TODO: apply fast threshold
        SH.m_HasFastTiming = true;

        if (m_ApplyTimingResolutionCalibration == true) {
          int PixelCode = 10000*DetID + 100*SH.m_OppositeStripID + StripID;
          if (m_Coeffs.count(PixelCode) == 1){
            vector<double> Coeffs = m_Coeffs[PixelCode];
            double CTD_FWHM = Coeffs[2] * m_Coeffs_Energy / SH.m_Energy;
            double CTD_Sigma = CTD_FWHM / 2.355;
            // Smear the timing value based on the given CTD resolution
            // --> divide by √2 to obtain TAC resolution from CTD resolution
            SH.m_Timing = gRandom->Gaus(SH.m_Timing, CTD_Sigma / TMath::Sqrt(2.0));

          } else {
            if (g_Verbosity >= c_Info) {
              cout<<"MSubModuleDepthReadout::AnalyzeEvent: No depth coefficient found for pixel with code "<<PixelCode<<"."<<endl;
            }
          }
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
          cout<<"MSubModuleDepthReadout::AnalyzeEvent: Unphysical drift time."<<endl;
        }
        SH.m_TAC = 0;
        SH.m_HasFastTiming = false;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleDepthReadout::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize()
  m_Coeffs.clear();
  m_TACCal.clear();

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleDepthReadout::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
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
  new MXmlNode(Node, "TACCalFileName", m_TACCalFileName);

  return Node;
}


// MSubModuleDepthReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
