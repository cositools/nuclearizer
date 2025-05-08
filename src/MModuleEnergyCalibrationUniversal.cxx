/*
 * MModuleEnergyCalibrationUniversal.cxx
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
// MModuleEnergyCalibrationUniversal
//
////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <fstream>
#include <iostream>
#include <map>
using namespace std;

// Include the header:
#include "MModuleEnergyCalibrationUniversal.h"


// ROOT libs:
#include "TGClient.h"
#include "TFile.h"

// MEGAlib libs:
#include "MString.h"
#include "MStreams.h"

// Nuclearizer libs:
#include "MReadOutElement.h"
#include "MReadOutElementDoubleStrip.h"
#include "MCalibratorEnergy.h"
#include "MCalibratorEnergyPointwiseLinear.h"
#include "MGUIOptionsEnergyCalibrationUniversal.h"
#include "MGUIExpoEnergyCalibration.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleEnergyCalibrationUniversal)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleEnergyCalibrationUniversal::MModuleEnergyCalibrationUniversal() : MModule()
{
  // Construct an instance of MModuleEnergyCalibrationUniversal
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Universal energy calibrator";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "EnergyCalibrationUniversal";
  
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
}


////////////////////////////////////////////////////////////////////////////////


MModuleEnergyCalibrationUniversal::~MModuleEnergyCalibrationUniversal()
{
  // Delete this instance of MModuleEnergyCalibrationUniversal
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEnergyCalibrationUniversal::CreateExpos()
{
  // If they are already created, return
  if (m_Expos.size() != 0) return;
  
  // Set the histogram display
  m_ExpoEnergyCalibration = new MGUIExpoEnergyCalibration(this);
  m_ExpoEnergyCalibration->SetEnergyHistogramParameters(200, 0, 2000);
  m_Expos.push_back(m_ExpoEnergyCalibration);  
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibrationUniversal::Initialize()
{
  // Initialize the module 
  
  MParser Parser;
  if (Parser.Open(m_FileName, MFile::c_Read) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open calibration file "<<m_FileName<<endl;
    return false;
  }

  MParser Parser_Temp;
  if (m_TemperatureEnabled ==true) {
    if (Parser_Temp.Open(m_TempFileName, MFile::c_Read) == false) {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open Temperature calibration file "<<m_TempFileName<<endl;  
      return false;
    }
  }

  map<MReadOutElementDoubleStrip, unsigned int> CP_ROEToLine; //Peak fits
  map<MReadOutElementDoubleStrip, unsigned int> CM_ROEToLine; //Energy Calibration Model
  map<MReadOutElementDoubleStrip, unsigned int> CR_ROEToLine; //Energy Resolution Calibration Model
  map<MReadOutElementDoubleStrip, unsigned int> CT_ROEToLine; //Temperature Calibration Model


  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
    if (NTokens < 2) continue;
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "CP") == true ||
      Parser.GetTokenizerAt(i)->IsTokenAt(0, "CM") == true || Parser.GetTokenizerAt(i)->IsTokenAt(0,"CR") == true) {
      if (Parser.GetTokenizerAt(i)->IsTokenAt(1, "dss") == true) {
        
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
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Line parser: Unknown read-out element ("<<Parser.GetTokenizerAt(i)->GetTokenAt(1)<<")"<<endl;
        return false;
      }
    }
  }
  
  if (m_TemperatureEnabled) {
    for (unsigned int i = 0; i < Parser_Temp.GetNLines(); ++i) {
      unsigned int NTokens = Parser_Temp.GetTokenizerAt(i)->GetNTokens();
      if (NTokens < 2) continue;
      if (Parser_Temp.GetTokenizerAt(i)->IsTokenAt(0, "CT") == true) {
        if (Parser_Temp.GetTokenizerAt(i)->IsTokenAt(1, "dss") == true) {
          MReadOutElementDoubleStrip R;
          R.SetDetectorID(Parser_Temp.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2));
          R.SetStripID(Parser_Temp.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3));
          R.IsLowVoltageStrip(Parser_Temp.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(4) == 1);
          CT_ROEToLine[R] = i;
        }
      }
    }
  }



  for (auto CM: CM_ROEToLine) {
    // If we have at least three data points, we store the calibration
    
    if (CP_ROEToLine.find(CM.first) != CP_ROEToLine.end()) {
      unsigned int i = CP_ROEToLine[CM.first];
      if (Parser.GetTokenizerAt(i)->IsTokenAt(5, "pakw") == false) {
        if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": Unknown calibration point descriptor found: "<<Parser.GetTokenizerAt(i)->GetTokenAt(5)<<endl;
        continue;
      }
    } else {
      if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": No good calibration for the following strip found: "<<CM.first<<endl;
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
      TF1* melinatorfit = new TF1("poly1zero","0. + [0]*x", 0., 8191.);
      melinatorfit->FixParameter(0, a0);
     
      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_Calibration[CM.first] = melinatorfit;
     
    }     
        
    // Below inclusion of poly1 and poly2 written by J. Beechert on 2019/10/24
    else if (CalibratorType == "poly1") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      
      //From the fit parameters I just extracted from the .ecal file, I can define a function
      TF1* melinatorfit = new TF1("poly1","[0] + [1]*x", 0., 8191.);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);
      
      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_Calibration[CM.first] = melinatorfit;
      
    } else if (CalibratorType == "poly2") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      
      //From the fit parameters I just extracted from the .ecal file, I can define a function
      TF1* melinatorfit = new TF1("poly2","[0] + [1]*x + [2]*x^2", 0., 8191.);
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
      TF1* melinatorfit = new TF1("poly3","[0] + [1]*x + [2]*x^2 + [3]*x^3", 0., 8191.);
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
      TF1 * melinatorfit = new TF1("poly4","[0] + [1]*x + [2]*x^2 + [3]*x^3 + [4]*x^4", 0., 8191.);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);
      melinatorfit->FixParameter(2, a2);
      melinatorfit->FixParameter(3, a3);
      melinatorfit->FixParameter(4, a4);

      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_Calibration[CM.first] = melinatorfit;

    } else {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Line parser: Unknown calibrator type ("<<CalibratorType<<") for strip"<<CM.first<<endl;
      continue;
    }
  }
  
  for (auto CR: CR_ROEToLine) {

    unsigned int Pos = 5;
    MString CalibratorType = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsString(Pos);
    CalibratorType.ToLower();
    if (CalibratorType == "p1" || CalibratorType == "poly1") {
      double f0 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      double f1 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      TF1* resolutionfit = new TF1("P1","([0]+[1]*x) / 2.355",0.,2000.);
      resolutionfit->FixParameter(0,f0);
      resolutionfit->FixParameter(1,f1);

      m_ResolutionCalibration[CR.first] = resolutionfit;
    }
    else if (CalibratorType == "p2" || CalibratorType == "poly2") {
      double f0 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      double f1 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      double f2 = Parser.GetTokenizerAt(CR.second)->GetTokenAtAsDouble(++Pos);
      TF1* resolutionfit = new TF1("P2","([0]+[1]*x+[2]*x*x) / 2.355",0.,2000.);
      resolutionfit->FixParameter(0,f0);
      resolutionfit->FixParameter(1,f1);
      resolutionfit->FixParameter(2,f2);
      m_ResolutionCalibration[CR.first] = resolutionfit;
    } else {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Line parser: Unknown resolution calibrator type ("<<CalibratorType<<") for strip"<<CR.first<<endl;
      continue;
    }
  }


  if (m_TemperatureEnabled == true) {
    for (auto CT: CT_ROEToLine) {
      unsigned int Pos = 5;
      double f0 = Parser_Temp.GetTokenizerAt(CT.second)->GetTokenAtAsDouble(Pos);
      double f1 = Parser_Temp.GetTokenizerAt(CT.second)->GetTokenAtAsDouble(++Pos);
      TF1 * temperaturefit = new TF1("temperaturefit", "pol1", 0, 40);
      temperaturefit->FixParameter(0, f0);
      temperaturefit->FixParameter(1, f1);

      m_TemperatureCalibration[CT.first] = temperaturefit;
    }
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibrationUniversal::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level, i.e. takes the raw ADC value from the .roa file loaded through nuclearizer and converts it into energy units.
  
  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
    MStripHit* SH = Event->GetStripHit(i);
    MReadOutElementDoubleStrip R = *dynamic_cast<MReadOutElementDoubleStrip*>(SH->GetReadOutElement());
    
    TF1* Fit = m_Calibration[R];
    TF1* FitRes = m_ResolutionCalibration[R];
    double temp, ADCMod, newADC;

    if (Fit == nullptr) {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: Energy-fit not found for read-out element "<<R<<endl;
      Event->SetEnergyCalibrationIncomplete_BadStrip(true);
    } else {

      double Energy = 0;
      if (m_TemperatureEnabled == true) {
        TF1* FitTemp = m_TemperatureCalibration[R];
        if (FitTemp == nullptr) {
          if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: temp-fit not found for read-out element "<<R<<endl;
          Event->SetEnergyCalibrationIncomplete_BadStrip(true);
        } else {
          temp = SH->GetPreampTemp();
          ADCMod = FitTemp->Eval(temp);
          newADC = (SH->GetADCUnits())/ADCMod;
          SH->SetADCUnits(newADC);
        }
      } 
       
      Energy = Fit->Eval(SH->GetADCUnits());

      if (Energy < 0) {
        Energy = 0;
        if (SH->GetADCUnits() > 100) { // TODO: That's a remaining COSI-balloon hack...
          Event->SetEnergyCalibrationIncomplete(true);
        }
      }
      
      SH->SetEnergy(Energy);
      if (FitRes == nullptr) {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: Energy Resolution fit not found for read-out element "<<R<<endl;
        Event->SetEnergyResolutionCalibrationIncomplete(true);
      } else {
        double EnergyResolution = FitRes->Eval(Energy);
        SH->SetEnergyResolution(EnergyResolution);
      }
      if (R.IsLowVoltageStrip() == true) {
        if (HasExpos() == true) {
          m_ExpoEnergyCalibration->AddEnergy(Energy);
        }
      }
      
      if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Energy: "<<SH->GetADCUnits()<<" adu --> "<<Energy<<" keV"<<endl;
    } 
  }

  for (unsigned int i = 0; i < Event->GetNStripHits(); ) {
    MStripHit* SH = Event->GetStripHit(i);
    if (SH->GetEnergy() < 8) {
      Event->RemoveStripHit(i);
      delete SH;
    } else {
      ++i;
    }
  }


  Event->SetAnalysisProgress(MAssembly::c_EnergyCalibration);
  
  return true;
}


/////////////////////////////////////////////////////////////////////////////////


double MModuleEnergyCalibrationUniversal::GetEnergy(MReadOutElementDoubleStrip R, double ADC)
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
    cout<<m_Name<<": GetEnergy: Error unable to find calibration"<<endl;
    return 0;
  }

  return Energy;
}


///////////////////////////////////////////////////////////////////////////////


double MModuleEnergyCalibrationUniversal::GetADC(MReadOutElementDoubleStrip R, double Energy)
{
  //! Return the ADC value for a given energy

  TF1* Fit = m_Calibration[R];
  if (Fit != nullptr) {
    return Fit->GetX(Energy);
  } else {
    cout<<m_Name<<": GetADC: Error unable to find calibration"<<endl;
    return 0;
  }
}


///////////////////////////////////////////////////////////////////////////////


void MModuleEnergyCalibrationUniversal::Finalize()
{
  // Finalize the calibrator and clean up

  MModule::Finalize();

  for (auto& F: m_Calibration) delete F.second;
  for (auto& F: m_ResolutionCalibration) delete F.second;
  for (auto& F: m_TemperatureCalibration) delete F.second;

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEnergyCalibrationUniversal::ShowOptionsGUI()
{
  // Show the options GUI
  
  MGUIOptionsEnergyCalibrationUniversal* Options = new MGUIOptionsEnergyCalibrationUniversal(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEnergyCalibrationUniversal::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }

  MXmlNode* TempFileNameNode = Node->GetNode("TempFileName");
  if (TempFileNameNode != 0) {
    m_TempFileName = TempFileNameNode->GetValue();
  }
  
  MXmlNode* PreampTemperatureNode = Node->GetNode("PreampTemperature");
  if( PreampTemperatureNode != NULL ){
      m_TemperatureEnabled = (bool) PreampTemperatureNode->GetValueAsInt();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleEnergyCalibrationUniversal::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);
  new MXmlNode(Node, "TempFileName", m_TempFileName);
  new MXmlNode(Node, "PreampTemperature",(unsigned int) m_TemperatureEnabled);  

  return Node;
}

/////////////////////////////////////////////////////////////////////////////////


double MModuleEnergyCalibrationUniversal::LookupEnergyResolution(MStripHit* SH, double Energy)
{
  //! Return the energy resolution or -1 in case of error

  MReadOutElementDoubleStrip* ROE = dynamic_cast<MReadOutElementDoubleStrip*>(SH->GetReadOutElement());
  if (ROE == nullptr) {
    cout<<m_Name<<": LookupEnergyResolution: Error unable to get read-out element"<<endl;
    return -1;
  }
  TF1* FitRes = m_ResolutionCalibration[*ROE];
  if (FitRes == nullptr) {
    cout<<m_Name<<": LookupEnergyResolutio: Error: Couldn't locate energy resolution"<<endl;
    return -1.0;
  } else {
    return FitRes->Eval(Energy);
  }
}



// MModuleEnergyCalibrationUniversal.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
