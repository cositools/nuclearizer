/*
 * MModuleLoaderMeasurementsFITS.cxx
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
// MModuleLoaderMeasurementsFITS
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleLoaderMeasurementsFITS.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsLoaderMeasurementsFITS.h"
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MReadOutElementDoubleStrip.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutDataTiming.h"
#include "MReadOutDataOrigins.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleLoaderMeasurementsFITS)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsFITS::MModuleLoaderMeasurementsFITS() : MModuleLoaderMeasurements()
{
  // Construct an instance of MModuleLoaderMeasurementsFITS

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Measurement loader for FITS files";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagMeasurementLoaderFITS";

  // This is a special start module which can generate its own events
  m_IsStartModule = true;

  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;

  m_FITSFile = nullptr;
  m_PrimaryHDU = nullptr;
  m_ComptonTable = nullptr;
  m_CurrentRow = 1;  // FITS uses 1-based row indexing
  m_TotalRows = 0;
}

////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsFITS::~MModuleLoaderMeasurementsFITS()
{
  // Delete this instance of MModuleLoaderMeasurementsFITS
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsFITS::Initialize()
{
  // Initialize the module

  // Clean:
  m_FileType = "Unknown";
  
  if (MFile::Exists(m_FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": The file "<<m_FileName<<" does not exist."<<endl;
    return false;
  }

  // Open the FITS file
  if (OpenFITSFile(m_FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open FIT file."<<endl;
    return false;
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsFITS::OpenFITSFile(MString FileName)
{
  // Open the FITS file using CCfits
  try {
    
    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": FITFileName: "<<string(FileName)<<endl;

    // Open the FITS file in read-only, default: rwmode=Read, readDataFlag = false
    m_FITSFile = new FITS(string(FileName));

    // Store const pointer to Primary HDU (HDU 0) - header/metadata
    m_PrimaryHDU = &m_FITSFile->pHDU();

    // Try to get Compton extension by index 1 (first extension after primary)
    // Note: FITS files have HDU 0 = Primary, HDU 1 = first extension
    try {
      m_ComptonTable = &m_FITSFile->extension(1);
      if (g_Verbosity >= c_Info) {
        cout<<m_XmlTag<<": Opened extension at index 1: "<<m_ComptonTable->name()<<endl;
      }
    } catch (FitsException& e) {
      if (g_Verbosity >= c_Error) {
        cout<<m_XmlTag<<": Failed to open extension by index 1, trying by name..."<<endl;
      }
      cout<<""<<endl;
      m_ComptonTable = &m_FITSFile->extension("Science Data Table 1st Extension");
    }

    // Validate the file by checking the 1st extension header keywords
    string extName;
    m_ComptonTable->readKey("EXTNAME", extName);
    if (extName != "GED_L1A") {
      if (g_Verbosity >= c_Error) {
        cout << m_XmlTag << ": Invalid EXTNAME: expected 'GED_L1A', got '" << extName << "'" << endl;
      }
      return false;
    }

    string origin;
    m_ComptonTable->readKey("ORIGIN", origin);
    if (origin != "SSL") {
      if (g_Verbosity >= c_Error) {
        cout << m_XmlTag << ": Invalid ORIGIN: expected 'SSL', got '" << origin << "'" << endl;
      }
      return false;
    }

    //Nothing in the GTI Table now.
    //m_GtiTable = &m_FITSFile->extension(2);

    // Get table metadata
    m_TotalRows = m_ComptonTable->rows();
    int nCols = m_ComptonTable->numCols();

    if (g_Verbosity >= c_Info) {
      cout << m_XmlTag << ": FITS table metadata" << endl;
      cout << m_XmlTag << ":   Rows=" << m_TotalRows << endl;
      cout << m_XmlTag << ":   Columns=" << nCols << endl;
    }

    // Get all columns
    const ColMap& columns = m_ComptonTable->column();

    // Print column information to verify structure, can comment out
    // cout<<"  Column details:"<<endl;
    // for (auto& col : columns) {
    //   cout<<"    - "<<col.first
    //       <<" (type: "<<col.second->type()
    //       <<", width: "<<col.second->width()
    //       <<", repeat: "<<col.second->repeat()
    //       <<")"<<endl;
    // }

    if (g_Verbosity >= c_Info) {
      cout << m_XmlTag << ": FITS file opened successfully" << endl;
    }

    return true;

  } catch (FitsException& e) {
    if (g_Verbosity >= c_Error) cout << m_XmlTag << ": Error opening FITS file: " << e.message() << endl;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsFITS::ReadBatch()
{
  // Read next event (loads new batch from FITS file if needed)

  // Check if we need to load a new batch
  if (m_CurrentEventInBatch >= m_BatchEventCount) {

    // Check if we reach the end of file
    if (m_BatchStartRow > m_TotalRows) {
      return false; // No more data
    }

    try {
      // Calculate how many rows to read in this batch
      long rowsToRead = std::min(m_BatchSize, m_TotalRows - m_BatchStartRow + 1);
      long lastRow = m_BatchStartRow + rowsToRead - 1;

      // Resize vectors to read this batch
      m_BatchTIME.resize(rowsToRead);
      m_BatchEVENTTYPE.resize(rowsToRead);
      m_BatchNUMSTRIPHIT.resize(rowsToRead);
      m_BatchTYPEHIT.resize(rowsToRead);
      m_BatchDETID.resize(rowsToRead);
      m_BatchSTRIPID.resize(rowsToRead);
      m_BatchSIDEID.resize(rowsToRead);
      m_BatchFASTTIME.resize(rowsToRead);
      m_BatchPHA.resize(rowsToRead);
      m_BatchTAC.resize(rowsToRead);

      // Read scalar columns - third parameter is LAST row index, NOT count
      m_ComptonTable->column("TIME").read(m_BatchTIME, m_BatchStartRow, lastRow);
      m_ComptonTable->column("EVENTTYPE").read(m_BatchEVENTTYPE, m_BatchStartRow, lastRow);
      m_ComptonTable->column("NUMSTRIPHIT").read(m_BatchNUMSTRIPHIT, m_BatchStartRow, lastRow);

      // Read variable-length array columns
      m_ComptonTable->column("TYPEHIT").readArrays(m_BatchTYPEHIT, m_BatchStartRow, lastRow);
      m_ComptonTable->column("DETID").readArrays(m_BatchDETID, m_BatchStartRow, lastRow);
      m_ComptonTable->column("STRIPID").readArrays(m_BatchSTRIPID, m_BatchStartRow, lastRow);
      m_ComptonTable->column("SIDEID").readArrays(m_BatchSIDEID, m_BatchStartRow, lastRow);
      m_ComptonTable->column("FASTTIME").readArrays(m_BatchFASTTIME, m_BatchStartRow, lastRow);
      m_ComptonTable->column("PHA").readArrays(m_BatchPHA, m_BatchStartRow, lastRow);
      m_ComptonTable->column("TAC").readArrays(m_BatchTAC, m_BatchStartRow, lastRow);

      // Update batch tracking
      m_BatchEventCount = rowsToRead;
      m_CurrentEventInBatch = 0;
      m_BatchStartRow += rowsToRead;

      // Display first 3 events in this batch to verify event, can comment out
      // cout<<"  First "<<std::min(3L, rowsToRead)<<" events in this batch:"<<endl;
      // for (long i = 0; i < std::min(3L, rowsToRead); ++i) {
      //   cout<<"    Event "<<i<<" (row "<<(m_BatchStartRow - rowsToRead + i)<<"): "
      //       <<"TIME="<<m_BatchTIME[i]
      //       <<", EVENTTYPE="<<(int)m_BatchEVENTTYPE[i]
      //       <<", NUMSTRIPHIT="<<(int)m_BatchNUMSTRIPHIT[i]
      //       <<", TYPEHIT="<<m_BatchTYPEHIT[i][0]
      //       <<", DETID="<<m_BatchDETID[i][0]
      //       <<", STRIPID="<<m_BatchSTRIPID[i][0]
      //       <<", SIDEID="<<m_BatchSIDEID[i][0]
      //       <<", FASTTIME="<<m_BatchFASTTIME[i][0]
      //       <<", PHA="<<m_BatchPHA[i][0]
      //       <<", TAC="<<m_BatchTAC[i][0]
      //       <<", NUMSTRIPHIT="<<(int)m_BatchNUMSTRIPHIT[i];

      //   cout<<endl;
      // }

    } catch (FitsException& e) {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error reading FITS batch: "<<e.message()<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsFITS::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine

  //Supervisor call this, if we are at 0 index, then we will read a batch. Then for index 1-99, ReadBatch() will return true,
  //which mean we will not read a new batch. Instead we will read from the current batch we have

  if (ReadBatch() == false) {
    return false; // No more events
  }

  // Get current event index (before incrementing)
  long idx = m_CurrentEventInBatch;

  // Extract event-level data
  double eventTime = m_BatchTIME[idx];
  // uint8_t eventType = m_BatchEVENTTYPE[idx]; //unused variable
  uint8_t numStripHit = m_BatchNUMSTRIPHIT[idx];

  // Set event-level properties
  // Event->SetID();  // TODO: No EventID
  Event->SetCL(eventTime);     // Mission time in seconds

  // Loop through strip hits and create MStripHit objects
  for (uint8_t hitIdx = 0; hitIdx < numStripHit; ++hitIdx) {
    // Extract hit data from arrays
    uint8_t typeHit = m_BatchTYPEHIT[idx][hitIdx];
    int detID = m_BatchDETID[idx][hitIdx];
    int stripID = m_BatchSTRIPID[idx][hitIdx];
    int sideID = m_BatchSIDEID[idx][hitIdx];
    uint8_t fastTime = m_BatchFASTTIME[idx][hitIdx];
    int pha = m_BatchPHA[idx][hitIdx];
    int tac = m_BatchTAC[idx][hitIdx];

    // Create new strip hit
    MStripHit* H = new MStripHit();

    // Set detector and strip information
    H->SetDetectorID(detID);
    H->SetStripID(stripID);
    H->IsLowVoltageStrip(sideID == 0); 

    // Set measured data
    H->SetADCUnits(pha);  // I think pha is the ADCunits?
    H->SetTAC(tac);       // Timing

    // Set boolean flags based on hit type
    H->IsGuardRing(typeHit == 2);
    if (H->IsGuardRing() == true) {
      Event->SetGuardRingVeto(true);
    }
    H->IsNearestNeighbor(typeHit == 1);
    H->HasFastTiming(fastTime == 1);

    // Add strip hit to event
    Event->AddStripHit(H);

    // DEBUG
    // if (m_CurrentRow > m_TotalRows - 10) {
    //   cout<<"Event "<<m_CurrentRow<<" (TIME="<<eventTime<<", numStripHit="<<(int)numStripHit<<")"
    //       <<" Hit "<<(int)hitIdx<<": detID="<<detID<<", stripID="<<stripID
    //       <<", sideID="<<sideID<<", typeHit="<<(int)typeHit
    //       <<", fastTime="<<(int)fastTime<<", pha="<<pha<<", tac="<<tac<<endl;
    // }
  }

  // Mark that this event has been loaded
  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement);

  // Increment batch index after processing this event
  m_CurrentEventInBatch++;
  m_CurrentRow++;

  m_NEventsInFile++;
  m_NGoodEventsInFile++;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsFITS::Finalize()
{
  // Finalize the module

  MModule::Finalize();

  if (g_Verbosity >= c_Info) {
    cout<< m_XmlTag <<": MModuleLoaderMeasurementsFITS"<<endl;
    cout<< m_XmlTag <<":  * all events on file: "<<m_NEventsInFile<<endl;
    cout<< m_XmlTag <<":  * good events on file: "<<m_NGoodEventsInFile<<endl;
  }

  // Close the FITS file (CCfits automatically closes on delete)
  if (m_FITSFile != nullptr) {
    delete m_FITSFile;
    m_FITSFile = nullptr;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsFITS::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* FileNameFITSNode = Node->GetNode("FileNameFITS");
  if (FileNameFITSNode != nullptr) {
    m_FileName = FileNameFITSNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleLoaderMeasurementsFITS::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileNameFITS", m_FileName);

  return Node;
}


///////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsFITS::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsLoaderMeasurementsFITS* Options = new MGUIOptionsLoaderMeasurementsFITS(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


// MModuleLoaderMeasurementsFITS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
