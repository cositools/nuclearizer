/*
 * MModuleDiagnostics.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleDiagnostics__
#define __MModuleDiagnostics__


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
#include "MGUIExpoDiagnostics.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleDiagnostics : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleDiagnostics();
  //! Default destructor
  virtual ~MModuleDiagnostics();
  
  //! Create a new object of this class 
  virtual MModuleDiagnostics* Clone() { return new MModuleDiagnostics(); }

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
  MGUIExpoDiagnostics* m_ExpoDiagnostics;

  // private members:
 private:

  
#ifdef ___CLING___
 public:
  ClassDef(MModuleDiagnostics, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
