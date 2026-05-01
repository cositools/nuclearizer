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

  //! Set geometry
  void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }

  //! Set filename for coefficients file
  void SetDepthCoefficientsFileName( const MString& FileName) { m_DepthCoefficientsFile = FileName; }
  //! Get filename for coefficients file
  MString GetDepthCoefficientsFileName() const { return m_DepthCoefficientsFile; }

  //! Set filename for CTD->Depth splines
  void SetDepthSplinesFileName( const MString& FileName) { m_DepthSplinesFile = FileName; }
  //! Get filename for CTD->Depth splines
  MString GetDepthSplinesFileName() const {return m_DepthSplinesFile;}

  //! Set filename for TAC calibration
  void SetTACCalFileName( const MString& FileName) { m_TACCalFile = FileName; }
  //! Get filename for TAC calibration
  MString GetTACCalFileName() const {return m_TACCalFile;}

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

  //! Load in the specified coefficients file
  bool LoadCoeffsFile();

  //! Load the splines file
  bool LoadSplinesFile();

  //! Load the TACcal file
  bool LoadTACCalFile();


  // private methods:
 private:



  // protected members:
 protected:

  //! The geometry
  MDGeometryQuest* m_Geometry;

  //! The detector dimensions
  unordered_map<int, double> m_Thicknesses;

  //! Filename of the depth coefficients (stretch, offset, timing noise, ...)
  MString m_DepthCoefficientsFile;
  unordered_map<int, vector<double>> m_Coeffs;
  double m_Coeffs_Energy;

  //! Filename of CTD->Depth splines
  MString m_DepthSplinesFile;

  //! CTD-to-depth splines
  // TODO: allow for multiple CTD-to-depth splines per detector
  // unordered_map<int, TSpline3*> m_DepthSplineMap;
  unordered_map<int, vector<double>> m_DepthGrid;
  unordered_map<int, vector<double>> m_CTDMap;
  unordered_map<int, vector<double>> m_ElectronDriftTimes;
  unordered_map<int, vector<double>> m_HoleDriftTimes;

  //! Filename of the TAC calibration file
  MString m_TACCalFile;
  unordered_map<int, vector<double>> m_TACCal;


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
