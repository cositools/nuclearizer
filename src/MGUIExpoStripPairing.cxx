/*
 * MGUIExpoStripPairing.cxx
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
#include "MGUIExpoStripPairing.h"

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
ClassImp(MGUIExpoStripPairing)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoStripPairing::MGUIExpoStripPairing(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "Strip Pairing Energies";

  // Add all histograms and canvases below
  m_Energies = new TH2D("", "Strip pairing: energy distribution LV vs. HV side", 1000, 0, 1000, 1000, 0, 1000);
  m_Energies->SetXTitle("Energy LV Side [keV]");
  m_Energies->SetYTitle("Energy HV Side [keV]");
  m_Energies->SetZTitle("counts");
  m_Energies->SetFillColor(kAzure+7);

  m_EnergiesCanvas = 0;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoStripPairing::~MGUIExpoStripPairing()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairing::Reset()
{
  //! Reset the data in the UI

  m_Mutex.Lock();
  
  m_Energies->Reset();
  
  m_Mutex.UnLock();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairing::SetEnergiesHistogramParameters(int NBins, double Min, double Max)
{
  // Set the energy histogram parameters 

  m_Mutex.Lock();
  
  m_Energies->SetBins(NBins, Min, Max, NBins, Min, Max);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairing::AddEnergies(double pEnergy, double nEnergy)
{
  // Add data to the energy histogram

  m_Mutex.Lock();
  
  m_Energies->Fill(pEnergy, nEnergy);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairing::Create()
{
  // Add the GUI options here
  
  // Do not create it twice!
  if (m_IsCreated == true) return;
  
  m_Mutex.Lock();

  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,
                                                  2, 2, 2, 2);

  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, CanvasLayout);

  
  m_EnergiesCanvas = new TRootEmbeddedCanvas("Energies", HFrame, 100, 100);
  HFrame->AddFrame(m_EnergiesCanvas, CanvasLayout);

  m_EnergiesCanvas->GetCanvas()->cd();
  m_EnergiesCanvas->GetCanvas()->SetGridy();
  m_EnergiesCanvas->GetCanvas()->SetGridx();
  m_Energies->Draw("colz");
  m_EnergiesCanvas->GetCanvas()->Update();
  
  m_IsCreated = true;
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairing::Update()
{
  //! Update the frame

  m_Mutex.Lock();
  
  if (m_EnergiesCanvas != 0) {
    m_EnergiesCanvas->GetCanvas()->Modified();
    m_EnergiesCanvas->GetCanvas()->Update();
  }
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoStripPairing::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();
  
  m_EnergiesCanvas->GetCanvas()->SaveAs(FileName);
  
  m_Mutex.UnLock();
}


// MGUIExpoStripPairing: the end...
////////////////////////////////////////////////////////////////////////////////
