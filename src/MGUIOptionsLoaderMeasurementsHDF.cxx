/*
 * MGUIOptionsLoaderMeasurementsHDF.cxx
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
#include "MGUIOptionsLoaderMeasurementsHDF.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleLoaderMeasurementsHDF.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsLoaderMeasurementsHDF)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderMeasurementsHDF::MGUIOptionsLoaderMeasurementsHDF(MModule* Module)
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderMeasurementsHDF::~MGUIOptionsLoaderMeasurementsHDF()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsLoaderMeasurementsHDF::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_FileSelectorHDF = new MGUIEFileSelector(m_OptionsFrame, "Please select a HDF5 file:",
    dynamic_cast<MModuleLoaderMeasurementsHDF*>(m_Module)->GetFileName());
  m_FileSelectorHDF->SetFileType("HDF5 file", "*.hdf5");
  m_FileSelectorHDF->SetFileType("HDF5 file", "*.hdf");
  m_OptionsFrame->AddFrame(m_FileSelectorHDF, LabelLayout);


  m_LoadContinuationFiles = new TGCheckButton(m_OptionsFrame, "Enable loading continuation HDF5 files", 1);
  m_LoadContinuationFiles->SetOn(dynamic_cast<MModuleLoaderMeasurementsHDF*>(m_Module)->GetLoadContinuationFiles());
  m_LoadContinuationFiles->Associate(this);
  m_OptionsFrame->AddFrame(m_LoadContinuationFiles, LabelLayout);


  m_FileSelectorStripMap = new MGUIEFileSelector(m_OptionsFrame, "Please select a strip map file:",
    dynamic_cast<MModuleLoaderMeasurementsHDF*>(m_Module)->GetFileNameStripMap());
  m_FileSelectorStripMap->SetFileType("Strip map file", "*.map");
  m_OptionsFrame->AddFrame(m_FileSelectorStripMap, LabelLayout);


  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsLoaderMeasurementsHDF::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsLoaderMeasurementsHDF::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleLoaderMeasurementsHDF*>(m_Module)->SetFileName(m_FileSelectorHDF->GetFileName());
  dynamic_cast<MModuleLoaderMeasurementsHDF*>(m_Module)->SetLoadContinuationFiles(m_LoadContinuationFiles->IsOn());
  dynamic_cast<MModuleLoaderMeasurementsHDF*>(m_Module)->SetFileNameStripMap(m_FileSelectorStripMap->GetFileName());

  return true;
}


// MGUIOptionsLoaderMeasurementsHDF: the end...
////////////////////////////////////////////////////////////////////////////////
