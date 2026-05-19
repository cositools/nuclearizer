/*
 * MModuleStripPairingChiSquare.cxx
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
// MModuleStripPairingChiSquare
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleStripPairingChiSquare.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MModule.h"
#include "MGUIOptionsStripPairing.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleStripPairingChiSquare)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleStripPairingChiSquare::MModuleStripPairingChiSquare() : MModule()
{
  // Construct an instance of MModuleStripPairingChiSquare

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Strip pairing - chi square approach";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagStripPairingChiSquare";

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


MModuleStripPairingChiSquare::~MModuleStripPairingChiSquare()
{
  // Delete this instance of MModuleStripPairingChiSquare
}


////////////////////////////////////////////////////////////////////////////////


void MModuleStripPairingChiSquare::CreateExpos()
{
  // Create all expos

  if (HasExpos() == true) return;

  // Set the histogram display
  m_ExpoStripPairing = new MGUIExpoStripPairing(this);
  m_ExpoStripPairing->SetEnergiesHistogramParameters(1500, 0, 1500);
  m_Expos.push_back(m_ExpoStripPairing);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleStripPairingChiSquare::Initialize()
{
  // Initialize the module 

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


vector<vector<vector<unsigned int>>> MModuleStripPairingChiSquare::FindNewCombinations(vector<vector<vector<unsigned int>>> OldOnes, vector<MStripHit*> StripHits)
{
  vector<vector<vector<unsigned int>>> NewOnes; // <list> of <combinations> of <combined strips>

  for (unsigned int listspot = 0; listspot < OldOnes.size(); ++listspot) {
    // New single merges
    for (unsigned int combi1 = 0; combi1 < OldOnes[listspot].size(); ++combi1) {
      for (unsigned int combi2 = combi1+1; combi2 < OldOnes[listspot].size(); ++combi2) {
        vector<unsigned int> NewCombinedStrips;
        NewCombinedStrips.insert(NewCombinedStrips.end(), OldOnes[listspot][combi1].begin(), OldOnes[listspot][combi1].end());
        NewCombinedStrips.insert(NewCombinedStrips.end(), OldOnes[listspot][combi2].begin(), OldOnes[listspot][combi2].end());
        sort(NewCombinedStrips.begin(), NewCombinedStrips.end());

        vector<unsigned int> NewCombinedAsIDs;
        for (unsigned int s = 0; s < NewCombinedStrips.size(); ++s) {
          NewCombinedAsIDs.push_back(StripHits[NewCombinedStrips[s]]->GetStripID());
        }
        sort(NewCombinedAsIDs.begin(), NewCombinedAsIDs.end());

        bool AllAdjacent = true;
        for (unsigned int c = 1; c < NewCombinedAsIDs.size(); ++c) {
          if (NewCombinedAsIDs[c-1] + 1 != NewCombinedAsIDs[c]) {
            AllAdjacent = false;
            break;
          }
        }

        if (AllAdjacent == true) {
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


bool MModuleStripPairingChiSquare::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level 

  mdebug<<"StripPairing started"<<endl;

  unsigned int MaxCombinations = 5;

  if (Event->GetNStripHits() == 0) {
    Event->SetStripPairingIncomplete(true, "No strip hits");
    Event->SetAnalysisProgress(MAssembly::c_StripPairing);
    return false;
  }

  // (1) Split hits by detector ID
  vector<unsigned int> DetectorIDs;
  vector<vector<vector<MStripHit*>>> StripHits; // list of detector ID, list of sides, list of hits

  for (unsigned int sh = 0; sh < Event->GetNStripHits(); ++sh) {
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

    if (DetectorFound == true) {
      StripHits[DetectorPos][Side].push_back(SH);
    } else {
      vector<vector<MStripHit*>> List; // list of sides, list of hits
      List.push_back(vector<MStripHit*>()); // X
      List.push_back(vector<MStripHit*>()); // Y
      List[Side].push_back(SH);
      StripHits.push_back(List);
      DetectorIDs.push_back(SH->GetDetectorID());
    }
  }

  // Limit the strip hits
  const unsigned int MaxStripHits = 6;
  for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
    for (unsigned int side = 0; side <=1; ++side) { // side loop
      if (StripHits[d][side].size() > MaxStripHits) {
        Event->SetStripPairingIncomplete(true, "More than 6 hit strIps on one side");
        Event->SetAnalysisProgress(MAssembly::c_StripPairing);
        return false;
      }
    }
  }


      /*
      cout<<"Strip hits: "<<endl;
      for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
        cout<<"Detector: "<<d<<endl;
        for (unsigned int side = 0; side <=1; ++side) { // side loop
          cout<<"  Side: "<<side<<endl;
          if (StripHits[d][side].size() > 0) {
            cout<<"    Hits: "<<endl;;
            for (unsigned int sh = 0; sh < StripHits[d][side].size(); ++sh) { // side loop
              cout<<StripHits[d][side][sh]->ToString();
            }
          } else {
            cout<<"no hits"<<endl;
          }
        }
      }
      */

  // (2) Check if we have enough strips and enough energy for each detector
  for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop

    if (StripHits[d][0].size() == 0 || StripHits[d][1].size() == 0) {
      Event->SetStripPairingIncomplete(true, "One detector side has not strip hits");
      Event->SetAnalysisProgress(MAssembly::c_StripPairing);
      return false;
    }

    double xEnergy = 0;
    double xEnergyRes = 0;
    for (unsigned int sh = 0; sh < StripHits[d][0].size(); ++sh) {
      xEnergy += StripHits[d][0][sh]->GetEnergy();
      xEnergyRes += StripHits[d][0][sh]->GetEnergyResolution()*StripHits[d][0][sh]->GetEnergyResolution();
    }
    double yEnergy = 0;
    double yEnergyRes = 0;
    for (unsigned int sh = 0; sh < StripHits[d][1].size(); ++sh) {
      yEnergy += StripHits[d][1][sh]->GetEnergy();
      yEnergyRes += StripHits[d][1][sh]->GetEnergyResolution()*StripHits[d][1][sh]->GetEnergyResolution();
    }

    //cout<<"Energies: "<<xEnergy<<":"<<xEnergyRes<<" -- "<<yEnergy<<":"<<yEnergyRes<<endl;
  }



  // (3) Find all possible combinations
  vector<vector<vector<vector<vector<unsigned int>>>>> Combinations;  // list of detector IDs, list of sides, list of combinations; combination with list of combined strips
  for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
    Combinations.push_back(vector<vector<vector<vector<unsigned int>>>>());
    for (unsigned int s = 0; s <= 1 ; ++s) {
      Combinations[d].push_back(vector<vector<vector<unsigned int>>>()); // X
      Combinations[d].push_back(vector<vector<vector<unsigned int>>>()); // Y
    }
    // Create the seed combinations
    for (unsigned int s = 0; s < StripHits[d].size(); ++s) {
      vector<vector<unsigned int>> Combination;
      for (unsigned int h = 0; h < StripHits[d][s].size(); ++h) {
        vector<unsigned int> CombinedStrips = { h };
        sort(CombinedStrips.begin(), CombinedStrips.end());
        Combination.push_back(CombinedStrips);
      }
      Combinations[d][s].push_back(Combination);
    }
  }

  // Starting from this seed, find more new combinations
  for (unsigned int d = 0; d < StripHits.size(); ++d) { // Detector loop
    for (unsigned int side = 0; side <=1; ++side) { // side loop

      vector<vector<vector<unsigned int>>> NewCombinations;

      bool CombinationsAdded = true;
      while (CombinationsAdded == true) {
        CombinationsAdded = false;

        NewCombinations = FindNewCombinations(Combinations[d][side], StripHits[d][side]);
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
          for (auto C: NewCombinations) {
            Combinations[d][side].push_back(C);
          }
          CombinationsAdded = true;
        }
      } // combination search
    } // side loop

    /*
    cout<<"All combinations:"<<endl;
    for (unsigned int xc = 0; xc < Combinations[d][0].size(); ++xc) {
      cout<<"X - "<<xc<<": ";
      for (unsigned h = 0; h < Combinations[d][0][xc].size(); ++h) {
        cout<<" (";
        for (unsigned int sh = 0; sh < Combinations[d][0][xc][h].size(); ++sh) {
          cout<<Combinations[d][0][xc][h][sh]<<" ";
        }
        cout<<")";
      }
      cout<<endl;
    }
    for (unsigned int yc = 0; yc < Combinations[d][1].size(); ++yc) {
      cout<<"Y - "<<yc<<": ";
      for (unsigned h = 0; h < Combinations[d][1][yc].size(); ++h) {
        cout<<" (";
        for (unsigned int sh = 0; sh < Combinations[d][1][yc][h].size(); ++sh) {
          cout<<Combinations[d][1][yc][h][sh]<<" ";
        }
        cout<<")";
      }
      cout<<endl;
    }
    */

    // (3) Evaluate all combinations
    // All strip combinations for one side have been found, now check for the best x-y combinations
    double BestChiSquare = numeric_limits<double>::max();
    vector<vector<unsigned int>> BestXSideCombo;
    vector<vector<unsigned int>> BestYSideCombo;

    for (unsigned int xc = 0; xc < Combinations[d][0].size(); ++xc) {
      for (unsigned int yc = 0; yc < Combinations[d][1].size(); ++yc) {

        if (abs(long(Combinations[d][0][xc].size()) - long(Combinations[d][1][yc].size())) > 1) {
          continue;
        }

        unsigned int MinSize = min(Combinations[d][0][xc].size(), Combinations[d][1][yc].size());

        if (max(Combinations[d][0][xc].size(), Combinations[d][1][yc].size()) > MaxCombinations) {
          continue;
        }

        bool MorePermutations = true;
        while (MorePermutations == true) {
          //cout<<"New permutation..."<<endl;
          //         if (Combinations[d][1][yc].size() > Combinations[d][0][xc].size()) {
          //           PrintCombi(Combinations[d][1][yc]);
          //         } else {
          //           PrintCombi(NCombi[p]);
          //         }
          double ChiSquare = 0;

          for (unsigned int en = 0; en < MinSize; ++en) {
            unsigned int ep = en;

            double xEnergy = 0;
            double xResolution = 0;
            for (unsigned int entry = 0; entry < Combinations[d][0][xc][en].size(); ++entry) {
              xEnergy += StripHits[d][0][Combinations[d][0][xc][en][entry]]->GetEnergy();
              xResolution += pow(StripHits[d][0][Combinations[d][0][xc][en][entry]]->GetEnergyResolution(), 2);
            }

            double yEnergy = 0;
            double yResolution = 0;
            for (unsigned int entry = 0; entry < Combinations[d][1][yc][ep].size(); ++entry) {
              yEnergy += StripHits[d][1][Combinations[d][1][yc][ep][entry]]->GetEnergy();
              yResolution += pow(StripHits[d][1][Combinations[d][1][yc][ep][entry]]->GetEnergyResolution(), 2);
            }
            //cout << "yEnergy: " << yEnergy << endl;
            //cout << "  Sub - Test en=" << en << " (" << xEnergy << ") with ep="
            //     << ep << " (" << yEnergy << "):" << endl;
            //cout<<xResolution<<":"<<yResolution<<endl;
            ChiSquare += (xEnergy - yEnergy)*(xEnergy - yEnergy) / (xResolution + yResolution);
          }
          ChiSquare /= MinSize;
          //cout<<"Chi square: "<<ChiSquare<<endl;

          if (ChiSquare < BestChiSquare) {
            BestChiSquare = ChiSquare;
            BestXSideCombo = Combinations[d][0][xc];
            BestYSideCombo = Combinations[d][1][yc];
          }

          //cout<<"ChiSquare: "<<ChiSquare<<endl;

          if (Combinations[d][1][yc].size() > Combinations[d][0][xc].size()) {
            MorePermutations = next_permutation(Combinations[d][1][yc].begin(), Combinations[d][1][yc].end());
          } else {
            MorePermutations = next_permutation(Combinations[d][0][xc].begin(), Combinations[d][0][xc].end());
          }
        }
      }
    }

    /*
    cout<<"Best combo:"<<endl;
    cout<<"X: ";
    for (unsigned h = 0; h < BestXSideCombo.size(); ++h) {
      cout<<" (";
      for (unsigned int sh = 0; sh < BestXSideCombo[h].size(); ++sh) {
        cout<<BestXSideCombo[h][sh]<<" ";
      }
      cout<<")";
    }
    cout<<endl;
    cout<<"Y: ";
    for (unsigned h = 0; h < BestYSideCombo.size(); ++h) {
      cout<<" (";
      for (unsigned int sh = 0; sh < BestYSideCombo[h].size(); ++sh) {
        cout<<BestYSideCombo[h][sh]<<" ";
      }
      cout<<")";
    }
    cout<<endl;
    */

    // Now create hits:
    if (BestChiSquare == numeric_limits<double>::max()) {
      Event->SetStripPairingIncomplete(true, "Pairing did not find a single match");
      Event->SetAnalysisProgress(MAssembly::c_StripPairing);
      return false;
    }

    // Create the hits
    double XPos = 0;
    double YPos = 0;
    double XEnergy = 0;
    double XEnergyRes = 0;
    double YEnergy = 0;
    double YEnergyRes = 0;
    double Energy = 0;
    double EnergyResolution = 0;

    double XEnergyTotal = 0;
    double YEnergyTotal = 0;
    double EnergyTotal = 0;

    for (unsigned int h = 0; h < min(BestXSideCombo.size(), BestYSideCombo.size()); ++h) {
      XPos = 0;
      YPos = 0;
      XEnergy = 0;
      YEnergy = 0;

      for (unsigned int sh = 0; sh < BestXSideCombo[h].size(); ++sh) {
        //cout<<"x-pos: "<<StripHits[d][0][BestXSideCombo[h][sh]]->GetNonStripPosition()<<endl;
        XEnergy += StripHits[d][0][BestXSideCombo[h][sh]]->GetEnergy();
        XEnergyRes += StripHits[d][0][BestXSideCombo[h][sh]]->GetEnergyResolution()*StripHits[d][0][BestXSideCombo[h][sh]]->GetEnergyResolution();
      }
      XEnergyRes = sqrt(XEnergyRes);
      XEnergyTotal += XEnergy;

      for (unsigned int sh = 0; sh < BestYSideCombo[h].size(); ++sh) {
        YEnergy += StripHits[d][1][BestYSideCombo[h][sh]]->GetEnergy();
        YEnergyRes += StripHits[d][1][BestYSideCombo[h][sh]]->GetEnergyResolution()*StripHits[d][1][BestYSideCombo[h][sh]]->GetEnergyResolution();
      }
      YEnergyRes = sqrt(YEnergyRes);
      YEnergyTotal += YEnergy;

      Energy = 0.0;
      if (XEnergy > YEnergy + 3*YEnergyRes) {
        Energy = XEnergy;
        EnergyResolution = XEnergyRes;
      } else if (YEnergy > XEnergy + 3*XEnergyRes) {
        Energy = YEnergy;
        EnergyResolution = YEnergyRes;
      } else {
        Energy = (XEnergy/(XEnergyRes*XEnergyRes) + YEnergy/(YEnergyRes*YEnergyRes)) / (1.0/(XEnergyRes*XEnergyRes) + 1.0/(YEnergyRes*YEnergyRes));
        EnergyResolution = sqrt( 1.0 / (1.0/(XEnergyRes*XEnergyRes) + 1.0/(YEnergyRes*YEnergyRes)) );
      }
      EnergyTotal += Energy;

      if (HasExpos() == true) {
        m_ExpoStripPairing->AddEnergies(XEnergy, YEnergy);
      }

      MHit* Hit = new MHit();
      Hit->SetEnergy(Energy);
      Hit->SetEnergyResolution(EnergyResolution);
      Event->AddHit(Hit);
      for (unsigned int sh = 0; sh < BestXSideCombo[h].size(); ++sh) {
        Hit->AddStripHit(StripHits[d][0][BestXSideCombo[h][sh]]);
      }
      for (unsigned int sh = 0; sh < BestYSideCombo[h].size(); ++sh) {
        Hit->AddStripHit(StripHits[d][1][BestYSideCombo[h][sh]]);
      }
    }

    if (EnergyTotal > max(XEnergyTotal, YEnergyTotal) + 2.5*max(XEnergyRes, YEnergyRes) || EnergyTotal < min(XEnergyTotal, YEnergyTotal) - 2.5*max(XEnergyRes, YEnergyRes)) {
      Event->SetStripPairingIncomplete(true, "Strips not pairable wihin 2.5 sigma of measure denergy");
      Event->SetAnalysisProgress(MAssembly::c_StripPairing);
      return false;
    }

    //

  } // detector loop

  Event->SetAnalysisProgress(MAssembly::c_StripPairing);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleStripPairingChiSquare::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


void MModuleStripPairingChiSquare::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsStripPairing* Options = new MGUIOptionsStripPairing(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleStripPairingChiSquare::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MModuleStripPairingChiSquare::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MModuleStripPairingChiSquare.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
