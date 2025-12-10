/*
 * MGUIExpoDepthCalibration2024.cxx
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
#include "MGUIExpoDepthCalibration2024.h"

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
ClassImp(MGUIExpoDepthCalibration2024)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoDepthCalibration2024::MGUIExpoDepthCalibration2024(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "Depth Calibration";
  
  // Set the histogram arrangment
  // SetDepthHistogramArrangement(1, 1);

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoDepthCalibration2024::~MGUIExpoDepthCalibration2024()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration2024::Reset()
{
  //! Reset the data in the UI

  m_Mutex.Lock();
  for (auto H: m_DepthHistograms) {  
    (H.second)->Reset();
  }
  m_Mutex.UnLock();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration2024::SetDepthHistogramArrangement(vector<unsigned int>* DetIDs)
{
  // Take in the list of detector IDs and determine the number in X and number in Y
  // Update the variable m_DetectorMap.
  m_Mutex.Lock();

  unsigned int column = 0;
  unsigned int row = 0;

  unsigned int max_columns = 4;

  unsigned int NDetectors = DetIDs->size();
  cout<<"MGUIExpoDepthCalibration2024::SetDepthHistogramArrangement: Number of detectors:" << NDetectors<<endl;

  for ( unsigned int i=0; i< NDetectors; ++i ){
    // iterate over detector IDs, make the map from ID to plot position, and initialize the histograms
    if ( (i % max_columns) == 0 ){
      ++row;
      vector<unsigned int> new_row;
      m_DetectorMap.push_back(new_row);
      column = 1;
    }

    unsigned int DetID = DetIDs->at(i);
    m_DetectorMap[row-1].push_back(DetID);

    TH1D* Depth = new TH1D("", "Depth", m_NBins[DetID], m_Min[DetID], m_Max[DetID]);
    Depth->SetXTitle("Depth [cm]");
    Depth->SetYTitle("counts");
    Depth->SetFillColor(kAzure+7);
    
    m_DepthHistograms[DetID] = Depth;
    // m_DepthCanvases[DetID] = 0;

    ++column;
  }

  if ( NDetectors < max_columns ){
    m_NColumns = NDetectors; 
  }
  else{
    m_NColumns=max_columns;
  }

  m_NRows = (NDetectors/max_columns) + 1; 
  
  m_Mutex.UnLock();
}

  
////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration2024::SetDepthHistogramParameters(unsigned int DetID, unsigned int NBins, double DepthMin, double DepthMax)
{
  // Set the energy histogram parameters 

  m_Mutex.Lock();

  m_NBins[DetID] = NBins;
  m_Min[DetID] = DepthMin;
  m_Max[DetID] = DepthMax;
  TH1D* H = m_DepthHistograms[DetID];
  H->SetBins(NBins, DepthMin, DepthMax);

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration2024::SetDepthHistogramName(unsigned int DetID, MString Name) 
{
  // Set the title of the histogram
  
  m_Mutex.Lock();

  if (m_DepthHistograms.find(DetID) != m_DepthHistograms.end()) {
    m_DepthHistograms[DetID]->SetTitle(Name);
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration2024::AddDepth(unsigned int DetID, double Depth)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  if (m_DepthHistograms.find(DetID) != m_DepthHistograms.end()) {
    m_DepthHistograms[DetID]->Fill(Depth);
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration2024::Create()
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
      TRootEmbeddedCanvas* DepthCanvas = new TRootEmbeddedCanvas("Depth", HFrame, 100, 100);
      HFrame->AddFrame(DepthCanvas, CanvasLayout);
      m_DepthCanvases[DetID] = DepthCanvas;

      DepthCanvas->GetCanvas()->cd();
      m_DepthHistograms[DetID]->Draw("colz");
      DepthCanvas->GetCanvas()->Update();
    }
  }
  
  m_IsCreated = true;

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration2024::Update()
{
  //! Update the frame

  m_Mutex.Lock();

  double Max = 0;
  // for (auto H : m_DepthHistograms) {
  for ( const auto dethistpair : m_DepthHistograms ){
    TH1D* H = dethistpair.second;
    for (int bx = 2; bx < H->GetNbinsX(); ++bx) { // Skip first and last
      if (Max < H->GetBinContent(bx)) {
        Max = H->GetBinContent(bx);
      }
    }
  }
  Max *= 1.1;
  for ( const auto dethistpair : m_DepthHistograms ){
    TH1D* H = dethistpair.second;
    H->SetMaximum(Max);
  }
  
  for (auto C : m_DepthCanvases) {

    (C.second)->GetCanvas()->Modified();
    (C.second)->GetCanvas()->Update();
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration2024::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  TCanvas* P = new TCanvas();
  P->Divide(m_NColumns, m_NRows);
  for (unsigned int y = 0; y < m_DetectorMap.size(); ++y) {
    for (unsigned int x = 0; x < m_DetectorMap[y].size(); ++x) {
      unsigned int DetID = m_DetectorMap[y][x];
      P->cd((x+1) + m_NColumns*y);
      m_DepthHistograms[DetID]->DrawCopy("colz");
    }
  }
  P->SaveAs(FileName);
  delete P;

  m_Mutex.UnLock();
}


// MGUIExpoDepthCalibration2024: the end...
////////////////////////////////////////////////////////////////////////////////
