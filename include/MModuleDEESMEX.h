/*
 * MModuleDEESMEX.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleDEESMEX__
#define __MModuleDEESMEX__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs
#include "MModule.h"
#include "MSubModuleDEEIntake.h"
#include "MSubModuleRandomCoincidence.h"
#include "MSubModuleShieldEnergyCorrection.h"
#include "MSubModuleShieldReadout.h"
#include "MSubModuleShieldTrigger.h"
#include "MSubModuleChargeTransport.h"
#include "MSubModuleStripReadout.h"
#include "MSubModuleStripReadoutNoise.h"
#include "MSubModuleStripTrigger.h"
#include "MSubModuleDepthReadout.h"
#include "MSubModuleDEEOutput.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleDEESMEX : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleDEESMEX();
  //! Default destructor
  virtual ~MModuleDEESMEX();
  
  //! Create a new object of this class 
  virtual MModuleDEESMEX* Clone() { return new MModuleDEESMEX(); }
 
  //! Set the geometry
  virtual void SetGeometry(MDGeometryQuest* Geometry) { MModule::SetGeometry(Geometry); }
  
  //! Set geometry file name
  void SetGeometryFileName(const MString& FileName) { cout<<"Use SetGeometry instead"<<endl; abort(); }
 
  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);
  
  //! Finalize the module
  virtual void Finalize();


  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
   //! Time when the dead time ends
   MTime m_DeadTimeEnd;

   //! The sub module handling random coincidences
   MSubModuleRandomCoincidence m_RandomCoincidence;

   //! The sub module handling the intake of the sim data into the event
   MSubModuleDEEIntake m_Intake;

   //! The sub module handling the shield energy correction
   MSubModuleShieldEnergyCorrection m_ShieldEnergyCorrection;

   //! The sub module handling the shield readout
   MSubModuleShieldReadout m_ShieldReadout;

   //! The sub module handling the shield veto
   MSubModuleShieldTrigger m_ShieldTrigger;

   //! The sub module handling charge transport to grid and voxelation into strips
   MSubModuleChargeTransport m_ChargeTransport;

   //! The sub module handling the strip readout: energy -> ADCs and thresholds
   MSubModuleStripReadout m_StripReadout;

   //! The sub module handling the strip readout noise on non-triggered strips
   MSubModuleStripReadout m_StripReadoutNoise;

   //! The sub module handling triggers and guard ring vetoes
   MSubModuleStripTrigger m_StripTrigger;

   //! The sub module handling depth and timing noise
   MSubModuleDepthReadout m_DepthReadout;

   //! The sub module handling the output of the DEE in to the standard nuclearizer classes
   MSubModuleDEEOutput m_Output;


#ifdef ___CLING___
 public:
  ClassDef(MModuleDEESMEX, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
