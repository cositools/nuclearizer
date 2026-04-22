/*
 * MSubModuleStripReadout.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Robin Anthony-Petersen.
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
// MSubModuleStripReadout
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleStripReadout.h"

// Standard libs:

// ROOT libs:
#include "TRandom.h"
#include "TMath.h"

// MEGAlib libs:
#include "MParser.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleStripReadout)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadout::MSubModuleStripReadout() : MSubModule()
{
  // Construct an instance of MSubModuleStripReadout

  m_Name = "DEE strip readout module";

  m_EnergyCalibrationFileName = "";
  
  // Max value for the ADC range (14-bit ADC maximum)
  m_MaxADCRange = 16383;
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadout::~MSubModuleStripReadout()
{
  // Delete this instance of MSubModuleStripReadout
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadout::Initialize()
{
  // Initialize the module

  // Check if we have a file
  if (m_EnergyCalibrationFileName == "") {
    if (g_Verbosity >= c_Error) {
      cout << m_Name << ": No energy calibration file specified." << endl;
    }
    return false;
  }

  // Open ecal file
  MParser Parser;
  if (Parser.Open(m_EnergyCalibrationFileName, MFile::c_Read) == false) {
    if (g_Verbosity >= c_Error) {
      cout << m_Name << ": Unable to open calibration file " << m_EnergyCalibrationFileName << endl;
    }
    return false;
  }

  // Create the map to store line numbers
  map<MReadOutElementDoubleStrip, unsigned int> CM_ROEToLine;
    
  // Clear the maps before reading
  m_ResolutionCalibration.clear();
  m_Calibration.clear();

  
  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    MTokenizer* T = Parser.GetTokenizerAt(i);
    
    // If user wants it, get the fits to smear the energies based on the FWHM from the ecal
    if (m_ApplyResolutionCalibration == true) {
      
      if (T->GetNTokens() >= 6 && T->IsTokenAt(0, "CR") == true && T->IsTokenAt(1, "dss") == true) {
        MReadOutElementDoubleStrip R;
        R.SetDetectorID(T->GetTokenAtAsUnsignedInt(2));
        R.SetStripID(T->GetTokenAtAsUnsignedInt(3));
        R.IsLowVoltageStrip((T->GetTokenAtAsString(4) == "p") || (T->GetTokenAtAsString(4) == "l"));
        
        MString ResolutionCalibrationType = T->GetTokenAtAsString(5);
        ResolutionCalibrationType.ToLower();
        
        // position of the fwhm and max keV value
        unsigned int PosResolution = 6;
        unsigned int HistMaxkeV = 10000;
        
        // Look for the CR lines
        if (ResolutionCalibrationType == "poly1") {
          double a0 = T->GetTokenAtAsDouble(PosResolution++);
          double a1 = T->GetTokenAtAsDouble(PosResolution++);
          
          TF1* resFit = new TF1("res_poly1", "[0] + [1]*x", 0., HistMaxkeV);
          resFit->SetParameters(a0, a1);
          m_ResolutionCalibration[R] = resFit;
          
        } else if (ResolutionCalibrationType == "poly2") {
          double a0 = T->GetTokenAtAsDouble(PosResolution++);
          double a1 = T->GetTokenAtAsDouble(PosResolution++);
          double a2 = T->GetTokenAtAsDouble(PosResolution++);
          
          TF1* resFit = new TF1("res_poly2", "[0] + [1]*x + [2]*x^2", 0., HistMaxkeV);
          resFit->SetParameters(a0, a1, a2);
          m_ResolutionCalibration[R] = resFit;
          
        } else if (ResolutionCalibrationType == "poly3") {
          double a0 = T->GetTokenAtAsDouble(PosResolution++);
          double a1 = T->GetTokenAtAsDouble(PosResolution++);
          double a2 = T->GetTokenAtAsDouble(PosResolution++);
          double a3 = T->GetTokenAtAsDouble(PosResolution++);
          
          TF1* resFit = new TF1("res_poly3", "[0] + [1]*x + [2]*x^2 + [3]*x^3", 0., HistMaxkeV);
          resFit->SetParameters(a0, a1, a2, a3);
          m_ResolutionCalibration[R] = resFit;
          
        } else {
          // TODO: @RobinAnthonyPetersen add all the other types of fits melinator can do
          // So far, only added these ones because these are the ones we use for the ecals
          if (g_Verbosity >= c_Warning) {
            cout << m_Name << ": Unknown FWHM calibrator type: " << ResolutionCalibrationType << endl;
          }
        }
      }
    }
    
    
    // Get enegry calibration fits second
    if (T->GetNTokens() >= 2 && T->IsTokenAt(0, "CM") == true && T->IsTokenAt(1, "dss") == true) {
      
      MReadOutElementDoubleStrip R;
      R.SetDetectorID(T->GetTokenAtAsUnsignedInt(2));
      R.SetStripID(T->GetTokenAtAsUnsignedInt(3));
      R.IsLowVoltageStrip((T->GetTokenAtAsString(4) == "p") ||
                          (T->GetTokenAtAsString(4) == "l"));
      
      unsigned int Pos = 5;
      
      // Get CM token
      MString CalibratorType = T->GetTokenAtAsString(Pos);
      CalibratorType.ToLower();
      
      
      if (CalibratorType == "poly1") {
        double a0 = T->GetTokenAtAsDouble(++Pos);
        double a1 = T->GetTokenAtAsDouble(++Pos);
        
        TF1* melinatorfit = new TF1("poly1", "[0] + [1]*x", 0., m_MaxADCRange);
        melinatorfit->FixParameter(0, a0);
        melinatorfit->FixParameter(1, a1);
        
        m_Calibration[R] = melinatorfit;
      } else if (CalibratorType == "poly2") {
        double a0 = T->GetTokenAtAsDouble(++Pos);
        double a1 = T->GetTokenAtAsDouble(++Pos);
        double a2 = T->GetTokenAtAsDouble(++Pos);
        
        TF1* melinatorfit = new TF1("poly2", "[0] + [1]*x + [2]*x^2", 0., m_MaxADCRange);
        melinatorfit->FixParameter(0, a0);
        melinatorfit->FixParameter(1, a1);
        melinatorfit->FixParameter(2, a2);
        
        m_Calibration[R] = melinatorfit;
      } else if (CalibratorType == "poly3") {
        double a0 = T->GetTokenAtAsDouble(++Pos);
        double a1 = T->GetTokenAtAsDouble(++Pos);
        double a2 = T->GetTokenAtAsDouble(++Pos);
        double a3 = T->GetTokenAtAsDouble(++Pos);
        
        TF1* melinatorfit = new TF1("poly3", "[0] + [1]*x + [2]*x^2 + [3]*x^3", 0., m_MaxADCRange);
        melinatorfit->FixParameter(0, a0);
        melinatorfit->FixParameter(1, a1);
        melinatorfit->FixParameter(2, a2);
        melinatorfit->FixParameter(3, a3);
        
        m_Calibration[R] = melinatorfit;
      } else {
        // TODO: @RobinAnthonyPetersen add all the other types of fits melinator can do
        // So far, only added these ones because these are the ones we use for the ecals
        if (g_Verbosity >= c_Error) {
          cout<<m_Name<<": Unhandled CalibratorType: "<<CalibratorType<<endl<<"Please update this module."<<endl;
        }
        return false;
      }
    }
  }

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripReadout::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadout::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level
  
  static const double FWHMtoSigma = 2.0 * TMath::Sqrt(2.0 * TMath::Log(2.0));

  // Get low-voltage and high-voltage hits
  for (auto* Hits : { &Event->GetDEEStripHitLVListReference(), &Event->GetDEEStripHitHVListReference() }) {
    
    for (MDEEStripHit& SH : *Hits) {
      
      // If the user wants it applied, apply the FWHM Guassian energy resolution
      if (m_ApplyResolutionCalibration == true) {
        // Look up the FWHM fit for this strip
        if (m_ResolutionCalibration.count(SH.m_ROE) > 0) {
          
          // Calculate FWHM (keV) at this energy
          double fwhm = m_ResolutionCalibration[SH.m_ROE]->Eval(SH.m_Energy);
          
          // Convert FWHM to Sigma (FWHM = 2.355 * Sigma)
          double sigma = fwhm / FWHMtoSigma;
          
          // Smear the hit energy using a Gaussian distribution
          SH.m_Energy = gRandom->Gaus(SH.m_Energy, sigma);
          
          // If energy is lower than zero now, floor it to zero
          if (SH.m_Energy < 0) {
            SH.m_Energy = 0;
          }
          
        } else {
          // The fit wasn't found! Handle the error
          if (g_Verbosity >= c_Warning) {
          cout << m_Name << ": Warning - No resolution calibration fit found for strip ID " << SH.m_ROE.GetStripID() << endl;
          }
          // Note, if there is not calibration found then the energy remains unsmeared
        }
      }
      
      // Apply the inverse energy calibration
      // Look up the fit using the ecal
      TF1* Fit = m_Calibration[SH.m_ROE];

      if (Fit != nullptr) {
        // Apply the inverse energy calibration using ROOT's poly inverter (keV -> ADC) in the allowed ADC range
        double calculatedADC = Fit->GetX(SH.m_Energy, 0., m_MaxADCRange);
        
        // Apply hardware limits
        if (calculatedADC > m_MaxADCRange) calculatedADC = m_MaxADCRange;
        if (calculatedADC < 0) calculatedADC = 0;
        
        SH.m_ADC = static_cast<unsigned int>(calculatedADC);
        
      } else {
        // If no calibration exists in the .ecal file for this strip set it to ADC value of 0
        if (g_Verbosity >= c_Warning) cout << m_Name << ": No inverse calibration found for element " << SH.m_ROE << endl;
        SH.m_ADC = 0;
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripReadout::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize()

  // Clean up the memory 
  for (auto& F : m_Calibration) {
    delete F.second;
  }
  m_Calibration.clear();
  
  // Clean up the resolution calibration memory
  for (auto& F : m_ResolutionCalibration) {
    delete F.second;
  }
  m_ResolutionCalibration.clear();

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadout::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* N = Node->GetNode("EnergyCalibrationFileName");
  if (N != nullptr) {
    m_EnergyCalibrationFileName = N->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleStripReadout::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  new MXmlNode(Node, "EnergyCalibrationFileName", m_EnergyCalibrationFileName);

  return Node;
}


// MSubModuleStripReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
