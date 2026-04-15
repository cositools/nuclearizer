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
  
  // Set all modules, which have to be done before this module
  //TODO: @RobinAnthonyPetersen, need to make sure the strip noise gets added first! 
  //AddPreceedingModuleType(MAssembly::c_StripReadoutNoise, true);

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

  // Create the map (same as the Universal Energy Calibrator)
  map<MReadOutElementDoubleStrip, unsigned int> CM_ROEToLine;

  // Add case to handle shorted strips
  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    if (Parser.GetTokenizerAt(i)->GetNTokens() < 2) continue;

    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "CM") == true &&
        Parser.GetTokenizerAt(i)->IsTokenAt(1, "dss") == true) {

      MReadOutElementDoubleStrip R;
      R.SetDetectorID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2));
      R.SetStripID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3));
      R.IsLowVoltageStrip((Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "p") ||
                          (Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "l"));
      
      CM_ROEToLine[R] = i;
    }
  }

  // Get the parameters and store the energy calibration fit function as ROOT's built-in TF1 
  for (auto CM : CM_ROEToLine) {
    unsigned int Pos = 5;
    MString CalibratorType = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsString(Pos);
    CalibratorType.ToLower();

    if (CalibratorType == "poly1") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      TF1* melinatorfit = new TF1("poly1", "[0] + [1]*x", 0., m_MaxADCRange);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);

      m_Calibration[CM.first] = melinatorfit;
    } else if (CalibratorType == "poly2") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      TF1* melinatorfit = new TF1("poly2", "[0] + [1]*x + [2]*x^2", 0., m_MaxADCRange);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);
      melinatorfit->FixParameter(2, a2);

      m_Calibration[CM.first] = melinatorfit;
    } else if (CalibratorType == "poly3") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a3 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      TF1* melinatorfit = new TF1("poly3", "[0] + [1]*x + [2]*x^2 + [3]*x^3", 0., m_MaxADCRange);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);
      melinatorfit->FixParameter(2, a2);
      melinatorfit->FixParameter(3, a3);

      m_Calibration[CM.first] = melinatorfit;
    } else {
      // TODO: Add all the other types of fits melinator can do
      // So far, only added these ones because these are the ones we use for the ecals
      if (g_Verbosity >= c_Error) {
        cout<<m_Name<<": Unhandled CalibratorType: "<<CalibratorType<<endl<<"Please update this module."<<endl;
      }
      return false;
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

  // Get low-voltage and high-voltage hits
  for (auto* Hits : { &Event->GetDEEStripHitLVListReference(), &Event->GetDEEStripHitHVListReference() }) {
    
    for (MDEEStripHit& SH : *Hits) {
    
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
