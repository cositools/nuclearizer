/*
 * MGUIExpoTrappingCorrection.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer
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
#include "MGUIExpoTrappingCorrection.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <TString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TCanvas.h>
#include <TGFrame.h>
#include <TLegend.h>
#include <THStack.h>
#include <TVirtualPad.h>
#include <algorithm>

// MEGAlib libs:
#include "MStreams.h"



////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIExpoTrappingCorrection)
#endif

////////////////////////////////////////////////////////////////////////////////

MGUIExpoTrappingCorrection::MGUIExpoTrappingCorrection(MModule* Module) : MGUIExpo(Module)
{
  m_TabTitle = "Trapping Correction";

  double eMin = 600;
  double eMax = 700;
  double nBins = 1000;


  // LV Histograms
  m_EnergyLVInitial = new TH1D("EnergyLVInitial", "LV Spectrum (Uncorrected vs Corrected)", nBins, eMin, eMax);
  m_EnergyLVInitial->SetXTitle("Energy [keV]");
  m_EnergyLVInitial->SetYTitle("Counts");
  m_EnergyLVInitial->GetYaxis()->SetNoExponent(kTRUE);
  m_EnergyLVInitial->SetLineColor(kGray+2);
  m_EnergyLVInitial->SetLineWidth(2);
  m_EnergyLVInitial->SetLineStyle(2);

  m_EnergyLVFinal = new TH1D("EnergyLVFinal", "LV Spectrum (Uncorrected vs Corrected)", nBins, eMin, eMax);
  m_EnergyLVFinal->SetXTitle("Energy [keV]");
  m_EnergyLVFinal->SetYTitle("Counts");
  m_EnergyLVFinal->GetYaxis()->SetNoExponent(kTRUE);
  m_EnergyLVFinal->SetLineColor(kAzure+1);
  m_EnergyLVFinal->SetLineWidth(2);
  m_EnergyLVFinal->SetFillColorAlpha(kAzure-9, 0.35);

  // HV Histograms
  m_EnergyHVInitial = new TH1D("EnergyHVInitial", "HV Spectrum (Uncorrected vs Corrected)", nBins, eMin, eMax);
  m_EnergyHVInitial->SetXTitle("Energy [keV]");
  m_EnergyHVInitial->SetYTitle("Counts");
  m_EnergyHVInitial->GetYaxis()->SetNoExponent(kTRUE);
  m_EnergyHVInitial->SetLineColor(kGray+2);
  m_EnergyHVInitial->SetLineWidth(2);
  m_EnergyHVInitial->SetLineStyle(2);

  m_EnergyHVFinal = new TH1D("EnergyHVFinal", "HV Spectrum (Uncorrected vs Corrected)", nBins, eMin, eMax);
  m_EnergyHVFinal->SetXTitle("Energy [keV]");
  m_EnergyHVFinal->SetYTitle("Counts");
  m_EnergyHVFinal->GetYaxis()->SetNoExponent(kTRUE);
  m_EnergyHVFinal->SetLineColor(kOrange+7);
  m_EnergyHVFinal->SetLineWidth(2);
  m_EnergyHVFinal->SetFillColorAlpha(kOrange-9, 0.35);

  // Initialize canvases and buttons 
  m_CanvasLV = nullptr;
  m_CanvasHV = nullptr;
  m_LegendLV = nullptr;
  m_LegendHV = nullptr;

  m_EntryNBins = nullptr;
  m_EntryMinEnergy = nullptr;
  m_EntryMaxEnergy = nullptr;
  m_CheckLogY = nullptr;
  m_ButtonApply = nullptr;

  SetCleanup(kDeepCleanup);
}

////////////////////////////////////////////////////////////////////////////////

MGUIExpoTrappingCorrection::~MGUIExpoTrappingCorrection()
{
  // Memory cleanup handled by ROOT's kDeepCleanup
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::Create()
{
  if (m_IsCreated == true) return;

  m_Mutex.Lock();

  // Create labels and buttons on GUI
  // Top frame
  TGHorizontalFrame* ControlFrame = new TGHorizontalFrame(this, 800, 30);
  
  // Binning entry
  TGLabel* LabelBins = new TGLabel(ControlFrame, "Bins:");
  ControlFrame->AddFrame(LabelBins, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));

  m_EntryNBins = new TGNumberEntry(ControlFrame, 200, 5, -1, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive);
  ControlFrame->AddFrame(m_EntryNBins, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

  // Min Energy entry
  TGLabel* LabelMin = new TGLabel(ControlFrame, "Min Energy [keV]:");
  ControlFrame->AddFrame(LabelMin, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));

  m_EntryMinEnergy = new TGNumberEntry(ControlFrame, 0, 6, -1, TGNumberFormat::kNESRealOne);
  ControlFrame->AddFrame(m_EntryMinEnergy, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

  // Max Energy entry
  TGLabel* LabelMax = new TGLabel(ControlFrame, "Max Energy [keV]:");
  ControlFrame->AddFrame(LabelMax, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));

  m_EntryMaxEnergy = new TGNumberEntry(ControlFrame, 1000, 6, -1, TGNumberFormat::kNESRealOne);
  ControlFrame->AddFrame(m_EntryMaxEnergy, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

  // Log Y Checkbox
  m_CheckLogY = new TGCheckButton(ControlFrame, "Log Y Scale");
  ControlFrame->AddFrame(m_CheckLogY, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 10, 2, 2));
  m_CheckLogY->Connect("Clicked()", "MGUIExpoTrappingCorrection", this, "OnApply()");

  // Apply Button
  m_ButtonApply = new TGTextButton(ControlFrame, " Apply Range ");
  ControlFrame->AddFrame(m_ButtonApply, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 5, 2, 2));
  m_ButtonApply->Connect("Clicked()", "MGUIExpoTrappingCorrection", this, "OnApply()");

  AddFrame(ControlFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 2));

  // Main Canvas Frame (Side-by-Side Plots)
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);
  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, CanvasLayout);

  // LV Canvas setup
  m_CanvasLV = new TRootEmbeddedCanvas("CanvasLV", HFrame, 100, 100);
  HFrame->AddFrame(m_CanvasLV, CanvasLayout);

  m_CanvasLV->GetCanvas()->cd();
  m_CanvasLV->GetCanvas()->SetGridx();
  m_CanvasLV->GetCanvas()->SetGridy();

  m_EnergyLVInitial->Draw("HIST");
  m_EnergyLVFinal->Draw("HIST SAME");

  m_LegendLV = new TLegend(0.55, 0.72, 0.88, 0.88);
  m_LegendLV->AddEntry(m_EnergyLVInitial, "LV Uncorrected", "l");
  m_LegendLV->AddEntry(m_EnergyLVFinal,   "LV Corrected",   "f");
  m_LegendLV->Draw();

  // HV Canvas setup
  m_CanvasHV = new TRootEmbeddedCanvas("CanvasHV", HFrame, 100, 100);
  HFrame->AddFrame(m_CanvasHV, CanvasLayout);

  m_CanvasHV->GetCanvas()->cd();
  m_CanvasHV->GetCanvas()->SetGridx();
  m_CanvasHV->GetCanvas()->SetGridy();

  m_EnergyHVInitial->Draw("HIST");
  m_EnergyHVFinal->Draw("HIST SAME");

  m_LegendHV = new TLegend(0.55, 0.72, 0.88, 0.88);
  m_LegendHV->AddEntry(m_EnergyHVInitial, "HV Uncorrected", "l");
  m_LegendHV->AddEntry(m_EnergyHVFinal,   "HV Corrected",   "f");
  m_LegendHV->Draw();

  MapSubwindows();
  Layout();

  // Signal that the canvas has been created so we know when we can start to fill it
  m_IsCreated = true;

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::OnApply()
{
  //Create function has already been run
  if (m_IsCreated == false) return;
  if (m_EntryNBins == nullptr || m_EntryMinEnergy == nullptr || m_EntryMaxEnergy == nullptr) return;

  int nBins   = m_EntryNBins->GetIntNumber();
  double minE = m_EntryMinEnergy->GetNumber();
  double maxE = m_EntryMaxEnergy->GetNumber();

  if (maxE <= minE || nBins <= 0) return;

  SetEnergyHistogramParameters(nBins, minE, maxE);
  Update();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::SetEnergyHistogramParameters(int NBins, double Min, double Max)
{
  m_Mutex.Lock();

  // 1. Zoom the X-axis view without altering underlying data/binning
  if (m_EnergyLVInitial != nullptr) m_EnergyLVInitial->GetXaxis()->SetRangeUser(Min, Max);
  if (m_EnergyLVFinal != nullptr)   m_EnergyLVFinal->GetXaxis()->SetRangeUser(Min, Max);
  if (m_EnergyHVInitial != nullptr) m_EnergyHVInitial->GetXaxis()->SetRangeUser(Min, Max);
  if (m_EnergyHVFinal != nullptr)   m_EnergyHVFinal->GetXaxis()->SetRangeUser(Min, Max);

  // 2. Update entry boxes
  if (m_EntryNBins != nullptr)     m_EntryNBins->SetIntNumber(NBins);
  if (m_EntryMinEnergy != nullptr) m_EntryMinEnergy->SetNumber(Min);
  if (m_EntryMaxEnergy != nullptr) m_EntryMaxEnergy->SetNumber(Max);

  m_Mutex.UnLock();
}
////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::Update()
{
  if (m_IsCreated == false) return;

  m_Mutex.Lock();

  bool isLog = (m_CheckLogY != nullptr && m_CheckLogY->IsOn());

  // --- Update LV Canvas ---
  if (m_CanvasLV != nullptr && m_CanvasLV->GetCanvas() != nullptr) {
    TCanvas* canvasLV = m_CanvasLV->GetCanvas();
    canvasLV->cd();
    canvasLV->SetLogy(isLog ? 1 : 0);

    double maxLVInitial = m_EnergyLVInitial->GetBinContent(m_EnergyLVInitial->GetMaximumBin());
    double maxLVFinal   = m_EnergyLVFinal->GetBinContent(m_EnergyLVFinal->GetMaximumBin());
    double realMaxLV    = std::max(maxLVInitial, maxLVFinal);

    if (isLog) {
      m_EnergyLVInitial->SetMinimum(0.1);
      m_EnergyLVInitial->SetMaximum(realMaxLV > 0 ? realMaxLV * 5.0 : 10.0);
    } else {
      m_EnergyLVInitial->SetMinimum(-1111);
      m_EnergyLVInitial->SetMaximum(realMaxLV > 0 ? realMaxLV * 1.15 : 10.0);
    }

    canvasLV->Modified();
    canvasLV->Update();
  }

  // --- Update HV Canvas ---
  if (m_CanvasHV != nullptr && m_CanvasHV->GetCanvas() != nullptr) {
    TCanvas* canvasHV = m_CanvasHV->GetCanvas();
    canvasHV->cd();
    canvasHV->SetLogy(isLog ? 1 : 0);

    double maxHVInitial = m_EnergyHVInitial->GetBinContent(m_EnergyHVInitial->GetMaximumBin());
    double maxHVFinal   = m_EnergyHVFinal->GetBinContent(m_EnergyHVFinal->GetMaximumBin());
    double realMaxHV    = std::max(maxHVInitial, maxHVFinal);

    if (isLog) {
      m_EnergyHVInitial->SetMinimum(0.1);
      m_EnergyHVInitial->SetMaximum(realMaxHV > 0 ? realMaxHV * 5.0 : 10.0);
    } else {
      m_EnergyHVInitial->SetMinimum(-1111);
      m_EnergyHVInitial->SetMaximum(realMaxHV > 0 ? realMaxHV * 1.15 : 10.0);
    }

    canvasHV->Modified();
    canvasHV->Update();
  }

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::Reset()
{
  m_Mutex.Lock();

  if (m_EnergyLVInitial != nullptr) m_EnergyLVInitial->Reset();
  if (m_EnergyLVFinal != nullptr)   m_EnergyLVFinal->Reset();
  if (m_EnergyHVInitial != nullptr) m_EnergyHVInitial->Reset();
  if (m_EnergyHVFinal != nullptr)   m_EnergyHVFinal->Reset();

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::AddEnergyInitial(double Energy, bool IsNearestNeighbor, bool IsLV)
{
  m_Mutex.Lock();

  if (IsLV == true) {
    if (m_EnergyLVInitial != nullptr) m_EnergyLVInitial->Fill(Energy);
  } else {
    if (m_EnergyHVInitial != nullptr) m_EnergyHVInitial->Fill(Energy);
  }

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::AddEnergyFinal(double Energy, bool IsNearestNeighbor, bool IsLV)
{
  m_Mutex.Lock();

  if (IsLV == true) {
    if (m_EnergyLVFinal != nullptr) m_EnergyLVFinal->Fill(Energy);
  } else {
    if (m_EnergyHVFinal != nullptr) m_EnergyHVFinal->Fill(Energy);
  }

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::Export(const MString& FileName)
{
  m_Mutex.Lock();

  if (m_CanvasLV != nullptr && m_CanvasLV->GetCanvas() != nullptr) {
    MString nameLV = FileName;
    nameLV.ReplaceAll(".png", "_LV.png");
    m_CanvasLV->GetCanvas()->SaveAs(nameLV);
  }

  if (m_CanvasHV != nullptr && m_CanvasHV->GetCanvas() != nullptr) {
    MString nameHV = FileName;
    nameHV.ReplaceAll(".png", "_HV.png");
    m_CanvasHV->GetCanvas()->SaveAs(nameHV);
  }

  m_Mutex.UnLock();
}