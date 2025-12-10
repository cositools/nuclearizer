/*
 * MModuleDiagnosticsEnergyPerStrip.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleDiagnosticsEnergyPerStrip__
#define __MModuleDiagnosticsEnergyPerStrip__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MFile.h"

// Nuclearizer libs:
#include "MModule.h"
#include "MGUIExpoDiagnosticsEnergyPerStrip.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleDiagnosticsEnergyPerStrip : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleDiagnosticsEnergyPerStrip();
  //! Default destructor
  virtual ~MModuleDiagnosticsEnergyPerStrip();
  
  //! Create a new object of this class 
  virtual MModuleDiagnosticsEnergyPerStrip* Clone() { return new MModuleDiagnosticsEnergyPerStrip(); }

  //! Create the expos
  virtual void CreateExpos();

  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);
  
  // protected methods:
 protected:
  
  // private methods:
 private:

  // protected members:
 protected:
  //! The display of debugging data
  MGUIExpoDiagnosticsEnergyPerStrip* m_ExpoDiagnosticsEnergyPerStrip;

  //! Indicator, that we have energies
  bool m_HasEnergies;
  //! Indictaor, that we have hits
  bool m_HasHits;
  //! Number of events for setup
  unsigned int m_NSetupEvents;


  // private members:
 private:

  
#ifdef ___CLING___
 public:
  ClassDef(MModuleDiagnosticsEnergyPerStrip, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
