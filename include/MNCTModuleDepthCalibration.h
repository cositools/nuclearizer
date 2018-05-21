/*
 * MNCTModuleDepthCalibration.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleDepthCalibration__
#define __MNCTModuleDepthCalibration__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <unordered_map>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MNCTModuleEnergyCalibrationUniversal.h"
#include "MNCTDepthCalibrator.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleDepthCalibration : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleDepthCalibration();
  //! Default destructor
  virtual ~MNCTModuleDepthCalibration();
  
  //! Create a new object of this class 
  virtual MNCTModuleDepthCalibration* Clone() { return new MNCTModuleDepthCalibration(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Set filename for coefficients file
  void SetCoeffsFileName( const MString& FileName) {m_CoeffsFile = FileName;}
  //! Get filename for coefficients file
  MString GetCoeffsFileName() const {return m_CoeffsFile;}

  //! Set filename for CTD->Depth splines
  void SetSplinesFileName( const MString& FileName) {m_SplinesFile = FileName;}
  //! Get filename for CTD->Depth splines
  MString GetSplinesFileName() const {return m_SplinesFile;}

  //! Read the XML configuration
  bool ReadXmlConfiguration(MXmlNode* Node);

  //! Create the XML configuration
  MXmlNode* CreateXmlConfiguration();

  //! Set the global timing FWHM noise
  void SetTimingNoiseFWHM(const double Time) {m_TimingNoiseFWHM = Time;}
  //! Get the global timing FWHM noise
  double GetTimingNoiseFWHM() const {return m_TimingNoiseFWHM;}

  //! Finalize
  void Finalize();

  // protected methods:
 protected:
  //! Returns the strip with most energy from vector Strips, also gives back the energy fraction
  MNCTStripHit* GetDominantStrip(std::vector<MNCTStripHit*>& Strips, double& EnergyFraction);
  //! Calculates the XYZ position of the hit.  Returns 0 if all is OK.
  int CalculateLocalPosition(MNCTStripHit* XSH, MNCTStripHit* YSH, MVector& GlobalPosition, MVector& PositionResolution, bool BadDepth);
  //! Converts the FWHM timing noise to FWHM depth noise
  double GetZFWHM(double CTD_s, int DetID, double Noise);
  //! Reads in spline data to convert CTD->Depth
	bool GetDepthSplines(MString fname, std::unordered_map<int, TSpline3*>& SplineMap, bool invert);

	//! Adds a spline
	void AddSpline(vector<double>& xvec, vector<double>& yvec, unordered_map<int, TSpline3*>& SplineMap, int DetID, bool invert);

  // private methods:
 private:



  // protected members:
 protected:

  std::unordered_map<int, std::vector<double>*> m_Coeffs;
  std::unordered_map<int, TSpline3*> m_Splines;   
  MString m_CoeffsFile;
  MString m_SplinesFile;
  std::vector<MString> m_DetectorNames;
  std::vector<double> m_Thicknesses;
  uint64_t m_NoError;
  uint64_t m_Error1;
  uint64_t m_Error2;
  uint64_t m_Error3;
  uint64_t m_Error4;
  uint64_t m_ErrorSH;
  double m_TimingNoiseFWHM;
  vector<MDVolume*> m_DetectorVolumes;
  MNCTModuleEnergyCalibrationUniversal* m_EnergyCalibration;
  MNCTDepthCalibrator* m_DepthCalibrator;


  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MNCTModuleDepthCalibration, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
