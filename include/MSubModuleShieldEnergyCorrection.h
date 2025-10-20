/*
 * MSubModuleShieldEnergyCorrection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleShieldEnergyCorrection__
#define __MSubModuleShieldEnergyCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleShieldEnergyCorrection : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleShieldEnergyCorrection();

  //! No copy constructor
  MSubModuleShieldEnergyCorrection(const MSubModuleShieldEnergyCorrection&) = delete;
  //! No copy assignment
  MSubModuleShieldEnergyCorrection& operator=(const MSubModuleShieldEnergyCorrection&) = delete;
  //! No move constructors
  MSubModuleShieldEnergyCorrection(MSubModuleShieldEnergyCorrection&&) = delete;
  //! No move operators
  MSubModuleShieldEnergyCorrection& operator=(MSubModuleShieldEnergyCorrection&&) = delete;

  //! Default destructor
  virtual ~MSubModuleShieldEnergyCorrection();

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
  ClassDef(MSubModuleShieldEnergyCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
