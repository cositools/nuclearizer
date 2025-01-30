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

  // Modify here

  TGLayoutHints* TACLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_TAC = new MGUIEMinMaxEntry(m_OptionsFrame, 
                                       "Choose the minimum and maximum TAC cut (in imaginary TAC units):", 
                                       false,
                                       dynamic_cast<MModuleTACcut
                                      *>(m_Module)->GetMinimumTAC(),
                                       dynamic_cast<MModuleTACcut
                                      *>(m_Module)->GetMaximumTAC(),
                                       true, 0.0);
  m_OptionsFrame->AddFrame(m_TAC, TACLayout);

  m_TACCalFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Select a TAC Calibration file:", 
    dynamic_cast<MModuleTACcut*>(m_Module)->GetTACCalFileName());
  m_TACCalFileSelector->SetFileType("TAC", "*.csv");
  TGLayoutHints* Label3Layout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_TACCalFileSelector, Label3Layout);


  // TGLabel* TACLabel = new TGLabel(m_OptionsFrame, 
  //   "This is a TAC cut and this text is here because.\n"
  //   "I'm not sure if I can remove it yet");
  // m_OptionsFrame->AddFrame(TACLabel, TACLayout);
  
  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsTACcut::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsTACcut::OnApply()
{
  // Store the data in the module

  dynamic_cast<MModuleTACcut
*>(m_Module)->SetMinimumTAC(m_TAC->GetMinValue());
  dynamic_cast<MModuleTACcut
*>(m_Module)->SetMaximumTAC(m_TAC->GetMaxValue());

  dynamic_cast<MModuleTACcut*>(m_Module)->SetTACCalFileName(m_TACCalFileSelector->GetFileName());


  return true;
}


// MGUIOptionsTACcut: the end...
////////////////////////////////////////////////////////////////////////////////
