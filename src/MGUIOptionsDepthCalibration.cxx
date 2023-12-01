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
#include "MGUIOptionsDepthCalibration.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleDepthCalibration.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsDepthCalibration)
#endif


	////////////////////////////////////////////////////////////////////////////////


	MGUIOptionsDepthCalibration::MGUIOptionsDepthCalibration(MModule* Module) 
: MGUIOptions(Module)
{
	// standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDepthCalibration::~MGUIOptionsDepthCalibration()
{
	// kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsDepthCalibration::Create()
{
	PreCreate();

	//ummmm might need to load a detector map so that we can look at calibration data from palestine

	m_CoeffsFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Select a coefficients file:",
			dynamic_cast<MModuleDepthCalibration*>(m_Module)->GetCoeffsFileName());
	m_CoeffsFileSelector->SetFileType("coeffs", "*.txt");
	TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
	m_OptionsFrame->AddFrame(m_CoeffsFileSelector, LabelLayout);

	m_SplinesFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Select a splines file:",
			dynamic_cast<MModuleDepthCalibration*>(m_Module)->GetSplinesFileName());
	m_SplinesFileSelector->SetFileType("splines", "*.ctd");
	TGLayoutHints* Label2Layout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
	m_OptionsFrame->AddFrame(m_SplinesFileSelector, Label2Layout);


	PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsDepthCalibration::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsDepthCalibration::OnApply()
{
	// Modify this to store the data in the module!

	dynamic_cast<MModuleDepthCalibration*>(m_Module)->SetCoeffsFileName(m_CoeffsFileSelector->GetFileName());
	dynamic_cast<MModuleDepthCalibration*>(m_Module)->SetSplinesFileName(m_SplinesFileSelector->GetFileName());

	return true;
}


// MGUIOptionsDepthCalibration: the end...
////////////////////////////////////////////////////////////////////////////////
