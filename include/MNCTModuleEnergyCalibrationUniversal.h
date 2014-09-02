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

  //! Set the calibration file name
  void SetFileName(const MString& FileName) { m_FileName = FileName; }
  //! Get the calibration file name
  MString GetFileName() const { return m_FileName; }
  
  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:
   //! The calibration file name
   MString m_FileName;

  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:
  //! A GUI to display the final energy histogram
  MGUIExpoEnergyCalibration* m_ExpoEnergyCalibration;
   
  //! Calibrators arranged by detectors
  vector<vector<MCalibratorEnergy*> > m_Calibrators;
  //! Associated detector IDs
  vector<unsigned int> m_DetectorIDs;
  //! Calibration map between read-out element and fitted function
  map<MReadOutElementDoubleStrip, TF1*> m_Calibration;
  
   
#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleEnergyCalibrationUniversal, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
