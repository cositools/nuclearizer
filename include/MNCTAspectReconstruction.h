/*
 * MNCTAspectReconstruction.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTAspectReconstruction__
#define __MNCTAspectReconstruction__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <deque>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MNCTAspect.h"
#include "MNCTAspectPacket.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTAspectReconstruction
{
  // public interface:
 public:
  //! Standard constructor
  MNCTAspectReconstruction();
  //! Default destructor
  virtual ~MNCTAspectReconstruction();

  //! Reset all data
  void Clear();

  //! Add and reconstruction one or more aspect frames - return false on error
  bool AddAspectFrame(MNCTAspectPacket PacketA); 

  //! Get the aspect for the given time, return 0 if we do not have enough data for the given time
  MNCTAspect* GetAspect_ares(MTime Time);
  MNCTAspect* GetAspect(MTime Time);
  
  
////////////////////////////////////////////////////////////////////////////////
  
  //Ares' adjustments begin here.
  
  
  //! Get the aspect for the given time, return 0 if we do not have enough data for the given time
  MNCTAspect* GetAspectGPS(MTime Time);

  //! Get the aspect for the given time, return 0 if we do not have enough data for the given time
  MNCTAspect* GetAspectMagnetometer(MTime Time);

  //! Get the aspect for the given time, return 0 if we do not have enough data for the given time
  MNCTAspect* GetPreviousGPS(MTime Time);

  //! Get the aspect for the given time, return 0 if we do not have enough data for the given time
  MNCTAspect* GetPreviousMagnetometer(MTime Time);
  
  
  
  
  
//!The following are trig functions that work with degrees.  
  
double sine(double sine_input);

double arcsine(double arcsine_input);

double cosine(double cosine_input);

double arccosine(double arccosine_input);

double tangent(double tangent_input);

double arctangent(double arctangent_input);

double arctangent2(double y, double x);



//!The Spherical Vincenty Formula (used to compute exact great circle distance between two points on a sphere). 
double Vincenty(double old_glat, double new_glat, double old_glon, double new_glon); 
  
  
  
  
  
  
  //Ares' adjustments end here.
  
////////////////////////////////////////////////////////////////////////////////


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Internal list of reconstructed aspects
  deque<MNCTAspect*> m_Aspects;


#ifdef ___CINT___
 public:
  ClassDef(MNCTAspectReconstruction, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
