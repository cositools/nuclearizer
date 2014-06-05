/*
 * MGUIOptions.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
#include "MGUIOptions.h"

// Standard libs:
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

// ROOT libs:
#include <KeySymbols.h>
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModule.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIOptions)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptions::MGUIOptions(MNCTModule* Module) 
  : TGTransientFrame(gClient->GetRoot(), gClient->GetRoot(), 320, 240), m_Module(Module)
{
  // standard constructor

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptions::~MGUIOptions()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptions::PreCreate()
{
  // Create the GUI options

  // We start with a name and an icon...
  SetWindowName("Module options");  

  // Main label
  const TGFont* lFont = gClient->GetFont("-*-helvetica-bold-r-*-*-14-*-*-*-*-*-iso8859-1");
  if (!lFont) lFont = gClient->GetResourcePool()->GetDefaultFont();
  FontStruct_t LargeFont = lFont->GetFontStruct();

  TGLabel* MainLabel = new TGLabel(this, MString("Options for module \"") + m_Module->GetName() + MString("\":"));
  MainLabel->SetTextFont(LargeFont);
  TGLayoutHints* MainLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);
	AddFrame(MainLabel, MainLabelLayout);
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptions::PostCreate()
{
  // OK and cancel buttons
	// Frame around the buttons:
	TGHorizontalFrame* ButtonFrame = new TGHorizontalFrame(this, 150, 25);
	TGLayoutHints* ButtonFrameLayout =	new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 10, 10, 10, 10);
	AddFrame(ButtonFrame, ButtonFrameLayout);
	
  // The buttons itself
	TGTextButton*	OKButton = new TGTextButton(ButtonFrame, "OK", e_Ok); 
  OKButton->Associate(this);
  TGLayoutHints* OKButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 20, 0, 0, 0);
	ButtonFrame->AddFrame(OKButton, OKButtonLayout);
	
	TGTextButton* CancelButton = new TGTextButton(ButtonFrame, "     Cancel     ", e_Cancel); 
  CancelButton->Associate(this);
  TGLayoutHints* CancelButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  ButtonFrame->AddFrame(CancelButton, CancelButtonLayout);


  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  MapSubwindows();
  MapWindow();  
  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptions::CloseWindow()
{
  // When the x is pressed, this function is called.

  DeleteWindow();
}



////////////////////////////////////////////////////////////////////////////////


bool MGUIOptions::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this window

	bool Status = true;
	
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case e_Ok:
				Status = OnOk();
        break;
        
      case e_Cancel:
        Status = OnCancel();
        break;

      case e_Apply:
				Status = OnApply();
        break;
        
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptions::OnOk()
{
	// The Ok button has been pressed

	if (OnApply() == true) {
		CloseWindow();
		return true;
	}
	
	return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptions::OnCancel()
{
	// The Cancel button has been pressed

	CloseWindow();

	return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptions::OnApply()
{
	// The Apply button has been pressed

	return true;
}


// MGUIOptions: the end...
////////////////////////////////////////////////////////////////////////////////
