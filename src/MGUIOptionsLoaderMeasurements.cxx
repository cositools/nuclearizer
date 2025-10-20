/*
 * MGUIOptionsLoaderMeasurements.cxx
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
#include "MGUIOptionsLoaderMeasurements.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleLoaderMeasurements.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsLoaderMeasurements)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderMeasurements::MGUIOptionsLoaderMeasurements(MModule* Module, MString FileType)
  : MGUIOptions(Module), m_FileType(FileType)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderMeasurements::~MGUIOptionsLoaderMeasurements()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsLoaderMeasurements::Create()
{
  PreCreate();

  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, MString("Please select a ") + m_FileType + " file:",
    dynamic_cast<MModuleLoaderMeasurements*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType(m_FileType + " file", MString("*.") + m_FileType);
  if (m_FileType == "roa") {
    m_FileSelector->SetFileType(m_FileType + " file", MString("*.") + m_FileType + ".gz");
  }
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsLoaderMeasurements::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsLoaderMeasurements::OnApply()
{
	// Modify this to store the data in the module!

  dynamic_cast<MModuleLoaderMeasurements*>(m_Module)->SetFileName(m_FileSelector->GetFileName());
	
	return true;
}


// MGUIOptionsLoaderMeasurements: the end...
////////////////////////////////////////////////////////////////////////////////
