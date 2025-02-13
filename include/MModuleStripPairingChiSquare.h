/*
 * MModuleStripPairingChiSquare.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleStripPairingChiSquare__
#define __MModuleStripPairingChiSquare__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include<iostream>
#include<vector>
#include<cmath>
#include<algorithm>
#include<map>
#include<limits>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MStripHit.h"
#include "MGUIExpoStripPairing.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleStripPairingChiSquare : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleStripPairingChiSquare();
  //! Default destructor
  virtual ~MModuleStripPairingChiSquare();
  
  //! Create a new object of this class 
  virtual MModuleStripPairingChiSquare* Clone() { return new MModuleStripPairingChiSquare(); }

  //! Create the expos
  virtual void CreateExpos();

  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  // protected methods:
 protected:
  // Find a new set of combinations giving the existing gone
  vector<vector<vector<unsigned int>>> FindNewCombinations(vector<vector<vector<unsigned int>>> OldOnes, vector<MStripHit*> StripHits);

  // private methods:
 private:



  // protected members:
 protected:
  //! The display of debugging data
  MGUIExpoStripPairing* m_ExpoStripPairing;


  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MModuleStripPairingChiSquare, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
