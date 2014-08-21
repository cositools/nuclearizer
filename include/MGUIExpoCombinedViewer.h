/*
 * MGUIExpoCombinedViewer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoCombinedViewer__
#define __MGUIExpoCombinedViewer__


////////////////////////////////////////////////////////////////////////////////


// Standard libs
#include <vector>
using namespace std;

// ROOT libs
#include <TGMenu.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGFileDialog.h>
#include <TGIcon.h>
#include <TGPicture.h>
#include <TGTab.h>

// MEGAlib libs
#include "MGUIDialog.h"
#include "MTimer.h"
#include "MGUIExpo.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExpoCombinedViewer : public MGUIDialog
{
  // Public members:
 public:
  //! Default constructor
  MGUIExpoCombinedViewer();
  //! Default destructor
  virtual ~MGUIExpoCombinedViewer();

  //! Set a new expo tab
  void AddExpo(MGUIExpo* Expo) { m_Expos.push_back(Expo); }
  //! Set a new expo tab
  void AddExpos(vector<MGUIExpo*> Expos) { m_Expos.insert(m_Expos.end(), Expos.begin(), Expos.end()); }

  //! Create the GUI
  virtual void Create();
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
 
  //! Return true if we need an update
  bool NeedsUpdate();
 
  //! Update all tabs
  virtual void Update();

  //! Handle the close window event
  virtual void CloseWindow();

  //! Handle the apply button event
  virtual bool OnApply();
  //! Handle the cancel button event
  virtual bool OnCancel();

  // protected members:
 protected:



  // private members:
 private:
  //! All the expo modules
  vector<MGUIExpo*> m_Expos;
  //! The main tab...
  TGTab* m_MainTab;
  //! ... and its layout
  TGLayoutHints* m_MainTabLayout;
  //! The observation time info:
  TGLabel* m_ObservationTime;

  //! The update timer
  MTimer m_Timer;

  

#ifdef ___CINT___
 public:
  ClassDef(MGUIExpoCombinedViewer, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
