/*
 * MGUIOptionsLoaderSimulationsCosima.cxx
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
#include "MGUIOptionsLoaderSimulationsCosima.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleLoaderSimulationsCosima.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsLoaderSimulationsCosima)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderSimulationsCosima::MGUIOptionsLoaderSimulationsCosima(MModule* Module)
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsLoaderSimulationsCosima::~MGUIOptionsLoaderSimulationsCosima()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsLoaderSimulationsCosima::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_SimulationFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a simulations file:",
    dynamic_cast<MModuleLoaderSimulationsCosima*>(m_Module)->GetSimulationFileName());
  m_SimulationFileSelector->SetFileType("Sim file", "*.sim");
  m_SimulationFileSelector->SetFileType("Sim file (gzip'ed)", "*.sim.gz");
  m_OptionsFrame->AddFrame(m_SimulationFileSelector, LabelLayout);

  TGHorizontalFrame* PassedFrame = new TGHorizontalFrame(m_OptionsFrame);
  TGLayoutHints* PassedFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);  
  m_OptionsFrame->AddFrame(PassedFrame, PassedFrameLayout);
  
  m_StopAfter = new TGCheckButton(PassedFrame, "Stop after this number of PASSED events: ", 1);
  m_StopAfter->Associate(this);
  m_StopAfter->SetOn(dynamic_cast<MModuleLoaderSimulationsCosima*>(m_Module)->UseStopAfter());
  TGLayoutHints* StopAfterLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 10, 10);
  PassedFrame->AddFrame(m_StopAfter, StopAfterLayout);
  
  m_MaximumAcceptedEvents = new MGUIEEntry(PassedFrame, " ", false, 
                                           dynamic_cast<MModuleLoaderSimulationsCosima*>(m_Module)->GetMaximumAcceptedEvents(), true, 0l);
  if (m_StopAfter->IsOn() == false) m_MaximumAcceptedEvents->SetEnabled(false);
  PassedFrame->AddFrame(m_MaximumAcceptedEvents, StopAfterLayout);
  
  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsLoaderSimulationsCosima::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    case kCM_CHECKBUTTON:
      if (Parameter1 == 1) {
        m_MaximumAcceptedEvents->SetEnabled(m_StopAfter->IsOn());
      }
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


bool MGUIOptionsLoaderSimulationsCosima::OnApply()
{
  // Modify this to store the data in the module!

  cout<<"Settint: "<<m_SimulationFileSelector->GetFileName()<<endl;

  dynamic_cast<MModuleLoaderSimulationsCosima*>(m_Module)->SetSimulationFileName(m_SimulationFileSelector->GetFileName());
  dynamic_cast<MModuleLoaderSimulationsCosima*>(m_Module)->SetUseStopAfter(m_StopAfter->IsOn());
  dynamic_cast<MModuleLoaderSimulationsCosima*>(m_Module)->SetMaximumAcceptedEvents(m_MaximumAcceptedEvents->GetAsInt());
  
  return true;
}


// MGUIOptionsLoaderSimulationsCosima: the end...
////////////////////////////////////////////////////////////////////////////////
