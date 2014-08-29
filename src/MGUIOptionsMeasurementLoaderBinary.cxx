/*
 * MGUIOptionsMeasurementLoaderBinary.cxx
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
#include "MGUIOptionsMeasurementLoaderBinary.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModule.h"
#include "MNCTModuleMeasurementLoaderBinary.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsMeasurementLoaderBinary)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsMeasurementLoaderBinary::MGUIOptionsMeasurementLoaderBinary(MNCTModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsMeasurementLoaderBinary::~MGUIOptionsMeasurementLoaderBinary()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsMeasurementLoaderBinary::Create()
{
  PreCreate();

  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a data file:",
    dynamic_cast<MNCTModuleMeasurementLoaderBinary*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Bin file", "*.bin");
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsMeasurementLoaderBinary::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsMeasurementLoaderBinary::OnApply()
{
	// Modify this to store the data in the module!

  dynamic_cast<MNCTModuleMeasurementLoaderBinary*>(m_Module)->SetFileName(m_FileSelector->GetFileName());
	
	return true;
}


// MGUIOptionsMeasurementLoaderBinary: the end...
////////////////////////////////////////////////////////////////////////////////
