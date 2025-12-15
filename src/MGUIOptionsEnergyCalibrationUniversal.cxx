/*
 * MGUIOptionsEnergyCalibrationUniversal.cxx
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


// Include the header:
#include "MGUIOptionsEnergyCalibrationUniversal.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModule.h"
#include "MModuleEnergyCalibrationUniversal.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsEnergyCalibrationUniversal)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEnergyCalibrationUniversal::MGUIOptionsEnergyCalibrationUniversal(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEnergyCalibrationUniversal::~MGUIOptionsEnergyCalibrationUniversal()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsEnergyCalibrationUniversal::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  
  //file loader for energy calibration file
  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select an energy calibration file:",
    dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Energy calibration file", "*.ecal");
  m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

  //Add button and file loader option for threshold calibration file 
  m_ThresholdFileCB = new TGCheckButton(m_OptionsFrame, "Enable slow threshold cut determined from file (unique to each strip):", c_ThresholdFile);
  m_ThresholdFileCB->SetState((dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetThresholdFileEnable() == 1) ?  kButtonDown : kButtonUp);
  m_ThresholdFileCB->Associate(this);
  m_OptionsFrame->AddFrame(m_ThresholdFileCB, LabelLayout);

  m_UseThresholdFile = dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetThresholdFileEnable();

  TGLayoutHints* FileLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX, m_FontScaler*65 + 21*m_FontScaler, m_FontScaler*65, 0, 2*m_FontScaler);

  m_ThresholdFile = new MGUIEFileSelector(m_OptionsFrame, "", dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetThresholdFileName());
  m_ThresholdFile->SetFileType("Threshold per strip file", "*.csv");
  m_OptionsFrame->AddFrame(m_ThresholdFile, FileLabelLayout);

  if (m_UseThresholdFile) {
    m_ThresholdFile->SetEnabled(true);
  } else {
    m_ThresholdFile->SetEnabled(false);
  }
    
  // Add button and value option for user-input threshold
  m_ThresholdValueCB = new TGCheckButton(m_OptionsFrame, "Enable slow threshold cut for all strips:", c_Threshold);
  m_ThresholdValueCB->Associate(this);
  m_ThresholdValueCB->SetOn(dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetThresholdValueEnable());
  m_OptionsFrame->AddFrame(m_ThresholdValueCB, LabelLayout);
  
  m_UseThresholdValue = dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetThresholdValueEnable();

  m_SetThresholdValue = new MGUIEEntry(m_OptionsFrame, "                                                                                                                  Set threshold value [keV]", false,
                                      dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetThresholdValue(), true, 35.0);
  if (m_ThresholdValueCB->IsOn() == false) m_SetThresholdValue->SetEnabled(false);
  m_OptionsFrame->AddFrame(m_SetThresholdValue, LabelLayout);
    

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEnergyCalibrationUniversal::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  // TODO :: only allow one check box option at a time, either value or file
  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    case kCM_CHECKBUTTON:
        switch (Parameter1) {
          case c_ThresholdFile:
            if (m_ThresholdFileCB->GetState() == kButtonDown) {
              m_UseThresholdFile = 1;
              m_ThresholdFile->SetEnabled(true);
            } else if (m_ThresholdFileCB->GetState() == kButtonUp) {
              m_UseThresholdFile = 0;
              m_ThresholdFile->SetEnabled(false);
            }
            break;
          case c_Threshold:
            m_SetThresholdValue->SetEnabled(m_ThresholdValueCB->IsOn());
            break;
        }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  
  if (Status == false) {
    return false;
  }
    
  
  // Call also base class
  return MGUIOptions::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEnergyCalibrationUniversal::OnApply()
{
 // Modify this to store the data in the module!

  dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetFileName(m_FileSelector->GetFileName());

  dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetThresholdFileName(m_ThresholdFile->GetFileName());
    
  if (dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetThresholdFileEnable() != m_UseThresholdFile) dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetThresholdFileEnable(m_UseThresholdFile);
  
  if (dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetThresholdValueEnable() != m_UseThresholdValue) dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetThresholdValueEnable(m_UseThresholdValue);

  dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetThresholdValueEnable(m_ThresholdValueCB->IsOn());
  dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetThresholdValue(m_SetThresholdValue->GetAsDouble());

  return true;
}


// MGUIOptionsEnergyCalibrationUniversal: the end...
////////////////////////////////////////////////////////////////////////////////
