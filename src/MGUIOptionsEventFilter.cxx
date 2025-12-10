/*
 * MGUIOptionsEventFilter.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer
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
#include "MGUIOptionsEventFilter.h"

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

// Nuclearizer libs:
#include "MModuleEventFilter.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsEventFilter)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventFilter::MGUIOptionsEventFilter(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventFilter::~MGUIOptionsEventFilter()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsEventFilter::Create()
{
  PreCreate();

  // Modify here

  TGLayoutHints* FirstLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 30, 2);
  TGLayoutHints* SecondariesLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 10, 10, 2, 2);

  TGLabel* TotalEnergyLabel = new TGLabel(m_OptionsFrame,
    "Energy window\n"
    "This requires that an energy calibration has been applied previously.\n"
    "In addition, it is always applied to the highest analysis level:\n"
    "Reconstructed event -> hits -> strip hits");
  m_OptionsFrame->AddFrame(TotalEnergyLabel, FirstLayout);

  m_TotalEnergy = new MGUIEMinMaxEntry(m_OptionsFrame, 
                                       "Choose the minimum and maximum energy [keV]:", 
                                       false,
                                       dynamic_cast<MModuleEventFilter*>(m_Module)->GetMinimumTotalEnergy(),
                                       dynamic_cast<MModuleEventFilter*>(m_Module)->GetMaximumTotalEnergy(),
                                       true, 0.0);
  m_OptionsFrame->AddFrame(m_TotalEnergy, SecondariesLayout);


  
  TGLabel* StripsLabel = new TGLabel(m_OptionsFrame,
    "Low- and high voltage read-out strips");
  m_OptionsFrame->AddFrame(StripsLabel, FirstLayout);
  m_LVStrips = new MGUIEMinMaxEntry(m_OptionsFrame,
                                   "Choose the minimum and maximum number of low voltage strips:",
                                   false,
                                   dynamic_cast<MModuleEventFilter*>(m_Module)->GetMinimumLVStrips(),
                                   dynamic_cast<MModuleEventFilter*>(m_Module)->GetMaximumLVStrips(),
                                   true, 0.0);
  m_OptionsFrame->AddFrame(m_LVStrips, SecondariesLayout);

  m_HVStrips = new MGUIEMinMaxEntry(m_OptionsFrame,
                                   "Choose the minimum and maximum number of high voltage strips:",
                                   false,
                                   dynamic_cast<MModuleEventFilter*>(m_Module)->GetMinimumHVStrips(),
                                   dynamic_cast<MModuleEventFilter*>(m_Module)->GetMaximumHVStrips(),
                                   true, 0.0);
  m_OptionsFrame->AddFrame(m_HVStrips, SecondariesLayout);

  TGLabel* HitsLabel = new TGLabel(m_OptionsFrame,
    "Hit window\n"
    "This requires that Strip pairing has been done.");
  m_OptionsFrame->AddFrame(HitsLabel, FirstLayout);

  m_Hits = new MGUIEMinMaxEntry(m_OptionsFrame,
                                "Choose the minimum and maximum number of hits:",
                                false,
                                dynamic_cast<MModuleEventFilter*>(m_Module)->GetMinimumHits(),
                                dynamic_cast<MModuleEventFilter*>(m_Module)->GetMaximumHits(),
                                true, 0.0);
  m_OptionsFrame->AddFrame(m_Hits, SecondariesLayout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEventFilter::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsEventFilter::OnApply()
{
  // Store the data in the module

  dynamic_cast<MModuleEventFilter*>(m_Module)->SetMinimumTotalEnergy(m_TotalEnergy->GetMinValue());
  dynamic_cast<MModuleEventFilter*>(m_Module)->SetMaximumTotalEnergy(m_TotalEnergy->GetMaxValue());

  dynamic_cast<MModuleEventFilter*>(m_Module)->SetMinimumHVStrips(m_HVStrips->GetMinValue());
  dynamic_cast<MModuleEventFilter*>(m_Module)->SetMaximumHVStrips(m_HVStrips->GetMaxValue());

  dynamic_cast<MModuleEventFilter*>(m_Module)->SetMinimumLVStrips(m_LVStrips->GetMinValue());
  dynamic_cast<MModuleEventFilter*>(m_Module)->SetMaximumLVStrips(m_LVStrips->GetMaxValue());

  dynamic_cast<MModuleEventFilter*>(m_Module)->SetMinimumHits(m_Hits->GetMinValue());
  dynamic_cast<MModuleEventFilter*>(m_Module)->SetMaximumHits(m_Hits->GetMaxValue());

  return true;
}


// MGUIOptionsEventFilter: the end...
////////////////////////////////////////////////////////////////////////////////
