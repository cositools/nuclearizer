/*
 * MGUIOptionsCOSIPayloadLollipopCut.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Nicole Rodriguez Cavero
 * Sean Pike
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Sean Pike, Andreas Zoglauer, Nicole Rodriguez Cavero.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MGUIOptionsCOSIPayloadLollipopCut.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MString.h"
#include "MGUIEFileSelector.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIEEntry.h"

// Nuclearizer libs:
#include "MCOSIPayloadLollipopCut.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsCOSIPayloadLollipopCut
)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsCOSIPayloadLollipopCut::MGUIOptionsCOSIPayloadLollipopCut(MModule* Module)
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsCOSIPayloadLollipopCut::~MGUIOptionsCOSIPayloadLollipopCut()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsCOSIPayloadLollipopCut::Create()
{
  
  PreCreate();
  
  MCOSIPayloadLollipopCut* Module = dynamic_cast<MCOSIPayloadLollipopCut*>(m_Module);
  
  TGLayoutHints* LabelLayout  = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);
  TGLayoutHints* RBLayout     = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 5, 5, 5);
  TGLayoutHints* ColumnLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX, 10, 10, 0, 0);
  TGLayoutHints* MainLayout   = new TGLayoutHints(kLHintsTop | kLHintsExpandX, 10, 10, 10, 10);

  // Build our grid of buttons
  TGHorizontalFrame* ColumnsFrame = new TGHorizontalFrame(m_OptionsFrame);
  
  //Column 1 for source 1
  TGVerticalFrame* Column1 = new TGVerticalFrame(ColumnsFrame);
  
  TGLabel* SourceLabel = new TGLabel(Column1, "Please choose the position of the lollipops:");
  Column1->AddFrame(SourceLabel, LabelLayout);
  
  m_RBSourcePosition1 = new TGRadioButton(Column1, "L0L1 Lollipops", c_SourcePosition1);
  m_RBSourcePosition1->Associate(this);
  Column1->AddFrame(m_RBSourcePosition1, RBLayout);
  
  m_RBSourcePosition2 = new TGRadioButton(Column1, "L1L2 Lollipops", c_SourcePosition2);
  m_RBSourcePosition2->Associate(this);
  Column1->AddFrame(m_RBSourcePosition2, RBLayout);
  
  m_RBSourcePosition3 = new TGRadioButton(Column1, "L2L3 Lollipops", c_SourcePosition3);
  m_RBSourcePosition3->Associate(this);
  Column1->AddFrame(m_RBSourcePosition3, RBLayout);
  
  ColumnsFrame->AddFrame(Column1, ColumnLayout);
  
  m_OptionsFrame->AddFrame(ColumnsFrame, MainLayout);

  // Set initial toggles from module saved positions
  if (Module != nullptr) {
    ToggleRadioButtons(c_SourcePosition1 + Module->GetSourcePosition() - 1);
  }
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsCOSIPayloadLollipopCut::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if there are more buttons
  
  bool Status = true;
  
  switch (GET_MSG(Message)) {
    case kC_COMMAND:
      switch (GET_SUBMSG(Message)) {
        case kCM_BUTTON:
          break;
          
        case kCM_RADIOBUTTON:
          ToggleRadioButtons(Parameter1);
          break;
          
        case kCM_CHECKBUTTON:
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


bool MGUIOptionsCOSIPayloadLollipopCut::OnApply()
{
  MCOSIPayloadLollipopCut* Module = dynamic_cast<MCOSIPayloadLollipopCut*>(m_Module);
  if (Module == nullptr) return false;

  // Store Source 1 Choice
  if (m_RBSourcePosition1->GetState() == kButtonDown) Module->SetSourcePosition(1);
  else if (m_RBSourcePosition2->GetState() == kButtonDown) Module->SetSourcePosition(2);
  else if (m_RBSourcePosition3->GetState() == kButtonDown) Module->SetSourcePosition(3);

  Module->UpdateKeptDetectors();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void MGUIOptionsCOSIPayloadLollipopCut::ToggleRadioButtons(int WidgetID)
{
  // Column 1 source 1
  if (WidgetID >= c_SourcePosition1 && WidgetID <= c_SourcePosition3) {
    m_RBSourcePosition1->SetState(WidgetID == c_SourcePosition1 ? kButtonDown : kButtonUp);
    m_RBSourcePosition2->SetState(WidgetID == c_SourcePosition2 ? kButtonDown : kButtonUp);
    m_RBSourcePosition3->SetState(WidgetID == c_SourcePosition3 ? kButtonDown : kButtonUp);
  }
}


// MGUIOptionsCOSIPayloadLollipopCut: the end...
////////////////////////////////////////////////////////////////////////////////
