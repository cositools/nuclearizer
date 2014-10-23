/*
 * MGUIOptionsReceiverCOSI2014.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsReceiverCOSI2014__
#define __MGUIOptionsReceiverCOSI2014__


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
#include "MGUIEFileSelector.h"
#include "MGUIOptions.h"
#include "MGUIEEntry.h"
#include "MGUIERBList.h"

// Nuclearizer libs:
#include "MModule.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsReceiverCOSI2014 : public MGUIOptions
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptionsReceiverCOSI2014(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptionsReceiverCOSI2014();

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

  // private members:
 private:
  MGUIEEntry* m_DistributorName;
  MGUIEEntry* m_DistributorPort;
  MGUIEEntry* m_DistributorStreamID;

  //MGUIEEntry* m_SendToName;
  //MGUIEEntry* m_SendToPort;

  MGUIERBList* m_DataMode;

  //! Select if we save the file to roa
  MGUIEFileSelector* m_FileSelector;
  
  #ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsReceiverCOSI2014, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
