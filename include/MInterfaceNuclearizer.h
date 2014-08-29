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
class MGUIExpoCombinedViewer;

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

  //! Analyze the data - single-threaded or multi-threaded mode
  bool Analyze();
  
  //! Exit the application - if multi-threaded prepare to exit after all threads have exited
  void Exit();

  //! Show the expo view
  void View();
  
  //! Set the interrupt which will end the analysis
  void SetInterrupt(bool Flag = true);

  // protected methods:
 protected:
  //! Analyze the data - single-threaded mode
  bool AnalyzeSingleThreaded();
  //! Analyze the data - multi-threaded mode
  bool AnalyzeMultiThreaded();
 
  //! End the program (and saves the GUI data)
  void Terminate();
  
  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! True if the GUI is used
  bool m_UseGui;
  //! The main graphical user interface
  MGUINuclearizerMain* m_Gui;
  //! The expos - main combined viewer
  MGUIExpoCombinedViewer* m_ExpoCombinedViewer;
  
  //! The store for all user data of the GUI:
  MNCTData* m_Data;

  //! The interrupt flag - the analysis will stop when this flag is set
  bool m_Interrupt;
  //! The terminate flag - should always be set together with the interrupt flag
  //! After the analysis is stopped by the interupt flag, this flag will terminate the 
  //! program
  bool m_Terminate;
  
  //! Chatty-ness of nuclearizer
  int m_Verbosity;
  
  //! True if multi-threading is enabled
  bool m_UseMultiThreading;
  
  //! True if the analysis is currently underway
  bool m_IsAnalysisRunning;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MInterfaceNuclearizer, 0) // image reconstruction management class 
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
