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
    c_Source1Position1 = 101,
    c_Source1Position2,
    c_Source1Position3,
    c_Source1Position4,
    c_Source1Position5,
    c_Source1Position6,

    c_Source2Position1 = 201,
    c_Source2Position2,
    c_Source2Position3,
    c_Source2Position4,
    c_Source2Position5,
    c_Source2Position6
  };

  // protected members:
protected:
  //! Radio buttons for Source 1 positions
  TGRadioButton* m_RBSource1Position1;
  TGRadioButton* m_RBSource1Position2;
  TGRadioButton* m_RBSource1Position3;
  TGRadioButton* m_RBSource1Position4;
  TGRadioButton* m_RBSource1Position5;
  TGRadioButton* m_RBSource1Position6;

  //! Radio buttons for Source 2 positions
  TGRadioButton* m_RBSource2Position1;
  TGRadioButton* m_RBSource2Position2;
  TGRadioButton* m_RBSource2Position3;
  TGRadioButton* m_RBSource2Position4;
  TGRadioButton* m_RBSource2Position5;
  TGRadioButton* m_RBSource2Position6;

  // private members:
private:

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsCOSIPayloadLollipopCut, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
