/*
 * MNCTModuleEventFilter.h
 *
 * Copyright (C) 2008-2010 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleEventFilter__
#define __MNCTModuleEventFilter__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>

// ROOT libs:
#include "MString.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MNCTModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleEventFilter : public MNCTModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleEventFilter();
  //! Default destructor
  virtual ~MNCTModuleEventFilter();

  //! Initialize the module
  virtual bool Initialize();

  //!
  virtual MString Report();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MNCTEvent* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  //!
  void SetVetoSetting(MString VSetting){m_VetoSetting=VSetting;}
  MString GetVetoSetting(){return m_VetoSetting;}

  // protected methods:
 protected:
  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //!
  string m_VetoSetting;
  
  //!
  int m_NEvent;
  
  //!
  int m_NVeto;
  
  //!
  vector<int> m_VetoList;

  //!
  //vector<int> m_IgnoreList;

#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleEventFilter, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
