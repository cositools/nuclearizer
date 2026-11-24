/*
 * MGUIOptionsLoaderMeasurementsL0.cxx
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
#include "MGUIOptionsLoaderMeasurementsL0.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleLoaderMeasurementsL0.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsLoaderMeasurementsL0)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderMeasurementsL0::MGUIOptionsLoaderMeasurementsL0(MModule* Module)
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderMeasurementsL0::~MGUIOptionsLoaderMeasurementsL0()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsLoaderMeasurementsL0::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_FileSelectorL0 = new MGUIEFileSelector(m_OptionsFrame, "Please select an L0 binary file:",
    dynamic_cast<MModuleLoaderMeasurementsL0*>(m_Module)->GetFileName());
  m_FileSelectorL0->SetFileType("L0 binary file", "*.bin");
  m_FileSelectorL0->SetFileType("L0 binary file", "*.dat");
  m_OptionsFrame->AddFrame(m_FileSelectorL0, LabelLayout);

  m_FileSelectorStripMap = new MGUIEFileSelector(m_OptionsFrame, "Please select a strip map file:",
    dynamic_cast<MModuleLoaderMeasurementsL0*>(m_Module)->GetFileNameStripMap());
  m_FileSelectorStripMap->SetFileType("Strip map file", "*.map");
  m_OptionsFrame->AddFrame(m_FileSelectorStripMap, LabelLayout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsLoaderMeasurementsL0::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsLoaderMeasurementsL0::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleLoaderMeasurementsL0*>(m_Module)->SetFileName(m_FileSelectorL0->GetFileName());
  dynamic_cast<MModuleLoaderMeasurementsL0*>(m_Module)->SetFileNameStripMap(m_FileSelectorStripMap->GetFileName());

  return true;
}


// MGUIOptionsLoaderMeasurementsL0: the end...
////////////////////////////////////////////////////////////////////////////////
