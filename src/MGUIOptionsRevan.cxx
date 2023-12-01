/*
 * MGUIOptionsRevan.cxx
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
#include "MGUIOptionsRevan.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleRevan.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsRevan)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsRevan::MGUIOptionsRevan(MModule* Module)
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsRevan::~MGUIOptionsRevan()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsRevan::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);  

  m_RevanCfgFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a revan configuration file:",
  dynamic_cast<MModuleRevan*>(m_Module)->GetRevanConfigurationFileName());
  m_RevanCfgFileSelector->SetFileType("revan configuration", "*.revan.cfg");
  m_OptionsFrame->AddFrame(m_RevanCfgFileSelector, LabelLayout);
  
  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsRevan::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsRevan::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleRevan*>(m_Module)->SetRevanConfigurationFileName(m_RevanCfgFileSelector->GetFileName());
  
  return true;
}


// MGUIOptionsRevan: the end...
////////////////////////////////////////////////////////////////////////////////
