/*
 * MGUIOptionsResponseGenerator.cxx
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
#include "MGUIOptionsResponseGenerator.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModuleResponseGenerator.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsResponseGenerator)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsResponseGenerator::MGUIOptionsResponseGenerator(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsResponseGenerator::~MGUIOptionsResponseGenerator()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsResponseGenerator::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);  
  
  m_Mode = new MGUIERBList(m_OptionsFrame, "Please select a response type:");
  m_Mode->Add("Spectral");
  m_Mode->Add("Bayesian event reconstruction");
  m_Mode->Add("Imaging");
  m_Mode->SetSelected(dynamic_cast<MNCTModuleResponseGenerator*>(m_Module)->GetMode());
  m_Mode->Create();
  m_OptionsFrame->AddFrame(m_Mode, LabelLayout);

  m_ResponseName = new MGUIEEntry(m_OptionsFrame, "Please choose a response name: ", false,
                                  dynamic_cast<MNCTModuleResponseGenerator*>(m_Module)->GetResponseName());
  m_OptionsFrame->AddFrame(m_ResponseName, LabelLayout);
  
  m_RevanCfgFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a revan configuration file:",
  dynamic_cast<MNCTModuleResponseGenerator*>(m_Module)->GetRevanConfigurationFileName());
  m_RevanCfgFileSelector->SetFileType("revan configuration", "*.revan.cfg");
  m_OptionsFrame->AddFrame(m_RevanCfgFileSelector, LabelLayout);
  
  m_MimrecCfgFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a mimrec configuration file:",
  dynamic_cast<MNCTModuleResponseGenerator*>(m_Module)->GetMimrecConfigurationFileName());
  m_MimrecCfgFileSelector->SetFileType("mimrec configuration", "*.mimrec.cfg");
  m_OptionsFrame->AddFrame(m_MimrecCfgFileSelector, LabelLayout);
  
  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsResponseGenerator::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsResponseGenerator::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MNCTModuleResponseGenerator*>(m_Module)->SetMode(m_Mode->GetSelected());
  dynamic_cast<MNCTModuleResponseGenerator*>(m_Module)->SetResponseName(m_ResponseName->GetAsString());
  dynamic_cast<MNCTModuleResponseGenerator*>(m_Module)->SetRevanConfigurationFileName(m_RevanCfgFileSelector->GetFileName());
  dynamic_cast<MNCTModuleResponseGenerator*>(m_Module)->SetMimrecConfigurationFileName(m_MimrecCfgFileSelector->GetFileName());
  
  return true;
}


// MGUIOptionsResponseGenerator: the end...
////////////////////////////////////////////////////////////////////////////////
