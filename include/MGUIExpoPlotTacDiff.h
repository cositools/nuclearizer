#ifndef __MGUIExpoPlotTacDiff__
#define __MGUIExpoPlotTacDiff__

#include <map>
#include <vector>

#include <TH2D.h>
#include <TGComboBox.h>
#include <TRootEmbeddedCanvas.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TGLayout.h>
#include <TGNumberEntry.h>

#include "MGUIExpo.h"
#include "MModule.h"

class MGUIExpoPlotTacDiff : public MGUIExpo
{
public:
  MGUIExpoPlotTacDiff(MModule* Module);
  virtual ~MGUIExpoPlotTacDiff();

  virtual void Reset();
  virtual void Create();
  virtual void Update();
  virtual void Export(const MString& FileName);

  void SetHistogramParameters(unsigned int DetID, unsigned int NBinsDepth, double DepthMin, double DepthMax,
                              unsigned int NBinsDtac, double DtacMin, double DtacMax,
                              unsigned int NBinsFrac, double FracMin, double FracMax);
  void AddData(int StripPairCode, double Depth, double dTac, double Fraction, double dTacAlt,double Energy);

  //void OnDetectorSelected(Int_t DetID);
  //void OnSideSelected(Int_t Side);
  //void OnStripSelected();
  //void OnEnergyRangeChanged();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  void OnUpdateSelection();


protected:
  void RedrawPlots();

  int m_SelectedDetector;
  int m_SelectedSide;
  int m_SelectedStrip;
  int m_SelectedStripPairCode;

  TGComboBox* m_DetectorSelector;
  TGComboBox* m_SideSelector;
  TGNumberEntry* m_StripEntry;
  TGNumberEntry* m_EnergyMinEntry;
  TGNumberEntry* m_EnergyMaxEntry;
  double m_EnergyMin;
  double m_EnergyMax;

  TRootEmbeddedCanvas* m_DtacVsDepthCanvas;
  TRootEmbeddedCanvas* m_DtacVsFracCanvas;
  TRootEmbeddedCanvas* m_DtacVsDtacCanvas;

  // Key: StripPairCode = 10000*DetID + 100*LVStripID (for LV) or 10000*DetID + HVStripID (for HV)
  std::map<int, TH2D*> m_DtacVsDepthHistograms;
  std::map<int, TH2D*> m_DtacVsFracHistograms;
  std::map<int, TH2D*> m_DtacVsDtacHistograms;

  std::map<unsigned int, unsigned int> m_NBinsDepth;
  std::map<unsigned int, double> m_DepthMin;
  std::map<unsigned int, double> m_DepthMax;
  std::map<unsigned int, unsigned int> m_NBinsDtac;
  std::map<unsigned int, double> m_DtacMin;
  std::map<unsigned int, double> m_DtacMax;
  std::map<unsigned int, unsigned int> m_NBinsFrac;
  std::map<unsigned int, double> m_FracMin;
  std::map<unsigned int, double> m_FracMax;

  std::vector<unsigned int> m_DetIDs;
private: 
  static const int c_UpdateSelection = 1001;
  TGTextButton* m_UpdateSelectionButton;

#ifdef ___CLING___
public:
  ClassDef(MGUIExpoPlotTacDiff, 0)
#endif
};

#endif
