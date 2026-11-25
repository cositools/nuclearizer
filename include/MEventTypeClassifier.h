/*
 * MEventTypeClassifier.h
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEventTypeClassifier__
#define __MEventTypeClassifier__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <cstdint>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Nuclearizer libs:
#include "MReadOutAssembly.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Determines the event type (ETYPE) of an event from its strip hits and veto flags
class MEventTypeClassifier
{
  // public interface:
 public:
  //! Default constructor
  MEventTypeClassifier() {}
  //! Default destructor
  virtual ~MEventTypeClassifier() {}

  //! Return the event type 0x00-0x0C of the event:
  //! If the event has an on-board event type, return it unchanged, otherwise determine it:
  //! 0x00-0x02 multi-detector Compton, single-detector Compton, single site, without veto,
  //! 0x03-0x05 the same with guard ring veto
  //! 0x09-0x0B the same with (soft) shield veto,
  //! and 0x0C for a guard ring veto without any strip hit
  static uint8_t Classify(MReadOutAssembly* Event);


#ifdef ___CLING___
 public:
  ClassDef(MEventTypeClassifier, 0) // Determines the FEE event type of an event
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
