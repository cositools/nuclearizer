/*
 * MModuleSaverMeasurementsL0.h
 *
 * Copyright (C) by Andreas Zoglauer, WingYeung Ma.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleSaverMeasurementsL0__
#define __MModuleSaverMeasurementsL0__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <fstream>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Nuclearizer libs:
#include "MModule.h"
#include "MReadOutAssembly.h"


////////////////////////////////////////////////////////////////////////////////


//! A module to save events to L0 binary format (DD packets)
class MModuleSaverMeasurementsL0 : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleSaverMeasurementsL0();
  //! Default destructor
  virtual ~MModuleSaverMeasurementsL0();

  //! Create a new object of this class
  virtual MModuleSaverMeasurementsL0* Clone() { return new MModuleSaverMeasurementsL0(); }

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
  //! Write CCSDS Primary Header
  void WriteCCSDSPrimaryHeader(uint16_t apid, uint16_t seqCount, uint16_t dataLength);
  //! Write Secondary Header (packet time)
  void WriteSecondaryHeader(uint32_t seconds, uint32_t subseconds);
  //! Pack bits into a continuous bit stream
  void PackBitsIntoBitstream(std::vector<uint8_t>& bitstream, int& bitOffset, uint64_t bits, int numBits);
  //! Encode a normal strip hit (Type 0x0) - returns 44 bits
  uint64_t EncodeNormalHit(int stripID, bool fastTiming, int energy, int timing);
  //! Encode a neighboring strip hit (Type 0x1) - returns 36 bits
  uint64_t EncodeNeighborHit(int stripID, bool fastTiming, int energy, int timing);
  //! Encode a guard ring hit (Type 0x2) - returns 24 bits
  uint32_t EncodeGuardRingHit(int stripID, int energy);
  //! Write 16-bit value in big-endian
  void WriteUInt16BE(uint16_t value);
  //! Write 32-bit value in big-endian
  void WriteUInt32BE(uint32_t value);


  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! Output file name
  MString m_FileName;

  //! Output file stream
  std::ofstream m_OutFile;

  //! Packet sequence counter
  uint16_t m_SequenceCount;

  //! Total number of events written
  long m_TotalEventsWritten;

  //! ApID for DD packets (Compton events)
  static const uint16_t APID_DD = 0x0DD;


#ifdef ___CLING___
 public:
  ClassDef(MModuleSaverMeasurementsL0, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
