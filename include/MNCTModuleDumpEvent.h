/*
 * MNCTModuleDumpEvent.h
 *
 * Copyright (C) 2008-2010 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleDumpEvent__
#define __MNCTModuleDumpEvent__


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


class MNCTModuleDumpEvent : public MNCTModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleDumpEvent();
  //! Default destructor
  virtual ~MNCTModuleDumpEvent();

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MNCTEvent* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  // protected methods:
 protected:
  //!
  void WriteHeader();

  //!
  void DumpBasic(MNCTEvent* Event);

  //!
  void DumpHitsSim(MNCTHit* HitSim);

  //!
  void DumpStripHits(MNCTStripHit* StrpHit);

  //!
  void DumpHits(MNCTHit* Hit);
  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //!
  ofstream m_OutFile;

#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleDumpEvent, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
