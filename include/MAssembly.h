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
  static const int c_EventLoader              = 14;
  static const int c_EventLoaderSimulation    = 11;
  static const int c_EventLoaderMeasurement   = 12;
  static const int c_EventOrdering            = 15;
  static const int c_Coincidence              = 15;
  static const int c_DetectorEffectsEngine    = 1;
  static const int c_EventFilter              = 2;
  static const int c_EnergyCalibration        = 3;
  static const int c_ChargeSharingCorrection  = 4;
  static const int c_DepthCorrection          = 5;
  static const int c_StripPairing             = 6;
  static const int c_Aspect                   = 7;
  static const int c_CrosstalkCorrection      = 8;
  static const int c_EventReconstruction      = 9;
  static const int c_Else                     = 10;
  static const int c_NoRestriction            = 10;
  static const int c_EventSaver               = 13;
  static const int c_EventTransmitter         = 16;
  static const int c_PositionDetermiation     = 18;
  static const int c_Statistics               = 19;
  
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
