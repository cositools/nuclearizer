/*
 * MModuleNearestNeighbor.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleNearestNeighbor__
#define __MModuleNearestNeighbor__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleNearestNeighbor : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleNearestNeighbor();
  //! Default destructor
  virtual ~MModuleNearestNeighbor();
  
  //! Create a new object of this class 
  virtual MModuleNearestNeighbor* Clone() { return new MModuleNearestNeighbor(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

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
  ClassDef(MModuleNearestNeighbor, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
