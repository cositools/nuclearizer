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
  void SetSimCCEFileName( const MString& FileName) { m_SimCCEFile = FileName; }

  //! Get filename for SimCCE file
  MString GetSimCCEFileName() const { return m_SimCCEFile; }

  //! Finalize the module
  virtual void Finalize();

  //! Read the XML configuration
  bool ReadXmlConfiguration(MXmlNode* Node);

  //! Create the XML configuration
  MXmlNode* CreateXmlConfiguration();
  
  // Getters to retrieve the calculated values after Finalize() runs
  double GetDirectFWHM_LV() const { return m_DirectFWHM_LV; }
  double GetDirectFWHM_HV() const { return m_DirectFWHM_HV; }


  // protected methods:
 protected:

  //! Load in the specified SimCCE file
  bool LoadSimCCEFile(MString FName);

  //! Get the Sim-based corrected energy given the CTD value, uncorrected energy, and the sorted Sim CCE values
  double GetSimBasedCorrectedEnergy(double ctd_val, double uncorrected_energy, const std::vector<double>& sim_cce_sorted_e, const std::vector<double>& sim_cce_sorted_h, double paramA, double paramB, double paramC);

  //! Interpolate a value given x, xp, and fp
  double Interpolate(double x, const std::vector<double>& xp, const std::vector<double>& fp);



  // private methods
  private:


  // protected members:
 protected:

  unordered_map<int, vector<double>> m_SimCCE;
  double m_SimCCE_Energy;
  MString m_SimCCEFile;
 
  // unordered_map<int, MDDetector*> m_Detectors;
  vector<unsigned int> m_DetectorIDs;
  MModuleEnergyCalibration* m_EnergyCalibration;
  MGUIExpoTrappingCorrection* m_ExpoTrappingCorrection;

  bool m_SimCCEFileIsLoaded;

  double m_ParamA_HV;
  double m_ParamA_LV;
  double m_ParamB;
  double m_ParamC;
  std::vector<double> m_Depths;
  std::vector<double> m_CCEs_HV_e;
  std::vector<double> m_CCEs_HV_h;
  std::vector<double> m_CCEs_LV_e;
  std::vector<double> m_CCEs_LV_h;



  // private members:
 private:

  MModuleDepthCalibration* m_DepthCalibration = nullptr;

  //! Updated GUI to display the energy histogram
  MGUIExpoPlotSpectrum* m_ExpoSpectrum;
  
  TF1* GeneratePhotopeakFunction();

  double CalculateDirectFWHM(TH1D* hist);
  
  double m_DirectFWHM_LV = 0.0;
  double m_DirectFWHM_HV = 0.0;


#ifdef ___CLING___
 public:
  ClassDef(MModuleTrappingCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
