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
  m_TabTitle = "Depth";
  m_SideSelector = nullptr;
  m_StripMinEntry = nullptr;
  m_StripMaxEntry = nullptr;
  m_SelectedSide = 0;
  m_StripMin = 0;
  m_StripMax = 63; 
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
  for (auto H: m_DepthHistograms) (H.second)->Reset();
  for (auto H: m_RawDepthHistograms) (H.second)->Reset();
  for (auto& H : m_RawDepthPerStrip) H.second->Reset();
  for (auto& H : m_DepthPerStrip) H.second->Reset();
  m_Mutex.UnLock();
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::SetDepthHistogramArrangement(vector<unsigned int>* DetIDs)
{
  // Take in the list of detector IDs and determine the number in X and number in Y
  // Update the variable m_DetectorMap.
  m_Mutex.Lock();

  unsigned int column = 0;
  unsigned int row = 0;

  unsigned int max_columns = 4;

  unsigned int NDetectors = DetIDs->size();
  cout<<"MGUIExpoDepthCalibration::SetDepthHistogramArrangement: Number of detectors:" << NDetectors<<endl;

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
    Depth->SetFillColorAlpha(kAzure+7,0.5);
    Depth->SetFillStyle(3001);
    m_DepthHistograms[DetID] = Depth;

    TH1D* RawDepth = new TH1D("", "Depth", m_NBins[DetID], m_Min[DetID], m_Max[DetID]);
    RawDepth->SetXTitle("Depth [cm]");
    RawDepth->SetYTitle("counts");
    RawDepth->SetFillColorAlpha(kRed,0.5);
    Depth->SetFillStyle(3001);
    m_RawDepthHistograms[DetID] = RawDepth;// m_DepthCanvases[DetID] = 0;
    
    // Create per-strip histograms
    for (int side = 0; side < 2; side++) {
      for (int strip = 0; strip < 64; strip++) {
        int key = GetStripKey(DetID, side, strip);
        m_RawDepthPerStrip[key] = new TH1D("", "", m_NBins[DetID], m_Min[DetID], m_Max[DetID]);
        m_DepthPerStrip[key] = new TH1D("", "", m_NBins[DetID], m_Min[DetID], m_Max[DetID]);
      }
    }

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


void MGUIExpoDepthCalibration::SetDepthHistogramParameters(unsigned int DetID, unsigned int NBins, double DepthMin, double DepthMax)
{
  m_Mutex.Lock();

  m_NBins[DetID] = NBins;
  m_Min[DetID] = DepthMin;
  m_Max[DetID] = DepthMax;

  // Only update bins if histograms already exist
  if (m_DepthHistograms.count(DetID) > 0 && m_DepthHistograms[DetID] != nullptr) {
    m_DepthHistograms[DetID]->SetBins(NBins, DepthMin, DepthMax);
  }
  if (m_RawDepthHistograms.count(DetID) > 0 && m_RawDepthHistograms[DetID] != nullptr) {
    m_RawDepthHistograms[DetID]->SetBins(NBins, DepthMin, DepthMax);
  }

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

void MGUIExpoDepthCalibration::AddRawDepth(unsigned int DetID, int LVStrip, int HVStrip, double Depth)
{
  m_Mutex.Lock();
  int lvKey = GetStripKey(DetID, 0, LVStrip);
  int hvKey = GetStripKey(DetID, 1, HVStrip);
  if (m_RawDepthPerStrip.count(lvKey) > 0) m_RawDepthPerStrip[lvKey]->Fill(Depth);
  if (m_RawDepthPerStrip.count(hvKey) > 0) m_RawDepthPerStrip[hvKey]->Fill(Depth);
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::AddDepth(unsigned int DetID, int LVStrip, int HVStrip, double Depth)
{
  m_Mutex.Lock();
  int lvKey = GetStripKey(DetID, 0, LVStrip);
  int hvKey = GetStripKey(DetID, 1, HVStrip);
  if (m_DepthPerStrip.count(lvKey) > 0) m_DepthPerStrip[lvKey]->Fill(Depth);
  if (m_DepthPerStrip.count(hvKey) > 0) m_DepthPerStrip[hvKey]->Fill(Depth);
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoDepthCalibration::OnStripSelectionChanged()
{
  m_Mutex.Lock();

  m_SelectedSide = m_SideSelector->GetSelected();
  m_StripMin = (int)m_StripMinEntry->GetNumber();
  m_StripMax = (int)m_StripMaxEntry->GetNumber();
  if (m_StripMin < 0) m_StripMin = 0;
  if (m_StripMax > 63) m_StripMax = 63;
  if (m_StripMin > m_StripMax) m_StripMin = m_StripMax;

  RebuildDisplayHistograms();

  // Rescale Y-axis
  double Max = 0;
  for (const auto& pair : m_DepthHistograms) {
    TH1D* H = pair.second;
    for (int bx = 2; bx < H->GetNbinsX(); ++bx) {
      if (Max < H->GetBinContent(bx)) Max = H->GetBinContent(bx);
    }
  }
  for (const auto& pair : m_RawDepthHistograms) {
    TH1D* H = pair.second;
    for (int bx = 2; bx < H->GetNbinsX(); ++bx) {
      if (Max < H->GetBinContent(bx)) Max = H->GetBinContent(bx);
    }
  }
  Max *= 1.1;
  if (Max == 0) Max = 1.0;

  for (const auto& pair : m_DepthHistograms) pair.second->SetMaximum(Max);
  for (const auto& pair : m_RawDepthHistograms) pair.second->SetMaximum(Max);

  // Redraw canvases
  for (auto& C : m_DepthCanvases) {
    C.second->GetCanvas()->Modified();
    C.second->GetCanvas()->Update();
  }

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoDepthCalibration::RebuildDisplayHistograms()
{ 
  cout << "RebuildDisplayHistograms called" << endl;
  for (auto& pair : m_DepthHistograms) pair.second->Reset();
  for (auto& pair : m_RawDepthHistograms) pair.second->Reset();

  for (auto& pair : m_DepthHistograms) {
    unsigned int DetID = pair.first;
    for (int s = m_StripMin; s <= m_StripMax; s++) {
      int key = GetStripKey(DetID, m_SelectedSide, s);
      if (m_DepthPerStrip.count(key) > 0)
        m_DepthHistograms[DetID]->Add(m_DepthPerStrip[key]);
        cout << " added "<< key << ": " << m_DepthPerStrip[key]->GetEntries() << " entries" << endl;
      if (m_RawDepthPerStrip.count(key) > 0)
        m_RawDepthHistograms[DetID]->Add(m_RawDepthPerStrip[key]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::Create()
{
  // Add the GUI options here

  // Do not create it twice!
  if (m_IsCreated == true) return;
  
  m_Mutex.Lock();

  TGLayoutHints* ExpandLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);
  TGLayoutHints* RowLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 5, 0, 0);
  TGLayoutHints* EntryLayout = new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 2, 0, 0);

  TGHorizontalFrame* MainHFrame = new TGHorizontalFrame(this);
  AddFrame(MainHFrame, ExpandLayout);

  // === Left: Controls ===
  TGVerticalFrame* ControlFrame = new TGVerticalFrame(MainHFrame, 180, 400);
  MainHFrame->AddFrame(ControlFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 5, 5, 5, 5));

  // Strip selection: Side dropdown + min + max
  TGHorizontalFrame* StripRow = new TGHorizontalFrame(ControlFrame);
  ControlFrame->AddFrame(StripRow, RowLayout);

  m_SideSelector = new TGComboBox(StripRow);
  m_SideSelector->AddEntry("LV", 0);
  m_SideSelector->AddEntry("HV", 1);
  m_SideSelector->Select(0);
  m_SideSelector->Resize(45, 20);
  //m_SideSelector->Connect("Selected(Int_t)", "MGUIExpoDepthCalibration", this, "OnStripSelectionChanged()");
  StripRow->AddFrame(m_SideSelector, EntryLayout);

  m_StripMinEntry = new TGNumberEntry(StripRow, 0, 3, -1,
                                       TGNumberFormat::kNESInteger,
                                       TGNumberFormat::kNEANonNegative,
                                       TGNumberFormat::kNELLimitMinMax, 0, 63);
  //m_StripMinEntry->Connect("ValueSet(Long_t)", "MGUIExpoDepthCalibration", this, "OnStripSelectionChanged()");
  //m_StripMinEntry->GetNumberEntry()->Connect("ReturnPressed()", "MGUIExpoDepthCalibration", this, "OnStripSelectionChanged()");
  m_StripMinEntry->Resize(45, 20);
  StripRow->AddFrame(m_StripMinEntry, EntryLayout);

  m_StripMaxEntry = new TGNumberEntry(StripRow, 63, 3, -1,
                                       TGNumberFormat::kNESInteger,
                                       TGNumberFormat::kNEANonNegative,
                                       TGNumberFormat::kNELLimitMinMax, 0, 63);
  //m_StripMaxEntry->Connect("ValueSet(Long_t)", "MGUIExpoDepthCalibration", this, "OnStripSelectionChanged()");
  //m_StripMaxEntry->GetNumberEntry()->Connect("ReturnPressed()", "MGUIExpoDepthCalibration", this, "OnStripSelectionChanged()");
  m_StripMaxEntry->Resize(45, 20);
  StripRow->AddFrame(m_StripMaxEntry, EntryLayout);

  // Update
  TGTextButton* m_UpdateSelectionButton = new TGTextButton(ControlFrame, "Update Plot Selection", c_UpdateSelection);
  m_UpdateSelectionButton->Associate(this);
  ControlFrame->AddFrame(m_UpdateSelectionButton, RowLayout);

  // === Right: Plots ===
  TGVerticalFrame* PlotFrame = new TGVerticalFrame(MainHFrame);
  MainHFrame->AddFrame(PlotFrame, ExpandLayout);

  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);  

  for (unsigned int y = 0; y < m_DetectorMap.size(); ++y) {
    TGHorizontalFrame* HFrame = new TGHorizontalFrame(PlotFrame);
    PlotFrame->AddFrame(HFrame, CanvasLayout);
 
    for (unsigned int x = 0; x < m_DetectorMap[y].size(); ++x) {
      unsigned int DetID = m_DetectorMap[y][x];
      TRootEmbeddedCanvas* DepthCanvas = new TRootEmbeddedCanvas("Depth", HFrame, 100, 100);
      HFrame->AddFrame(DepthCanvas, CanvasLayout);
      m_DepthCanvases[DetID] = DepthCanvas;

      DepthCanvas->GetCanvas()->cd();
      m_RawDepthHistograms[DetID]->Draw("colz");
      m_DepthHistograms[DetID]->Draw("SAME");

      // Add legend
      if (x ==  0 && m_DepthHistograms[0] != nullptr) {
	
        m_Legend = new TLegend(0.15, 0.75, 0.35, 0.88);
	m_Legend->SetTextSize(0.03);
        m_Legend->AddEntry(m_RawDepthHistograms[DetID], "Raw Depth", "f");
	m_Legend->AddEntry(m_DepthHistograms[DetID],"Final Depth", "f");
        m_Legend->Draw();
      }

      DepthCanvas->GetCanvas()->Update();
    }
  }
  m_SelectedSide = m_SideSelector->GetSelected();
  m_StripMin = (int)m_StripMinEntry->GetNumber();
  m_StripMax = (int)m_StripMaxEntry->GetNumber();
  
  m_IsCreated = true;



  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoDepthCalibration::Update()
{
  //! Update the frame

  m_Mutex.Lock();

  if (m_SideSelector) m_SideSelector->SetEnabled(false);
  if (m_StripMinEntry) m_StripMinEntry->SetState(false);
  if (m_StripMaxEntry) m_StripMaxEntry->SetState(false);


  RebuildDisplayHistograms();

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
  for ( const auto dethistpair : m_RawDepthHistograms ){
    TH1D* H = dethistpair.second;
    for (int bx = 2; bx < H->GetNbinsX(); ++bx) { // Skip first and last
      if (Max < H->GetBinContent(bx)) {
        Max = H->GetBinContent(bx);
      }
    }
  }
  Max *= 1.1;
  for (const auto& pair : m_DepthHistograms) pair.second->SetMaximum(Max);
  for (const auto& pair : m_RawDepthHistograms) pair.second->SetMaximum(Max);
  
  for (auto C : m_DepthCanvases) {

    (C.second)->GetCanvas()->Modified();
    (C.second)->GetCanvas()->Update();
  }

  if (m_SideSelector) m_SideSelector->SetEnabled(true);
  if (m_StripMinEntry) m_StripMinEntry->SetState(true);
  if (m_StripMaxEntry) m_StripMaxEntry->SetState(true);

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

bool MGUIExpoDepthCalibration::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case c_UpdateSelection:
        OnStripSelectionChanged();
        return true;
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  return true;
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
      P->cd((x+1) + m_NColumns*y);
      m_RawDepthHistograms[DetID]->DrawCopy("colz");
      m_DepthHistograms[DetID]->DrawCopy("SAME");
    }
  }
  P->SaveAs(FileName);
  delete P;

  m_Mutex.UnLock();
}


// MGUIExpoDepthCalibration: the end...
////////////////////////////////////////////////////////////////////////////////
