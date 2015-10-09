/*
 * MNCTHit.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTHitInVoxel__
#define __MNCTHitInVoxel__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTHitInVoxel
{
  // public interface:
 public:
  //! Standard constructor
  MNCTHitInVoxel();
  MNCTHitInVoxel(int DetectorID, int XStripID, int YStripID, MVector Displace, double energy);
//  MNCTHitInVoxel(const MNCTHitInVoxel &);
  //! Default destructor
  virtual ~MNCTHitInVoxel();

  //! Reset all data
  void Clear();

  //!
  void SetDetectorID(const int DetectorID ){m_DetectorID = DetectorID;}
  int GetDetectorID() const { return m_DetectorID; }
  
  //! Set the StripX number of the hit
  void SetXStripID(const int XStripID) {m_XStripID = XStripID;}
  int GetXStripID() const { return m_XStripID; }
  
  //! Set the StripY number of the hit
  void SetYStripID(const int YStripID) {m_YStripID = YStripID;}
  int GetYStripID() const { return m_YStripID; }

  //! Set the displacement of the hit from center of voxel
  void SetDisplace(const MVector& Displace) { m_Displace = Displace; }
  //! Return the position of the hit
  MVector GetDisplace() const { return m_Displace; }

  
  //! Setup the displacement and strip numbers from position in detector
//  void SetByPosInDet(const MVector& PositionInDetector);
  //! Return the position of the hit
//  MVector GetPosition() const { return m_Position; }
  

  //! Set the energy
  void SetEnergy(double Energy) { m_Energy = Energy; }
  //! Return the energy
  double GetEnergy() const { return m_Energy; }


  // protected methods:
 protected:
  //MNCTHit() {};
  //MNCTHit(const MNCTHit& NCTHit) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  //! Detector number
  int m_DetectorID;
  //! StripX
  int m_XStripID;
  //! StripY
  int m_YStripID;
  
  //! Displacement of the hit from center of Voxel
  MVector m_Displace;
 
  //! Energy of the hit
  double m_Energy;


#ifdef ___CINT___
 public:
  ClassDef(MNCTHitInVoxel, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
