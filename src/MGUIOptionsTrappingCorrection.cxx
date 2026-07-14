/*
 * MGUIOptionsTrappingCorrection.cxx
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
#include "MGUIOptionsTrappingCorrection.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModule.h"
#include "MModuleTrappingCorrection.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsTrappingCorrection)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTrappingCorrection::MGUIOptionsTrappingCorrection(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsTrappingCorrection::~MGUIOptionsTrappingCorrection()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsTrappingCorrection::Create()
{
  PreCreate();

  m_SimCCEFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Select a trapping parameter file:",
      dynamic_cast<MModuleTrappingCorrection*>(m_Module)->GetSimCCEFileName());
  m_SimCCEFileSelector->SetFileType("trapping parameters", "*.csv");
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_SimCCEFileSelector, LabelLayout);

  TGLayoutHints* RBLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 40, 10, 2, 0);
  TGLayoutHints* RBOptionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 60, 10, 2, 0);
  TGLayoutHints* RBOptionStretchLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 60, 10, 2, 0);

  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsTrappingCorrection::OnApply()
{
 // Modify this to store the data in the module!

  dynamic_cast<MModuleTrappingCorrection*>(m_Module)->SetSimCCEFileName(m_SimCCEFileSelector->GetFileName());

  return true;
}


// MGUIOptionsTrappingCorrection: the end...
////////////////////////////////////////////////////////////////////////////////
