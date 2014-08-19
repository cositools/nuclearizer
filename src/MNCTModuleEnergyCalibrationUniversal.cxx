/*
 * MNCTModuleEnergyCalibrationUniversal.cxx
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
// MNCTModuleEnergyCalibrationUniversal
//
////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <fstream>
#include <iostream>
#include <map>
using namespace std;

// Include the header:
#include "MNCTModuleEnergyCalibrationUniversal.h"


// ROOT libs:
#include "TGClient.h"
#include "TFile.h"

// MEGAlib libs:
#include "MString.h"
#include "MStreams.h"

// Nuclearizer libs:
#include "MNCTModule.h"
#include "MReadOutElement.h"
#include "MReadOutElementDoubleStrip.h"
#include "MCalibratorEnergyPointwiseLinear.h"
#include "MGUIOptionsEnergyCalibrationUniversal.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleEnergyCalibrationUniversal)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEnergyCalibrationUniversal::MNCTModuleEnergyCalibrationUniversal() : MNCTModule()
{
  // Construct an instance of MNCTModuleEnergyCalibrationUniversal

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Universal energy calibrator";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "EnergyCalibrationUniversal";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(c_EventLoader);
  //AddPreceedingModuleType(c_DetectorEffectsEngine);
  //AddPreceedingModuleType(c_EnergyCalibration);
  //AddPreceedingModuleType(c_ChargeSharingCorrection);
  //AddPreceedingModuleType(c_DepthCorrection);
  //AddPreceedingModuleType(c_StripPairing);
  //AddPreceedingModuleType(c_EventReconstruction);

  // Set all types this modules handles
  //AddModuleType(c_DetectorEffectsEngine);
  AddModuleType(c_EnergyCalibration);
  //AddModuleType(c_ChargeSharingCorrection);
  //AddModuleType(c_DepthCorrection);
  //AddModuleType(c_StripPairing);
  //AddModuleType(c_EventReconstruction);

  // Set all modules, which can follow this module
  //AddSucceedingModuleType(c_DetectorEffectsEngine);
  //AddSucceedingModuleType(c_EnergyCalibration);
  AddSucceedingModuleType(c_CrosstalkCorrection);
  AddSucceedingModuleType(c_ChargeSharingCorrection);
  AddSucceedingModuleType(c_DepthCorrection);
  AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_Aspect);
  AddSucceedingModuleType(c_Else);
  AddSucceedingModuleType(c_EventSaver);
  AddSucceedingModuleType(c_EventReconstruction);

  // Set if this module has an options GUI
  m_HasOptionsGUI = true;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEnergyCalibrationUniversal::~MNCTModuleEnergyCalibrationUniversal()
{
  // Delete this instance of MNCTModuleEnergyCalibrationUniversal
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEnergyCalibrationUniversal::Initialize()
{
  // Initialize the module 
  
  MParser Parser;
  if (Parser.Open(m_FileName, MFile::c_Read) == false) {
    mout<<"Unable to open file "<<m_FileName<<endl;
    return false;
  }
  
  
  map<MReadOutElementDoubleStrip, unsigned int> CP_ROEToLine;
  map<MReadOutElementDoubleStrip, unsigned int> CM_ROEToLine;
  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
    if (NTokens < 2) continue;
    if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "CP") == true ||
      Parser.GetTokenizerAt(i)->IsTokenAt(0, "CM") == true) {
      if (Parser.GetTokenizerAt(i)->IsTokenAt(1, "dss") == true) {
        
        MReadOutElementDoubleStrip R;
        R.SetDetectorID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(2));
        R.SetStripID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3));
        R.IsPositiveStrip(Parser.GetTokenizerAt(i)->GetTokenAtAsString(4) == "p");
        if (Parser.GetTokenizerAt(i)->IsTokenAt(0, "CP") == true) {
          CP_ROEToLine[R] = i;
        } else {
          CM_ROEToLine[R] = i;
        }
      } else {
        cout<<m_XmlTag<<": Line parser: Unknown read out element ("<<Parser.GetTokenizerAt(i)->GetTokenAt(1)<<")"<<endl;
        return false;
      }
    }
  }
  
  
  for (auto CM: CM_ROEToLine) {
    // If we have at least three data points, we store the calibration
    
    if (CP_ROEToLine.find(CM.first) != CP_ROEToLine.end()) {
      unsigned int i = CP_ROEToLine[CM.first];
      if (Parser.GetTokenizerAt(i)->IsTokenAt(5, "pakw") == true) {
        if (Parser.GetTokenizerAt(i)->GetTokenAtAsInt(6) < 3) {
          cout<<m_XmlTag<<": Not enough calibration points (only "<<Parser.GetTokenizerAt(i)->GetTokenAtAsInt(6)<<") for strip: "<<CM.first<<endl;
          continue;
        }
      } else {
        cout<<m_XmlTag<<": Unknown calibration point descriptor found: "<<Parser.GetTokenizerAt(i)->GetTokenAt(5)<<endl;
        continue;
      }
    } else {
      cout<<m_XmlTag<<": No good calibration for the following strip found: "<<CM.first<<endl;
      continue;
    }
    
    // Read the calibrator, i.e. read the fit function from the .ecal Melinator file.
    
    unsigned int Pos = 5;
    MString CalibratorType = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsString(Pos);
    CalibratorType.ToLower();
    
    //Eventually, I'll be including other possible fits, but for now, let's just get this working with poly3...
    if (CalibratorType == "poly3") {
      double a0 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a1 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a2 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      double a3 = Parser.GetTokenizerAt(CM.second)->GetTokenAtAsDouble(++Pos);
      
      //From the fit parameters I just extracted from the .ecal file, I can define a function
      TF1* melinatorfit = new TF1("poly3","[0]+[1]*x+[2]*x^2+[3]*x^3",0.,8000.);
      melinatorfit->FixParameter(0, a0);
      melinatorfit->FixParameter(1, a1);
      melinatorfit->FixParameter(2, a2);
      melinatorfit->FixParameter(3, a3);
      
      //Define the map by saving the fit function I just created as a map to the current ReadOutElement
      m_Calibration[CM.first] = melinatorfit;;
      
    } else {
      cout<<m_XmlTag<<": Line parser: Unknown calibrator type ("<<CalibratorType<<") for strip"<<CM.first<<endl;
      continue;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEnergyCalibrationUniversal::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level, i.e. takes the raw ADC value from the .roa file loaded through nuclearizer and converts it into energy units.

  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
    MNCTStripHit* SH = Event->GetStripHit(i);
    MReadOutElement* R = SH->GetReadOutElement();

    TF1* Fit = m_Calibration[*dynamic_cast<MReadOutElementDoubleStrip*>(R)];
    if (Fit == 0) {
      cout<<"Error: Fit not found for read-out element "<<*R<<endl;
      Event->SetEnergyCalibrationIncomplete(true);
    } else {
      double Energy = Fit->Eval(SH->GetADCUnits());
      if (Energy < 0 && SH->GetADCUnits() > 100) {
	Event->SetEnergyCalibrationIncomplete(true);
	Energy = 0;
      } else if (Energy < 0) {
	Energy = 0;
      }
      SH->SetEnergy(Energy);
      cout<<"Energy: "<<SH->GetADCUnits()<<" --> "<<Energy<<endl;
    }
  } 

  Event->SetEnergyCalibrated(true);
  return true;
}



////////////////////////////////////////////////////////////////////////////////


void MNCTModuleEnergyCalibrationUniversal::ShowOptionsGUI()
{
  // Show the options GUI

  MGUIOptionsEnergyCalibrationUniversal* Options = new MGUIOptionsEnergyCalibrationUniversal(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEnergyCalibrationUniversal::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleEnergyCalibrationUniversal::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);
  
  return Node;
}




// MNCTModuleEnergyCalibrationUniversal.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
