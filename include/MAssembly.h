/*
 * MAssembly.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNuclearizerAssembly__
#define __MNuclearizerAssembly__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include "MVector.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MSupervisor.h"


////////////////////////////////////////////////////////////////////////////////


class MAssembly
{
  // Public Interface:
 public:
  //! Default constructor
  MAssembly();
  //! Default destructor
  virtual ~MAssembly();

  //! Each interface must be able to parse a command line - 
  //! this function is called by main()
  bool ParseCommandLine(int argc, char** argv);
  
  //! Called when hit Control-C: Set the interrupt which will end the analysis in the supervisor
  void SetInterrupt(bool Flag = true) { m_Supervisor->SetHardInterrupt(Flag); }

  // Module types:
  static const uint64_t c_EventLoader              = (1 << 0);  // = 1
  static const uint64_t c_EventLoaderSimulation    = (1 << 1);  // = 2
  static const uint64_t c_EventLoaderMeasurement   = (1 << 2);  // = 4
  static const uint64_t c_EventOrdering            = (1 << 3);  // = 8
  static const uint64_t c_Coincidence              = (1 << 4);  // = 16
  static const uint64_t c_TACcut                   = (1 << 5);  // = 32
  static const uint64_t c_NearestNeighbor          = (1 << 6);  // = 32
  static const uint64_t c_DetectorEffectsEngine    = (1 << 7);
  static const uint64_t c_EventFilter              = (1 << 8);
  static const uint64_t c_EnergyCalibration        = (1 << 9);
  static const uint64_t c_ChargeSharingCorrection  = (1 << 10);
  static const uint64_t c_DepthCorrection          = (1 << 11);
  static const uint64_t c_StripPairing             = (1 << 12);
  static const uint64_t c_Aspect                   = (1 << 13);
  static const uint64_t c_CrosstalkCorrection      = (1 << 14);
  static const uint64_t c_EventReconstruction      = (1 << 15);
  static const uint64_t c_Else                     = (1 << 16);
  static const uint64_t c_NoRestriction            = (1 << 17);
  static const uint64_t c_EventSaver               = (1 << 18);
  static const uint64_t c_EventTransmitter         = (1 << 19);
  static const uint64_t c_PositionDetermiation     = (1 << 20);
  static const uint64_t c_Statistics               = (1 << 21);
  static const uint64_t c_FlagHits                 = (1 << 22);
  static const uint64_t c_Diagnostics              = (1 << 23);
  static const uint64_t c_ResponseGeneration       = (1 << 24);


  // IMPORTANT:
  // If you add one analysis level, make sure you also handle it in:
  // -> ALL module constructors!
  // -> MData::GetHighestAnalysislevel()

  // protected methods:
 protected:
  
  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! True if the GUI is used
  bool m_UseGui;
  
  //! The store for all user data of the GUI:
  MSupervisor* m_Supervisor;
  
  //! The interrupt flag - the analysis will stop when this flag is set
  bool m_Interrupt;

  
#ifdef ___CLING___
 public:
  ClassDef(MAssembly, 1) 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
