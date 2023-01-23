/*
 * MGuardringHit.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGuardringHit__
#define __MGuardringHit__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGuardringHit
{
  // public interface:
 public:
  //! Default constructor
  MGuardringHit();
  //! Default destructor
  virtual ~MGuardringHit();

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
  //MGuardringHit() {};
  //MGuardringHit(const MGuardringHit& NCTGuardringHit) {};

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
  ClassDef(MGuardringHit, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
