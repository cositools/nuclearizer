/*
 * MMNCTDetectorEffectsEngineCOSI.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MMNCTDetectorEffectsEngineCOSI__
#define __MMNCTDetectorEffectsEngineCOSI__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>
#include <vector>
#include <list>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElementDoubleStrip.h"
#include "MFileEventsSim.h"

// Nuclearizer libs:
#include "MNCTDepthCalibrator.h"
#include "MReadOutAssembly.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTDetectorEffectsEngineCOSI
{
public:
  //! Default constructor
  MNCTDetectorEffectsEngineCOSI();
  //! Default destructor
  virtual ~MNCTDetectorEffectsEngineCOSI();
  
  //! Set the simulation file name 
  void SetSimulationFileName(const MString& FileName) { m_SimulationFileName = FileName; }
  //! Get the simulation file name 
  MString GetSimulationFileName() const { return m_SimulationFileName; }
  
  //! Show the progress of simulation file reading
  void ShowProgressBar(bool Flag) { m_ShowProgressBar = Flag; }
  
  //! Set the roa file name 
  void SetRoaFileName(const MString& FileName) { m_RoaFileName = FileName; m_SaveToFile = true; }
  //! Get the simulation file name 
  MString GetRoaFileName() const { return m_RoaFileName; }
  
  //! Set geometry file name
  void SetGeometryFileName(const MString& FileName) { m_GeometryFileName = FileName; }
  //! Set geometry file name
  MString GetGeometryFileName() { return m_GeometryFileName; }
  
  //! Set geometry
  void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }
  
  //! Set energy calibration file name
  void SetEnergyCalibrationFileName(const MString& FileName) { m_EnergyCalibrationFileName = FileName; } 
  //! Set energy calibration file name
  MString GetEnergyCalibrationFileName() const { return m_EnergyCalibrationFileName; } 
  
  //! Set threshold file name
  void SetThresholdFileName(const MString& FileName) { m_ThresholdFileName = FileName; } 
  //! Set threshold file name
  MString GetThresholdFileName() const { return m_ThresholdFileName; } 
  
  //! Set the dead strips file name
  void SetDeadStripFileName(const MString& FileName) { m_DeadStripFileName = FileName; } 
  //! Set the dead strips file name
  MString GetDeadStripFileName() const { return m_DeadStripFileName; } 
  
  //! Set the depth calibration coefficients file name
  void SetDepthCalibrationCoeffsFileName(const MString& FileName) { m_DepthCalibrationCoeffsFileName = FileName; } 
  //! Set the depth calibration coefficients file name
  MString GetDepthCalibrationCoeffsFileName() const { return m_DepthCalibrationCoeffsFileName; } 
  
  //!  Set the depth calibration splines file name
  void SetDepthCalibrationSplinesFileName(const MString& FileName) { m_DepthCalibrationSplinesFileName = FileName; }
  //!  Set the depth calibration splines file name
  MString GetDepthCalibrationSplinesFileName() const { return m_DepthCalibrationSplinesFileName; }
  
  //! Initialize the module
  bool Initialize();
  //! Analyze whatever needs to be analyzed...
  bool GetNextEvent(MReadOutAssembly* Event);
  //! Finalize the module
  bool Finalize();
  
  
protected:
  //! Read in and parse energy calibration file
  void ParseEnergyCalibrationFile();
  //! Read in and parse thresholds file
  void ParseThresholdFile();
  //! Read in and parse dead strip file
  void ParseDeadStripFile();
  //! noise shield energy
  double NoiseShieldEnergy(double energy, MString ShieldName);
  //! Read and initialize charge loss coefficients
  void InitializeChargeLoss();

  

public:
  //! Tiny helper class for MNCTDetectorEffectsEngineCOSI describing a special strip hit
  class MNCTDEEStripHit
  {
  public:
    //! The read-out element
    MReadOutElementDoubleStrip m_ROE;
    //! The ADC value
    double m_ADC;
    //! The timing value;
    double m_Timing;
    
    //! The simulated position
    MVector m_Position;
    //! The simulated energy deposit
    double m_Energy;
    
    //! True if this is a guard ring
    bool m_IsGuardRing;
    
    vector<MNCTDEEStripHit> m_OppositeStrips;
    
    //! ID of the event
    long m_ID;
    
    //! list of origins of strip hits from cosima output
    list<int> m_Origins;
    
    //! A list of original strip hits making up this strip hit
    vector<MNCTDEEStripHit> m_SubStripHits;
    
    //! lists indices of other substriphits that have same IA origin
    vector<int> m_SharedOrigin;
    
    //! for charge loss
    int m_OppositeStrip;
  };
  
protected:
  //! Convert Energy to ADC value
  int EnergyToADC(MNCTDEEStripHit& Hit, double energy);
  
  
protected:  
  
  //! Simulation file name
  MString m_SimulationFileName;
  //! The file reader
  MFileEventsSim* m_Reader;
  
  //! True if we should save data to file
  bool m_SaveToFile;
  //! Roa file name
  MString m_RoaFileName;
  //! Geometry file name
  MString m_GeometryFileName;
  //! Energy calibration file name
  MString m_EnergyCalibrationFileName;
  //! Dead strip file name
  MString m_DeadStripFileName;
  //! Thresholds file name
  MString m_ThresholdFileName;
  //! Depth calibration coefficients file name
  MString m_DepthCalibrationCoeffsFileName;
  //! Depth calibration splines file name
  MString m_DepthCalibrationSplinesFileName;
  
  
  //! The far field start area
  double m_StartAreaFarField;
  
  
private:
  
  //! The geometry
  MDGeometryQuest* m_Geometry;
  //! True if this class owns the geometry
  bool m_OwnGeometry;
  //! Show the reading of the progress bar
  bool m_ShowProgressBar;
  
  //! The roa output file
  ofstream m_Roa;
  
  //! Calibration map between read-out element and LLD thresholds
  map<MReadOutElementDoubleStrip, double> m_LLDThresholds;
  //! Calibration map between read-out element and fast thresholds
  map<MReadOutElementDoubleStrip, double> m_FSTThresholds;
  //! Calibration map between read-out element and fast threshold noise
  map<MReadOutElementDoubleStrip, double> m_FSTNoise;
  
  //! Calibration map between read-out element and fitted function for energy calibration
  map<MReadOutElementDoubleStrip, TF1*> m_EnergyCalibration;
  //! Calibration map between read-out element and fitted function for energy resolution calibration
  map<MReadOutElementDoubleStrip, TF1*> m_ResolutionCalibration;
  
	//! Dead time buffer with 16 slots
	double m_DeadTimeBuffer[12][16];
  //! Stores dead time for each detector
  double m_CCDeadTime[12];
	//! Stores last hit time for any detector
	double m_LastHitTime;
  //! Stores last time detector was hit to check if detector still dead
  double m_LastHitTimeByDet[12];
  double m_TotalDeadTime[12];
  int m_TriggerRates[12];
  
	int m_MaxBufferFullIndex;
	int m_MaxBufferDetector;

  double m_ShieldDeadTime;
  
  //! List of dead strips
  int m_DeadStrips[12][2][37];
  
  //! Depth calibrator class
  MNCTDepthCalibrator* m_DepthCalibrator;
  
  //! Charge loss fit coefficients
  double m_ChargeLossCoefficients[12][2][2];
  
  unsigned long m_MultipleHitsCounter;
  unsigned long m_TotalHitsCounter;
  unsigned long m_ChargeLossCounter;
  
  double m_ShieldPulseDuration;
  double m_CCDelay;
  double m_ShieldTime;  
  bool m_IsShieldDead;
  
  long m_NumShieldCounts;
  
  
  #ifdef ___CLING___
public:
  ClassDef(MNCTDetectorEffectsEngineCOSI, 0) // no description
  #endif
};

#endif


////////////////////////////////////////////////////////////////////////////////
