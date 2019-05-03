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
#include "TH2D.h"

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

	//! Set the charge sharing factors file name
	void SetChargeSharingFileName(const MString& FileName){ m_ChargeSharingFileName = FileName; }
	//! Get the charge sharing factors file name
	MString GetChargeSharingFileName() const { return m_ChargeSharingFileName; }
 
	//! Set the crosstalk coefficients file name
	void SetCrosstalkFileName(const MString& FileName) { m_CrosstalkFileName = FileName; }
	//! Get the crosstalk coefficients file name
	MString GetCrosstalkFileName() const { return m_CrosstalkFileName; }

	//! Set the charge loss coefficients file name
	void SetChargeLossFileName(const MString& FileName) { m_ChargeLossFileName = FileName; }
	//! Get the charge loss coefficients file name
	MString GetChargeLossFileName() const { return m_ChargeLossFileName; }

  //! Set the depth calibration coefficients file name
  void SetDepthCalibrationCoeffsFileName(const MString& FileName) { m_DepthCalibrationCoeffsFileName = FileName; } 
  //! Set the depth calibration coefficients file name
  MString GetDepthCalibrationCoeffsFileName() const { return m_DepthCalibrationCoeffsFileName; } 
  
  //!  Set the depth calibration splines file name
  void SetDepthCalibrationSplinesFileName(const MString& FileName) { m_DepthCalibrationSplinesFileName = FileName; }
  //!  Set the depth calibration splines file name
  MString GetDepthCalibrationSplinesFileName() const { return m_DepthCalibrationSplinesFileName; }
 
	//! Get include fudge factor
	bool GetApplyFudgeFactor() const { return m_ApplyFudgeFactor; }
	//! Set whether to include the fudge factor
	void SetApplyFudgeFactor(bool ApplyFudgeFactor){ m_ApplyFudgeFactor = ApplyFudgeFactor; }
 
  //! Initialize the module
  bool Initialize();
  //! Analyze whatever needs to be analyzed...
  bool GetNextEvent(MReadOutAssembly* Event);
  //! Finalize the module
  bool Finalize();

	//! empty function used to make breakpoints for debugger
	void dummy_func();
  
  
protected:
  //! Read in and parse energy calibration file
  bool ParseEnergyCalibrationFile();
  //! Read in and parse thresholds file
  bool ParseThresholdFile();
  //! Read in and parse dead strip file
  bool ParseDeadStripFile();
  //! noise shield energy
  double NoiseShieldEnergy(double energy, MString ShieldName);
	//! Read charge sharing factor file
	bool ParseChargeSharingFile();
  //! Read and initialize charge loss coefficients
  bool InitializeChargeLoss();
	//! Parse crosstalk coefficients file
	bool ParseCrosstalkFile();
	//! Apply charge loss correction
	vector<double> ApplyChargeLoss(double energy1, double energy2, int detID, int side, double depth1, double depth2);
 

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
		//! The simulated energy deposit -- not changed by crosstalk and charge loss
		double m_EnergyOrig;

		//! SimHT index that the strip hit came from to check if hit was completely absorbed
		unsigned int m_HitIndex;
    
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
		//! save depth information for charge loss, and maybe other things
		double m_Depth;
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
	//! Charge sharing file name
	MString m_ChargeSharingFileName;
	//! Crosstalk file name
	MString m_CrosstalkFileName;
	//! Charge loss file name
	MString m_ChargeLossFileName;
  //! Depth calibration coefficients file name
  MString m_DepthCalibrationCoeffsFileName;
  //! Depth calibration splines file name
  MString m_DepthCalibrationSplinesFileName;
	//! whether fudge factor is applied
	bool m_ApplyFudgeFactor;
  
  //! The far field start area
  double m_StartAreaFarField;
  
  
private:
 
	//COSI constants
	//! number of detectors
	static const int nDets = 12;
	//! number of sides
	static const int nSides = 2;
	//! number of strips
	static const int nStrips = 37;
	//! slots in DSP dead time buffer
	static const int nDTBuffSlots = 16;
 
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
	//! Calibration map between read-out element and FST threshold functions
	map<MReadOutElementDoubleStrip, TF1*> m_FSTThresholds;
  //! Calibration map between read-out element and fast thresholds
