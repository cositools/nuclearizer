/*
 * MSubModuleChargeLoss.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * This code implementation is the intellectual property of Andreas Zoglauer.
 * By copying, distributing or modifying the Program (or any work based on the
 * Program) you indicate your acceptance of this statement, and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MSubModuleChargeLoss
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSubModuleChargeLoss.h"

// Standard libs:
#include <cmath>

// MEGAlib libs:
#include "MStreams.h"
#include "MFile.h"
#include "MReadOutAssembly.h"
#include "MDEEStripHit.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSubModuleChargeLoss)
#endif


////////////////////////////////////////////////////////////////////////////////


MSubModuleChargeLoss::MSubModuleChargeLoss() : MSubModule()
{
  // Construct an instance of MSubModuleChargeLoss
  m_ChargeLossFileName = "";
  m_ChargeLossCounter = 0;
}


////////////////////////////////////////////////////////////////////////////////


MSubModuleChargeLoss::~MSubModuleChargeLoss()
{
  // Delete this instance of MSubModuleChargeLoss
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeLoss::Initialize()
{
  // Read the charge-loss CSV into the per-detector/side grids.
  // CSV columns: detector, side, depth_cm, energy_keV, boundary_keV, edge_a, core_a
  // Grouped by detector/side, then by depth, then ascending energy (same energy grid per depth).

  m_ChargeLossCounter = 0;

  MFile chargeLossFile;
  if (chargeLossFile.Open(m_ChargeLossFileName) == false){
    cout << "MSubModuleChargeLoss: unable to open charge loss file: " << m_ChargeLossFileName << endl;
    return false;
  }

  // remember the current depth per det/side so we start a new depth row when it changes
  double chargeLossCurrentDepth[nDets][nSides];
  bool chargeLossHaveDepth[nDets][nSides];
  for (int det=0; det<nDets; det++){
    for (int side=0; side<nSides; side++){
      chargeLossHaveDepth[det][side] = false;
    }
  }

  MString chargeLossLine;
  while (chargeLossFile.ReadLine(chargeLossLine) == true){
    if (chargeLossLine.IsEmpty() == true || chargeLossLine.BeginsWith('#') == true){
      continue;
    }

    vector<MString> chargeLossTokens = chargeLossLine.Tokenize(",");
    if (chargeLossTokens.size() < 6){
      continue;
    }

    int det = (int) chargeLossTokens[0].Strip().ToDouble();
    int side = (int) chargeLossTokens[1].Strip().ToDouble();
    double depth = chargeLossTokens[2].Strip().ToDouble();
    double energy = chargeLossTokens[3].Strip().ToDouble();
    double boundary = chargeLossTokens[4].Strip().ToDouble();
    double edgeA = chargeLossTokens[5].Strip().ToDouble();
    double coreA = 0;   // core_a is blank until calibrated -> 0
    if (chargeLossTokens.size() >= 7 && chargeLossTokens[6].Strip().IsEmpty() == false){
      coreA = chargeLossTokens[6].Strip().ToDouble();
    }

    if (det < 0 || det >= nDets || side < 0 || side >= nSides){
      continue;
    }

    // new depth value -> start a new depth row for this det/side
    if (chargeLossHaveDepth[det][side] == false || depth != chargeLossCurrentDepth[det][side]){
      m_ChargeLossDepth[det][side].push_back(depth);
      m_ChargeLossEdgeA[det][side].push_back(vector<double>());
      m_ChargeLossCoreA[det][side].push_back(vector<double>());
      chargeLossCurrentDepth[det][side] = depth;
      chargeLossHaveDepth[det][side] = true;
    }

    // energy grid and boundary are energy-only: record them once, on the first depth row
    if (m_ChargeLossDepth[det][side].size() == 1){
      m_ChargeLossEnergy[det][side].push_back(energy);
      m_ChargeLossBoundary[det][side].push_back(boundary);
    }

    // append this energy point to the current (last) depth row
    m_ChargeLossEdgeA[det][side].back().push_back(edgeA);
    m_ChargeLossCoreA[det][side].back().push_back(coreA);
  }

  chargeLossFile.Close();

  return MSubModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleChargeLoss::Clear()
{
  // Clear event data from the module (no per-event state here)

  MSubModule::Clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeLoss::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Apply charge loss to adjacent same-side strips that share an interaction origin.
  // The two strip-hit lists (HV and LV) are handled the same way; the side index is
  // taken from each hit (0 = high voltage, 1 = low voltage) so the right CSV curve is used.

  list<MDEEStripHit>* sideLists[2];
  sideLists[0] = &Event->GetDEEStripHitHVListReference();
  sideLists[1] = &Event->GetDEEStripHitLVListReference();

  for (int listIndex = 0; listIndex < 2; listIndex++){
    list<MDEEStripHit>& hits = *sideLists[listIndex];

    for (list<MDEEStripHit>::iterator hitA = hits.begin(); hitA != hits.end(); ++hitA){
      list<MDEEStripHit>::iterator hitB = hitA;
      ++hitB;
      for ( ; hitB != hits.end(); ++hitB){

        // same detector?
        int det = hitA->m_ROE.GetDetectorID();
        if (det != hitB->m_ROE.GetDetectorID()){
          continue;
        }

        // adjacent strips?
        int stripOne = hitA->m_ROE.GetStripID();
        int stripTwo = hitB->m_ROE.GetStripID();
        if (abs(stripOne - stripTwo) != 1){
          continue;
        }

        // TODO: @RAnthonypetersen check with Felix is this is how we want to apply charge loss -- since it's different from how we'll correct for it
        // do the two strips share an interaction origin?
        bool sharedOrigin = false;
        for (int originOne: hitA->m_SimulatedOrigins){
          for (int originTwo: hitB->m_SimulatedOrigins){
            if (originOne == originTwo){
              sharedOrigin = true; break;
            }
          }
          if (sharedOrigin == true){
            break;
          }
        }
        
        if (sharedOrigin == false){
          continue;
        }

        int side = hitA->m_ROE.IsLowVoltageStrip() ? 0 : 1;
        if (det < 0 || det >= nDets || side < 0 || side >= nSides){
          continue;
        }
        
        // no calibration curve loaded for this detector/side -> skip (no correction)
        if (m_ChargeLossEnergy[det][side].size() == 0){
          continue;
        }

        double energyOne = hitA->m_Energy;
        double energyB = hitB->m_Energy;
        double trueSum = energyOne + energyTwo;
        double diff = fabs(energyOne - energyTwo);
        // one depth per pair (relative depth; a single CSV depth slice makes this a no-op for now)
        double depth = 0.5 * (hitA->m_SimulatedRelativeDepth + hitB->m_SimulatedRelativeDepth);

        // boundary (energy only) and the parabola coefficient a at this energy and depth:
        // edge_a in the edge (|diff| >= boundary), core_a in the core (|diff| < boundary)
        double boundary = ChargeLossInterp1D(trueSum, m_ChargeLossEnergy[det][side], m_ChargeLossBoundary[det][side]);

        double chargeLossA;
        if (diff >= boundary){
          chargeLossA = ChargeLossInterp2D(trueSum, depth, m_ChargeLossEnergy[det][side], m_ChargeLossDepth[det][side], m_ChargeLossEdgeA[det][side]);
        }
        else {
          chargeLossA = ChargeLossInterp2D(trueSum, depth, m_ChargeLossEnergy[det][side], m_ChargeLossDepth[det][side], m_ChargeLossCoreA[det][side]);
        }

        // loss is zero when one strip holds all the energy (diff = trueSum), deepest at an even split
        double loss = chargeLossA * (trueSum*trueSum - diff*diff);
        if (loss < 0){
          loss = 0;
        }

        if (loss > 0){
          // subtract the same amount from each strip
          // TODO: @RAnthonypetersen, right now it's an even split, but is this true? Or is it proportional to charge?
          hitA->m_Energy = energyA - loss/2.0;
          hitB->m_Energy = energyTwo - loss/2.0;
          m_ChargeLossCounter++;
        }
      }
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSubModuleChargeLoss::Finalize()
{
  // Finalize the module

  cout << "MSubModuleChargeLoss: charge-loss corrections applied to " << m_ChargeLossCounter << " strip pairs" << endl;

  MSubModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MSubModuleChargeLoss::ReadXmlConfiguration(MXmlNode* Node)
{
  // Read the configuration data from an XML node

  MXmlNode* N = Node->GetNode("ChargeLossFileName");
  if (N != nullptr) {
    m_ChargeLossFileName = N->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MSubModuleChargeLoss::CreateXmlConfiguration(MXmlNode* Node)
{
  // Create an XML node tree from the configuration

  new MXmlNode(Node, "ChargeLossFileName", m_ChargeLossFileName);

  return Node;
}


////////////////////////////////////////////////////////////////////////////////


double MSubModuleChargeLoss::ChargeLossInterp1D(double chargeLossEnergy,
    const vector<double>& chargeLossEnergyGrid, const vector<double>& chargeLossValueGrid)
{
  // Linear interpolation of the value grid over energy; holds the endpoint outside the grid.

  if (chargeLossEnergyGrid.size() == 0){
    return 0;
  }
  
  if (chargeLossEnergy <= chargeLossEnergyGrid.front()){
    return chargeLossValueGrid.front();
  }
  
  if (chargeLossEnergy >= chargeLossEnergyGrid.back()){
    return chargeLossValueGrid.back();
  }

  // chargeLossEnergyGrid is sorted ascending: find the bracket around chargeLossEnergy
  unsigned int i = 1;
  while (i < chargeLossEnergyGrid.size() && chargeLossEnergyGrid[i] < chargeLossEnergy){
    i++;
  }

  double chargeLossEnergyLow = chargeLossEnergyGrid[i-1];
  double chargeLossEnergyHigh = chargeLossEnergyGrid[i];
  double chargeLossValueLow = chargeLossValueGrid[i-1];
  double chargeLossValueHigh = chargeLossValueGrid[i];
  return chargeLossValueLow + (chargeLossEnergy - chargeLossEnergyLow)*(chargeLossValueHigh - chargeLossValueLow) / (chargeLossEnergyHigh - chargeLossEnergyLow);
}


////////////////////////////////////////////////////////////////////////////////


double MSubModuleChargeLoss::ChargeLossInterp2D(double chargeLossEnergy, double chargeLossDepth,
    const vector<double>& chargeLossEnergyGrid, const vector<double>& chargeLossDepthGrid,
    const vector<vector<double> >& chargeLossValueGrid)
{
  // Bilinear interpolation of chargeLossValueGrid[iDepth][iEnergy] at (energy, depth); clamps both axes.

  if (chargeLossDepthGrid.size() == 0 || chargeLossEnergyGrid.size() == 0){
    return 0;
  }

  // interpolate over energy at each bracketing depth row, then between the two rows in depth
  if (chargeLossDepth <= chargeLossDepthGrid.front()){
    return ChargeLossInterp1D(chargeLossEnergy, chargeLossEnergyGrid, chargeLossValueGrid.front());
  }
  
  if (chargeLossDepth >= chargeLossDepthGrid.back()){
    return ChargeLossInterp1D(chargeLossEnergy, chargeLossEnergyGrid, chargeLossValueGrid.back());
  }

  unsigned int j = 1;
  while (j < chargeLossDepthGrid.size() && chargeLossDepthGrid[j] < chargeLossDepth){
    j++;
  }

  double chargeLossDepthLow = chargeLossDepthGrid[j-1];
  double chargeLossDepthHigh = chargeLossDepthGrid[j];
  double chargeLossValueLow = ChargeLossInterp1D(chargeLossEnergy, chargeLossEnergyGrid, chargeLossValueGrid[j-1]);
  double chargeLossValueHigh = ChargeLossInterp1D(chargeLossEnergy, chargeLossEnergyGrid, chargeLossValueGrid[j]);
  return chargeLossValueLow + (chargeLossDepth - chargeLossDepthLow)*(chargeLossValueHigh - chargeLossValueLow) / (chargeLossDepthHigh - chargeLossDepthLow);
}


////////////////////////////////////////////////////////////////////////////////
