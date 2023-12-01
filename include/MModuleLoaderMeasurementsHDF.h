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
#include "MModuleLoaderMeasurements.h"

// H5 libs
#include "H5Cpp.h"
using namespace H5;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MReadOutHDF
{
public:
  uint16_t m_DT;
  uint16_t m_AsicID;
  uint16_t m_ChannelID;
  uint16_t m_ADCValue;
  uint16_t m_TACValue;
  uint16_t m_OscillatorValue;
  uint16_t m_Headers;
};


////////////////////////////////////////////////////////////////////////////////


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
  bool Convert();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Start of the observation time
  MTime m_StartObservationTime;
  //! Clock time belonging to the start of the observation time
  unsigned long m_StartClock; 
  //! End of the observation time
  MTime m_EndObservationTime;
  //! Clock time belonging to the end of the observation time
  unsigned long m_EndClock;

  //! The HDF5 file
  H5File m_FileHDF5;
  
  //! The raw data buffer
  vector<hvl_t> m_RawBuffer;

  //! The next read element in the raw buffer to read
  size_t m_NextRawBufferPosition;

  //! The intermediate data buffer
  list<MReadOutHDF> m_IntermediateBuffer;

  //! The intermediate buffer size - it's a list, thus we store the size for fast access
  size_t m_IntermediateBufferSize;
  
  //! The previously read oscillator value. Needed to check for overflows
  unsigned long m_LastOscillatorValue;

  //! The oscillator frequency
  unsigned long m_OscillatorFrequency;

  //! The oscillator offset
  unsigned long m_OscillatorOffset;

  //! The oscillator offset
  unsigned long m_LastEventID;

  //! The file name of the strip map
  MString m_FileNameStripMap;


#ifdef ___CLING___
 public:
  ClassDef(MModuleLoaderMeasurementsHDF, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