//  map<MReadOutElementDoubleStrip, double> m_FSTThresholds;
  //! Calibration map between read-out element and fast threshold noise
//  map<MReadOutElementDoubleStrip, double> m_FSTNoise;
  
  //! Calibration map between read-out element and fitted function for energy calibration
  map<MReadOutElementDoubleStrip, TF1*> m_EnergyCalibration;
  //! Calibration map between read-out element and fitted function for energy resolution calibration
  map<MReadOutElementDoubleStrip, TF1*> m_ResolutionCalibration;
  
	//! Dead time buffer with 16 slots
	vector<vector<double> > m_DeadTimeBuffer = vector<vector<double> >(nDets, vector<double> (nDTBuffSlots));
  //! Stores dead time for each detector
  vector<double> m_CCDeadTime = vector<double>(nDets);
	//! Stores last hit time for any detector
	double m_LastHitTime;
  //! Stores last time detector was hit to check if detector still dead
  vector<double> m_LastHitTimeByDet = vector<double>(nDets);
	//! Stores total dead time by detector
  vector<double> m_TotalDeadTime = vector<double>(nDets);
	//! Stores trigger rates (number of events) for each detector
  vector<int> m_TriggerRates = vector<int>(nDets);
	//! Stores time of first event; used to get number of events per second
	double m_FirstTime;
	//! Stores time of last event; used to get number of events per second
	double m_LastTime;
  
	int m_MaxBufferFullIndex;
	int m_MaxBufferDetector;

	//! dead time on the shields
  double m_ShieldDeadTime;
	//! whether or not the event is vetoed by the shields
	bool m_ShieldVeto;
  
  //! List of dead strips
  vector<vector<vector<int> > > m_DeadStrips = vector<vector<vector<int> > >(nDets, vector<vector<int> >(nSides, vector<int>(nStrips)));
  
  //! Depth calibrator class
  MNCTDepthCalibrator* m_DepthCalibrator;

	//! charge sharing factors
	vector<vector<TF1*> > m_ChargeSharingFactors = vector<vector<TF1*> >(nDets, vector<TF1*>(nSides));
 
  //! Charge loss fit coefficients
	double m_ChargeLossCoefficients[nDets][nSides][3][2];
	//for some reason when I turn this into a vector I get a seg fault, too lazy to debug now

	//! Crosstalk coefficients
	vector<vector<vector<vector<double> > > > m_CrosstalkCoefficients = vector<vector<vector<vector<double> > > >(12, vector<vector<vector<double> > > (2, vector<vector<double> > (2, vector<double> (2))));

  unsigned long m_MultipleHitsCounter;
  unsigned long m_TotalHitsCounter;
  unsigned long m_ChargeLossCounter;
  
  double m_ShieldPulseDuration;
  double m_CCDelay;
  double m_ShieldTime;
	double m_ShieldDelay;
	double m_ShieldVetoWindowSize;
  bool m_IsShieldDead;
  
  long m_NumShieldCounts;
  
	//! drift constant: used for charge sharing due to diffusion; one for each detector
	vector<double> m_DriftConstant;

	TH2D* m_ChargeLossHist;


  // Some housekeeping
  
  //! Counter for events with strips in overflow
  unsigned long m_NumberOfEventsWithADCOverflows;
  //! Counter for events with no strips in overflow
  unsigned long m_NumberOfEventsWithNoADCOverflows;
  
  //! Counter for the number of times the IA was not in the detector for the charge sharing determination
  unsigned long m_NumberOfFailedIASearches;
  //! Counter for the number of times the IA was in the detector for the charge sharing determination
  unsigned long m_NumberOfSuccessfulIASearches;

  #ifdef ___CLING___
public:
  ClassDef(MNCTDetectorEffectsEngineCOSI, 0) // no description
  #endif
};

#endif


////////////////////////////////////////////////////////////////////////////////
