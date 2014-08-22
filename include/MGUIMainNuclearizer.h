/*
* MGUINuclearizerMain.h
*
* Copyright (C) by Andreas Zoglauer.
* All rights reserved.
*
* Please see the source-file for the copyright-notice.
*
*/


#ifndef __MGUINuclearizerMain__
#define __MGUINuclearizerMain__


////////////////////////////////////////////////////////////////////////////////


// Standard libs

// ROOT libs
#include <TGMenu.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGFileDialog.h>
#include <TGIcon.h>
#include <TGPicture.h>

// MEGAlib libs
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MInterfaceNuclearizer.h"
#include "MNCTData.h"
#include "MGUIEModule.h"

// Forward declarations:
class MGUIEFileSelector;

////////////////////////////////////////////////////////////////////////////////


class MGUINuclearizerMain : public TGMainFrame
{
  // Public members:
public:
  //! Default constructor
  MGUINuclearizerMain(MInterfaceNuclearizer* Interface, MNCTData* Data);
  //! Default destructor
  virtual ~MGUINuclearizerMain();

  //! Create the GUI
  virtual void Create();
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  //! Called when the "x" is pressed
  virtual void CloseWindow();

  //! Handle some keys
  bool HandleKey(Event_t* Event);

  // protected members:
protected:
  //! Update the module section
  void UpdateModules();

  //! Actions when the change button has been pressed
  virtual bool OnChange(unsigned int ModuleID);
  //! Actions when the remove button has been pressed
  virtual bool OnRemove(unsigned int ModuleID);
  //! Actions when the options button has been pressed
  virtual bool OnOptions(unsigned int ModuleID);
  //! Actions when the apply button has been pressed
  virtual bool OnApply();
  //! Actions when the start button has been pressed
  virtual bool OnStart();
  //! Actions when the exit button has been pressed
  virtual bool OnExit();
  //! Actions when the stop button has been pressed
  virtual bool OnStop();
  //! Actions when the view button has been pressed
  virtual bool OnView();
  //! Actions when the save key has been pressed
  virtual bool OnSaveConfiguration();
  //! Actions when the load key has been pressed
  virtual bool OnLoadConfiguration();
  //! Actions when the load key has been pressed
  virtual bool OnGeometry();
  //! Actions when the about button has been pressed
  virtual bool OnAbout();

  // private members:
private:
  //! Reference to all interface functions
  MInterfaceNuclearizer* m_Interface;
  //! Reference to all GUI data
  MNCTData* m_Data;

  // Some common used GUI elements

  //! The frame for the modules
  TGGroupFrame* m_ModuleFrame;
  //! The layout of an module
  TGLayoutHints* m_ModuleLayout;
  
  //! List of the modules
  vector<MGUIEModule*> m_Modules;

  //! GUI element storing the current load file name
  MGUIEFileSelector* m_FileSelectorLoad;
  //! GUI element storing the current save file name
  MGUIEFileSelector* m_FileSelectorSave;
  //! GUI element storing the current geometry file name
  MGUIEFileSelector* m_FileSelectorGeometry;


  // IDs:
  static const int c_Start      =   1;
  static const int c_Exit       =   2;
  static const int c_LoadConfig =   3;
  static const int c_SaveConfig =   4;
  static const int c_Geometry   =   5;
  static const int c_About      =   6;
  static const int c_Stop       =   7;
  static const int c_View       =   8;
  static const int c_Remove     = 400;
  static const int c_Options    = 500;
  static const int c_Change     = 600;

#ifdef ___CINT___
public:
  ClassDef(MGUINuclearizerMain, 0) // main window of the Nuclearizer GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
