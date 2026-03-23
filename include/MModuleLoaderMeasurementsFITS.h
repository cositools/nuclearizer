/*
 * MModuleLoaderMeasurementsFITS.h
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleLoaderMeasurementsFITS__
#define __MModuleLoaderMeasurementsFITS__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <valarray>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFileReadOuts.h"

// Nuclearizer libs:
#include "MModuleLoaderMeasurements.h"

// CCfits libs
#include <CCfits/CCfits>
using namespace CCfits;


////////////////////////////////////////////////////////////////////////////////

//! A module to load FITS data files
class MModuleLoaderMeasurementsFITS : public MModuleLoaderMeasurements
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderMeasurementsFITS();
  //! Default destructor
  virtual ~MModuleLoaderMeasurementsFITS();

  //! Create a new object of this class
  virtual MModuleLoaderMeasurementsFITS* Clone() { return new MModuleLoaderMeasurementsFITS(); }

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


  // protected methods:
 protected:
  //! Open the FITS file
  bool OpenFITSFile(MString FileName);
  //! Read next event (loads new batch if needed)
  bool ReadBatch();

  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! The FITS file object pointer
  FITS* m_FITSFile;

  //! Primary HDU (HDU 0) - header/metadata
  const PHDU* m_PrimaryHDU;

  //! Compton L1a table extension (extension 1)
  ExtHDU* m_ComptonTable;

  //! Current row number in the FITS table
  long m_CurrentRow;

  //! Total number of rows in the FITS table
  long m_TotalRows;

  //! Batch size for reading FITS data
  const long m_BatchSize = 100;

  //! Current batch start row
  long m_BatchStartRow = 1;

  //! Current event index within the batch
  long m_CurrentEventInBatch = 0;

  //! Number of events in current batch
  long m_BatchEventCount = 0;

  //! Batch data storage for scalar columns
  std::vector<double> m_BatchTIME;
  std::vector<uint8_t> m_BatchEVENTTYPE;
  std::vector<uint8_t> m_BatchNUMSTRIPHIT;

  //! Batch data storage for variable-length array columns
  std::vector<std::valarray<uint8_t>> m_BatchTYPEHIT;
  std::vector<std::valarray<int>> m_BatchDETID;
  std::vector<std::valarray<int>> m_BatchSTRIPID;
  std::vector<std::valarray<int>> m_BatchSIDEID;
  std::vector<std::valarray<uint8_t>> m_BatchFASTTIME;
  std::vector<std::valarray<int>> m_BatchPHA;
  std::vector<std::valarray<int>> m_BatchTAC;


#ifdef ___CLING___
 public:
  ClassDef(MModuleLoaderMeasurementsFITS, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
