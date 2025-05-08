/*
 * MModuleTACcut.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleTACcut__
#define __MModuleTACcut__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <algorithm>

// ROOT libs:
#include "TGClient.h"
#include "TH1.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MGUIExpoTACcut.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleTACcut : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleTACcut();
  //! Default destructor
  virtual ~MModuleTACcut();
  
  //! Create a new object of this class 
  virtual MModuleTACcut* Clone() { return new MModuleTACcut(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Create expos 
  virtual void CreateExpos();

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

  ///////////// Creating functions that will update and get the min/max TAC values //////////////////////////

  //! Set filename for TAC Calibration
  void SetTACCalFileName( const MString& FileName) {m_TACCalFile = FileName;}
  //! Get filename for TAC Calibration
  MString GetTACCalFileName() const {return m_TACCalFile;}

  //! Set filename for TAC Cut
  void SetTACCutFileName( const MString& FileName) {m_TACCutFile = FileName;}
  //! Get filename for TAC Cut
  MString GetTACCutFileName() const {return m_TACCutFile;}

  //! Set the disable time
  void SetDisableTime(double DisableTime) { m_DisableTime = DisableTime; }
  //! Get the disable time
  unsigned int GetDisableTime() const { return m_DisableTime; }

  //! Set the shaping flag_to_en_delay
  void SetFlagToEnDelay(double FlagToEnDelay) { m_FlagToEnDelay = FlagToEnDelay; }
  //! Get the shaping flag_to_en_delay
  unsigned int GetFlagToEnDelay() const { return m_FlagToEnDelay; }

  //! Load the TAC calibration file
  bool LoadTACCalFile(MString FName);

  //! Load the TAC cut file
  bool LoadTACCutFile(MString FName);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 


  // protected methods:
 protected:

  
  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:

// declare TAC Cut and calibration variables here
double m_DisableTime, m_FlagToEnDelay;
MString m_TACCalFile;
MString m_TACCutFile;
unordered_map<int, unordered_map<int, vector<double>>> m_HVTACCal;
unordered_map<int, unordered_map<int, vector<double>>> m_LVTACCal;
unordered_map<int, unordered_map<int, vector<double>>> m_HVTACCut;
unordered_map<int, unordered_map<int, vector<double>>> m_LVTACCut;

vector<unsigned int> m_DetectorIDs;

MGUIExpoTACcut* m_ExpoTACcut;


#ifdef ___CLING___
 public:
  ClassDef(MModuleTACcut, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
