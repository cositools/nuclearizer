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

  TGLayoutHints* TotalEnergyLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_TotalEnergy = new MGUIEMinMaxEntry(m_OptionsFrame, 
                                       "Choose the minimum and maximum energy [keV]:", 
                                       false,
                                       dynamic_cast<MModuleEventFilter*>(m_Module)->GetMinimumTotalEnergy(),
                                       dynamic_cast<MModuleEventFilter*>(m_Module)->GetMaximumTotalEnergy(),
                                       true, 0.0);
  m_OptionsFrame->AddFrame(m_TotalEnergy, TotalEnergyLayout);

  TGLabel* TotalEnergyLabel = new TGLabel(m_OptionsFrame, 
    "The energy window requires that an energy calibration has been applied.\n"
    "In addition, it is always applied to the highest analysis level:\n"
    "Reconstructed event -> hits -> strip hits");
  m_OptionsFrame->AddFrame(TotalEnergyLabel, TotalEnergyLayout);

  m_SingleSiteOnly = new TGCheckButton(m_OptionsFrame, "Only keep single site events (one strip on each side)", 1);
  m_SingleSiteOnly->SetOn(dynamic_cast<MModuleEventFilter*>(m_Module)->GetSingleSiteOnly());
  TGLayoutHints* Label4Layout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
  m_OptionsFrame->AddFrame(m_SingleSiteOnly, Label4Layout);
  
  
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
  dynamic_cast<MModuleEventFilter*>(m_Module)->SetSingleSiteOnly(m_SingleSiteOnly->IsOn());

  return true;
}


// MGUIOptionsEventFilter: the end...
////////////////////////////////////////////////////////////////////////////////
