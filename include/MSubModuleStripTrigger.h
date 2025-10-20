/*
 * MSubModuleStripTrigger.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleStripTrigger__
#define __MSubModuleStripTrigger__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleStripTrigger : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleStripTrigger();

  //! No copy constructor
  MSubModuleStripTrigger(const MSubModuleStripTrigger&) = delete;
  //! No copy assignment
  MSubModuleStripTrigger& operator=(const MSubModuleStripTrigger&) = delete;
  //! No move constructors
  MSubModuleStripTrigger(MSubModuleStripTrigger&&) = delete;
  //! No move operators
  MSubModuleStripTrigger& operator=(MSubModuleStripTrigger&&) = delete;

  //! Default destructor
  virtual ~MSubModuleStripTrigger();

  //! Initialize the module
  virtual bool Initialize();

  //! Clear event data from the module
  virtual void Clear();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Return true if we have a trigger - filled after AnalyzeEvent
  bool HasTrigger() const { return m_HasTrigger; }

  //! Return true if we have a veto - filled after AnalyzeEvent
  bool HasVeto() const { return m_HasVeto; }

  //! Return the time when the dead time ends - filled after AnalyzeEvent
  MTime GetDeadTimeEnd() const { return m_DeadTimeEnd; }

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
  //! Flag indicating that a trigger has been raised
  bool m_HasTrigger;

  //! Flag indicating that a veto has been raised
  bool m_HasVeto;

  //! Time when the shield dead time ends
  MTime m_DeadTimeEnd;



#ifdef ___CLING___
 public:
  ClassDef(MSubModuleStripTrigger, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
