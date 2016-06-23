/*
 * MGUIOptionsDepthCalibration.cxx
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
#include "MGUIOptionsDepthCalibrationB.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModuleDepthCalibrationB.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsDepthCalibrationB)
#endif


	////////////////////////////////////////////////////////////////////////////////


	MGUIOptionsDepthCalibrationB::MGUIOptionsDepthCalibrationB(MModule* Module) 
: MGUIOptions(Module)
{
	// standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDepthCalibrationB::~MGUIOptionsDepthCalibrationB()
{
	// kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsDepthCalibrationB::Create()
{
	PreCreate();

	m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Select a lookup table file:",
			dynamic_cast<MNCTModuleDepthCalibrationB*>(m_Module)->GetLookupTableFilename());
	m_FileSelector->SetFileType("", "*.txt");
	TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
	m_OptionsFrame->AddFrame(m_FileSelector, LabelLayout);

	PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsDepthCalibrationB::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsDepthCalibrationB::OnApply()
{
	// Modify this to store the data in the module!

	dynamic_cast<MNCTModuleDepthCalibrationB*>(m_Module)->SetLookupTableFilename(m_FileSelector->GetFileName());

	return true;
}


// MGUIOptionsDepthCalibrationB: the end...
////////////////////////////////////////////////////////////////////////////////
