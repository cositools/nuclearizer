/*
 * MCalibratorEnergyPointwiseLinear.cxx
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
// MCalibratorEnergyPointwiseLinear
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MCalibratorEnergyPointwiseLinear.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MCalibratorEnergyPointwiseLinear)
#endif


////////////////////////////////////////////////////////////////////////////////


MCalibratorEnergyPointwiseLinear::MCalibratorEnergyPointwiseLinear() : MCalibratorEnergy()
{
  // Construct an instance of MCalibratorEnergyPointwiseLinear
  
  m_HasMultipleEntries = true;
}


////////////////////////////////////////////////////////////////////////////////


MCalibratorEnergyPointwiseLinear::~MCalibratorEnergyPointwiseLinear()
{
  // Delete this instance of MCalibratorEnergyPointwiseLinear
}


////////////////////////////////////////////////////////////////////////////////


bool MCalibratorEnergyPointwiseLinear::Add(double ADCUnits, double Energy) 
{   
  // Add a new data point
  
  return m_ADCToEnergy.Add(ADCUnits, Energy); 
}


////////////////////////////////////////////////////////////////////////////////


double MCalibratorEnergyPointwiseLinear::GetEnergy(double ADCUnits)
{
  //! Return the energy associated with the given ADC units
  
  return m_ADCToEnergy.Evaluate(ADCUnits);
}
  

////////////////////////////////////////////////////////////////////////////////


MString MCalibratorEnergyPointwiseLinear::ToString() const
{
  MString Out("Calibrator - point-wise linear: "); 

  for (unsigned int e = 0; e < m_ADCToEnergy.GetNDataPoints(); ++e) {
    Out += m_ADCToEnergy.GetDataPointX(e);
    Out += " -> ";
    Out += m_ADCToEnergy.GetDataPointY(e);
    Out += " keV   "; 
  }
  
  return Out;
}


////////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, const MCalibratorEnergyPointwiseLinear& C)
{
  os<<C.ToString();
  return os;
}


// MCalibratorEnergyPointwiseLinear.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
