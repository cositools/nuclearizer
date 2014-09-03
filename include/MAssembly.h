/*
 * MAssembly.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MAssembly__
#define __MAssembly__


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
  void SetInterrupt(bool Flag = true) { m_Supervisor->SetInterrupt(Flag); }

  // Module types:
  static const uint64_t c_EventLoader              = (1 << 0);  // = 1
  static const uint64_t c_EventLoaderSimulation    = (1 << 1);  // = 2
  static const uint64_t c_EventLoaderMeasurement   = (1 << 2);  // = 4
  static const uint64_t c_EventOrdering            = (1 << 3);  // = 8
  static const uint64_t c_Coincidence              = (1 << 4);  // = 16
  static const uint64_t c_DetectorEffectsEngine    = (1 << 5);  // = 32
  static const uint64_t c_EventFilter              = (1 << 6);  // = 64
  static const uint64_t c_EnergyCalibration        = (1 << 7);  // = 128
  static const uint64_t c_ChargeSharingCorrection  = (1 << 8);  // = 256
  static const uint64_t c_DepthCorrection          = (1 << 9);  // = 512
  static const uint64_t c_StripPairing             = (1 << 10); // = 1024
  static const uint64_t c_Aspect                   = (1 << 11); // = 2048
  static const uint64_t c_CrosstalkCorrection      = (1 << 12); // = 4096
  static const uint64_t c_EventReconstruction      = (1 << 13); // = 8196
  static const uint64_t c_Else                     = (1 << 14);
  static const uint64_t c_NoRestriction            = (1 << 15);
  static const uint64_t c_EventSaver               = (1 << 16);
  static const uint64_t c_EventTransmitter         = (1 << 17);
  static const uint64_t c_PositionDetermiation     = (1 << 18);
  static const uint64_t c_Statistics               = (1 << 19);
  
  // IMPORTANT:
  // If you add one analysis level, make sure you also handle it in:
  // -> ALL module constructors!
  // -> MNCTData::GetHighestAnalysislevel()

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

  
#ifdef ___CINT___
 public:
  ClassDef(MAssembly, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
