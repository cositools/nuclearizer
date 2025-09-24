/*
 * MModuleEventSaver.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleEventSaver__
#define __MModuleEventSaver__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MFile.h"

// Nuclearizer libs:
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleEventSaver : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleEventSaver();
  //! Default destructor
  virtual ~MModuleEventSaver();
  
  //! Create a new object of this class 
  virtual MModuleEventSaver* Clone() { return new MModuleEventSaver(); }

  //! Get the mode
  unsigned int GetMode() const { return m_Mode; }
  //! Set the mode
  void SetMode(unsigned int Mode) { m_Mode = Mode; }
  
  //! Get the file name
  MString GetFileName() const { return m_FileName; }
  //! Set the file name
  void SetFileName(const MString& Name) { m_FileName = Name; }
  
  //! Return true if the Bad events should be saved
  bool GetSaveBadEvents() const { return m_SaveBadEvents; }
  //! Set whether the Bad events should be saved
  void SetSaveBadEvents(bool SaveBadEvents) { m_SaveBadEvents = SaveBadEvents; }

  //! Return true if the Veto events should be saved
  bool GetSaveVetoEvents() const { return m_SaveVetoEvents; }
  //! Set whether the Veto events should be saved
  void SetSaveVetoEvents(bool SaveVetoEvents) {m_SaveVetoEvents = SaveVetoEvents; }
  
  //! Get the add time tag flag
  bool GetAddTimeTag() const { return m_AddTimeTag; }
  //! Set the add time tag flag
  void SetAddTimeTag(bool AddTimeTag) { m_AddTimeTag = AddTimeTag; }
  
  //! Retun true if the file should be split
  bool GetSplitFile() const { return m_SplitFile; }
  //! Set whether the file should be split
  void SetSplitFile(bool SplitFile) { m_SplitFile = SplitFile; }
  
  //! Return the time after which the file should be split
  MTime GetSplitFileTime() const { return m_SplitFileTime; }
  //! Set the time after which the file should be split
  void SetSplitFileTime(MTime SplitFileTime) { m_SplitFileTime = SplitFileTime; }

  //! Return whether ADCs should be included in the roa file
  bool GetRoaWithADCs() const { return m_RoaWithADCs; }
  //! Set whether ADCs should be included in the roa file
  void SetRoaWithADCs(bool Flag) { m_RoaWithADCs = Flag; }

  //! Return whether TACs should be included in the roa file
  bool GetRoaWithTACs() const { return m_RoaWithTACs; }
  //! Set whether TACs should be included in the roa file
  void SetRoaWithTACs(bool Flag) { m_RoaWithTACs = Flag; }

  //! Return whether energies should be included in the roa file
  bool GetRoaWithEnergies() const { return m_RoaWithEnergies; }
  //! Set whether energies should be included in the roa file
  void SetRoaWithEnergies(bool Flag) { m_RoaWithEnergies = Flag; }

  //! Return whether timings should be included in the roa file
  bool GetRoaWithTimings() const { return m_RoaWithTimings; }
  //! Set whether timings should be included in the roa file
  void SetRoaWithTimings(bool Flag) { m_RoaWithTimings = Flag; }

  //! Return whether temperatures should be included in the roa file
  bool GetRoaWithTemperatures() const { return m_RoaWithTemperatures; }
  //! Set whether temperatures should be included in the roa file
  void SetRoaWithTemperatures(bool Flag) { m_RoaWithTemperatures = Flag; }

  //! Return whether flags should be included in the roa file
  bool GetRoaWithFlags() const { return m_RoaWithFlags; }
  //! Set whether flags should be included in the roa file
  void SetRoaWithFlags(bool Flag) { m_RoaWithFlags = Flag; }

  //! Return whether origins should be included in the roa file
  bool GetRoaWithOrigins() const { return m_RoaWithOrigins; }
  //! Set whether origins should be included in the roa file
  void SetRoaWithOrigins(bool Flag) { m_RoaWithOrigins = Flag; }

  //! Return whether nearest neighbors should be included in the roa file
  bool GetRoaWithNearestNeighbors() const { return m_RoaWithNearestNeighbors; }
  //! Set whether nearest neighbors should be included in the roa file
  void SetRoaWithNearestNeighbors(bool Flag) { m_RoaWithNearestNeighbors = Flag; }

  //! Set the start area of the far field simulation if there was any
  void SetStartAreaFarField(double Area) { m_StartAreaFarField = Area; } 
  //! Set the number if simulated events
  void SetSimulatedEvents(long NumberOfSimulatedEvents) { m_NumberOfSimulatedEvents = NumberOfSimulatedEvents; } 
  
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

  static const unsigned int c_RoaFile  = 0;
  static const unsigned int c_DatFile  = 1;
  static const unsigned int c_EvtaFile = 2;
  static const unsigned int c_SimFile  = 3;
  static const unsigned int c_TraFile  = 4;
  
  // protected methods:
 protected:
  //! Start a new sub-file
  bool StartSubFile();
   
  //!
  void WriteHeader();

  //!
  void DumpBasic(MReadOutAssembly* Event);

  //!
  void DumpHitsSim(MHit* HitSim);

  //!
  void DumpStripHits(MStripHit* StrpHit);

  //!
  void DumpHits(MHit* Hit);
  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The operation mode
  unsigned int m_Mode;
  //! The file name
  MString m_FileName;
  //! The internal filename with tags etc.
  MString m_InternalFileName;
  
  //! True if the file should be gzip'ed
  bool m_Zip;
  
  //! Save bad events
  bool m_SaveBadEvents;

  //! Save Veto events
  bool m_SaveVetoEvents;

  //! Add a time tag to the file
  bool m_AddTimeTag;
  
  //! The file header 
  MString m_Header;
  
  //! Split the file into multiple ones
  bool m_SplitFile;
  //! If we split the file, this is the time in seconds after which we split
  MTime m_SplitFileTime;

  // Roa options

  //! If true write ADC values to the roa file
  bool m_RoaWithADCs;
  //! If true write TAC values to the roa file
  bool m_RoaWithTACs;
  //! If true write energy values to the roa file
  bool m_RoaWithEnergies;
  //! If true write timing values to the roa file
  bool m_RoaWithTimings;
  //! If true write temperature values to the roa file
  bool m_RoaWithTemperatures;
  //! If true write flags to the roa file
  bool m_RoaWithFlags;
  //! If true write origins to the roa file
  bool m_RoaWithOrigins;
  //! True if we should include next neighbors in the data stream
  bool m_RoaWithNearestNeighbors;
  
  //! Main output stream for file
  MFile m_Out;
  //! Sub-output stream if we split it into multiple files
  MFile m_SubFileOut;
  //! Start time in case we split the file in mutliples
  MTime m_SubFileStart;

  //! The start area of far field simulations
  double m_StartAreaFarField;
  //! The numebr of simulated events
  long m_NumberOfSimulatedEvents;
  
#ifdef ___CLING___
 public:
  ClassDef(MModuleEventSaver, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
