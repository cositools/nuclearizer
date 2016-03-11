/*
 * MGUIOptionsMeasurementLoader.cxx
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
#include "MGUIOptionsMeasurementLoader.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModuleMeasurementLoader.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsMeasurementLoader)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsMeasurementLoader::MGUIOptionsMeasurementLoader(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsMeasurementLoader::~MGUIOptionsMeasurementLoader()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsMeasurementLoader::Create()
{
  PreCreate();

  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a data file:",
    dynamic_cast<MNCTModuleMeasurementLoader*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Roa file", "*.roa");
  m_FileSelector->SetFileType("Roa file", "*.roa.gz");
  m_FileSelector->SetFileType("Data file", "*.dat");
  m_FileSelector->SetFileType("Data file", "*.dat.gz");
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsMeasurementLoader::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsMeasurementLoader::OnApply()
{
	// Modify this to store the data in the module!

  dynamic_cast<MNCTModuleMeasurementLoader*>(m_Module)->SetFileName(m_FileSelector->GetFileName());
	
	return true;
}


// MGUIOptionsMeasurementLoader: the end...
////////////////////////////////////////////////////////////////////////////////
