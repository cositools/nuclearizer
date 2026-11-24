/*
 * MGUIOptionsStripPairingMultiRoundChiSquare.h
 *
 * Copyright (C) by Julian Gerber.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsStripPairingMultiRoundChiSquare__
#define __MGUIOptionsStripPairingMultiRoundChiSquare__


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


class MGUIOptionsStripPairingMultiRoundChiSquare : public MGUIOptions
{
    // public Session:
     public:
      //! Default constructor
    MGUIOptionsStripPairingMultiRoundChiSquare(MModule* Module);
      //! Default destructor
      virtual ~MGUIOptionsStripPairingMultiRoundChiSquare();

      //! Process all button, etc. messages
      virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);

      //! The creation part which gets overwritten
      virtual void Create();

      // protected methods:
     protected:

      //! Actions after the Apply or OK button has been pressed
      virtual bool OnApply();


      // protected members:
     protected:
      //! Maximum number of strip hits
      MGUIEEntry* m_MaximumStrips;

      // private members:
     private:



#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsStripPairingMultiRoundChiSquare, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
