/*
 * MSubModuleRandomCoincidence.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleRandomCoincidence__
#define __MSubModuleRandomCoincidence__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleRandomCoincidence : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleRandomCoincidence();

  //! No copy constructor
  MSubModuleRandomCoincidence(const MSubModuleRandomCoincidence&) = delete;
  //! No copy assignment
  MSubModuleRandomCoincidence& operator=(const MSubModuleRandomCoincidence&) = delete;
  //! No move constructors
  MSubModuleRandomCoincidence(MSubModuleRandomCoincidence&&) = delete;
  //! No move operators
  MSubModuleRandomCoincidence& operator=(MSubModuleRandomCoincidence&&) = delete;

  //! Default destructor
  virtual ~MSubModuleRandomCoincidence();

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
  ClassDef(MSubModuleRandomCoincidence, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
