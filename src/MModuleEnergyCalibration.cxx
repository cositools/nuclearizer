/*
 * MModuleEnergyCalibration.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Mark Bandstra, Carolyn Kierans.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer, Mark Bandstra, Carolyn Kierans.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MModuleEnergyCalibration
//
////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <fstream>
#include <iostream>
#include <map>
using namespace std;

// Include the header:
#include "MModuleEnergyCalibration.h"


// ROOT libs:
#include "TGClient.h"
#include "TFile.h"

// MEGAlib libs:
#include "MString.h"
#include "MStreams.h"

// Nuclearizer libs:
#include "MReadOutElement.h"
#include "MReadOutElementDoubleStrip.h"
#include "MGUIOptionsEnergyCalibration.h"
#include "MGUIExpoPlotSpectrum.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleEnergyCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleEnergyCalibration::MModuleEnergyCalibration() : MModule()
{
  // Construct an instance of MModuleEnergyCalibration

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Universal energy calibrator";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "EnergyCalibration";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  // AddPreceedingModuleType(MAssembly::c_TACcut);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_EnergyCalibration);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_TACcut);

  // Set if this module has an options GUI
  m_HasOptionsGUI = true;

  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = true;

  // Initiate the Slow Threshold Cut variables
  m_SlowThresholdCutMode = MSlowThresholdCutModes::e_Ignore;
  m_SlowThresholdCutFixedValue = 15;
  m_SlowThresholdCutFileName = "";
  
  // Nearest Neighbor threshold (-1000 because we don't want to default to cut neg values) 
  m_NearestNeighborCutMode = MNearestNeighborCutModes::e_Ignore;
  m_NearestNeighborThreshold = -1000.0;

}

////////////////////////////////////////////////////////////////////////////////


MModuleEnergyCalibration::~MModuleEnergyCalibration()
{
  // Delete this instance of MModuleEnergyCalibration
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEnergyCalibration::CreateExpos()
{
  // If they are already created, return
  if (m_Expos.size() != 0) {
    return;
  }

  // Set the histogram display
  m_ExpoSpectrum = new MGUIExpoPlotSpectrum(this);
  m_ExpoSpectrum->SetEnergyHistogramParameters(200, 0, 2000);
  m_Expos.push_back(m_ExpoSpectrum);
  
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibration::Initialize()
{
  // Initialize the module

  //Parse the Energy Calibration file
  MParser Parser;
  if (Parser.Open(m_FileName, MFile::c_Read) == false) {
    if (g_Verbosity >= c_Error) {
      cout << m_XmlTag << ": Unable to open calibration file " << m_FileName << endl;
    }
    return false;
  }
  //Parse the slow Threshold file
  if (m_SlowThresholdCutMode == MSlowThresholdCutModes::e_File) {
    MParser Parser_Threshold;
    if (Parser_Threshold.Open(m_SlowThresholdCutFileName, MFile::c_Read) == false) {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open Threshold file "<<m_SlowThresholdCutFileName<<endl;
      return false;
    } else {
      MString Line; // each line of the threshold csv file
      while (Parser_Threshold.ReadLine(Line)) {
        if (!Line.BeginsWith("#")) {
          std::vector<MString> Tokens = Line.Tokenize(","); // for each line, Create tokens seperated by commas
          if (Tokens.size() == 6) {
            int IndexOffset = Tokens.size() % 6; //index counter
            int DetID = Tokens[1 + IndexOffset].ToInt(); // Detector ID
            MString Side = Tokens[2 + IndexOffset].ToString(); // side is a string, either 'l' or 'h'
            int StripID = Tokens[3 + IndexOffset].ToInt(); // stripID
            //int ThresholdADC = Tokens[4 + IndexOffset].ToInt(); // energy threshold in ADC
            double ThresholdKeVFile = Tokens[5 + IndexOffset].ToDouble(); //energy threshold in keV

            MReadOutElementDoubleStrip R;
            R.SetDetectorID(DetID);
            R.SetStripID(StripID);
            R.IsLowVoltageStrip(Side == "l");

            // map detectorID, strip number, and voltage side to the threshold (keV)
            m_ThresholdMap[R] = ThresholdKeVFile;
          }
        }
      }
    }
  }
  // create other maps
  map<MReadOutElementDoubleStrip, unsigned int> CP_ROEToLine; //Peak fits
  map<MReadOutElementDoubleStrip, unsigned int> CM_ROEToLine; //Energy Calibration Model
  map<MReadOutElementDoubleStrip, unsigned int> CR_ROEToLine; //Energy Resolution Calibration Model

  //tokenize ecal file
  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
    if (NTokens < 2) {
      continue;
    }
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "CP") == true ||
        Parser.GetTokenizerAt(i)->IsTokenAt(0, "CM") == true || Parser.GetTokenizerAt(i)->IsTokenAt(0, "CR") == true) {
      if (Parser.GetTokenizerAt(i)->IsTokenAt(1, "dss") == true) {

        // input token values to map
        MReadOutElementDoubleStrip R;
        R.SetDetectorID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2));
        R.SetStripID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3));
        R.IsLowVoltageStrip((Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "p") || (Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "l"));
        if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "CP") == true) {
          CP_ROEToLine[R] = i;
        } else if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "CM") == true) {
          CM_ROEToLine[R] = i;
        } else {
          CR_ROEToLine[R] = i;
        }
      } else {
        if (g_Verbosity >= c_Error) {
          cout << m_XmlTag << ": Line parser: Unknown read-out element (" << Parser.GetTokenizerAt(i)->GetTokenAt(1) << ")" << endl;
        }
        return false;
      }
    }
  }

  for (auto CM : CM_ROEToLine) {
    // If we have at least three data points, we store the calibration

    if (CP_ROEToLine.find(CM.first) != CP_ROEToLine.end()) {
      unsigned int i = CP_ROEToLine[CM.first];
      if (Parser.GetTokenizerAt(i)->IsTokenAt(5, "pakw") == false) {
        if (g_Verbosity >= c_Warning) {
          cout << m_XmlTag << ": Unknown calibration point descriptor found: " << Parser.GetTokenizerAt(i)->GetTokenAt(5) << endl;
        }
        continue;
      }
    } else {
      if (g_Verbosity >= c_Warning) {
        cout << m_XmlTag << ": No good calibration for the following strip found: " << CM.first << endl;
      }
      continue;
    }

    // Read the calibrator, i.e. read the fit function from the .ecal Melinator file.

    unsigned int Pos = 5;
    MString CalibratorType = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsString(Pos);
    CalibratorType.ToLower();

    // Below inclusion of poly1zero written by J. Beechert on 2019/11/15
    if (CalibratorType == "poly1zero") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      //From the fit parameters I just extracted from the .ecal file, I can define a function
      TF1* melinatorfit = new TF1("poly1zero", "0. + [0]*x", 0., 8191.);
      melinatorfit->FixParameter(0, a0);

      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_Calibration[CM.first] = melinatorfit;

    }

    // Below inclusion of poly1 and poly2 written by J. Beechert on 2019/10/24
    else if (CalibratorType == "poly1") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      //From the fit parameters I just extracted from the .ecal file, I can define a function
      TF1* melinatorfit = new TF1("poly1", "[0] + [1]*x", 0., 8191.);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);

      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_Calibration[CM.first] = melinatorfit;

    } else if (CalibratorType == "poly2") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      //From the fit parameters I just extracted from the .ecal file, I can define a function
      TF1* melinatorfit = new TF1("poly2", "[0] + [1]*x + [2]*x^2", 0., 8191.);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);
      melinatorfit->FixParameter(2, a2);

      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_Calibration[CM.first] = melinatorfit;

    }
    //Eventually, I'll be including other possible fits, but for now, we've just include poly3 and poly4
    else if (CalibratorType == "poly3") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a3 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      //From the fit parameters I just extracted from the .ecal file, I can define a function
      TF1* melinatorfit = new TF1("poly3", "[0] + [1]*x + [2]*x^2 + [3]*x^3", 0., 8191.);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);
      melinatorfit->FixParameter(2, a2);
      melinatorfit->FixParameter(3, a3);

      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_Calibration[CM.first] = melinatorfit;

    } else if (CalibratorType == "poly4") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a3 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a4 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);

      //From the fit parameters I just extracted from the .ecal file, I can define a function
      TF1* melinatorfit = new TF1("poly4", "[0] + [1]*x + [2]*x^2 + [3]*x^3 + [4]*x^4", 0., 8191.);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);
      melinatorfit->FixParameter(2, a2);
      melinatorfit->FixParameter(3, a3);
      melinatorfit->FixParameter(4, a4);

      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_Calibration[CM.first] = melinatorfit;

    } else {
      if (g_Verbosity >= c_Error) {
        cout << m_XmlTag << ": Line parser: Unknown calibrator type (" << CalibratorType << ") for strip" << CM.first << endl;
      }
      continue;
    }
  }

  for (auto CR : CR_ROEToLine) {

    unsigned int Pos = 5;
    MString CalibratorType = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsString(Pos);
    CalibratorType.ToLower();
    if (CalibratorType == "p1" || CalibratorType == "poly1") {
      double f0 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      double f1 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      TF1* resolutionfit = new TF1("P1", "([0]+[1]*x) / 2.355", 0., 2000.);
      resolutionfit->FixParameter(0, f0);
      resolutionfit->FixParameter(1, f1);

      m_ResolutionCalibration[CR.first] = resolutionfit;
    } else if (CalibratorType == "p2" || CalibratorType == "poly2") {
      double f0 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      double f1 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      double f2 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      TF1* resolutionfit = new TF1("P2", "([0]+[1]*x+[2]*x*x) / 2.355", 0., 2000.);
      resolutionfit->FixParameter(0, f0);
      resolutionfit->FixParameter(1, f1);
      resolutionfit->FixParameter(2, f2);
      m_ResolutionCalibration[CR.first] = resolutionfit;
    } else {
      if (g_Verbosity >= c_Error) {
        cout << m_XmlTag << ": Line parser: Unknown resolution calibrator type (" << CalibratorType << ") for strip" << CR.first << endl;
      }
      continue;
    }
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibration::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level, i.e. takes the raw ADC value from the .roa file loaded through nuclearizer and converts it into energy units.

  for (unsigned int i = 0; i < Event->GetNStripHits();) {

    MStripHit* SH = Event->GetStripHit(i);
    MReadOutElementDoubleStrip R = *dynamic_cast<MReadOutElementDoubleStrip*>(SH->GetReadOutElement());

    TF1* Fit = m_Calibration[R];
    TF1* FitRes = m_ResolutionCalibration[R];

    if (Fit == nullptr) {
      if (g_Verbosity >= c_Error) {
        cout << m_XmlTag << ": Error: Energy-fit not found for read-out element " << R << endl;
      }
      Event->SetEnergyCalibrationError("calibration not found for " + R.ToString());
      ++i; // iterate to next SH
      continue;

    } else {

      double Energy = 0;
      double Threshold; // No default value here, we set it below (different for strips and nearest neighbors)

      Energy = Fit->Eval(SH->GetADCUnits());
      
      // TODO(@RobinAnthonyPetersen): Determine if we want to force negative energy values to be zero or not
      // If the calibrated energy is less than 0, force it to be 0.
      //if (Energy < 0) {
      //  Energy = 0;
      //}
      
      if (SH->IsNearestNeighbor() == true) {
        // If nothing is selected, we set thresholds to zero
        // Threshold = 0.0; // But keeping negative values for now
        // If nothing is selected, we keep negative nearest neighbors
        Threshold = -100.0;
        
        // Otherwise, if the user inputs a value, we use that threshold
        if (m_NearestNeighborCutMode == MNearestNeighborCutModes::e_Fixed) {
          // Get the value user typed in the box (for example 6.0 keV)
          // TODO(@RobinAnthonyPetersen): Nearest Neighbor threhsold cut subject to change pending more analysis
          Threshold = m_NearestNeighborThreshold;
        }
      } else { // If not Nearest Neighbor, then it's a triggered strip
        
        // Set the default slow threshold
        Threshold = 0.0;
        
        if (m_SlowThresholdCutMode == MSlowThresholdCutModes::e_Fixed) { // check if user input threshold is enabled (one value applied to all strips)
          Threshold = m_SlowThresholdCutFixedValue;
        } else if (m_SlowThresholdCutMode == MSlowThresholdCutModes::e_File) { // check if threshold file is enabled (unique value applied to each strip)
          double ThresholdFromFile = m_ThresholdMap[R]; // if file enabled, declare value from map

          if (ThresholdFromFile == 0) {
            if (g_Verbosity >= c_Error) {
              cout << m_XmlTag << ": Error: Threshold not found for read-out element " << R << endl;
            }
            const double DefaultSlowThreshold = 15.0;
            Threshold = DefaultSlowThreshold; // set default threshold if threshold not found
          } else {
            Threshold = ThresholdFromFile; // set threshold variable to value found in map
          }
        }
      }
  

      // Remove SH for any energy value below the established threshold (0 is default)
      if (Energy < Threshold) {
        if (g_Verbosity >= c_Warning) {
          cout << m_XmlTag << ": Strip Hit below threshold, deleting SH with Energy " << Energy << " keV " << endl;
        }
        Event->SetStripHitBelowThreshold_QualityFlag("Strip hit removed with energy " + to_string(Energy));
        Event->RemoveStripHit(i);
        delete SH;
        continue; // continue to next SH without iterating i
      } else {

        // if the energy isn't filtered out with the threshold, then assign the energy to the SH
        ++i; // iterate to next SH
        SH->SetEnergy(Energy);

        if (FitRes == nullptr) {
          if (g_Verbosity >= c_Error) {
            cout << m_XmlTag << ": Error: Energy Resolution fit not found for read-out element " << R << endl;
          }
	  // There is not expected to be a time in which the energy resolution calibration is not defined when the energy calibration itself is. Therefore, don't need a seperate BD flag for this.
        } else {
          double EnergyResolution = FitRes->Eval(Energy);
          SH->SetEnergyResolution(EnergyResolution);
        }
        if (HasExpos() == true) {
          m_ExpoSpectrum->AddEnergyFinal(Energy, SH->IsNearestNeighbor(), SH->IsLowVoltageStrip());
        }
        if (g_Verbosity >= c_Info) {
          cout << m_XmlTag << ": Energy: " << SH->GetADCUnits() << " adc --> " << Energy << " keV" << endl;
        }
      }
    }
  }
  Event->SetAnalysisProgress(MAssembly::c_EnergyCalibration);

  return true;
}


/////////////////////////////////////////////////////////////////////////////////


double MModuleEnergyCalibration::GetEnergy(MReadOutElementDoubleStrip R, double ADC)
{
  //! Return the energy for a given ADC value or zero in case of error

  TF1* Fit = m_Calibration[R];
  double Energy = 0.0;
  if (Fit != nullptr) {
    Energy = Fit->Eval(ADC);
    if (Energy < 0.0) {
      Energy = 0.0;
    }
  } else {
    cout << m_Name << ": GetEnergy: Error unable to find calibration" << endl;
    return 0;
  }

  return Energy;
}


///////////////////////////////////////////////////////////////////////////////


double MModuleEnergyCalibration::GetADC(MReadOutElementDoubleStrip R, double Energy)
{
  //! Return the ADC value for a given energy

  TF1* Fit = m_Calibration[R]; // TF1* is a function to be applied
  if (Fit != nullptr) {
    return Fit->GetX(Energy);
  } else {
    cout << m_Name << ": GetADC: Error unable to find calibration" << endl;
    return 0;
  }
}


///////////////////////////////////////////////////////////////////////////////


void MModuleEnergyCalibration::Finalize()
{
  // Finalize the calibrator and clean up

  MModule::Finalize();

  for (auto& F : m_Calibration) {
    delete F.second;
  }
  for (auto& F : m_ResolutionCalibration) {
    delete F.second;
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEnergyCalibration::ShowOptionsGUI()
{
  // Show the options GUI

  MGUIOptionsEnergyCalibration* Options = new MGUIOptionsEnergyCalibration(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibration::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != nullptr) {
    m_FileName = FileNameNode->GetValue();
  }
  
  MXmlNode* SlowThresholdCutModeNode = Node->GetNode("SlowThresholdCutMode");
  if (SlowThresholdCutModeNode != nullptr) {
    m_SlowThresholdCutMode = static_cast<MSlowThresholdCutModes>(SlowThresholdCutModeNode->GetValueAsInt());
  }
  MXmlNode* SlowThresholdCutFixedValueNode = Node->GetNode("SlowThresholdCutFixedValue");
  if (SlowThresholdCutFixedValueNode != nullptr) {
    m_SlowThresholdCutFixedValue = SlowThresholdCutFixedValueNode->GetValueAsDouble();
  }
  MXmlNode* SlowThresholdCutFileNameNode = Node->GetNode("SlowThresholdCutThresholdFileName");
  if (SlowThresholdCutFileNameNode != nullptr) {
    m_SlowThresholdCutFileName = SlowThresholdCutFileNameNode->GetValue();
  }
  MXmlNode* NNCutModeNode = Node->GetNode("NearestNeighborCutMode");
  if (NNCutModeNode != nullptr) {
    m_NearestNeighborCutMode = static_cast<MNearestNeighborCutModes>(NNCutModeNode->GetValueAsInt());
  }
  MXmlNode* NearestNeighborThresholdNode = Node->GetNode("NearestNeighborThreshold");
  if (NearestNeighborThresholdNode != nullptr) {
    m_NearestNeighborThreshold = NearestNeighborThresholdNode->GetValueAsDouble();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleEnergyCalibration::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);
  new MXmlNode(Node, "SlowThresholdCutMode", static_cast<int>(m_SlowThresholdCutMode));
  new MXmlNode(Node, "SlowThresholdCutFixedValue", m_SlowThresholdCutFixedValue);
  new MXmlNode(Node, "SlowThresholdCutThresholdFileName", m_SlowThresholdCutFileName);
  new MXmlNode(Node, "NearestNeighborCutMode", static_cast<int>(m_NearestNeighborCutMode));
  new MXmlNode(Node, "NearestNeighborThreshold", m_NearestNeighborThreshold);

  return Node;
}

/////////////////////////////////////////////////////////////////////////////////


double MModuleEnergyCalibration::LookupEnergyResolution(MStripHit* SH, double Energy)
{
  //! Return the energy resolution or -1 in case of error

  MReadOutElementDoubleStrip* ROE = dynamic_cast<MReadOutElementDoubleStrip*>(SH->GetReadOutElement());
  if (ROE == nullptr) {
    cout << m_Name << ": LookupEnergyResolution: Error unable to get read-out element" << endl;
    return -1;
  }
  TF1* FitRes = m_ResolutionCalibration[*ROE];
  if (FitRes == nullptr) {
    cout << m_Name << ": LookupEnergyResolutio: Error: Couldn't locate energy resolution" << endl;
    return -1.0;
  } else {
    return FitRes->Eval(Energy);
  }
}


// MModuleEnergyCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
