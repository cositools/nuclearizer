/*
 * MSubModuleDepthReadout.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleDepthReadout__
#define __MSubModuleDepthReadout__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleDepthReadout : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleDepthReadout();

  //! No copy constructor
  MSubModuleDepthReadout(const MSubModuleDepthReadout&) = delete;
  //! No copy assignment
  MSubModuleDepthReadout& operator=(const MSubModuleDepthReadout&) = delete;
  //! No move constructors
  MSubModuleDepthReadout(MSubModuleDepthReadout&&) = delete;
  //! No move operators
  MSubModuleDepthReadout& operator=(MSubModuleDepthReadout&&) = delete;

  //! Default destructor
  virtual ~MSubModuleDepthReadout();

  //! Initialize the module
  virtual bool Initialize();

  //! Clear event data from the module
  virtual void Clear();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Set filename for coefficients file
  void SetDepthCoefficientsFileName( const MString& FileName) { m_DepthCoefficientsFileName = FileName; }
  //! Get filename for coefficients file
  MString GetDepthCoefficientsFileName() const { return m_DepthCoefficientsFileName; }

  //! Set filename for TAC calibration
  void SetTACCalFileName( const MString& FileName) { m_TACCalFileName = FileName; }
  //! Get filename for TAC calibration
  MString GetTACCalFileName() const {return m_TACCalFileName; }

  //! Set if timing values should be smeared based on FWHM
  void SetApplyTimingResolutionCalibration(bool ApplyTimingResolutionCalibration) { m_ApplyTimingResolutionCalibration = ApplyTimingResolutionCalibration; }
  //! Get if timing values should be smeared based on FWHM
  bool GetApplyTimingResolutionCalibration() { return m_ApplyTimingResolutionCalibration; }

  //! Finalize the module
  virtual void Finalize();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration(MXmlNode* Node);

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:

  //! Filename of the depth calibration coefficients (stretch, offset, timing noise, ...)
  MString m_DepthCoefficientsFileName;

  //! Map of the depth calibration coefficients
  unordered_map<int, vector<double>> m_Coeffs;
  //! Reference energy of the depth calibration coefficients
  double m_Coeffs_Energy;

  //! Filename of the TAC calibration file
  MString m_TACCalFileName;

  //! Map DetID -> Side (LV=0, HV=1) -> Strip ID -> TAC calibration parameters
  unordered_map<int, vector<unordered_map<int, vector<double>>>> m_TACCal;


  // private members:
 private:

  //! Flag to determine if timing resolution calibration should be applied
  bool m_ApplyTimingResolutionCalibration;




#ifdef ___CLING___
 public:
  ClassDef(MSubModuleDepthReadout, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
