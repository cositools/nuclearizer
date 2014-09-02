/*
 * MNCTModuleSimulationLoader.h
 *
 * Copyright (C) by Jau-Shian Liang, Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleSimulationLoader__
#define __MNCTModuleSimulationLoader__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cmath>

// ROOT libs:
#include "TRandom.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MDGeometryQuest.h"
#include "MFileEventsSim.h"
#include "MVector.h"
#include "MNCTArray.h"
#include "MNCTMath.h"
#include "MNCTDetectorArray.h"
#include "MNCTDetectorResponse.h"
#include "MNCTInverseCrosstalkCorrection.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleSimulationLoader : public MModule, public MFileEventsSim
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleSimulationLoader();
  //! Default destructor
  virtual ~MNCTModuleSimulationLoader();

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Apply the detector effects
  virtual bool ApplyDetectorEffects(MReadOutAssembly* Event);

  //! 
  virtual MString Report();

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //
  bool ReadXmlConfiguration(MXmlNode* Node);
  MXmlNode* CreateXmlConfiguration();

  //! Option switch flags
  void SetLoadDeadStrip(bool yn){m_LoadDeadStrip = yn;}
  void SetLoadCoincidence(bool yn){m_LoadCoincidence = yn;}
  void SetLoadAntiCoincidence(bool yn){m_LoadAntiCoincidence = yn;}
  void SetRunEnergySharing(bool yn){m_RunEnergySharing = yn;}
  void SetRunCrosstalk(bool yn){m_RunCrosstalk = yn;}
  void SetNonlinearGain(bool yn){m_NonlinearGain = yn;}
  void SetKeepLLDOnly(bool yn){m_KeepLLDOnly = yn;}
  void SetVerbose(bool yn){m_Verbose = yn;}
  bool GetLoadDeadStrip(){return m_LoadDeadStrip;}
  bool GetLoadCoincidence(){return m_LoadCoincidence;}
  bool GetLoadAntiCoincidence(){return m_LoadAntiCoincidence;}
  bool GetRunEnergySharing(){return m_RunEnergySharing;}
  bool GetRunCrosstalk(){return m_RunCrosstalk;}
  bool GetNonlinearGain(){return m_NonlinearGain;}
  bool GetKeepLLDOnly(){return m_KeepLLDOnly;}
  bool GetVerbose(){return m_Verbose;}

  //! Use for change time information
  void SetTimeOffset0(int TimeOffset0){m_TimeOffset0 = TimeOffset0;}
  int GetTimeOffset0(){return m_TimeOffset0;}
  void SetTimeOffset(double TimeOffset){m_TimeOffset = TimeOffset;}
  double GetTimeOffset(){return m_TimeOffset;}
  
  //! Set the detector array
  //void SetDetectorArray(MNCTDetectorArray* NCTDetectors){m_NCTDetectors = NCTDetectors ;}

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:
  MNCTDetectorArray m_NCTDetectors;
  MNCTDetectorResponse m_NCTResponse;
  MNCTInverseCrosstalkCorrection m_InverseCrosstalk;

  bool m_LoadDeadStrip;
  bool m_LoadCoincidence;
  bool m_LoadAntiCoincidence;
  bool m_RunEnergySharing;
  bool m_RunCrosstalk;
  bool m_NonlinearGain;
  bool m_KeepLLDOnly;
  bool m_Verbose;

  int m_TimeOffset0;
  double m_TimeOffset;

  // private members:
 private:
  //!
  int m_NEvent;
  int m_NAnalyzed;
  int m_NTriggered;
  int m_NSingleDet;
  int m_NMultipleDet;
  int m_NGuardring;

#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleSimulationLoader, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
