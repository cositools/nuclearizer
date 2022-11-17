/*
 * MNCTCoincideceVolume.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTCoincidenceVolume__
#define __MNCTCoincidenceVolume__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDVolume.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTCoincidenceVolume
{
  // public interface:
 public:
  //! Default constructor
  MNCTCoincidenceVolume();
  MNCTCoincidenceVolume(MDVolume* Coin_Vol);
  MNCTCoincidenceVolume(MDVolume* Coin_Vol, double Threshold);
  
  //! Default destructor
  virtual ~MNCTCoincidenceVolume();

  //! Reset all data
  void Clear();

  //! Set the Volume
  void SetVolume(MDVolume* Vol) { m_Volume = Vol; }
  //! Return the Volume
  MDVolume* GetVolume() const { return m_Volume; }

  //! Set the Energy Threshold
  void SetThreshold(double Threshold) { m_Threshold = Threshold; }
  //! Return the Energy Threshold
  double GetThreshold() const { return m_Threshold; }

  //! Return Trigger or not
  bool IsTriggered(double Energy) { return (Energy > m_Threshold); }


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Volume
  MDVolume* m_Volume;
  //! Energy threshold
  double m_Threshold;


#ifdef ___CLING___
 public:
  ClassDef(MNCTMNCTCoincidenceVolume, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
