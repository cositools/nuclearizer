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
  
  // TODO: Sources are always placed as a pair in practice, so replace the
  //       two columns of 6 radio buttons with a single column of 6 paired positions.
  
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
  
  TGLabel* Source1Label = new TGLabel(Column1, "Please choose the position of source 1:");
  Column1->AddFrame(Source1Label, LabelLayout);
  
  m_RBSource1Position1 = new TGRadioButton(Column1, "Q0Q1 L0L1 Side Illuminated", c_Source1Position1);
  m_RBSource1Position1->Associate(this);
  Column1->AddFrame(m_RBSource1Position1, RBLayout);
  
  m_RBSource1Position2 = new TGRadioButton(Column1, "Q1Q2 L0L1 Side Illuminated", c_Source1Position2);
  m_RBSource1Position2->Associate(this);
  Column1->AddFrame(m_RBSource1Position2, RBLayout);
  
  m_RBSource1Position3 = new TGRadioButton(Column1, "Q0Q1 L1L2 Side Illuminated", c_Source1Position3);
  m_RBSource1Position3->Associate(this);
  Column1->AddFrame(m_RBSource1Position3, RBLayout);
  
  m_RBSource1Position4 = new TGRadioButton(Column1, "Q1Q2 L1L2 Side Illuminated", c_Source1Position4);
  m_RBSource1Position4->Associate(this);
  Column1->AddFrame(m_RBSource1Position4, RBLayout);
  
  m_RBSource1Position5 = new TGRadioButton(Column1, "Q0Q1 L2L3 Side Illuminated", c_Source1Position5);
  m_RBSource1Position5->Associate(this);
  Column1->AddFrame(m_RBSource1Position5, RBLayout);

  m_RBSource1Position6 = new TGRadioButton(Column1, "Q1Q2 L2L3 Side Illuminated", c_Source1Position6);
  m_RBSource1Position6->Associate(this);
  Column1->AddFrame(m_RBSource1Position6, RBLayout);
  
  ColumnsFrame->AddFrame(Column1, ColumnLayout);
  
  // Column 2 for source 2
  TGVerticalFrame* Column2 = new TGVerticalFrame(ColumnsFrame);
  
  TGLabel* Source2Label = new TGLabel(Column2, "Please choose the position of source 2:");
  Column2->AddFrame(Source2Label, LabelLayout);
  
  m_RBSource2Position1 = new TGRadioButton(Column2, "Q2Q3 L0L1 Side Illuminated", c_Source2Position1);
  m_RBSource2Position1->Associate(this);
  Column2->AddFrame(m_RBSource2Position1, RBLayout);
  
  m_RBSource2Position2 = new TGRadioButton(Column2, "Q3Q0 L0L1 Side Illuminated", c_Source2Position2);
  m_RBSource2Position2->Associate(this);
  Column2->AddFrame(m_RBSource2Position2, RBLayout);
  
  m_RBSource2Position3 = new TGRadioButton(Column2, "Q2Q3 L1L2 Side Illuminated", c_Source2Position3);
  m_RBSource2Position3->Associate(this);
  Column2->AddFrame(m_RBSource2Position3, RBLayout);
  
  m_RBSource2Position4 = new TGRadioButton(Column2, "Q3Q0 L1L2 Side Illuminated", c_Source2Position4);
  m_RBSource2Position4->Associate(this);
  Column2->AddFrame(m_RBSource2Position4, RBLayout);
  
  m_RBSource2Position5 = new TGRadioButton(Column2, "Q2Q3 L2L3 Side Illuminated", c_Source2Position5);
  m_RBSource2Position5->Associate(this);
  Column2->AddFrame(m_RBSource2Position5, RBLayout);
  
  m_RBSource2Position6 = new TGRadioButton(Column2, "Q3Q0 L2L3 Side Illuminated", c_Source2Position6);
  m_RBSource2Position6->Associate(this);
  Column2->AddFrame(m_RBSource2Position6, RBLayout);
  
  ColumnsFrame->AddFrame(Column2, ColumnLayout);

  m_OptionsFrame->AddFrame(ColumnsFrame, MainLayout);

  // Set initial toggles from module saved positions
  if (Module != nullptr) {
    ToggleRadioButtons(c_Source1Position1 + Module->GetSource1Position() - 1);
    ToggleRadioButtons(c_Source2Position1 + Module->GetSource2Position() - 1);
  }
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsCOSIPayloadLollipopCut::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons
  
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
  if (m_RBSource1Position1->GetState() == kButtonDown) Module->SetSource1Position(1);
  else if (m_RBSource1Position2->GetState() == kButtonDown) Module->SetSource1Position(2);
  else if (m_RBSource1Position3->GetState() == kButtonDown) Module->SetSource1Position(3);
  else if (m_RBSource1Position4->GetState() == kButtonDown) Module->SetSource1Position(4);
  else if (m_RBSource1Position5->GetState() == kButtonDown) Module->SetSource1Position(5);
  else if (m_RBSource1Position6->GetState() == kButtonDown) Module->SetSource1Position(6);

  // Store Source 2 Choice
  if (m_RBSource2Position1->GetState() == kButtonDown) Module->SetSource2Position(1);
  else if (m_RBSource2Position2->GetState() == kButtonDown) Module->SetSource2Position(2);
  else if (m_RBSource2Position3->GetState() == kButtonDown) Module->SetSource2Position(3);
  else if (m_RBSource2Position4->GetState() == kButtonDown) Module->SetSource2Position(4);
  else if (m_RBSource2Position5->GetState() == kButtonDown) Module->SetSource2Position(5);
  else if (m_RBSource2Position6->GetState() == kButtonDown) Module->SetSource2Position(6);

  Module->UpdateKeptDetectors();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void MGUIOptionsCOSIPayloadLollipopCut::ToggleRadioButtons(int WidgetID)
{
  // Column 1 source 1
  if (WidgetID >= c_Source1Position1 && WidgetID <= c_Source1Position6) {
    m_RBSource1Position1->SetState(WidgetID == c_Source1Position1 ? kButtonDown : kButtonUp);
    m_RBSource1Position2->SetState(WidgetID == c_Source1Position2 ? kButtonDown : kButtonUp);
    m_RBSource1Position3->SetState(WidgetID == c_Source1Position3 ? kButtonDown : kButtonUp);
    m_RBSource1Position4->SetState(WidgetID == c_Source1Position4 ? kButtonDown : kButtonUp);
    m_RBSource1Position5->SetState(WidgetID == c_Source1Position5 ? kButtonDown : kButtonUp);
    m_RBSource1Position6->SetState(WidgetID == c_Source1Position6 ? kButtonDown : kButtonUp);
  }
  // Column 2 source 2
  else if (WidgetID >= c_Source2Position1 && WidgetID <= c_Source2Position6) {
    m_RBSource2Position1->SetState(WidgetID == c_Source2Position1 ? kButtonDown : kButtonUp);
    m_RBSource2Position2->SetState(WidgetID == c_Source2Position2 ? kButtonDown : kButtonUp);
    m_RBSource2Position3->SetState(WidgetID == c_Source2Position3 ? kButtonDown : kButtonUp);
    m_RBSource2Position4->SetState(WidgetID == c_Source2Position4 ? kButtonDown : kButtonUp);
    m_RBSource2Position5->SetState(WidgetID == c_Source2Position5 ? kButtonDown : kButtonUp);
    m_RBSource2Position6->SetState(WidgetID == c_Source2Position6 ? kButtonDown : kButtonUp);
  }
}


// MGUIOptionsCOSIPayloadLollipopCut: the end...
////////////////////////////////////////////////////////////////////////////////
