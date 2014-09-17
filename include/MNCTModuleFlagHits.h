/*
 * MNCTModuleFlagHits.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleFlagHits__
#define __MNCTModuleFlagHits__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

using namespace std;

class MNCTModuleFlagHits : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleFlagHits();
  //! Default destructor
  virtual ~MNCTModuleFlagHits();

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

	void ReadHit(MNCTHit*);
	bool CheckCrossTalk(int, bool,  double, double, double);
	bool CheckChargeLoss(bool, double, double, double);
	TF1* GetCrossTalkEnergyDiff(int);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:




#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleFlagHits, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
