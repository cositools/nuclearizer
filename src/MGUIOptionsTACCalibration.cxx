/*
 * MGUIOptionsTACCalibration.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Nicole Rodriguez Cavero
 * Sean Pike
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Sean Pike, Andreas Zoglauer, Nicole Rodriguez Cavero.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MGUIOptionsTACCalibration.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MString.h"
#include "MGUIEFileSelector.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIEEntry.h"

// Nuclearizer libs:
#include "MModuleTACCalibration.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsTACCalibration
)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTACCalibration::MGUIOptionsTACCalibration(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTACCalibration::~MGUIOptionsTACCalibration()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsTACCalibration::Create()
{
  PreCreate();

  m_TACCalFileSelector = new MGUIEFileSelector(
    m_OptionsFrame,
    "Select a TAC Calibration file:",
    dynamic_cast<MModuleTACCalibration*>(m_Module)->GetTACCalFileName()
  );

  m_TACCalFileSelector->SetFileType("TAC", "*.csv");

  TGLayoutHints* TACCalLayout =
    new TGLayoutHints(
      kLHintsTop | kLHintsCenterX | kLHintsExpandX,
      10, 10, 10, 10
    );

  m_OptionsFrame->AddFrame(m_TACCalFileSelector, TACCalLayout);

  TGLayoutHints* LabelLayout =
    new TGLayoutHints(
      kLHintsTop | kLHintsLeft,
      10, 10, 10, 5
    );

  TGLayoutHints* RBLayout =
    new TGLayoutHints(
      kLHintsTop | kLHintsLeft,
      20, 10, 5, 5
    );

  TGLayoutHints* RBOptionLayout =
    new TGLayoutHints(
      kLHintsTop | kLHintsLeft | kLHintsExpandX,
      40, 10, 5, 10
    );

  TGLabel* TACCutLabel =
  new TGLabel(m_OptionsFrame,
              "Please choose how to handle TAC cuts:");

  m_OptionsFrame->AddFrame(TACCutLabel, LabelLayout);

  m_TACCutRBIgnore =
  new TGRadioButton(
    m_OptionsFrame,
    "Do not apply TAC cuts",
    c_TACCutIgnore
  );

  m_TACCutRBIgnore->Associate(this);
  m_OptionsFrame->AddFrame(m_TACCutRBIgnore, RBLayout);

  m_TACCutRBApply =
  new TGRadioButton(
    m_OptionsFrame,
    "Apply TAC cuts",
    c_TACCutApply
  );

  m_TACCutRBApply->Associate(this);
  m_OptionsFrame->AddFrame(m_TACCutRBApply, RBLayout);

  m_CoincidenceWindow =
  new MGUIEEntry(
    m_OptionsFrame,
    "Set coincidence window [ns]:",
    false,
    dynamic_cast<MModuleTACCalibration*>(m_Module)->GetCoincidenceWindow(),
    true,
    0.0
  );

  m_OptionsFrame->AddFrame(
    m_CoincidenceWindow,
    RBOptionLayout
  );

  bool ApplyTACCuts = dynamic_cast<MModuleTACCalibration*>(m_Module)->GetApplyTACCuts();

  if (ApplyTACCuts == true) {
    ToggleRadioButtons(c_TACCutApply);
  } else {
    ToggleRadioButtons(c_TACCutIgnore);
  }

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsTACCalibration::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;

    case kCM_RADIOBUTTON:
      ToggleRadioButtons(Parameter1);
      break;
    
    case kCM_CHECKBUTTON:
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


bool MGUIOptionsTACCalibration::OnApply()
{
  // Store the data in the module
  dynamic_cast<MModuleTACCalibration*>(m_Module)->SetTACCalFileName(m_TACCalFileSelector->GetFileName());

  // Apply TAC cuts or not
  if (m_TACCutRBIgnore->GetState() == kButtonDown) {
    dynamic_cast<MModuleTACCalibration*>(m_Module)->
      SetApplyTACCuts(false);
  } else if (m_TACCutRBApply->GetState() == kButtonDown) {
    dynamic_cast<MModuleTACCalibration*>(m_Module)->
      SetApplyTACCuts(true);
  }

  // Coincidence window
  dynamic_cast<MModuleTACCalibration*>(m_Module)->
    SetCoincidenceWindow(
      m_CoincidenceWindow->GetAsDouble()
    );
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void MGUIOptionsTACCalibration::ToggleRadioButtons(int WidgetID)
{
  if (WidgetID == c_TACCutIgnore) {

    m_TACCutRBIgnore->SetState(kButtonDown);
    m_TACCutRBApply->SetState(kButtonUp);

    m_CoincidenceWindow->SetEnabled(false);

  } else if (WidgetID == c_TACCutApply) {

    m_TACCutRBIgnore->SetState(kButtonUp);
    m_TACCutRBApply->SetState(kButtonDown);

    m_CoincidenceWindow->SetEnabled(true);
  }
}


// MGUIOptionsTACCalibration: the end...
////////////////////////////////////////////////////////////////////////////////
