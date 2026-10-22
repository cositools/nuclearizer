/*
 * MGUIOptionsCOSIPayloadLollipopCut.h
 *
 * Copyright (C) 2008-2010 by Robin Anthony-Petersen.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsCOSIPayloadLollipopCut__
#define __MGUIOptionsCOSIPayloadLollipopCut__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGButtonGroup.h>
#include <MString.h>
#include <TGClient.h>
#include <TGNumberEntry.h>
#include <TGTextEntry.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIERBList.h"
#include "MModule.h"
#include "MGUIOptions.h"

// Forward declarations:
class MGUIEFileSelector;
class MGUIEMinMaxEntry;
class MGUIEEntry;

////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsCOSIPayloadLollipopCut : public MGUIOptions
{
  // public methods:
public:
  //! Default constructor
  MGUIOptionsCOSIPayloadLollipopCut(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsCOSIPayloadLollipopCut();

  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

  //! The creation part which gets overwritten
  virtual void Create();

  // protected methods:
protected:
  //! Actions after the Apply or OK button has been pressed
  virtual bool OnApply();

  //! Toggle radio buttons for source positions
  void ToggleRadioButtons(int WidgetID);

  //! Widget IDs
  enum {
    c_SourcePosition1 = 101,
    c_SourcePosition2,
    c_SourcePosition3,
  };

  // protected members:
protected:
  //! Radio buttons for Source 1 positions
  TGRadioButton* m_RBSourcePosition1;
  TGRadioButton* m_RBSourcePosition2;
  TGRadioButton* m_RBSourcePosition3;

  // private members:
private:

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsCOSIPayloadLollipopCut, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
