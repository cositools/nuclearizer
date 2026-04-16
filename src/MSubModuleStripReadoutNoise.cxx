/*
 * MSubModuleStripReadoutNoise.cxx
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
// MSubModuleStripReadoutNoise
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleStripReadoutNoise.h"

// Standard libs:

// ROOT libs:
#include "TRandom.h"
#include "TMath.h"

// MEGAlib libs:
#include "MSubModule.h"
#include "MParser.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleStripReadoutNoise)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadoutNoise::MSubModuleStripReadoutNoise() : MSubModule()
{
  // Construct an instance of MSubModuleStripReadoutNoise

  m_Name = "DEE strip readout noise module";
  m_EnergyCalibrationFileName = "";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleStripReadoutNoise::~MSubModuleStripReadoutNoise()
{
  // Delete this instance of MSubModuleStripReadoutNoise
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadoutNoise::Initialize()
{
  // Initialize the module

  // Check for ecal file
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

  // Look for the CR lines
  m_ResolutionCalibration.clear();
  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    MTokenizer* T = Parser.GetTokenizerAt(i);
    if (T->GetNTokens() < 6) continue;

    if (T->IsTokenAt(0, "CR") == true && T->IsTokenAt(1, "dss") == true) {
      
      MReadOutElementDoubleStrip R;
      R.SetDetectorID(T->GetTokenAtAsUnsignedInt(2));
      R.SetStripID(T->GetTokenAtAsUnsignedInt(3));
      R.IsLowVoltageStrip((T->GetTokenAtAsString(4) == "p") ||
                          (T->GetTokenAtAsString(4) == "l"));

      MString CalibratorType = T->GetTokenAtAsString(5);
      CalibratorType.ToLower();
      
      // position of the fwhm
      unsigned int Pos = 6;
      unsigned int HistMaxkeV = 10000;

      if (CalibratorType == "poly1") {
        double a0 = T->GetTokenAtAsDouble(Pos++);
        double a1 = T->GetTokenAtAsDouble(Pos++);

        TF1* resFit = new TF1("res_poly1", "[0] + [1]*x", 0., HistMaxkeV);
        resFit->SetParameters(a0, a1);
        m_ResolutionCalibration[R] = resFit;

      } else if (CalibratorType == "poly2") {
        double a0 = T->GetTokenAtAsDouble(Pos++);
        double a1 = T->GetTokenAtAsDouble(Pos++);
        double a2 = T->GetTokenAtAsDouble(Pos++);

        TF1* resFit = new TF1("res_poly2", "[0] + [1]*x + [2]*x^2", 0., HistMaxkeV);
        resFit->SetParameters(a0, a1, a2);
        m_ResolutionCalibration[R] = resFit;

      } else if (CalibratorType == "poly3") {
        double a0 = T->GetTokenAtAsDouble(Pos++);
        double a1 = T->GetTokenAtAsDouble(Pos++);
        double a2 = T->GetTokenAtAsDouble(Pos++);
        double a3 = T->GetTokenAtAsDouble(Pos++);

        TF1* resFit = new TF1("res_poly3", "[0] + [1]*x + [2]*x^2 + [3]*x^3", 0., HistMaxkeV);
        resFit->SetParameters(a0, a1, a2, a3);
        m_ResolutionCalibration[R] = resFit;

      } else {
        if (g_Verbosity >= c_Warning) {
          cout << m_Name << ": Unknown resolution calibrator type: " << CalibratorType << endl;
        }
      }
    }
  }
  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripReadoutNoise::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadoutNoise::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level
  
  double FWHMtoSigma = 2.0 * TMath::Sqrt(2.0 * TMath::Log(2.0));

  // Apply energy resolution smearing to both sides
  for (auto* Hits : { &Event->GetDEEStripHitLVListReference(), &Event->GetDEEStripHitHVListReference() }) {
    
    for (MDEEStripHit& SH : *Hits) {
      
      // Look up the FWHM fit for this strip
      if (m_ResolutionCalibration.count(SH.m_ROE) > 0) {
        
        // Calculate FWHM (keV) at this energy
        double fwhm = m_ResolutionCalibration[SH.m_ROE]->Eval(SH.m_Energy);
        
        // Convert FWHM to Sigma (FWHM = 2.355 * Sigma)
        double sigma = fwhm / FWHMtoSigma;
        
        // Smear the hit energy using a Gaussian distribution
        SH.m_Energy = gRandom->Gaus(SH.m_Energy, sigma);
        
        // If energy is lower than zero now, floor it to zero
        if (SH.m_Energy < 0) SH.m_Energy = 0;
      }
    }
  }

  /*
  // Dummy code:
  list<MDEEStripHit>& LVHits = Event->GetDEEStripHitLVListReference();
  for (MDEEStripHit& SH: LVHits) {
    SH.m_ADC = 2000 + 4*SH.m_Energy;
    if (SH.m_ADC > 16383) SH.m_ADC = 16383;
  }
  list<MDEEStripHit>& HVHits = Event->GetDEEStripHitHVListReference();
  for (MDEEStripHit& SH: HVHits) {
    SH.m_ADC = 2000 + 4*SH.m_Energy;
    if (SH.m_ADC > 16383) SH.m_ADC = 16383;
  }
  */
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleStripReadoutNoise::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  // Clean up the memory 
  for (auto& F : m_ResolutionCalibration) {
    delete F.second;
  }
  m_ResolutionCalibration.clear();

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleStripReadoutNoise::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  /*
  MXmlNode* SomeTagNode = Node->GetNode("SomeTag");
  if (SomeTagNode != 0) {
    m_SomeTagValue = SomeTagNode->GetValue();
  }
  */
  
  MXmlNode* EnergyCalibrationFileNode = Node->GetNode("EnergyCalibrationFileName");
  if (EnergyCalibrationFileNode != 0) {
    m_EnergyCalibrationFileName = EnergyCalibrationFileNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleStripReadoutNoise::CreateXmlConfiguration(MXmlNode* Node)
{
  //! Create an XML node tree from the configuration
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */
  
  new MXmlNode(Node, "EnergyCalibrationFileName", m_EnergyCalibrationFileName);

  return Node;
}


// MSubModuleStripReadoutNoise.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
