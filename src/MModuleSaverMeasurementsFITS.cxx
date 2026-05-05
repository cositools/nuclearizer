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
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsSaverMeasurementsFITS.h"
#include "MHit.h"
#include "MPhysicalEvent.h"
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
  m_OutputLevel = 0; // 0 = L1b (default), 1 = L2
  m_FirstEventTime = 0.0;
  m_LastEventTime = 0.0;
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

    string levelStr = (m_OutputLevel == 1) ? "L2" : "L1b";
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

    // Define columns for science data table per specification
    // PE(100) = variable-length single-precision float array (max 100)
    // 4E = fixed-length array of 4 single-precision floats
    // 3E = fixed-length array of 3 single-precision floats
    // L1b has BAD_FLAG, L2 does not (per spec: L2 removes bad events and BAD_FLAG column)
    std::vector<string> colNames = {
      "TIME", "EVENTTYPE", "EVENTCLASS", "NUMHIT", "SEQHIT",
      "STATTEST", "RECOILDIR", "RECOILDIR_ERR",
      "X", "Y", "Z",
      "X_ERR", "Y_ERR", "Z_ERR",
      "ENERGY", "ENERGY_ERR"
    };

    std::vector<string> colFormats = {
      "1D",      // TIME - scalar double
      "1B",      // EVENTTYPE - scalar byte
      "1B",      // EVENTCLASS - scalar byte
      "1B",      // NUMHIT - scalar byte
      "1B",      // SEQHIT - scalar byte
      "4E",      // STATTEST
      "3E",      // RECOILDIR
      "3E",      // RECOILDIR_ERR
      "PE(100)", // X - variable-length float array
      "PE(100)", // Y
      "PE(100)", // Z
      "PE(100)", // X_ERR
      "PE(100)", // Y_ERR
      "PE(100)", // Z_ERR
      "PE(100)", // ENERGY
      "PE(100)"  // ENERGY_ERR
    };

    std::vector<string> colUnits = {
      "s", "", "", "", "",
      "", "", "",
      "cm", "cm", "cm",
      "unit", "unit", "unit",
      "keV", "unit"
    };

    // L1b includes BAD_FLAG column, L2 does not
    if (m_OutputLevel == 0) {
      colNames.push_back("BAD_FLAG");
      colFormats.push_back("PE(100)");
      colUnits.push_back("");
    }

    // Create binary table extension
    string extName = (m_OutputLevel == 1) ? "GED_L2" : "GED_L1B";
    m_ScienceTable = m_FITSFile->addTable(extName, 0, colNames, colFormats, colUnits);

    // Add keywords to science table
    m_ScienceTable->addKey("EXTNAME", extName, "name of this HDU");
    m_ScienceTable->addKey("TELESCOP", "COSI", "Telescope mission name");
    m_ScienceTable->addKey("INSTRUME", "GED", "Instrument name");
    m_ScienceTable->addKey("DATAMODE", "TBD", "Instrument datamode");
    // removed observer
    m_ScienceTable->addKey("OBS_ID", "YYMMDD", "Observation ID"); //should match the YYMMDD of the filename
    // removed object
    m_ScienceTable->addKey("MJDREFI", 60676, "MJD reference day 01 Jan 2025 00:00:00");
    m_ScienceTable->addKey("MJDREFF", 8.007407407407E-04, "MJD reference (fraction of day)");
    m_ScienceTable->addKey("TIMEREF", "LOCAL", "Reference Frame");
    m_ScienceTable->addKey("TASSIGN", "SATELLITE", "Time assigned");
    m_ScienceTable->addKey("TIMESYS", "TT", "Time System");
    m_ScienceTable->addKey("TIMEUNIT", "s", "Time unit for timing header keywords");
    m_ScienceTable->addKey("TIMEDEL", 0.0, "Integration time");
    m_ScienceTable->addKey("CLOCKAPP", false, "If clock corrections are applied (T/F)");
    m_ScienceTable->addKey("DATE-OBS", "yyyy-mm-ddThh:mm:ss", "Start Date"); //placeholder, this will be wroten after we read through all the event
    m_ScienceTable->addKey("DATE-END", "yyyy-mm-ddThh:mm:ss", "Stop Date"); // 
    m_ScienceTable->addKey("TSTART", 0.0, "Start time"); //placeholder, this will be wroten after we read through all the event
    m_ScienceTable->addKey("TSTOP", 0.0, "Stop time"); //
    m_ScienceTable->addKey("HDUCLASS", "OGIP", "format conforms to OGIP standard");
    m_ScienceTable->addKey("HDUCLAS1", "ARRAY", "hduclass1");
    m_ScienceTable->addKey("HDUCLAS2", "TOTAL", "hduclas2");
    m_ScienceTable->addKey("CREATOR", "TBD", "Software that create 1st the file");
    m_ScienceTable->addKey("PROCVER", "TBD", "Processing Version");
    m_ScienceTable->addKey("CALDBVER", "TBD", "CALDB version");
    m_ScienceTable->addKey("SEQPHUM", "TBD", "Times the dataset has been processed");
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
  if (m_OutputLevel == 1 && Event->IsBad()) {
    m_TotalEventsSkipped++;
    Event->SetAnalysisProgress(MAssembly::c_EventSaver);
    return true;
  }

  // Extract event-level data
  MTime eventTime = Event->GetTime();
  //Get the seconds since epoch in double format
  double time = eventTime.GetAsSeconds();
  unsigned int numHits = Event->GetNHits();

  // loop through all event, and record the start and end time for TSTART/TSTOP
  if (!m_HasEvents) {
    m_FirstEventTime = time;
    m_LastEventTime = time;
    m_HasEvents = true;
  } else {
    if (time < m_FirstEventTime) m_FirstEventTime = time;
    if (time > m_LastEventTime) m_LastEventTime = time;
  }

  // Event-level metadata defaults
  uint8_t eventType = 0;    // 0 = unknown/default
  uint8_t eventClass = 2;   // 2 = unreconstructed
  uint8_t seqHit = 0;

  // Fixed-length arrays for event-level data (initialize to zeros)
  std::valarray<float> statTest(0.0f, 4);
  std::valarray<float> recoilDir(0.0f, 3);
  std::valarray<float> recoilDirErr(0.0f, 3);

  // Extract revan reconstruction data if available
  MPhysicalEvent* PE = Event->GetPhysicalEvent();
  if (PE != nullptr) {
    int peType = PE->GetType();

    if (peType == MPhysicalEvent::c_Compton) {
      eventClass = 0;  // 0 = Compton

      MComptonEvent* CE = dynamic_cast<MComptonEvent*>(PE);
      if (CE != nullptr) {
        seqHit = (uint8_t)CE->SequenceLength();

        MVector de = CE->De();
        recoilDir[0] = (float)de.X();
        recoilDir[1] = (float)de.Y();
        recoilDir[2] = (float)de.Z();

        MVector dde = CE->dDe();
        recoilDirErr[0] = (float)dde.X();
        recoilDirErr[1] = (float)dde.Y();
        recoilDirErr[2] = (float)dde.Z();

        // TODO: Statistical test values (spec TBD) 
        statTest[0] = (float)CE->Phi();
        statTest[1] = (float)CE->DeltaTheta();
        statTest[2] = (float)CE->MinLeverArm();
        statTest[3] = 0.0f;
      }

    } else if (peType == MPhysicalEvent::c_Photo) {
      eventClass = 1;  // 1 = photoabsorption
      seqHit = 1;

    } else {
      eventClass = 2;  // 2 = unreconstructed
    }
  }

  // Override eventClass for bad events (per spec: 3 = bad)
  if (Event->IsBad()) {
    eventClass = 3;
  }

  // Hit-level arrays
  std::valarray<float> x(numHits);
  std::valarray<float> y(numHits);
  std::valarray<float> z(numHits);
  std::valarray<float> x_err(numHits);
  std::valarray<float> y_err(numHits);
  std::valarray<float> z_err(numHits);
  std::valarray<float> energy(numHits);
  std::valarray<float> energy_err(numHits);
  std::valarray<float> bad_flag(0.0f, numHits);

  // TODO: Set bad flag if event failed any calibration step (L1b only)
  // Detail TBD, for now setting bad_flag based on IsBad()
  if (m_OutputLevel == 0 && Event->IsBad()) {
    bad_flag = 1.0f;
  }

  // Extract hit-level data
  for (unsigned int i = 0; i < numHits; ++i) {
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

  // Add to batch
  m_BatchTIME.push_back(time);
  m_BatchEVENTTYPE.push_back(eventType);
  m_BatchEVENTCLASS.push_back(eventClass);
  m_BatchNUMHIT.push_back((uint8_t)numHits);
  m_BatchSEQHIT.push_back(seqHit);
  m_BatchSTATTEST.push_back(statTest);
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
  if (m_OutputLevel == 0) {
    m_BatchBAD_FLAG.push_back(bad_flag);
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

    // Write scalar columns
    m_ScienceTable->column("TIME").write(m_BatchTIME, m_BatchStartRow);
    m_ScienceTable->column("EVENTTYPE").write(m_BatchEVENTTYPE, m_BatchStartRow);
    m_ScienceTable->column("EVENTCLASS").write(m_BatchEVENTCLASS, m_BatchStartRow);
    m_ScienceTable->column("NUMHIT").write(m_BatchNUMHIT, m_BatchStartRow);
    m_ScienceTable->column("SEQHIT").write(m_BatchSEQHIT, m_BatchStartRow);

    // Write fixed-length array columns (event-level)
    m_ScienceTable->column("STATTEST").writeArrays(m_BatchSTATTEST, m_BatchStartRow);
    m_ScienceTable->column("RECOILDIR").writeArrays(m_BatchRECOILDIR, m_BatchStartRow);
    m_ScienceTable->column("RECOILDIR_ERR").writeArrays(m_BatchRECOILDIR_ERR, m_BatchStartRow);

    // Write variable-length array columns (hit-level)
    m_ScienceTable->column("X").writeArrays(m_BatchX, m_BatchStartRow);
    m_ScienceTable->column("Y").writeArrays(m_BatchY, m_BatchStartRow);
    m_ScienceTable->column("Z").writeArrays(m_BatchZ, m_BatchStartRow);
    m_ScienceTable->column("X_ERR").writeArrays(m_BatchX_ERR, m_BatchStartRow);
    m_ScienceTable->column("Y_ERR").writeArrays(m_BatchY_ERR, m_BatchStartRow);
    m_ScienceTable->column("Z_ERR").writeArrays(m_BatchZ_ERR, m_BatchStartRow);
    m_ScienceTable->column("ENERGY").writeArrays(m_BatchENERGY, m_BatchStartRow);
    m_ScienceTable->column("ENERGY_ERR").writeArrays(m_BatchENERGY_ERR, m_BatchStartRow);
    if (m_OutputLevel == 0) {
      m_ScienceTable->column("BAD_FLAG").writeArrays(m_BatchBAD_FLAG, m_BatchStartRow);
    }

    // Update tracking
    m_TotalEventsWritten += m_BatchEventCount;
    m_BatchStartRow += m_BatchEventCount;

    // Clear batch vectors - scalar columns
    m_BatchTIME.clear();
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
    m_BatchBAD_FLAG.clear();

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

      time_t startUnix = MISSION_EPOCH_UNIX + (time_t)m_FirstEventTime;
      time_t stopUnix = MISSION_EPOCH_UNIX + (time_t)m_LastEventTime;

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
      m_ScienceTable->addKey("TSTART", m_FirstEventTime, "Start time");
      m_ScienceTable->addKey("TSTOP", m_LastEventTime, "Stop time");

      // Also update OBS_ID to match the start date (YYMMDD format)
      char obsIdBuf[8];
      strftime(obsIdBuf, sizeof(obsIdBuf), "%y%m%d", gmtime(&startUnix));
      m_PrimaryHDU->addKey("OBS_ID", string(obsIdBuf), "Observation ID");
      m_ScienceTable->addKey("OBS_ID", string(obsIdBuf), "Observation ID");

      if (g_Verbosity >= c_Info) {
        cout<<m_XmlTag<<": Updated time headers — DATE-OBS="<<startBuf
            <<", DATE-END="<<stopBuf<<", TSTART="<<m_FirstEventTime
            <<", TSTOP="<<m_LastEventTime<<endl;
      }
    } catch (const CCfits::FitsException& e) {
      if (g_Verbosity >= c_Error) {
        cout<<m_XmlTag<<": Error updating time headers: "<<e.message()<<endl;
      }
    }
  }

  MModule::Finalize();

  if (g_Verbosity >= c_Info) {
    string levelStr = (m_OutputLevel == 1) ? "L2" : "L1b";
    cout<< m_XmlTag <<": MModuleSaverMeasurementsFITS ("<<levelStr<<")"<<endl;
    cout<< m_XmlTag <<":   * total events written: "<<m_TotalEventsWritten<<endl;
    if (m_OutputLevel == 1) {
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
      m_OutputLevel = 1;
    } else {
      m_OutputLevel = 0;
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
  new MXmlNode(Node, "OutputLevel", (m_OutputLevel == 1) ? "L2" : "L1b");

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
