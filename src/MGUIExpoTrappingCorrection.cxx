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

  // Default parameters: 200 bins, 0 to 1000 keV
  m_EnergyLVInitial = new TH1D("EnergyLVInitial", "LV Spectrum (Uncorrected vs Corrected)", 100, 620, 700);
  m_EnergyLVInitial->SetXTitle("Energy [keV]");
  m_EnergyLVInitial->SetYTitle("Counts");
  m_EnergyLVInitial->GetYaxis()->SetNoExponent(kTRUE);
  m_EnergyLVInitial->SetLineColor(kGray+2);
  m_EnergyLVInitial->SetLineWidth(2);
  m_EnergyLVInitial->SetLineStyle(2);

  m_EnergyLVFinal = new TH1D("EnergyLVFinal", "LV Spectrum (Uncorrected vs Corrected)", 100, 620, 700);
  m_EnergyLVFinal->SetXTitle("Energy [keV]");
  m_EnergyLVFinal->SetYTitle("Counts");
  m_EnergyLVFinal->GetYaxis()->SetNoExponent(kTRUE);
  m_EnergyLVFinal->SetLineColor(kAzure+1);
  m_EnergyLVFinal->SetLineWidth(2);
  m_EnergyLVFinal->SetFillColorAlpha(kAzure-9, 0.35);

  m_EnergyHVInitial = new TH1D("EnergyHVInitial", "HV Spectrum (Uncorrected vs Corrected)", 100, 620, 700);
  m_EnergyHVInitial->SetXTitle("Energy [keV]");
  m_EnergyHVInitial->SetYTitle("Counts");
  m_EnergyHVInitial->GetYaxis()->SetNoExponent(kTRUE);
  m_EnergyHVInitial->SetLineColor(kGray+2);
  m_EnergyHVInitial->SetLineWidth(2);
  m_EnergyHVInitial->SetLineStyle(2);

  m_EnergyHVFinal = new TH1D("EnergyHVFinal", "HV Spectrum (Uncorrected vs Corrected)", 100, 620, 700);
  m_EnergyHVFinal->SetXTitle("Energy [keV]");
  m_EnergyHVFinal->SetYTitle("Counts");
  m_EnergyHVFinal->GetYaxis()->SetNoExponent(kTRUE);
  m_EnergyHVFinal->SetLineColor(kOrange+7);
  m_EnergyHVFinal->SetLineWidth(2);
  m_EnergyHVFinal->SetFillColorAlpha(kOrange-9, 0.35);

  m_CanvasLV = nullptr;
  m_CanvasHV = nullptr;
  m_LegendLV = nullptr;
  m_LegendHV = nullptr;

  m_EntryNBins = nullptr;
  m_EntryMinEnergy = nullptr;
  m_EntryMaxEnergy = nullptr;
  m_ButtonApply = nullptr;

  SetCleanup(kDeepCleanup);
}

////////////////////////////////////////////////////////////////////////////////

