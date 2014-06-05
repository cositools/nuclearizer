/*
 * MNCTModuleEnergyCalibrationUniversal.cxx
 *
 *
 * Copyright (C) 2008-2009 by Mark Bandstra.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Mark Bandstra.
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


// Include the header:
#include "MNCTModuleEnergyCalibrationUniversal.h"

// Standard libs:
#include <fstream>
#include <iostream>
using namespace std;

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

  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
    unsigned int Pos = 0;
    unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
    if (NTokens == 0) continue;
    if (Parser.GetTokenizerAt(i)->IsTokenAt(Pos, "CP") == false) continue;
    
    // Only CP - calibration points at this stage

    // Read the read out element
    ++Pos;
    if (NTokens <= 4) {
      cout<<m_XmlTag<<": "<<Parser.GetTokenizerAt(i)->GetText()<<endl
          <<"Line parser: Not enough tokens for a calibration point (failed at read-out element)"<<endl;
      return false;
    }
    MReadOutElement* ReadOutElement = 0;
    MString ReadOutElementString = Parser.GetTokenizerAt(i)->GetTokenAtAsString(Pos);
    ReadOutElementString.ToLower();
    if (ReadOutElementString == "doublestrip") {
      ++Pos;
      if (Pos+3 > NTokens) {
        cout<<m_XmlTag<<": "<<Parser.GetTokenizerAt(i)->GetText()<<endl
            <<"Line parser: Not enough tokens for a calibration point (failed at doublestrip)"<<endl;
        return false;
      }
      MReadOutElementDoubleStrip* R = new MReadOutElementDoubleStrip();
      R->SetDetectorID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(Pos));
      ++Pos;
      R->IsPositiveStrip(Parser.GetTokenizerAt(i)->GetTokenAtAsString(Pos) == "p");
      ++Pos;
      R->SetStripID(Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(Pos));
      ReadOutElement = R;
    } else {
      cout<<m_XmlTag<<": "<<Parser.GetTokenizerAt(i)->GetText()<<endl
          <<"Line parser: Unknown read out element: "<<ReadOutElementString<<endl;
      return false;
    }
    
    // Check if we already have a calibrator for this readout element
    MCalibratorEnergy* Calibrator = 0;
    unsigned int DetectorID = ReadOutElement->GetDetectorID();
    unsigned int DetectorIndex = g_UnsignedIntNotDefined;
    for (unsigned int d = 0; d < m_DetectorIDs.size(); ++d) {
      if (m_DetectorIDs[d] == DetectorID) {
        DetectorIndex = d;
        break;
      }
    }
    if (DetectorIndex != g_UnsignedIntNotDefined) {
      for (unsigned int c = 0; c < m_Calibrators[DetectorIndex].size(); ++c) {
        if (*ReadOutElement == *(m_Calibrators[DetectorIndex][c]->GetReadOutElement())) {
          Calibrator = m_Calibrators[DetectorIndex][c];
          // We don't need the read-out element any more
          delete ReadOutElement;
          if (Calibrator->HasMultipleEntries() == false) {
            cout<<m_XmlTag<<": "<<Parser.GetTokenizerAt(i)->GetText()<<endl
                <<"Line parser: This calibrator does not allow multiple entries!"<<endl;
            return false;
          }
        }
      }
    }
    
    
    // Read the calibrator
    ++Pos;
    if (Pos+2 > NTokens) {
      cout<<m_XmlTag<<": "<<Parser.GetTokenizerAt(i)->GetText()<<endl
          <<"Line parser: Not enough tokens for this calibration point (failed at calibrator)"<<endl;
      return false;
    }
    MString CalibratorType = Parser.GetTokenizerAt(i)->GetTokenAtAsString(Pos);
    CalibratorType.ToLower();
    if (CalibratorType == "pointwiselinear") {
      MCalibratorEnergyPointwiseLinear* PWL = 0;
      if (Calibrator == 0) {
        PWL = new MCalibratorEnergyPointwiseLinear();
        PWL->SetReadOutElement(ReadOutElement); // PWL owns read-out element now
      } else {
        PWL = dynamic_cast<MCalibratorEnergyPointwiseLinear*>(Calibrator);
      }
      ++Pos;
      if (Pos+2 > NTokens) {
        cout<<m_XmlTag<<": "<<Parser.GetTokenizerAt(i)->GetText()<<endl
            <<"Line parser: Not enough tokens for this calibration point (pointwiselinear)"<<endl;
        return false;
      }
      double ADC = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(Pos);
      ++Pos;
      double Energy = Parser.GetTokenizerAt(i)->GetTokenAtAsDouble(Pos);      
      PWL->Add(ADC, Energy);
      if (Calibrator == 0) {
        Calibrator = PWL;
        if (DetectorIndex != g_UnsignedIntNotDefined) {
          m_Calibrators[DetectorIndex].push_back(PWL);
        } else {
          m_DetectorIDs.push_back(PWL->GetReadOutElement()->GetDetectorID());
          vector<MCalibratorEnergy*> v;
          v.push_back(PWL);
          m_Calibrators.push_back(v);
        }
      }
    } else {
      cout<<m_XmlTag<<": "<<Parser.GetTokenizerAt(i)->GetText()<<endl
          <<"Line parser: Unknown calibrator type: "<<CalibratorType<<endl;
      return false;
    }
    
  }
  
  // List the calibrators:
  for (unsigned int d = 0; d < m_Calibrators.size(); ++d) {
    for (unsigned int s = 0; s < m_Calibrators[d].size(); ++s) {
      cout<<m_Calibrators[d][s]->ToString()<<endl; 
    }
  }
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEnergyCalibrationUniversal::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
    MNCTStripHit* SH = Event->GetStripHit(i);
    //if (SH->HasTriggered() == false) continue;
    MReadOutElement* R = SH->GetReadOutElement();
    unsigned int DetectorID = R->GetDetectorID();
    unsigned int DetectorIndex = g_UnsignedIntNotDefined;
    for (unsigned int d = 0; d < m_DetectorIDs.size(); ++d) {
      if (m_DetectorIDs[d] == DetectorID) {
        DetectorIndex = d;
        break;
      }
    }
    if (DetectorIndex != g_UnsignedIntNotDefined) {
      for (unsigned int c = 0; c < m_Calibrators[DetectorIndex].size(); ++c) {
        if (*R == *(m_Calibrators[DetectorIndex][c]->GetReadOutElement())) {
          double Energy = m_Calibrators[DetectorIndex][c]->GetEnergy(SH->GetADCUnits());
          if (Energy < 0) Energy = 0;
          SH->SetEnergy(Energy);
          //cout<<m_Calibrators[DetectorIndex][c]->ToString()<<endl;
          //cout<<"Energy: "<<SH->GetADCUnits()<<" --> "<<Energy<<endl;
        }
      }
    } else {
      cout<<"Read-out element ("<<R->ToString()<<") is not calibrated"<<endl; 
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
