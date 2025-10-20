/*
 * MSubModuleShieldReadout.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleShieldReadout__
#define __MSubModuleShieldReadout__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleShieldReadout : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleShieldReadout();

  //! No copy constructor
  MSubModuleShieldReadout(const MSubModuleShieldReadout&) = delete;
  //! No copy assignment
  MSubModuleShieldReadout& operator=(const MSubModuleShieldReadout&) = delete;
  //! No move constructors
  MSubModuleShieldReadout(MSubModuleShieldReadout&&) = delete;
  //! No move operators
  MSubModuleShieldReadout& operator=(MSubModuleShieldReadout&&) = delete;

  //! Default destructor
  virtual ~MSubModuleShieldReadout();

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
  ClassDef(MSubModuleShieldReadout, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
