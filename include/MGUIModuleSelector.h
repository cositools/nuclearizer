/*
 * MGUIModuleSelector.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIModuleSelector__
#define __MGUIModuleSelector__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <MString.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIERBList.h"
#include "MNCTData.h"
#include "MNCTModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIModuleSelector : public TGTransientFrame
{
  // public methods:
 public:
  //! Default constructor
  MGUIModuleSelector(MNCTData* Data, unsigned int Position);
  //! Default destructor
  virtual ~MGUIModuleSelector();

  //! Close this window
  void CloseWindow();
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);


  // protected methods:
 protected:
  //! Create the Gui
  void Create();

  //! Actions when the OK button has been pressed
	virtual bool OnOk();
  //! Actions when the Cancel button has been pressed
	virtual bool OnCancel();
  //! Actions when the Apply button has been pressed
	virtual bool OnApply();


  // protected members:
 protected:

	enum BasicButtonIDs { e_Ok = 1, e_Cancel, e_Apply };

  // private members:
 private:
  //! All the data
  MNCTData* m_Data;
  //! The position in the analysis pipeline
  unsigned int m_Position;

  //! The list
  MGUIERBList* m_List;

#ifdef ___CINT___
 public:
  ClassDef(MGUIModuleSelector, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
