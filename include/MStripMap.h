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
#include <map>
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

  //! Load a strip map
  //! Return false on error and clear the existing map; only full-line comments are supported
  //! All records must use the same format: the column count of the first record applies to the whole file
  //! Accepted values are detector 0-15, side 0 (LV) or 1 (HV), and strip 0-64, where strip 64 is the guard ring
  //! A file without any mapping entry is an error, as is a duplicated read-out ID or (detector, side, strip) tuple
  bool Open(const MString& FileName);

  //! Remove all mappings whose detector is not in the given list, e.g. the detectors which were enabled
  //! during the run - return false and leave the map unchanged if no mapping would remain
  bool RestrictToEnabledDetectors(const vector<unsigned int>& DetectorIDs);

  //! Update which ASICs are LV/HV depending on their polarities, indexed by [detector][is-primary][ASIC]
  //! Return false and leave the map unchanged if the data is missing for any ASIC, or if the update
  //! would map two read-out IDs onto the same (detector, side, strip) tuple. An empty map succeeds.
  bool UpdateASICPolarities(const vector<map<bool, vector<bool>>>& ASICPolarities);

  //! Check if we have a certain read-out ID
  bool HasReadOutID(unsigned int ROI) const;

  //! Get detector by read out ID - check with HasReadOutID(ROI) first
  unsigned int GetDetectorID(unsigned int ROI) const;

  //! Get detector side by read out ID - check with HasReadOutID(ROI) first
  bool IsLowVoltage(unsigned int ROI) const;

  //! Get strip ID by read out ID - check with HasReadOutID(ROI) first
  unsigned int GetStripNumber(unsigned int ROI) const;

  //! Check if a (detector, side, strip) tuple is mapped to a read-out ID - out-of-range values return false
  bool HasROIDetSideStrip(unsigned int DetectorID, bool IsLowVoltage, unsigned int StripNumber) const;

  //! Reverse lookup: get read-out ID for a (detector, side, strip) tuple - check with HasROIDetSideStrip() first
  //! Throw MExceptionValueNotFound if the tuple is unmapped or any value is out of range
  unsigned int GetReadOutID(unsigned int DetectorID, bool IsLowVoltage, unsigned int StripNumber) const;


  // protected methods:
 protected:
  //! Return the index of the read-out ID or throw an exception
  unsigned int GetReadOutIDIndex(unsigned int ROI) const;


  // private methods:
 private:
  //! Compute the packed (detector, side, strip) lookup key - return false if any value is out of range
  bool ComputeDetSideStripKey(unsigned int DetectorID, bool IsLowVoltage, unsigned int StripNumber, unsigned int& Key) const;


  // protected members:
 protected:


  // private members:
 private:
  //! The internal struct for the strip map
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

  //! The largest detector ID which fits into the lookup key
  static constexpr unsigned int c_MaxDetectorID = 15;
  //! The largest strip number which fits into the lookup key - strip 64 is the guard ring
  static constexpr unsigned int c_MaxStripNumber = 64;
  //! Bit position of the detector ID within the lookup key
  static constexpr unsigned int c_DetectorIDBitPosition = 8;
  //! Bit position of the side flag within the lookup key
  static constexpr unsigned int c_SideBitPosition = 7;

  //! Reverse index: (det<<8) | (side<<7) | strip → read-out ID. 12-bit key
  //! det 0-15 (4 bits), side 0/1 (1 bit), strip 0-64 (7 bits - guard-ring strip 64)
  unordered_map<unsigned int, unsigned int> m_DetSideStripToROI;


#ifdef ___CLING___
 public:
  ClassDef(MStripMap, 0) // A strip map
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
