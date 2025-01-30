/*
 * MGUIExpoStripPairingStripHits.cxx
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
#include "MGUIExpoStripPairingStripHits.h"

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
ClassImp(MGUIExpoStripPairingStripHits)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoStripPairingStripHits::MGUIExpoStripPairingStripHits(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "Strip Pairing StripHits";

  // Add all histograms and canvases below
  m_StripHits = new TH2D("", "Strip pairing: Strip Hit distribution LV vs. HV side", 10, 0.5, 10.5, 10, 0.5, 10.5);
  m_StripHits->SetXTitle("Strip Hits LV Side");
  m_StripHits->SetYTitle("Strip Hits HV Side");
  m_StripHits->SetZTitle("Hits");
  m_StripHits->SetFillColor(kAzure+7);

  m_StripHitsCanvas = 0;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoStripPairingStripHits::~MGUIExpoStripPairingStripHits()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingStripHits::Reset()
{
  //! Reset the data in the UI

  m_Mutex.Lock();
  
  m_StripHits->Reset();
  
  m_Mutex.UnLock();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingStripHits::SetStripHitsHistogramParameters(int NBins, double Min, double Max)
{
  // Set the energy histogram parameters 

  m_Mutex.Lock();
  
  m_StripHits->SetBins(NBins, Min, Max, NBins, Min, Max);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingStripHits::AddStripHits(double LVStripHits, double HVStripHits)
{
  // Add data to the energy histogram

  m_Mutex.Lock();
  
  m_StripHits->Fill(LVStripHits, HVStripHits);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingStripHits::Create()
{
  // Add the GUI options here
  
  // Do not create it twice!
  if (m_IsCreated == true) return;
  
  m_Mutex.Lock();

  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,
                                                  2, 2, 2, 2);

  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, CanvasLayout);

  
  m_StripHitsCanvas = new TRootEmbeddedCanvas("StripHits", HFrame, 100, 100);
  HFrame->AddFrame(m_StripHitsCanvas, CanvasLayout);

  m_StripHitsCanvas->GetCanvas()->cd();
  m_StripHitsCanvas->GetCanvas()->SetGridy();
  m_StripHitsCanvas->GetCanvas()->SetGridx();
  m_StripHits->Draw("colz");
  m_StripHitsCanvas->GetCanvas()->Update();
  
  m_IsCreated = true;
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingStripHits::Update()
{
  //! Update the frame

  m_Mutex.Lock();
  
  if (m_StripHitsCanvas != 0) {
    m_StripHitsCanvas->GetCanvas()->Modified();
    m_StripHitsCanvas->GetCanvas()->Update();
  }
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingStripHits::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();
  
  m_StripHitsCanvas->GetCanvas()->SaveAs(FileName);
  
  m_Mutex.UnLock();
}


// MGUIExpoStripPairingStripHits: the end...
////////////////////////////////////////////////////////////////////////////////
