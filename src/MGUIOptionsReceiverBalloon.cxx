/*
 * MGUIOptionsReceiverBalloon.cxx
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
#include "MGUIOptionsReceiverBalloon.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModule.h"
#include "MModuleReceiverBalloon.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsReceiverBalloon)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsReceiverBalloon::MGUIOptionsReceiverBalloon(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsReceiverBalloon::~MGUIOptionsReceiverBalloon()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsReceiverBalloon::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, m_FontScaler*20, m_FontScaler*2);
  TGLayoutHints* ContentLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, m_FontScaler*2, m_FontScaler*2);

  TGLabel* HandshakerLabel = new TGLabel(m_OptionsFrame, "Connection to the distributor:");
  m_OptionsFrame->AddFrame(HandshakerLabel, LabelLayout);
  
  m_DistributorName = new MGUIEEntry(m_OptionsFrame, "Distributor IP (e.g. 128.32.13.133): ", false,
                              dynamic_cast<MModuleReceiverBalloon*>(m_Module)->GetDistributorName());
  m_OptionsFrame->AddFrame(m_DistributorName, ContentLayout);

  m_DistributorPort = new MGUIEEntry(m_OptionsFrame, "Distributor listening port (e.g. 21526): ", false,
                              dynamic_cast<MModuleReceiverBalloon*>(m_Module)->GetDistributorPort());
  m_OptionsFrame->AddFrame(m_DistributorPort, ContentLayout);

  m_DistributorStreamID = new MGUIEEntry(m_OptionsFrame, "Stream ID (e.g. ALL): ", false,
                              dynamic_cast<MModuleReceiverBalloon*>(m_Module)->GetDistributorStreamID());
  m_OptionsFrame->AddFrame(m_DistributorStreamID, ContentLayout);

  /*
  TGLabel* SendToLabel = new TGLabel(m_OptionsFrame, "Connection where the data should be sent to:");
  m_OptionsFrame->AddFrame(SendToLabel, LabelLayout);

  m_SendToName = new MGUIEEntry(m_OptionsFrame, "Name/IP of the machine where we send the data to: ", false,
                              dynamic_cast<MModuleReceiverBalloon*>(m_Module)->GetLocalReceivingHostName());
  m_OptionsFrame->AddFrame(m_SendToName, ContentLayout);

  m_SendToPort = new MGUIEEntry(m_OptionsFrame, "Port on the machine where we send the data to: ", false,
                              dynamic_cast<MModuleReceiverBalloon*>(m_Module)->GetLocalReceivingPort());
  m_OptionsFrame->AddFrame(m_SendToPort, ContentLayout);
  */
  
  m_DataMode = new MGUIERBList(m_OptionsFrame, "Choose the data to look at: ");
  m_DataMode->Add("Raw mode");
  m_DataMode->Add("Compton mode");
  m_DataMode->Add("Both");
  m_DataMode->SetSelected((int) dynamic_cast<MModuleReceiverBalloon*>(m_Module)->GetDataSelectionMode());
  m_DataMode->Create();
  m_OptionsFrame->AddFrame(m_DataMode, LabelLayout);

  m_AspectMode = new MGUIERBList(m_OptionsFrame, "Choose the aspect mode");
  m_AspectMode->Add("GPS");
  m_AspectMode->Add("Magnetometer");
  m_AspectMode->Add("None");
  m_AspectMode->SetSelected((int) dynamic_cast<MModuleReceiverBalloon*>(m_Module)->GetAspectMode());
  m_AspectMode->Create();
  m_OptionsFrame->AddFrame(m_AspectMode, LabelLayout);
  
  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "If a file is selected, then the input stream is saved as roa :",
  dynamic_cast<MModuleReceiverBalloon*>(m_Module)->GetRoaFileName());
  m_FileSelector->SetFileType("Read-out file", "*.roa");
  m_OptionsFrame->AddFrame(m_FileSelector, ContentLayout);

  
  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsReceiverBalloon::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsReceiverBalloon::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetDistributorName(m_DistributorName->GetAsString());
  dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetDistributorPort(m_DistributorPort->GetAsInt());
  dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetDistributorStreamID(m_DistributorStreamID->GetAsString());

  //dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetLocalReceivingHostName(m_SendToName->GetAsString());
  //dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetLocalReceivingPort(m_SendToPort->GetAsInt());

  if (m_DataMode->GetSelected() == 0) {
    dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetDataSelectionMode(MBinaryFlightDataParserDataModes::c_Raw);     
  } else if (m_DataMode->GetSelected() == 1) {
    dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetDataSelectionMode(MBinaryFlightDataParserDataModes::c_Compton);     
  } else if (m_DataMode->GetSelected() == 2) {
    dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetDataSelectionMode(MBinaryFlightDataParserDataModes::c_All);     
  }

  if (m_AspectMode->GetSelected() == 0) {
    dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetAspectMode(MBinaryFlightDataParserAspectModes::c_GPS);     
  } else if (m_AspectMode->GetSelected() == 1) {
    dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetAspectMode(MBinaryFlightDataParserAspectModes::c_Magnetometer);     
  } else if (m_AspectMode->GetSelected() == 2) {
    dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetAspectMode(MBinaryFlightDataParserAspectModes::c_Neither);     
  }
  
  dynamic_cast<MModuleReceiverBalloon*>(m_Module)->SetRoaFileName(m_FileSelector->GetFileName());  
  
  return true;
}


// MGUIOptionsReceiverBalloon: the end...
////////////////////////////////////////////////////////////////////////////////
