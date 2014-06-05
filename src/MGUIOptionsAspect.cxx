/*
 * MGUIOptionsAspect.cxx
 *
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Jau-Shian Liang.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MGUIOptionsAspect.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TGNumberEntry.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModule.h"
#include "MNCTModuleAspect.h"
#include "MGUIEFileSelector.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsAspect)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsAspect::MGUIOptionsAspect(MNCTModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsAspect::~MGUIOptionsAspect()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsAspect::Create()
{
  PreCreate();

  // Modify here
//  m_FileSelector = new MGUIEFileSelector(this, "Please select an aspect file:",
//		    dynamic_cast<MNCTModuleAspect*>(m_Module)->GetAspectFilename());

//  m_FileSelector->SetFileType("Aspect file", "*.txt");
  
  TGHorizontalFrame* NumberEntryFrame = new TGHorizontalFrame(this, 100, 25);
  TGLabel* NumberEntryLabel = new TGLabel(NumberEntryFrame, "TimeZero (MJD) =  ");
  m_TimeZeroNumEntry = new TGNumberEntry(NumberEntryFrame,0,6);
  m_TimeZeroNumEntry->SetNumber(dynamic_cast<MNCTModuleAspect*>(m_Module)->GetMJDZero());
  //testNumEntry->SetText("test!!!!"); 
  
  TGHorizontalFrame* NumberEntryFrame2 = new TGHorizontalFrame(this, 100, 25);
  TGLabel* NumberEntryLabel2 = new TGLabel(NumberEntryFrame2, "Aspect delay (still testing) =  ");
  m_AspectDelayNumEntry = new TGNumberEntry(NumberEntryFrame2,0,6);
  m_AspectDelayNumEntry->SetNumber(dynamic_cast<MNCTModuleAspect*>(m_Module)->GetAspectDelay());
  
  m_CoordinateButtonGroup = new TGButtonGroup(this, "Coordinate System", kVerticalFrame);
  new TGRadioButton(m_CoordinateButtonGroup, "Equatorial");
  new TGRadioButton(m_CoordinateButtonGroup, "Galactic");
  m_CoordinateButtonGroup->SetButton(dynamic_cast<MNCTModuleAspect*>(m_Module)->GetCoordinate());
  //  m_EquatorialButton->SetOn(dynamic_cast<MNCTModuleAspect*>(m_Module)->GetLoadDeadStrip());
  
  m_RunTimeCorrectionButton =  new TGCheckButton(this, "Run Time correction (ON for flight data; OFF for else purples)", 1);
  m_RunTimeCorrectionButton->SetOn(dynamic_cast<MNCTModuleAspect*>(m_Module)->GetRunTimeCorrection());

  m_VerboseButton =  new TGCheckButton(this, "Verbose", 1);
  m_VerboseButton->SetOn(dynamic_cast<MNCTModuleAspect*>(m_Module)->GetVerbose());

  TGLayoutHints* NumEntryFrameLayout = new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 10, 10, 5, 5);
  TGLayoutHints* NumEntryLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 1, 1, 2, 2);
  TGLayoutHints* NumEntryLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 1, 1, 2, 2);
  //TGLayoutHints* FileSelectorLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  TGLayoutHints* ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 5, 5);
  TGLayoutHints* ButtonGroupLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 5, 5);
//  TGLayoutHints* RadioButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 5, 5);
  
//  AddFrame(m_FileSelector, FileSelectorLayout);
  AddFrame(NumberEntryFrame,NumEntryFrameLayout);
  NumberEntryFrame->AddFrame(NumberEntryLabel,NumEntryLabelLayout);
  NumberEntryFrame->AddFrame(m_TimeZeroNumEntry,NumEntryLayout);

  AddFrame(NumberEntryFrame2,NumEntryFrameLayout);
  NumberEntryFrame2->AddFrame(NumberEntryLabel2,NumEntryLabelLayout);
  NumberEntryFrame2->AddFrame(m_AspectDelayNumEntry,NumEntryLayout);

  AddFrame(m_CoordinateButtonGroup, ButtonGroupLayout);
  AddFrame(m_RunTimeCorrectionButton, ButtonLayout);
  AddFrame(m_VerboseButton, ButtonLayout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsAspect::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsAspect::OnApply()
{
	// Modify this to store the data in the module!
//  dynamic_cast<MNCTModuleAspect*>(m_Module)->SetAspectFilename((const char *)m_FileSelector->GetFileName());
  dynamic_cast<MNCTModuleAspect*>(m_Module)->SetMJDZero(m_TimeZeroNumEntry->GetNumber());
  dynamic_cast<MNCTModuleAspect*>(m_Module)->SetAspectDelay(m_AspectDelayNumEntry->GetNumber());

  if(m_CoordinateButtonGroup->GetButton(1)->IsOn()){
    dynamic_cast<MNCTModuleAspect*>(m_Module)->SetCoordinate(1);
  } else if (m_CoordinateButtonGroup->GetButton(2)->IsOn()){
    dynamic_cast<MNCTModuleAspect*>(m_Module)->SetCoordinate(2);
  }

  dynamic_cast<MNCTModuleAspect*>(m_Module)->SetRunTimeCorrection(m_RunTimeCorrectionButton->IsOn());
  dynamic_cast<MNCTModuleAspect*>(m_Module)->SetVerbose(m_VerboseButton->IsOn());
  
  return true;
}


// MGUIOptionsTemplate: the end...
////////////////////////////////////////////////////////////////////////////////
