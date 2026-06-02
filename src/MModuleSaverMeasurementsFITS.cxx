/*
 * MModuleSaverMeasurementsFITS.cxx
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
// MModuleSaverMeasurementsFITS
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleSaverMeasurementsFITS.h"

// Standard libs:
#include <algorithm>
#include <ctime>
#include <sstream>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsSaverMeasurementsFITS.h"
#include "MHit.h"
#include "MPhysicalEvent.h"
#include "MPhysicalEventHit.h"
#include "MComptonEvent.h"
#include "MPhotoEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleSaverMeasurementsFITS)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleSaverMeasurementsFITS::MModuleSaverMeasurementsFITS() : MModule()
{
  // Construct an instance of MModuleSaverMeasurementsFITS

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Save events to FITS files (L1b/L2)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagSaverMeasurementsFITS";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  AddPreceedingModuleType(MAssembly::c_StripPairing);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventSaver);

  // Set if this module has an options GUI
  m_HasOptionsGUI = true;

  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;

  m_FITSFile = nullptr;
  m_PrimaryHDU = nullptr;
  m_ScienceTable = nullptr;
  m_TotalEventsWritten = 0;
  m_TotalEventsSkipped = 0;
  m_BatchStartRow = 1;
  m_BatchEventCount = 0;
  m_OutputDataLevel = 1; // 1 = L1b (default), 2 = L2
  m_FirstEventTime_RTS = 0.0;
  m_LastEventTime_RTS = 0.0;
  m_HasEvents = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleSaverMeasurementsFITS::~MModuleSaverMeasurementsFITS()
{
  // Delete this instance of MModuleSaverMeasurementsFITS
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaverMeasurementsFITS::Initialize()
{
  // Initialize the module

  if (m_FileName == "") {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": No output file name specified."<<endl;
    return false;
  }

  // Create the FITS file
  if (CreateFITSFile(m_FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to create FITS file."<<endl;
    return false;
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaverMeasurementsFITS::CreateFITSFile(MString FileName)
{
  // Create the FITS file using CCfits
  try {

    string levelStr = (m_OutputDataLevel == 2) ? "L2" : "L1b";
    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Creating "<<levelStr<<" FITS file: "<<string(FileName)<<endl;

    // Create new FITS file (overwrite if exists)
    m_FITSFile = new FITS(string(FileName), RWmode::Write);

    // Get pointer to Primary HDU
    m_PrimaryHDU = &m_FITSFile->pHDU();

    // Add some keywords to primary HDU
    m_PrimaryHDU->addKey("TELESCOP", "COSI", "Mission name");
    m_PrimaryHDU->addKey("INSTRUME", "GeD", "Instrument name");
    m_PrimaryHDU->addKey("OBS_ID", "YYMMDD", "Observation ID"); //OBS_ID should have the same YYMMDD as the filename
    m_PrimaryHDU->addKey("DATE-OBS", "yyyy-mm-ddThh:mm:ss", "Start Date");  //DATE-OBS should have the start date and time of the data, and this should match the YYMMDD in the filename
    m_PrimaryHDU->addKey("DATE-END", "yyyy-mm-ddThh:mm:ss", "Stop Date");   //DATE-END should have the stop time of the data, i.e. the last timestamp
    m_PrimaryHDU->addKey("ORIGIN", "SSL", "Organization");

    // Get current time for DATE keyword
    time_t now = time(nullptr);
    struct tm* utc = gmtime(&now);
    char dateBuffer[32];
    strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%dT%H:%M:%S", utc);
    m_PrimaryHDU->addKey("DATE", string(dateBuffer), "File creation date (UTC)"); //DATE should have the date of the file creation

    m_PrimaryHDU->addKey("CREATOR", "TBD", "Software that created this file");

    // Define columns for science data table per HEASARC Tech Agreement v1.1.
    bool isL1b = (m_OutputDataLevel == 1);
    string seqHitFormat = isL1b ? "PB(50)" : "10B";
    string hitFormat    = isL1b ? "PE(50)" : "10E";

    std::vector<string> colNames = {
      "TIME", "EVENTID", "EVENTCLASS", "NUMHIT", "SEQHIT",
      "X", "Y", "Z",
      "X_ERR", "Y_ERR", "Z_ERR",
      "ENERGY", "ENERGY_ERR", 
      "RECOILDIR", "RECOILDIR_ERR"
    };

    std::vector<string> colFormats = {
      "1D",          // TIME
      "1J",          // EVENTID
      "1B",          // EVENTCLASS
      "1B",          // NUMHIT
      seqHitFormat,  // SEQHIT
      hitFormat,     // X
      hitFormat,     // Y
      hitFormat,     // Z
      hitFormat,     // X_ERR
      hitFormat,     // Y_ERR
      hitFormat,     // Z_ERR
      hitFormat,     // ENERGY
      hitFormat,     // ENERGY_ERR
      "3E",          // RECOILDIR
      "3E",          // RECOILDIR_ERR
    };

    std::vector<string> colUnits = {
      "s", "", "", "", "",
      "cm", "cm", "cm",
      "cm", "cm", "cm",
      "keV", "keV",
      "", ""
    };

    // L2 drops EVENTTYPE, STATTEST, VETO, and QUALITY_FLAG.
    // VETO: 0=none, 1=hard ACD veto, 2=soft ACD veto, 3=guard ring veto
    if (m_OutputDataLevel == 1) {
      colNames.push_back("EVENTTYPE");
      colFormats.push_back("1B");
      colUnits.push_back("");

      colNames.push_back("STATTEST");
      colFormats.push_back("8E");
      colUnits.push_back("");

      colNames.push_back("VETO");
      colFormats.push_back("1B");
      colUnits.push_back("");

      colNames.push_back("QUALITY_FLAG");
      colFormats.push_back("64A");
      colUnits.push_back("");
    }

    // Create binary table extension
    string extName = (m_OutputDataLevel == 2) ? "GED_L2" : "GED_L1B";
    m_ScienceTable = m_FITSFile->addTable(extName, 0, colNames, colFormats, colUnits);

    // Add keywords to science table
    m_ScienceTable->addKey("EXTNAME", extName, "name of this HDU");
    m_ScienceTable->addKey("TELESCOP", "COSI", "Telescope mission name");
    m_ScienceTable->addKey("INSTRUME", "GED", "Instrument name");
    m_ScienceTable->addKey("DATAMODE", "SYNC", "Instrument datamode: SYNC or ASYNC");
    m_ScienceTable->addKey("OBSERVER", "John Tomsick", "Principal Investigator");
    m_ScienceTable->addKey("OBS_ID", "YYMMDD", "Observation ID"); //should match the YYMMDD of the filename
    m_ScienceTable->addKey("OBJECT", "ALL SKY", "Object/Target name or ALL SKY");
    m_ScienceTable->addKey("MJDREFI", 60676, "MJD reference day 01 Jan 2025 00:00:00");
    m_ScienceTable->addKey("MJDREFF", 8.007407407407E-04, "MJD reference (fraction of day)");
    m_ScienceTable->addKey("TIMEREF", "LOCAL", "Reference Frame");
    m_ScienceTable->addKey("TASSIGN", "SATELLITE", "Time assigned");
    m_ScienceTable->addKey("TIMESYS", "TT", "Time System");
    m_ScienceTable->addKey("TIMEUNIT", "s", "Time unit for timing header keywords");
    m_ScienceTable->addKey("CLOCKAPP", false, "If clock corrections are applied (T/F)");
    m_ScienceTable->addKey("DATE-OBS", "yyyy-mm-ddThh:mm:ss", "Start Date"); //placeholder, this will be written after we read through all the events
    m_ScienceTable->addKey("DATE-END", "yyyy-mm-ddThh:mm:ss", "Stop Date"); // 
    m_ScienceTable->addKey("TSTART", 0.0, "Start time"); //placeholder, this will be written after we read through all the events
    m_ScienceTable->addKey("TSTOP", 0.0, "Stop time"); //
    m_ScienceTable->addKey("HDUCLASS", "OGIP", "format conforms to OGIP standard");
    m_ScienceTable->addKey("HDUCLAS1", "ARRAY", "hduclass1");
    m_ScienceTable->addKey("HDUCLAS2", "TOTAL", "hduclas2");
    m_ScienceTable->addKey("CREATOR", "TBD", "Software that create 1st the file");
    m_ScienceTable->addKey("PROCVER", "TBD", "Processing Version");
    m_ScienceTable->addKey("CALDBVER", "TBD", "CALDB version");
    m_ScienceTable->addKey("SEQPNUM", "TBD", "Times the dataset has been processed");
    m_ScienceTable->addKey("ORIGIN", "SSL", "Origin of the FITS files");
    m_ScienceTable->addKey("DATE", "TOTAL", "File creation date"); //DATE should have the date of the file creation (same as primary header)
    //CHECKSUM
    //DATESUM

    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": FITS file created successfully"<<endl;

    return true;

  } catch (const CCfits::FitsException& e) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error creating FITS file: "<<e.message()<<endl;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaverMeasurementsFITS::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Add this event to the batch, write batch when full

  // L2 mode: skip bad events (screening)
  if (m_OutputDataLevel == 2 && Event->IsBad()) {
    m_TotalEventsSkipped++;
    Event->SetAnalysisProgress(MAssembly::c_EventSaver);
    return true;
  }

  // Extract event-level data
  double time = 0;
  if (Event->GetTimeRTS() == 0 && Event->GetTimeUTC() != 0) {
    // If UTC time is defined, calculate RTS
    MTime RTS = Event->ComputeRTSfromUTCTime(Event->GetTimeUTC());
    Event->SetTimeRTS(RTS);
    time = RTS.GetAsDouble();
  } else {
    time = Event->GetTimeRTS().GetAsDouble();
  }
  unsigned int numHits = Event->GetNHits();

  // loop through all event, and record the start and end time for TSTART/TSTOP
  if (!m_HasEvents) {
    m_FirstEventTime_RTS = time;
    m_LastEventTime_RTS = time;
    m_HasEvents = true;
  } else {
    if (time < m_FirstEventTime_RTS) m_FirstEventTime_RTS = time;
    if (time > m_LastEventTime_RTS) m_LastEventTime_RTS = time;
  }

  // Event-level metadata defaults
  uint8_t eventType = 0;    // TODO: 0 = unknown/default

  // EVENTCLASS per HEASARC Tech Agreement v1.1
  //   0 = Compton, 1 = photoabsorption, 2 = tracked Compton, 3 = charge particle, 4 = pair, 5 = unknown.
  uint8_t eventClass = 5;   // 5 = unknown
  uint32_t eventID = (uint32_t)Event->GetID();
  
  // TODO: figure out where to get VETO
  uint8_t veto = 0;
  std::string quality_flag;

  // L2: fixed-length 10 hit arrays, zero-padded 
  const unsigned int L2_HIT_LEN = 10;
  bool isL2 = (m_OutputDataLevel == 2);
  unsigned int arrayLen = isL2 ? L2_HIT_LEN : numHits;

  // Fixed-length arrays for event-level data (initialize to zeros)
  std::valarray<float> statTest(0.0f, 8);   // new spec changed to 8E
  std::valarray<float> recoilDir(0.0f, 3);
  std::valarray<float> recoilDirErr(0.0f, 3);

  // Hit-level arrays sized to arrayLen (numHits for L1b, fixed 10 for L2)
  std::valarray<uint8_t> seqHitArr((uint8_t)0, arrayLen);
  std::valarray<float> x(0.0f, arrayLen);
  std::valarray<float> y(0.0f, arrayLen);
  std::valarray<float> z(0.0f, arrayLen);
  std::valarray<float> x_err(0.0f, arrayLen);
  std::valarray<float> y_err(0.0f, arrayLen);
  std::valarray<float> z_err(0.0f, arrayLen);
  std::valarray<float> energy(0.0f, arrayLen);
  std::valarray<float> energy_err(0.0f, arrayLen);
  
  // Extract revan reconstruction data if available
  MPhysicalEvent* PE = Event->GetPhysicalEvent();
  if (PE != nullptr) {
    int peType = PE->GetType();

    if (peType == MPhysicalEvent::c_Compton) {
      eventClass = 0;  // 0 = Compton

      MComptonEvent* CE = dynamic_cast<MComptonEvent*>(PE);
      if (CE != nullptr) {

        MVector de = CE->De();
        recoilDir[0] = (float)de.X();
        recoilDir[1] = (float)de.Y();
        recoilDir[2] = (float)de.Z();

        MVector dde = CE->dDe();
        recoilDirErr[0] = (float)dde.X();
        recoilDirErr[1] = (float)dde.Y();
        recoilDirErr[2] = (float)dde.Z();

        // TODO: need to figure out what exactly should be in statTest[0-7]
        statTest[0] = (float)CE->Phi();
        statTest[1] = (float)CE->DeltaTheta();
        statTest[2] = (float)CE->MinLeverArm();
      }

    } else if (peType == MPhysicalEvent::c_Photo) {
      eventClass = 1;  // 1 = photoabsorption
      if (arrayLen > 0) seqHitArr[0] = 1;
    } else {
      eventClass = 5;  // 5 = unknown
    }
  }

  if (m_OutputDataLevel == 1 && Event->IsBad()) {
    std::ostringstream oss;
    Event->StreamBDFlags(oss);
    quality_flag = oss.str();

    std::replace(quality_flag.begin(), quality_flag.end(), '\n', ';');

    if (quality_flag.length() > 64) {
      quality_flag = quality_flag.substr(0, 64);
    }
  }

  // check if PE is Compton event, and the PE-> GetNHits() is the same as the Event->GetNHits()
  bool comptonEvent = (PE != nullptr && PE->GetType() == MPhysicalEvent::c_Compton && PE->GetNHits() == numHits);

  for (unsigned int i = 0; i < numHits; ++i) {
    // For Compton events, get hits from MPhysicalEventHit
    // For photo / unreconstructed events, get hits from MHit
    if (comptonEvent) {
      const MPhysicalEventHit& hit = PE->GetHit(i);
      MVector position = hit.GetPosition();
      x[i] = (float)position.X();
      y[i] = (float)position.Y();
      z[i] = (float)position.Z();

      MVector positionUncertainty = hit.GetPositionUncertainty();
      x_err[i] = (float)positionUncertainty.X();
      y_err[i] = (float)positionUncertainty.Y();
      z_err[i] = (float)positionUncertainty.Z();

      energy[i] = (float)hit.GetEnergy();
      energy_err[i] = (float)hit.GetEnergyUncertainty();

      seqHitArr[i] = (uint8_t)(i + 1); 
    } else {
      MHit* hit = Event->GetHit(i);
      MVector position = hit->GetPosition();
      x[i] = (float)position.X();
      y[i] = (float)position.Y();
      z[i] = (float)position.Z();

      MVector positionResolution = hit->GetPositionResolution();
      x_err[i] = (float)positionResolution.X();
      y_err[i] = (float)positionResolution.Y();
      z_err[i] = (float)positionResolution.Z();

      energy[i] = (float)hit->GetEnergy();
      energy_err[i] = (float)hit->GetEnergyResolution();
    }
  }

  // Add to batch
  m_BatchTIME.push_back(time);
  m_BatchEVENTID.push_back(eventID);
  m_BatchEVENTCLASS.push_back(eventClass);
  m_BatchNUMHIT.push_back((uint8_t)numHits);
  m_BatchSEQHIT.push_back(seqHitArr);
  m_BatchRECOILDIR.push_back(recoilDir);
  m_BatchRECOILDIR_ERR.push_back(recoilDirErr);
  m_BatchX.push_back(x);
  m_BatchY.push_back(y);
  m_BatchZ.push_back(z);
  m_BatchX_ERR.push_back(x_err);
  m_BatchY_ERR.push_back(y_err);
  m_BatchZ_ERR.push_back(z_err);
  m_BatchENERGY.push_back(energy);
  m_BatchENERGY_ERR.push_back(energy_err);

  // L1b-only columns (VENTTYPE, STATTEST, VETO, QUALITY_FLAG)
  if (m_OutputDataLevel == 1) {
    m_BatchEVENTTYPE.push_back(eventType);
    m_BatchSTATTEST.push_back(statTest);
    m_BatchVETO.push_back(veto);
    m_BatchQUALITY_FLAG.push_back(quality_flag);
  }

  m_BatchEventCount++;

  // Write batch if full
  if (m_BatchEventCount >= m_BatchSize) {
    if (FlushBatch() == false) {
      m_IsOK = false;
      return false;
    }
  }

  Event->SetAnalysisProgress(MAssembly::c_EventSaver);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaverMeasurementsFITS::FlushBatch()
{
  // Write the current batch to the FITS file

  if (m_BatchEventCount == 0) {
    return true; // Nothing to write
  }

  try {
    // Calculate last row for this batch
    long lastRow = m_BatchStartRow + m_BatchEventCount - 1;

    if (g_Verbosity >= c_Info) {
      cout<< m_XmlTag <<": Writing batch: "<<m_BatchEventCount<<" events (rows "<<m_BatchStartRow<<" to "<<lastRow<<")"<<endl;
    }

    // Write columns common to both L1b and L2
    m_ScienceTable->column("TIME").write(m_BatchTIME, m_BatchStartRow);
    m_ScienceTable->column("EVENTID").write(m_BatchEVENTID, m_BatchStartRow);
    m_ScienceTable->column("EVENTCLASS").write(m_BatchEVENTCLASS, m_BatchStartRow);
    m_ScienceTable->column("NUMHIT").write(m_BatchNUMHIT, m_BatchStartRow);
    m_ScienceTable->column("SEQHIT").writeArrays(m_BatchSEQHIT, m_BatchStartRow);
    m_ScienceTable->column("RECOILDIR").writeArrays(m_BatchRECOILDIR, m_BatchStartRow);
    m_ScienceTable->column("RECOILDIR_ERR").writeArrays(m_BatchRECOILDIR_ERR, m_BatchStartRow);
    m_ScienceTable->column("X").writeArrays(m_BatchX, m_BatchStartRow);
    m_ScienceTable->column("Y").writeArrays(m_BatchY, m_BatchStartRow);
    m_ScienceTable->column("Z").writeArrays(m_BatchZ, m_BatchStartRow);
    m_ScienceTable->column("X_ERR").writeArrays(m_BatchX_ERR, m_BatchStartRow);
    m_ScienceTable->column("Y_ERR").writeArrays(m_BatchY_ERR, m_BatchStartRow);
    m_ScienceTable->column("Z_ERR").writeArrays(m_BatchZ_ERR, m_BatchStartRow);
    m_ScienceTable->column("ENERGY").writeArrays(m_BatchENERGY, m_BatchStartRow);
    m_ScienceTable->column("ENERGY_ERR").writeArrays(m_BatchENERGY_ERR, m_BatchStartRow);

    // Write L1b-only columns
    if (m_OutputDataLevel == 1) {
      m_ScienceTable->column("EVENTTYPE").write(m_BatchEVENTTYPE, m_BatchStartRow);
      m_ScienceTable->column("STATTEST").writeArrays(m_BatchSTATTEST, m_BatchStartRow);
      m_ScienceTable->column("VETO").write(m_BatchVETO, m_BatchStartRow);
      m_ScienceTable->column("QUALITY_FLAG").write(m_BatchQUALITY_FLAG, m_BatchStartRow);
    }

    // Update tracking
    m_TotalEventsWritten += m_BatchEventCount;
    m_BatchStartRow += m_BatchEventCount;

    // Clear batch vectors - scalar columns
    m_BatchTIME.clear();
    m_BatchEVENTID.clear();
    m_BatchEVENTTYPE.clear();
    m_BatchEVENTCLASS.clear();
    m_BatchNUMHIT.clear();
    m_BatchSEQHIT.clear();

    // Clear batch vectors - fixed-length arrays
    m_BatchSTATTEST.clear();
    m_BatchRECOILDIR.clear();
    m_BatchRECOILDIR_ERR.clear();

    // Clear batch vectors - variable-length arrays
    m_BatchX.clear();
    m_BatchY.clear();
    m_BatchZ.clear();
    m_BatchX_ERR.clear();
    m_BatchY_ERR.clear();
    m_BatchZ_ERR.clear();
    m_BatchENERGY.clear();
    m_BatchENERGY_ERR.clear();
    m_BatchVETO.clear();
    m_BatchQUALITY_FLAG.clear();

    m_BatchEventCount = 0;

    return true;

  } catch (const CCfits::FitsException& e) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error writing FITS batch"<<e.message()<<endl;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaverMeasurementsFITS::Finalize()
{
  // Finalize the module

  // Write any remaining events in the batch
  if (m_BatchEventCount > 0) {
    FlushBatch();
  }

  // Update time-related header keywords with actual values from event data: TSTART, TSTOP, DATE-OBS, DATE-END
  if (m_HasEvents && m_ScienceTable != nullptr && m_PrimaryHDU != nullptr) {
    try {
      // Mission epoch is 2025-01-01 00:00:00 UTC
      const time_t MISSION_EPOCH_UNIX = 1735689600;

      time_t startUnix = MISSION_EPOCH_UNIX + (time_t)m_FirstEventTime_RTS;
      time_t stopUnix = MISSION_EPOCH_UNIX + (time_t)m_LastEventTime_RTS;

      //convert to ISO string
      char startBuf[32], stopBuf[32];
      strftime(startBuf, sizeof(startBuf), "%Y-%m-%dT%H:%M:%S", gmtime(&startUnix));
      strftime(stopBuf, sizeof(stopBuf), "%Y-%m-%dT%H:%M:%S", gmtime(&stopUnix));

      // Update primary HDU
      m_PrimaryHDU->addKey("DATE-OBS", string(startBuf), "Start Date");
      m_PrimaryHDU->addKey("DATE-END", string(stopBuf), "Stop Date");

      // Update science table HDU
      m_ScienceTable->addKey("DATE-OBS", string(startBuf), "Start Date");
      m_ScienceTable->addKey("DATE-END", string(stopBuf), "Stop Date");
      m_ScienceTable->addKey("TSTART", m_FirstEventTime_RTS, "Start time");
      m_ScienceTable->addKey("TSTOP", m_LastEventTime_RTS, "Stop time");

      // Also update OBS_ID to match the start date (YYMMDD format)
      char obsIdBuf[8];
      strftime(obsIdBuf, sizeof(obsIdBuf), "%y%m%d", gmtime(&startUnix));
      m_PrimaryHDU->addKey("OBS_ID", string(obsIdBuf), "Observation ID");
      m_ScienceTable->addKey("OBS_ID", string(obsIdBuf), "Observation ID");

      if (g_Verbosity >= c_Info) {
        cout<<m_XmlTag<<": Updated time headers — DATE-OBS="<<startBuf
            <<", DATE-END="<<stopBuf<<", TSTART="<<m_FirstEventTime_RTS
            <<", TSTOP="<<m_LastEventTime_RTS<<endl;
      }
    } catch (const CCfits::FitsException& e) {
      if (g_Verbosity >= c_Error) {
        cout<<m_XmlTag<<": Error updating time headers: "<<e.message()<<endl;
      }
    }
  }

  MModule::Finalize();

  if (g_Verbosity >= c_Info) {
    string levelStr = (m_OutputDataLevel == 2) ? "L2" : "L1b";
    cout<< m_XmlTag <<": MModuleSaverMeasurementsFITS ("<<levelStr<<")"<<endl;
    cout<< m_XmlTag <<":   * total events written: "<<m_TotalEventsWritten<<endl;
    if (m_OutputDataLevel == 2) {
      cout<< m_XmlTag <<":   * total events skipped (screening): "<<m_TotalEventsSkipped<<endl;
    }
  }

  // Close the FITS file (CCfits automatically closes on delete)
  if (m_FITSFile != nullptr) {
    delete m_FITSFile;
    m_FITSFile = nullptr;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaverMeasurementsFITS::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != nullptr) {
    m_FileName = FileNameNode->GetValue();
  }

  MXmlNode* OutputLevelNode = Node->GetNode("OutputLevel");
  if (OutputLevelNode != nullptr) {
    MString Level = OutputLevelNode->GetValue();
    if (Level == "L2" || Level == "l2") {
      m_OutputDataLevel = 2;
    } else {
      m_OutputDataLevel = 1;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleSaverMeasurementsFITS::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);
  new MXmlNode(Node, "OutputLevel", (m_OutputDataLevel == 2) ? "L2" : "L1b");

  return Node;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleSaverMeasurementsFITS::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsSaverMeasurementsFITS* Options = new MGUIOptionsSaverMeasurementsFITS(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


// MModuleSaverMeasurementsFITS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
