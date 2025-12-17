/*
 * MModuleSaverMeasurementsFITS.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsSaverMeasurementsFITS.h"
#include "MHit.h"


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
  m_Name = "Save events to FITS files (L1b)";

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
  m_BatchStartRow = 1;
  m_BatchEventCount = 0;
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

    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Creating FITS file "<<string(FileName)<<endl;

    // Create new FITS file (overwrite if exists)
    m_FITSFile = new FITS(string(FileName), RWmode::Write);

    // Get pointer to Primary HDU
    m_PrimaryHDU = &m_FITSFile->pHDU();

    // Add some keywords to primary HDU
    m_PrimaryHDU->addKey("CREATOR", "Nuclearizer", "Software that created this file");
    m_PrimaryHDU->addKey("ORIGIN", "UC Berkeley SSL", "Organization");
    m_PrimaryHDU->addKey("TELESCOP", "COSI", "Mission name");
    m_PrimaryHDU->addKey("INSTRUME", "GeD", "Instrument name");

    // Define columns for science data table per specification
    // PE(100) = variable-length single-precision float array (max 100)
    // 4E = fixed-length array of 4 single-precision floats
    // 3E = fixed-length array of 3 single-precision floats
    std::vector<string> colNames = {
      "TIME", "EVENTTYPE", "EVENTCLASS", "NUMHIT", "SEQHIT",
      "STATTEST", "RECOILDIR", "RECOILDIR_ERR",
      "X", "Y", "Z",
      "X_ERR", "Y_ERR", "Z_ERR",
      "ENERGY", "ENERGY_ERR", "BAD_FLAG"
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
      "PE(100)", // ENERGY_ERR
      "PE(100)"  // BAD_FLAG
    };

    std::vector<string> colUnits = {
      "s", "", "", "", "",
      "", "unit", "unit",
      "cm", "unit", "unit",
      "unit", "unit", "unit",
      "keV", "unit", ""
    };

    // Create binary table extension
    m_ScienceTable = m_FITSFile->addTable("Compton_L1b_1st_Ext", 0, colNames, colFormats, colUnits);

    // Add keywords to science table
    m_ScienceTable->addKey("EXTNAME", "COMPTON_L1B", "name of this HDU");
    m_ScienceTable->addKey("TELESCOP", "COSI", "Telescope mission name");
    m_ScienceTable->addKey("INSTRUME", "GED", "Instrument name");
    m_ScienceTable->addKey("DATAMODE", "string", "Instrument datamode");
    m_ScienceTable->addKey("OBSERVER", "string", "Principal Investigator");
    m_ScienceTable->addKey("OBS_ID", "YYMMDD", "Observation ID");
    m_ScienceTable->addKey("OBJECT", "string", "Object/Target name");
    m_ScienceTable->addKey("MJDREFI", 60676, "MJD reference day 01 Jan 2025 00:00:00");
    m_ScienceTable->addKey("MJDREFF", 8.007407407407E-04, "MJD reference (fraction of day)");
    m_ScienceTable->addKey("TIMEREF", "LOCAL", "Reference Frame");
    m_ScienceTable->addKey("TASSIGN", "SATELLITE", "Time assigned");
    m_ScienceTable->addKey("TIMESYS", "TT", "Time System");
    m_ScienceTable->addKey("TIMEUNIT", "s", "Time unit for timing header keywords");
    m_ScienceTable->addKey("TIMEDEL", 0.0, "Integration time");
    m_ScienceTable->addKey("CLOCKAPP", false, "If clock corrections are applied (T/F)");
    m_ScienceTable->addKey("DATE-OBS", "yyyy-mm-ddThh:mm:ss", "Start Date");
    m_ScienceTable->addKey("DATE-END", "yyyy-mm-ddThh:mm:ss", "Stop Date");
    m_ScienceTable->addKey("TSTART", 0.0, "Start time");
    m_ScienceTable->addKey("TSTOP", 0.0, "Stop time");
    m_ScienceTable->addKey("HDUCLASS", "OGIP", "format conforms to OGIP standard");
    m_ScienceTable->addKey("HDUCLAS1", "ARRAY", "hduclass1");
    m_ScienceTable->addKey("HDUCLAS2", "TOTAL", "hduclas2");

    cout<<"FITS file created successfully"<<endl;

    return true;

  } catch (FitsException& e) {
    cout<<"Error creating FITS file: "<<e.message()<<endl;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleSaverMeasurementsFITS::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Add this event to the batch, write batch when full

  // Extract event-level data
  double time = Event->GetCL();
  unsigned int numHits = Event->GetNHits();

  // Event-level metadata (placeholders for now - can be filled in later)
  uint8_t eventType = 0;    // 0 = unknown/default
  uint8_t eventClass = 0;   // 0 = unknown (can check for Compton/photoabsorption later)
  uint8_t seqHit = 0;       // 0 = first/only sequence

  // Fixed-length arrays for event-level data (initialize to zeros)
  std::valarray<float> statTest(0.0f, 4);         // 4 statistical test values
  std::valarray<float> recoilDir(0.0f, 3);        // Recoil electron direction (x,y,z)
  std::valarray<float> recoilDirErr(0.0f, 3);     // Recoil direction error

  // Resize arrays for this event's hits (using float to match PE format)
  std::valarray<float> x(numHits);
  std::valarray<float> y(numHits);
  std::valarray<float> z(numHits);
  std::valarray<float> x_err(numHits);
  std::valarray<float> y_err(numHits);
  std::valarray<float> z_err(numHits);
  std::valarray<float> energy(numHits);
  std::valarray<float> energy_err(numHits);
  std::valarray<float> bad_flag(0.0f, numHits);   // Initialize flags to 0

  // Extract hit-level data
  for (unsigned int i = 0; i < numHits; ++i) {
    MHit* hit = Event->GetHit(i);

    // Get position
    MVector position = hit->GetPosition();
    x[i] = (float)position.X();
    y[i] = (float)position.Y();
    z[i] = (float)position.Z();

    // Get position errors
    MVector positionResolution = hit->GetPositionResolution();
    x_err[i] = (float)positionResolution.X();
    y_err[i] = (float)positionResolution.Y();
    z_err[i] = (float)positionResolution.Z();

    // Get energy
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
  m_BatchBAD_FLAG.push_back(bad_flag);

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
      cout<<"Writing batch: "<<m_BatchEventCount<<" events (rows "<<m_BatchStartRow<<" to "<<lastRow<<")"<<endl;
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
    m_ScienceTable->column("BAD_FLAG").writeArrays(m_BatchBAD_FLAG, m_BatchStartRow);

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

  } catch (FitsException& e) {
    cout<<"Error writing FITS batch: "<<e.message()<<endl;
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

  MModule::Finalize();

  cout<<"MModuleSaverMeasurementsFITS: "<<endl;
  cout<<"  * total events written: "<<m_TotalEventsWritten<<endl;

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

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleSaverMeasurementsFITS::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);

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
