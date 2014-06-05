/*
 * MCalibratorEnergy.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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
// MCalibratorEnergy
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MCalibratorEnergy.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibratorEnergy)
#endif


////////////////////////////////////////////////////////////////////////////////


MCalibratorEnergy::MCalibratorEnergy() : m_ReadOut(0), m_HasMultipleEntries(false)
{
  // Construct an instance of MCalibratorEnergy
}


////////////////////////////////////////////////////////////////////////////////


MCalibratorEnergy::~MCalibratorEnergy()
{
  // Delete this instance of MCalibratorEnergy
  
  delete m_ReadOut;
}


////////////////////////////////////////////////////////////////////////////////


MString MCalibratorEnergy::ToString() const
{
  return MString("Calibrator - base class"); 
}
  

////////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, const MCalibratorEnergy& C)
{
  os<<C.ToString();
  return os;
}


// MCalibratorEnergy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
