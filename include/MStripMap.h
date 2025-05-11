/*
 * MStripMap.h
 *
 * Copyright (C) by YOUR NAME HERE.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MStripMap__
#define __MStripMap__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;
//
// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This class represents the mapping from asic channels to detector, side, and strip ID
class MStripMap
{
  // public interface:
 public:
  //! Default constructor
  MStripMap();
  //! Default destructor
  virtual ~MStripMap();

  //! Load a strip map
  bool Open(MString FileName);

  //! Get number of channels
  unsigned int GetNumberOfReadOutIDs() const;

  //! Get number of channels
  bool HasReadOutID(unsigned int ROI) const;

  //! Get detector by read out ID
  unsigned int GetDetectorID(unsigned int ROI) const;

  //! Get detector side by read out ID
  bool IsLowVoltage(unsigned int ROI) const;

  //! Get strip ID by read out ID
  unsigned int GetStripNumber(unsigned int ROI) const;


  // protected methods:
 protected:
  //MStripMap() {};
  //MStripMap(const MStripMap& StripMap) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The read-out ID
  vector<unsigned int> m_ReadOutID;
  //! The ID of the RTB
  vector<unsigned int> m_RTB;
  //! The ID of the DRM board
  vector<unsigned int> m_DRM;
  //! ???
  vector<bool> m_IsPrimary;
  //! The ID of the ASIC
  vector<unsigned int> m_ASICID;
  //! The ID of the channel
  vector<unsigned int> m_ChannelID;
  //! The detector ID
  vector<unsigned int> m_DetectorID;
  //! Is the read-out unit on the low voltage side
  vector<bool> m_IsLowVoltage;
  //! The strip number on that side
  vector<unsigned int> m_StripNumber;


#ifdef ___CLING___
 public:
  ClassDef(MStripMap, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
