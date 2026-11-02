/*
 * MGUIExpoDepthCalibration.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Alex Lowell, Sean Pike.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer, Alex Lowell, Sean Pike.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


// Include the header:
#include "MGUIExpoDepthCalibration.h"

// Standard libs:
#include <map>

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
  
  // Set the histogram arrangment
  // SetDepthHistogramArrangement(1, 1);

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
    (H.second)->Reset();
  }
  m_Mutex.UnLock();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::SetDepthHistogramArrangement(vector<unsigned int>* DetIDs)
{
  // Place each histogram by its detector ID, four detectors per column as on the payload:
  //   -   4   8  12
  //   1   5   9  13
  //   2   6  10  14
  //   3   7  11  15
  // Slots of missing detectors stay empty, rows and columns without any detector are dropped
  m_Mutex.Lock();

  const unsigned int DetectorsPerColumn = 4;

  unsigned int NDetectors = DetIDs->size();
  if (g_Verbosity >= c_Info) cout<<"MGUIExpoDepthCalibration::SetDepthHistogramArrangement: Number of detectors:" << NDetectors<<endl;

  // Find the occupied rows and columns and number them in order
  map<unsigned int, unsigned int> Rows;
  map<unsigned int, unsigned int> Columns;
  for (unsigned int DetID: *DetIDs) {
    Rows[DetID % DetectorsPerColumn] = 0;
    Columns[DetID / DetectorsPerColumn] = 0;
  }
  unsigned int Index = 0;
  for (auto& R: Rows) R.second = Index++;
  Index = 0;
  for (auto& C: Columns) C.second = Index++;

  m_NRows = Rows.size();
  m_NColumns = Columns.size();
  m_DetectorMap.assign(m_NRows, vector<unsigned int>(m_NColumns, g_UnsignedIntNotDefined));

  for (unsigned int DetID: *DetIDs) {
    // Make the map from ID to plot position, and initialize the histograms
    m_DetectorMap[Rows[DetID % DetectorsPerColumn]][Columns[DetID / DetectorsPerColumn]] = DetID;

    TH1D* Depth = new TH1D("", MString("Detector ") + DetID, m_NBins[DetID], m_Min[DetID], m_Max[DetID]);
    Depth->SetXTitle("Depth [cm]");
    Depth->SetYTitle("counts");
    Depth->SetFillColor(kAzure+7);

    m_DepthHistograms[DetID] = Depth;
  }

  m_Mutex.UnLock();
}

  
////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::SetDepthHistogramParameters(unsigned int DetID, unsigned int NBins, double DepthMin, double DepthMax)
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


void MGUIExpoDepthCalibration::SetDepthHistogramName(unsigned int DetID, MString Name) 
{
  // Set the title of the histogram
  
  m_Mutex.Lock();

  if (m_DepthHistograms.find(DetID) != m_DepthHistograms.end()) {
    m_DepthHistograms[DetID]->SetTitle(Name);
  }

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::AddDepth(unsigned int DetID, double Depth)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  if (m_DepthHistograms.find(DetID) != m_DepthHistograms.end()) {
    m_DepthHistograms[DetID]->Fill(Depth);
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
  
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);

  for (unsigned int y = 0; y < m_DetectorMap.size(); ++y) {
    TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
    AddFrame(HFrame, CanvasLayout);

    for (unsigned int x = 0; x < m_DetectorMap[y].size(); ++x) {
      unsigned int DetID = m_DetectorMap[y][x];
      if (DetID == g_UnsignedIntNotDefined) {
        // Empty slot of the same size as the canvases, so the columns line up
        HFrame->AddFrame(new TGFrame(HFrame, 100, 100), CanvasLayout);
        continue;
      }
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


void MGUIExpoDepthCalibration::Update()
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


void MGUIExpoDepthCalibration::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  TCanvas* P = new TCanvas();
  P->Divide(m_NColumns, m_NRows);
  for (unsigned int y = 0; y < m_DetectorMap.size(); ++y) {
    for (unsigned int x = 0; x < m_DetectorMap[y].size(); ++x) {
      unsigned int DetID = m_DetectorMap[y][x];
      if (DetID == g_UnsignedIntNotDefined) continue;
      P->cd((x+1) + m_NColumns*y);
      m_DepthHistograms[DetID]->DrawCopy("colz");
    }
  }
  P->SaveAs(FileName);
  delete P;

  m_Mutex.UnLock();
}


// MGUIExpoDepthCalibration: the end...
////////////////////////////////////////////////////////////////////////////////
