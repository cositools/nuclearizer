/*
 * MGUIOptionsDepthCalibration3rdPolyPixel.cxx
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
#include "MGUIOptionsDepthCalibration3rdPolyPixel.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModuleDepthCalibration3rdPolyPixel.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsDepthCalibration3rdPolyPixel)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDepthCalibration3rdPolyPixel::MGUIOptionsDepthCalibration3rdPolyPixel(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDepthCalibration3rdPolyPixel::~MGUIOptionsDepthCalibration3rdPolyPixel()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsDepthCalibration3rdPolyPixel::Create()
{
  PreCreate();

  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a detector map:",
    dynamic_cast<MNCTModuleDepthCalibration3rdPolyPixel*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Detector map", "*.txt");
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsDepthCalibration3rdPolyPixel::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsDepthCalibration3rdPolyPixel::OnApply()
{
	// Modify this to store the data in the module!

  dynamic_cast<MNCTModuleDepthCalibration3rdPolyPixel*>(m_Module)->SetFileName(m_FileSelector->GetFileName());

	return true;
}


// MGUIOptionsDepthCalibration3rdPolyPixel: the end...
////////////////////////////////////////////////////////////////////////////////