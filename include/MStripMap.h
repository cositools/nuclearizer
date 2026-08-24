/*
 * MStripMap.h
 *
 * Copyright (C) by Andreas Zoglauer, Felix Hagemann.
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
#include <algorithm>
#include <unordered_map>
using namespace std;

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

  //! Load a strip map - return false on error
  bool Open(MString FileName);

  //! Update which ASICs are LV/HV depending on their polarities
  bool UpdateASICPolarities(vector<map<bool, vector<bool>>> ASICPolarities);

  //! Check if we have a certain read-out ID
  bool HasReadOutID(unsigned int ROI) const;

  //! Get detector by read out ID - check with HasReadOutID(ROI) first
  unsigned int GetDetectorID(unsigned int ROI) const;

  //! Get detector side by read out ID - check with HasReadOutID(ROI) first
  bool IsLowVoltage(unsigned int ROI) const;

  //! Get strip ID by read out ID - check with HasReadOutID(ROI) first
  unsigned int GetStripNumber(unsigned int ROI) const;

  //! Check if a (detector, side, strip) tuple is mapped to a read-out ID
  bool HasROIDetSideStrip(unsigned int DetectorID, bool IsLowVoltage, unsigned int StripNumber) const;

  //! Reverse lookup: get read-out ID for a (detector, side, strip) tuple.
  unsigned int GetReadOutID(unsigned int DetectorID, bool IsLowVoltage, unsigned int StripNumber) const;


  // protected methods:
 protected:
  //! Return the index of the read-out ID or throw an exception
  unsigned int GetReadOutIDIndex(unsigned int ROI) const;


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The internal struct for the map
  struct MSingleStripMapping {
    unsigned int m_ReadOutID;
    unsigned int m_RTB;
    unsigned int m_DRM;
    bool m_IsPrimary;
    unsigned int m_ASICID;
    unsigned int m_ChannelID;
    unsigned int m_DetectorID;
    bool m_IsLowVoltage;
    unsigned int m_StripNumber;
  };

  //! The strip mapping data
  vector<MSingleStripMapping> m_StripMappings;

  //! Reverse index: (det<<8) | (side<<7) | strip → read-out ID. 12-bit key
  //! det 0-15 (4 bits), side 0/1 (1 bit), strip 0-64 (7 bits - guard-ring strip 64)
  unordered_map<unsigned int, unsigned int> m_DetSideStripToROI;


#ifdef ___CLING___
 public:
  ClassDef(MStripMap, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
