/*
 * MModuleDepthCalibration.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleDepthCalibration__
#define __MModuleDepthCalibration__


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
#include "MModuleEnergyCalibration.h"
#include "MDStrip3D.h"
#include "MDShapeBRIK.h"
#include "MGUIExpoDepthCalibration.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleDepthCalibration : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleDepthCalibration();
  //! Default destructor
  virtual ~MModuleDepthCalibration();
  
  //! Create a new object of this class 
  virtual MModuleDepthCalibration* Clone() { return new MModuleDepthCalibration(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Create the expos
  virtual void CreateExpos();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Set filename for coefficients file
  void SetCoeffsFileName( const MString& FileName) { m_CoeffsFileName = FileName; }
  //! Get filename for coefficients file
  MString GetCoeffsFileName() const { return m_CoeffsFileName; }

  //! Set filename for CTD->Depth splines
  void SetSplinesFileName( const MString& FileName) { m_SplinesFile = FileName; }
  //! Get filename for CTD->Depth splines
  MString GetSplinesFileName() const {return m_SplinesFile;}

  //! Enable/Disable Mask Metrology Correction
  void SetMaskMetrologyCorrectionEnable(bool X) { m_MaskMetrologyEnabled = X; }
  //! Get enable/disable status of mask metrology correction
  bool GetMaskMetrologyCorrectionEnable() const { return m_MaskMetrologyEnabled; }

  //! Set filename for mask metrology
  void SetMaskMetrologyFileName( const MString& FileName) { m_MaskMetrologyFileName = FileName; }
  //! Get filename for CTD->Depth splines
  MString GetMaskMetrologyFileName() const { return m_MaskMetrologyFileName; }

  //TODO Remove UCSD code here and place within it's own branch
  //! Set whether the data came from the card cage at UCSD
  void SetUCSDOverride( bool Override ) { m_UCSDOverride = Override; }
  //! Get whether the data came from the card cage at UCSD
  bool GetUCSDOverride() const { return m_UCSDOverride; }

  //! Load the detector and strip dimensions from the geometry object
  bool LoadDetectorDimensions(MDGeometryQuest* Geometry);

  //! Load the pixel-based coefficients file
  bool LoadCoeffsFile(MString FName);
  //! Load the strip-based coefficients file
  bool LoadStripCoeffsFile(MString FName);

  //! Set the pixel-based depth calibration coefficients
  void SetCoeffs( unordered_map<int, vector<double>> Coeffs ) { m_Coeffs = Coeffs; }
  //! Get the pixel-based depth calibration coefficients
  unordered_map<int, vector<double>> GetCoeffs() { return m_Coeffs; }

  //! Set the strip-based depth calibration coefficients
  void SetStripCoeffs(unordered_map<int, vector<unordered_map<int, vector<double>>>> Coeffs) { m_StripCoeffs = Coeffs; }
  //! Get the strip-based depth calibration coefficients
  unordered_map<int, vector<unordered_map<int, vector<double>>>> GetStripCoeffs() { return m_StripCoeffs; }

  //! Set the mean depth calibration offsets for all detectors
  void SetMeanOffset(unordered_map<int, double> MeanOffset) { m_MeanOffset = MeanOffset; }
  //! Get the mean depth calibration offsets for all detectors
  unordered_map<int, double> GetMeanOffset() { return m_MeanOffset; }

  //! Set the mean depth calibration stretches for all detectors
  void SetMeanStretch(unordered_map<int, double> MeanStretch) { m_MeanStretch = MeanStretch; }
  //! Get the mean depth calibration stretches for all detectors
  unordered_map<int, double> GetMeanStretch() { return m_MeanStretch; }

  //! Set the energy at which the depth calibration coefficients were determined
  void SetCoeffsEnergy( double Coeffs_Energy ) { m_Coeffs_Energy = Coeffs_Energy; }
  //! Get the energy at which the depth calibration coefficients were determined
  double GetCoeffsEnergy() { return m_Coeffs_Energy; }

  //! Load the splines file
  bool LoadSplinesFile(MString FName);

  //! Get the CTD->Depth Spline Grid
  unordered_map<int, vector<double>> GetDepthGrid() { return m_DepthGrid; }

  //! Get the CTD->Depth Spline CTD map
  unordered_map<int, vector<vector<double>>> GetCTDMap() { return m_CTDMap; }

  //! Read the XML configuration
  bool ReadXmlConfiguration(MXmlNode* Node);

  //! Create the XML configuration
  MXmlNode* CreateXmlConfiguration();

  //! Finalize
  void Finalize();

  // protected methods:
 protected:
  //! Returns the strip with most energy from vector Strips, also gives back the energy fraction
  MStripHit* GetDominantStrip(std::vector<MStripHit*>& Strips, double& EnergyFraction);
  
  //! Retrieve the appropriate Depth values given the DetID
  vector<double> GetDepth(int DetID);
  
  //! Retrieve the appropriate CTD values given the DetID and Grade
  vector<double> GetCTD(int DetID, int Grade);

  //! Retrieve the appropriate depth-to-CTD spline given the DetID and Grade
  TSpline3* GetSpline(int DetID, int Grade);
  //! Normal distribution
  vector<double> norm_pdf(vector<double> x, double mu, double sigma);
  
  //! Adds a Depth-to-CTD relation
  bool AddDepthCTD(vector<double> Depth, vector<vector<double>> CTDArr, int DetID, unordered_map<int, vector<double>>& DepthGrid, unordered_map<int,vector<vector<double>>>& CTDMap, unordered_map<int,vector<TSpline3*>>& SplineMap, unsigned int NPoints);

  //! Determine the Grade (geometry of charge sharing) of the Hit
  int GetHitGrade(MHit* H);

  //! Return the coefficients for a pixel
  vector<double>* GetPixelCoeffs(int PixelCode);
  
  //! Load the metrology mask file
  bool LoadMaskMetrologyFile(MString FName);

  //! Get the x, y position of the intersection of two strips based on the Metrology Mask  
  vector<double> GetStripIntersection(MReadOutElementDoubleStrip LVStrip, MReadOutElementDoubleStrip HVStrip);

  //! Get the timing FWHM noise for the specified pixel and Energy
  double GetTimingNoiseFWHM(int PixelCode, double Energy);


  // private methods
  private:


  // protected members:
 protected:

  unordered_map<int, vector<double>> m_Coeffs;
  double m_Coeffs_Energy;
  MString m_CoeffsFileName;
  MString m_SplinesFile;
  unordered_map<int, double> m_Thicknesses;
  unordered_map<int, int> m_NXStrips;
  unordered_map<int, int> m_NYStrips;
  unordered_map<int, double> m_XPitches;
  unordered_map<int, double> m_YPitches;
  uint64_t m_NoError;
  uint64_t m_Error1;
  uint64_t m_Error2;
  uint64_t m_Error3;
  uint64_t m_Error4;
  uint64_t m_Error5;
  uint64_t m_Error6;
  uint64_t m_ErrorSH;
  uint64_t m_ErrorNullSH;
  uint64_t m_ErrorNoE;
  unordered_map<int, MDDetector*> m_Detectors;
  vector<unsigned int> m_DetectorIDs;
  MModuleEnergyCalibration* m_EnergyCalibration;
  MGUIExpoDepthCalibration* m_ExpoDepthCalibration;


  //! Map: detector ID (int) -> mean stretch over all pixels / strips
  unordered_map<int, double> m_MeanStretch;
  //! Map: detector ID (int) -> mean offset over all pixels / strips
  unordered_map<int, double> m_MeanOffset;
  //! Map: detector ID (int) -> Side (LV=0, HV=1) -> Strip ID -> Depth calibration coefficients (per strip)
  unordered_map<int, vector<unordered_map<int, vector<double>>>> m_StripCoeffs;

  // The CTD Map maps each detector (int) to a 2D array of CTD values.
  unordered_map<int, vector<vector<double>>> m_CTDMap;
  unordered_map<int, vector<double>> m_DepthGrid;
  unordered_map<int, vector<TSpline3*>> m_SplineMap;
  bool m_SplinesFileIsLoaded;
  bool m_CoeffsFileIsLoaded;

  //! The Mask Metrology file name
  MString m_MaskMetrologyFileName;

  //! The Mask Metrology values
  map<MReadOutElementDoubleStrip, vector<double>> m_MaskMetrology;

  //! Mask Metrology Correction
  bool m_MaskMetrologyEnabled;
  bool m_MaskMetrologyFileIsLoaded;

  // boolean for use with the card cage at UCSD since it tags all events as detector 11
  bool m_UCSDOverride;



  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MModuleDepthCalibration, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
