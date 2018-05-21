/*
 * MGUIOptionsEventReconstruction.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
#include "MGUIOptionsEventReconstruction.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModule.h"
#include "MNCTModuleEventReconstruction.h"
#include "MGUIEFileSelector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsEventReconstruction)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventReconstruction::MGUIOptionsEventReconstruction(MNCTModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventReconstruction::~MGUIOptionsEventReconstruction()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsEventReconstruction::Create()
{
  PreCreate();

  // Modify here

  m_FileSelector = 
    new MGUIEFileSelector(this, "Please select a revan configuration file:", 
                          dynamic_cast<MNCTModuleEventReconstruction*>(m_Module)->GetRevanConfigurationFileName());
  m_FileSelector->SetFileType("Configuration file", "*.cfg");
  TGLayoutHints* Layout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  AddFrame(m_FileSelector, Layout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEventReconstruction::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsEventReconstruction::OnApply()
{
	// Modify this to store the data in the module!

  dynamic_cast<MNCTModuleEventReconstruction*>(m_Module)->SetRevanConfigurationFileName(m_FileSelector->GetFileName());

	return true;
}


// MGUIOptionsEventReconstruction: the end...
////////////////////////////////////////////////////////////////////////////////
