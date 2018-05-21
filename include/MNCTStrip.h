/*
 * MNCTStrip.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTStrip__
#define __MNCTStrip__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTStrip
{
  // public interface:
 public:
  //! Default constructor
  MNCTStrip();
  MNCTStrip(int DetectorID, bool IsXStrip, int StripID);
  
  //! Default destructor
  virtual ~MNCTStrip();

  //! Reset all data
  void Clear();

  //! Set the Detector ID
  void SetDetectorID(int DetectorID) { m_DetectorID = DetectorID; }
  //! Return the Detector ID
  int GetDetectorID() const { return m_DetectorID; }

  //! Set the Strip ID
  void SetStripID(int StripID) { m_StripID = StripID; }
  //! Return the Strip ID
  int GetStripID() const { return m_StripID; }

  //! Set the strip type (x/y)
  void IsXStrip(bool IsXStrip) { m_IsXStrip = IsXStrip; }
  //! Return the strip type (x/y)
  bool IsXStrip() const { return m_IsXStrip; }

  //! Comparison
  bool operator==(const MNCTStrip& Strip);

  //! Assignment
  void operator=(const MNCTStrip& Strip);


  // protected methods:
 protected:
  //MNCTStrip() {};
  //MNCTStrip(const MNCTStrip& NCTStrip) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Detector ID
  int m_DetectorID;
  //! Strip type
  bool m_IsXStrip;
  //! Strip ID
  int m_StripID;


#ifdef ___CLING___
 public:
  ClassDef(MNCTStrip, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
