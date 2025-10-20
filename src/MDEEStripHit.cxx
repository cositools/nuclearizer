/*
 * MDEEStripHit.cxx
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
// MDEEStripHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDEEStripHit.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDEEStripHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MDEEStripHit::MDEEStripHit() : m_SimulatedEventID(0), m_SimulatedPosition(0,0,0), m_SimulatedPositionInDetector(0,0,0), m_SimulatedRelativeDepth(0), m_SimulatedEnergy(0), m_SimulatedIsGuardRing(false), m_SimulatedHitIndex(0), m_IsGuardRing(false), m_Energy(0), m_ADC(0), m_HasTriggered(false), m_TAC(0), m_Temperature(0)
{
  // Construct an instance of MDEEStripHit
}


////////////////////////////////////////////////////////////////////////////////


  //! Create new real strip hit
MStripHit* MDEEStripHit::Convert()
{
  MStripHit* SH = new MStripHit();

  SH->SetDetectorID(m_ROE.GetDetectorID());
  SH->SetStripID(m_ROE.GetStripID());
  SH->IsLowVoltageStrip(m_ROE.IsLowVoltageStrip());
  SH->HasTriggered(m_HasTriggered);
  SH->SetADCUnits(m_ADC);
  SH->SetTAC(m_TAC);
  //SH->AddOrigins();
  SH->IsGuardRing(m_IsGuardRing);

  return SH;
}

// MDEEStripHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
