/*
 * MGUIOptionsStripPairingMultiRoundChiSquare.cxx
 *
 *
 * Copyright (C) by Julian Gerber
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


////////////////////////////////////////////////////////////////////////////////
//
// MGUIOptionsStripPairingMultiRoundChiSquare
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIOptionsStripPairingMultiRoundChiSquare.h"

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
#include "MModuleStripPairingMultiRoundChiSquare.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsStripPairingMultiRoundChiSquare)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsStripPairingMultiRoundChiSquare::MGUIOptionsStripPairingMultiRoundChiSquare(MModule* Module)
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsStripPairingMultiRoundChiSquare::~MGUIOptionsStripPairingMultiRoundChiSquare()
{
  // kDeepCleanup is activated
}

///////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsStripPairingMultiRoundChiSquare::Create()
{
  PreCreate();
    
  TGLayoutHints* FirstLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 30, 2);

  m_MaximumStrips = new MGUIEEntry(m_OptionsFrame,
                                           "Choose the maximum number of strips on either side:",
                                           false, dynamic_cast<MModuleStripPairingMultiRoundChiSquare*>(m_Module)->GetMaximumStrips());
      m_OptionsFrame->AddFrame(m_MaximumStrips, FirstLayout);
  
  PostCreate();
}
       
                               
///////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsStripPairingMultiRoundChiSquare::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


///////////////////////////////////////////////////////////////////////////////////

                               
bool MGUIOptionsStripPairingMultiRoundChiSquare::OnApply()
{
  dynamic_cast<MModuleStripPairingMultiRoundChiSquare*>(m_Module)->SetMaximumStrips(m_MaximumStrips->GetAsDouble());
    
  return true;
}


// MGUIOptionsStripPairingMultiRoundChiSquare.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
