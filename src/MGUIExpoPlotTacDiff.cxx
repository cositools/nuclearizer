#include "MGUIExpoPlotTacDiff.h"

#include <cmath>

#include <TSystem.h>
#include <TString.h>
#include <TCanvas.h>
#include <TGLayout.h>
#include <TGNumberEntry.h>

#include "MStreams.h"

using namespace std;

#ifdef ___CLING___
ClassImp(MGUIExpoPlotTacDiff)
#endif

////////////////////////////////////////////////////////////////////////////////

MGUIExpoPlotTacDiff::MGUIExpoPlotTacDiff(MModule* Module) : MGUIExpo(Module)
{
  m_TabTitle = "Zombie";
  m_SelectedDetector = -1;
  m_SelectedSide = 0;
  m_SelectedStrip = 0;
  m_SelectedStripPairCode = 0;
  m_DetectorSelector = nullptr;
  m_SideSelector = nullptr;
  m_StripEntry = nullptr;
  m_DtacVsDepthCanvas = nullptr;
  m_DtacVsFracCanvas = nullptr;
  m_DtacVsDtacCanvas = nullptr;
  m_EnergyMinEntry = nullptr;
  m_EnergyMaxEntry = nullptr;
  m_EnergyMin = 0;
  m_EnergyMax = 1e9;  // default: accept all
  SetCleanup(kDeepCleanup);
}

////////////////////////////////////////////////////////////////////////////////

