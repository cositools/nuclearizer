/*
 * MFITSWriterL1a.cxx
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
// MFITSWriterL1a
//
// Writes a GED_L1A (Level 1a, raw strip hits) FITS file. Owned by
// MModuleSaverMeasurementsFITS; not a pipeline module itself.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFITSWriterL1a.h"

// Standard libs:

// MEGAlib libs:
#include "MStripHit.h"
#include "MStreams.h"

using namespace std;
using namespace CCfits;


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFITSWriterL1a)
#endif


////////////////////////////////////////////////////////////////////////////////


MFITSWriterL1a::MFITSWriterL1a()
{
  // Construct an instance of MFITSWriterL1a

  m_FITSFile = nullptr;
  m_Table = nullptr;
  m_BatchStartRow = 1;
  m_BatchEventCount = 0;
  m_HasEvents = false;
  m_FirstEventTime_RTS = 0;
  m_LastEventTime_RTS = 0;
}


////////////////////////////////////////////////////////////////////////////////


MFITSWriterL1a::~MFITSWriterL1a()
{
  delete m_FITSFile;
  m_FITSFile = nullptr;
}


////////////////////////////////////////////////////////////////////////////////


bool MFITSWriterL1a::Create(const MString& FileName)
{
  // Create the FITS file with an empty primary HDU and the GED_L1A extension.
  try {
    m_FITSFile = new FITS(string(FileName), RWmode::Write);

    // Primary header (Table 6.2d primary)
    PHDU& primary = m_FITSFile->pHDU();
    primary.addKey("TELESCOP", "COSI", "Telescope mission name");
    primary.addKey("INSTRUME", "GED", "Instrument name");
    primary.addKey("ORIGIN", "SSL", "Origin of the FITS file");
    primary.addKey("CREATOR", "TBD", "Software that create 1st the file");

    // File creation date (UTC, computed at write time)
    time_t now = time(nullptr);
    char dateBuffer[32];
    strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%dT%H:%M:%S", gmtime(&now));
    primary.addKey("DATE", string(dateBuffer), "File creation date (UTC)");

    // add placeholder at creation, write at the end
    primary.addKey("OBS_ID", "YYYYMMDD", "Observation ID");
    primary.addKey("DATE-OBS", "yyyy-mm-ddThh:mm:ss", "Start Date");
    primary.addKey("DATE-END", "yyyy-mm-ddThh:mm:ss", "Stop Date");

    // GED_L1A binary table: 11 columns. P* = variable-length arrays (max 2080).
    struct ColSpec { string name, format, unit, comment; };
    const std::vector<ColSpec> colsGedL1a = {
      {"TIME",        "1D",       "s",    "Mission Time in sec since 01 Jan 2025 00:00:00"},
      {"EVENTID",     "1J",       "",     "Event ID, unique number that starts at 1 each day"},
      {"EVENTTYPE",   "1B",       "",     "Type of event"},
      {"NUMSTRIPHIT", "1I",       "",     "Number of strips involved in the hit"},
      {"HITTYPE",     "PB(2080)", "",     "Type of Hits"},
      {"DETID",       "PB(2080)", "",     "DETECTOR ID 0-15"},
      {"STRIPID",     "PB(2080)", "",     "Strip ID Number 0-64"},
      {"SIDEID",      "PX(2080)", "",     "Strip Side ID 0-1"},
      {"FASTTIME",    "PX(2080)", "",     "Interaction fast timing value 0 or 1"},
      {"PHA",         "PI(2080)", "chan", "PHA ADC value for each strip hit"},
      {"TAC",         "PI(2080)", "chan", "Timing analog converter for each strip hit"}
    };

    std::vector<string> colNames, colFormats, colUnits;
    colNames.reserve(colsGedL1a.size());
    colFormats.reserve(colsGedL1a.size());
    colUnits.reserve(colsGedL1a.size());
    for (const auto& c : colsGedL1a) {
      colNames.push_back(c.name);
      colFormats.push_back(c.format);
      colUnits.push_back(c.unit);
    }

    m_Table = m_FITSFile->addTable("GED_L1A", 0, colNames, colFormats, colUnits);

    {
      m_Table->makeThisCurrent();
      int status = 0;
      for (size_t i = 0; i < colsGedL1a.size(); ++i) {
        char keyName[16];
        snprintf(keyName, sizeof(keyName), "TTYPE%zu", i + 1);
        fits_modify_comment(m_FITSFile->fitsPointer(),
                            keyName,
                            const_cast<char*>(colsGedL1a[i].comment.c_str()),
                            &status);
      }
      if (status != 0 && g_Verbosity >= c_Warning) {
        cout << "MFITSWriterL1a: fits_modify_comment status=" << status << endl;
      }
    }

    // Extension header keywords (Table 6.2d)
    m_Table->addKey("EXTNAME", "GED_L1A", "name of this HDU");
    m_Table->addKey("TELESCOP", "COSI", "Telescope mission name");
    m_Table->addKey("INSTRUME", "GED", "Instrument name");
    m_Table->addKey("DATAMODE", "SYNC", "Instrument datamode");
    m_Table->addKey("OBSMODE", "SCANNING", "Spacecraft observing mode");
    m_Table->addKey("OBSERVER", "John Tomsick", "Principal Investigator");
    m_Table->addKey("OBJECT", "ALLSKY", "Object/target name of ALLSKY");
    m_Table->addKey("MJDREFI", 60676, "MJD reference day 01 Jan 2025 00:00:00");
    m_Table->addKey("MJDREFF", 8.007407407407E-04, "MJD reference (fraction of day)");
    m_Table->addKey("TIMEREF", "LOCAL", "Reference Frame");
    m_Table->addKey("TASSIGN", "SATELLITE", "Time assigned");
    m_Table->addKey("TIMESYS", "TT", "Time System");
    m_Table->addKey("TIMEUNIT", "s", "Time unit for timing header keywords");
    m_Table->addKey("CLOCKAPP", "F", "If clock corrections are applied (T/F)");
    m_Table->addKey("HDUCLASS", "OGIP", "format conforms to OGIP standard");
    m_Table->addKey("HDUCLAS1", "EVENTS", "hduclass1");
    m_Table->addKey("HDUCLAS2", "ALL", "hduclas2");
    m_Table->addKey("CREATOR", "TBD", "Software that create 1st the file");
    m_Table->addKey("PROCVER", "MM.XX.NN.YY", "Processing version");
    m_Table->addKey("CALDBVER", "csYYYYMMDD", "CALDB version");
    m_Table->addKey("SEQPNUM", "nn", "Times the dataset has been processed");
    m_Table->addKey("ORIGIN", "SSL", "Origin of the FITS files");

    m_Table->addKey("DATE", string(dateBuffer), "File creation date (UTC)");
    m_Table->addKey("OBS_ID", "YYYYMMDD", "Observation ID");
    m_Table->addKey("DATE-OBS", "yyyy-mm-ddThh:mm:ss", "Start Date");
    m_Table->addKey("DATE-END", "yyyy-mm-ddThh:mm:ss", "Stop Date");

    m_BatchStartRow = 1;
    m_BatchEventCount = 0;
    m_HasEvents = false;
    return true;
  } catch (const FitsException& e) {
    if (g_Verbosity >= c_Error) cout << "MFITSWriterL1a: Create error: " << e.message() << endl;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MFITSWriterL1a::Write(MReadOutAssembly* Event)
{

  double time = Event->GetTimeRTS().GetAsDouble();
  if (!m_HasEvents) {
    m_FirstEventTime_RTS = time;
    m_LastEventTime_RTS = time;
    m_HasEvents = true;
  } else {
    if (time < m_FirstEventTime_RTS) m_FirstEventTime_RTS = time;
    if (time > m_LastEventTime_RTS) m_LastEventTime_RTS = time;
  }

  unsigned int nStripHits = Event->GetNStripHits();

  std::valarray<uint8_t> hittype(nStripHits), detid(nStripHits), stripid(nStripHits), sideid(nStripHits), fasttime(nStripHits);
  std::valarray<int32_t> pha(nStripHits), tac(nStripHits);

  for (unsigned int i = 0; i < nStripHits; ++i) {
    MStripHit* hit = Event->GetStripHit(i);
    uint8_t hitType = 0;
    if (hit->IsGuardRing()) hitType = 2;
    else if (hit->IsNearestNeighbor()) hitType = 1;
    hittype[i]  = hitType;
    detid[i]    = hit->GetDetectorID();
    stripid[i]  = hit->GetStripID();
    sideid[i]   = hit->IsLowVoltageStrip() ? 0 : 1;
    fasttime[i] = hit->HasFastTiming() ? 1 : 0;
    pha[i]      = hit->GetADCUnits();
    tac[i]      = hit->GetTAC();
  }

  m_BatchTIME.push_back(time);
  m_BatchEVENTID.push_back((uint32_t)Event->GetID());
  m_BatchEVENTTYPE.push_back(0);            // TODO
  m_BatchNUMSTRIPHIT.push_back(nStripHits);
  m_BatchHITTYPE.push_back(hittype);
  m_BatchDETID.push_back(detid);
  m_BatchSTRIPID.push_back(stripid);
  m_BatchSIDEID.push_back(sideid);
  m_BatchFASTTIME.push_back(fasttime);
  m_BatchPHA.push_back(pha);
  m_BatchTAC.push_back(tac);
  m_BatchEventCount++;

  if (m_BatchEventCount >= m_BatchSize) {
    return FlushBatch();
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFITSWriterL1a::FlushBatch()
{
  if (m_BatchEventCount == 0) return true;
  try {
    long firstRow = m_BatchStartRow;
    m_Table->column("TIME").write(m_BatchTIME, firstRow);
    m_Table->column("EVENTID").write(m_BatchEVENTID, firstRow);
    m_Table->column("EVENTTYPE").write(m_BatchEVENTTYPE, firstRow);
    m_Table->column("NUMSTRIPHIT").write(m_BatchNUMSTRIPHIT, firstRow);

    // Variable-length array columns
    m_Table->column("HITTYPE").writeArrays(m_BatchHITTYPE, firstRow);
    m_Table->column("DETID").writeArrays(m_BatchDETID, firstRow);
    m_Table->column("STRIPID").writeArrays(m_BatchSTRIPID, firstRow);
    m_Table->column("SIDEID").writeArrays(m_BatchSIDEID, firstRow);
    m_Table->column("FASTTIME").writeArrays(m_BatchFASTTIME, firstRow);
    m_Table->column("PHA").writeArrays(m_BatchPHA, firstRow);
    m_Table->column("TAC").writeArrays(m_BatchTAC, firstRow);

    m_BatchStartRow += m_BatchEventCount;

    m_BatchTIME.clear();
    m_BatchEVENTID.clear();
    m_BatchEVENTTYPE.clear();
    m_BatchNUMSTRIPHIT.clear();
    m_BatchHITTYPE.clear();
    m_BatchDETID.clear();
    m_BatchSTRIPID.clear();
    m_BatchSIDEID.clear();
    m_BatchFASTTIME.clear();
    m_BatchPHA.clear();
    m_BatchTAC.clear();
    m_BatchEventCount = 0;
    return true;
  } catch (const FitsException& e) {
    if (g_Verbosity >= c_Error) cout << "MFITSWriterL1a: FlushBatch error: " << e.message() << endl;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MFITSWriterL1a::Close()
{
  if (m_BatchEventCount > 0) FlushBatch();

  if (m_HasEvents && m_Table != nullptr) {
    try {
      m_Table->addKey("TSTART", m_FirstEventTime_RTS, "Start time [s] since MJDREFI");
      m_Table->addKey("TSTOP", m_LastEventTime_RTS, "Stop time [s] since MJDREFI");

      constexpr time_t MISSION_EPOCH_UNIX = 1735689600;  // 2025-01-01T00:00:00 UTC
      time_t startUnix = MISSION_EPOCH_UNIX + (time_t)m_FirstEventTime_RTS;
      time_t stopUnix  = MISSION_EPOCH_UNIX + (time_t)m_LastEventTime_RTS;
      char startBuf[32], stopBuf[32];
      strftime(startBuf, sizeof(startBuf), "%Y-%m-%dT%H:%M:%S", gmtime(&startUnix));
      strftime(stopBuf,  sizeof(stopBuf),  "%Y-%m-%dT%H:%M:%S", gmtime(&stopUnix));

      PHDU& primary = m_FITSFile->pHDU();
      primary.addKey("DATE-OBS", string(startBuf), "Start Date");
      primary.addKey("DATE-END", string(stopBuf),  "Stop Date");
      m_Table->addKey("DATE-OBS", string(startBuf), "Start Date");
      m_Table->addKey("DATE-END", string(stopBuf),  "Stop Date");

      char obsIdBuf[16];
      strftime(obsIdBuf, sizeof(obsIdBuf), "%Y%m%d", gmtime(&startUnix));
      primary.addKey("OBS_ID", string(obsIdBuf), "Observation ID");
      m_Table->addKey("OBS_ID", string(obsIdBuf), "Observation ID");
    } catch (const FitsException& e) {
      if (g_Verbosity >= c_Error) cout << "MFITSWriterL1a: Close keyword error: " << e.message() << endl;
    }
  }

  if (m_FITSFile != nullptr) {
    try {
      m_FITSFile->pHDU().writeChecksum();
      if (m_Table != nullptr) {
        m_Table->writeChecksum();
      }
    } catch (const FitsException& e) {
      if (g_Verbosity >= c_Error) cout << "MFITSWriterL1a: writeChecksum error: " << e.message() << endl;
    }
  }

  delete m_FITSFile;   // CCfits flushes + closes on destruction
  m_FITSFile = nullptr;
  m_Table = nullptr;
}


// MFITSWriterL1a.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
