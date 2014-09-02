/*
 * MGUIOptionsEventSaver.cxx
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
#include "MGUIOptionsEventSaver.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModuleEventSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsEventSaver)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventSaver::MGUIOptionsEventSaver(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventSaver::~MGUIOptionsEventSaver()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsEventSaver::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);  
  
  m_Mode = new MGUIERBList(m_OptionsFrame, "Please select an output mode:");
  m_Mode->Add("*.dat file containing all information");
  m_Mode->Add("*.evta file to use with revan");
  m_Mode->SetSelected(dynamic_cast<MNCTModuleEventSaver*>(m_Module)->GetMode());
  m_Mode->Create();
  m_OptionsFrame->AddFrame(m_Mode, LabelLayout);

  
  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select an output file:",
  dynamic_cast<MNCTModuleEventSaver*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Dat file (all info)", "*.dat");
  m_FileSelector->SetFileType("evta file (evta file)", "*.evta");
  m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

  m_SaveBadEvents = new TGCheckButton(m_OptionsFrame, "Save events which are flagged bad (BD)", 1);
  m_SaveBadEvents->SetOn(dynamic_cast<MNCTModuleEventSaver*>(m_Module)->GetSaveBadEvents());
  m_OptionsFrame->AddFrame(m_SaveBadEvents, LabelLayout);
 
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEventSaver::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsEventSaver::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MNCTModuleEventSaver*>(m_Module)->SetMode(m_Mode->GetSelected());
  dynamic_cast<MNCTModuleEventSaver*>(m_Module)->SetFileName(m_FileSelector->GetFileName());
  dynamic_cast<MNCTModuleEventSaver*>(m_Module)->SetSaveBadEvents(m_SaveBadEvents->IsOn());
  
  return true;
}


// MGUIOptionsEventSaver: the end...
////////////////////////////////////////////////////////////////////////////////
