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

  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select an energy calibration file:",
    dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Energy calibration file", "*.ecal");
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

  m_TempModeCB = new TGCheckButton(m_OptionsFrame, "Enable preamp temperature correction and read calibration from file:", c_TempFile);
  m_TempModeCB->SetState((dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetPreampTempCorrection() == 1) ?  kButtonDown : kButtonUp);
  m_TempModeCB->Associate(this);
  m_OptionsFrame->AddFrame(m_TempModeCB, LabelLayout);

  m_UseTempCal = dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetPreampTempCorrection();

  TGLayoutHints* FileLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX, m_FontScaler*65 + 21*m_FontScaler, m_FontScaler*65, 0, 2*m_FontScaler);

  m_TempFile = new MGUIEFileSelector(m_OptionsFrame, "", dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetTempFileName());
  m_TempFile->SetFileType("Temperature calibration file", "*.txt");
  m_OptionsFrame->AddFrame(m_TempFile, FileLabelLayout);

  

  if (m_UseTempCal) {
    m_TempFile->SetEnabled(true);
  } else {
    m_TempFile->SetEnabled(false);
  }

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEnergyCalibrationUniversal::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    case kCM_CHECKBUTTON:
      switch (Parameter1) {
        case c_TempFile:
          if (m_TempModeCB->GetState() == kButtonDown) {
            m_UseTempCal = 1;
            m_TempFile->SetEnabled(true);
          } else if (m_TempModeCB->GetState() == kButtonUp) {
            m_UseTempCal = 0;
            m_TempFile->SetEnabled(false);
          }
          break;
      }
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
  dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetTempFileName(m_TempFile->GetFileName());

  if (dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetPreampTempCorrection() != m_UseTempCal) dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->EnablePreampTempCorrection(m_UseTempCal);

  return true;
}


// MGUIOptionsEnergyCalibrationUniversal: the end...
////////////////////////////////////////////////////////////////////////////////
