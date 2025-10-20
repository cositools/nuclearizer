/*
 * MShieldCrystalHit.cxx
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
// MShieldCrystalHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MShieldCrystalHit.h"

// Standard libs:
#include <iomanip>
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MShieldCrystalHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MShieldCrystalHit::MShieldCrystalHit()
{
  // Construct an instance of MShieldCrystalHit

  m_ReadOutElement = new MReadOutElement();
  
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MShieldCrystalHit::~MShieldCrystalHit()
{
  // Delete this instance of MShieldCrystalHit
  
  delete m_ReadOutElement;
}


////////////////////////////////////////////////////////////////////////////////


void MShieldCrystalHit::Clear()
{
  // Reset all data

  m_ReadOutElement->Clear();
  m_HasTriggered = false;
  m_HasVetoed = false;
  m_ADCUnits = 0;
  m_Energy = 0;
  m_EnergyResolution = 0;

  m_Origins.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MShieldCrystalHit::Parse(MString& Line, int Version)
{
  /* To write - probably CH - crystal hit
  const char* line = Line.Data();
  if (line[0] == 'S' && line[1] == 'H') {
    int det_id, strip_id, has_triggered, timing, un_adc, adc;
    float energy, energy_res;
    char pos_strip;
    unsigned int flags;
    sscanf(&line[3],"%d %c %d %d %d %d %d %f %f %u", &det_id, &pos_strip, &strip_id, &has_triggered,  &timing, &un_adc, &adc, &energy, &energy_res, &flags);
    SetDetectorID(det_id);
    pos_strip == 'l' ? IsLowVoltageStrip(true) : IsLowVoltageStrip(false);
    SetStripID(strip_id);
    has_triggered == 0 ? HasTriggered(false) : HasTriggered(true);
    SetTiming((double)timing);
    SetUncorrectedADCUnits((double)un_adc);
    SetADCUnits((double)adc);
    SetEnergy(energy);
    SetEnergyResolution(energy_res);
    ParseFlags(flags);
    return true;
  } else {
    return false;
  }
  */

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Set the origins from the simulations (take care of duplicates)
void MShieldCrystalHit::AddOrigins(vector<int> Origins)
{
  m_Origins.insert(m_Origins.end(), Origins.begin(), Origins.end());
  sort(m_Origins.begin(), m_Origins.end());
  m_Origins.erase(unique(m_Origins.begin(), m_Origins.end()), m_Origins.end());
}

  
////////////////////////////////////////////////////////////////////////////////


bool MShieldCrystalHit::StreamDat(ostream& S, int Version)
{
  //! Stream the content to an ASCII file 
  
  S<<"CH "
   <<m_ReadOutElement->GetDetectorID()<<" "
   <<m_HasTriggered<<" "
   <<m_HasVetoed<<" "
   <<m_ADCUnits<<" "
   <<m_Energy<<" "
   <<m_EnergyResolution<<" "
   <<MakeFlags()<<endl;
 
  return true;
}




////////////////////////////////////////////////////////////////////////////////


void MShieldCrystalHit::StreamRoa(ostream& S, bool WithADC, bool WithEnergy, bool WithFlags, bool WithOrigins)
{
  //! Stream the content in MEGAlib's evta format 

  S<<"UC "
   <<m_ReadOutElement->GetDetectorID()<<" ";
  if (WithADC == true) {
    S<<m_ADCUnits<<" ";
  }
  if (WithEnergy == true) {
    S<<m_Energy<<" ";
  }
  if (WithFlags == true) {
    S<<MakeFlags()<<" ";
  }
  if (WithOrigins == true) {
    if (m_Origins.size() == 0) {
      S<<"- ";
    } else {
      for (unsigned int i = 0; i < m_Origins.size(); ++i) {
        if (i != 0) S<<";";
        S<<m_Origins[i];
      }
    }
  }
  S<<endl;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MShieldCrystalHit::MakeFlags()
{
  //! Return flags to indicate the type of crstyal; hit

  // No flags at the moment

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MShieldCrystalHit::ParseFlags(unsigned int Flags)
{
  //! Set internal booleans according to flag
}


// MShieldCrystalHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
