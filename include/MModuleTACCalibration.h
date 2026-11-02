/*
 * MModuleTACCalibration.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleTACCalibration__
#define __MModuleTACCalibration__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Nuclearizer libs:
#include "MStripMap.h"


// Forward declarations:
class MGUIExpoTACcut;
class MGUIExpoPlotSpectrum;


////////////////////////////////////////////////////////////////////////////////


class MModuleTACCalibration : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleTACCalibration();
  //! Default destructor
  virtual ~MModuleTACCalibration();
  
  //! Create a new object of this class 
  virtual MModuleTACCalibration* Clone() { return new MModuleTACCalibration(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  //! Create Expos
  virtual void CreateExpos();

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);

  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  //! Set filename for TAC calibration
  void SetTACCalFileName( const MString& FileName) { m_TACCalFile = FileName; }

  //! Get filename for TAC calibration
  MString GetTACCalFileName() const {return m_TACCalFile;}

  //! Set the strip map file name - only used if no preceding HDF or L0 loader or DEE provides a strip map
  void SetStripMapFileName(const MString& FileName) { m_StripMapFileName = FileName; }

  //! Get the strip map file name
  MString GetStripMapFileName() const { return m_StripMapFileName; }

  //! Return the closest preceding module in the module sequence which provides a strip map (HDF or L0 loader, or DEE),
  //! or nullptr if there is none
  MModule* FindStripMapProvider();

  //! Load the TAC calibration file - the strip map is required, the loading fails without one
  //! Detector, side, and strip of each entry are taken from the strip map via the read-out ID in the first column,
  //! and entries whose read-out ID is not in the strip map are skipped.
  //! The detector column of the file natively holds the DIB ID, not the detector ID, and the side column
  //! does not account for the ASIC polarities, thus both are ignored - only the strip column is cross-checked
  bool LoadTACCalFile(MString FName, const MStripMap* StripMap);

  //! Set the TAC calibration parameters
  void SetTACCalParameters(unordered_map<int, vector<unordered_map<int, vector<double>>>> TACCal) { m_TACCal = TACCal; }

  //! Get the TAC calibration parameters
  unordered_map<int, vector<unordered_map<int, vector<double>>>> GetTACCalParameters() { return m_TACCal; }

  //! Enable or disable TAC cuts
  void SetApplyTACCuts(bool ApplyTACCuts) { m_ApplyTACCuts = ApplyTACCuts; }

  //! Return whether TAC cuts are enabled
  bool GetApplyTACCuts() const { return m_ApplyTACCuts; }

  //! Set TAC coincidence window in ns
  void SetCoincidenceWindow(double CoincidenceWindow) { m_CoincidenceWindow = CoincidenceWindow; }

  //! Get TAC coincidence window in ns
  double GetCoincidenceWindow() const { return m_CoincidenceWindow; }


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 


  // protected methods:
 protected:

  
  // private methods:
 private:

  //! Return the strip map of the closest preceding HDF or L0 loader or DEE, or nullptr if there is none
  const MStripMap* FindUpstreamStripMap();

  //! Apply TAC calibration to strip hits
  bool ApplyTACCal(MReadOutAssembly* Event);

  //! Apply TAC cuts to calibrated strip hits
  //! Assumes ApplyTACCal() has already been called for this event
  bool ApplyTACCuts(MReadOutAssembly* Event);


  // protected members:
 protected:

  // private members:
 private:

  //! TAC calibration parameter file name
  MString m_TACCalFile;

  //! Strip map file name - only used if no preceding HDF or L0 loader or DEE provides a strip map
  MString m_StripMapFileName;

  //! The strip map loaded from m_StripMapFileName if no preceding HDF or L0 loader or DEE provides one
  MStripMap m_StripMap;

  //! Map DetID -> Side (LV=0, HV=1) -> Strip ID -> TAC calibration parameters
  //! DetID, Side, and Strip ID are taken from the strip map via the read-out ID of each entry
  unordered_map<int, vector<unordered_map<int, vector<double>>>> m_TACCal;

  //! Map characters representing detector sides to LV/HV indices
  unordered_map<char, int> m_SideToIndex;

  //! Vector of Detector IDs
  vector<unsigned int> m_DetectorIDs;

  //! TAC coincidence window in ns
  double m_CoincidenceWindow;

  //! Option to apply TAC cuts after TAC calibration
  bool m_ApplyTACCuts;

  //! TAC distribution and energy spectra
  MGUIExpoTACcut* m_ExpoTACcut;
  MGUIExpoPlotSpectrum* m_ExpoEnergySpectrum;

  //! Option to plot energy spectra in GUI
  bool m_PlotEnergySpectrum;

#ifdef ___CLING___
 public:
  ClassDef(MModuleTACCalibration, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
