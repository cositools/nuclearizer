/*
 * MGUIOptionsEventFilter.cxx
 *
 *
 * Copyright (C) 2008-2010 by Jau-Shian Liang.
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
#include "MGUIOptionsEventFilter.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModule.h"
#include "MNCTModuleEventFilter.h"
#include "MGUIEFileSelector.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsEventFilter)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventFilter::MGUIOptionsEventFilter(MNCTModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventFilter::~MGUIOptionsEventFilter()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsEventFilter::Create()
{
  PreCreate();

  // Modify here

  TGHorizontalFrame* VetoSettingFrame = new TGHorizontalFrame(this, 200, 25);
  m_VetoSetting =  new TGTextEntry(VetoSettingFrame, "");
  m_VetoSetting->SetText(dynamic_cast<MNCTModuleEventFilter*>(m_Module)->GetVetoSetting());

  TGLabel* VetoSettingLabel = new TGLabel(VetoSettingFrame, "Veto detector list (seperated by white-space): ");

  TGLayoutHints* VetoSettingFrameLayout = new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 10, 10, 5, 5);
  TGLayoutHints* VetoSettingLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 1, 1, 2, 2);
  TGLayoutHints* VetoSettingLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 1, 1, 2, 2);

  AddFrame(VetoSettingFrame,VetoSettingFrameLayout);
  VetoSettingFrame->AddFrame(VetoSettingLabel,VetoSettingLabelLayout);
  VetoSettingFrame->AddFrame(m_VetoSetting, VetoSettingLayout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEventFilter::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsEventFilter::OnApply()
{
	// Modify this to store the data in the module!
  dynamic_cast<MNCTModuleEventFilter*>(m_Module)->SetVetoSetting(m_VetoSetting->GetText());

  return true;
}


// MGUIOptionsEventFilter: the end...
////////////////////////////////////////////////////////////////////////////////
