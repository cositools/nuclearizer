/*
 * MHit.cxx
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
// MHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MHit.h"

// Standard libs:
#include <algorithm>
#include <iterator>

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MHit::MHit()
{
  // Construct an instance of MHit

  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MHit::~MHit()
{
  // Delete this instance of MHit

  // MHit does not own the strip hits, so they are not deleted
}


////////////////////////////////////////////////////////////////////////////////


void MHit::Clear()
{
  // Reset all data

  m_Position = g_VectorNotDefined;
  m_Energy = g_DoubleNotDefined;

  m_LVEnergy = g_DoubleNotDefined;
  m_HVEnergy = g_DoubleNotDefined;

  m_PositionResolution = g_VectorNotDefined;
  m_EnergyResolution = g_DoubleNotDefined;

  m_StripHits.clear();
  m_Origins.clear();

  m_CrossTalk = false;
  m_GuardRingHit = false;
  m_ChargeLoss = false;
  m_StripHitMultipleTimesLV = false;
  m_StripHitMultipleTimesHV = false;
  m_ChargeSharingLV = false;
  m_ChargeSharingHV = false;
  m_NoDepth = false;
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* MHit::GetStripHit(unsigned int i)
{
  // Return strip hit i

  if (i < m_StripHits.size()) {
    return m_StripHits[i];
  }

  if (g_Verbosity >= c_Error) cout<<"Error in MHit::GetStripHit: Strip hit index "<<i<<" is out of bounds: "<<m_StripHits.size()<<" strip hits available"<<endl;

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


void MHit::AddStripHit(MStripHit* StripHit)
{
  // Add a strip hit

  if (StripHit != nullptr) {
    m_StripHits.push_back(StripHit);
  } else {
    if (g_Verbosity >= c_Error) cout<<"Error in MHit::AddStripHit: Strip hit is nullptr"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MHit::RemoveStripHit(unsigned int i)
{
  // Remove a strip hit without deleting it

  if (i < m_StripHits.size()) {
    m_StripHits.erase(m_StripHits.begin() + i);
  } else {
    if (g_Verbosity >= c_Error) cout<<"Error in MHit::RemoveStripHit: Strip hit index "<<i<<" is out of bounds: "<<m_StripHits.size()<<" strip hits available"<<endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MHit::RemoveStripHit(MStripHit* StripHit)
{
  // Remove a strip hit without deleting it

  vector<MStripHit*>::iterator I = find(m_StripHits.begin(), m_StripHits.end(), StripHit);
  if (I != m_StripHits.end()) {
    m_StripHits.erase(I);
  } else {
    if (g_Verbosity >= c_Error) cout<<"Error in MHit::RemoveStripHit: Strip hit not found"<<endl;
  }
}

////////////////////////////////////////////////////////////////////////////////


bool MHit::StreamDat(ostream& S, int Version)
{
  // Stream the hit in a way Nuclearizer can read it in again

  if (Version == 1) {
    S<<"HT "<<m_Position.GetX()<<" "<<m_Position.GetY()<<" "<<m_Position.GetZ()<<" "<<m_Energy<<endl;
  } else if (Version == 2) {
    // Stream the hit information, then stream the strip hit information for this hit
    S<<"HT "<<m_Position.GetX()<<" "<<m_Position.GetY()<<" "<<m_Position.GetZ()<<" "<<m_Energy<<endl;
    for (auto SH : m_StripHits) {
      SH->StreamDat(S, 0);
    }
  } else if (Version == 3) {
    // Stream the hit information, including low-voltage and high-voltage energy, then stream the strip hit information
    S<<"HT "<<m_Position.GetX()<<" "<<m_Position.GetY()<<" "<<m_Position.GetZ()<<" "<<m_Energy<<" "<<m_LVEnergy<<" "<<m_HVEnergy<<endl;
    for (auto SH : m_StripHits) {
      SH->StreamDat(S, 0);
    }
  } else {
    if (g_Verbosity >= c_Error) cout<<"Error in MHit::StreamDat: Stream version "<<Version<<" not handled"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MHit::StreamEvta(ostream& S)
{
  // Stream the hit in MEGAlib's EVTA format

  // Assemble the origin information
  vector<int> Origins;

  // Only origins existing on both low-voltage and high-voltage strips count
  vector<int> LVOrigins;
  vector<int> HVOrigins;
  for (unsigned int s = 0; s < GetNStripHits(); ++s) {
    MStripHit* StripHit = m_StripHits[s];
    vector<int> NewOrigins = StripHit->GetOrigins();
    if (StripHit->IsLowVoltageStrip() == true) {
      for (int o: NewOrigins) {
        LVOrigins.push_back(o);
      }
    } else {
      for (int o: NewOrigins) {
        HVOrigins.push_back(o);
      }
    }
  }

  sort(LVOrigins.begin(), LVOrigins.end());
  LVOrigins.erase(unique(LVOrigins.begin(), LVOrigins.end()), LVOrigins.end());
  sort(HVOrigins.begin(), HVOrigins.end());
  HVOrigins.erase(unique(HVOrigins.begin(), HVOrigins.end()), HVOrigins.end());

  set_intersection(LVOrigins.begin(), LVOrigins.end(),
                   HVOrigins.begin(), HVOrigins.end(),
                   std::back_inserter(Origins));

  if ((LVOrigins.size() != 0 || HVOrigins.size() != 0) && Origins.size() == 0) {
    // If strip pairing mixed the hits completely, keep the mixed origin information
    for (unsigned int s = 0; s < GetNStripHits(); ++s) {
      MStripHit* StripHit = m_StripHits[s];
      vector<int> NewOrigins = StripHit->GetOrigins();
      for (int o: NewOrigins) {
        Origins.push_back(o);
      }
    }
    sort(Origins.begin(), Origins.end());
    Origins.erase(unique(Origins.begin(), Origins.end()), Origins.end());
  }

  S<<"HT 3;"<<m_Position.GetX()<<";"<<m_Position.GetY()<<";"<<m_Position.GetZ()<<";"<<m_Energy
    <<";"<<m_PositionResolution.GetX()<<";"<<m_PositionResolution.GetY()<<";"<<m_PositionResolution.GetZ()<<";"<<m_EnergyResolution;
  for (unsigned int i = 0; i < Origins.size(); ++i) {
    S<<";"<<Origins[i];
  }
  S<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MHit::Parse(MString& Line, int Version)
{
  // Parse a hit in Nuclearizer's DAT format

  Clear();

  if (Line.Length() < 3) {
    if (g_Verbosity >= c_Error) cout<<"Error in MHit::Parse: line too short with length "<<Line.Length()<<endl;
    return false;
  }
  if (Line.BeginsWith("HT") == false) {
    if (g_Verbosity >= c_Error) cout<<"Error in MHit::Parse: line does not start with 'HT'"<<endl;
    return false;
  }
  if (Version < 1 || Version > 3) {
    if (g_Verbosity >= c_Error) cout<<"Error in MHit::Parse: unsupported version "<<Version<<endl;
    return false;
  }

  if (Version == 3) {
    double X, Y, Z, Energy, LowVoltageEnergy, HighVoltageEnergy;
    int N = sscanf(Line.Data() + 3, "%lf %lf %lf %lf %lf %lf", &X, &Y, &Z, &Energy, &LowVoltageEnergy, &HighVoltageEnergy);
    if (N != 6) {
      if (g_Verbosity >= c_Error) cout<<"Error in MHit::Parse: malformed HT V3 line with "<<N<<" fields instead of 6"<<endl;
      return false;
    }
    m_Position.SetX(X);
    m_Position.SetY(Y);
    m_Position.SetZ(Z);
    m_Energy = Energy;
    m_LVEnergy = LowVoltageEnergy;
    m_HVEnergy = HighVoltageEnergy;
  } else {
    double X, Y, Z, Energy;
    int N = sscanf(Line.Data() + 3, "%lf %lf %lf %lf", &X, &Y, &Z, &Energy);
    if (N != 4) {
      if (g_Verbosity >= c_Error) cout<<"Error in MHit::Parse: malformed HT V"<<Version<<" line with "<<N<<" fields instead of 4"<<endl;
      return false;
    }
    m_Position.SetX(X);
    m_Position.SetY(Y);
    m_Position.SetZ(Z);
    m_Energy = Energy;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MHit::AddOrigins(const vector<int>& Origins)
{
  // Set the origins from the simulations (take care of duplicates)

  m_Origins.insert(m_Origins.end(), Origins.begin(), Origins.end());
  sort(m_Origins.begin(), m_Origins.end());
  m_Origins.erase(unique(m_Origins.begin(), m_Origins.end()), m_Origins.end());
}


// MHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
