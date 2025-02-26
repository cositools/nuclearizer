/*
 * MModuleDepthCalibration2024.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleDepthCalibration2024__
#define __MModuleDepthCalibration2024__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <unordered_map>
#include <vector>
#include <numeric>
#include <math.h>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MDStrip3D.h"
#include "MDShapeBRIK.h"
#include "MGUIExpoDepthCalibration2024.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleDepthCalibration2024 : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleDepthCalibration2024();
  //! Default destructor
  virtual ~MModuleDepthCalibration2024();
  
  //! Create a new object of this class 
  virtual MModuleDepthCalibration2024* Clone() { return new MModuleDepthCalibration2024(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Create the expos
  virtual void CreateExpos();

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

  //! Set whether the data came from the card cage at UCSD
  void SetUCSDOverride( bool Override ) {m_UCSDOverride = Override;}
  //! Get whether the data came from the card cage at UCSD
  bool GetUCSDOverride() const {return m_UCSDOverride;}


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
  MStripHit* GetMinimumStrip(std::vector<MStripHit*>& Strips, double& EnergyFraction);
  //! Retrieve the appropriate Depth values given the DetID
	vector<double> GetDepth(int DetID);
  //! Retrieve the appropriate CTD values given the DetID and Grade
  vector<double> GetCTD(int DetID, int Grade);
  //! Normal distribution
  vector<double> norm_pdf(vector<double> x, double mu, double sigma);
	//! Adds a Depth-to-CTD relation
	bool AddDepthCTD(vector<double> depthvec, vector<vector<double>> ctdarr, int DetID, unordered_map<int, vector<double>>& DepthGrid, unordered_map<int,vector<vector<double>>>& CTDMap);
  //! Determine the Grade (geometry of charge sharing) of the Hit
  int GetHitGrade(MHit* H);
  //! Load in the specified coefficients file
  bool LoadCoeffsFile(MString FName);
  //! Return the coefficients for a pixel
  vector<double>* GetPixelCoeffs(int pixel_code);
  //! Load the splines file
  bool LoadSplinesFile(MString FName);
  //! Get the timing FWHM noise for the specified pixel and Energy
  double GetTimingNoiseFWHM(int pixel_code, double Energy);


  // private methods
  private:


  // protected members:
 protected:

  unordered_map<int, vector<double>> m_Coeffs;
  double m_Coeffs_Energy;
  MString m_CoeffsFile;
  MString m_SplinesFile;
  unordered_map<int, MString> m_DetectorNames;
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
  vector<MDDetector*> m_Detectors;
  vector<unsigned int> m_DetectorIDs;
  MModuleEnergyCalibrationUniversal* m_EnergyCalibration;
  MGUIExpoDepthCalibration2024* m_ExpoDepthCalibration;

  // The CTD Map maps each detector (int) to a 2D array of CTD values.
  unordered_map<int, vector<vector<double>>> m_CTDMap;
  unordered_map<int, vector<double>> m_DepthGrid;
  bool m_SplinesFileIsLoaded;
  bool m_CoeffsFileIsLoaded;

  // boolean for use with the card cage at UCSD since it tags all events as detector 11
  bool m_UCSDOverride;



  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MModuleDepthCalibration2024, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
