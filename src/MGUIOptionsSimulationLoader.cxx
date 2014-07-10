/*
 * MGUIOptionsSimulationLoader.cxx
 *
 *
 * Copyright (C) by Jau-Shian Liang.
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
#include "MGUIOptionsSimulationLoader.h"

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
#include "MNCTModuleSimulationLoader.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsSimulationLoader)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsSimulationLoader::MGUIOptionsSimulationLoader(MNCTModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsSimulationLoader::~MGUIOptionsSimulationLoader()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsSimulationLoader::Create()
{
  PreCreate();

  
  m_FileSelector = new MGUIEFileSelector(this, "Please select a simulation file:",
    dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Simulation file", "*.sim");
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 10, 10, 10, 10);
  AddFrame(m_FileSelector, LabelLayout);

  
  TGLabel* DEEIntro = new TGLabel(this, "Detector effects engine options:");
  AddFrame(DEEIntro, LabelLayout);
  
  
  
  TGHorizontalFrame* NumberEntryFrame = new TGHorizontalFrame(this, 150, 25);
  TGLabel* NumberEntryLabel = new TGLabel(NumberEntryFrame, "TimeOffset (Time += TimeOffset):   ");
  m_TimeOffset0NumEntry = new TGNumberEntry(NumberEntryFrame,0,9,-1,TGNumberFormat::kNESInteger);
  m_TimeOffset0NumEntry->SetNumber(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetTimeOffset0());
  m_TimeOffsetNumEntry = new TGNumberEntry(NumberEntryFrame,0,9);
  m_TimeOffsetNumEntry->SetNumber(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetTimeOffset());
  TGLabel* NumberEntryLabel20 = new TGLabel(NumberEntryFrame, " X 10^5 + ");
  TGLabel* NumberEntryLabel2 = new TGLabel(NumberEntryFrame, "sec   ");
  //testNumEntry->SetText("test!!!!"); 
  
  m_DeadStripButton = new TGCheckButton(this, "Check dead strip (dead_strip.list)", 1);
  m_DeadStripButton->SetOn(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetLoadDeadStrip());
  m_CoincidenceButton = new TGCheckButton(this, "Check coincidence (CoincidenceVolume.list)", 1);
  m_CoincidenceButton->SetOn(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetLoadCoincidence());
  m_AntiCoincidenceButton = new TGCheckButton(this, "Check anti-coincidence (AntiCoincidenceVolume.list)", 1);
  m_AntiCoincidenceButton->SetOn(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetLoadAntiCoincidence());
  m_ChargeSharingButton =  new TGCheckButton(this, "Run charge sharing", 1);
  m_ChargeSharingButton->SetOn(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetRunEnergySharing());
  m_CrosstalkButton =  new TGCheckButton(this, "Run crosstalk", 1);
  m_CrosstalkButton->SetOn(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetRunCrosstalk());
  m_NonlinearGainButton =  new TGCheckButton(this, "Nonlinear Energy/ADC", 1);
  m_NonlinearGainButton->SetOn(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetNonlinearGain());
  m_KeepLLDOnlyButton = new TGCheckButton(this, "Keep LLD_Only hits (timing = -9999)", 1);
  m_KeepLLDOnlyButton->SetOn(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetKeepLLDOnly());
  m_VerboseButton = new TGCheckButton(this, "Verbose", 1);
  m_VerboseButton->SetOn(dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetVerbose());

  TGLayoutHints* NumEntryFrameLayout = new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 10, 10, 5, 5);
  TGLayoutHints* NumEntryLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 1, 1, 2, 2);
  TGLayoutHints* NumEntryLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 1, 1, 2, 2);
  TGLayoutHints* ButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 5, 5);
  
  AddFrame(NumberEntryFrame,NumEntryFrameLayout);
  NumberEntryFrame->AddFrame(NumberEntryLabel,NumEntryLabelLayout);
  NumberEntryFrame->AddFrame(m_TimeOffset0NumEntry,NumEntryLayout);
  NumberEntryFrame->AddFrame(NumberEntryLabel20,NumEntryLabelLayout);
  NumberEntryFrame->AddFrame(m_TimeOffsetNumEntry,NumEntryLayout);
  NumberEntryFrame->AddFrame(NumberEntryLabel2,NumEntryLabelLayout);

  AddFrame(m_DeadStripButton, ButtonLayout);
  AddFrame(m_CoincidenceButton, ButtonLayout);
  AddFrame(m_AntiCoincidenceButton, ButtonLayout);
  AddFrame(m_ChargeSharingButton, ButtonLayout);
  AddFrame(m_CrosstalkButton, ButtonLayout);
  AddFrame(m_NonlinearGainButton, ButtonLayout);
  AddFrame(m_KeepLLDOnlyButton, ButtonLayout);
  AddFrame(m_VerboseButton, ButtonLayout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsSimulationLoader::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsSimulationLoader::OnApply()
{
  // Modify this to store the data in the module!
  
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetFileName(m_FileSelector->GetFileName());
  
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetTimeOffset0(m_TimeOffset0NumEntry->GetIntNumber());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetTimeOffset(m_TimeOffsetNumEntry->GetNumber());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetLoadDeadStrip(m_DeadStripButton->IsOn());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetLoadCoincidence(m_CoincidenceButton->IsOn());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetLoadAntiCoincidence(m_AntiCoincidenceButton->IsOn());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetRunEnergySharing(m_ChargeSharingButton->IsOn());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetRunCrosstalk(m_CrosstalkButton->IsOn());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetNonlinearGain(m_NonlinearGainButton->IsOn());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetKeepLLDOnly(m_KeepLLDOnlyButton->IsOn());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetVerbose(m_VerboseButton->IsOn());
  
  return true;
}


// MGUIOptionsTemplate: the end...
////////////////////////////////////////////////////////////////////////////////