MGUIExpoPlotTacDiff::~MGUIExpoPlotTacDiff()
{
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoPlotTacDiff::Reset()
{
  m_Mutex.Lock();
  for (auto& pair : m_DtacVsDepthHistograms) pair.second->Reset();
  for (auto& pair : m_DtacVsFracHistograms) pair.second->Reset();
  for (auto& pair : m_DtacVsDtacHistograms) pair.second->Reset();
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoPlotTacDiff::SetHistogramParameters(unsigned int DetID, unsigned int NBinsDepth, double DepthMin, double DepthMax,
                                                  unsigned int NBinsDtac, double DtacMin, double DtacMax,
                                                  unsigned int NBinsFrac, double FracMin, double FracMax)
{
  m_Mutex.Lock();

  m_NBinsDepth[DetID] = NBinsDepth;
  m_DepthMin[DetID] = DepthMin;
  m_DepthMax[DetID] = DepthMax;
  m_NBinsDtac[DetID] = NBinsDtac;
  m_DtacMin[DetID] = DtacMin;
  m_DtacMax[DetID] = DtacMax;
  m_NBinsFrac[DetID] = NBinsFrac;
  m_FracMin[DetID] = FracMin;
  m_FracMax[DetID] = FracMax;

  bool found = false;
  for (auto id : m_DetIDs) { if (id == DetID) { found = true; break; } }
  if (!found) m_DetIDs.push_back(DetID);

    // LV strips: code = 10000*DetID + 100*StripID + 99
  for (int s = 0; s < 63; s++) {
    int key = 10000 * DetID + 100 * s + 99;
    if (m_DtacVsDepthHistograms.count(key) == 0) {
      m_DtacVsDepthHistograms[key] = new TH2D("", TString::Format("Det %d LV %d: dTAC vs Depth", DetID, s),
                                               NBinsDepth, DepthMin, DepthMax, NBinsDtac, DtacMin, DtacMax);
      m_DtacVsDepthHistograms[key]->SetXTitle("Depth [cm]");
      m_DtacVsDepthHistograms[key]->SetYTitle(TString::Format("TAC %s - TAC %s [ns]",s,s+1));

      m_DtacVsFracHistograms[key] = new TH2D("", TString::Format("Det %d Charge Shared between LV %d and %d", DetID, s,s+1),
                                              NBinsFrac, FracMin, FracMax, NBinsDtac, DtacMin, DtacMax);
      m_DtacVsFracHistograms[key]->SetXTitle(TString::Format("Charge Sharing Fraction: (strip %d) / (strip %d + strip %d",s,s,s+1));
      m_DtacVsFracHistograms[key]->SetYTitle(TString::Format("TAC %s - TAC %s [ns]",s,s+1));

      m_DtacVsDtacHistograms[key] = new TH2D("", TString::Format("Det %d LV %d: dTAC vs dTAC", DetID, s),
                                              NBinsDtac, DtacMin, DtacMax, NBinsDtac, DtacMin, DtacMax);
      m_DtacVsDtacHistograms[key]->SetXTitle("dTAC [ns]");
      m_DtacVsDtacHistograms[key]->SetYTitle("dTAC alt [ns]");
    }
  }

  // HV strips: code = 10000*DetID + StripID
  for (int s = 0; s < 63; s++) {
    int key = 10000 * DetID + s + 9900;
    if (m_DtacVsDepthHistograms.count(key) == 0) {
      m_DtacVsDepthHistograms[key] = new TH2D("", TString::Format("Det %d HV %d: dTAC vs Depth", DetID, s),
                                               NBinsDepth, DepthMin, DepthMax, NBinsDtac, DtacMin, DtacMax);
      m_DtacVsDepthHistograms[key]->SetXTitle("Depth [cm]");
      m_DtacVsDepthHistograms[key]->SetYTitle(TString::Format("TAC %s - TAC %s [ns]",s,s+1));

      m_DtacVsFracHistograms[key] = new TH2D("", TString::Format("Det %d Charge Shared between LV %s and %s", DetID, s,s+1),
                                              NBinsFrac, FracMin, FracMax, NBinsDtac, DtacMin, DtacMax);
      m_DtacVsFracHistograms[key]->SetXTitle(TString::Format("Charge Sharing Fraction: (strip %d) / (strip %d + strip %d",s,s,s+1));
      m_DtacVsFracHistograms[key]->SetYTitle(TString::Format("TAC %s - TAC %s [ns]",s,s+1));

      m_DtacVsDtacHistograms[key] = new TH2D("", TString::Format("Det %d HV %d: dTAC vs dTAC", DetID, s),
                                              NBinsDtac, DtacMin, DtacMax, NBinsDtac, DtacMin, DtacMax);
      m_DtacVsDtacHistograms[key]->SetXTitle("dTAC [ns]");
      m_DtacVsDtacHistograms[key]->SetYTitle("dTAC alt [ns]");
    }
  }

  m_Mutex.UnLock();
  cout << "MGUIExpoPlotTacDiff: created histograms, total count = " << m_DtacVsDepthHistograms.size() << endl;
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoPlotTacDiff::AddData(int StripPairCode, double Depth, double dTac, double Fraction, double dTacAlt,double Energy)
{
  m_Mutex.Lock();
  if (m_DtacVsDepthHistograms.count(StripPairCode) > 0) {
    if (!std::isnan(Depth) && !std::isnan(dTac))
      m_DtacVsDepthHistograms[StripPairCode]->Fill(Depth, dTac);
    if (!std::isnan(Fraction) && !std::isnan(dTac))
      m_DtacVsFracHistograms[StripPairCode]->Fill(Fraction, dTac);
    if (!std::isnan(dTac) && !std::isnan(dTacAlt))
      m_DtacVsDtacHistograms[StripPairCode]->Fill(dTac, dTacAlt);
  }else {
    cout << "MGUIExpoPlotTacDiff::AddData: StripPairCode " << StripPairCode << " not found in histograms!" << endl;
  }
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoPlotTacDiff::Create()
{
  if (m_IsCreated == true) return;
  if (m_DtacVsDepthHistograms.empty()) return;

  m_Mutex.Lock();

  m_SelectedDetector = m_DetIDs[0];
  m_SelectedSide = 0;
  m_SelectedStrip = 0;
  m_SelectedStripPairCode = 10000 * m_SelectedDetector + 100 * m_SelectedStrip + 99;

  TGLayoutHints* ExpandLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);
  TGLayoutHints* WidgetLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5);

  // Main vertical frame (two rows)
  TGVerticalFrame* MainVFrame = new TGVerticalFrame(this);
  AddFrame(MainVFrame, ExpandLayout);

  // === Top row: two canvases side by side ===
  TGHorizontalFrame* TopRow = new TGHorizontalFrame(MainVFrame);
  MainVFrame->AddFrame(TopRow, ExpandLayout);

  m_DtacVsDepthCanvas = new TRootEmbeddedCanvas("DtacVsDepth", TopRow, 400, 300);
  TopRow->AddFrame(m_DtacVsDepthCanvas, ExpandLayout);

  m_DtacVsFracCanvas = new TRootEmbeddedCanvas("DtacVsFrac", TopRow, 400, 300);
  TopRow->AddFrame(m_DtacVsFracCanvas, ExpandLayout);

  // === Bottom row: controls left, square plot right ===
  TGHorizontalFrame* BottomRow = new TGHorizontalFrame(MainVFrame);
  MainVFrame->AddFrame(BottomRow, ExpandLayout);

// === Controls ===
  TGVerticalFrame* ControlFrame = new TGVerticalFrame(BottomRow, 180, 300);
  BottomRow->AddFrame(ControlFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 5, 5, 5, 5));

  TGLayoutHints* RowLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 5, 0, 0);
  TGLayoutHints* EntryLayout = new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 2, 0, 0);

  // Detector row
  TGHorizontalFrame* DetRow = new TGHorizontalFrame(ControlFrame);
  ControlFrame->AddFrame(DetRow, RowLayout);
  TGLabel* DetLabel = new TGLabel(DetRow, "Detector");
  DetRow->AddFrame(DetLabel, LabelLayout);
  m_DetectorSelector = new TGComboBox(DetRow);
  for (auto id : m_DetIDs) {
    m_DetectorSelector->AddEntry(TString::Format("%d", id), id);
  }
  m_DetectorSelector->Select(m_SelectedDetector);
  m_DetectorSelector->Resize(50, 20);
  //m_DetectorSelector->Connect("Selected(Int_t)", "MGUIExpoPlotTacDiff", this, "OnDetectorSelected(Int_t)");
  DetRow->AddFrame(m_DetectorSelector, EntryLayout);

  // Side + Strip row
  TGHorizontalFrame* StripRow = new TGHorizontalFrame(ControlFrame);
  ControlFrame->AddFrame(StripRow, RowLayout);
  m_SideSelector = new TGComboBox(StripRow);
  m_SideSelector->AddEntry("LV", 0);
  m_SideSelector->AddEntry("HV", 1);
  m_SideSelector->Select(0);
  m_SideSelector->Resize(45, 20);
  //m_SideSelector->Connect("Selected(Int_t)", "MGUIExpoPlotTacDiff", this, "OnSideSelected(Int_t)");
  StripRow->AddFrame(m_SideSelector, EntryLayout);
  m_StripEntry = new TGNumberEntry(StripRow, 0, 3, -1,
                                    TGNumberFormat::kNESInteger,
                                    TGNumberFormat::kNEANonNegative,
                                    TGNumberFormat::kNELLimitMinMax, 0, 62);
  //m_StripEntry->Connect("ValueSet(Long_t)", "MGUIExpoPlotTacDiff", this, "OnStripSelected()");
  m_StripEntry->GetNumberEntry()->Connect("ReturnPressed()", "MGUIExpoPlotTacDiff", this, "OnStripSelected()");
  m_StripEntry->Resize(50, 20);
  StripRow->AddFrame(m_StripEntry, EntryLayout);

  // Energy min row
  TGHorizontalFrame* EMinRow = new TGHorizontalFrame(ControlFrame);
  ControlFrame->AddFrame(EMinRow, RowLayout);
  TGLabel* EMinLabel = new TGLabel(EMinRow, "Min E");
  EMinRow->AddFrame(EMinLabel, LabelLayout);
  m_EnergyMinEntry = new TGNumberEntry(EMinRow, 0, 5, -1,
                                        TGNumberFormat::kNESRealTwo,
                                        TGNumberFormat::kNEANonNegative);
  //m_EnergyMinEntry->Connect("ValueSet(Long_t)", "MGUIExpoPlotTacDiff", this, "OnEnergyRangeChanged()");
  m_EnergyMinEntry->GetNumberEntry()->Connect("ReturnPressed()", "MGUIExpoPlotTacDiff", this, "OnEnergyRangeChanged()");
  m_EnergyMinEntry->Resize(70, 20);
  EMinRow->AddFrame(m_EnergyMinEntry, EntryLayout);

  // Energy max row
  TGHorizontalFrame* EMaxRow = new TGHorizontalFrame(ControlFrame);
  ControlFrame->AddFrame(EMaxRow, RowLayout);
  TGLabel* EMaxLabel = new TGLabel(EMaxRow, "Max E");
  EMaxRow->AddFrame(EMaxLabel, LabelLayout);
  m_EnergyMaxEntry = new TGNumberEntry(EMaxRow, 10000, 5, -1,
                                        TGNumberFormat::kNESRealTwo,
                                        TGNumberFormat::kNEANonNegative);
  //m_EnergyMaxEntry->Connect("ValueSet(Long_t)", "MGUIExpoPlotTacDiff", this, "OnEnergyRangeChanged()");
  m_EnergyMaxEntry->GetNumberEntry()->Connect("ReturnPressed()", "MGUIExpoPlotTacDiff", this, "OnEnergyRangeChanged()");
  m_EnergyMaxEntry->Resize(70, 20);
  EMaxRow->AddFrame(m_EnergyMaxEntry, EntryLayout);

  // === UPDATE BUTTON ===
  m_UpdateSelectionButton = new TGTextButton(ControlFrame, "Update Plot Selection", c_UpdateSelection);
  m_UpdateSelectionButton->Associate(this);
  ControlFrame->AddFrame(m_UpdateSelectionButton, RowLayout);
 
  // Square canvas
  m_DtacVsDtacCanvas = new TRootEmbeddedCanvas("DtacVsDtac", BottomRow, 400, 400);
  BottomRow->AddFrame(m_DtacVsDtacCanvas, ExpandLayout);

  RedrawPlots();

  m_IsCreated = true;
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

