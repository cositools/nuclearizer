/*
 * MGUIOptionsDEESMEX.cxx
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
#include "MGUIOptionsDEESMEX.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleDEESMEX.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsDEESMEX)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDEESMEX::MGUIOptionsDEESMEX(MModule* Module)
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDEESMEX::~MGUIOptionsDEESMEX()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsDEESMEX::Create()
{
  PreCreate();

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsDEESMEX::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
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


bool MGUIOptionsDEESMEX::OnApply()
{
  // Modify this to store the data in the module!
  
  return true;
}


// MGUIOptionsDEESMEX: the end...
////////////////////////////////////////////////////////////////////////////////
