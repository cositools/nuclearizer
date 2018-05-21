/*
 * MGUIExpoDepthCalibration.cxx
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
#include "MGUIExpoDepthCalibration.h"

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
ClassImp(MGUIExpoDepthCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoDepthCalibration::MGUIExpoDepthCalibration(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "Depth Calibration";

  m_NBins = 30;
  m_Min = -0.75;
  m_Max = +0.75;
  
  // Set the histogram arrangment
  SetDepthHistogramArrangement(1, 1);

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoDepthCalibration::~MGUIExpoDepthCalibration()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::Reset()
{
  //! Reset the data in the UI

  m_Mutex.Lock();
  for (auto H: m_DepthHistograms) {  
    H->Reset();
  }
  m_Mutex.UnLock();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::SetDepthHistogramArrangement(unsigned int NDetectorsX, unsigned int NDetectorsY)
{
  m_Mutex.Lock();

  m_NDetectorsX = NDetectorsX; 
  m_NDetectorsY = NDetectorsY; 
  
  
  unsigned int Counter = 0;
  for (unsigned int x = 0; x < m_NDetectorsX; ++x) {
    for (unsigned int y = 0; y < m_NDetectorsY; ++y) {
      if (Counter < m_DepthHistograms.size()) {
        // Nothing yet
      } else {
        TH1D* Depth = new TH1D("", "Depth", m_NBins, m_Min, m_Max);
        Depth->SetXTitle("Depth [cm]");
        Depth->SetYTitle("counts");
        Depth->SetFillColor(kAzure+7);
        
        m_DepthHistograms.push_back(Depth);
        m_DepthCanvases.push_back(0);
      }
      ++Counter;
    }
  }
  m_Mutex.UnLock();
}

  
////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::SetDepthHistogramParameters(unsigned int NBins, double Min, double Max)
{
  // Set the energy histogram parameters 

  m_Mutex.Lock();

  for (auto H: m_DepthHistograms) {
    m_NBins = NBins;
    m_Min = Min;
    m_Max = Max;
    H->SetBins(m_NBins, m_Min, m_Max);
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::SetDepthHistogramName(unsigned int DetectorID, MString Name) 
{
  // Set the title of the histogram
  
  m_Mutex.Lock();

  if (DetectorID < m_DepthHistograms.size()) {
    m_DepthHistograms[DetectorID]->SetTitle(Name);
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::AddDepth(unsigned int DetectorID, double Depth)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  if (DetectorID < m_DepthHistograms.size()) {
    m_DepthHistograms[DetectorID]->Fill(Depth);
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::Create()
{
  // Add the GUI options here

  // Do not create it twice!
  if (m_IsCreated == true) return;
  
  m_Mutex.Lock();
  
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,                                                   2, 2, 2, 2);

  unsigned int Counter = 0;
  for (unsigned int y = 0; y < m_NDetectorsY; ++y) {
    TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
    AddFrame(HFrame, CanvasLayout);

    for (unsigned int x = 0; x < m_NDetectorsX; ++x) {
      TRootEmbeddedCanvas* DepthCanvas = new TRootEmbeddedCanvas("Depth", HFrame, 100, 100);
      HFrame->AddFrame(DepthCanvas, CanvasLayout);
      m_DepthCanvases[Counter] = DepthCanvas;

      DepthCanvas->GetCanvas()->cd();
      m_DepthHistograms[Counter]->Draw("colz");
      DepthCanvas->GetCanvas()->Update();
      ++Counter;
    }
  }
  
  m_IsCreated = true;

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::Update()
{
  //! Update the frame

  m_Mutex.Lock();

  double Max = 0;
  for (auto H : m_DepthHistograms) {
    for (int bx = 2; bx < H->GetNbinsX(); ++bx) { // Skip first and last
      if (Max < H->GetBinContent(bx)) {
        Max = H->GetBinContent(bx);
      }
    }
  }
  Max *= 1.1;
  for (auto H : m_DepthHistograms) {
    H->SetMaximum(Max);
  }
  
  for (auto C : m_DepthCanvases) {
    if (C != 0) {
      C->GetCanvas()->Modified();
      C->GetCanvas()->Update();
    }
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  TCanvas* P = new TCanvas();
  P->Divide(m_NDetectorsX, m_NDetectorsY);
  for (unsigned int y = 0; y < m_NDetectorsY; ++y) {
    for (unsigned int x = 0; x < m_NDetectorsX; ++x) {
      P->cd((x+1) + m_NDetectorsX*y);
      m_DepthHistograms[x + m_NDetectorsX*y]->DrawCopy("colz");
    }
  }
  P->SaveAs(FileName);
  delete P;

  m_Mutex.UnLock();
}


// MGUIExpoDepthCalibration: the end...
////////////////////////////////////////////////////////////////////////////////
