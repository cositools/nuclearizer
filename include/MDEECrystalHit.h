/*
 * MDEECrystalHit.h
 *
 * Copyright (C) by YOUR NAME HERE.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDEECrystalHit__
#define __MDEECrystalHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <list>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MCrystalHit.h"
// #include "MReadOutElement.h"
#include "MReadOutElementVoxel3D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A shared data structure of all DEE classes holding the strip data on it's way
//! to the final version
struct MDEECrystalHit {
  // public interface:
 public:
  //! Default constructor
  MDEECrystalHit();
  //! Default destructor
  virtual ~MDEECrystalHit() {};

  //! Create new real strip hit
  MCrystalHit* Convert();

  // Simulation data

  //! The simulated event ID
  unsigned long m_SimulatedEventID; // original: long m_ID;
  //! The position from simulations (global coordinates)
  MVector m_SimulatedPosition; // original: Position
  //! The simulated position in the detector
  MVector m_SimulatedPositionInDetector;
  //! Detector ID  (X0, X1, Y0, Y1, Z0, Z1)
  MString m_DetectorID;
  //! Crystal ID
  unsigned int m_CrystalID;
  //! The simulated voxel (X, Y, Z) IDs
  MVector m_VoxelInDetector;
  //! The energy from simulations
  double m_SimulatedEnergy; // original: m_EnergyOrig
  //! SimHT index that the strip hit came from to check if hit was completely absorbed
  unsigned int m_SimulatedHitIndex; // original: m_HitIndex;
  //! The list of origin IDs form the simulation
  list<int> m_SimulatedOrigins; // original: m_Origins

  // The shield data - which can be in process of being created

  //! The read-out element
  MReadOutElementVoxel3D m_ROE;

  //! A unique lookup ID of the crystal hit
  unsigned int m_ID;
  //! The measured energy
  double m_Energy;
  //! The measured ADC value
  unsigned int m_ADC;
  //! Is this a guard ring
  bool m_HasTriggered;
  //! Is this a guard ring
  bool m_HasVetoed;
  //! The measured temperature value
  double m_Temperature;

  //! A list of original strip hits making up this strip hit
  vector<MDEECrystalHit> m_SubCrystalHits;

  //! lists indices of other substriphits that have same IA origin
  vector<int> m_SharedOrigin; // <--- Check if needed

  //! ??
  // vector<MDEECrystalHit> m_OppositeStripID;  // <--- Check if needed


  // protected methods:
 protected:
  // private methods:
 private:
  // protected members:
 protected:
  // private members:
 private:
#ifdef ___CLING___
 public:
  ClassDef(MDEECrystalHit, 0) // no description
#endif
};

#endif


////////////////////////////////////////////////////////////////////////////////
