/*
 * MGUIExpoCombinedViewer.cxx
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
#include "MGUIExpoCombinedViewer.h"

// Standard libs:
#include <iomanip>
using namespace std;

// ROOT libs:
#include <KeySymbols.h>
#include <TApplication.h>
#include <TGPicture.h>
#include <TStyle.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGWindow.h>
#include <TGFrame.h>
#include <TString.h>
#include <TGClient.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"
#include "MGUIEFileSelector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIExpoCombinedViewer)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoCombinedViewer::MGUIExpoCombinedViewer() : MGUIDialog(gClient->GetRoot(), gClient->GetRoot())
{
  // No deep clean-up allowed in this function!
  SetCleanup(kNoCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoCombinedViewer::~MGUIExpoCombinedViewer()
{
  // Deep Cleanup automatically deletes all used GUI elements
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoCombinedViewer::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Expo - combined viewer");  

   
  // The subtitle
  //AddSubTitle(TString("Diagnostics"));

  m_MainTab = new TGTab(this, m_FontScaler*900, m_FontScaler*550);
  m_MainTabLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 10, 10, 20, 0);
  AddFrame(m_MainTab, m_MainTabLayout);

  for (unsigned int d = 0; d < m_Expos.size(); ++d) {
    m_Expos[d]->ReparentWindow(m_MainTab);
    m_Expos[d]->Create();
    m_MainTab->AddTab(m_Expos[d]->GetTabTitle(), m_Expos[d]);
  }

  // The buttons
  AddButtons(c_Ok | c_Apply, true);
  

  m_ApplyButton->SetText("Update");
  m_OKButton->SetText("Close");

  // Give this element the default size of its content:
  Resize(m_FontScaler*900, m_FontScaler*580); 

  MapSubwindows();
  MapWindow();  
  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoCombinedViewer::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Process the messages for this application

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case e_Ok:
        Status = OnCancel();
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
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoCombinedViewer::NeedsUpdate()
{
  //! Return true if we need an update
 
  for (unsigned int d = 0; d < m_Expos.size(); ++d) {
    if (m_Expos[d]->NeedsUpdate() == true) {
      return true;
    }
  }
  
  return false;
}
 
  
////////////////////////////////////////////////////////////////////////////////


void MGUIExpoCombinedViewer::Update()
{
  //! Update all tabs

  for (unsigned int d = 0; d < m_Expos.size(); ++d) {
    m_Expos[d]->Update();
  }
}



////////////////////////////////////////////////////////////////////////////////


void MGUIExpoCombinedViewer::CloseWindow()
{
  // When the x is pressed, this function is called.
  // We do not delete it automatically!

  UnmapWindow();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoCombinedViewer::OnApply()
{
  // The Apply button has been pressed

  Update();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoCombinedViewer::OnCancel()
{
  // The Apply button has been pressed

  CloseWindow();

  return true;
}


// MGUIExpoCombinedViewer: the end...
////////////////////////////////////////////////////////////////////////////////
