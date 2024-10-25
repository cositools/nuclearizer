/*
 * MGUIExpoTACcut.cxx
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
#include "MGUIExpoTACcut.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <TString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TCanvas.h>

// MEGAlib libs:
#include "MStreams.h"



////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIExpoTACcut)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoTACcut::MGUIExpoTACcut(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "TAC Values";

  // Add all histograms and canvases below
  m_TAC = new TH1D("", "Spectrum combined hits", 200, 0, 1000);
  m_TAC->SetXTitle("TAC");
  m_TAC->SetYTitle("counts");
  m_TAC->SetFillColor(kAzure+7);

  m_TACCanvas = 0;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoTACcut::~MGUIExpoTACcut()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::Reset()
{
  //! Reset the data in the UI

  m_Mutex.Lock();

  m_TAC->Reset();
  
  m_Mutex.UnLock();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::SetTACHistogramParameters(int NBins, double Min, double Max)
{
  // Set the TAC histogram parameters 

  m_Mutex.Lock();

  m_TAC->SetBins(NBins, Min, Max);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::AddTAC(double TAC)
{
  // Add data to the TAC histogram

  m_Mutex.Lock();

  m_TAC->Fill(TAC);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::Export(const MString& FileName)
{
  // Add data to the TAC histogram

  m_Mutex.Lock();

  m_TACCanvas->GetCanvas()->SaveAs(FileName);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::Create()
{
  // Add the GUI options here

  // Do not create it twice!
  if (m_IsCreated == true) return;

  m_Mutex.Lock();
  
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,
                                                  2, 2, 2, 2);

  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, CanvasLayout);

  
  m_TACCanvas = new TRootEmbeddedCanvas("TAC", HFrame, 100, 100);
  HFrame->AddFrame(m_TACCanvas, CanvasLayout);

  m_TACCanvas->GetCanvas()->cd();
  m_TACCanvas->GetCanvas()->SetGridy();
  m_TACCanvas->GetCanvas()->SetGridx();
  m_TAC->Draw();
  m_TACCanvas->GetCanvas()->Update();
  
  m_IsCreated = true;
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::Update()
{
  //! Update the frame

  m_Mutex.Lock();

  if (m_TACCanvas != 0) {
    m_TACCanvas->GetCanvas()->Modified();
    m_TACCanvas->GetCanvas()->Update();
  }
  
  m_Mutex.UnLock();
}


// MGUIExpoTACcut: the end...
////////////////////////////////////////////////////////////////////////////////
