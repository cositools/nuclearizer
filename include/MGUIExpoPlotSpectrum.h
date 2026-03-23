/*
 * MGUIExpoPlotSpectrum.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 */

#ifndef __MGUIExpoPlotSpectrum__
#define __MGUIExpoPlotSpectrum__

// Standard libs:
#include <vector>

// ROOT libs:
#include <TH1D.h>
#include <TRootEmbeddedCanvas.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIExpo.h"
#include "MGUIEEntry.h"
#include "MModule.h"

// Forward declarations:
class MModule;

////////////////////////////////////////////////////////////////////////////////

class MGUIExpoPlotSpectrum : public MGUIExpo
{
public:
  //! Standard constructor
  MGUIExpoPlotSpectrum(MModule* Module);
  //! Standard destructor
  virtual ~MGUIExpoPlotSpectrum();

  //! Reset the data in the UI
  virtual void Reset();
  
  //! Set the histogram parameters
  virtual void SetEnergyHistogramParameters(int NBins, double Min, double Max);
  
  //! Add energy to the histograms inital
  virtual void AddEnergyInitial(double Energy, bool IsNearestNeighbor, bool IsLV);
  
  //! Add energy to the histograms final
  virtual void AddEnergyFinal(double Energy, bool IsNearestNeighbor, bool IsLV);
  
  //! Save plots
  virtual void  Export(const MString& FileName);

  //! Create the GUI
  virtual void Create();

  //! Process all GUI commands
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  //! Update the frame
  virtual void Update();


protected:
  //! Update the histogram range
  void OnUpdateRange();

private:
  
  // GUI Elements
  TRootEmbeddedCanvas* m_EnergyCanvas;
  
  MGUIEEntry* m_RangeMinEntry;
  MGUIEEntry* m_RangeMaxEntry;
  MGUIEEntry* m_BinWidthEntry;
  
  TGTextButton* m_UpdateRangeButton;
  TGCheckButton* m_LogXButton;
  TGCheckButton* m_LogYButton;
  
  // Define unique IDs for the buttons
  enum {
    c_UpdateRange = 101,
    c_LogX = 102,
    c_LogY = 103
  };

  // Histograms
  TH1D* m_EnergyHistogramLVInitial;
  TH1D* m_EnergyHistogramHVInitial;
  TH1D* m_EnergyHistogramNearestNeighborLVInitial;
  TH1D* m_EnergyHistogramNearestNeighborHVInitial;
  TH1D* m_EnergyHistogramLVFinal;
  TH1D* m_EnergyHistogramHVFinal;
  TH1D* m_EnergyHistogramNearestNeighborLVFinal;
  TH1D* m_EnergyHistogramNearestNeighborHVFinal;

  // Data buffers (to hold data Initial window opens)
  std::vector<double> m_DataBufferLVInitial;
  std::vector<double> m_DataBufferHVInitial;
  std::vector<double> m_DataBufferNearestNeighborLVInitial;
  std::vector<double> m_DataBufferNearestNeighborHVInitial;
  std::vector<double> m_DataBufferLVFinal;
  std::vector<double> m_DataBufferHVFinal;
  std::vector<double> m_DataBufferNearestNeighborLVFinal;
  std::vector<double> m_DataBufferNearestNeighborHVFinal;
  
  bool m_PlotSpectrum;

#ifdef ___CLING___
public:
  ClassDef(MGUIExpoPlotSpectrum, 0)
#endif

};

#endif
