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

//! TAC cut and TAC calibration parameter files
MString m_TACCalFile;
MString m_TACCutFile;

//! Map DetID -> Side (LV=0, HV=1) -> Strip ID -> TAC calibration/cut parameters
unordered_map<int, vector<unordered_map<int, vector<double>>>> m_TACCal;
unordered_map<int, vector<unordered_map<int, vector<double>>>> m_TACCut;

//! Map characters representing sides of the detectors indices to avoid mistakes
unordered_map<char, int> m_SideToIndex;

vector<unsigned int> m_DetectorIDs;

MGUIExpoTACcut* m_ExpoTACcut;


#ifdef ___CLING___
 public:
  ClassDef(MModuleTACcut, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
