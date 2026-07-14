/*
 * MModuleLoaderMeasurementsL0.h
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleLoaderMeasurementsL0__
#define __MModuleLoaderMeasurementsL0__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>
#include <vector>
#include <cstdint>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Nuclearizer libs:
#include "MModuleLoaderMeasurements.h"
#include "MStripMap.h"


////////////////////////////////////////////////////////////////////////////////


//! A module to load L0 binary files (CCSDS DD packets) into MReadOutAssembly events.
class MModuleLoaderMeasurementsL0 : public MModuleLoaderMeasurements
{
  // public interface:
 public:
  //! Default constructor
  MModuleLoaderMeasurementsL0();
  //! Default destructor
  virtual ~MModuleLoaderMeasurementsL0();

  //! Create a new object of this class
  virtual MModuleLoaderMeasurementsL0* Clone() { return new MModuleLoaderMeasurementsL0(); }

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

  //! Set the L0 binary file name
  void SetFileName(const MString& FileName) { m_FileName = FileName; }
  //! Get the L0 binary file name
  MString GetFileName() const { return m_FileName; }

  //! Set the strip map file name
  void SetFileNameStripMap(const MString& FileName) { m_FileNameStripMap = FileName; }
  //! Get the strip map file name
  MString GetFileNameStripMap() const { return m_FileNameStripMap; }


  // protected methods:
 protected:
  //! Open the L0 file. Auto-detects whether it has a 20-byte L0 header (from MOC simulator) or is raw CCSDS packets.
  bool OpenL0File(MString FileName);

  //! Read the next CCSDS packet from the file and populate Event.
  //! Returns false on EOF or unrecoverable read error.
  bool ReadNextPacket(MReadOutAssembly* Event);

  //! Decode the bit-packed HIT_DATA payload into MStripHit objects on Event.
  //! Returns false on parsing error.
  bool DecodeHitData(const std::vector<uint8_t>& hitData, unsigned int expectedHits, MReadOutAssembly* Event);


  // private members:
 private:
  //! Input L0 binary file name (.dat or .bin)
  MString m_FileName;

  //! Strip map file name for read-out ID -> (detector, side, strip) lookup
  MString m_FileNameStripMap;

  //! Strip map (loaded at Initialize)
  MStripMap m_StripMap;

  //! Whether the strip map was successfully loaded
  bool m_StripMapLoaded;

  //! The opened input file stream
  std::ifstream m_InFile;

  //! Total number of packets read so far
  unsigned long m_PacketsRead;


#ifdef ___CLING___
 public:
  ClassDef(MModuleLoaderMeasurementsL0, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
