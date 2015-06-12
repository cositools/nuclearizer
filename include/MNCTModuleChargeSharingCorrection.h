/*
 * MNCTModuleChargeSharingCorrection.h
 *
 * Copyright (C) 2009-2009 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleChargeSharingCorrection__
#define __MNCTModuleChargeSharingCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleChargeSharingCorrection : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleChargeSharingCorrection();
  //! Default destructor
  virtual ~MNCTModuleChargeSharingCorrection();
  
  //! Create a new object of this class 
  virtual MNCTModuleChargeSharingCorrection* Clone() { return new MNCTModuleChargeSharingCorrection(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

	vector<vector<double> > ParseOneSource(string);
	void LoadCorrectionInfo();
	double Interpolate(double,int,int);
	double EstimateE0(double,double,int,int);
	void dummy_func();	//for debugging

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
	//holds the fit parameter information
	//B[source][detector][side]
	vector<vector<vector<double> > > B;

	//number of sources used for correction
	int nSources;


  // private members:
 private:
  //bool m_IsCalibrationLoaded[10];
  //bool m_IsCalibrationLoadedPixel[10][37][37];
  //unsigned long ShareHitNumber0,ShareHitNumber1;
  //unsigned long SingleHitNumber,OtherHitNumber;  
  //unsigned long ShareEventNumber0,ShareEventNumber1, SingleEventNumber,OtherEventNumber;

#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleChargeSharingCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
