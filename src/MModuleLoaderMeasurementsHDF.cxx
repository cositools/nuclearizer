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

  m_LoadContinuationFiles = false;
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

  if (OpenHDF5File(m_FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open HDF5 file."<<endl;
    return false;
  }
  m_ContinuationFileID = 0;

  if (m_StripMap.Open(m_FileNameStripMap) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to read strip map."<<endl;
    return false;
  }

  m_NEventsInFile = 0;
  m_NGoodEventsInFile = 0;
    
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


//! Convert more data from raw to intermediate format - return false if no more data can be converted
bool MModuleLoaderMeasurementsHDF::OpenHDF5File(MString FileName)
{
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

    // Get the data set
    m_DataSet = m_FileHDF5.openDataSet("/Hits");

    // Get the data space
    DataSpace DS = m_DataSet.getSpace();

    // Get creation property list
    DSetCreatPropList PropertyList = m_DataSet.getCreatePlist();
    int Rank = DS.getSimpleExtentNdims();

    // Check if chunked
    if (PropertyList.getLayout() == H5D_CHUNKED) {
      hsize_t ChunkDims[H5S_MAX_RANK];
      PropertyList.getChunk(Rank, ChunkDims);

      cout<<"Chunk dimensions: ";
      for (int i = 0; i < Rank; ++i) {
        cout<<ChunkDims[i]<<" ";
      }
      cout<<endl;
    } else {
      cout<<"Dataset is not chunked (layout is not H5D_CHUNKED)."<<endl;
    }

    if (m_HitVersion == "1.0") {
      m_CompoundDataType = CompType(sizeof(MReadOutHDF_1_0));
      m_CompoundDataType.insertMember("EVENT_ID",              HOFFSET(MReadOutHDF_1_0, m_EventID),              PredType::STD_U16LE);
      m_CompoundDataType.insertMember("TIMECODE",              HOFFSET(MReadOutHDF_1_0, m_TimeCode),              PredType::STD_U32LE);
      m_CompoundDataType.insertMember("HIT_TYPE",              HOFFSET(MReadOutHDF_1_0, m_HitType),               PredType::STD_U8LE);
      m_CompoundDataType.insertMember("TIMING_TYPE",           HOFFSET(MReadOutHDF_1_0, m_TimingType),           PredType::STD_U8LE);
      m_CompoundDataType.insertMember("STRIP_ID",              HOFFSET(MReadOutHDF_1_0, m_StripID),              PredType::STD_U16LE);
      m_CompoundDataType.insertMember("CRYSTAL_ID",            HOFFSET(MReadOutHDF_1_0, m_CrystalID),            PredType::STD_U8LE);
      m_CompoundDataType.insertMember("GAIN",                  HOFFSET(MReadOutHDF_1_0, m_Gain),                  PredType::STD_U8LE);
      m_CompoundDataType.insertMember("OVERFLOW",              HOFFSET(MReadOutHDF_1_0, m_Overflow),          PredType::STD_U8LE);
      m_CompoundDataType.insertMember("CURRENT_MAXIMUM",       HOFFSET(MReadOutHDF_1_0, m_CurrentMaximum),       PredType::STD_U16LE);
      m_CompoundDataType.insertMember("HIGH_CURRENT_SAMPLES",  HOFFSET(MReadOutHDF_1_0, m_HighCurrentSamples),  PredType::STD_U16LE);
      m_CompoundDataType.insertMember("ENERGY_DATA",           HOFFSET(MReadOutHDF_1_0, m_EnergyData),           PredType::STD_U16LE);
      m_CompoundDataType.insertMember("ENERGY_DATA_LOW_GAIN",  HOFFSET(MReadOutHDF_1_0, m_EnergyDataLowGain),  PredType::STD_U16LE);
      m_CompoundDataType.insertMember("ENERGY_DATA_HIGH_GAIN", HOFFSET(MReadOutHDF_1_0, m_EnergyDataHighGain), PredType::STD_U16LE);
      m_CompoundDataType.insertMember("TIMING_DATA",           HOFFSET(MReadOutHDF_1_0, m_TimingData),           PredType::STD_U16LE);
      m_CompoundDataType.insertMember("PAD",                   HOFFSET(MReadOutHDF_1_0, m_Pad),                   PredType::STD_U8LE);
      m_CompoundDataType.insertMember("HITS",                  HOFFSET(MReadOutHDF_1_0, m_Hits),                  PredType::STD_U8LE);
      m_CompoundDataType.insertMember("EVENT_TYPE",            HOFFSET(MReadOutHDF_1_0, m_EventType),            PredType::STD_U8LE);
      m_CompoundDataType.insertMember("CRC",                   HOFFSET(MReadOutHDF_1_0, m_CRC),                   PredType::STD_U8LE);
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

    DS.getSimpleExtentDims(&m_TotalHits, nullptr);
    m_CurrentHit = 0;

    if (ReadBatchHits() == false) {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": HDF5 read batch error"<<endl;
      return false;
    }

  } catch (const H5::Exception& E) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": HDF5 initializion error: "<<E.getDetailMsg()<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Read a batch of hits using a hyperslab
bool MModuleLoaderMeasurementsHDF::ReadBatchHits()
{
  try {
    m_CurrentBatchSize = m_DefaultBatchSize;
    if (m_TotalHits - m_CurrentHit < m_CurrentBatchSize) {
      m_CurrentBatchSize = m_TotalHits - m_CurrentHit;
    }

    hsize_t Offset[1] = { m_CurrentHit };
    hsize_t Count[1] = { m_CurrentBatchSize };

    DataSpace DS = m_DataSet.getSpace();
    DS.selectHyperslab(H5S_SELECT_SET, Count, Offset);

    DataSpace MS(1, Count);

    if (m_HitVersion == "1.0") {
      m_Buffer_1_0.resize(m_CurrentBatchSize);
      m_DataSet.read(m_Buffer_1_0.data(), m_CompoundDataType, MS, DS);
    } else if (m_HitVersion == "1.2") {
      m_Buffer_1_2.resize(m_CurrentBatchSize);
      m_DataSet.read(m_Buffer_1_2.data(), m_CompoundDataType, MS, DS);
    } else {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HitVersion<<endl<<"Please update this module."<<endl;
      return false;
    }

    m_CurrentBatchIndex = 0;

  } catch (const H5::Exception& E) {
    cout<<m_XmlTag<<": HDF5 read error: "<<E.getDetailMsg()<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsHDF::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.

  unsigned int NStripHits = 1;
  for (unsigned int s = 0; s < NStripHits; ++s) {

    // First step is to check if we have events left in the file:
    if (m_CurrentHit >= m_TotalHits) {
      if (m_LoadContinuationFiles == true) {
        // Check if we have more files to load:
        MString FileName = m_FileName;
        MString NextSuffix = MString("_") + (m_ContinuationFileID+1) + ".hdf5";
        FileName.ReplaceAllInPlace(".hdf5", NextSuffix);
        if (MFile::Exists(FileName) == true) {
          if (OpenHDF5File(FileName) == false) {
            cout<<m_XmlTag<<": No more events!"<<endl;
            m_IsFinished = true;
            return false;
          } else {
            cout<<m_XmlTag<<": Switched to file: "<<FileName<<endl;
            m_ContinuationFileID++;
          }
        } else {
          cout<<m_XmlTag<<": No more events!"<<endl;
          m_IsFinished = true;
          return false;
        }
      } else {
        cout<<m_XmlTag<<": No more events!"<<endl;
        m_IsFinished = true;
        return false;
      }
    }

    // Second step is to check is we have events left in the batch
    if (m_CurrentBatchIndex >= m_CurrentBatchSize) {
      if (ReadBatchHits() == false) {
        return false;
      }
    }

    // Extract the data we need
    uint16_t EventID;
    uint64_t TimeCode;
    uint16_t StripID;
    uint16_t ADCs;
    uint16_t TACs;
    uint8_t NumberOfHits;

    if (m_HitVersion == "1.0") {
      MReadOutHDF_1_0& Hit = m_Buffer_1_0[m_CurrentBatchIndex];
      ++m_CurrentBatchIndex;
      ++m_CurrentHit;

      EventID = Hit.m_EventID;
      TimeCode = Hit.m_TimeCode;
      StripID = Hit.m_StripID;
      ADCs = Hit.m_EnergyData;
      TACs = Hit.m_TimingData;
      NumberOfHits = Hit.m_Hits;
    } else if (m_HitVersion == "1.2") {
      MReadOutHDF_1_2& Hit = m_Buffer_1_2[m_CurrentBatchIndex];
      ++m_CurrentBatchIndex;
      ++m_CurrentHit;

      EventID = Hit.m_EventID;
      TimeCode = Hit.m_TimeCode;
      StripID = Hit.m_StripID;
      ADCs = Hit.m_EnergyData;
      TACs = Hit.m_TimingData;
      NumberOfHits = Hit.m_Hits;
    } else {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HitVersion<<endl<<"Please update this module."<<endl;
      return false;
    }

    if (g_Verbosity >= c_Info) {
      cout<<endl;
      cout<<"Hit "<<m_CurrentHit<<endl;
      cout<<"  EventID: "<<EventID<<endl;
      cout<<"  TimeCode: "<<TimeCode<<endl;
      cout<<"  StripID: "<<StripID<<endl;
      cout<<"  EnergyData: "<<ADCs<<endl;
      cout<<"  TimingData: "<<TACs<<endl;
      cout<<"  Hits: "<<(int) NumberOfHits<<endl;
    }

    if (EventID < m_LastEventID) {
      m_NumberOfEventIDRollOvers++;
    }
    m_LastEventID = EventID;

    unsigned long LongEventID = EventID + m_NumberOfEventIDRollOvers*(numeric_limits<uint16_t>::max() + 1);

    Event->SetID(LongEventID);
    if (m_HitVersion == "1.0") {
      Event->SetCL(TimeCode);
    } else {
      Event->SetTI(TimeCode);
    }

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

  MXmlNode* LoadContinuationFilesNode = Node->GetNode("LoadContinuationFiles");
  if (LoadContinuationFilesNode != nullptr) {
    m_LoadContinuationFiles = LoadContinuationFilesNode->GetValueAsBoolean();
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
  new MXmlNode(Node, "LoadContinuationFiles", m_LoadContinuationFiles);
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
