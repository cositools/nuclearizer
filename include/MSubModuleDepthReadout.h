/*
 * MSubModuleDepthReadout.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleDepthReadout__
#define __MSubModuleDepthReadout__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleDepthReadout : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleDepthReadout();

  //! No copy constructor
  MSubModuleDepthReadout(const MSubModuleDepthReadout&) = delete;
  //! No copy assignment
  MSubModuleDepthReadout& operator=(const MSubModuleDepthReadout&) = delete;
  //! No move constructors
  MSubModuleDepthReadout(MSubModuleDepthReadout&&) = delete;
  //! No move operators
  MSubModuleDepthReadout& operator=(MSubModuleDepthReadout&&) = delete;

  //! Default destructor
  virtual ~MSubModuleDepthReadout();

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
  ClassDef(MSubModuleDepthReadout, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