bool MGUIExpoPlotTacDiff::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      switch (Parameter1) {
      case c_UpdateSelection:
        OnUpdateSelection();
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

void MGUIExpoPlotTacDiff::OnUpdateSelection()
{
  m_Mutex.Lock();

  // Read current widget values
  m_SelectedDetector = m_DetectorSelector->GetSelected();
  m_SelectedSide = m_SideSelector->GetSelected();
  m_SelectedStrip = (int)m_StripEntry->GetNumber();
  m_EnergyMin = m_EnergyMinEntry->GetNumber();
  m_EnergyMax = m_EnergyMaxEntry->GetNumber();

  RedrawPlots();

  m_Mutex.UnLock();
}
////////////////////////////////////////////////////////////////////////////////

void MGUIExpoPlotTacDiff::RedrawPlots()
{
  if (m_SelectedSide == 0) {
    m_SelectedStripPairCode = 10000 * m_SelectedDetector + 100 * m_SelectedStrip + 99;
  } else {
    m_SelectedStripPairCode = 10000 * m_SelectedDetector + m_SelectedStrip + 9900;
  }

  int key = m_SelectedStripPairCode;
  if (m_DtacVsDepthHistograms.count(key) > 0) {
    m_DtacVsDepthCanvas->GetCanvas()->cd();
    m_DtacVsDepthHistograms[key]->Draw("colz");
    m_DtacVsDepthCanvas->GetCanvas()->Modified();
    m_DtacVsDepthCanvas->GetCanvas()->Update();

    m_DtacVsFracCanvas->GetCanvas()->cd();
    m_DtacVsFracHistograms[key]->Draw("colz");
    m_DtacVsFracCanvas->GetCanvas()->Modified();
    m_DtacVsFracCanvas->GetCanvas()->Update();

    m_DtacVsDtacCanvas->GetCanvas()->cd();
    m_DtacVsDtacHistograms[key]->Draw("colz");
    m_DtacVsDtacCanvas->GetCanvas()->Modified();
    m_DtacVsDtacCanvas->GetCanvas()->Update();
  }
}

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoPlotTacDiff::Update()
{
  m_Mutex.Lock();
  if (m_IsCreated) {
    RedrawPlots();
  }
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoPlotTacDiff::Export(const MString& FileName)
{
  m_Mutex.Lock();

  int key = m_SelectedStripPairCode;
  TCanvas* P = new TCanvas("", "", 1200, 800);
  P->Divide(2, 2);

  if (m_DtacVsDepthHistograms.count(key) > 0) {
    P->cd(1);
    m_DtacVsDepthHistograms[key]->DrawCopy("colz");
    P->cd(2);
    m_DtacVsFracHistograms[key]->DrawCopy("colz");
    P->cd(3);
    m_DtacVsDtacHistograms[key]->DrawCopy("colz");
  }

  P->SaveAs(FileName);
  delete P;

  m_Mutex.UnLock();
}
