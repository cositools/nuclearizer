/*
 * MModuleTrappingCorrection.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleTrappingCorrection__
#define __MModuleTrappingCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <unordered_map>
#include <vector>
#include <numeric>
#include <cmath>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MGUIEEntry.h"



// Nuclearizer libs:
#include "MGUIExpoPlotSpectrum.h"
#include "MModuleEnergyCalibration.h"
#include "MGUIExpoTrappingCorrection.h"
#include "MGUIOptionsTrappingCorrection.h"
#include "MModuleDepthCalibration.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleTrappingCorrection : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleTrappingCorrection();
  //! Default destructor
  virtual ~MModuleTrappingCorrection();
  
  //! Create a new object of this class 
  virtual MModuleTrappingCorrection* Clone() { return new MModuleTrappingCorrection(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Create the expos
  virtual void CreateExpos();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Set filename for SimCCE file
  void SetSimCCEFileName( const MString& FileName) { m_SimCCEFileName = FileName; }

  //! Get filename for SimCCE file
  MString GetSimCCEFileName() const { return m_SimCCEFileName; }

  //! Finalize the module
  virtual void Finalize();

  //! Read the XML configuration
  bool ReadXmlConfiguration(MXmlNode* Node);

  //! Create the XML configuration
  MXmlNode* CreateXmlConfiguration();
  
  // protected methods:
 protected:

  //! Load in the specified SimCCE file
  bool LoadSimCCEFile(MString FName);

  //! Get the Sim-based corrected energy given the depth value, uncorrected energy, and the sorted Sim CCE values
  double GetSimBasedCorrectedEnergy(double depth_val, double uncorrected_energy, const std::vector<double>& depths, const std::vector<double>& sim_cce_sorted_e, const std::vector<double>& sim_cce_sorted_h, double paramB, double paramC);

  //! Interpolate a value given x, xp, and fp
  double Interpolate(double x, const std::vector<double>& xp, const std::vector<double>& fp);



  // private methods
  private:


  // protected members:
 protected:

  double m_SimCCE_Energy;
  MString m_SimCCEFileName;
 
  // unordered_map<int, MDDetector*> m_Detectors;
  vector<unsigned int> m_DetectorIDs;
  MModuleEnergyCalibration* m_EnergyCalibration;
  MGUIExpoTrappingCorrection* m_ExpoTrappingCorrection;

  bool m_SimCCEFileIsLoaded;

  // private members:
 private:

  MModuleDepthCalibration* m_DepthCalibration = nullptr;

  //! Updated GUI to display the energy histogram
  MGUIExpoTrappingCorrection* m_ExpoSpectrum;
  
  TF1* GeneratePhotopeakFunction();

  TGCheckButton* m_LogYButton;

  //! struct definition for trapping parameters and CCE curves for each detector
  struct DetectorTrappingData {
    double m_ParamA_HV = 1.0;
    double m_ParamA_LV = 1.0;
    double m_ParamB    = 0.0;
    double m_ParamC    = 0.0;

    std::vector<double> m_Depths;
    std::vector<double> m_CCEs_HV_e;
    std::vector<double> m_CCEs_HV_h;
    std::vector<double> m_CCEs_LV_e;
    std::vector<double> m_CCEs_LV_h;
  };

  //! Helper method using the struct pointer
  const DetectorTrappingData* GetDetectorData(int DetID) const;

  //! Stores trapping parameters and CCE curves for all loaded detectors
  std::map<int, DetectorTrappingData> m_DetectorParamMap; 


#ifdef ___CLING___
 public:
  ClassDef(MModuleTrappingCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
