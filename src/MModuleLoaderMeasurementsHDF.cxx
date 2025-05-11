/*
 * MModuleLoaderMeasurementsHDF.cxx
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
// MModuleLoaderMeasurementsHDF
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleLoaderMeasurementsHDF.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsLoaderMeasurementsHDF.h"
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MReadOutElementDoubleStrip.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutDataTiming.h"
#include "MReadOutDataOrigins.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleLoaderMeasurementsHDF)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsHDF::MModuleLoaderMeasurementsHDF() : MModuleLoaderMeasurements()
{
  // Construct an instance of MModuleLoaderMeasurementsHDF
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Measurement loader for HDF files";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagMeasurementLoaderHDF";
  
  // This is a special start module which can generate its own events
  m_IsStartModule = true;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;

  m_FileNameStripMap = "";
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsHDF::~MModuleLoaderMeasurementsHDF()
{
  // Delete this instance of MModuleLoaderMeasurementsHDF
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsHDF::Initialize()
{
  // Initialize the module 
  
  // Clean:
  m_FileType = "Unknown";
  m_Detector = "Unknown";
  m_Version = -1;
  m_StartObservationTime = MTime(0);
  m_EndObservationTime = MTime(0);
  m_StartClock = numeric_limits<long>::max();
  m_EndClock = numeric_limits<long>::max();
  
  m_TotalHits = 0;
  m_CurrentHit = 0;

  m_NumberOfEventIDRollOvers = 0;
  m_LastEventID = 0;

  // Need to put this all in a try-catch block

  try {
    m_FileHDF5 = H5File(m_FileName, H5F_ACC_RDONLY);

    // ToDo: Check for version.
    m_HitVersion = "1.0";
    if (H5Lexists(m_FileHDF5.getId(), "HDFVersion", H5P_DEFAULT) > 0) {
      DataSet VersionDataset = m_FileHDF5.openDataSet("/HDFVersion");

      // Create compound type for version
      CompType VersionType(sizeof(MReadOutHDFVersionString));
      StrType StringType(PredType::C_S1, 256);
      VersionType.insertMember("string_col", HOFFSET(MReadOutHDFVersionString, string_col), StringType);

      MReadOutHDFVersionString VS;
      VersionDataset.read(&VS, VersionType);

      if (string(VS.string_col) == "1.2") {
        m_HitVersion = string(VS.string_col);
      } else {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<string(VS.string_col)<<endl<<"Please update this module."<<endl;
        return false;
      }
    }
    cout<<m_XmlTag<<": HDF5 hit version found: "<<m_HitVersion<<endl;

    m_DataSet = m_FileHDF5.openDataSet("/Hits");

    if (m_HitVersion == "1.0") {
      m_CompoundDataType = CompType(sizeof(MReadOutHDF_1_1));
      m_CompoundDataType.insertMember("EVENT_ID",              HOFFSET(MReadOutHDF_1_1, m_EventID),              PredType::STD_U16LE);
      m_CompoundDataType.insertMember("TIMECODE",              HOFFSET(MReadOutHDF_1_1, m_TimeCode),              PredType::STD_U32LE);
      m_CompoundDataType.insertMember("HIT_TYPE",              HOFFSET(MReadOutHDF_1_1, m_HitType),               PredType::STD_U8LE);
      m_CompoundDataType.insertMember("TIMING_TYPE",           HOFFSET(MReadOutHDF_1_1, m_TimingType),           PredType::STD_U8LE);
      m_CompoundDataType.insertMember("STRIP_ID",              HOFFSET(MReadOutHDF_1_1, m_StripID),              PredType::STD_U16LE);
      m_CompoundDataType.insertMember("CRYSTAL_ID",            HOFFSET(MReadOutHDF_1_1, m_CrystalID),            PredType::STD_U8LE);
      m_CompoundDataType.insertMember("GAIN",                  HOFFSET(MReadOutHDF_1_1, m_Gain),                  PredType::STD_U8LE);
      m_CompoundDataType.insertMember("OVERFLOW",              HOFFSET(MReadOutHDF_1_1, m_Overflow),          PredType::STD_U8LE);
      m_CompoundDataType.insertMember("CURRENT_MAXIMUM",       HOFFSET(MReadOutHDF_1_1, m_CurrentMaximum),       PredType::STD_U16LE);
      m_CompoundDataType.insertMember("HIGH_CURRENT_SAMPLES",  HOFFSET(MReadOutHDF_1_1, m_HighCurrentSamples),  PredType::STD_U16LE);
      m_CompoundDataType.insertMember("ENERGY_DATA",           HOFFSET(MReadOutHDF_1_1, m_EnergyData),           PredType::STD_U16LE);
      m_CompoundDataType.insertMember("ENERGY_DATA_LOW_GAIN",  HOFFSET(MReadOutHDF_1_1, m_EnergyDataLowGain),  PredType::STD_U16LE);
      m_CompoundDataType.insertMember("ENERGY_DATA_HIGH_GAIN", HOFFSET(MReadOutHDF_1_1, m_EnergyDataHighGain), PredType::STD_U16LE);
      m_CompoundDataType.insertMember("TIMING_DATA",           HOFFSET(MReadOutHDF_1_1, m_TimingData),           PredType::STD_U16LE);
      m_CompoundDataType.insertMember("PAD",                   HOFFSET(MReadOutHDF_1_1, m_Pad),                   PredType::STD_U8LE);
      m_CompoundDataType.insertMember("HITS",                  HOFFSET(MReadOutHDF_1_1, m_Hits),                  PredType::STD_U8LE);
      m_CompoundDataType.insertMember("EVENT_TYPE",            HOFFSET(MReadOutHDF_1_1, m_EventType),            PredType::STD_U8LE);
      m_CompoundDataType.insertMember("CRC",                   HOFFSET(MReadOutHDF_1_1, m_CRC),                   PredType::STD_U8LE);
    } else if (m_HitVersion == "1.2") {
      m_CompoundDataType = CompType(sizeof(MReadOutHDF_1_2));
      m_CompoundDataType.insertMember("EVENT_ID",              HOFFSET(MReadOutHDF_1_2, m_EventID),              PredType::STD_U16LE);
      m_CompoundDataType.insertMember("TIMECODE",              HOFFSET(MReadOutHDF_1_2, m_TimeCode),              PredType::STD_U64LE);
      m_CompoundDataType.insertMember("GSE_TIMECODE",          HOFFSET(MReadOutHDF_1_2, m_GSETimeCode),              PredType::IEEE_F64LE);
      m_CompoundDataType.insertMember("HIT_TYPE",              HOFFSET(MReadOutHDF_1_2, m_HitType),               PredType::STD_U8LE);
      m_CompoundDataType.insertMember("TIMING_TYPE",           HOFFSET(MReadOutHDF_1_2, m_TimingType),           PredType::STD_U8LE);
      m_CompoundDataType.insertMember("STRIP_ID",              HOFFSET(MReadOutHDF_1_2, m_StripID),              PredType::STD_U16LE);
      m_CompoundDataType.insertMember("CRYSTAL_ID",            HOFFSET(MReadOutHDF_1_2, m_CrystalID),            PredType::STD_U8LE);
      m_CompoundDataType.insertMember("GAIN",                  HOFFSET(MReadOutHDF_1_2, m_Gain),                  PredType::STD_U8LE);
      m_CompoundDataType.insertMember("OVERFLOW",              HOFFSET(MReadOutHDF_1_2, m_Overflow),          PredType::STD_U8LE);
      m_CompoundDataType.insertMember("CURRENT_MAXIMUM",       HOFFSET(MReadOutHDF_1_2, m_CurrentMaximum),       PredType::STD_U16LE);
      m_CompoundDataType.insertMember("HIGH_CURRENT_SAMPLES",  HOFFSET(MReadOutHDF_1_2, m_HighCurrentSamples),  PredType::STD_U16LE);
      m_CompoundDataType.insertMember("ENERGY_DATA",           HOFFSET(MReadOutHDF_1_2, m_EnergyData),           PredType::STD_U16LE);
      m_CompoundDataType.insertMember("ENERGY_DATA_LOW_GAIN",  HOFFSET(MReadOutHDF_1_2, m_EnergyDataLowGain),  PredType::STD_U16LE);
      m_CompoundDataType.insertMember("ENERGY_DATA_HIGH_GAIN", HOFFSET(MReadOutHDF_1_2, m_EnergyDataHighGain), PredType::STD_U16LE);
      m_CompoundDataType.insertMember("TIMING_DATA",           HOFFSET(MReadOutHDF_1_2, m_TimingData),           PredType::STD_U16LE);
      m_CompoundDataType.insertMember("PAD",                   HOFFSET(MReadOutHDF_1_2, m_Pad),                   PredType::STD_U8LE);
      m_CompoundDataType.insertMember("HITS",                  HOFFSET(MReadOutHDF_1_2, m_Hits),                  PredType::STD_U8LE);
      m_CompoundDataType.insertMember("BYTES",                 HOFFSET(MReadOutHDF_1_2, m_Bytes),                  PredType::STD_U16LE);
      m_CompoundDataType.insertMember("EVENT_TYPE",            HOFFSET(MReadOutHDF_1_2, m_EventType),            PredType::STD_U8LE);
      m_CompoundDataType.insertMember("CRC",                   HOFFSET(MReadOutHDF_1_2, m_CRC),                   PredType::STD_U8LE);
    } else {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HitVersion<<endl<<"Please update this module."<<endl;
      return false;
    }

    DataSpace DS = m_DataSet.getSpace();
    DS.getSimpleExtentDims(&m_TotalHits, nullptr);
    m_CurrentHit = 0;

  } catch (const Exception& E) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": HDF5 initializion error: "<<E.getDetailMsg()<<endl;
    return false;
  }

  if (m_StripMap.Open(m_FileNameStripMap) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to read strip map"<<endl;
  }

  m_NEventsInFile = 0;
  m_NGoodEventsInFile = 0;
    
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsHDF::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.

  // Check if we need to read more data
  if (m_CurrentHit >= m_TotalHits) {
    cout<<m_Name<<": No more events!"<<endl;
    m_IsFinished = true;
    return false;
  }

  try {

    unsigned int NStripHits = 1;
    for (unsigned int s = 0; s < NStripHits; ++s) {
      hsize_t offset[1] = { m_CurrentHit };
      hsize_t count[1] = { 1 };

      DataSpace DS = m_DataSet.getSpace();
      DS.selectHyperslab(H5S_SELECT_SET, count, offset);
      DataSpace MS(1, count);

      // Extract the data we need
      uint16_t EventID;
      uint64_t TimeCode;
      uint16_t StripID;
      uint16_t ADCs;
      uint16_t TACs;
      uint8_t NumberOfHits;

      if (m_HitVersion == "1.0") {
        MReadOutHDF_1_1 h;
        m_DataSet.read(&h, m_CompoundDataType, MS, DS);
        ++m_CurrentHit;

        EventID = h.m_EventID;
        TimeCode = h.m_TimeCode;
        StripID = h.m_StripID;
        ADCs = h.m_EnergyData;
        TACs = h.m_TimingData;
        NumberOfHits = h.m_Hits;
      } else if (m_HitVersion == "1.2") {
        MReadOutHDF_1_2 h;
        m_DataSet.read(&h, m_CompoundDataType, MS, DS);
        ++m_CurrentHit;

        EventID = h.m_EventID;
        TimeCode = h.m_TimeCode;
        StripID = h.m_StripID;
        ADCs = h.m_EnergyData;
        TACs = h.m_TimingData;
        NumberOfHits = h.m_Hits;
      } else {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HitVersion<<endl<<"Please update this module."<<endl;
        return false;
      }

      //if (g_Verbosity >= c_Info) {
        cout<<endl;
        cout<<"Hit "<<m_CurrentHit<<endl;
        cout<<"  EventID: "<<EventID<<endl;
        cout<<"  TimeCode: "<<TimeCode<<endl;
        cout<<"  StripID: "<<StripID<<endl;
        cout<<"  EnergyData: "<<ADCs<<endl;
        cout<<"  TimingData: "<<TACs<<endl;
        cout<<"  Hits: "<<(int) NumberOfHits<<endl;
      //}

      if (EventID < m_LastEventID) {
        m_NumberOfEventIDRollOvers++;
      }
      m_LastEventID = EventID;

      unsigned long LongEventID = EventID + m_NumberOfEventIDRollOvers*(numeric_limits<uint16_t>::max() + 1);

      Event->SetID(LongEventID);
      Event->SetCL(TimeCode);

      if (m_StripMap.HasReadOutID(StripID) == true) {
        MStripHit* H = new MStripHit();
        H->SetDetectorID(m_StripMap.GetDetectorID(StripID));
        H->SetStripID(m_StripMap.GetStripNumber(StripID));
        H->IsLowVoltageStrip(m_StripMap.IsLowVoltage(StripID));
        H->SetADCUnits(ADCs);
        H->SetTAC(TACs);
        Event->AddStripHit(H);
      } else {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Read-out ID "<<StripID<<" not found in strip map"<<endl;
        return false;
      }

      NStripHits = static_cast<unsigned int>(NumberOfHits);
    }

  } catch (const Exception& E) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": HDF5 read error: "<<E.getDetailMsg()<<endl;
    return false;
  }

  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsHDF::Finalize()
{
  // Initialize the module 
  
  MModule::Finalize();
  
  cout<<"MModuleLoaderMeasurementsHDF: "<<endl;
  cout<<"  * all events on file: "<<m_NEventsInFile<<endl;
  cout<<"  * good events on file: "<<m_NGoodEventsInFile<<endl;

  m_FileHDF5.close();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsHDF::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameHDF5Node = Node->GetNode("FileNameHDF5");
  if (FileNameHDF5Node != nullptr) {
    m_FileName = FileNameHDF5Node->GetValue();
  }

  MXmlNode* FileNameStripMapNode = Node->GetNode("FileNameStripMap");
  if (FileNameStripMapNode != nullptr) {
    m_FileNameStripMap = FileNameStripMapNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleLoaderMeasurementsHDF::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileNameHDF5", m_FileName);
  new MXmlNode(Node, "FileNameStripMap", m_FileNameStripMap);

  return Node;
}


///////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsHDF::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsLoaderMeasurementsHDF* Options = new MGUIOptionsLoaderMeasurementsHDF(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


// MModuleLoaderMeasurementsHDF.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
