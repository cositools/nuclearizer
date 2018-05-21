/*
 * MNuclearizerMain.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNuclearizerMain
//
////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <sstream>
#include <vector>
#include <csignal>
using namespace std;

// ROOT libs:
#include "TROOT.h"
#include "TApplication.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssembly.h"


//////////////////////////////////////////////////////////////////////////////////


MAssembly* g_Prg = 0;
int g_NInterruptCatches = 1;


////////////////////////////////////////////////////////////////////////////////


//! Called when an interrupt signal is flagged
//! All catched signals lead to a well defined exit of the program
void CatchSignal(int a)
{
  if (g_Prg != 0 && g_NInterruptCatches-- > 0) {
    cout<<"Catched signal Ctrl-C: sent the signal to interrupt, call Ctrl-C again for abort."<<endl;
    g_Prg->SetInterrupt();
  } else {
    abort();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! In the beginning Andreas created main and Andreas said "Let there be code!"
//! After many years of coding and debugging, Andreas saw all that he had made, 
//! and it was very good.
int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Catch a user interrupt for graceful shutdown
  signal(SIGINT, CatchSignal);

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication* AppNuclearizer = new TApplication("Nuclearizer", 0, 0);

  MAssembly Nuclearizer;
  g_Prg = &Nuclearizer;
  if (Nuclearizer.ParseCommandLine(argc, argv) == false) {
    return 0;
  } else {
    AppNuclearizer->Run();
  }  

  return 0;
}


// MNuclearizerMain: the end...
////////////////////////////////////////////////////////////////////////////////
