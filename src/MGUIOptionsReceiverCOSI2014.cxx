/*
 * MGUIOptionsReceiverCOSI2014.cxx
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
#include "MGUIOptionsReceiverCOSI2014.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModule.h"
#include "MNCTModuleReceiverCOSI2014.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptionsReceiverCOSI2014)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsReceiverCOSI2014::MGUIOptionsReceiverCOSI2014(MNCTModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsReceiverCOSI2014::~MGUIOptionsReceiverCOSI2014()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsReceiverCOSI2014::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_DistributorName = new MGUIEEntry(this, "Distributor name/IP: ", false,
                              dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->GetDistributorName());
  AddFrame(m_DistributorName, LabelLayout);

  m_DistributorPort = new MGUIEEntry(this, "Distributor listening port: ", false,
                              dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->GetDistributorPort());
  AddFrame(m_DistributorPort, LabelLayout);

  m_DistributorStreamID = new MGUIEEntry(this, "Stream ID: ", false,
                              dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->GetDistributorStreamID());
  AddFrame(m_DistributorStreamID, LabelLayout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsReceiverCOSI2014::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsReceiverCOSI2014::OnApply()
{
	// Modify this to store the data in the module!

  dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetDistributorName(m_DistributorName->GetAsString());
  dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetDistributorPort(m_DistributorPort->GetAsInt());
  dynamic_cast<MNCTModuleReceiverCOSI2014*>(m_Module)->SetDistributorStreamID(m_DistributorStreamID->GetAsString());
	
	return true;
}


// MGUIOptionsReceiverCOSI2014: the end...
////////////////////////////////////////////////////////////////////////////////
