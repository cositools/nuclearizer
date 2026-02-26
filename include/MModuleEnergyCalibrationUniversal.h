/*
 * MModuleEnergyCalibrationUniversal.h
 *
 * Copyright (C) by Andreas Zoglauer
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleEnergyCalibrationUniversal__
#define __MModuleEnergyCalibrationUniversal__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElementDoubleStrip.h"
#include "MGUIEEntry.h"

// Neclearizer libe:
#include "MModule.h"
#include "MCalibratorEnergy.h"
//#include "MGUIExpoEnergyCalibration.h"
#include "MGUIExpoPlotSpectrum.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Definition of the slow threshold modes
// Attention: The ints are also used in the UI, thus don't change the numbers
enum class MSlowThresholdCutModes : int
{
  e_Ignore = 0,
  e_Fixed = 1,
  e_File = 2
};

//! Definition of the Nearest Neighbor modes
enum class MNearestNeighborCutModes : int
{
  e_Ignore = 100,
  e_Fixed = 101
};

//! Definition of the plot spectrum modes
// Attention: The ints are also used in the Plot Spectrum Model! Thus don't change the numbers
enum MPlotSpectrumModes : int
{
  e_PlotNone = 200,
  e_PlotNoBuffer = 201,
  e_PlotWithBuffer = 202
};


////////////////////////////////////////////////////////////////////////////////


//! A universal energy calibrator
class MModuleEnergyCalibrationUniversal : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleEnergyCalibrationUniversal();
  //! Default destructor
  virtual ~MModuleEnergyCalibrationUniversal();
  
  //! Create a new object of this class 
  virtual MModuleEnergyCalibrationUniversal* Clone() { return new MModuleEnergyCalibrationUniversal(); }

  //! Set the calibration file name
  void SetFileName(const MString& FileName) { m_FileName = FileName; }
  //! Get the calibration file name
  MString GetFileName() const { return m_FileName; }
 
  //! Set the slow threshold mode
  void SetSlowThresholdCutMode(const MSlowThresholdCutModes& Mode) { m_SlowThresholdCutMode = Mode; }
  //! Get the slow threshold mode
  MSlowThresholdCutModes GetSlowThresholdCutMode() const { return m_SlowThresholdCutMode; }

  //! Set the slow threshold value for fixed mode
  void SetSlowThresholdCutFixedValue(double Value) { m_SlowThresholdCutFixedValue = Value; }
  //! Get the slow threshold value for fixed mode
  double GetSlowThresholdCutFixedValue() const { return m_SlowThresholdCutFixedValue; }

  //! Set the slow threshold cut file name
  void SetSlowThresholdCutFileName(const MString& SlowThresholdCutFileName) { m_SlowThresholdCutFileName = SlowThresholdCutFileName; }
  //! Get the slow threshold cut calibration file name
  MString GetSlowThresholdCutFileName() const {return m_SlowThresholdCutFileName; }
  
  // Nearest Neighbors
  //! Set the Nearest Neighbor mode
  void SetNearestNeighborCutMode(const MNearestNeighborCutModes& Mode) { m_NearestNeighborCutMode = Mode; }
  //! Get the Nearest Neighbor mode
  MNearestNeighborCutModes GetNearestNeighborCutMode() const { return m_NearestNeighborCutMode; }

  //! Set the threshold value for Nearest Neighbors
  void SetNearestNeighborThreshold(double Threshold) { m_NearestNeighborThreshold = Threshold; }
  //! Get the threshold value for Nearest Neighbors
  double GetNearestNeighborThreshold() const { return m_NearestNeighborThreshold; }
  
  // Spectrum plotting options
  //! Set the plot spectrum mode
  void SetPlotSpectrumMode(const MPlotSpectrumModes& Mode) { m_PlotSpectrumMode = Mode; }
  //! Get the plot spectrum mode
  MPlotSpectrumModes GetPlotSpectrumMode() const { return m_PlotSpectrumMode; }
 
 
  //! Create the expos
  virtual void CreateExpos();
  
  //! Initialize the module
  virtual bool Initialize();
  
  //! Finalize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  //! Look up energy resolution
  double LookupEnergyResolution(MStripHit* SH, double Energy);
  
  //! Standalone function to return energy of certain strip given ADC
  double GetEnergy(MReadOutElementDoubleStrip R, double ADC);
  //! Standalone function to return ADC of certain strip given energy
  double GetADC(MReadOutElementDoubleStrip R, double energy);


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The energy calibration file name
  MString m_FileName;

  //! The slow threshold cut mode (see defnition of MModuleEnergyCalibrationUniversalSlowThresholdCutModes)
  MSlowThresholdCutModes m_SlowThresholdCutMode;

  //! The slow threshold cut value
  double m_SlowThresholdCutFixedValue;

  //! The slow threshold cut file name
  MString m_SlowThresholdCutFileName;
  
  //! The Nearest Neighbor cut mode
  MNearestNeighborCutModes m_NearestNeighborCutMode;
    
  //! The Nearest Neighbor threshold value
  double m_NearestNeighborThreshold;
  
  //! The spectrum plotting mode
  MPlotSpectrumModes m_PlotSpectrumMode;
  
  

  // private members:
 private:
  //! A GUI to display the final energy histogram
  //MGUIExpoEnergyCalibration* m_ExpoEnergyCalibration;
  
  //! Updated GUI to display the energy histogram
  MGUIExpoPlotSpectrum* m_ExpoSpectrum;
   
  //! Calibrators arranged by detectors
  //vector<vector<MCalibratorEnergy*> > m_Calibrators;
  //! Associated detector IDs
  vector<unsigned int> m_DetectorIDs;
  //! Calibration map between read-out element and fitted function
  map<MReadOutElementDoubleStrip, TF1*> m_Calibration; // TF1* is a function to be applied
  //! Resolution Calibration map between read-out element and fitted function
  map<MReadOutElementDoubleStrip, TF1*> m_ResolutionCalibration;
  //! Temperature Calibration map between read-out element and fitted function
  map<MReadOutElementDoubleStrip, double> m_ThresholdMap;
 
#ifdef ___CLING___
 public:
  ClassDef(MModuleEnergyCalibrationUniversal, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
