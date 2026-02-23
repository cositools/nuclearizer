/*
 * MGUIExpoPlotSpectrum.cxx
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
#include "MGUIExpoPlotSpectrum.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <TString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TCanvas.h>
#include <TGButton.h>
#include <THStack.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MGUIEEntry.h"



////////////////////////////////////////////////////////////////////////////////


// To update the histogram energy range
static const int c_UpdateRange = 1001;

#ifdef ___CLING___
ClassImp(MGUIExpoPlotSpectrum)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoPlotSpectrum::MGUIExpoPlotSpectrum(MModule* Module) : MGUIExpo(Module)
{
  //! This allows you to plot the enegry spectrum Initial and Final some operation in a module
  //! For example, Initial and Final the TAC cut is applied
  //! You can also just plot the Initial or Final case, for example with the energy calibration you just want to plot the Final case

  // standard constructor
  
  m_PlotSpectrum = true;

  // Set the new title of the tab here:
  if (Module != nullptr) {
      m_TabTitle = "Energy Spectrum (" + Module->GetName() + ")";
  } else {
      m_TabTitle = "Energy Spectrum";
  }
  
  m_LogXButton = nullptr;
  m_LogYButton = nullptr;

  // Initialize pointers to 0
  // Initial
  m_EnergyHistogramLVInitial = 0;
  m_EnergyHistogramHVInitial = 0;
  m_EnergyHistogramNearestNeighborLVInitial = 0;
  m_EnergyHistogramNearestNeighborHVInitial = 0;
  
  // Final
  m_EnergyHistogramLVFinal = 0;
  m_EnergyHistogramHVFinal = 0;
  m_EnergyHistogramNearestNeighborLVFinal = 0;
  m_EnergyHistogramNearestNeighborHVFinal = 0;
  
  m_EnergyCanvas = 0;
  
  // LV Strips Initial
  m_EnergyHistogramLVInitial = new TH1D("EnergyLVInitial", "LV Side: Strips Initial", 200, 0, 1000);
  m_EnergyHistogramLVInitial->SetXTitle("Energy [keV]");
  m_EnergyHistogramLVInitial->SetYTitle("Counts");
  m_EnergyHistogramLVInitial->SetLineColor(kAzure+7);
  m_EnergyHistogramLVInitial->SetLineWidth(1);
  m_EnergyHistogramLVInitial->SetFillColorAlpha(kGray+1, 0.5);
  m_EnergyHistogramLVInitial->SetFillStyle(1001);
  m_EnergyHistogramLVInitial->SetDirectory(0);

  // HV Strips Initial
  m_EnergyHistogramHVInitial = new TH1D("EnergyHVInitial", "HV Side: Strips Initial", 200, 0, 1000);
  m_EnergyHistogramHVInitial->SetXTitle("Energy [keV]");
  m_EnergyHistogramHVInitial->SetYTitle("Counts");
  m_EnergyHistogramHVInitial->SetLineColor(kAzure+7);
  m_EnergyHistogramHVInitial->SetLineWidth(1);
  m_EnergyHistogramHVInitial->SetFillColorAlpha(kGray+1, 0.5);
  m_EnergyHistogramHVInitial->SetFillStyle(1001);
  m_EnergyHistogramHVInitial->SetDirectory(0);
  
  // LV Nearest Neighbor Initial
  m_EnergyHistogramNearestNeighborLVInitial = new TH1D("EnergyNearestNeighborLVInitial", "LV Side: Nearest Neighbor Initial", 200, 0, 1000);
  m_EnergyHistogramNearestNeighborLVInitial->SetXTitle("Energy [keV]");
  m_EnergyHistogramNearestNeighborLVInitial->SetYTitle("Counts");
  m_EnergyHistogramNearestNeighborLVInitial->SetLineColor(kViolet);
  m_EnergyHistogramNearestNeighborLVInitial->SetLineWidth(1);
  m_EnergyHistogramNearestNeighborLVInitial->SetFillColorAlpha(kGray+4, 0.2);
  m_EnergyHistogramNearestNeighborLVInitial->SetFillStyle(1001);
  m_EnergyHistogramNearestNeighborLVInitial->SetDirectory(0);

  // HV Nearest Neighbor Initial
  m_EnergyHistogramNearestNeighborHVInitial = new TH1D("EnergyNearestNeighborHVInitial", "HV Side: Nearest Neighbor Initial", 200, 0, 1000);
  m_EnergyHistogramNearestNeighborHVInitial->SetXTitle("Energy [keV]");
  m_EnergyHistogramNearestNeighborHVInitial->SetYTitle("Counts");
  m_EnergyHistogramNearestNeighborHVInitial->SetLineColor(kViolet);
  m_EnergyHistogramNearestNeighborHVInitial->SetLineWidth(1);
  m_EnergyHistogramNearestNeighborHVInitial->SetFillColorAlpha(kGray+4, 0.2);
  m_EnergyHistogramNearestNeighborHVInitial->SetFillStyle(1001);
  m_EnergyHistogramNearestNeighborHVInitial->SetDirectory(0);
  
  // LV Strips Final
  m_EnergyHistogramLVFinal = new TH1D("EnergyLVFinal", "LV Side: Strips Final", 200, 0, 1000);
  m_EnergyHistogramLVFinal->SetXTitle("Energy [keV]");
  m_EnergyHistogramLVFinal->SetYTitle("Counts");
  m_EnergyHistogramLVFinal->SetLineColor(kAzure+7);
  m_EnergyHistogramLVFinal->SetLineWidth(1);
  m_EnergyHistogramLVFinal->SetFillColorAlpha(kAzure+7, 1.0);
  m_EnergyHistogramLVFinal->SetFillStyle(1001);
  m_EnergyHistogramLVFinal->SetDirectory(0);

  // HV Strips Final
  m_EnergyHistogramHVFinal = new TH1D("EnergyHVFinal", "HV Side: Strips Final", 200, 0, 1000);
  m_EnergyHistogramHVFinal->SetXTitle("Energy [keV]");
  m_EnergyHistogramHVFinal->SetYTitle("Counts");
  m_EnergyHistogramHVFinal->SetLineColor(kAzure+7);
  m_EnergyHistogramHVFinal->SetLineWidth(1);
  m_EnergyHistogramHVFinal->SetFillColorAlpha(kAzure+7, 1.0);
  m_EnergyHistogramHVFinal->SetFillStyle(1001);
  m_EnergyHistogramHVFinal->SetDirectory(0);
  
  // LV Nearest Neighbor Final
  m_EnergyHistogramNearestNeighborLVFinal = new TH1D("EnergyNearestNeighborLVFinal", "LV Side: Nearest Neighbor Final", 200, 0, 1000);
  m_EnergyHistogramNearestNeighborLVFinal->SetXTitle("Energy [keV]");
  m_EnergyHistogramNearestNeighborLVFinal->SetYTitle("Counts");
  m_EnergyHistogramNearestNeighborLVFinal->SetLineColor(kViolet);
  m_EnergyHistogramNearestNeighborLVFinal->SetLineWidth(1);
  m_EnergyHistogramNearestNeighborLVFinal->SetFillColorAlpha(kViolet, 0.5);
  m_EnergyHistogramNearestNeighborLVFinal->SetFillStyle(1001);
  m_EnergyHistogramNearestNeighborLVFinal->SetDirectory(0);

  // HV Nearest Neighbor Final
  m_EnergyHistogramNearestNeighborHVFinal = new TH1D("EnergyNearestNeighborHVFinal", "HV Side: Nearest Neighbor Final", 200, 0, 1000);
  m_EnergyHistogramNearestNeighborHVFinal->SetXTitle("Energy [keV]");
  m_EnergyHistogramNearestNeighborHVFinal->SetYTitle("Counts");
  m_EnergyHistogramNearestNeighborHVFinal->SetLineColor(kViolet);
  m_EnergyHistogramNearestNeighborHVFinal->SetLineWidth(1);
  m_EnergyHistogramNearestNeighborHVFinal->SetFillColorAlpha(kViolet, 0.5);
  m_EnergyHistogramNearestNeighborHVFinal->SetFillStyle(1001);
  m_EnergyHistogramNearestNeighborHVFinal->SetDirectory(0);

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}

////////////////////////////////////////////////////////////////////////////////



MGUIExpoPlotSpectrum::~MGUIExpoPlotSpectrum()
{
  // kDeepCleanup is activated
}

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoPlotSpectrum::Reset()
{

  //! Reset the data in the UI
  
  m_Mutex.Lock();

  // Have to add this so we can change the binning
  // Initial
  if (m_EnergyHistogramLVInitial != nullptr){
    m_EnergyHistogramLVInitial->Reset();
  }
  if (m_EnergyHistogramHVInitial != nullptr){
    m_EnergyHistogramHVInitial->Reset();
  }
  if (m_EnergyHistogramNearestNeighborLVInitial != nullptr){
    m_EnergyHistogramNearestNeighborLVInitial->Reset();
  }
  if (m_EnergyHistogramNearestNeighborHVInitial != nullptr){
    m_EnergyHistogramNearestNeighborHVInitial->Reset();
  }
  
  // Final
  if (m_EnergyHistogramLVFinal != nullptr){
    m_EnergyHistogramLVFinal->Reset();
  }
  if (m_EnergyHistogramHVFinal != nullptr){
    m_EnergyHistogramHVFinal->Reset();
  }
  
  if (m_EnergyHistogramNearestNeighborLVFinal != nullptr){
    m_EnergyHistogramNearestNeighborLVFinal->Reset();
  }
  
  if (m_EnergyHistogramNearestNeighborHVFinal != nullptr){
    m_EnergyHistogramNearestNeighborHVFinal->Reset();
  }
  
  // Initial
  m_DataBufferLVInitial.clear();
  m_DataBufferHVInitial.clear();
  m_DataBufferNearestNeighborLVInitial.clear();
  m_DataBufferNearestNeighborHVInitial.clear();
  
  // Final
  m_DataBufferLVFinal.clear();
  m_DataBufferHVFinal.clear();
  m_DataBufferNearestNeighborLVFinal.clear();
  m_DataBufferNearestNeighborHVFinal.clear();
  
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoPlotSpectrum::SetEnergyHistogramParameters(int NBins, double Min, double Max)
{
  //! Set the energy histogram parameters

  // SAFETY FIX: Never allow 0 or negative bins. Default to 100 if something goes wrong.
  if (NBins <= 0) {
    NBins = 100;
  }
  // SAFETY FIX: Ensure Max is actually greater than Min
  if (Max <= Min) {
    Max = Min + 100.0;
  }

  m_Mutex.Lock();
  
  // Initial
  if (m_EnergyHistogramLVInitial != nullptr){
    m_EnergyHistogramLVInitial->SetBins(NBins, Min, Max);
  }
  if (m_EnergyHistogramHVInitial != nullptr){
    m_EnergyHistogramHVInitial->SetBins(NBins, Min, Max);
  }
  if (m_EnergyHistogramNearestNeighborLVInitial != nullptr){
    m_EnergyHistogramNearestNeighborLVInitial->SetBins(NBins, Min, Max);
  }
  if (m_EnergyHistogramNearestNeighborHVInitial != nullptr){
    m_EnergyHistogramNearestNeighborHVInitial->SetBins(NBins, Min, Max);
  }
  
  // Final
  if (m_EnergyHistogramLVFinal != nullptr){
    m_EnergyHistogramLVFinal->SetBins(NBins, Min, Max);
  }
  if (m_EnergyHistogramHVFinal != nullptr){
    m_EnergyHistogramHVFinal->SetBins(NBins, Min, Max);
  }
  if (m_EnergyHistogramNearestNeighborLVFinal != nullptr){
    m_EnergyHistogramNearestNeighborLVFinal->SetBins(NBins, Min, Max);
  }
  if (m_EnergyHistogramNearestNeighborHVFinal != nullptr){
    m_EnergyHistogramNearestNeighborHVFinal->SetBins(NBins, Min, Max);
  }
  
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////



void MGUIExpoPlotSpectrum::AddEnergyInitial(double Energy, bool IsNearestNeighbor, bool IsLV)
{
  //! Add data to the energy histogram Initial
  
  
  // First check if we want to plot the spectrum 
  if (m_PlotSpectrum == false) {
    return;
  }
  
  m_Mutex.Lock();

  if (IsNearestNeighbor == true) {
    if (IsLV == true) {
      if (m_EnergyHistogramNearestNeighborLVInitial != nullptr) {
        m_EnergyHistogramNearestNeighborLVInitial->Fill(Energy);
      }
      m_DataBufferNearestNeighborLVInitial.push_back(Energy);
    } else {
      if (m_EnergyHistogramNearestNeighborHVInitial != nullptr) {
        m_EnergyHistogramNearestNeighborHVInitial->Fill(Energy);
      }
      m_DataBufferNearestNeighborHVInitial.push_back(Energy);
    }
  } else {
    if (IsLV == true) {
      if (m_EnergyHistogramLVInitial != nullptr) {
        m_EnergyHistogramLVInitial->Fill(Energy);
      }
      m_DataBufferLVInitial.push_back(Energy);
    } else {
      if (m_EnergyHistogramHVInitial != nullptr) {
        m_EnergyHistogramHVInitial->Fill(Energy);
      }
      m_DataBufferHVInitial.push_back(Energy);
    }
  }
    
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoPlotSpectrum::AddEnergyFinal(double Energy, bool IsNearestNeighbor, bool IsLV)
{
  //! Add data to the energy histogram Final
  
  // First check if we want to plot the spectrum
  if (m_PlotSpectrum == false) {
    return;
  }
  
  m_Mutex.Lock();

  if (IsNearestNeighbor == true) {
    if (IsLV == true) {
      if (m_EnergyHistogramNearestNeighborLVFinal != nullptr) {
        m_EnergyHistogramNearestNeighborLVFinal->Fill(Energy);
      }
      m_DataBufferNearestNeighborLVFinal.push_back(Energy);
    } else {
      if (m_EnergyHistogramNearestNeighborHVFinal != nullptr) {
        m_EnergyHistogramNearestNeighborHVFinal->Fill(Energy);
      }
      m_DataBufferNearestNeighborHVFinal.push_back(Energy);
    }
  } else {
    if (IsLV == true) {
      if (m_EnergyHistogramLVFinal != nullptr) {
        m_EnergyHistogramLVFinal->Fill(Energy);
      }
      m_DataBufferLVFinal.push_back(Energy);
    } else {
      if (m_EnergyHistogramHVFinal != nullptr) {
        m_EnergyHistogramHVFinal->Fill(Energy);
      }
      m_DataBufferHVFinal.push_back(Energy);
    }
  }
    
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoPlotSpectrum::Export(const MString& FileName)
{
  //! Add data to the energy histogram

  m_Mutex.Lock();

  if (m_EnergyCanvas != nullptr) {
    m_EnergyCanvas->GetCanvas()->SaveAs(FileName);
  }
  
  m_Mutex.UnLock();
}

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoPlotSpectrum::Create()
{
  //! Add the GUI options here

  // Do not create it twice!
  if (m_IsCreated == true) return;

  m_Mutex.Lock();
  
  // Main place to hold controls
  TGLayoutHints* MainLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2);
  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, MainLayout);

  // Control panel (Left Side)
  TGLayoutHints* ControlLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 5, 5, 5, 5);
  TGVerticalFrame* ControlFrame = new TGVerticalFrame(HFrame, 200, 500); // Fixed width for controls
  HFrame->AddFrame(ControlFrame, ControlLayout);

  // Add place to control the range
  TGLayoutHints* RangeLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5);

  m_RangeMinEntry = new MGUIEEntry(ControlFrame, "Min Energy [keV]:", false, 0.0, true, 0.0, 10000.0);
  ControlFrame->AddFrame(m_RangeMinEntry, RangeLayout);

  m_RangeMaxEntry = new MGUIEEntry(ControlFrame, "Max Energy [keV]:", false, 1000.0, true, 0.0, 10000.0);
  ControlFrame->AddFrame(m_RangeMaxEntry, RangeLayout);
  
  // Control the number of bins (by setting the number of keVs per a bin)
  m_BinWidthEntry = new MGUIEEntry(ControlFrame, "Bin Width [keV]:", false, 5.0, true, 0.001, 1000.0);
  ControlFrame->AddFrame(m_BinWidthEntry, RangeLayout);

  m_UpdateRangeButton = new TGTextButton(ControlFrame, "Update Plot Range", c_UpdateRange);
  m_UpdateRangeButton->Associate(this);
  ControlFrame->AddFrame(m_UpdateRangeButton, RangeLayout);
  
  // Log X
  m_LogXButton = new TGCheckButton(ControlFrame, "Log X-Axis", c_LogX);
  m_LogXButton->Associate(this);
  m_LogXButton->SetState(kButtonUp); // Default to off (Linear)
  ControlFrame->AddFrame(m_LogXButton, new TGLayoutHints(kLHintsLeft | kLHintsTop, 10, 2, 5, 2));

  // Log Y
  m_LogYButton = new TGCheckButton(ControlFrame, "Log Y-Axis", c_LogY);
  m_LogYButton->Associate(this);
  m_LogYButton->SetState(kButtonUp); // Default to off (Linear)
  ControlFrame->AddFrame(m_LogYButton, new TGLayoutHints(kLHintsLeft | kLHintsTop, 10, 2, 2, 10));


  // Canvas (Right Side)
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 5, 5, 5, 5);
  
  m_EnergyCanvas = new TRootEmbeddedCanvas("Energy", HFrame, 100, 100);
  HFrame->AddFrame(m_EnergyCanvas, CanvasLayout);
  
  // Draw Histograms
  // Note! We put the Final hisgrams Initial so incase you only want the Final data plotted you can do that (which is the case for the energy calirbation module)
  m_EnergyCanvas->GetCanvas()->Divide(2, 1);
  
  // LV side
  m_EnergyCanvas->GetCanvas()->cd(1);
  // Final
  m_EnergyCanvas->GetCanvas()->cd(1);
  if (m_EnergyHistogramLVFinal != nullptr) {
    m_EnergyHistogramLVFinal->Draw("HIST");
  }
  if (m_EnergyHistogramNearestNeighborLVFinal != nullptr) {
    m_EnergyHistogramNearestNeighborLVFinal->Draw("HIST SAME");
  }
  // Initial
  if (m_EnergyHistogramLVInitial != nullptr) {
    m_EnergyHistogramLVInitial->Draw("HIST SAME");
  }
  if (m_EnergyHistogramNearestNeighborLVInitial != nullptr) {
    m_EnergyHistogramNearestNeighborLVInitial->Draw("HIST SAME");
  }

  // HV side
  m_EnergyCanvas->GetCanvas()->cd(2);
  // Final
  m_EnergyCanvas->GetCanvas()->cd(2);
  if (m_EnergyHistogramHVFinal != nullptr) {
    m_EnergyHistogramHVFinal->Draw("HIST");
  }
  if (m_EnergyHistogramNearestNeighborHVFinal != nullptr) {
    m_EnergyHistogramNearestNeighborHVFinal->Draw("HIST SAME");
  }
  // Initial
  if (m_EnergyHistogramHVInitial != nullptr) {
    m_EnergyHistogramHVInitial->Draw("HIST SAME");
  }
  if (m_EnergyHistogramNearestNeighborHVInitial != nullptr) {
    m_EnergyHistogramNearestNeighborHVInitial->Draw("HIST SAME");
  }
  
  m_EnergyCanvas->GetCanvas()->Update();
  
  m_IsCreated = true;
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIExpoPlotSpectrum::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  //! Process the messages for this GUI

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
    case kCM_CHECKBUTTON:
      switch (Parameter1) {
      case c_UpdateRange:
        OnUpdateRange();
        return true;
      
      case c_LogX:
      case c_LogY:
        OnUpdateRange();
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


void MGUIExpoPlotSpectrum::Update()
{
  //! Update the frame
  
  m_Mutex.Lock();
  
  if (m_EnergyCanvas != nullptr && m_EnergyCanvas->GetCanvas() != nullptr) {
    TCanvas* C = m_EnergyCanvas->GetCanvas();
    if (C->GetPad(1)) C->GetPad(1)->Modified();
    if (C->GetPad(2)) C->GetPad(2)->Modified();
    
    C->Modified();
    C->Update();
  }
    
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoPlotSpectrum::OnUpdateRange()
{
  //! Update the histogram range and binning

  double Min = m_RangeMinEntry->GetAsDouble();
  double Max = m_RangeMaxEntry->GetAsDouble();
  double Width = m_BinWidthEntry->GetAsDouble();

  // Basic sanity checks
  if (Max <= Min) return;
  if (Width <= 0.0) return;

  // If Log X is requested, Min cannot be <= 0. Bump it to a small positive number.
  if (m_LogXButton->IsOn() && Min <= 0.0) {
    Min = 0.1;
  }

  // Calculate bins
  int NBins = (int)((Max - Min) / Width);
   
  // SAFETY FIX: Ensure we have at least one bin
  if (NBins < 1) NBins = 1;

  m_Mutex.Lock();
   
  // Rebin and refill all histograms

  // Helper to refill histograms
  auto RefillHist = [&](TH1D* Hist, const std::vector<double>& Buffer) {
    if (Hist != nullptr) {
      Hist->SetBins(NBins, Min, Max);
      Hist->Reset();
      for (double val : Buffer) {
        Hist->Fill(val);
      }
    }
  };

  // Initial
  RefillHist(m_EnergyHistogramLVInitial, m_DataBufferLVInitial);
  RefillHist(m_EnergyHistogramHVInitial, m_DataBufferHVInitial);
  RefillHist(m_EnergyHistogramNearestNeighborLVInitial, m_DataBufferNearestNeighborLVInitial);
  RefillHist(m_EnergyHistogramNearestNeighborHVInitial, m_DataBufferNearestNeighborHVInitial);

  // Final
  RefillHist(m_EnergyHistogramLVFinal, m_DataBufferLVFinal);
  RefillHist(m_EnergyHistogramHVFinal, m_DataBufferHVFinal);
  RefillHist(m_EnergyHistogramNearestNeighborLVFinal, m_DataBufferNearestNeighborLVFinal);
  RefillHist(m_EnergyHistogramNearestNeighborHVFinal, m_DataBufferNearestNeighborHVFinal);
   
  // Redraw using THStack
  if (m_EnergyCanvas != nullptr) {
    auto C = m_EnergyCanvas->GetCanvas();
    
    // Check button states
    int logX = m_LogXButton->IsOn() ? 1 : 0;
    int logY = m_LogYButton->IsOn() ? 1 : 0;
    
    // LV Side
    C->cd(1);
    gPad->SetLogx(logX); // Apply Log X
    gPad->SetLogy(logY); // Apply Log Y
    C->GetPad(1)->Clear();
    
    THStack* LVStack = new THStack("lvStack", "LV Energy Spectrum");
    if (m_EnergyHistogramLVInitial != nullptr){
      LVStack->Add(m_EnergyHistogramLVInitial);
    }
    if (m_EnergyHistogramNearestNeighborLVInitial != nullptr) {
      LVStack->Add(m_EnergyHistogramNearestNeighborLVInitial);
    }
    if (m_EnergyHistogramLVFinal != nullptr) {
      LVStack->Add(m_EnergyHistogramLVFinal);
    }
    if (m_EnergyHistogramNearestNeighborLVFinal != nullptr) {
      LVStack->Add(m_EnergyHistogramNearestNeighborLVFinal);
    }

    // Scaling to the plot with the most data :)
    LVStack->Draw("nostack hist");
    LVStack->GetXaxis()->SetTitle("Energy [keV]");
    LVStack->GetYaxis()->SetTitle("Counts");

    // HV
    C->cd(2);
    gPad->SetLogx(logX);
    gPad->SetLogy(logY);
    C->GetPad(2)->Clear();

    THStack* HVStack = new THStack("hvStack", "HV Energy Spectrum");

    if (m_EnergyHistogramHVInitial != nullptr) {
      HVStack->Add(m_EnergyHistogramHVInitial);
    }
    if (m_EnergyHistogramNearestNeighborHVInitial != nullptr) {
      HVStack->Add(m_EnergyHistogramNearestNeighborHVInitial);
    }
    if (m_EnergyHistogramHVFinal != nullptr) {
      HVStack->Add(m_EnergyHistogramHVFinal);
    }
    if (m_EnergyHistogramNearestNeighborHVFinal != nullptr) {
      HVStack->Add(m_EnergyHistogramNearestNeighborHVFinal);
    }

    // Scaling to the plot with the most data :)
    HVStack->Draw("nostack hist");
    HVStack->GetXaxis()->SetTitle("Energy [keV]");
    HVStack->GetYaxis()->SetTitle("Counts");
    
    C->Modified();
    C->Update();
  }
   
  m_Mutex.UnLock();
}

// MGUIExpoPlotSpectrum the end...
////////////////////////////////////////////////////////////////////////////////
