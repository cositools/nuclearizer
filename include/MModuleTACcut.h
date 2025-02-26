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

 //! Set the minimum TAC value!
  void SetMinimumTAC(unsigned int MinimumTAC) { m_MinimumTAC = MinimumTAC; }
  //! Get the minimum TAC value!
  unsigned int GetMinimumTAC() const { return m_MinimumTAC; }

  //! Set the maximum TAC value!
  void SetMaximumTAC(unsigned int MaximumTAC) { m_MaximumTAC = MaximumTAC; }
  //! Get the maximum TAC value!
  unsigned int GetMaximumTAC() const { return m_MaximumTAC; }

  //! Set filename for TAC Calibration
  void SetTACCalFileName( const MString& FileName) {m_TACCalFile = FileName;}
  //! Get filename for TAC Calibration
  MString GetTACCalFileName() const {return m_TACCalFile;}

  //! Load the TAC calibration file
  bool LoadTACCalFile(MString FName);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 


  // protected methods:
 protected:

  
  // private methods:
 private:



  // protected members:
 protected:

  // private members:
 private:

// declare min and max TAC variables here
unsigned int m_MinimumTAC, m_MaximumTAC;
MString m_TACCalFile;
unordered_map<int, unordered_map<int, vector<double>>> m_HVTACCal;
unordered_map<int, unordered_map<int, vector<double>>> m_LVTACCal;

vector<unsigned int> m_DetectorIDs;

MGUIExpoTACcut* m_ExpoTACcut;


#ifdef ___CLING___
 public:
  ClassDef(MModuleTACcut, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
