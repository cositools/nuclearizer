/*
 * __MSubModuleChargeTransport__.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleChargeTransport__
#define __MSubModuleChargeTransport__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Class handling the charge transport in the GeD detectors
//! End point is the energy in the individual strips and the guard ring
class MSubModuleChargeTransport : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleChargeTransport();

  //! No copy constructor
  MSubModuleChargeTransport(const MSubModuleChargeTransport&) = delete;
  //! No copy assignment
  MSubModuleChargeTransport& operator=(const MSubModuleChargeTransport&) = delete;
  //! No move constructors
  MSubModuleChargeTransport(MSubModuleChargeTransport&&) = delete;
  //! No move operators
  MSubModuleChargeTransport& operator=(MSubModuleChargeTransport&&) = delete;

  //! Default destructor
  virtual ~MSubModuleChargeTransport();

  //! Initialize the module
  virtual bool Initialize();

  //! Clear event data from the module
  virtual void Clear();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Finalize the module
  virtual void Finalize();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration(MXmlNode* Node);

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
  ClassDef(MSubModuleChargeTransport, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
