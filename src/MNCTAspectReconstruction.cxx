/*
 * MNCTAspectReconstruction.cxx
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
// MNCTAspectReconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTAspectReconstruction.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTAspectReconstruction)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTAspectReconstruction::MNCTAspectReconstruction()
{
  // Construct an instance of MNCTAspectReconstruction

  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MNCTAspectReconstruction::~MNCTAspectReconstruction()
{
  // Delete this instance of MNCTAspectReconstruction
}


////////////////////////////////////////////////////////////////////////////////


void MNCTAspectReconstruction::Clear()
{
  // Reset all data

  for (auto A: m_Aspects) {
    delete A;
  }
  m_Aspects.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTAspectReconstruction::AddAspectFrame(vector<uint8_t> Frame)
{
  // Add and reconstruction one or more aspect frames - return false on error

  MNCTAspect* Aspect = new MNCTAspect;
  
  // Fill the Aspect object with data from Frame
  
  
  // Add it, but make sure m_Aspect stays sorted by time! 
  m_Aspects.push_back(Aspect);
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


MNCTAspect* MNCTAspectReconstruction::GetAspect(MTime Time)
{
  //! Get the aspect for the given time, return 0 if we do not have enough data for the given time

  return m_Aspects.front(); // <--- this is definitely wrong
}


// MNCTAspectReconstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