MGUIExpoTrappingCorrection::~MGUIExpoTrappingCorrection()
{
  // kDeepCleanup handles memory deletion for embedded GUI widgets
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::Create()
{
  if (m_IsCreated == true) return;

  m_Mutex.Lock();

  // 1. Top Control Bar Frame for parameters
  TGHorizontalFrame* ControlFrame = new TGHorizontalFrame(this, 800, 30);
  
  // Binning Entry
  TGLabel* LabelBins = new TGLabel(ControlFrame, "Bins:");
  ControlFrame->AddFrame(LabelBins, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));

  m_EntryNBins = new TGNumberEntry(ControlFrame, 200, 5, -1, TGNumberFormat::kNESInteger, TGNumberFormat::kNEAPositive);
  ControlFrame->AddFrame(m_EntryNBins, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

  // Min Energy Entry
  TGLabel* LabelMin = new TGLabel(ControlFrame, "Min Energy [keV]:");
  ControlFrame->AddFrame(LabelMin, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));

  m_EntryMinEnergy = new TGNumberEntry(ControlFrame, 0, 6, -1, TGNumberFormat::kNESRealOne);
  ControlFrame->AddFrame(m_EntryMinEnergy, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

  // Max Energy Entry
  TGLabel* LabelMax = new TGLabel(ControlFrame, "Max Energy [keV]:");
  ControlFrame->AddFrame(LabelMax, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 5, 2, 2, 2));

  m_EntryMaxEnergy = new TGNumberEntry(ControlFrame, 1000, 6, -1, TGNumberFormat::kNESRealOne);
  ControlFrame->AddFrame(m_EntryMaxEnergy, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 10, 2, 2));

  // Apply Button
  m_ButtonApply = new TGTextButton(ControlFrame, " Apply Range ");
  ControlFrame->AddFrame(m_ButtonApply, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 10, 5, 2, 2));

  // Connect the Apply button click to the OnApply method slot
  m_ButtonApply->Connect("Clicked()", "MGUIExpoTrappingCorrection", this, "OnApply()");

  // Add the control bar at the top of the tab
  AddFrame(ControlFrame, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 5, 5, 5, 2));

  // 2. Main Canvas Frame (Side-by-Side Plots)
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);
  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, CanvasLayout);

  // LV Canvas
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

  // HV Canvas
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

  m_IsCreated = true;

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::OnApply()
{
  //! Called when the user clicks "Apply Range" in the GUI

  if (m_EntryNBins == nullptr || m_EntryMinEnergy == nullptr || m_EntryMaxEnergy == nullptr) return;

  int nBins   = m_EntryNBins->GetIntNumber();
  double minE = m_EntryMinEnergy->GetNumber();
  double maxE = m_EntryMaxEnergy->GetNumber();

  if (maxE <= minE || nBins <= 0) {
    cout << "WARNING in MGUIExpoTrappingCorrection: Invalid histogram parameters (" 
         << nBins << " bins, min=" << minE << ", max=" << maxE << ")" << endl;
    return;
  }

  SetEnergyHistogramParameters(nBins, minE, maxE);
  Update();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::SetEnergyHistogramParameters(int NBins, double Min, double Max)
{
  m_Mutex.Lock();

  m_EnergyLVInitial->SetBins(NBins, Min, Max);
  m_EnergyLVFinal->SetBins(NBins, Min, Max);
  m_EnergyHVInitial->SetBins(NBins, Min, Max);
  m_EnergyHVFinal->SetBins(NBins, Min, Max);

  // Keep entry controls in sync if set programmatically from code (e.g., CreateExpos)
  if (m_EntryNBins != nullptr)     m_EntryNBins->SetIntNumber(NBins);
  if (m_EntryMinEnergy != nullptr) m_EntryMinEnergy->SetNumber(Min);
  if (m_EntryMaxEnergy != nullptr) m_EntryMaxEnergy->SetNumber(Max);

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::Update()
{
  m_Mutex.Lock();

  if (m_CanvasLV != nullptr && m_CanvasLV->GetCanvas() != nullptr) {
    m_CanvasLV->GetCanvas()->cd();

    double maxLV = std::max(m_EnergyLVInitial->GetMaximum(), m_EnergyLVFinal->GetMaximum());
    if (maxLV > 0.0) {
      m_EnergyLVInitial->SetMaximum(maxLV * 1.15);
    }

    m_CanvasLV->GetCanvas()->Modified();
    m_CanvasLV->GetCanvas()->Update();
  }

  if (m_CanvasHV != nullptr && m_CanvasHV->GetCanvas() != nullptr) {
    m_CanvasHV->GetCanvas()->cd();

    double maxHV = std::max(m_EnergyHVInitial->GetMaximum(), m_EnergyHVFinal->GetMaximum());
    if (maxHV > 0.0) {
      m_EnergyHVInitial->SetMaximum(maxHV * 1.15);
    }

    m_CanvasHV->GetCanvas()->Modified();
    m_CanvasHV->GetCanvas()->Update();
  }

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::Reset()
{
  m_Mutex.Lock();

  m_EnergyLVInitial->Reset();
  m_EnergyLVFinal->Reset();
  m_EnergyHVInitial->Reset();
  m_EnergyHVFinal->Reset();

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::AddEnergyInitial(double Energy, bool IsNearestNeighbor, bool IsLV)
{
  m_Mutex.Lock();

  if (IsLV == true) {
    m_EnergyLVInitial->Fill(Energy);
  } else {
    m_EnergyHVInitial->Fill(Energy);
  }

  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////

void MGUIExpoTrappingCorrection::AddEnergyFinal(double Energy, bool IsNearestNeighbor, bool IsLV)
{
  m_Mutex.Lock();

  if (IsLV == true) {
    m_EnergyLVFinal->Fill(Energy);
  } else {
    m_EnergyHVFinal->Fill(Energy);
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