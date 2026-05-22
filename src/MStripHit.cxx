/*
 * MStripHit.cxx
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
// MStripHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MStripHit.h"

// Standard libs:
#include <iomanip>
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MStripHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MStripHit::MStripHit()
{
  // Construct an instance of MStripHit

  m_ReadOutElement = new MReadOutElementDoubleStrip();
  
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MStripHit::~MStripHit()
{
  // Delete this instance of MStripHit
  
  delete m_ReadOutElement;
}


////////////////////////////////////////////////////////////////////////////////


void MStripHit::Clear()
{
  // Reset all data

  m_ReadOutElement->Clear();
  m_HasTriggered = false;
  m_UncorrectedADCUnits = 0;
  m_ADCUnits = 0;
  m_Energy = 0;
  m_EnergyResolution = 0;
  m_TAC = 0;
  m_TACResolution = 0;
  m_Timing = 0;
  m_TimingResolution = 0;
  m_PreampTemp = 0;

  m_IsGuardRing = false;
  m_IsNearestNeighbor = false;

  m_HasFastTiming = false;
  m_HasCalibratedTiming = false;

  m_Origins.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MStripHit::Parse(MString& Line, int Version)
{
  const char* line = Line.Data();
  if (Line.Length() < 3) {
    if (g_Verbosity >= c_Error) cout<<"Error in MStripHit::Parse: line too short"<<endl;
    return false;
  }

  if (line[0] == 'S' && line[1] == 'H') {
    unsigned int det_id, strip_id;
    int has_triggered;
    double timing, un_adc, adc;
    double energy, energy_res;
    char pos_strip;
    unsigned int flags;
    int N = sscanf(&line[3], "%u %c %u %d %lf %lf %lf %lf %lf %u",
                   &det_id, &pos_strip, &strip_id, &has_triggered,
                   &timing, &un_adc, &adc, &energy, &energy_res, &flags);
    if (N != 10) {
      if (g_Verbosity >= c_Error) cout<<"Error in MStripHit::Parse: malformed SH line"<<endl;
      return false;
    }
    if (pos_strip != 'l' && pos_strip != 'h') {
      if (g_Verbosity >= c_Error) cout<<"Error in MStripHit::Parse: unknown detector face: "<<pos_strip<<endl;
      return false;
    }
    SetDetectorID(det_id);
    IsLowVoltageStrip(pos_strip == 'l');
    SetStripID(strip_id);
    HasTriggered(has_triggered != 0);
    SetTiming(timing);
    SetUncorrectedADCUnits(un_adc);
    SetADCUnits(adc);
    SetEnergy(energy);
    SetEnergyResolution(energy_res);
    ParseFlags(flags);
    return true;
  } else {
    if (g_Verbosity >= c_Error) cout<<"Error in MStripHit::Parse: line does not start with SH"<<endl;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Set the origins from the simulations (take care of duplicates)
void MStripHit::AddOrigins(vector<int> Origins)
{
  m_Origins.insert(m_Origins.end(), Origins.begin(), Origins.end());
  sort(m_Origins.begin(), m_Origins.end());
  m_Origins.erase(unique(m_Origins.begin(), m_Origins.end()), m_Origins.end());
}

  
////////////////////////////////////////////////////////////////////////////////


bool MStripHit::StreamDat(ostream& S, int Version)
{
  //! Stream the content to an ASCII file 
  
  S<<"SH "
   <<m_ReadOutElement->GetDetectorID()<<" "
   <<((m_ReadOutElement->IsLowVoltageStrip() == true) ? "l" : "h")<<" "
   <<m_ReadOutElement->GetStripID()<<" "
   <<m_HasTriggered<<" "
   <<setprecision(9)<<m_Timing<<" "
   <<m_UncorrectedADCUnits<<" "
   <<m_ADCUnits<<" "
   <<m_Energy<<" "
   <<m_EnergyResolution<<" "
   <<MakeFlags()<<endl;
 
  return true;
}




////////////////////////////////////////////////////////////////////////////////


void MStripHit::StreamRoa(ostream& S, bool WithADC, bool WithTAC, bool WithEnergy, bool WithTiming, bool WithTemperature, bool WithFlags, bool WithOrigins)
{
  //! Stream the content in MEGAlib's evta format 

  S<<"UH " 
   <<m_ReadOutElement->GetDetectorID()<<" "
   <<m_ReadOutElement->GetStripID()<<" "
   <<((m_ReadOutElement->IsLowVoltageStrip() == true) ? "l" : "h")<<" ";
  if (WithADC == true) {
    S<<m_ADCUnits<<" ";
  }
  if (WithTAC == true) {
    S<<m_TAC<<" ";
  }
  if (WithTemperature == true) {
    S<<m_PreampTemp<<" ";
  }
  if (WithEnergy == true) {
    S<<m_Energy<<" ";
  }
  if (WithTiming == true) {
    S<<m_Timing<<" ";
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


unsigned int MStripHit::MakeFlags()
{
  //! Return flags to indicate the type of strip hit
  //! Currently, 3 bits:
  //!   v = Has fast timing
  //!    v = Is a nearest neighbor
  //!     v = Is a guard ring
  //! 0b111u

  unsigned int Flags = 0b000u;
  if (m_IsGuardRing == true) {
    Flags = Flags | 0b001u;
  }
  if (m_IsNearestNeighbor == true) {
    Flags = Flags | 0b010u;
  }
  if (m_HasFastTiming == true) {
    Flags = Flags | 0b100u;
  }

  return Flags;
}


////////////////////////////////////////////////////////////////////////////////


void MStripHit::ParseFlags(unsigned int Flags)
{
  //! Set internal booleans according to flag
  //! Currently, 3 bits:
  //!   v = Has fast timing
  //!    v = Is a nearest neighbor
  //!     v = Is a guard ring
  //! 0b111u

  // "Flags & 0b001u" extracts bit 0, "!= 0u" turns it into an explicit bool.
  IsGuardRing((Flags & 0b001u) != 0u);
  IsNearestNeighbor((Flags & 0b010u) != 0u);
  HasFastTiming((Flags & 0b100u) != 0u);
}


// MStripHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
