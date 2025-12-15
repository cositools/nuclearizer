/*
 * MModuleStripPairingMultiRoundChiSquare.h
 *
 * Copyright (C) by Julian Gerber & Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleStripPairingMultiRoundChiSquare__
#define __MModuleStripPairingMultiRoundChiSquare__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include<iostream>
#include<vector>
#include<cmath>
#include<algorithm>
#include<map>
#include<limits>
#include <numeric>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MStripHit.h"
#include "MGUIExpoStripPairing.h"
#include "MGUIExpoStripPairingHits.h"
#include "MGUIExpoStripPairingStripHits.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleStripPairingMultiRoundChiSquare : public MModule
{
  // public interface:
 public:
  //! Default constructor
    MModuleStripPairingMultiRoundChiSquare();
  //! Default destructor
  virtual ~MModuleStripPairingMultiRoundChiSquare();
  
  //! Create a new object of this class 
  virtual MModuleStripPairingMultiRoundChiSquare* Clone() { return new MModuleStripPairingMultiRoundChiSquare(); }

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
  //! Find a new set of combinations giving the existing gone
    vector<vector<vector<unsigned int>>> FindNewCombinations(vector<vector<vector<unsigned int>>> OldOnes, vector<MStripHit*> StripHits, bool RoundTwo);
    
  //! Function to apply charge trapping correction
    float ChargeTrappingCorrection(unsigned int d, vector<vector<MStripHit*>> StripHits);
    
  //! Divide an event's strip hits by detector and LV/HV side
    vector<vector<vector<MStripHit*>>> CollectStripHits(MReadOutAssembly* Event);

  //! Read in strip hits on each side for each detector and perform quality selections
    bool EventSelection(MReadOutAssembly* Event, vector<vector<vector<MStripHit*>>> StripHits);
    
  //! Find all strip combinations for each detector on LV and HV sides given seed combinations
    vector<vector<vector<vector<vector<unsigned int>>>>> FindAllCombinations(unsigned int d, vector<vector<vector<vector<vector<unsigned int>>>>> Combinations, vector<vector<vector<MStripHit*>>> StripHits, bool RoundTwo);

  //! Evaluate the reduced chi square for all possible strip pairings
    tuple<vector<vector<unsigned int>>, vector<vector<unsigned int>>, double> EvaluateAllCombinations(unsigned int d, vector<vector<vector<vector<vector<unsigned int>>>>> Combinations, vector<vector<vector<MStripHit*>>> StripHits);
  //! Create hits
    bool CreateHits(unsigned int d, MReadOutAssembly* Event, vector<vector<vector<MStripHit*>>> StripHits, vector<vector<unsigned int>> BestLVSideCombo, vector<vector<unsigned int>> BestHVSideCombo);
    
    //! Return the order of indices resulting from sorting a vector
    vector<size_t> Argsort(vector<double> &list);

  // private methods:
 private:



  // protected members:
 protected:
  //! The display of debugging data
  MGUIExpoStripPairing* m_ExpoStripPairing;
  MGUIExpoStripPairingHits* m_ExpoStripPairingHits;
  MGUIExpoStripPairingStripHits* m_ExpoStripPairingStripHits;


  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MModuleStripPairingMultiRoundChiSquare, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
