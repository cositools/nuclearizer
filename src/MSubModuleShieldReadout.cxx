/*
 * MSubModuleShieldReadout.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Valentina Fioretti.
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
// MSubModuleShieldReadout
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleShieldReadout.h"

// Standard libs:
#include <list>

// ROOT libs:

// MEGAlib libs:
#include "MDEECrystalHit.h"
#include "MParser.h"
#include "MSubModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleShieldReadout)
#endif


constexpr unsigned int MSubModuleShieldReadout::m_NumberOfCSVColumns;
constexpr unsigned int MSubModuleShieldReadout::m_ChannelIdentifierColumn;
constexpr unsigned int MSubModuleShieldReadout::m_QuadraticCoefficientColumn;
constexpr unsigned int MSubModuleShieldReadout::m_LinearCoefficientColumn;
constexpr unsigned int MSubModuleShieldReadout::m_ConstantCoefficientColumn;
constexpr double MSubModuleShieldReadout::m_MaxADCRange;


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldReadout::MSubModuleShieldReadout() : MSubModule()
{
  // Construct an instance of MSubModuleShieldReadout

  m_Name = "DEE shield readout module";
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleShieldReadout::~MSubModuleShieldReadout()
{
  // Delete this instance of MSubModuleShieldReadout
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldReadout::Initialize()
{
  // Initialize the module

  if (!ParseShieldReadoutFile()) {
    if (g_Verbosity >= c_Error) cout << "ERROR: Failed to parse shield readout calibration file "
                                    << m_ShieldReadoutFileName << endl;
    return false;
  }

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldReadout::Clear()
{
  // Clear for the next event

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldReadout::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Convert each shield hit energy into the corresponding ADC value

  std::list<MDEECrystalHit>& Hits = Event->GetDEECrystalHitListReference();

  for (MDEECrystalHit& CrystalHit: Hits) {
    CrystalHit.m_ADC = GetADC(CrystalHit.m_Energy, CrystalHit.m_DetectorID, CrystalHit.m_CrystalID);

    if (CrystalHit.m_ADC > m_MaxADCRange) {
      if (g_Verbosity >= c_Warning) cout << "Shield ADC value above the maximum range defined as "
                                          << m_MaxADCRange << endl;
      CrystalHit.m_ADC = m_MaxADCRange;
    }

    if (CrystalHit.m_ADC < 0.0) {
      if (g_Verbosity >= c_Warning) cout << "Shield ADC value below 0, set to 0" << endl;
      CrystalHit.m_ADC = 0.0;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleShieldReadout::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize()

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


double MSubModuleShieldReadout::GetADC(double Energy,
                                       const MString& DetectorID,
                                       unsigned int CrystalID)
{
  // The two-argument constructor intentionally creates a detector/crystal-only map key.
  const MReadOutElementVoxel3D ReadOutElement(DetectorID, CrystalID);

  const auto CalibrationIterator = m_ADCCalibration.find(ReadOutElement);
  if (CalibrationIterator == m_ADCCalibration.end()) {
    if (g_Verbosity >= c_Error) cout << "ERROR: Shield ADC calibration not found for detector "
                                      << DetectorID << " and crystal " << CrystalID << endl;
    return 0.0;
  }

  const double QuadraticCoefficient = CalibrationIterator->second.m_QuadraticCoefficient;
  const double LinearCoefficient = CalibrationIterator->second.m_LinearCoefficient;
  const double ConstantCoefficient = CalibrationIterator->second.m_ConstantCoefficient;

  // The NRL CSV calibration directly provides ADC as a function of energy:
  // ADC = A*Energy^2 + B*Energy + C
  const double ADC = QuadraticCoefficient * Energy * Energy +
                     LinearCoefficient * Energy +
                     ConstantCoefficient;

  if (g_Verbosity >= c_Info) {
    cout << "DEE shield energy-to-ADC conversion:" << endl;
    cout << "Channel: " << DetectorID << "-" << CrystalID << endl;
    cout << "Energy: " << Energy << endl;
    cout << "ADC: " << ADC << endl;
  }

  return ADC;
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldReadout::ParseShieldReadoutFile()
{

  m_ADCCalibration.clear();

  if (m_ShieldReadoutFileName == "") {
    if (g_Verbosity >= c_Error) cout << "ERROR: Shield readout calibration filename is empty." << endl;
    return false;
  }

  // The shield readout calibration file is a comma-separated CSV file
  MParser Parser(',');
  if (Parser.Open(m_ShieldReadoutFileName, MFile::c_Read) == false) {
    if (g_Verbosity >= c_Error) cout << "Unable to open shield readout calibration file " << m_ShieldReadoutFileName << endl;
    return false;
  }

  unsigned int Parsed = 0;
  unsigned int Skipped = 0;

  for (unsigned int i = 0; i < Parser.GetNLines(); i++) {
    unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
    if (NTokens == 0)
      continue; // skip empty lines

    // Skip the two header lines of the NRL calibration CSV
    MString FirstToken = Parser.GetTokenizerAt(i)->GetTokenAtAsString(0);
    if (FirstToken == "Detector Meta Data" || FirstToken == "scb")
      continue;

    if (NTokens != m_NumberOfCSVColumns) {
      if (g_Verbosity >= c_Warning) {
        cout << "WARNING: Line " << i << ": expected " << m_NumberOfCSVColumns
             << " tokens, got " << NTokens << " (skipping)" << endl;
      }
      ++Skipped;
      continue;
    } // this shouldn't happen but just in case

    // sipm_crystal contains the detector and crystal identifier, e.g. Z1-0
    MString ChannelIdentifier = Parser.GetTokenizerAt(i)->GetTokenAtAsString(m_ChannelIdentifierColumn);
    MTokenizer ChannelTokenizer('-', false);
    ChannelTokenizer.Analyze(ChannelIdentifier);

    if (ChannelTokenizer.GetNTokens() != 2) {
      if (g_Verbosity >= c_Warning) {
        cout << "WARNING: Line " << i << ": invalid sipm_crystal value "
             << ChannelIdentifier << " (skipping)" << endl;
      }
      ++Skipped;
      continue;
    }

    // Detector ID and crystal ID
    MString DetectorID = ChannelTokenizer.GetTokenAtAsString(0);
    unsigned int CrystalID = ChannelTokenizer.GetTokenAtAsUnsignedInt(1);

    // ADC = a*Energy^2 + b*Energy + c
    ShieldCalibration Calibration;
    Calibration.m_QuadraticCoefficient = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(m_QuadraticCoefficientColumn);
    Calibration.m_LinearCoefficient = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(m_LinearCoefficientColumn);
    Calibration.m_ConstantCoefficient = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(m_ConstantCoefficientColumn);

    MReadOutElementVoxel3D ReadOutElement(DetectorID, CrystalID);

    if (m_ADCCalibration.find(ReadOutElement) != m_ADCCalibration.end()) {
      if (g_Verbosity >= c_Warning) {
        cout << "WARNING: Duplicate shield calibration for detector " << DetectorID
             << " and crystal " << CrystalID << " on line " << i
             << "; replacing previous entry" << endl;
      }
    }

    m_ADCCalibration[ReadOutElement] = Calibration;

    ++Parsed;
  }

  if (Parsed == 0) {
    if (g_Verbosity >= c_Error) cout << "ERROR: Parsed 0 valid calibration lines from " << m_ShieldReadoutFileName << endl;
    return false;
  }

  if (Skipped > 0) {
    if (g_Verbosity >= c_Warning) cout << "WARNING: Skipped " << Skipped << " line(s) while parsing " << m_ShieldReadoutFileName << endl;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MSubModuleShieldReadout::ReadXmlConfiguration(MXmlNode* Node)
{
  // Read the configuration data from an XML node

  MXmlNode* ShieldReadoutFileName = Node->GetNode("ShieldReadoutFileName");
  if (ShieldReadoutFileName != nullptr) {
    m_ShieldReadoutFileName = ShieldReadoutFileName->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleShieldReadout::CreateXmlConfiguration(MXmlNode* Node)
{
  // Create an XML node tree from the configuration

  new MXmlNode(Node, "ShieldReadoutFileName", m_ShieldReadoutFileName);

  return Node;
}


// MSubModuleShieldReadout.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

