/*
 * MModuleStripPairingMultiRoundChiSquare.cxx
 * Multi Round Chi Square Version
 *
 * Copyright (C) by Julian Gerber & Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Julian Gerber & Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MModuleStripPairingMultiRoundChiSquare
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleStripPairingMultiRoundChiSquare.h"

// Standard libs:
#include <fstream>
#include <iostream>

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MModule.h"
#include "MGUIOptionsStripPairing.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleStripPairingMultiRoundChiSquare)
#endif


////////////////////////////////////////////////////////////////////////////////

//! Define constants to be used in strip pairing

const unsigned int MaxCombinations = 5; // Defines maximum number of strip combinations allowed in pairing
const unsigned int MaxStripHits = 6; // Define maximum number of strip hits on any one side
const unsigned int ChiSquareThreshold = 100; // If strip pairing does not reach this threshold, it will enter round two

////////////////////////////////////////////////////////////////////////////////


MModuleStripPairingMultiRoundChiSquare::MModuleStripPairingMultiRoundChiSquare() : MModule()
{
  // Construct an instance of MModuleStripPairingMultiRoundChiSquare

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Strip pairing - Multi Round Chi Square";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagStripPairingMultiRoundChiSquare";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_TACcut);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_StripPairing);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options

  // Can the program be run multi-threaded
  m_AllowMultiThreading = true;

  // Can we use multiple instances of this class
  m_AllowMultipleInstances = true;
}


////////////////////////////////////////////////////////////////////////////////


MModuleStripPairingMultiRoundChiSquare::~MModuleStripPairingMultiRoundChiSquare()
{
  // Delete this instance of MModuleStripPairingMultiRoundChiSquare
}


////////////////////////////////////////////////////////////////////////////////


void MModuleStripPairingMultiRoundChiSquare::CreateExpos()
{
  // Create all expos

  if (HasExpos() == true) {
    return;
  }

  // Set the histogram display
  m_ExpoStripPairing = new MGUIExpoStripPairing(this);
  m_ExpoStripPairing->SetEnergiesHistogramParameters(1500, 0, 1500);
  m_Expos.push_back(m_ExpoStripPairing);

  m_ExpoStripPairingHits = new MGUIExpoStripPairingHits(this);
  m_ExpoStripPairingHits->SetHitsHistogramParameters(5, 0.5, 5.5);
  m_Expos.push_back(m_ExpoStripPairingHits);

  m_ExpoStripPairingStripHits = new MGUIExpoStripPairingStripHits(this);
  m_ExpoStripPairingStripHits->SetStripHitsHistogramParameters(10, 0.5, 10.5);
  m_Expos.push_back(m_ExpoStripPairingStripHits);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleStripPairingMultiRoundChiSquare::Initialize()
{
  // Initialize the module

  return MModule::Initialize();
}

////////////////////////////////////////////////////////////////////////////////
//! Function returns new combinations of strips based on seed combinations
vector<vector<vector<unsigned int>>> MModuleStripPairingMultiRoundChiSquare::FindNewCombinations(vector<vector<vector<unsigned int>>> OldOnes, vector<MStripHit*> StripHits, bool RoundTwo)
{
  // Define new vector of ints NewOnes
  vector<vector<vector<unsigned int>>> NewOnes; // <list> of <combinations> of <combined strips>

  for (unsigned int listspot = 0; listspot < OldOnes.size(); ++listspot) { // Iterate over set of seed combinations
    // New single merges
    for (unsigned int combi1 = 0; combi1 < OldOnes[listspot].size(); ++combi1) { // Iterate over individual combos within set
      for (unsigned int combi2 = combi1 + 1; combi2 < OldOnes[listspot].size(); ++combi2) { // Iterate through all the combos AFTER combi1
        vector<unsigned int> NewCombinedStrips;
        NewCombinedStrips.insert(NewCombinedStrips.end(), OldOnes[listspot][combi1].begin(), OldOnes[listspot][combi1].end());
        NewCombinedStrips.insert(NewCombinedStrips.end(), OldOnes[listspot][combi2].begin(), OldOnes[listspot][combi2].end());
        sort(NewCombinedStrips.begin(), NewCombinedStrips.end());

        // The above combines each combination with all the subsequent combinations in order to produce new combinations of strips

        vector<unsigned int> NewCombinedAsIDs;
        for (unsigned int s = 0; s < NewCombinedStrips.size(); ++s) {
          NewCombinedAsIDs.push_back(StripHits[NewCombinedStrips[s]]->GetStripID()); // Translates the hit number to the actual strip ID
        }
        sort(NewCombinedAsIDs.begin(), NewCombinedAsIDs.end());

        if (RoundTwo == false) {
          bool AllAdjacent = true;
          for (unsigned int c = 1; c < NewCombinedAsIDs.size(); ++c) {
            if (NewCombinedAsIDs[c - 1] + 1 != NewCombinedAsIDs[c]) {
              AllAdjacent = false;
              break;
            }
          } // Checks if the new combo is of adjacent strips

          if (AllAdjacent == true) {
            vector<vector<unsigned int>> NewCombo; // List of newly combined strips
            for (unsigned int news = 0; news < OldOnes[listspot].size(); ++news) {
              if (news != combi1 && news != combi2) {
                NewCombo.push_back(OldOnes[listspot][news]);
              }
              if (news == combi1) {
                NewCombo.push_back(NewCombinedStrips);
              }
            }
            NewOnes.push_back(NewCombo);
          }
        } else {
          vector<vector<unsigned int>> NewCombo;
          for (unsigned int news = 0; news < OldOnes[listspot].size(); ++news) {
            if (news != combi1 && news != combi2) {
              NewCombo.push_back(OldOnes[listspot][news]);
            }
            if (news == combi1) {
              NewCombo.push_back(NewCombinedStrips);
            }
          }
          NewOnes.push_back(NewCombo);
        }
      }
    }
  }

  return NewOnes;
}


////////////////////////////////////////////////////////////////////////////////

//! Apply a charge trapping correction to each potential pair of LV/HV strips
float MModuleStripPairingMultiRoundChiSquare::ChargeTrappingCorrection(unsigned int d, vector<vector<MStripHit*>> StripHits)
{

  // Dummy Function
  // Idea: Read in detector number and set of LV/HV strips to be paired
  // Will also need to read in actual charge trapping parameters from text file
  // The LV/HVStripSet will only be multiple strips if there is charge sharing going on. Otherwise it'll just be a vector with a single number. That number will have to be converted to the actual stripID
  // In the case of charge sharing, will have to find "dominant" LV/HV strip to get correct CTD
  // I'm going to make another function "FindDominantStrip" to just get out the strip on LV or HV side that has the highest energy
  // Return a corrected energy after applying charge trapping correction. Does there need to be two values or is correcting one side enough?

  float EnergyCorrection = 0;

  return EnergyCorrection;
}

////////////////////////////////////////////////////////////////////////////////

//! Divide an event's strip hits by detector and LV/HV side
vector<vector<vector<MStripHit*>>> MModuleStripPairingMultiRoundChiSquare::CollectStripHits(MReadOutAssembly* Event)
{

  // Split hits by detector ID
  vector<unsigned int> DetectorIDs; // List of detector IDs
  vector<vector<vector<MStripHit*>>> StripHits; // list of detector IDs, list of sides (LV and HV), list of strip hits

  for (unsigned int sh = 0; sh < Event->GetNStripHits(); ++sh) { // Populate StripHits with this event's strip hits
    MStripHit* SH = Event->GetStripHit(sh);
    unsigned int Side = (SH->IsLowVoltageStrip() == true) ? 0 : 1;

    // Check if detector is on list
    bool DetectorFound = false;
    unsigned int DetectorPos = 0;
    for (unsigned int d = 0; d < DetectorIDs.size(); ++d) {
      if (DetectorIDs[d] == SH->GetDetectorID()) {
        DetectorFound = true;
        DetectorPos = d;
      }
    }

    // Once the correct detector is found, add strip hit to StripHits
    if (DetectorFound == true) {
      StripHits[DetectorPos][Side].push_back(SH);
    } else { // If encountering a new detector, initialize list of sides/hits corresponding to that detector
      vector<vector<MStripHit*>> List; // list of sides, list of hits
      List.push_back(vector<MStripHit*>()); // LV
      List.push_back(vector<MStripHit*>()); // HV
      List[Side].push_back(SH);
      StripHits.push_back(List);
      DetectorIDs.push_back(SH->GetDetectorID());
    }
  }
  return StripHits;
}

////////////////////////////////////////////////////////////////////////////////

//! Read in strip hits on each side for each detector and perform quality selections
bool MModuleStripPairingMultiRoundChiSquare::EventSelection(MReadOutAssembly* Event, vector<vector<vector<MStripHit*>>> StripHits)
{

  // Limit the number of strip hits on each side
  for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
    for (unsigned int side = 0; side <= 1; ++side) { // Side loop
      if (StripHits[d][side].size() > MaxStripHits) {
        Event->SetStripPairingIncomplete(true, "More than 6 hit strips on one side");
        Event->SetAnalysisProgress(MAssembly::c_StripPairing);
        return false;
      }

      // Check if one side of the detector has no strip hits
      if (StripHits[d][side].size() == 0) {
        Event->SetStripPairingIncomplete(true, "One detector side has no strip hits");
        Event->SetAnalysisProgress(MAssembly::c_StripPairing);
        return false;
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

//! Find all strip combinations for each detector on LV and HV sides given seed combinations
vector<vector<vector<vector<vector<unsigned int>>>>> MModuleStripPairingMultiRoundChiSquare::FindAllCombinations(unsigned int d, vector<vector<vector<vector<vector<unsigned int>>>>> Combinations, vector<vector<vector<MStripHit*>>> StripHits, bool RoundTwo)
{

  for (unsigned int side = 0; side <= 1; ++side) { // Side loop (LV and HV)

    vector<vector<vector<unsigned int>>> NewCombinations; // List of combinations of combined strips

    bool CombinationsAdded = true;
    while (CombinationsAdded == true) {
      CombinationsAdded = false;

      NewCombinations = FindNewCombinations(Combinations[d][side], StripHits[d][side], RoundTwo);
      //cout<<"Size: "<<NewCombinations.size()<<endl;

      // Find equal combinations and eliminate them from the new list
      for (unsigned int c = 0; c < Combinations[d][side].size(); ++c) {
        auto Iter = NewCombinations.begin();
        while (Iter != NewCombinations.end()) {
          if (Combinations[d][side][c] == (*Iter)) {
            bool Equal = true;
            for (unsigned int deep = 0; deep < Combinations[d][side][c].size(); ++deep) {
              if (Combinations[d][side][c][deep] != (*Iter)[deep]) {
                Equal = false;
                break;
              }
            }
            if (Equal == true) {
              Iter = NewCombinations.erase(Iter);
            } else {
              Iter++;
            }
          } else {
            Iter++;
          }
        }
      }
      // If there are new combinations left, add them, and restart
      if (NewCombinations.size() > 0) {
        //cout<<NewCombinations.size()<<" new combinations found"<<endl;
        for (auto C : NewCombinations) {
          Combinations[d][side].push_back(C);
        }
        CombinationsAdded = true; //keep going until no more combos added
      }
    }
  } // End Side loop

  return Combinations;
}

//! Evaluate the reduced chi square for all possible strip pairings
tuple<vector<vector<unsigned int>>, vector<vector<unsigned int>>, double> MModuleStripPairingMultiRoundChiSquare::EvaluateAllCombinations(unsigned int d, vector<vector<vector<vector<vector<unsigned int>>>>> Combinations, vector<vector<vector<MStripHit*>>> StripHits)
{

  double BestChiSquare = numeric_limits<double>::max();
  vector<vector<unsigned int>> BestLVSideCombo; // List of combined strips that comprise the best LV side combination
  vector<vector<unsigned int>> BestHVSideCombo; // List of combined strips that comprise the best HV side combination

  for (unsigned int lv = 0; lv < Combinations[d][0].size(); ++lv) { // Loop over combinations of lv-strips (lv represents a list of sets of strips,  and each set is a proposed Hit)
    for (unsigned int hv = 0; hv < Combinations[d][1].size(); ++hv) {
      // Skip if lv and hv strip combos differ in size by more than one
      if (abs(long(Combinations[d][0][lv].size()) - long(Combinations[d][1][hv].size())) > 1) {
        continue;
      }

      unsigned int MinSize = min(Combinations[d][0][lv].size(), Combinations[d][1][hv].size());

      // Skip pairing if either side has more than 5 sets of strips
      if (max(Combinations[d][0][lv].size(), Combinations[d][1][hv].size()) > MaxCombinations) {
        continue;
      }

      bool MorePermutations = true;
      while (MorePermutations == true) {

        double ChiSquare = 0;

        for (unsigned int en = 0; en < MinSize; ++en) { // en and ep are on the strip grouping level (ie if there's charge sharing)
          unsigned int ep = en;

          // Collect LV and HV strips for current hit pairing
          vector<vector<MStripHit*>> CurrentHitPairing; // List of combined strips that are being analyzed currently
          CurrentHitPairing.push_back(vector<MStripHit*>());
          CurrentHitPairing.push_back(vector<MStripHit*>());

          double LVEnergy = 0;
          double LVResolution = 0;

          // Add up LV energy and energy resolution for grouping of strips
          for (unsigned int entry = 0; entry < Combinations[d][0][lv][en].size(); ++entry) { // Entry is on the strip level
            LVEnergy += StripHits[d][0][Combinations[d][0][lv][en][entry]]->GetEnergy();
            LVResolution += pow(StripHits[d][0][Combinations[d][0][lv][en][entry]]->GetEnergyResolution(), 2);

            // Add strip to current hit pairing
            CurrentHitPairing[0].push_back(StripHits[d][0][Combinations[d][0][lv][en][entry]]);
          }

          // Repeats for HV side
          double HVEnergy = 0;
          double HVResolution = 0;

          for (unsigned int entry = 0; entry < Combinations[d][1][hv][ep].size(); ++entry) {
            HVEnergy += StripHits[d][1][Combinations[d][1][hv][ep][entry]]->GetEnergy();
            HVResolution += pow(StripHits[d][1][Combinations[d][1][hv][ep][entry]]->GetEnergyResolution(), 2);

            // Add strip to current hit pairing
            CurrentHitPairing[1].push_back(StripHits[d][1][Combinations[d][1][hv][ep][entry]]);
          }

          // Apply charge trapping correction for each LV/HV pairing
          // !!! TODO: Fill ChargeTrappingCorrection function with actual trapping parameters
          // Is it sufficient to only correct one side's energy or should a correction be applied to LV as well?
          HVEnergy += ChargeTrappingCorrection(d, CurrentHitPairing);

          // Sum chi square over entire LV/HV combination
          ChiSquare += (LVEnergy - HVEnergy) * (LVEnergy - HVEnergy) / (LVResolution + HVResolution);
        }

        // Calculate reduced chi^2 be dividing by number of strip groupings within the combination
        ChiSquare /= MinSize;

        if (ChiSquare < BestChiSquare) {
          BestChiSquare = ChiSquare;
          BestLVSideCombo = Combinations[d][0][lv];
          BestHVSideCombo = Combinations[d][1][hv];
        }

        // Cycle through all permutations to reach every possible strip pairing
        if (Combinations[d][1][hv].size() > Combinations[d][0][lv].size()) {
          MorePermutations = next_permutation(Combinations[d][1][hv].begin(), Combinations[d][1][hv].end());
        } else {
          MorePermutations = next_permutation(Combinations[d][0][lv].begin(), Combinations[d][0][lv].end());
        }
      }
    }
  }
  return { BestLVSideCombo, BestHVSideCombo, BestChiSquare };
}

////////////////////////////////////////////////////////////////////////////////

//! Create hits
bool MModuleStripPairingMultiRoundChiSquare::CreateHits(unsigned int d, MReadOutAssembly* Event, vector<vector<vector<MStripHit*>>> StripHits, vector<vector<unsigned int>> BestLVSideCombo, vector<vector<unsigned int>> BestHVSideCombo)
{


  double LVEnergy = 0;
  double LVEnergyRes = 0;

  double HVEnergy = 0;
  double HVEnergyRes = 0;

  double Energy = 0;
  double EnergyResolution = 0;

  // "Total" is across event
  double EnergyTotal = 0;
  double LVEnergyTotal = 0;
  double HVEnergyTotal = 0;
  double LVEnergyResTotal = 0;
  double HVEnergyResTotal = 0;

  // Create vectors for plotting LV and HV energies
  vector<double> LVEnergies;
  vector<double> HVEnergies;

  for (unsigned int h = 0; h < min(BestLVSideCombo.size(), BestHVSideCombo.size()); ++h) { // Loop over groupings of strips

    LVEnergy = 0;
    HVEnergy = 0;
    LVEnergyRes = 0;
    HVEnergyRes = 0;

    // Collect LV and HV strips for current hit pairing
    vector<vector<MStripHit*>> CurrentHitPairing; // List of combined strips that are being analyzed currently
    CurrentHitPairing.push_back(vector<MStripHit*>());
    CurrentHitPairing.push_back(vector<MStripHit*>());

    // Check if there are any non-adjacent groupings of strips
    bool AllAdjacentLV = true;
    bool AllAdjacentHV = true;
    bool AllAdjacent = true;

    for (unsigned int sh = 0; sh < BestLVSideCombo[h].size() - 1; ++sh) {
      if (StripHits[d][0][BestLVSideCombo[h][sh]]->GetStripID() + 1 != StripHits[d][0][BestLVSideCombo[h][sh + 1]]->GetStripID()) {
        AllAdjacentLV = false;
        AllAdjacent = false;
        break;
      }
    }

    for (unsigned int sh = 0; sh < BestHVSideCombo[h].size() - 1; ++sh) {
      if (StripHits[d][1][BestHVSideCombo[h][sh]]->GetStripID() + 1 != StripHits[d][1][BestHVSideCombo[h][sh + 1]]->GetStripID()) {
        AllAdjacentHV = false;
        AllAdjacent = false;
        break;
      }
    }

    // If there are no non-adjacent strip groupings, continue pairing as normal
    if (AllAdjacent) {

      // Add up energy and energy resolution for each grouping of strips
      for (unsigned int sh = 0; sh < BestLVSideCombo[h].size(); ++sh) {
        //cout<<"x-pos: "<<StripHits[d][0][BestXSideCombo[h][sh]]->GetNonStripPosition()<<endl;
        LVEnergy += StripHits[d][0][BestLVSideCombo[h][sh]]->GetEnergy();
        LVEnergyRes += StripHits[d][0][BestLVSideCombo[h][sh]]->GetEnergyResolution() * StripHits[d][0][BestLVSideCombo[h][sh]]->GetEnergyResolution();

        // Add strip to current hit pairing
        CurrentHitPairing[0].push_back(StripHits[d][0][BestLVSideCombo[h][sh]]);
      }

      LVEnergyResTotal += LVEnergyRes;
      LVEnergyTotal += LVEnergy;

      LVEnergyRes = sqrt(LVEnergyRes);

      for (unsigned int sh = 0; sh < BestHVSideCombo[h].size(); ++sh) {
        HVEnergy += StripHits[d][1][BestHVSideCombo[h][sh]]->GetEnergy();
        HVEnergyRes += StripHits[d][1][BestHVSideCombo[h][sh]]->GetEnergyResolution() * StripHits[d][1][BestHVSideCombo[h][sh]]->GetEnergyResolution();

        // Add strip to current hit pairing
        CurrentHitPairing[1].push_back(StripHits[d][1][BestHVSideCombo[h][sh]]);
      }

      // Apply charge trapping correction for each LV/HV pairing
      // !!! TODO: Fill ChargeTrappingCorrection function with actual trapping parameters
      // Is it sufficient to only correct one side's energy or should a correction be applied to LV as well?
      HVEnergy += ChargeTrappingCorrection(d, CurrentHitPairing);

      HVEnergyResTotal += HVEnergyRes;
      HVEnergyTotal += HVEnergy;

      HVEnergyRes = sqrt(HVEnergyRes);

      // Assign hit energy based on HV and LV energies
      Energy = 0.0;
      if (LVEnergy > HVEnergy + 3 * HVEnergyRes) {
        Energy = LVEnergy;
        EnergyResolution = LVEnergyRes;
      } else if (HVEnergy > LVEnergy + 3 * LVEnergyRes) {
        Energy = HVEnergy;
        EnergyResolution = HVEnergyRes;
      } else { // Take weighted average of LV and HV energies if one is not significantly higher than the other
        Energy = (LVEnergy / (LVEnergyRes * LVEnergyRes) + HVEnergy / (HVEnergyRes * HVEnergyRes)) / (1.0 / (LVEnergyRes * LVEnergyRes) + 1.0 / (HVEnergyRes * HVEnergyRes));
        EnergyResolution = sqrt(1.0 / (1.0 / (LVEnergyRes * LVEnergyRes) + 1.0 / (HVEnergyRes * HVEnergyRes)));
      }

      EnergyTotal += Energy;

      LVEnergies.push_back(LVEnergy);
      HVEnergies.push_back(HVEnergy);

      // Populate event with hits
      MHit* Hit = new MHit();
      Hit->SetEnergy(Energy);
      Hit->SetLVEnergy(LVEnergy);
      Hit->SetHVEnergy(HVEnergy);
      Hit->SetEnergyResolution(EnergyResolution);
      Event->AddHit(Hit);

      // Populate hit with strip hits
      for (unsigned int sh = 0; sh < BestLVSideCombo[h].size(); ++sh) {
        Hit->AddStripHit(StripHits[d][0][BestLVSideCombo[h][sh]]);
      }
      for (unsigned int sh = 0; sh < BestHVSideCombo[h].size(); ++sh) {
        Hit->AddStripHit(StripHits[d][1][BestHVSideCombo[h][sh]]);
      }
    }

    // If there are non-adjacent strip groupings, then have to separate them out again to form multiple (physical) hits
    // Multiple hits on LV side

    // !!! TODO: Figure out how to apply charge trapping correction to events with multiple hits on a single strip
    if (AllAdjacentHV == false && AllAdjacentLV == true) {
      //cout<<"Multiple hits on single LV strip"<<endl;
      bool MultipleHitsOnLV = true;

      // Assign hit energy based on energy measured on HV side
      for (unsigned int sh = 0; sh < BestHVSideCombo[h].size(); ++sh) {
        Energy = StripHits[d][1][BestHVSideCombo[h][sh]]->GetEnergy();
        EnergyTotal += Energy;
        EnergyResolution = StripHits[d][1][BestHVSideCombo[h][sh]]->GetEnergyResolution();

        HVEnergy = Energy;
        LVEnergy = Energy;
        LVEnergyRes = EnergyResolution;
        HVEnergyRes = EnergyResolution;
        LVEnergyResTotal += LVEnergyRes * LVEnergyRes;
        HVEnergyResTotal += HVEnergyRes * HVEnergyRes;

        // Populate event with hits
        MHit* Hit = new MHit();
        Hit->SetEnergy(Energy);
        Hit->SetEnergyResolution(EnergyResolution);
        Hit->SetLVEnergy(LVEnergy);
        Hit->SetHVEnergy(HVEnergy);
        Hit->SetStripHitMultipleTimesX(MultipleHitsOnLV);
        Event->AddHit(Hit);
          

        HVEnergyTotal += HVEnergy;
        LVEnergyTotal += LVEnergy;

        LVEnergies.push_back(LVEnergy);
        HVEnergies.push_back(HVEnergy);

        // Populate hit with strip hits
        Hit->AddStripHit(StripHits[d][1][BestHVSideCombo[h][sh]]);
        for (unsigned int sh = 0; sh < BestLVSideCombo[h].size(); ++sh) {
          Hit->AddStripHit(StripHits[d][0][BestLVSideCombo[h][sh]]);
        }
      }
    }

    // Multiple hits on HV side
    else if (AllAdjacentLV == false && AllAdjacentHV == true) {
      // cout<<"Multiple hits on single HV strip"<<endl;
      bool MultipleHitsOnHV = true;

      // Assign hit energy based on energy measured on LV side
      for (unsigned int sh = 0; sh < BestLVSideCombo[h].size(); ++sh) {
        Energy = StripHits[d][0][BestLVSideCombo[h][sh]]->GetEnergy();
        EnergyTotal += Energy;
        EnergyResolution = StripHits[d][0][BestLVSideCombo[h][sh]]->GetEnergyResolution();

        HVEnergy = Energy;
        LVEnergy = Energy;
        LVEnergyRes = EnergyResolution;
        HVEnergyRes = EnergyResolution;
        LVEnergyResTotal += LVEnergyRes * LVEnergyRes;
        HVEnergyResTotal += HVEnergyRes * HVEnergyRes;

        // Populate event with hits
        MHit* Hit = new MHit();
        Hit->SetEnergy(Energy);
        Hit->SetEnergyResolution(EnergyResolution);
        Hit->SetLVEnergy(LVEnergy);
        Hit->SetHVEnergy(HVEnergy);
        Hit->SetStripHitMultipleTimesY(MultipleHitsOnHV);
        Event->AddHit(Hit);

        HVEnergyTotal += HVEnergy;
        LVEnergyTotal += LVEnergy;
        LVEnergies.push_back(LVEnergy);
        HVEnergies.push_back(HVEnergy);

        // Populate hit with strip hits
        Hit->AddStripHit(StripHits[d][0][BestLVSideCombo[h][sh]]);
        for (unsigned int sh = 0; sh < BestHVSideCombo[h].size(); ++sh) {
          Hit->AddStripHit(StripHits[d][1][BestHVSideCombo[h][sh]]);
        }
      }
    }

    // If both HV and LV have multiple hits per strip, can't pair
    else if (AllAdjacentLV == false and AllAdjacentHV == false) {
      Event->SetStripPairingIncomplete(true, "Strips not pairable. Multiple hits per strip on LV and HV sides");
      Event->SetAnalysisProgress(MAssembly::c_StripPairing);
      return false;
    }
  }

  LVEnergyResTotal = sqrt(LVEnergyResTotal);
  HVEnergyResTotal = sqrt(HVEnergyResTotal);

  // One last quality selection based on total event energies
  if ((EnergyTotal > max(LVEnergyTotal, HVEnergyTotal) + 2.5 * max(LVEnergyResTotal, HVEnergyResTotal) || EnergyTotal < min(LVEnergyTotal, HVEnergyTotal) - 2.5 * max(LVEnergyResTotal, HVEnergyResTotal))) {
    Event->SetStripPairingIncomplete(true, "Strips not pairable wihin 2.5 sigma of measured energy");
    Event->SetAnalysisProgress(MAssembly::c_StripPairing);
    return false;
  }
  // Plot the good events
  else if ((HasExpos() == true) and Event->IsGood() == true) {
    m_ExpoStripPairingHits->AddHits(Event->GetNHits());
    for (unsigned int i = 0; i < LVEnergies.size(); ++i) {
      m_ExpoStripPairing->AddEnergies(LVEnergies[i], HVEnergies[i]);
    }
    for (unsigned int h = 0; h < Event->GetNHits(); h++) {
      double HVStrips = 0;
      double LVStrips = 0;
      for (unsigned int sh = 0; sh < Event->GetHit(h)->GetNStripHits(); sh++) {
        if (Event->GetHit(h)->GetStripHit(sh)->IsLowVoltageStrip() == true) {
          LVStrips++;
        } else {
          HVStrips++;
        }
      }
      m_ExpoStripPairingStripHits->AddStripHits(LVStrips, HVStrips);
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

//! Main data analysis routine, which updates the event to a new level
bool MModuleStripPairingMultiRoundChiSquare::AnalyzeEvent(MReadOutAssembly* Event)
{

  // Check if there are actually any strip hits
  if (Event->GetNStripHits() == 0) {
    Event->SetStripPairingIncomplete(true, "No strip hits");
    Event->SetAnalysisProgress(MAssembly::c_StripPairing);
    return false;
  }

  // Collect strip hits from input event
  vector<vector<vector<MStripHit*>>> StripHits = CollectStripHits(Event); // List of detectors, list of sides, list of strip hits

  // Perform some event selections
  bool CheckStripHits = EventSelection(Event, StripHits);

  if (CheckStripHits == false) {
    return false;
  }

  // Find seed combinations from which all strip combinations will be computed

  // Define Combinations as list of: detector IDs, sides, strip combinations, set of grouped strips (ex. charge sharing on adjacent strip), strip
  vector<vector<vector<vector<vector<unsigned int>>>>> Combinations;

  for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
    Combinations.push_back(vector<vector<vector<vector<unsigned int>>>>()); // Create 4D vectors for each detector
    Combinations[d].push_back(vector<vector<vector<unsigned int>>>()); // Create 3D vectors within each detector vector for each side
    Combinations[d].push_back(vector<vector<vector<unsigned int>>>());

    // Create the seed combinations that will be added (and expanded upon) for each side of each detector
    for (unsigned int s = 0; s <= 1; ++s) {
      vector<vector<unsigned int>> Combination;
      for (unsigned int sh = 0; sh < StripHits[d][s].size(); ++sh) {
        vector<unsigned int> CombinedStrips = { sh }; // Use grouping of single strip hit as seed for subsequent groupings
        // sort(CombinedStrips.begin(), CombinedStrips.end());
        Combination.push_back(CombinedStrips);
      }
      Combinations[d][s].push_back(Combination);
    }
  }

  for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop

    // Begin the first round of strip pairing
    bool RoundTwo = false;

    // Find all possible combinations based on the above seed combination
    Combinations = FindAllCombinations(d, Combinations, StripHits, RoundTwo);

    // Evaluate reduced chi square for all combinations and select best LV/HV combinations
    auto [BestLVSideCombo, BestHVSideCombo, BestChiSquare] = EvaluateAllCombinations(d, Combinations, StripHits);

    if (BestChiSquare > ChiSquareThreshold) {
      RoundTwo = true;

      // Repeat strip pairing, now allowing groupings of non adjacent strips
      Combinations = FindAllCombinations(d, Combinations, StripHits, RoundTwo);
      auto [BestLVSideComboRoundTwo, BestHVSideComboRoundTwo, BestChiSquareRoundTwo] = EvaluateAllCombinations(d, Combinations, StripHits);

      // Update best LV/HV combos if a better pairing is found
      if (BestChiSquareRoundTwo < BestChiSquare) {
        BestChiSquare = BestChiSquareRoundTwo;
        BestLVSideCombo = BestLVSideComboRoundTwo;
        BestHVSideCombo = BestHVSideComboRoundTwo;
      }
    }
    // Check if chi^2 was ever actually updated
    if (BestChiSquare == numeric_limits<double>::max()) {
      Event->SetStripPairingIncomplete(true, "Pairing did not find a single match");
      Event->SetAnalysisProgress(MAssembly::c_StripPairing);
      return false;
    }
    // Flag events with a reduced chi square > 25
    else if (BestChiSquare > 25) {
      Event->SetStripPairingIncomplete(true, "Best reduced chi square is not below 25");
      Event->SetAnalysisProgress(MAssembly::c_StripPairing);
      return false;
    }

    // Assign the best reduced chi square to the event
    Event->SetReducedChiSquare(BestChiSquare);

    // Populate hits with best strip paired combination
    bool PopulateHits = CreateHits(d, Event, StripHits, BestLVSideCombo, BestHVSideCombo);

    if (PopulateHits == false) {
      return false;
    }

  } // End Detector loop

  Event->SetAnalysisProgress(MAssembly::c_StripPairing);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


vector<size_t> MModuleStripPairingMultiRoundChiSquare::Argsort(vector<double>& list)
{
  // Return the order of indices resulting from list sorting
  // initialize original index locations
  vector<size_t> idx(list.size());
  iota(idx.begin(), idx.end(), 0);

  // sort indexes based on comparing values in v
  // using std::stable_sort instead of std::sort
  // to avoid unnecessary index re-orderings
  // when v contains elements of equal values
  stable_sort(idx.begin(), idx.end(),
              [&list](size_t i1, size_t i2) { return list[i1] < list[i2]; });

  return idx;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleStripPairingMultiRoundChiSquare::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize()

  MModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


void MModuleStripPairingMultiRoundChiSquare::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsStripPairing* Options = new MGUIOptionsStripPairing(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleStripPairingMultiRoundChiSquare::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  /*
  MXmlNode* SomeTagNode = Node->GetNode("SomeTag");
  if (SomeTagNode != 0) {
    m_SomeTagValue = SomeTagNode->GetValue();
  }
  */

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleStripPairingMultiRoundChiSquare::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);

  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MModuleStripPairingMultiRoundChiSquare.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
