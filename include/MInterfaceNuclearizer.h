/*
 * MInterfaceNuclearizer.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MInterfaceNuclearizer__
#define __MInterfaceNuclearizer__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>
using namespace std;

// ROOT libs
#include "MVector.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MNCTData.h"

class MGUINuclearizerMain;

////////////////////////////////////////////////////////////////////////////////


class MInterfaceNuclearizer
{
  // Public Interface:
 public:
  //! Default constructor
  MInterfaceNuclearizer();
  //! Default destructor
  ~MInterfaceNuclearizer();

  //! Each interface must be able to parse a command line - 
  //! this function is called by main()
  bool ParseCommandLine(int argc, char** argv);

  //! Analyze the data
  bool Analyze();

  //! Exit the application
  void Exit();

  // protected methods:
 protected:
 

  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! True if the GUI is used
  bool m_UseGui;
  //! The graphical usr interface
  MGUINuclearizerMain* m_Gui;

  //! The store for all user data of the GUI:
  MNCTData* m_Data;

#ifdef ___CINT___
 public:
  ClassDef(MInterfaceNuclearizer, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
