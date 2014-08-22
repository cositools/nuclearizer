/*
 * MGUIExpo.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpo__
#define __MGUIExpo__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TString.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MTimer.h"
#include "MXmlNode.h"
#include "MGUIERBList.h"

// Nuclearizer libs

// Forward declarations:
class MNCTModule;


////////////////////////////////////////////////////////////////////////////////


class MGUIExpo : public TGCompositeFrame
{
  // public Session:
 public:
  //! Default constructor - if the module is not set, this is the first thing to set
  MGUIExpo(MNCTModule* Module = 0);
  //! Default destructor
  virtual ~MGUIExpo();

  //! Set the module
  void SetModule(MNCTModule* Module) { m_Module = Module; }
  
  //! Close the window
  void CloseWindow();
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  
  //! The creation part which gets overwritten
  virtual void Create() {};

  //! Update the frame - must be overwritten to refresh the histograms, etc.
  virtual void Update() {};

  //! Reset the data in the UI - must be overwritten to clear the histograms, etc.
  virtual void Reset() {};

  //! Print the data in the UI - must be overwritten
  virtual void Print(const MString& FileName) {};

  //! Get the title
  MString GetTabTitle() { return m_TabTitle; }

  //! Return true if we need an update
  bool NeedsUpdate() { return m_NeedsUpdate; }

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Add XML nodes to the tree of the module 
  virtual bool WriteXmlConfiguration(MXmlNode* ModuleNode);
  
  // protected methods:
 protected:


  // protected members:
 protected:
  //! The module
  MNCTModule* m_Module;
   
  //! Tab Title of the GUI element
  MString m_TabTitle;
  
  //! Flag to indicate we need an update
  bool m_NeedsUpdate;
  
  //! True if it has been created
  bool m_IsCreated;
  
  // private members:
 private:

#ifdef ___CINT___
 public:
  ClassDef(MGUIExpo, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
