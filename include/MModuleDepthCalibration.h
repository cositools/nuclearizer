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
#include <tuple>

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

  //! Set filename for charge sharing coefficients file
  void SetChargeSharingConfigFileName( const MString& FileName) { m_ChargeSharingConfigFileName = FileName; }
  //! Get filename for coefficients file
  MString GetChargeSharingConfigFileName() const { return m_ChargeSharingConfigFileName; }

  //! Set filename for ctd-z calibration coefficients file
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

  //! Load in the specified charge sharing calibration coefficients file
  bool LoadChargeSharingConfigFile(MString FName);

  //! Load in the specified ctd-z coefficients file
  bool LoadCoeffsFile(MString FName);

  //! Set the vector of z values for each detector (which depths are in the config for the charge sharing correction for each detector)
  void SetChargeSharingDepths( unordered_map<int, vector<double>> ChargeSharingDepths) {m_ChargeSharingDepths = ChargeSharingDepths;}

  //! Set the charge sharing correction depth calibration coefficient, one vector per strip-pair (map-int)  per depth (the vector)
  void SetChargeSharingCoeffs( unordered_map<int, vector<vector<double>>> ChargeSharingCoeffs ) { m_ChargeSharingCoeffs = ChargeSharingCoeffs; }

  //! Set the coefficients of the polynomial describing the charge sharing correction depth calibration coefficient, one vector per detector (map-int)  per depth (the vector)
  void SetChargeSharingPolyCoeffsHV( unordered_map<int, vector<vector<double>>> ChargeSharingPolyCoeffsHV ) { m_ChargeSharingPolyCoeffsHV = ChargeSharingPolyCoeffsHV; }
  void SetChargeSharingPolyCoeffsLV( unordered_map<int, vector<vector<double>>> ChargeSharingPolyCoeffsLV ) { m_ChargeSharingPolyCoeffsLV = ChargeSharingPolyCoeffsLV; }

  //! Set the depth calibration coefficients
  void SetCoeffs( unordered_map<int, vector<double>> Coeffs ) { m_Coeffs = Coeffs; }

  //! Get the depths for the charge sharing correction calibration coefficients for a detector
  unordered_map<int, vector<double>> GetChargeSharingDepths() { return m_ChargeSharingDepths; }

  //! Get the charge sharing correction calibration coefficients
  unordered_map<int, vector<vector<double>>> GetChargeSharingCoeffs() { return m_ChargeSharingCoeffs; }

  //! Get the coefficients of the polynomial for the ccharge sharing correction calibration
  unordered_map<int, vector<vector<double>>> GetChargeSharingPolyCoeffsHV() { return m_ChargeSharingPolyCoeffsHV; }
  unordered_map<int, vector<vector<double>>> GetChargeSharingPolyCoeffsLV() { return m_ChargeSharingPolyCoeffsLV; }

  //! Get the depth calibration coefficients
  unordered_map<int, vector<double>> GetCoeffs() { return m_Coeffs; }

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

  //! Returns the z as a function of ctd, given some ctd and some detector
  std::tuple<double, double> CalculateZfromCTD(double CTDvalue, double noise, int DetID,int Grade, bool sean_weighting);

  //! Returns the strip with the specified strip ID
  MStripHit* GetStrip(std::vector<MStripHit*>& Strips, int StripID);

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

  //! Return the coefficients of the dTac polynomial for a detector at a depth
  vector<double>* GetChargeSharingPolyCoeffsHV(int DetID,double z);
  vector<double>* GetChargeSharingPolyCoeffsLV(int DetID,double z);
  
  //! Return the charge-sharing stretch/offset coefficients for a strip-pair at a depth
  vector<double>* GetChargeSharingCoeffs(int StripPairCode,double z);
  
  //! Return the ctd-z stretch/offset coefficients for a pixel
  vector<double>* GetPixelCoeffs(int PixelCode);
  
  //! Load the metrology mask file
  bool LoadMaskMetrologyFile(MString FName);

  //! Get the x, y position of the intersection of two strips based on the Metrology Mask  
  vector<double> GetStripIntersection(MReadOutElementDoubleStrip LVStrip, MReadOutElementDoubleStrip HVStrip);

  // TODO this should require strip energy, not hit energy... 
  //! Get the timing FWHM noise for the specified pixel and Energy
  double GetTimingNoiseFWHM(int PixelCode, double Energy);


  // private methods
  private:


  // protected members:
  protected:
 
  unordered_map<int, vector<double>> m_ChargeSharingDepths; // maps DetID to the depths for which the charge sharing polynomial correction and charge sharing coefficients per-strip-pair were calculated
  unordered_map<int, vector<vector<double>>> m_ChargeSharingCoeffs; // maps StripPairID to a vector of coefficients, for a vector of depths (needs interpolation)
  vector<double> m_InterpolatedCoeffs; // holder for interpolated charge sharing coeffs between different depths
  unordered_map<int, vector<vector<double>>> m_ChargeSharingPolyCoeffsLV; // maps DetID to the LV coefficients for dTAC vs CS map and charge sharing correction vs CS map, for a vector of depths
  unordered_map<int, vector<vector<double>>> m_ChargeSharingPolyCoeffsHV; // maps DetID to the HV coefficients for dTAC vs CS map and charge sharing correction vs CS map, for a given depth
  unordered_map<int, vector<double>> m_Coeffs; // maps pix id to a vector of coefficients... 
  double m_Coeffs_Energy;
  MString m_ChargeSharingConfigFileName;
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
  uint64_t m_ZombieBump;
  unordered_map<int, MDDetector*> m_Detectors;
  vector<unsigned int> m_DetectorIDs;
  MModuleEnergyCalibration* m_EnergyCalibration;
  MGUIExpoDepthCalibration* m_ExpoDepthCalibration;

  // The CTD Map maps each detector (int) to a 2D array of CTD values.
  unordered_map<int, vector<vector<double>>> m_CTDMap;
  unordered_map<int, vector<double>> m_DepthGrid;
  unordered_map<int, vector<TSpline3*>> m_SplineMap;
  bool m_SplinesFileIsLoaded;
  bool m_CoeffsFileIsLoaded;
  bool m_ChargeSharingConfigFileIsLoaded;

  //! The Mask Metrology file name
  MString m_MaskMetrologyFileName;

  //! The Mask Metrology values
  map<MReadOutElementDoubleStrip, vector<double>> m_MaskMetrology;

  //! Mask Metrology Correction
  bool m_MaskMetrologyEnabled;
  bool m_MaskMetrologyFileIsLoaded;

  // boolean for use with the card cage at UCSD since it tags all events as detector 11
  bool m_UCSDOverride;

  // variable to describe the fraction of charge sharing that we define as a single strip
  // TODO -- should this be a variable in the config? 
  double m_SingleStripChargeSharing = 0.9;

  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MModuleDepthCalibration, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
