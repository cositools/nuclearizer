/*
 * MSubModuleTemplate.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleTemplate__
#define __MSubModuleTemplate__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleTemplate : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleTemplate();

  //! No copy constructor
  MSubModuleTemplate(const MSubModuleTemplate&) = delete;
  //! No copy assignment
  MSubModuleTemplate& operator=(const MSubModuleTemplate&) = delete;
  //! No move constructors
  MSubModuleTemplate(MSubModuleTemplate&&) = delete;
  //! No move operators
  MSubModuleTemplate& operator=(MSubModuleTemplate&&) = delete;

  //! Default destructor
  virtual ~MSubModuleTemplate();

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
  ClassDef(MSubModuleTemplate, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
