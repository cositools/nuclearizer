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
  m_TabTitle = "TAC Calibration";
  
  // Set the histogram arrangment
  // SetTACHistogramArrangement(1, 1);

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
  for (auto H: m_TACHistograms) {  
    (H.second)->Reset();
  }
  m_Mutex.UnLock();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::SetTACHistogramArrangement(const vector<unsigned int> DetIDs)
{
  // Take in the list of detector IDs and determine the number in X and number in Y
  // Update the variable m_DetectorMap.
  m_Mutex.Lock();

  unsigned int column = 0;
  unsigned int row = 0;

  unsigned int max_columns = 4;

  unsigned int NDetectors = DetIDs.size();
  cout<<"MGUIExpoTACcut::SetTACHistogramArrangement: Number of detectors:"<< NDetectors<<endl;

  for (unsigned int i=0; i < NDetectors; ++i) {
    // iterate over detector IDs, make the map from ID to plot position, and initialize the histograms
    if ((i % max_columns) == 0) {
      ++row;
      vector<unsigned int> new_row;
      m_DetectorMap.push_back(new_row);
      column = 1;
    }

    unsigned int DetID = DetIDs.at(i);
    m_DetectorMap[row-1].push_back(DetID);

    TH1D* TAC = new TH1D("", "TAC", m_NBins[DetID], m_Min[DetID], m_Max[DetID]);
    TAC->SetXTitle("Calibrated TAC (ns)");
    TAC->SetYTitle("counts");
    TAC->SetFillColor(kAzure+7);
    
    m_TACHistograms[DetID] = TAC;
    // m_TACCanvases[DetID] = 0;

    ++column;
  }

  if (NDetectors < max_columns) {
    m_NColumns = NDetectors; 
  } else {
    m_NColumns=max_columns;
  }

  m_NRows = (NDetectors/max_columns) + 1; 
  m_Mutex.UnLock();
}

  
////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::SetTACHistogramParameters(unsigned int DetID, unsigned int NBins, double MinimumTAC, double MaximumTAC)
{
  // Set the energy histogram parameters 

  m_Mutex.Lock();

  m_NBins[DetID] = NBins;
  m_Min[DetID] = MinimumTAC;
  m_Max[DetID] = MaximumTAC;
  TH1D* H = m_TACHistograms[DetID];
  H->SetBins(NBins, MinimumTAC, MaximumTAC);

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::SetTACHistogramName(unsigned int DetID, MString Name) 
{
  // Set the title of the histogram
  
  m_Mutex.Lock();

  if (m_TACHistograms.find(DetID) != m_TACHistograms.end()) {
    m_TACHistograms[DetID]->SetTitle(Name);
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::AddTAC(unsigned int DetID, double TAC)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  if (m_TACHistograms.find(DetID) != m_TACHistograms.end()) {
    m_TACHistograms[DetID]->Fill(TAC);
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::Create()
{
  // Add the GUI options here

  // Do not create it twice!
  if (m_IsCreated == true) return;
  
  m_Mutex.Lock();
  
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);

  for (unsigned int y = 0; y < m_DetectorMap.size(); ++y) {
    TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
    AddFrame(HFrame, CanvasLayout);

    for (unsigned int x = 0; x < m_DetectorMap[y].size(); ++x) {
      unsigned int DetID = m_DetectorMap[y][x];
      TRootEmbeddedCanvas* TACCanvas = new TRootEmbeddedCanvas("TAC", HFrame, 100, 100);
      HFrame->AddFrame(TACCanvas, CanvasLayout);
      m_TACCanvases[DetID] = TACCanvas;

      TACCanvas->GetCanvas()->cd();
      m_TACHistograms[DetID]->Draw("colz");
      TACCanvas->GetCanvas()->Update();
    }
  }
  
  m_IsCreated = true;

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::Update()
{
  //! Update the frame

  m_Mutex.Lock();

  double Max = 0;
  // for (auto H : m_TACHistograms) {
  for (const auto dethistpair : m_TACHistograms) {
    TH1D* H = dethistpair.second;
    for (int bx = 2; bx < H->GetNbinsX(); ++bx) { // Skip first and last
      if (Max < H->GetBinContent(bx)) {
        Max = H->GetBinContent(bx);
      }
    }
  }
  Max *= 1.1;
  for (const auto dethistpair : m_TACHistograms) {
    TH1D* H = dethistpair.second;
    H->SetMaximum(Max);
  }
  
  for (auto C : m_TACCanvases) {

    (C.second)->GetCanvas()->Modified();
    (C.second)->GetCanvas()->Update();
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoTACcut::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  TCanvas* P = new TCanvas();
  P->Divide(m_NColumns, m_NRows);
  for (unsigned int y = 0; y < m_DetectorMap.size(); ++y) {
    for (unsigned int x = 0; x < m_DetectorMap[y].size(); ++x) {
      unsigned int DetID = m_DetectorMap[y][x];
      P->cd((x+1) + m_NColumns*y);
      m_TACHistograms[DetID]->DrawCopy("colz");
    }
  }
  P->SaveAs(FileName);
  delete P;

  m_Mutex.UnLock();
}
