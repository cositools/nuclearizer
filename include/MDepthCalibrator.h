/*
 * MDepthCalibrator.h
 *
 * Copyright (C) by Alex Lowell.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDepthCalibrator__
#define __MDepthCalibrator__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <unordered_map>
#include <vector>

// ROOT libs:
#include "TMultiGraph.h"
#include "TSpline.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MString.h"
#include "MFile.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDepthCalibrator
{
	public:
    //! Default constructor
    MDepthCalibrator();
    //! Default destructor
    virtual ~MDepthCalibrator() {};
		//! Load the coefficients file (i.e. fit parameters for each pixel)
		bool LoadCoeffsFile(MString FName);
		//! Load the TAC Calibration file
  		bool LoadTACCalFile(MString FName);
		//! Return the coefficients for a pixel
		std::vector<double>* GetPixelCoeffs(int pixel_code);
		//! Return the TAC calibration coefficients for a LV strip
		std::vector<double>* GetLVTACCal(int DetID, int StripID);
		//! Return the TAC calibration coefficients for a HV strip
		std::vector<double>* GetHVTACCal(int DetID, int StripID);
		//! Load the splines file
		bool LoadSplinesFile(MString FName);
		//! Return a pointer to a spline
		TSpline3* GetSpline(int Det, bool Depth2CTD);
		//! Return detector thickness
		double GetThickness(int DetID);
		//! Get spline relating depth to anode timing
		TSpline3* GetAnodeSpline(int Det);
		//! Get spline relating depth to cathode timing
		TSpline3* GetCathodeSpline(int Det);


	private:
		//! Generate the spline from the data and add it to the internal spline map
		void AddSpline(vector<double> xvec, vector<double> yvec, int DetID, std::unordered_map<int,TSpline3*>& SplineMap, bool invert);
		//! Adds a Depth-to-CTD relation
		bool AddDepthCTD(vector<double> depthvec, vector<vector<double>> ctdarr, int DetID, unordered_map<int, vector<double>>& DepthGrid, unordered_map<int,vector<vector<double>>>& CTDMap);



	private:
		unordered_map<int, vector<double>> m_Coeffs;
		double m_Coeffs_Energy;
		std::unordered_map<int,TSpline3*> m_SplineMap_Depth2CTD;
		std::unordered_map<int,TSpline3*> m_SplineMap_CTD2Depth;
		std::unordered_map<int,TSpline3*> m_SplineMap_Depth2AnoTiming;
		std::unordered_map<int,TSpline3*> m_SplineMap_Depth2CatTiming;
		bool m_SplinesFileIsLoaded;
		bool m_CoeffsFileIsLoaded;
		bool m_TACCalFileIsLoaded;
		vector<MDDetector*> m_Detectors;
		unordered_map<int, unordered_map<int, vector<double>>> m_HVTACCal;
  		unordered_map<int, unordered_map<int, vector<double>>> m_LVTACCal;
		std::vector<double> m_Thicknesses;
		// The CTD Map maps each detector (int) to a 2D array of CTD values.
  		unordered_map<int, vector<vector<double>>> m_CTDMap;
  		unordered_map<int, vector<double>> m_DepthGrid;



#ifdef ___CLING___
 public:
  ClassDef(MDepthCalibrator, 0) // no description
#endif

};

#endif