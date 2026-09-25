/*
 * MFITSWriterL1a.h
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFITSWriterL1a__
#define __MFITSWriterL1a__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <valarray>
#include <string>
#include <cstdint>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Nuclearizer libs:
#include "MReadOutAssembly.h"

// CCfits libs:
#include <CCfits/CCfits>


////////////////////////////////////////////////////////////////////////////////


class MFITSWriterL1a
{
  // public interface:
 public:
  //! Default constructor
  MFITSWriterL1a();
  //! Default destructor
  virtual ~MFITSWriterL1a();

  //! Create the FITS file + GED_L1A extension. Returns false on error.
  bool Create(const MString& FileName);
  //! Append one event's strip hits as a row. Returns false on error.
  bool Write(MReadOutAssembly* Event);
  //! Flush buffered rows to the table. Returns false on error.
  bool FlushBatch();

  void Close();


 private:
  //! The FITS file object
  CCfits::FITS* m_FITSFile;
  //! The GED_L1A science table extension
  CCfits::ExtHDU* m_Table;

  long m_BatchStartRow;
  long m_BatchEventCount;
  //! Batch size
  static const long m_BatchSize = 100;

  bool m_HasEvents;

  //! First / last event time seen, RTS (mission seconds since 2025-01-01)
  double m_FirstEventTime_RTS;
  double m_LastEventTime_RTS;

  std::vector<double>   m_BatchTIME;    
  std::vector<uint32_t> m_BatchEVENTID;
  std::vector<uint8_t>  m_BatchEVENTTYPE;
  std::vector<uint16_t> m_BatchNUMSTRIPHIT;

  std::vector<std::valarray<uint8_t>>  m_BatchHITTYPE;   // PB
  std::vector<std::valarray<uint8_t>>  m_BatchDETID;     // PB
  std::vector<std::valarray<uint8_t>>  m_BatchSTRIPID;   // PB
  std::vector<std::valarray<uint8_t>>  m_BatchSIDEID;    // PX (0/1)
  std::vector<std::valarray<uint8_t>>  m_BatchFASTTIME;  // PX (0/1)
  std::vector<std::valarray<int32_t>>  m_BatchPHA;       // PI
  std::vector<std::valarray<int32_t>>  m_BatchTAC;       // PI


#ifdef ___CLING___
 public:
  ClassDef(MFITSWriterL1a, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
