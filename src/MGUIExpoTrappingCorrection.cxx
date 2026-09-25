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

// Nuclearizer libs:
#include "MModuleTrappingCorrection.h"



////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIExpoTrappingCorrection)
#endif

////////////////////////////////////////////////////////////////////////////////

MGUIExpoTrappingCorrection::MGUIExpoTrappingCorrection(MModule* Module) : MGUIExpo(Module)
{
  m_TabTitle = "Trapping Correction";

  double eMin = 0.0;
  double eMax = 2000.0;
  int nBins = 20000;


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

  double defaultMu = 661.7; // Default for Cs-137 
  double nBins = 1000;

  // Top control frame
  TGHorizontalFrame* ControlFrame = new TGHorizontalFrame(this, 800, 30);
  
  // Binning entry
  TGLabel* LabelBins = new TGLabel(ControlFrame, "Bins:");
  ControlFrame->AddFrame(LabelBins, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));

  m_EntryNBins = new TGNumberEntry(ControlFrame, nBins, 5, -1, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive);
  ControlFrame->AddFrame(m_EntryNBins, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

  // Photopeak Mu entry
  TGLabel* LabelMu = new TGLabel(ControlFrame, "Photopeak [keV]:");
  ControlFrame->AddFrame(LabelMu, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));

  m_EntryPhotopeakMu = new TGNumberEntry(ControlFrame, defaultMu, 6, -1, TGNumberFormat::kNESRealOne);
  ControlFrame->AddFrame(m_EntryPhotopeakMu, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

  // Log Y Checkbox
  m_CheckLogY = new TGCheckButton(ControlFrame, "Log Y Scale");
  ControlFrame->AddFrame(m_CheckLogY, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 10, 2, 2));
  m_CheckLogY->Connect("Clicked()", "MGUIExpoTrappingCorrection", this, "OnApply()");

  // Apply Button
  m_ButtonApply = new TGTextButton(ControlFrame, " Apply ");
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
  if (m_IsCreated == false) return;
  if (m_EntryNBins == nullptr || m_EntryPhotopeakMu == nullptr) return;

  int nBins = m_EntryNBins->GetIntNumber();
  double mu = m_EntryPhotopeakMu->GetNumber();

  if (mu <= 20.0 || nBins <= 0) return;

  // Calculate dynamic histogram bounds based on mu 
  double minE = mu - 50.0;
  double maxE = mu + 50.0;

  // Pass mu directly to the underlying module
  if (m_Module != nullptr) {
    // Dynamic cast if m_Module is stored as a generic MModule*
    MModuleTrappingCorrection* module = dynamic_cast<MModuleTrappingCorrection*>(m_Module);
    if (module != nullptr) {
      module->SetPhotopeakMu(mu);
    }
  }

  SetEnergyHistogramParameters(nBins, minE, maxE);
  Update();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::SetEnergyHistogramParameters(int NBins, double Min, double Max)
{
  m_Mutex.Lock();

  // Zoom the X-axis view without altering underlying histogram data or bin contents
  if (m_EnergyLVInitial != nullptr) m_EnergyLVInitial->GetXaxis()->SetRangeUser(Min, Max);
  if (m_EnergyLVFinal != nullptr)   m_EnergyLVFinal->GetXaxis()->SetRangeUser(Min, Max);
  if (m_EnergyHVInitial != nullptr) m_EnergyHVInitial->GetXaxis()->SetRangeUser(Min, Max);
  if (m_EnergyHVFinal != nullptr)   m_EnergyHVFinal->GetXaxis()->SetRangeUser(Min, Max);

  // Update entry box
  if (m_EntryNBins != nullptr) m_EntryNBins->SetIntNumber(NBins);

  m_Mutex.UnLock();
}
////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::Update()
{
  if (m_IsCreated == false) return;

  m_Mutex.Lock();

  bool isLog = (m_CheckLogY != nullptr && m_CheckLogY->IsOn());

  // Helper lambda to calculate max content strictly within visible X-axis range
  auto GetVisibleMax = [](TH1D* hist) -> double {
    if (hist == nullptr) return 0.0;
    
    TAxis* axis = hist->GetXaxis();
    int minBin = axis->GetFirst(); // First visible bin
    int maxBin = axis->GetLast();  // Last visible bin
    
    double maxVal = 0.0;
    for (int b = minBin; b <= maxBin; ++b) {
      double content = hist->GetBinContent(b);
      if (content > maxVal) maxVal = content;
    }
    return maxVal;
  };

  // --- Update LV Canvas ---
  if (m_CanvasLV != nullptr && m_CanvasLV->GetCanvas() != nullptr) {
    TCanvas* canvasLV = m_CanvasLV->GetCanvas();
    canvasLV->cd();
    canvasLV->SetLogy(isLog ? 1 : 0);

    double realMaxLV = std::max(GetVisibleMax(m_EnergyLVInitial), GetVisibleMax(m_EnergyLVFinal));

    if (isLog) {
      double minLogLV = (realMaxLV > 0.1) ? 0.1 : 0.01;
      double maxLogLV = (realMaxLV > 0.1) ? realMaxLV * 5.0 : 10.0;

      m_EnergyLVInitial->SetMinimum(minLogLV);
      m_EnergyLVInitial->SetMaximum(maxLogLV);
      m_EnergyLVFinal->SetMinimum(minLogLV);
      m_EnergyLVFinal->SetMaximum(maxLogLV);
    } else {
      m_EnergyLVInitial->SetMinimum(-1111);
      m_EnergyLVInitial->SetMaximum(-1111);
      m_EnergyLVFinal->SetMinimum(-1111);
      m_EnergyLVFinal->SetMaximum(-1111);

      double maxLinLV = (realMaxLV > 0) ? realMaxLV * 1.15 : 10.0;
      m_EnergyLVInitial->SetMinimum(0.0);
      m_EnergyLVInitial->SetMaximum(maxLinLV);
      m_EnergyLVFinal->SetMinimum(0.0);
      m_EnergyLVFinal->SetMaximum(maxLinLV);
    }

    canvasLV->Modified();
    canvasLV->Update();
  }

  // --- Update HV Canvas ---
  if (m_CanvasHV != nullptr && m_CanvasHV->GetCanvas() != nullptr) {
    TCanvas* canvasHV = m_CanvasHV->GetCanvas();
    canvasHV->cd();
    canvasHV->SetLogy(isLog ? 1 : 0);

    double realMaxHV = std::max(GetVisibleMax(m_EnergyHVInitial), GetVisibleMax(m_EnergyHVFinal));

    if (isLog) {
      double minLogHV = (realMaxHV > 0.1) ? 0.1 : 0.01;
      double maxLogHV = (realMaxHV > 0.1) ? realMaxHV * 5.0 : 10.0;

      m_EnergyHVInitial->SetMinimum(minLogHV);
      m_EnergyHVInitial->SetMaximum(maxLogHV);
      m_EnergyHVFinal->SetMinimum(minLogHV);
      m_EnergyHVFinal->SetMaximum(maxLogHV);
    } else {
      m_EnergyHVInitial->SetMinimum(-1111);
      m_EnergyHVInitial->SetMaximum(-1111);
      m_EnergyHVFinal->SetMinimum(-1111);
      m_EnergyHVFinal->SetMaximum(-1111);

      double maxLinHV = (realMaxHV > 0) ? realMaxHV * 1.15 : 10.0;
      m_EnergyHVInitial->SetMinimum(0.0);
      m_EnergyHVInitial->SetMaximum(maxLinHV);
      m_EnergyHVFinal->SetMinimum(0.0);
      m_EnergyHVFinal->SetMaximum(maxLinHV);
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