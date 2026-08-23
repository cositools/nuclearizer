/*
 * MModuleSaverMeasurementsFITS.h
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleSaverMeasurementsFITS__
#define __MModuleSaverMeasurementsFITS__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <valarray>
#include <string>
#include <cstdint>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Nuclearizer libs:
#include "MModule.h"
#include "MFITSWriterL1a.h"

// CCfits libs
#include <CCfits/CCfits>
using namespace CCfits;


////////////////////////////////////////////////////////////////////////////////


//! A module to save hit-level events to FITS data files
class MModuleSaverMeasurementsFITS : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleSaverMeasurementsFITS();
  //! Constructor with explicit XML tag and default output level
  MModuleSaverMeasurementsFITS(MString XmlTag, int OutputDataLevel, MString Name);
  //! Default destructor
  virtual ~MModuleSaverMeasurementsFITS();

  //! Create a new object of this class
  virtual MModuleSaverMeasurementsFITS* Clone() { return new MModuleSaverMeasurementsFITS(m_XmlTag, m_OutputDataLevel, m_Name); }

  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  //! Set the output file name
  void SetFileName(const MString& FileName) { m_FileName = FileName; }
  //! Get the output file name
  MString GetFileName() const { return m_FileName; }

  //! Set the output data level: 0 = L1a, 1 = L1b, 2 = L2
  void SetOutputDataLevel(int Level) { m_OutputDataLevel = Level; ConfigurePreceedingModules(); }
  //! Get the output data level: 1 = L1b, 2 = L2
  int GetOutputDataLevel() const { return m_OutputDataLevel; }


  // protected methods:
 protected:
  //! Create the FITS file and extensions
  bool CreateFITSFile(MString FileName);
  //! Flush current batch to FITS file
  bool FlushBatch();
  //! Configure predecessor requirements for the selected output level
  void ConfigurePreceedingModules();


  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! Output file name
  MString m_FileName;

  //! Output data level: 0 = L1a (raw hits), 1 = L1b (all events, with QUALITY_FLAG), 2 = L2 (screened, no QUALITY_FLAG)
  int m_OutputDataLevel;

  //! L1a writer, used only when m_OutputDataLevel == 0
  MFITSWriterL1a m_L1aWriter;

  //! The FITS file object pointer
  FITS* m_FITSFile;

  //! Primary HDU (HDU 0) - header/metadata
  PHDU* m_PrimaryHDU;

  //! Science data table extension
  ExtHDU* m_ScienceTable;

  //! Total number of events written
  long m_TotalEventsWritten;

  //! Total number of events skipped (L2 screening)
  long m_TotalEventsSkipped;

  //! First event time seen, RTS (mission seconds since 2025-01-01)
  double m_FirstEventTime_RTS;

  //! Last event time seen, RTS (mission seconds since 2025-01-01)
  double m_LastEventTime_RTS;

  //! Whether any events have been processed yet
  bool m_HasEvents;

  //! Batch size for writing FITS data
  static const long m_BatchSize = 100;

  //! Current row where next batch will be written
  long m_BatchStartRow;

  //! Number of events in current batch
  long m_BatchEventCount;

  //! Batch data storage for scalar columns
  std::vector<double> m_BatchTIME;
  std::vector<uint32_t> m_BatchEVENTID;       // Document said 1J = signed 32-bit, but I think we should use unsigned because EVENTID should always be >= 1?
  std::vector<uint8_t> m_BatchEVENTTYPE;
  std::vector<uint8_t> m_BatchEVENTCLASS;
  std::vector<uint8_t> m_BatchNUMHIT;
  std::vector<uint8_t> m_BatchVETO;
  std::vector<std::string> m_BatchQUALITY_FLAG; //64A string

  //! Batch data storage for fixed-length array columns (event-level)
  std::vector<std::valarray<float>> m_BatchSTATTEST;     // 8E
  std::vector<std::valarray<float>> m_BatchRECOILDIR;    // 3E
  std::vector<std::valarray<float>> m_BatchRECOILDIR_ERR;// 3E

  //! Batch data storage for variable-length array columns (hit-level data)
  std::vector<std::valarray<uint8_t>> m_BatchSEQHIT; //L1b PB(50), L2: 10B
  std::vector<std::valarray<float>> m_BatchX;
  std::vector<std::valarray<float>> m_BatchY;
  std::vector<std::valarray<float>> m_BatchZ;
  std::vector<std::valarray<float>> m_BatchX_ERR;
  std::vector<std::valarray<float>> m_BatchY_ERR;
  std::vector<std::valarray<float>> m_BatchZ_ERR;
  std::vector<std::valarray<float>> m_BatchENERGY;
  std::vector<std::valarray<float>> m_BatchENERGY_ERR;


#ifdef ___CLING___
 public:
  ClassDef(MModuleSaverMeasurementsFITS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
