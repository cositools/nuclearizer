/*
 * MNuclearizerMain.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
// MNuclearizerMain.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNuclearizerMain.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:
#include <TROOT.h>
#include <TEnv.h>
#include <TApplication.h>
#include <MString.h>
#include <TSystem.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MInterfaceNuclearizer.h"


//////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
  // Main function... the beginning...

  // Initialize global MEGALIB variables, especially mgui, etc.
  MGlobal::Initialize();

  TApplication* AppNuclearizer = new TApplication("Nuclearizer", 0, 0);

  MInterfaceNuclearizer Nuclearizer;
  if (Nuclearizer.ParseCommandLine(argc, argv) == false) {
    return 0;
  } else {
    AppNuclearizer->Run();
  }  

  return 0;
}


// MNuclearizerMain: the end...
//////////////////////////////////////////////////////////////////////////////////
