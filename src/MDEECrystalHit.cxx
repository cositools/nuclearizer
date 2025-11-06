/*
 * MDEECrystalHit.cxx
 *
 *
 * Copyright (C) by YOUR NAME HERE.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MDEECrystalHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDEECrystalHit.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDEECrystalHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MDEECrystalHit::MDEECrystalHit()
    : m_SimulatedEventID(0),
      m_SimulatedPosition(0, 0, 0),
      m_SimulatedPositionInDetector(0, 0, 0),
      m_DetectorID(0),
      m_CrystalID(0),
      m_VoxelInDetector(0, 0, 0),
      m_SimulatedEnergy(0),
      m_SimulatedHitIndex(0),
      m_Energy(0),
      m_ADC(0),
      m_HasTriggered(false),
      m_HasVetoed(false),
      m_Temperature(0)
{
  // Construct an instance of MDEECrystalHit
}


////////////////////////////////////////////////////////////////////////////////


//! Create new real strip hit
MCrystalHit* MDEECrystalHit::Convert()
{
  MCrystalHit* CH = new MCrystalHit();

  CH->SetDetectorID(m_ROE.GetDetectorID());
  CH->HasTriggered(m_HasTriggered);
  CH->SetADCUnits(m_ADC);

  return CH;
}

// MDEECrystalHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
