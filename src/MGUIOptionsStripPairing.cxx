/*
 * MGUIOptionsStripPairing.cxx
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
#include "MGUIOptionsStripPairing.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleStripPairingGreedy.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsStripPairing)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsStripPairing::MGUIOptionsStripPairing(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsStripPairing::~MGUIOptionsStripPairing()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsStripPairing::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);  
  
  m_Mode = new MGUIERBList(m_OptionsFrame, "Please select a strip pairing mode:");
  m_Mode->Add("Andreas's algorithm");
  m_Mode->Add("Multi Round Chi Square - Julian");
  m_Mode->Add("Daniel's 'greedy' algorithm");
  m_Mode->SetSelected(dynamic_cast<MModuleStripPairingGreedy*>(m_Module)->GetMode());
  m_Mode->Create();
  m_OptionsFrame->AddFrame(m_Mode, LabelLayout);

   PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsStripPairing::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsStripPairing::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleStripPairingGreedy*>(m_Module)->SetMode(m_Mode->GetSelected());
  
  return true;
}


// MGUIOptionsStripPairing: the end...
////////////////////////////////////////////////////////////////////////////////
