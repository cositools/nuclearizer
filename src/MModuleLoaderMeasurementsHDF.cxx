/*
 * MModuleLoaderMeasurementsHDF.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Felix Hagemann.
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
#include <regex>
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
  
  m_IncludeNearestNeighbor = true;
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
  
  // Start time of the file taken from the file name
  // to be used to find absolute time for Spacewire brick
  //TODO: Get more accurate start time from data files?
  m_StartObservationTime = MTime(0);
  /*
  m_EndObservationTime = MTime(0);
  m_StartClock = numeric_limits<long>::max();
  m_EndClock = numeric_limits<long>::max();
  */
  
  m_TotalHits = 0;
  m_CurrentHit = 0;

  m_NumberOfEventIDRollOvers = 0;
  m_LastEventID = 0;

  // Clear all data buffers and related variables
  m_Buffer_1_0.clear();
  m_Buffer_1_2.clear();
  m_Buffer_2.clear();
  m_EventIndices_2.clear();
  m_EventData_2_0.clear();
  m_EventData_2_2.clear();
  m_CurrentBatchSize = 0;
  m_CurrentBatchIndex = 0;
  m_MinHitIndex = 0;

  // Clear ASIC polarities
  m_ASICPolarities.clear();

  if (MFile::Exists(m_FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": The file "<<m_FileName<<" does not exist."<<endl;
    return false;
  }

  if (OpenHDF5File(m_FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open HDF5 file."<<endl;
    return false;
  }
  m_ContinuationFileID = 0;

  if (MFile::Exists(m_FileNameStripMap) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": The file "<<m_FileNameStripMap<<" does not exist."<<endl;
    return false;
  }

  if (m_StripMap.Open(m_FileNameStripMap) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to read strip map."<<endl;
    return false;
  }

  // Update the ASIC polarities in the strip map (only if existent)
  if (!m_ASICPolarities.empty() && m_StripMap.UpdateASICPolarities(m_ASICPolarities) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to update ASIC polarities based on the config JSON."<<endl;
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
  try { // HDF5 throws exceptions, thus need to encapsulate everything in try..catch

    MFile::ExpandFileName(FileName);
    
    // Get the observation start time from the file name 
    if (FileName.EndsWith(".hdf5") == true && FileName.Contains("gse_") == true)  {
      MString FileDateTime = FileName.Extract("gse_",".hdf5");
      unsigned int Year = FileDateTime.GetSubString(0,4).ToInt();
      unsigned int Month = FileDateTime.GetSubString(4,2).ToInt();
      unsigned int Day = FileDateTime.GetSubString(6,2).ToInt();
      unsigned int Hour = FileDateTime.GetSubString(9,2).ToInt();
      unsigned int Min = FileDateTime.GetSubString(11,2).ToInt();
      unsigned int Sec = FileDateTime.GetSubString(13,2).ToInt();
      m_StartObservationTime = MTime(Year, Month, Day, Hour, Min, Sec, 0);
      if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Found start time from file name (UTC): "<<m_StartObservationTime<<endl;
    } else {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to determine start time from file name: "<<FileName<<endl;
      return false;
    }
    
    m_HDFFile = H5File(FileName, H5F_ACC_RDONLY);

    // JSON config string containing the information on the ASIC polarities
    string ConfigJSON;

    // ToDo: Check for version
    // Version 1.0 and 1.1 did not have /HDFVersion,
    // Some GSE versions (6.1.1 ?) did not write /HDFVersion to EVERY file of a 
    // multi-file measurement but only to the first one --> How to deal with this?
    m_HDFStripHitVersion = MHDFStripHitVersion::V1_0;
    if (H5Lexists(m_HDFFile.getId(), "HDFVersion", H5P_DEFAULT) > 0) {
      DataSet VersionDataset = m_HDFFile.openDataSet("/HDFVersion");

      // Create compound type for version
      CompType VersionType(sizeof(MHDFStripHitVersionString));
      StrType StringType(PredType::C_S1, 256);
      VersionType.insertMember("string_col", HOFFSET(MHDFStripHitVersionString, string_col), StringType);

      MHDFStripHitVersionString VS;
      VersionDataset.read(&VS, VersionType);

      if (string(VS.string_col) == "1.2") {
        m_HDFStripHitVersion = MHDFStripHitVersion::V1_2;
      } else {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<string(VS.string_col)<<endl<<"Please update this module."<<endl;
        return false;
      }

      // Check that the HDF5 file contains a dataset /Config with the JSON config string
      if (H5Lexists(m_HDFFile.getId(), "Config", H5P_DEFAULT) > 0) {
        DataSet ConfigDataset = m_HDFFile.openDataSet("/Config");
        StrType ConfigStringType(PredType::C_S1, 262144);

        // Create compound type for reading the JSON config string
        CompType ConfigType(sizeof(MHDFJSONConfigString));
        ConfigType.insertMember("string_col", HOFFSET(MHDFJSONConfigString, string_col), ConfigStringType);

        MHDFJSONConfigString CS;
        ConfigDataset.read(&CS, ConfigType);
        ConfigJSON = string(CS.string_col);
      }

    // Check for existence of HDFVersion in /Events/HDFVersion (HDF v2)
    } else if (H5Lexists(m_HDFFile.getId(), "Events", H5P_DEFAULT) > 0 && H5Lexists(m_HDFFile.getId(), "EventIndices", H5P_DEFAULT) > 0) {
      m_EventDataSet = m_HDFFile.openDataSet("/Events");
      m_EventIndicesDataSet = m_HDFFile.openDataSet("/EventIndices");

      // Read HDF5 version from Events/HDF5Version to a string
      Attribute VersionAttribute = m_EventDataSet.openAttribute("HDFVersion");
      string VersionString;
      VersionAttribute.read(VersionAttribute.getStrType(), VersionString);

      if (VersionString == "2.0" || VersionString == "2.1") {
        m_HDFStripHitVersion = MHDFStripHitVersion::V2_0;
      } else if (VersionString.length() >= 2 && VersionString.compare(0, 2, "2.") == 0) {
        m_HDFStripHitVersion = MHDFStripHitVersion::V2_2;
      } else {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<VersionString<<endl<<"Please update this module."<<endl;
        return false;
      }

      Attribute Config = m_EventDataSet.openAttribute("Config");
      Config.read(Config.getStrType(), ConfigJSON);
    }

    cout<<m_XmlTag<<": HDF5 hit version found: "<<m_HDFStripHitVersion<<endl;

    // Read ASIC polarities from the JSON config string (if existent)
    m_ASICPolarities.clear();
    if (!ConfigJSON.empty()) {
      bool ASICIsPrimary;

      // Regex to match either "primary"/"secondary", or the polarity stored in "SP"
      regex pattern(R"(\"(primary|secondary)\"|\"SP\"\s*:\s*(\d+))");
      for (sregex_iterator i = sregex_iterator(ConfigJSON.begin(), ConfigJSON.end(), pattern); i != sregex_iterator(); ++i) {
        
        smatch match = *i;

        // Check Group 1: Marker (primary/secondary)
        if (match[1].matched) {

          ASICIsPrimary = match[1].str() == "primary";

          if (m_ASICPolarities.empty() || m_ASICPolarities.back().find(ASICIsPrimary) != m_ASICPolarities.back().end()) {

            // Check that the previous entry has both primary or secondary before creating a new one
            if (!m_ASICPolarities.empty() && (
                 m_ASICPolarities.back().find(true) == m_ASICPolarities.back().end() || 
                 m_ASICPolarities.back().find(false) == m_ASICPolarities.back().end())
            ) {
              if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Parsing ASIC polarities for detector "<<m_ASICPolarities.size()-1<<" unsuccessful"<<endl;
                return false;
            }

            m_ASICPolarities.push_back(map<bool, vector<bool>>());
          }

          // Initialize the vector for this ASIC key if it doesn't exist
          m_ASICPolarities.back()[ASICIsPrimary] = vector<bool>();
        }
        
        // Check Group 2: SP value
        else if (match[2].matched) {
          if (m_ASICPolarities.empty()) {
            if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": SP found without active ASIC section"<<endl;
            return false;
          }
          
          string val = match[2].str();
          if (val != "0" && val != "1") {
            if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Cannot interpret polarity \""<<val<<"\" (allowed are \"0\" and \"1\")"<< endl;
            return false;
          }

          // val == 1 <=> LV; val == 0 <=> HV
          m_ASICPolarities.back()[ASICIsPrimary].push_back(val == "1");
        }
      }

      // Output results for verification
      if (g_Verbosity >= c_Info) {
        for (size_t i = 0; i < m_ASICPolarities.size(); ++i) {
          cout << "Detector ID " << i << ":" << endl;
          for (bool key : {true, false} ) {
            cout << "  " << (key ? "Primary" : "Secondary") << ": ";
            for (bool s : m_ASICPolarities[i][key]) cout << (s ? "LV" : "HV") << " ";
            cout << endl;
          }
        }
      }
    }

    // Get the data set
    if (m_HDFStripHitVersion <= MHDFStripHitVersion::V1_2) {
      m_HDFDataSet = m_HDFFile.openDataSet("/Hits");
    } else {
      m_HDFDataSet = m_HDFFile.openDataSet("/FEEHits");
    }

    // Get the data space
    DataSpace DS = m_HDFDataSet.getSpace();

    // Get creation property list
    DSetCreatPropList PropertyList = m_HDFDataSet.getCreatePlist();
    int Rank = DS.getSimpleExtentNdims();

    // Check if chunked
    if (PropertyList.getLayout() == H5D_CHUNKED) {
      hsize_t ChunkDims[H5S_MAX_RANK];
      PropertyList.getChunk(Rank, ChunkDims);
      if (g_Verbosity > c_Info) {
        cout<<"Chunk dimensions: ";
        for (int i = 0; i < Rank; ++i) {
          cout<<ChunkDims[i]<<" ";
        }
        cout<<endl;
      }
    } else {
      if (g_Verbosity > c_Info) {
        cout<<"Dataset is not chunked (layout is not H5D_CHUNKED)."<<endl;
      }
    }

    if (m_HDFStripHitVersion == MHDFStripHitVersion::V1_0) {
      m_HDFCompoundDataType = CompType(sizeof(MHDFStripHit_V1_0));
      m_HDFCompoundDataType.insertMember("EVENT_ID",              HOFFSET(MHDFStripHit_V1_0, m_EventID),             PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("TIMECODE",              HOFFSET(MHDFStripHit_V1_0, m_TimeCode),            PredType::STD_U32LE);
      m_HDFCompoundDataType.insertMember("HIT_TYPE",              HOFFSET(MHDFStripHit_V1_0, m_HitType),             PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("TIMING_TYPE",           HOFFSET(MHDFStripHit_V1_0, m_TimingType),          PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("STRIP_ID",              HOFFSET(MHDFStripHit_V1_0, m_StripID),             PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("CRYSTAL_ID",            HOFFSET(MHDFStripHit_V1_0, m_CrystalID),           PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("GAIN",                  HOFFSET(MHDFStripHit_V1_0, m_Gain),                PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("OVERFLOW",              HOFFSET(MHDFStripHit_V1_0, m_Overflow),            PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("CURRENT_MAXIMUM",       HOFFSET(MHDFStripHit_V1_0, m_CurrentMaximum),      PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("HIGH_CURRENT_SAMPLES",  HOFFSET(MHDFStripHit_V1_0, m_HighCurrentSamples),  PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("ENERGY_DATA",           HOFFSET(MHDFStripHit_V1_0, m_EnergyData),          PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("ENERGY_DATA_LOW_GAIN",  HOFFSET(MHDFStripHit_V1_0, m_EnergyDataLowGain),   PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("ENERGY_DATA_HIGH_GAIN", HOFFSET(MHDFStripHit_V1_0, m_EnergyDataHighGain),  PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("TIMING_DATA",           HOFFSET(MHDFStripHit_V1_0, m_TimingData),          PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("PAD",                   HOFFSET(MHDFStripHit_V1_0, m_Pad),                 PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("HITS",                  HOFFSET(MHDFStripHit_V1_0, m_Hits),                PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("EVENT_TYPE",            HOFFSET(MHDFStripHit_V1_0, m_EventType),           PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("CRC",                   HOFFSET(MHDFStripHit_V1_0, m_CRC),                 PredType::STD_U8LE);
    } else if (m_HDFStripHitVersion == MHDFStripHitVersion::V1_2) {
      m_HDFCompoundDataType = CompType(sizeof(MHDFStripHit_V1_2));
      m_HDFCompoundDataType.insertMember("EVENT_ID",              HOFFSET(MHDFStripHit_V1_2, m_EventID),             PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("TIMECODE",              HOFFSET(MHDFStripHit_V1_2, m_TimeCode),            PredType::STD_U64LE);
      m_HDFCompoundDataType.insertMember("GSE_TIMECODE",          HOFFSET(MHDFStripHit_V1_2, m_GSETimeCode),         PredType::IEEE_F64LE);
      m_HDFCompoundDataType.insertMember("HIT_TYPE",              HOFFSET(MHDFStripHit_V1_2, m_HitType),             PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("TIMING_TYPE",           HOFFSET(MHDFStripHit_V1_2, m_TimingType),          PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("STRIP_ID",              HOFFSET(MHDFStripHit_V1_2, m_StripID),             PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("CRYSTAL_ID",            HOFFSET(MHDFStripHit_V1_2, m_CrystalID),           PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("GAIN",                  HOFFSET(MHDFStripHit_V1_2, m_Gain),                PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("OVERFLOW",              HOFFSET(MHDFStripHit_V1_2, m_Overflow),            PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("CURRENT_MAXIMUM",       HOFFSET(MHDFStripHit_V1_2, m_CurrentMaximum),      PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("HIGH_CURRENT_SAMPLES",  HOFFSET(MHDFStripHit_V1_2, m_HighCurrentSamples),  PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("ENERGY_DATA",           HOFFSET(MHDFStripHit_V1_2, m_EnergyData),          PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("ENERGY_DATA_LOW_GAIN",  HOFFSET(MHDFStripHit_V1_2, m_EnergyDataLowGain),   PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("ENERGY_DATA_HIGH_GAIN", HOFFSET(MHDFStripHit_V1_2, m_EnergyDataHighGain),  PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("TIMING_DATA",           HOFFSET(MHDFStripHit_V1_2, m_TimingData),          PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("PAD",                   HOFFSET(MHDFStripHit_V1_2, m_Pad),                 PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("HITS",                  HOFFSET(MHDFStripHit_V1_2, m_Hits),                PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("BYTES",                 HOFFSET(MHDFStripHit_V1_2, m_Bytes),               PredType::STD_U16LE);
      m_HDFCompoundDataType.insertMember("EVENT_TYPE",            HOFFSET(MHDFStripHit_V1_2, m_EventType),           PredType::STD_U8LE);
      m_HDFCompoundDataType.insertMember("CRC",                   HOFFSET(MHDFStripHit_V1_2, m_CRC),                 PredType::STD_U8LE);
    } else if (m_HDFStripHitVersion <= MHDFStripHitVersion::V2_2) {

      // Create compound data type for reading detector hit information from /FEEHits
      m_HDFFEECompoundDataType = CompType(sizeof(MHDFStripHit_V2));
      m_HDFFEECompoundDataType.insertMember("event_index",        HOFFSET(MHDFStripHit_V2, m_EventIndex),            PredType::STD_U32LE);
      m_HDFFEECompoundDataType.insertMember("hit_type",           HOFFSET(MHDFStripHit_V2, m_HitType),               PredType::STD_U8LE);
      m_HDFFEECompoundDataType.insertMember("timing_type",        HOFFSET(MHDFStripHit_V2, m_TimingType),            PredType::STD_U8LE);
      m_HDFFEECompoundDataType.insertMember("strip_id",           HOFFSET(MHDFStripHit_V2, m_StripID),               PredType::STD_U16LE);
      m_HDFFEECompoundDataType.insertMember("energy",             HOFFSET(MHDFStripHit_V2, m_EnergyData),            PredType::STD_U16LE);
      m_HDFFEECompoundDataType.insertMember("timing",             HOFFSET(MHDFStripHit_V2, m_TimingData),            PredType::STD_U16LE);

      // Create compound data type for reading event information from /Events
      if (m_HDFStripHitVersion == MHDFStripHitVersion::V2_0) {
        m_EventCompoundDataType = CompType(sizeof(MHDFEvent_V2_0));
        m_EventCompoundDataType.insertMember("event_id",          HOFFSET(MHDFEvent_V2_0, m_EventID),                PredType::STD_U16LE);
        m_EventCompoundDataType.insertMember("timecode",          HOFFSET(MHDFEvent_V2_0, m_TimeCode),               PredType::STD_U64LE);
        m_EventCompoundDataType.insertMember("gse_timecode",      HOFFSET(MHDFEvent_V2_0, m_GSETimeCode),            PredType::STD_U64LE);
        m_EventCompoundDataType.insertMember("hits",              HOFFSET(MHDFEvent_V2_0, m_Hits),                   PredType::STD_U8LE);
        m_EventCompoundDataType.insertMember("bytes",             HOFFSET(MHDFEvent_V2_0, m_Bytes),                  PredType::STD_U16LE);
        m_EventCompoundDataType.insertMember("event_type",        HOFFSET(MHDFEvent_V2_0, m_EventType),              PredType::STD_U8LE);
        m_EventCompoundDataType.insertMember("crc",               HOFFSET(MHDFEvent_V2_0, m_CRC),                    PredType::STD_U8LE);
      } else if (m_HDFStripHitVersion <= MHDFStripHitVersion::V2_2) {
        m_EventCompoundDataType = CompType(sizeof(MHDFEvent_V2_2));
        m_EventCompoundDataType.insertMember("event_id",          HOFFSET(MHDFEvent_V2_2, m_EventID),                PredType::STD_U16LE);
        m_EventCompoundDataType.insertMember("timecode",          HOFFSET(MHDFEvent_V2_2, m_TimeCode),               PredType::STD_U64LE);
        m_EventCompoundDataType.insertMember("gse_timecode",      HOFFSET(MHDFEvent_V2_2, m_GSETimeCode),            PredType::STD_U64LE);
        m_EventCompoundDataType.insertMember("spw_timecode",      HOFFSET(MHDFEvent_V2_2, m_SPWTimeCode),            PredType::STD_U64LE);
        m_EventCompoundDataType.insertMember("hits",              HOFFSET(MHDFEvent_V2_2, m_Hits),                   PredType::STD_U8LE);
        m_EventCompoundDataType.insertMember("bytes",             HOFFSET(MHDFEvent_V2_2, m_Bytes),                  PredType::STD_U16LE);
        m_EventCompoundDataType.insertMember("event_type",        HOFFSET(MHDFEvent_V2_2, m_EventType),              PredType::STD_U8LE);
        m_EventCompoundDataType.insertMember("crc",               HOFFSET(MHDFEvent_V2_2, m_CRC),                    PredType::STD_U8LE);
      } else {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HDFStripHitVersion<<endl<<"Please update this module."<<endl;
        return false;
      }

      // Create compound data type for reading event indices information from /EventIndices
      hsize_t array_dims[1] = {2};
      ArrayType uint32_pair(PredType::STD_U32LE, 1, array_dims);
      m_EventIndicesCompoundDataType = CompType(sizeof(MHDFEventIndices_V2));
      m_EventIndicesCompoundDataType.insertMember("fee_hits",            HOFFSET(MHDFEventIndices_V2, m_FEEHits),            uint32_pair);
      m_EventIndicesCompoundDataType.insertMember("acs_hits",            HOFFSET(MHDFEventIndices_V2, m_ACSHits),            uint32_pair);
      m_EventIndicesCompoundDataType.insertMember("hs",                  HOFFSET(MHDFEventIndices_V2, m_HS),                 uint32_pair);
      m_EventIndicesCompoundDataType.insertMember("singles_counts",      HOFFSET(MHDFEventIndices_V2, m_SinglesCounts),      uint32_pair);
      m_EventIndicesCompoundDataType.insertMember("dib_coincidence",     HOFFSET(MHDFEventIndices_V2, m_DIBCoincidence),     uint32_pair);
      m_EventIndicesCompoundDataType.insertMember("detector_hits",       HOFFSET(MHDFEventIndices_V2, m_DetectorHits),       uint32_pair);
      m_EventIndicesCompoundDataType.insertMember("detector_live_time",  HOFFSET(MHDFEventIndices_V2, m_DetectorLiveTime),   uint32_pair);

      DataSpace EventDataSpace = m_EventDataSet.getSpace();
      EventDataSpace.getSimpleExtentDims(&m_TotalHits, nullptr);

    } else {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HDFStripHitVersion<<endl<<"Please update this module."<<endl;
      return false;
    }

    if (m_HDFStripHitVersion <= MHDFStripHitVersion::V1_2) DS.getSimpleExtentDims(&m_TotalHits, nullptr);
    m_CurrentHit = 0;

    if (ReadBatchHits() == false) {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": HDF5 read batch error"<<endl;
      return false;
    }

  } catch (const H5::Exception& E) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": HDF5 initialization error: "<<E.getDetailMsg()<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Read a batch of hits using a hyperslab
bool MModuleLoaderMeasurementsHDF::ReadBatchHits()
{
  try { // HDF5 throws exceptions, thus need to encapsulate everything in try..catch

    m_CurrentBatchSize = m_DefaultBatchSize;
    if (m_TotalHits - m_CurrentHit < m_CurrentBatchSize) {
      m_CurrentBatchSize = m_TotalHits - m_CurrentHit;
    }

    if (m_CurrentBatchSize == 0) {
      m_Buffer_1_0.resize(0);
      m_Buffer_1_2.resize(0);
      m_Buffer_2.resize(0);
      m_CurrentBatchIndex = 0;
      return false;
    }

    hsize_t Offset[1] = { m_CurrentHit };
    hsize_t Count[1] = { m_CurrentBatchSize };

    if (m_HDFStripHitVersion <= MHDFStripHitVersion::V1_2) {
      DataSpace DS = m_HDFDataSet.getSpace();
      DS.selectHyperslab(H5S_SELECT_SET, Count, Offset);
      DataSpace MS(1, Count);

      if (m_HDFStripHitVersion == MHDFStripHitVersion::V1_0) {
        if (m_Buffer_1_0.size() != m_CurrentBatchSize) {
          m_Buffer_1_0.resize(m_CurrentBatchSize);
        }
        m_HDFDataSet.read(m_Buffer_1_0.data(), m_HDFCompoundDataType, MS, DS);
      } else if (m_HDFStripHitVersion == MHDFStripHitVersion::V1_2) {
        if (m_Buffer_1_2.size() != m_CurrentBatchSize) {
          m_Buffer_1_2.resize(m_CurrentBatchSize);
        }
        m_HDFDataSet.read(m_Buffer_1_2.data(), m_HDFCompoundDataType, MS, DS);
      }
    } else if (m_HDFStripHitVersion <= MHDFStripHitVersion::V2_2) {

      // Read /Events
      DataSpace ES = m_EventDataSet.getSpace();
      ES.selectHyperslab(H5S_SELECT_SET, Count, Offset);
      DataSpace MES(1, Count);

      if (m_HDFStripHitVersion == MHDFStripHitVersion::V2_0) {
        if (m_EventData_2_0.size() != Count[0]) {
          m_EventData_2_0.resize(Count[0]);
        }
        m_EventDataSet.read(m_EventData_2_0.data(), m_EventCompoundDataType, MES, ES);
      } else if (m_HDFStripHitVersion == MHDFStripHitVersion::V2_2) {
        if (m_EventData_2_2.size() != Count[0]) {
          m_EventData_2_2.resize(Count[0]);
        }
        m_EventDataSet.read(m_EventData_2_2.data(), m_EventCompoundDataType, MES, ES);
      }

      // Read /EventIndices
      DataSpace EIS = m_EventIndicesDataSet.getSpace();
      EIS.selectHyperslab(H5S_SELECT_SET, Count, Offset);
      DataSpace MEIS(1, Count);
      if (m_EventIndices_2.size() != Count[0]) {
        m_EventIndices_2.resize(Count[0]);
      }
      m_EventIndicesDataSet.read(m_EventIndices_2.data(), m_EventIndicesCompoundDataType, MEIS, EIS);

      // Read /FEEHits (only the part that is accessed by the current Events batch)
      uint32_t MinHitIndex = m_EventIndices_2.front().m_FEEHits[0];
      uint32_t MaxHitIndex = m_EventIndices_2.back().m_FEEHits[1];
      hsize_t HitOffset[1] = { MinHitIndex };
      hsize_t HitCount[1] = { MaxHitIndex - MinHitIndex };

      DataSpace DS = m_HDFDataSet.getSpace();
      DS.selectHyperslab(H5S_SELECT_SET, HitCount, HitOffset);
      DataSpace MS(1, HitCount);
      if (m_Buffer_2.size() != HitCount[0]) {
        m_Buffer_2.resize(HitCount[0]);
      }
      m_HDFDataSet.read(m_Buffer_2.data(), m_HDFFEECompoundDataType, MS, DS);
      m_MinHitIndex = MinHitIndex;

    } else {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HDFStripHitVersion<<endl<<"Please update this module."<<endl;
      m_CurrentBatchSize = 0;
      m_Buffer_1_0.resize(0);
      m_Buffer_1_2.resize(0);
      m_Buffer_2.resize(0);
      m_CurrentBatchIndex = 0;
      return false;
    }

    m_CurrentBatchIndex = 0;

  } catch (const H5::Exception& E) {
    cout<<m_XmlTag<<": HDF5 read error: "<<E.getDetailMsg()<<endl;
    m_CurrentBatchSize = 0;
    m_Buffer_1_0.resize(0);
    m_Buffer_1_2.resize(0);
    m_Buffer_2.resize(0);
    m_CurrentBatchIndex = 0;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsHDF::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.

  bool IsZeroDataBug = false;

  unsigned int NStripHits = 1;
  unsigned int StripHitIndex = 0;
  while (StripHitIndex < NStripHits) {

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

    // Second step is to check if we have events left in the batch
    if (m_CurrentBatchIndex >= m_CurrentBatchSize) {
      if (ReadBatchHits() == false) {
        return false;
      }
    }

    // Extract the data we need
    uint16_t EventID;
    double TimeCode; // Sometimes TimeCode is int, but here we'll define double to not lose precision for HDF v1.2
    uint8_t NumberOfHits;
    MTime TimeUTC;

    // Setting SPWTimeCode default to 0, as it is defined only iin HDF version >= 2.2
    uint64_t SPWTimeCode = 0;


    if (m_HDFStripHitVersion <= MHDFStripHitVersion::V1_2) {

      uint16_t StripID;
      uint16_t ADCs;
      uint16_t TACs;
      uint8_t HitType;
      uint8_t TimingType;

      if (m_HDFStripHitVersion == MHDFStripHitVersion::V1_0) {
        MHDFStripHit_V1_0& Hit = m_Buffer_1_0[m_CurrentBatchIndex];
        ++m_CurrentBatchIndex;
        ++m_CurrentHit;

        EventID = Hit.m_EventID;
        TimeCode = Hit.m_TimeCode;
        StripID = Hit.m_StripID;
        ADCs = Hit.m_EnergyData;
        TACs = Hit.m_TimingData;
        NumberOfHits = Hit.m_Hits;
        HitType = Hit.m_HitType;
        TimingType = Hit.m_TimingType;

      } else if (m_HDFStripHitVersion == MHDFStripHitVersion::V1_2) {
        MHDFStripHit_V1_2& Hit = m_Buffer_1_2[m_CurrentBatchIndex];
        ++m_CurrentBatchIndex;
        ++m_CurrentHit;

        EventID = Hit.m_EventID;
        TimeCode = Hit.m_GSETimeCode;
        StripID = Hit.m_StripID;
        ADCs = Hit.m_EnergyData;
        TACs = Hit.m_TimingData;
        NumberOfHits = Hit.m_Hits;
        HitType = Hit.m_HitType;
        TimingType = Hit.m_TimingType;
      } else {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HDFStripHitVersion<<endl<<"Please update this module."<<endl;
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
        cout<<"  HitType: "<<(int) HitType<<endl;
        cout<<"  TimingType: "<<(int) TimingType<<endl;
      }

      // Catch a bug in the HDF5 data (v1)
      if (EventID == 0 && StripID == 0 && ADCs == 0) {
        IsZeroDataBug = true;
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": ZERO-DATA-BUG: Found empty data set. Ignoring event."<<endl;
        continue;
      } else {
        if (IsZeroDataBug == true) { // We are now out of the bug and need to recover
          // Clear the strip hits - everything else gets overwritten later
          while (Event->GetNStripHits() > 0) {
            MStripHit* H = Event->GetStripHit(0);
            delete H;
            Event->RemoveStripHit(0);
          }
        }
        IsZeroDataBug = false;
      }

      if (m_StripMap.HasReadOutID(StripID) == true) {
        MStripHit* H = new MStripHit();
        H->SetDetectorID(m_StripMap.GetDetectorID(StripID));
        H->SetStripID(m_StripMap.GetStripNumber(StripID));
        H->IsLowVoltageStrip(m_StripMap.IsLowVoltage(StripID));
        H->SetADCUnits(ADCs);
        H->SetTAC(TACs);

        // Set boolean flags based on HitType and TimingType
        H->IsGuardRing(HitType == 2);
        if (H->IsGuardRing() == true) {
          Event->SetGuardRingVeto(true);
        }
        
        H->IsNearestNeighbor(HitType == 1);
        H->HasFastTiming(TimingType == 1);
        
        // If the user does not want to include Nearest Neighbors in the data, then this is where we remove them
        // NOTE: at some point we will want to remove this code and always include nearest neighbor data
        if (m_IncludeNearestNeighbor == false && HitType == 1) {
          delete H; // Clean up the memory we just allocated
        } else {
          Event->AddStripHit(H);
        }
      } else {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Read-out ID "<<StripID<<" not found in strip map"<<endl;
        return false;
      }

      // Remove incomplete events (fewer strip hits than what is listed in HITS)
      if (StripHitIndex > 0 && NumberOfHits != NStripHits) {
        if (g_Verbosity >= c_Error) {
          cout<<m_XmlTag<<": Event "<<Event->GetID()<<" had fewer strip hits ("<<StripHitIndex<<") than expected ("<<NStripHits<<"). Ignoring event."<<endl;
        }
        // Reduce the batch index and current hit counter to still process the hit from the next event
        m_CurrentBatchIndex--;
        m_CurrentHit--;
        return false;
      }

      // Increase counters
      NStripHits = static_cast<unsigned int>(NumberOfHits);
      StripHitIndex++;
      
    } else if (m_HDFStripHitVersion <= MHDFStripHitVersion::V2_2) {

      if (m_HDFStripHitVersion == MHDFStripHitVersion::V2_0) {
        MHDFEvent_V2_0& HitEvent = m_EventData_2_0[m_CurrentBatchIndex];
        EventID = HitEvent.m_EventID;
        TimeCode = HitEvent.m_GSETimeCode;
        NumberOfHits = HitEvent.m_Hits;
      } else if (m_HDFStripHitVersion == MHDFStripHitVersion::V2_2) {
        MHDFEvent_V2_2& HitEvent = m_EventData_2_2[m_CurrentBatchIndex];
        EventID = HitEvent.m_EventID;
        TimeCode = HitEvent.m_GSETimeCode;
        SPWTimeCode = HitEvent.m_SPWTimeCode;
        NumberOfHits = HitEvent.m_Hits;
      } else {
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HDFStripHitVersion<<endl<<"Please update this module."<<endl;
        return false;
      }

      MHDFEventIndices_V2& EventIndices = m_EventIndices_2[m_CurrentBatchIndex];
      ++m_CurrentBatchIndex;
      ++m_CurrentHit;

      if (m_Buffer_2.empty()) {
        if (g_Verbosity >= c_Error) cout << "Buffer is empty or null!" << endl;
        return false;
      }

      // Create objects for all hits that belong to that event
      for (uint32_t i = EventIndices.m_FEEHits[0]; i < EventIndices.m_FEEHits[1]; i++) {

        if (i < m_MinHitIndex || i >= (m_MinHitIndex + m_Buffer_2.size())) {
          if (g_Verbosity >= c_Error) cout << m_XmlTag << ": Entry " << i << " is NOT in the current FEEHits buffer!" << endl;
          return false;
        } 

        uint32_t IndexInBatch = i - m_MinHitIndex;
        
        MHDFStripHit_V2& Hit = m_Buffer_2[IndexInBatch];
        if (m_StripMap.HasReadOutID(Hit.m_StripID) == true) {
          MStripHit* H = new MStripHit();
          H->SetDetectorID(m_StripMap.GetDetectorID(Hit.m_StripID));
          H->SetStripID(m_StripMap.GetStripNumber(Hit.m_StripID));
          H->IsLowVoltageStrip(m_StripMap.IsLowVoltage(Hit.m_StripID));
          H->SetADCUnits(Hit.m_EnergyData);
          H->SetTAC(Hit.m_TimingData);

          // Set boolean flags based on HitType and TimingType
          H->IsGuardRing(Hit.m_HitType == 2);
          if (H->IsGuardRing() == true) {
            Event->SetGuardRingVeto(true);
          }
          
          H->IsNearestNeighbor(Hit.m_HitType == 1);
          H->HasFastTiming(Hit.m_TimingType == 1);
          
          // If the user does not want to include Nearest Neighbors in the data, then this is where we remove them
          // NOTE: at some point we will want to remove this code and always include nearest neighbor data
          if (m_IncludeNearestNeighbor == false && Hit.m_HitType == 1) {
            delete H; // Clean up the memory we just allocated
          } else {
            Event->AddStripHit(H);
          }
        } else {
          if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Read-out ID "<<Hit.m_StripID<<" not found in strip map"<<endl;
          return false;
        }

        // Use StripIndex here (without updating NStripHits) to exit the while-loop after finalizing the Event
        StripHitIndex++;
      }
    } else {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unhandled HDF hit version found: "<<m_HDFStripHitVersion<<endl<<"Please update this module."<<endl;
      return false;
    }

    if (EventID < m_LastEventID) {
      m_NumberOfEventIDRollOvers++;
    }
    m_LastEventID = EventID;

    unsigned long LongEventID = EventID + m_NumberOfEventIDRollOvers*(numeric_limits<uint16_t>::max() + 1);

    Event->SetID(LongEventID);

    // Define event time based on the timecode within the HDF versions
    if (m_HDFStripHitVersion <= MHDFStripHitVersion::V2_0) {
      TimeUTC.Set(TimeCode); // Timecode in early versions is GSE computer time in s since Epoch
      Event->SetTimeUTC(TimeUTC);
    } else if (m_HDFStripHitVersion >= MHDFStripHitVersion::V2_2) {
      MTime SPWTimeforEvent(m_StartObservationTime.GetAsSystemSeconds(),SPWTimeCode); // Spacewire Timecode is ns since start of aquisition
      Event->SetTimeUTC(SPWTimeforEvent);
    } else {
      TimeUTC.Set(TimeCode);
      Event->SetTimeUTC(TimeUTC);
    }
  }

  // Remove all Events with no (valid) strip hits
  if (Event->GetNStripHits() == 0){
    if (g_Verbosity >= c_Error) {
      cout<<m_XmlTag<<": Event had no (valid) strip hits"<< endl;
    }
    return false;
  }

  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement);

  m_NEventsInFile++;
  m_NGoodEventsInFile++;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsHDF::Finalize()
{
  // Initialize the module 
  
  MModule::Finalize();
  
  cout<<"MModuleLoaderMeasurementsHDF: "<<endl;
  cout<<"  * all events on file:  "<<m_NEventsInFile<<endl;
  cout<<"  * good events on file: "<<m_NGoodEventsInFile<<endl;

  m_HDFFile.close();
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
  
  MXmlNode* IncludeNearestNeighborNode = Node->GetNode("IncludeNearestNeighbor");
  if (IncludeNearestNeighborNode != nullptr) {
    m_IncludeNearestNeighbor = IncludeNearestNeighborNode->GetValueAsBoolean();
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
  new MXmlNode(Node, "IncludeNearestNeighbor", m_IncludeNearestNeighbor);

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
