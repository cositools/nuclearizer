/*
 * MModuleChargeSharingCorrection.h
 *
 * Copyright (C) 2009-2009 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleChargeSharingCorrection__
#define __MModuleChargeSharingCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleChargeSharingCorrection : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleChargeSharingCorrection();
  //! Default destructor
  virtual ~MModuleChargeSharingCorrection();
  
  //! Create a new object of this class 
  virtual MModuleChargeSharingCorrection* Clone() { return new MModuleChargeSharingCorrection(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

	vector<vector<double> > ParseOneSource(string);
	void LoadCorrectionInfo();
	void LoadCorrectionInfoUpdated();
	double Interpolate(double,int,int);
	double EstimateE0(double,double,double,int,int);
	void dummy_func();	//for debugging

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
	//holds the fit parameter information
	//B[source][detector][side]
	vector<vector<vector<double> > > m_B;

	//holds B for fractional fits
	//B[detector][side]
	vector<vector<double> > m_Bfrac;

	//holds linear interpolation coefficients
	//m_linInterpCoeffs[detector][side][0 or 1]
	double m_linInterpCoeffs[12][2][2];

	//number of sources used for correction
	int m_nSources;


  // private members:
 private:
  //bool m_IsCalibrationLoaded[10];
  //bool m_IsCalibrationLoadedPixel[10][37][37];
  //unsigned long ShareHitNumber0,ShareHitNumber1;
  //unsigned long SingleHitNumber,OtherHitNumber;  
  //unsigned long ShareEventNumber0,ShareEventNumber1, SingleEventNumber,OtherEventNumber;

#ifdef ___CLING___
 public:
  ClassDef(MModuleChargeSharingCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
