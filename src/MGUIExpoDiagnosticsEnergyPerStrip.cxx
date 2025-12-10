/*
 * MGUIExpoDiagnosticsEnergyPerStrip.cxx
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
#include "MGUIExpoDiagnosticsEnergyPerStrip.h"

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
ClassImp(MGUIExpoDiagnosticsEnergyPerStrip)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoDiagnosticsEnergyPerStrip::MGUIExpoDiagnosticsEnergyPerStrip(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "Dignostics";

  // Add all histograms and canvases below
  m_StripHitsLV = new TH2D("", "Diagnostics: Strip Hits LV - counts vs energy", 64, -0.5, 63.5, 100, 0, 2000);
  m_StripHitsLV->SetXTitle("Strip ID");
  m_StripHitsLV->SetYTitle("Energy [keV]");
  m_StripHitsLV->SetZTitle("counts");
  m_StripHitsLV->SetFillColor(kAzure+7);

  m_StripHitsHV = new TH2D("", "Diagnostics: Strip Hits HV - counts vs energy", 64, -0.5, 63.5, 100, 0, 2000);
  m_StripHitsHV->SetXTitle("Strip ID");
  m_StripHitsLV->SetYTitle("Energy [keV]");
  m_StripHitsHV->SetZTitle("counts");
  m_StripHitsHV->SetFillColor(kAzure+7);

  m_DiagnosticsCanvas = nullptr;

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoDiagnosticsEnergyPerStrip::~MGUIExpoDiagnosticsEnergyPerStrip()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDiagnosticsEnergyPerStrip::Reset()
{
  //! Reset the data in the UI

  m_Mutex.Lock();
  
  m_StripHitsLV->Reset();
  m_StripHitsHV->Reset();

  m_Mutex.UnLock();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDiagnosticsEnergyPerStrip::SetStripHitHistogramParameters(int NBinsIDs, double MinIDs, double MaxIDs, int NBinsEnergy, double MinEnergy, double MaxEnergy)
{
  // Set the energy histogram parameters 

  m_Mutex.Lock();
  
  m_StripHitsLV->SetBins(NBinsIDs, MinIDs, MaxIDs, NBinsEnergy, MinEnergy, MaxEnergy);
  m_StripHitsHV->SetBins(NBinsIDs, MinIDs, MaxIDs, NBinsEnergy, MinEnergy, MaxEnergy);

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDiagnosticsEnergyPerStrip::AddHit(MHit* H)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  for (unsigned int h = 0; h < H->GetNStripHits(); ++h) {
    MStripHit* SH = H->GetStripHit(h);
    // TODO: Should be LV / HV - old version?
    if (SH->IsLowVoltageStrip() == true) {
      m_StripHitsLV->Fill(SH->GetStripID(), SH->GetEnergy());
    } else {
      m_StripHitsHV->Fill(SH->GetStripID(), SH->GetEnergy());
    }
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDiagnosticsEnergyPerStrip::AddStripHit(MStripHit* SH, bool UseEnergy)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  // TODO: Should be LV / HV - old version?
  if (SH->IsLowVoltageStrip() == true) {
    m_StripHitsLV->Fill(SH->GetStripID(), UseEnergy == true ? SH->GetEnergy() : SH->GetADCUnits());
  } else {
    m_StripHitsHV->Fill(SH->GetStripID(), UseEnergy == true ? SH->GetEnergy() : SH->GetADCUnits());
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDiagnosticsEnergyPerStrip::Create()
{
  // Add the GUI options here
  
  // Do not create it twice!
  if (m_IsCreated == true) return;
  
  m_Mutex.Lock();

  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,
                                                  2, 2, 2, 2);

  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, CanvasLayout);

  
  m_DiagnosticsCanvas = new TRootEmbeddedCanvas("Energies", HFrame, 100, 100);
  HFrame->AddFrame(m_DiagnosticsCanvas, CanvasLayout);
  m_DiagnosticsCanvas->GetCanvas();
  m_DiagnosticsCanvas->GetCanvas()->Divide(2,1);
  m_DiagnosticsCanvas->GetCanvas()->cd(1);
  m_StripHitsLV->Draw("colz");
  m_DiagnosticsCanvas->GetCanvas()->cd(2);
  m_StripHitsHV->Draw("colz");
  m_DiagnosticsCanvas->GetCanvas()->Update();
  
  m_IsCreated = true;
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDiagnosticsEnergyPerStrip::Update()
{
  //! Update the frame

  m_Mutex.Lock();
  
  if (m_DiagnosticsCanvas != nullptr) {
    m_DiagnosticsCanvas->GetCanvas()->Modified();
    m_DiagnosticsCanvas->GetCanvas()->Update();
  }
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDiagnosticsEnergyPerStrip::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();
  
  m_DiagnosticsCanvas->GetCanvas()->SaveAs(FileName);
  
  m_Mutex.UnLock();
}


// MGUIExpoDiagnosticsEnergyPerStrip: the end...
////////////////////////////////////////////////////////////////////////////////
