/*
 * MNCTGuardringHit.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTGuardringHit__
#define __MNCTGuardringHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTGuardringHit
{
  // public interface:
 public:
  //! Default constructor
  MNCTGuardringHit();
  //! Default destructor
  virtual ~MNCTGuardringHit();

  //! Reset all data
  void Clear();

  //! Set the Detector ID
  void SetDetectorID(int DetectorID) { m_DetectorID = DetectorID; }
  //! Return the Detector ID
  int GetDetectorID() const { return m_DetectorID; }

  //! Set the ADCUnits of the top side
  void SetADCUnits(double ADCUnits) { m_ADCUnits = ADCUnits; }
  //! Return the ADCUnits of the top side
  double GetADCUnits() const { return m_ADCUnits; }

  //! Set the position of the hit
  void SetPosition(const MVector& Position) { m_Position = Position; }
  //! Return the position of the hit
  MVector GetPosition() const { return m_Position; }


  // protected methods:
 protected:
  //MNCTGuardringHit() {};
  //MNCTGuardringHit(const MNCTGuardringHit& NCTGuardringHit) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Detector ID
  int m_DetectorID;
  //! ADCUnits of the top side
  double m_ADCUnits;
  //! Position of the guardring
  MVector m_Position;


#ifdef ___CLING___
 public:
  ClassDef(MNCTGuardringHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
