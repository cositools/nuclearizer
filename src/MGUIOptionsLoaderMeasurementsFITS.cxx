/*
 * MGUIOptionsLoaderMeasurementsFITS.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
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
#include "MGUIOptionsLoaderMeasurementsFITS.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleLoaderMeasurementsFITS.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsLoaderMeasurementsFITS)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderMeasurementsFITS::MGUIOptionsLoaderMeasurementsFITS(MModule* Module)
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderMeasurementsFITS::~MGUIOptionsLoaderMeasurementsFITS()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsLoaderMeasurementsFITS::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_FileSelectorFITS = new MGUIEFileSelector(m_OptionsFrame, "Please select a FITS file:",
    dynamic_cast<MModuleLoaderMeasurementsFITS*>(m_Module)->GetFileName());
  m_FileSelectorFITS->SetFileType("FITS file", "*.fits");
  m_FileSelectorFITS->SetFileType("FITS file", "*.fit");
  m_OptionsFrame->AddFrame(m_FileSelectorFITS, LabelLayout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsLoaderMeasurementsFITS::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsLoaderMeasurementsFITS::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleLoaderMeasurementsFITS*>(m_Module)->SetFileName(m_FileSelectorFITS->GetFileName());

  return true;
}


// MGUIOptionsLoaderMeasurementsFITS: the end...
////////////////////////////////////////////////////////////////////////////////
