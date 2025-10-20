/*
 * MDEEStripHit.h
 *
 * Copyright (C) by YOUR NAME HERE.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDEEStripHit__
#define __MDEEStripHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <list>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MStripHit.h"
#include "MReadOutElementDoubleStrip.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A shared data structure of all DEE classes holding the strip data on it's way
//! to the final version
struct MDEEStripHit
{
  // public interface:
 public:
  //! Default constructor
  MDEEStripHit();
  //! Default destructor
  virtual ~MDEEStripHit() {};

  //! Create new real strip hit
  MStripHit* Convert();

  // Simulation data

  //! The simulated event ID
  unsigned long m_SimulatedEventID; // original: long m_ID;
  //! The position from simulations (global coordinates)
  MVector m_SimulatedPosition; // original: Position
  //! The simulated position in the detector
  MVector m_SimulatedPositionInDetector;
  //! The simulated depth relative to the top of the detector in [0..1]
  double m_SimulatedRelativeDepth;
  //! The energy from simulations
  double m_SimulatedEnergy; // original: m_EnergyOrig
  //! The list of origin IDs form the simulation
  list<int> m_SimulatedOrigins; // original: m_Origins
  //! True if this is a guard ring
  bool m_SimulatedIsGuardRing; // original: m_IsGuardRing
  //! SimHT index that the strip hit came from to check if hit was completely absorbed
  unsigned int m_SimulatedHitIndex; // original: m_HitIndex;

  // The strip data - which can be in process of being created

  //! The read-out element
  MReadOutElementDoubleStrip m_ROE;

  //! A unique lookup ID of the strup hit
  unsigned int m_ID;
  //! The ID of the strip on the opposite side
  unsigned int m_OppositeSideID;
  //! Is this a guard ring
  bool m_IsGuardRing;
  //! The measured energy
  double m_Energy;
  //! The measured ADC value
  unsigned int m_ADC;
  //! Is this a guard ring
  bool m_HasTriggered;
  //! The measured TAC value;
  unsigned int m_TAC;
  //! The measured temperature value
  double m_Temperature;

  //! A list of original strip hits making up this strip hit
  vector<MDEEStripHit> m_SubStripHits;

  //! lists indices of other substriphits that have same IA origin
  vector<int> m_SharedOrigin; // <--- Check if needed

  //! ??
  // vector<MDEEStripHit> m_OppositeStripID;  // <--- Check if needed

  //! The
  // int m_OppositeStrip;  // <--- Check if needed



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
  ClassDef(MDEEStripHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
