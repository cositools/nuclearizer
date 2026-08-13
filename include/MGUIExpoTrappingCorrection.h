/*
 * MGUIExpoTrappingCorrection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoTrappingCorrection__
#define __MGUIExpoTrappingCorrection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TString.h>
#include <TGClient.h>
#include <TRootEmbeddedCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TLegend.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGNumberEntry.h>
#include <TGLabel.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIERBList.h"

// NuSTAR libs
#include "MGUIExpo.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExpoTrappingCorrection : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoTrappingCorrection(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoTrappingCorrection();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Export the data in the UI
  virtual void Export(const MString& FileName);

  //! Set the energy histogram parameters 
  void SetEnergyHistogramParameters(int NBins, double Min, double Max);

  //! Add data to the uncorrected energy histogram
  void AddEnergyInitial(double Energy, bool IsNearestNeighbor, bool IsLV);

  //! Add data to the corrected energy histogram
  void AddEnergyFinal(double Energy, bool IsNearestNeighbor, bool IsLV);

  // //! Add data to the energy histogram
  // void AddEnergy(double Energy);

  // Callback slot for the Apply button
  void OnApply();

  // Getters for Finalize analysis
  TH1D* GetEnergyHistogramLVInitial() const { return m_EnergyLVInitial; }
  TH1D* GetEnergyHistogramLVFinal()   const { return m_EnergyLVFinal; }
  TH1D* GetEnergyHistogramHVInitial() const { return m_EnergyHVInitial; }
  TH1D* GetEnergyHistogramHVFinal()   const { return m_EnergyHVFinal; }

  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  // //! Energy canvas
  // TRootEmbeddedCanvas* m_EnergyCanvas;
  // //! Energy histogram
  // TH1D* m_Energy;

  //! Energy Histograms
  TH1D* m_EnergyLVInitial;
  TH1D* m_EnergyLVFinal;
  TH1D* m_EnergyHVInitial;
  TH1D* m_EnergyHVFinal;

  //! Canvases & Display elements
  TRootEmbeddedCanvas* m_CanvasLV;
  TRootEmbeddedCanvas* m_CanvasHV;
  TLegend* m_LegendLV;
  TLegend* m_LegendHV;

  //! Interactive Control Widgets
  TGNumberEntry* m_EntryNBins;
  TGNumberEntry* m_EntryMinEnergy;
  TGNumberEntry* m_EntryMaxEnergy;
  TGTextButton*  m_ButtonApply;



#ifdef ___CLING___
 public:
  ClassDef(MGUIExpoTrappingCorrection, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
