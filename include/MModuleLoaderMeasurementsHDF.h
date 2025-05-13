/*
 * MModuleLoaderMeasurementsHDF.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleLoaderMeasurementsHDF__
#define __MModuleLoaderMeasurementsHDF__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileReadOuts.h"

// Nuclearizer libs:
#include "MStripMap.h"
#include "MModuleLoaderMeasurements.h"

// H5 libs
#include "H5Cpp.h"
using namespace H5;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! The versions of the strip hits stored in the HDF file
enum class MHDFStripHitVersion { V1_0, V1_2 };

//! And a streamer for it
inline ostream& operator<<(ostream& os, MHDFStripHitVersion Version) {
  switch (Version) {
    case MHDFStripHitVersion::V1_0: return os<<"1.0";
    case MHDFStripHitVersion::V1_2: return os<<"1.2";
    default: return os<<"Unknown";
  }
}

//! Version 1.0 & 1.1 of the HDF5 hit info
struct MHDFStripHit_V1_0 {
  uint16_t m_EventID;
  uint32_t m_TimeCode;
  uint8_t  m_HitType;
  uint8_t  m_TimingType;
  uint16_t m_StripID;
  uint8_t  m_CrystalID;
  uint8_t  m_Gain;
  uint8_t  m_Overflow;
  uint16_t m_CurrentMaximum;
  uint16_t m_HighCurrentSamples;
  uint16_t m_EnergyData;
  uint16_t m_EnergyDataLowGain;
  uint16_t m_EnergyDataHighGain;
  uint16_t m_TimingData;
  uint8_t  m_Pad;
  uint8_t  m_Hits;
  uint8_t  m_EventType;
  uint8_t  m_CRC;
};

//! Version 1.2 of the HDF5 hit info
struct MHDFStripHit_V1_2 {
  uint16_t m_EventID;
  uint64_t m_TimeCode;
  double   m_GSETimeCode;
  uint8_t  m_HitType;
  uint8_t  m_TimingType;
  uint16_t m_StripID;
  uint8_t  m_CrystalID;
  uint8_t  m_Gain;
  uint8_t  m_Overflow;
  uint16_t m_CurrentMaximum;
  uint16_t m_HighCurrentSamples;
  uint16_t m_EnergyData;
  uint16_t m_EnergyDataLowGain;
  uint16_t m_EnergyDataHighGain;
  uint16_t m_TimingData;
  uint8_t  m_Pad;
  uint8_t  m_Hits;
  uint16_t m_Bytes;
  uint8_t  m_EventType;
  uint8_t  m_CRC;
};

//! The version string
struct MHDFStripHitVersionString {
  char string_col[256];
};

////////////////////////////////////////////////////////////////////////////////


//! A module to load HDF5 data files
class MModuleLoaderMeasurementsHDF : public MModuleLoaderMeasurements
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderMeasurementsHDF();
  //! Default destructor
  virtual ~MModuleLoaderMeasurementsHDF();
  
  //! Create a new object of this class 
  virtual MModuleLoaderMeasurementsHDF* Clone() { return new MModuleLoaderMeasurementsHDF(); }

  //! Get the file name of the strip map
  MString GetFileNameStripMap() const { return m_FileNameStripMap; }
  //! Set the file name of the strip map
  void SetFileNameStripMap(const MString& Name) { m_FileNameStripMap = Name; }


  //! Enable/Disable loading continuation files
  bool GetLoadContinuationFiles() const { return m_LoadContinuationFiles; }
  //! Set loading continuation files
  void SetLoadContinuationFiles(bool LoadContinuationFiles) { m_LoadContinuationFiles = LoadContinuationFiles; }

  //! Initialize the module
  virtual bool Initialize();

  //! Initialize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();


  // protected methods:
 protected:
  //! Convert more data from raw to intermediate format - return false if no more data can be converted
  bool OpenHDF5File(MString FileName);
  //! Read a batch of hits using a hyperslab
  bool ReadBatchHits();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
   /*
  //! Start of the observation time
  MTime m_StartObservationTime;
  //! Clock time belonging to the start of the observation time
  unsigned long m_StartClock; 
  //! End of the observation time
  MTime m_EndObservationTime;
  //! Clock time belonging to the end of the observation time
  unsigned long m_EndClock;
  */

  //! The HDF5 file
  H5File m_HDFFile;

  //! True, if we want to load continuation files
  bool m_LoadContinuationFiles;

  //! The ID of the currently loaded continuation file
  unsigned int m_ContinuationFileID;
  
  //! The HDF5 data set
  DataSet m_HDFDataSet;

  //! The HDF5 compond data type
  CompType m_HDFCompoundDataType;

  //! The version of the strip hit structure
  MHDFStripHitVersion m_HDFStripHitVersion;

  //! The default batch size
  static constexpr unsigned int m_DefaultBatchSize = 10000;

  //! The current batch size
  unsigned int m_CurrentBatchSize;

  //! The current index in the batch
  unsigned int m_CurrentBatchIndex;

  // The various batches:
  //! The MHDFStripHit_V1_0 batch:
  vector<MHDFStripHit_V1_0> m_Buffer_1_0;
  //! The MHDFStripHit_V1_2 batch:
  vector<MHDFStripHit_V1_2> m_Buffer_1_2;

  //! Total number of hits in a file
  hsize_t m_TotalHits;

  //! Current hit number in the file
  hsize_t m_CurrentHit;

  //! Number of event ID roll-overs:
  unsigned int m_NumberOfEventIDRollOvers;

  //! The last handles event ID
  unsigned int m_LastEventID;

  //! The file name of the strip map
  MString m_FileNameStripMap;

  //! The strip map
  MStripMap m_StripMap;

#ifdef ___CLING___
 public:
  ClassDef(MModuleLoaderMeasurementsHDF, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
