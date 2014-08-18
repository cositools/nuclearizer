/*
 * MGUIOptionsReceiverCOSI2014.cxx
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
#include "MGUIOptionsReceiverCOSI2014.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModule.h"
#include "MNCTModuleReceiverCOSI2014.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsReceiverCOSI2014)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsReceiverCOSI2014::MGUIOptionsReceiverCOSI2014(MNCTModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsReceiverCOSI2014::~MGUIOptionsReceiverCOSI2014()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsReceiverCOSI2014::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, m_FontScaler*20, m_FontScaler*2);
  TGLayoutHints* ContentLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, m_FontScaler*2, m_FontScaler*2);

  TGLabel* HandshakerLabel = new TGLabel(m_OptionsFrame, "Connection to the distributor:");
  m_OptionsFrame->AddFrame(HandshakerLabel, LabelLayout);
  
  m_DistributorName = new MGUIEEntry(m_OptionsFrame, "Distributor name/IP: ", false,
                              dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->GetDistributorName());
  m_OptionsFrame->AddFrame(m_DistributorName, ContentLayout);

  m_DistributorPort = new MGUIEEntry(m_OptionsFrame, "Distributor listening port: ", false,
                              dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->GetDistributorPort());
  m_OptionsFrame->AddFrame(m_DistributorPort, ContentLayout);

  m_DistributorStreamID = new MGUIEEntry(m_OptionsFrame, "Stream ID: ", false,
                              dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->GetDistributorStreamID());
  m_OptionsFrame->AddFrame(m_DistributorStreamID, ContentLayout);

  
  TGLabel* SendToLabel = new TGLabel(m_OptionsFrame, "Connection where the data should be sent to:");
  m_OptionsFrame->AddFrame(SendToLabel, LabelLayout);

  m_SendToName = new MGUIEEntry(m_OptionsFrame, "Name/IP of the machine where we send the data to: ", false,
                              dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->GetLocalReceivingHostName());
  m_OptionsFrame->AddFrame(m_SendToName, ContentLayout);

  m_SendToPort = new MGUIEEntry(m_OptionsFrame, "Port on the machine where we send the data to: ", false,
                              dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->GetLocalReceivingPort());
  m_OptionsFrame->AddFrame(m_SendToPort, ContentLayout);

  
  m_DataMode = new MGUIERBList(m_OptionsFrame, "Choose the data to look at: ");
  m_DataMode->Add("Raw mode");
  m_DataMode->Add("Compton mode");
  m_DataMode->Add("Both");
  m_DataMode->SetSelected((int) dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->GetDataSelectionMode());
  m_DataMode->Create();
  m_OptionsFrame->AddFrame(m_DataMode, LabelLayout);
  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsReceiverCOSI2014::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsReceiverCOSI2014::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetDistributorName(m_DistributorName->GetAsString());
  dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetDistributorPort(m_DistributorPort->GetAsInt());
  dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetDistributorStreamID(m_DistributorStreamID->GetAsString());

  dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetLocalReceivingHostName(m_SendToName->GetAsString());
  dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetLocalReceivingPort(m_SendToPort->GetAsInt());

  if (m_DataMode->GetSelected() == 0) {
    dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetDataSelectionMode(MNCTModuleReceiverCOSI2014DataModes::c_Raw);     
  } else if (m_DataMode->GetSelected() == 1) {
    dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetDataSelectionMode(MNCTModuleReceiverCOSI2014DataModes::c_Compton);     
  } else if (m_DataMode->GetSelected() == 2) {
    dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetDataSelectionMode(MNCTModuleReceiverCOSI2014DataModes::c_All);     
  }
  
  return true;
}


// MGUIOptionsReceiverCOSI2014: the end...
////////////////////////////////////////////////////////////////////////////////
