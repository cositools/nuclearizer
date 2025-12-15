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
#include "MGUIExpoEnergyCalibration.h"

// Forward declarations:


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

  //! Enable/Disable soft threshold from File
  void SetThresholdFileEnable(bool X) {m_ThresholdFileEnabled = X;}
  //! Get threshold from file true/false
  bool GetThresholdFileEnable() const { return m_ThresholdFileEnabled; }
  
  //! Set the calibration file name
  void SetFileName(const MString& FileName) { m_FileName = FileName; }
  //! Get the calibration file name
  MString GetFileName() const { return m_FileName; }
 
  //! Set the threshold calibration file name
  void SetThresholdFileName(const MString& ThresholdFileName) {m_ThresholdFileName = ThresholdFileName; }
  //! Get the threshold calibration file name
  MString GetThresholdFileName() const {return m_ThresholdFileName; }
 
  //! Enable/Disable soft threshold value from GUI input
  void SetThresholdValueEnable(bool X) {m_ThresholdValueEnabled = X;}
  //! Get threshold value from GUI input true/false
  bool GetThresholdValueEnable() const { return m_ThresholdValueEnabled; }
  
  //! Set the threshold value
  void SetThresholdValue(double ThresholdValue) { m_ThresholdValue = ThresholdValue; }
  //! Get the threshold value
  double GetThresholdValue() const { return m_ThresholdValue; }

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
  //! The energy calibration file name
  MString m_FileName;
  
  //! Threshold file enable boolean
  bool m_ThresholdFileEnabled;
  //! The threshold file name
  MString m_ThresholdFileName;
  
  //! Threshold value enable boolean
  bool m_ThresholdValueEnabled;
  //! The threshold value
  double m_ThresholdValue;
  

  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:
  //! A GUI to display the final energy histogram
  MGUIExpoEnergyCalibration* m_ExpoEnergyCalibration;
   
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
