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
#include "MGlobal.h"
#include "MVector.h"
#include "MNCTAspect.h"

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
  bool AddAspectFrame(vector<uint8_t> Frame); 

  //! Get the aspect for the given time, return 0 if we do not have enough data for the given time
  MNCTAspect* GetAspect(MTime Time);
  
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
