/*
 * MNCTHit.cxx
 *
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Jau-Shian Liang.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTHitInVoxel.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTHitInVoxel)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTHitInVoxel::MNCTHitInVoxel()
{
  // Construct an instance of MNCTHit

  Clear();
}


////////////////////////////////////////////////////////////////////////////////

MNCTHitInVoxel::MNCTHitInVoxel(int DetectorID, int XStripID, int YStripID, MVector Displace, double energy):
m_DetectorID(DetectorID), m_XStripID(XStripID), m_YStripID(YStripID), m_Displace(Displace), m_Energy(energy)
{
  
}

////////////////////////////////////////////////////////////////////////////////


MNCTHitInVoxel::~MNCTHitInVoxel()
{
  // Delete this instance of MNCTHit
}

////////////////////////////////////////////////////////////////////////////////

/*
void MNCTHitInVoxel::SetByPosInDet(const MVector& PositionInDetector)
{
  m_XStripID = PositionInDetector.GetX() / pitch; //<--not yet
  m_YStripID = PositionInDetector.GetY() / pitch; //<--not yet
  m_Displace.SetX(PositionInDetector.GetX() - m_XStripID*pitch);//<--not yet
  m_Displace.SetY(PositionInDetector.GetY() - m_YStripID*pitch);//<--not yet
  m_Displace.SetZ(PositionInDetector.GetZ());
}
*/

////////////////////////////////////////////////////////////////////////////////


void MNCTHitInVoxel::Clear()
{
  // Reset all data
  m_DetectorID = -1;
  m_XStripID = g_IntNotDefined;
  m_YStripID = g_IntNotDefined;
  m_Displace = g_VectorNotDefined;
  m_Energy = 0;
}


// MNCTHitInVoxel.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
