/*
 * MNCTModuleEnergyCalibrationUniversal.h
 *
 * Copyright (C) by Andreas Zoglauer
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleEnergyCalibrationUniversal__
#define __MNCTModuleEnergyCalibrationUniversal__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElementDoubleStrip.h"

// Neclearizer libe:
#include "MModule.h"
#include "MCalibratorEnergy.h"
#include "MGUIExpoEnergyCalibration.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! A universal energy calibrator
class MNCTModuleEnergyCalibrationUniversal : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleEnergyCalibrationUniversal();
  //! Default destructor
  virtual ~MNCTModuleEnergyCalibrationUniversal();
  
  //! Create a new object of this class 
  virtual MNCTModuleEnergyCalibrationUniversal* Clone() { return new MNCTModuleEnergyCalibrationUniversal(); }

  //! Set the calibration file name
  void SetFileName(const MString& FileName) { m_FileName = FileName; }
  //! Get the calibration file name
  MString GetFileName() const { return m_FileName; }
  
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
  double LookupEnergyResolution(MNCTStripHit* SH, double Energy);

  //! Enable/Disable Preamp Temp Correction
  void EnablePreampTempCorrection(bool X) {m_TemperatureEnabled = X;}
  //! Get coincidence merging true/false
  bool GetPreampTempCorrection() const { return m_TemperatureEnabled; }


	//! Standalone function to return energy of certain strip given ADC
	double GetEnergy(MReadOutElementDoubleStrip R, double ADC);
	//! Standalone function to return ADC of certain strip given energy
	double GetADC(MReadOutElementDoubleStrip R, double energy);


  // protected methods:
 protected:
  //! The calibration file name
  MString m_FileName;
  //! Preamp Temperature Correction
  bool m_TemperatureEnabled;


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
  map<MReadOutElementDoubleStrip, TF1*> m_Calibration;
  //! Resolution Calibration map between read-out element and fitted function
	map<MReadOutElementDoubleStrip, TF1*> m_ResolutionCalibration;
   
#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleEnergyCalibrationUniversal, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
