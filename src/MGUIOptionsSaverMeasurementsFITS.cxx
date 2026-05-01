/*
 * MGUIOptionsSaverMeasurementsFITS.cxx
 *
 *
 * Copyright (C) by Andreas Zoglaue, WingYeung Ma.
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
#include "MGUIOptionsSaverMeasurementsFITS.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleSaverMeasurementsFITS.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsSaverMeasurementsFITS)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsSaverMeasurementsFITS::MGUIOptionsSaverMeasurementsFITS(MModule* Module)
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsSaverMeasurementsFITS::~MGUIOptionsSaverMeasurementsFITS()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsSaverMeasurementsFITS::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_FileSelectorFITS = new MGUIEFileSelector(m_OptionsFrame, "Please select output FITS file:",
    dynamic_cast<MModuleSaverMeasurementsFITS*>(m_Module)->GetFileName());
  m_FileSelectorFITS->SetFileType("FITS file", "*.fits");
  m_FileSelectorFITS->SetFileType("FITS file", "*.fit");
  m_OptionsFrame->AddFrame(m_FileSelectorFITS, LabelLayout);

  // Output level selector: L1b or L2
  TGLabel* LevelLabel = new TGLabel(m_OptionsFrame, "Output Level:");
  m_OptionsFrame->AddFrame(LevelLabel, LabelLayout);

  m_OutputLevelCombo = new TGComboBox(m_OptionsFrame);
  m_OutputLevelCombo->AddEntry("L1b (all events, with BAD_FLAG)", 0);
  m_OutputLevelCombo->AddEntry("L2 (screened, no BAD_FLAG)", 1);
  m_OutputLevelCombo->Select(dynamic_cast<MModuleSaverMeasurementsFITS*>(m_Module)->GetOutputLevel());
  m_OutputLevelCombo->Resize(300, 25);
  m_OptionsFrame->AddFrame(m_OutputLevelCombo, LabelLayout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsSaverMeasurementsFITS::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
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


bool MGUIOptionsSaverMeasurementsFITS::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleSaverMeasurementsFITS*>(m_Module)->SetFileName(m_FileSelectorFITS->GetFileName());
  dynamic_cast<MModuleSaverMeasurementsFITS*>(m_Module)->SetOutputLevel(m_OutputLevelCombo->GetSelected());

  return true;
}


// MGUIOptionsSaverMeasurementsFITS: the end...
////////////////////////////////////////////////////////////////////////////////
