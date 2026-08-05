/*
 * MModuleDEESMEX.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleDEESMEX__
#define __MModuleDEESMEX__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs:
#include "MModule.h"
#include "MSubModuleDEEIntake.h"
#include "MSubModuleRandomCoincidence.h"
#include "MSubModuleShieldEnergyCorrection.h"
#include "MSubModuleShieldReadout.h"
#include "MSubModuleShieldTrigger.h"
#include "MSubModuleChargeTransport.h"
#include "MSubModuleStripReadout.h"
#include "MSubModuleStripReadoutNoise.h"
#include "MSubModuleStripTrigger.h"
#include "MSubModuleDepthReadout.h"
#include "MSubModuleDEEOutput.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleDEESMEX : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleDEESMEX();
  //! Default destructor
  virtual ~MModuleDEESMEX();

  //! Create a new object of this class
  virtual MModuleDEESMEX* Clone()
  {
    MModuleDEESMEX* M = new MModuleDEESMEX();
    M->SetGeometry(m_Geometry);
    return M;
  }

  //! Set the geometry
  virtual void SetGeometry(MDGeometryQuest* Geometry)
  {
    MModule::SetGeometry(Geometry);
  }

  //! Set geometry file name
  void SetGeometryFileName(const MString& FileName)
  {
    cout << "Use SetGeometry instead" << endl;
    abort();
  }

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Finalize the module
  virtual void Finalize();

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // Pass through interfaces to sub-modules

  //! Set energy calibration file name
  void SetEnergyCalibrationFileName(const MString& FileName)
  {
    m_StripReadout.SetEnergyCalibrationFileName(FileName);
  }
  //! Get energy calibration file name
  MString GetEnergyCalibrationFileName() const
  {
    return m_StripReadout.GetEnergyCalibrationFileName();
  }

  //! Set depth coefficients file name
  void SetDepthCoefficientsFileName(const MString& FileName) { m_DepthCoefficientsFileName = FileName; }
  //! Get depth coefficients file name
  MString GetDepthCoefficientsFileName() const { return m_DepthCoefficientsFileName; }

  //! Set depth splines file name
  void SetDepthSplinesFileName(const MString& FileName) { m_DepthSplinesFileName = FileName; }
  //! Get depth splines file name
  MString GetDepthSplinesFileName() const { return m_DepthSplinesFileName;}

  //! Set TAC calibration file name
  void SetTACCalFileName(const MString& FileName)
  {
    m_DepthReadout.SetTACCalFileName(FileName);
  }
  //! Get TAC calibration file name
  MString GetTACCalFileName() const
  {
    return m_DepthReadout.GetTACCalFileName();
  }


  //! Set shield energy correction file name
  void SetShieldEnergyCorrectionFileName(const MString& FileName)
  {
    m_ShieldEnergyCorrection.SetShieldEnergyCorrectionFileName(FileName);
  }
  //! Get shield energy correction file name
  MString GetShieldEnergyCorrectionFileName() const
  {
    return m_ShieldEnergyCorrection.GetShieldEnergyCorrectionFileName();
  }

  //! Set dead time file name
  void SetDeadtimeFileName(const MString& FileName)
  {
    m_StripTrigger.SetDeadtimeFileName(FileName);
  }
  //! Get dead time file name
  MString GetDeadtimeFileName() const
  {
    return m_StripTrigger.GetDeadtimeFileName();
  }
  
  //! Button to apply the FWHM energy resolution to the energies
  bool GetApplyResolutionCalibration() const { return m_ApplyResolutionCalibration; }
  void SetApplyResolutionCalibration(bool ApplyResolutionCalibration) {
    m_ApplyResolutionCalibration = ApplyResolutionCalibration;
  }
  //! Enable or disable shield veto effects
  bool GetEnableShieldVeto() const { return m_EnableShieldVeto; }
  void SetEnableShieldVeto(bool EnableShieldVeto) {
    m_EnableShieldVeto = EnableShieldVeto;
  }
  //! Enable or disable guard ring veto effects
  bool GetEnableGuardRingVeto() const { return m_EnableGuardRingVeto; }
  void SetEnableGuardRingVeto(bool EnableGuardRingVeto) {
    m_EnableGuardRingVeto = EnableGuardRingVeto;
  }

  //! Button to apply the FWHM timing resolution to the timing values
  bool GetApplyTimingResolutionCalibration() const { return m_ApplyTimingResolutionCalibration; }
  void SetApplyTimingResolutionCalibration(bool ApplyTimingResolutionCalibration) {
    m_ApplyTimingResolutionCalibration = ApplyTimingResolutionCalibration;
  }

  // protected methods:
 protected:
  // private methods:
 private:
  // protected members:
 protected:
  // private members:
 private:
  //! The sub module handling random coincidences
  MSubModuleRandomCoincidence m_RandomCoincidence;

  //! The sub module handling the intake of the sim data into the event
  MSubModuleDEEIntake m_Intake;

  //! The sub module handling the shield energy correction
  MSubModuleShieldEnergyCorrection m_ShieldEnergyCorrection;

  //! The sub module handling the shield readout
  MSubModuleShieldReadout m_ShieldReadout;

  //! The sub module handling the shield deadtime and veto
  MSubModuleShieldTrigger m_ShieldTrigger;

  //! The sub module handling charge transport to grid and voxelation into strips
  MSubModuleChargeTransport m_ChargeTransport;

  //! The sub module handling the strip readout: energy -> ADCs and thresholds
  MSubModuleStripReadout m_StripReadout;

  //! The sub module handling the strip readout noise on non-triggered strips
  MSubModuleStripReadoutNoise m_StripReadoutNoise;

  //! The sub module handling triggers and guard ring vetoes
  MSubModuleStripTrigger m_StripTrigger;

  //! The sub module handling depth and timing noise
  MSubModuleDepthReadout m_DepthReadout;

  //! The sub module handling the output of the DEE in to the standard nuclearizer classes
  MSubModuleDEEOutput m_Output;
  
  //! The file name of the simulated CTD and drift time splines
  MString m_DepthSplinesFileName;

  //! The file name of the depth-calibration coefficients
  MString m_DepthCoefficientsFileName;

  //! Option to add noise to the strip energies
  bool m_ApplyResolutionCalibration; 
  //! Option to enable shield veto effects
  bool m_EnableShieldVeto;
  //! Option to enable guard ring veto effects
  bool m_EnableGuardRingVeto;

  //! Option to add noise to the strip timing values
  bool m_ApplyTimingResolutionCalibration;

#ifdef ___CLING___
 public:
  ClassDef(MModuleDEESMEX, 0) // no description
#endif
};

#endif


////////////////////////////////////////////////////////////////////////////////
