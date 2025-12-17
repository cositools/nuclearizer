/*
 * MModuleSaverMeasurementsFITS.h
 *
 * Copyright (C) by Andreas Zoglauer.
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

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Nuclearizer libs:
#include "MModule.h"

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
  //! Default destructor
  virtual ~MModuleSaverMeasurementsFITS();

  //! Create a new object of this class
  virtual MModuleSaverMeasurementsFITS* Clone() { return new MModuleSaverMeasurementsFITS(); }

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


  // protected methods:
 protected:
  //! Create the FITS file and extensions
  bool CreateFITSFile(MString FileName);
  //! Flush current batch to FITS file
  bool FlushBatch();


  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! Output file name
  MString m_FileName;

  //! The FITS file object pointer
  FITS* m_FITSFile;

  //! Primary HDU (HDU 0) - header/metadata
  PHDU* m_PrimaryHDU;

  //! Science data table extension
  ExtHDU* m_ScienceTable;

  //! Total number of events written
  long m_TotalEventsWritten;

  //! Batch size for writing FITS data
  static const long m_BatchSize = 100;

  //! Current row where next batch will be written
  long m_BatchStartRow;

  //! Number of events in current batch
  long m_BatchEventCount;

  //! Batch data storage for scalar columns
  std::vector<double> m_BatchTIME;
  std::vector<uint8_t> m_BatchEVENTTYPE;
  std::vector<uint8_t> m_BatchEVENTCLASS;
  std::vector<uint8_t> m_BatchNUMHIT;
  std::vector<uint8_t> m_BatchSEQHIT;

  //! Batch data storage for fixed-length array columns (event-level)
  std::vector<std::valarray<float>> m_BatchSTATTEST;      
  std::vector<std::valarray<float>> m_BatchRECOILDIR;     
  std::vector<std::valarray<float>> m_BatchRECOILDIR_ERR;

  //! Batch data storage for variable-length array columns (hit-level data)
  std::vector<std::valarray<float>> m_BatchX;
  std::vector<std::valarray<float>> m_BatchY;
  std::vector<std::valarray<float>> m_BatchZ;
  std::vector<std::valarray<float>> m_BatchX_ERR;
  std::vector<std::valarray<float>> m_BatchY_ERR;
  std::vector<std::valarray<float>> m_BatchZ_ERR;
  std::vector<std::valarray<float>> m_BatchENERGY;
  std::vector<std::valarray<float>> m_BatchENERGY_ERR;
  std::vector<std::valarray<float>> m_BatchBAD_FLAG;


#ifdef ___CLING___
 public:
  ClassDef(MModuleSaverMeasurementsFITS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
