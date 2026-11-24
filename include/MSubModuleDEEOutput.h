/*
 * MSubModuleDEEOutput.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleDEEOutput__
#define __MSubModuleDEEOutput__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleDEEOutput : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleDEEOutput();

  //! No copy constructor
  MSubModuleDEEOutput(const MSubModuleDEEOutput&) = delete;
  //! No copy assignment
  MSubModuleDEEOutput& operator=(const MSubModuleDEEOutput&) = delete;
  //! No move constructors
  MSubModuleDEEOutput(MSubModuleDEEOutput&&) = delete;
  //! No move operators
  MSubModuleDEEOutput& operator=(MSubModuleDEEOutput&&) = delete;

  //! Default destructor
  virtual ~MSubModuleDEEOutput();

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
  ClassDef(MSubModuleDEEOutput, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
