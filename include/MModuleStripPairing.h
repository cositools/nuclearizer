/*
 * MModuleStripPairing.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleStripPairing__
#define __MModuleStripPairing__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleStripPairing : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleStripPairing();
  //! Default destructor
  virtual ~MModuleStripPairing();
  
  //! Create a new object of this class 
  virtual MModuleStripPairing* Clone() { return new MModuleStripPairing(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MModuleStripPairing, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
