/*
 * MNCTStripEnergyDepth.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTStripEnergyDepth__
#define __MNCTStripEnergyDepth__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MNCTStrip.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTStripEnergyDepth
{
  // public interface:
 public:
  //! Default constructor
  MNCTStripEnergyDepth();
  MNCTStripEnergyDepth(MNCTStrip strip, double energy, double depth);
  //! Default destructor
  virtual ~MNCTStripEnergyDepth();

  //! Reset all data
  void Clear();

  //! Set the Strip
  void SetStrip(MNCTStrip Strip){m_Strip = Strip;}

  //! Get the Strip
  MNCTStrip GetStrip() const {return m_Strip;}

  //! Set the Energy
  void SetEnergy(double Energy) { m_Energy = Energy; }
  //! Return the Energy
  double GetEnergy() const { return m_Energy; }

  //! Set the Depth
  void SetDepth(double Depth) { m_Depth = Depth; }
  //! Return the Depth
  double GetDepth() const { return m_Depth; }

  // protected methods:
 protected:
  //MNCTStripEnergyDepth() {};
  //MNCTStripEnergyDepth(const MNCTStripEnergyDepth& NCTStripEnergyDepth) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //!
  MNCTStrip m_Strip;
  //! ADCUnits of the top side
  double m_Energy;
  //! Timing of the top side
  double m_Depth;


#ifdef ___CINT___
 public:
  ClassDef(MNCTStripEnergyDepth, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
