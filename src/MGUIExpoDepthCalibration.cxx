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


#ifdef ___CINT___
ClassImp(MGUIExpoDepthCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoDepthCalibration::MGUIExpoDepthCalibration() : MGUIExpo()
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

  for (auto H: m_DepthHistograms) {  
    H->Reset();
  }
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::SetDepthHistogramArrangement(unsigned int NDetectorsX, unsigned int NDetectorsY)
{
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
}

  
////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::SetDepthHistogramParameters(unsigned int NBins, double Min, double Max)
{
  // Set the energy histogram parameters 

  for (auto H: m_DepthHistograms) {
    m_NBins = NBins;
    m_Min = Min;
    m_Max = Max;
    H->SetBins(m_NBins, m_Min, m_Max);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::SetDepthHistogramName(unsigned int DetectorID, MString Name) 
{
  // Set the title of the histogram
  
  if (DetectorID < m_DepthHistograms.size()) {
    m_DepthHistograms[DetectorID]->SetTitle(Name);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::AddDepth(unsigned int DetectorID, double Depth)
{
  // Add data to the energy histogram

  cout<<DetectorID<<":"<<m_DepthHistograms.size()<<endl;
  if (DetectorID < m_DepthHistograms.size()) {
    m_DepthHistograms[DetectorID]->Fill(Depth);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::Create()
{
  // Add the GUI options here
  
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
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::Update()
{
  //! Update the frame

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
      gSystem->ProcessEvents();
    }
  }
}


// MGUIExpoDepthCalibration: the end...
////////////////////////////////////////////////////////////////////////////////
