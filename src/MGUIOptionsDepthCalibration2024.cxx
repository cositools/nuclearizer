/*
 * MGUIOptionsDepthCalibration2024.cxx
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
#include "MGUIOptionsDepthCalibration2024.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleDepthCalibration2024.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsDepthCalibration2024)
#endif


  ////////////////////////////////////////////////////////////////////////////////


  MGUIOptionsDepthCalibration2024::MGUIOptionsDepthCalibration2024(MModule* Module) 
: MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDepthCalibration2024::~MGUIOptionsDepthCalibration2024()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsDepthCalibration2024::Create()
{
  PreCreate();

  m_CoeffsFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Select a coefficients file:",
      dynamic_cast<MModuleDepthCalibration2024*>(m_Module)->GetCoeffsFileName());
  m_CoeffsFileSelector->SetFileType("coeffs", "*.csv");
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_CoeffsFileSelector, LabelLayout);

  m_SplinesFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Select a splines file:",
      dynamic_cast<MModuleDepthCalibration2024*>(m_Module)->GetSplinesFileName());
  m_SplinesFileSelector->SetFileType("splines", "*.csv");
  TGLayoutHints* Label2Layout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_SplinesFileSelector, Label2Layout);

  m_UCSDOverride = new TGCheckButton(m_OptionsFrame, "Check this box if you're using the card cage at UCSD", 1);
  m_UCSDOverride->SetOn(dynamic_cast<MModuleDepthCalibration2024*>(m_Module)->GetUCSDOverride());
  TGLayoutHints* Label4Layout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_UCSDOverride, Label4Layout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsDepthCalibration2024::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsDepthCalibration2024::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleDepthCalibration2024*>(m_Module)->SetCoeffsFileName(m_CoeffsFileSelector->GetFileName());
  dynamic_cast<MModuleDepthCalibration2024*>(m_Module)->SetSplinesFileName(m_SplinesFileSelector->GetFileName());
  dynamic_cast<MModuleDepthCalibration2024*>(m_Module)->SetUCSDOverride(m_UCSDOverride->IsOn());

  return true;
}


// MGUIOptionsDepthCalibration2024: the end...
////////////////////////////////////////////////////////////////////////////////
