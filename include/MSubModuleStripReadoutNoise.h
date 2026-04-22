/*
 * MSubModuleStripReadoutNoise.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleStripReadoutNoise__
#define __MSubModuleStripReadoutNoise__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleStripReadoutNoise : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleStripReadoutNoise();

  //! No copy constructor
  MSubModuleStripReadoutNoise(const MSubModuleStripReadoutNoise&) = delete;
  //! No copy assignment
  MSubModuleStripReadoutNoise& operator=(const MSubModuleStripReadoutNoise&) = delete;
  //! No move constructors
  MSubModuleStripReadoutNoise(MSubModuleStripReadoutNoise&&) = delete;
  //! No move operators
  MSubModuleStripReadoutNoise& operator=(MSubModuleStripReadoutNoise&&) = delete;

  //! Default destructor
  virtual ~MSubModuleStripReadoutNoise();

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
  ClassDef(MSubModuleStripReadoutNoise, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
