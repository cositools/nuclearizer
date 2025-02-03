/*
 * MGUIExpoStripPairingHits.cxx
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
#include "MGUIExpoStripPairingHits.h"

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
ClassImp(MGUIExpoStripPairingHits)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoStripPairingHits::MGUIExpoStripPairingHits(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "Strip Pairing Hits";

  // Add all histograms and canvases below

  m_Hits = new TH1D("", "Strip pairing: Number of Hits per Event", 5, 1, 5);
  m_Hits->SetXTitle("Number of Hits");
  m_Hits->SetYTitle("Number of Events");
  m_Hits->SetFillColor(kAzure+7);

  m_HitsCanvas = 0;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoStripPairingHits::~MGUIExpoStripPairingHits()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingHits::Reset()
{
  //! Reset the data in the UI

  m_Mutex.Lock();
  
  m_Hits->Reset();
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingHits::AddHits(int NHits)
{
  // Add data to the energy histogram

  m_Mutex.Lock();
  
  m_Hits->Fill(NHits);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingHits::SetHitsHistogramParameters(int NBins, double Min, double Max)
{
  // Set the energy histogram parameters 

  m_Mutex.Lock();
  
  m_Hits->SetBins(NBins, Min, Max);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingHits::Create()
{
  // Add the GUI options here
  
  // Do not create it twice!
  if (m_IsCreated == true) return;
  
  m_Mutex.Lock();

  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,
                                                  2, 2, 2, 2);

  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, CanvasLayout);

  m_HitsCanvas = new TRootEmbeddedCanvas("Hits", HFrame, 100, 100);
  HFrame->AddFrame(m_HitsCanvas, CanvasLayout);

  m_HitsCanvas->GetCanvas()->cd();
  m_HitsCanvas->GetCanvas()->SetGridy();
  m_HitsCanvas->GetCanvas()->SetGridx();
  m_Hits->Draw("colz");
  m_HitsCanvas->GetCanvas()->Update();
  
  m_IsCreated = true;
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairingHits::Update()
{
  //! Update the frame

  m_Mutex.Lock();

  if (m_HitsCanvas != 0) {
    m_HitsCanvas->GetCanvas()->Modified();
    m_HitsCanvas->GetCanvas()->Update();
  }
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////



void MGUIExpoStripPairingHits::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();
  
  m_HitsCanvas->GetCanvas()->SaveAs(FileName);
  
  m_Mutex.UnLock();
}



// MGUIExpoStripPairingHits: the end...
////////////////////////////////////////////////////////////////////////////////
