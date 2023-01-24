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


MGUIOptionsLoaderMeasurements::MGUIOptionsLoaderMeasurements(MModule* Module) 
  : MGUIOptions(Module)
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

  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a data file:",
    dynamic_cast<MModuleLoaderMeasurements*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Roa file", "*.roa");
  m_FileSelector->SetFileType("Roa file", "*.roa.gz");
  m_FileSelector->SetFileType("Data file", "*.dat");
  m_FileSelector->SetFileType("Data file", "*.dat.gz");
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
