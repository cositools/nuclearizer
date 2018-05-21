/*
 * MNCTDetectorResponse.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTDetectorResponse__
#define __MNCTDetectorResponse__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cmath>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDGeometryQuest.h"
#include "MVector.h"
#include "TRandom.h"
#include "MNCTArray.h"
#include "MNCTMath.h"
#include "MNCTStrip.h"
#include "MNCTStripEnergyDepth.h"
#include "MNCTHitInVoxel.h"
#include "MNCTDetectorArray.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTDetectorResponse
{
  // public interface:
 public:
  //! Default constructor
  MNCTDetectorResponse();
  //! Default destructor
  virtual ~MNCTDetectorResponse();
  //!
  bool Activate();

  //!
  void Clear();


  //! loading model parameters for each strip###
  void LoadResponseArray();
  void DeleteResponseArray();

  //! set common parameters
  void SetNCTDetectorArray(MNCTDetectorArray* NCTDetectors){m_NCTDetectors = NCTDetectors; m_IsNCTDetectorArraySet=true;}
  void SetParameters();

  //! guardring()###
  bool GuardringTriger(double energy);

  //! sharing ###
  vector<MNCTStripEnergyDepth> noEnergySharing(const MNCTHitInVoxel& HitInVoxel, bool IsXStrip);
  vector<MNCTStripEnergyDepth> EnergySharing(const MNCTHitInVoxel& HitInVoxel, bool IsXStrip);

  //! CTD()###
  double Depth2Time(MNCTStrip Strip, double depth);
  int Time2CTD(MNCTStrip Strip, double time);
  int Time2TriggerTime(MNCTStrip XStrip, MNCTStrip YStrip, bool Xstrip, double time);

  //! ADC()###
  double EnergyNoise(double energy, MNCTStrip Strip);
  int Energy2ADC(double energy, MNCTStrip Strip);
  double NonlinearEnergyNoise(double energy, MNCTStrip Strip);
  int NonlinearEnergy2ADC(double energy, MNCTStrip Strip);

  //!trigers###
  //! Return threshold
  double FastThreshold(MNCTStrip Strip);
  double SlowThreshold(MNCTStrip Strip);
  

  // protected methods:
 protected:
  
  // private methods:
  
 private:
  //used for nonlinear energy depth conversion
  double delta(double x, double a2, double a3, double a4);

  // protected members:
 protected:
  //! check point
  bool m_IsNCTDetectorArraySet;
  bool m_IsParametersSet;
  bool m_IsResponseArrayLoaded;

  //! NCT detector array
  MNCTDetectorArray* m_NCTDetectors;

  //! Parameters
  unsigned int m_DetN;
  unsigned int m_StripN;

  
  //! Random number ganerator
  TRandom* Jiggle;
  TRandom* NoiseError;

  //! response array and threshold settings
  double_array4* m_DCalArr;
  double_array3* m_Depth2TArr;
  double_array4* m_E2ADCArr;
  double_array4* m_nonlinearE2ADCArr;
  double_array4* m_FastArr;
  double_array4* m_SlowArr;
  double_array4* m_NoiseArr;
  double_array4* m_nonlinearNoiseArr;

  //! energy sharing parameter array
  vector<double> e0;
  vector<double> e1;
  
  //!misc
  int m_DCalArr_n;
  int m_Depth2TArr_n;
  int m_E2ADCArr_n;
  int m_NoiseArr_n;
  int m_nonlinearNoiseArr_n;


  // private members:
 private:

#ifdef ___CLING___
 public:
  ClassDef(MNCTDetectorResponse, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
