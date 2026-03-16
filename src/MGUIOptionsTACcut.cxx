/*
 * MGUIOptionsTACcut
.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Jau-Shian Liang.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MGUIOptionsTACcut.h"

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
#include "MModuleTACcut.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsTACcut
)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTACcut::MGUIOptionsTACcut(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTACcut::~MGUIOptionsTACcut()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsTACcut::Create()
{
  PreCreate();

  m_TACCalFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Select a TAC Calibration file:", dynamic_cast<MModuleTACcut*>(m_Module)->GetTACCalFileName());
  m_TACCalFileSelector->SetFileType("TAC", "*.csv");
  TGLayoutHints* TACCalLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_TACCalFileSelector, TACCalLayout);

  m_TACCutFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Select a TAC Cut file:", dynamic_cast<MModuleTACcut*>(m_Module)->GetTACCutFileName());
  m_TACCutFileSelector->SetFileType("TAC", "*.csv");
  TGLayoutHints* TACCutLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_TACCutFileSelector, TACCutLayout);
  
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 10, 10);
  TGLayoutHints* RBLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 40, 10, 2, 0);

  TGLabel* PlotSpectrumLabel = new TGLabel(m_OptionsFrame, "Please choose a spectrum plotting and memory option:");
  m_OptionsFrame->AddFrame(PlotSpectrumLabel, LabelLayout);
      
  m_PlotSpectrumNoneRB = new TGRadioButton(m_OptionsFrame, "Do not plot spectrum", c_PlotSpectrumNone);
  m_PlotSpectrumNoneRB->Associate(this);
  m_OptionsFrame->AddFrame(m_PlotSpectrumNoneRB, RBLayout);

  m_PlotSpectrumNoBufferRB = new TGRadioButton(m_OptionsFrame, "Plot spectrum without buffering data", c_PlotSpectrumNoBuffer);
  m_PlotSpectrumNoBufferRB->Associate(this);
  m_OptionsFrame->AddFrame(m_PlotSpectrumNoBufferRB, RBLayout);
      
  m_PlotSpectrumWithBufferRB = new TGRadioButton(m_OptionsFrame, "Plot spectrum with buffered data", c_PlotSpectrumWithBuffer);
  m_PlotSpectrumWithBufferRB->Associate(this);
  m_OptionsFrame->AddFrame(m_PlotSpectrumWithBufferRB, RBLayout);

  int PlotMode = static_cast<int>(dynamic_cast<MModuleTACcut*>(m_Module)->GetPlotSpectrumMode());
  ToggleRadioButtons(PlotMode);
  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsTACcut::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    case kCM_RADIOBUTTON:
      ToggleRadioButtons(Parameter1);
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

  return MGUIOptions::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsTACcut::OnApply()
{
  // Store the data in the module
  dynamic_cast<MModuleTACcut*>(m_Module)->SetTACCalFileName(m_TACCalFileSelector->GetFileName());
  dynamic_cast<MModuleTACcut*>(m_Module)->SetTACCutFileName(m_TACCutFileSelector->GetFileName());
  
  if (m_PlotSpectrumNoneRB->GetState() == kButtonDown) {
    dynamic_cast<MModuleTACcut*>(m_Module)->SetPlotSpectrumMode(MTACPlotSpectrumModes::e_PlotNone);
  } else if (m_PlotSpectrumNoBufferRB->GetState() == kButtonDown) {
    dynamic_cast<MModuleTACcut*>(m_Module)->SetPlotSpectrumMode(MTACPlotSpectrumModes::e_PlotNoBuffer);
  } else if (m_PlotSpectrumWithBufferRB->GetState() == kButtonDown) {
    dynamic_cast<MModuleTACcut*>(m_Module)->SetPlotSpectrumMode(MTACPlotSpectrumModes::e_PlotWithBuffer);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void MGUIOptionsTACcut::ToggleRadioButtons(int WidgetID)
{
  // Plot spectrum
  if (WidgetID == c_PlotSpectrumNone) {
    m_PlotSpectrumNoneRB->SetState(kButtonDown);
    m_PlotSpectrumNoBufferRB->SetState(kButtonUp);
    m_PlotSpectrumWithBufferRB->SetState(kButtonUp);
  } else if (WidgetID == c_PlotSpectrumNoBuffer) {
    m_PlotSpectrumNoneRB->SetState(kButtonUp);
    m_PlotSpectrumNoBufferRB->SetState(kButtonDown);
    m_PlotSpectrumWithBufferRB->SetState(kButtonUp);
  } else if (WidgetID == c_PlotSpectrumWithBuffer) {
    m_PlotSpectrumNoneRB->SetState(kButtonUp);
    m_PlotSpectrumNoBufferRB->SetState(kButtonUp);
    m_PlotSpectrumWithBufferRB->SetState(kButtonDown);
  }
}


// MGUIOptionsTACcut: the end...
////////////////////////////////////////////////////////////////////////////////
