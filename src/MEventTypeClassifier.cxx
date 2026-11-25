/*
 * MEventTypeClassifier.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
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
// MEventTypeClassifier
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MEventTypeClassifier.h"

// Standard libs:
#include <algorithm>
#include <set>
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:

// Nuclearizer libs:
#include "MStripHit.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MEventTypeClassifier)
#endif


////////////////////////////////////////////////////////////////////////////////


uint8_t MEventTypeClassifier::Classify(MReadOutAssembly* Event)
{
  // Event types from COSI-ICD-011 (FEE), Table 36:
  //   0x00 Multi-detector Compton, no veto       0x03 / 0x06 / 0x09 the same with GR / hard shield / soft shield veto
  //   0x01 Single-detector Compton, no veto      0x04 / 0x07 / 0x0A the same with GR / hard shield / soft shield veto
  //   0x02 Single site, no veto                  0x05 / 0x08 / 0x0B the same with GR / hard shield / soft shield veto
  //   0x0C GR event

  if (Event->HasOnboardEventType() == true) {
    return (uint8_t) Event->GetOnboardEventType();
  }

  set<int> Detectors;
  vector<int> LVStrips;
  vector<int> HVStrips;

  for (unsigned int h = 0; h < Event->GetNStripHits(); ++h) {

    MStripHit* Hit = Event->GetStripHit(h);
    if (Hit->IsGuardRing() == true) continue;

    if (Hit->IsNearestNeighbor() == true) continue;

    Detectors.insert(Hit->GetDetectorID());

    if (Hit->IsLowVoltageStrip() == true) {
      LVStrips.push_back(Hit->GetStripID());
    } else {
      HVStrips.push_back(Hit->GetStripID());
    }

  }

  // A guard ring veto without any strip hit is a GR event
  if (Detectors.empty() == true && Event->GetGuardRingVeto() == true) {
    return 0x0C;
  }

  // Category: 0 = multi-detector, 1 = single-detector Compton, 2 = single site
  uint8_t Category = 2;
  if (Detectors.size() >= 2) {
    Category = 0;
  } else if (Detectors.size() == 1) {
    bool HasComptonGap = false;
    for (vector<int>* Strips : { &LVStrips, &HVStrips }) {
      sort(Strips->begin(), Strips->end());
      for (size_t s = 1; s < Strips->size(); ++s) {
        if ((*Strips)[s] - (*Strips)[s-1] >= 3) {
          HasComptonGap = true;
        }
      }
    }
    if (HasComptonGap == true) Category = 1;
  }

  if (Event->GetGuardRingVeto() == true) {
    return 0x03 + Category;
  } else if (Event->GetShieldVeto() == true) {
    return 0x09 + Category;
  }
  return Category;
}


// MEventTypeClassifier.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
