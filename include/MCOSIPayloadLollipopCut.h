/*
 * MCOSIPayloadLollipopCut.h
 *
 * Copyright (C) by Robin Anthony-Petersen.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

#ifndef __MCOSIPayloadLollipopCut__
#define __MCOSIPayloadLollipopCut__

////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <map>
#include <vector>
#include <string>

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs:
#include "MModule.h"
#include "MReadOutAssembly.h"
#include "MGUIExpoPlotSpectrum.h"

////////////////////////////////////////////////////////////////////////////////

//! Structure storing allowed detector IDs and their target illuminated sides
struct KeptDetectorSide {
  int DetectorIlluminatedID;
  bool isLV;
};

////////////////////////////////////////////////////////////////////////////////

class MCOSIPayloadLollipopCut : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MCOSIPayloadLollipopCut();
  //! Default destructor
  virtual ~MCOSIPayloadLollipopCut();

  //! Initialize module and parameters
  virtual bool Initialize() override;

  //! Main event processing loop
  virtual bool AnalyzeEvent(MReadOutAssembly* Event) override;
  
  //! Create a clone of this module
  virtual MModule* Clone() override;

  //! Filters events based on lollipop configuration and CTD timing
  bool ApplyKeepSidesIlluminatedByLollipops(MReadOutAssembly* Event);
  
  //! Check if module has options GUI
  virtual bool HasOptionsGUI() {
    return true;
  }
    
  //! Show options GUI
  virtual void ShowOptionsGUI();
  
  //! Create exposures and plots
  virtual void CreateExpos() override;
  
  
  int GetSource1Position() const {
    return m_Source1Position;
  }
  
  int GetSource2Position() const {
    return m_Source2Position;
  }
  
  void SetSource1Position(int pos) {
    m_Source1Position = pos;
  }
  
  void SetSource2Position(int pos) {
    m_Source2Position = pos;
  }
  
  void UpdateKeptDetectors();
  
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  virtual MXmlNode* CreateXmlConfiguration();

  // private methods:
 private:
  void SetupLollipopMapping();

  // private members:
 private:
  
  //! Source location 
  int m_Source1Position;
  int m_Source2Position;
  
  //! Master toggle for the lollipop cut
  bool m_ApplyKeepSidesIlluminatedByLollipops;
  
  ///! GUI toggle flags for Source 1 positions
  bool m_Source1_Q0Q1_L0L1;
  bool m_Source1_Q1Q2_L0L1;
  bool m_Source1_Q0Q1_L1L2;
  bool m_Source1_Q1Q2_L1L2;
  bool m_Source1_Q0Q1_L2L3;
  bool m_Source1_Q1Q2_L2L3;

  ///! GUI toggle flags for Source 2 positions
  bool m_Source2_Q2Q3_L0L1;
  bool m_Source2_Q3Q0_L0L1;
  bool m_Source2_Q2Q3_L1L2;
  bool m_Source2_Q3Q0_L1L2;
  bool m_Source2_Q2Q3_L2L3;
  bool m_Source2_Q3Q0_L2L3;
  
  //! Vector storing detector positions in QxLy positions and sides to keep
  std::map<std::string, bool> DetectorSideIlluminated;

  //! Vector storing detector IDs and sides to keep
  std::vector<KeptDetectorSide> m_DetectorsToKeep;
  
  //! Exposure plot spectrum GUI element
  MGUIExpoPlotSpectrum* m_ExpoEnergySpectrum;

#ifdef ___CLING___
 public:
  ClassDef(MCOSIPayloadLollipopCut, 0) // Cut module for lollipop illuminated sides
#endif
};

#endif

////////////////////////////////////////////////////////////////////////////////
