/*
 * MModuleTACcal.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleTACcal__
#define __MModuleTACcal__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"


// Forward declarations:
class MGUIExpoTACcut;
class MGUIExpoPlotSpectrum;


////////////////////////////////////////////////////////////////////////////////


class MModuleTACcal : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleTACcal();
  //! Default destructor
  virtual ~MModuleTACcal();
  
  //! Create a new object of this class 
  virtual MModuleTACcal* Clone() { return new MModuleTACcal(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Finalize the module
  virtual void Finalize();

  //! Create Expos
  virtual void CreateExpos();

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);

  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  //! Set filename for TAC calibration
  void SetTACCalFileName( const MString& FileName) {m_TACCalFile = FileName;}

  //! Get filename for TAC calibration
  MString GetTACCalFileName() const {return m_TACCalFile;}

  //! Load the TAC calibration file
  bool LoadTACCalFile(MString FName);

  //! Set the TAC calibration parameters
  void SetTACCalParameters(unordered_map<int, vector<unordered_map<int, vector<double>>>> TACCal) { m_TACCal = TACCal; }

  //! Get the TAC calibration parameters
  unordered_map<int, vector<unordered_map<int, vector<double>>>> GetTACCalParameters() { return m_TACCal; }

  //! Enable or disable TAC cuts
  void SetApplyTACCuts(bool ApplyTACCuts)
  {
    m_ApplyTACCuts = ApplyTACCuts;
  }

  //! Return whether TAC cuts are enabled
  bool GetApplyTACCuts() const
  {
    return m_ApplyTACCuts;
  }

  //! Set TAC coincidence window in ns
  void SetCoincidenceWindow(double CoincidenceWindow)
  {
    m_CoincidenceWindow = CoincidenceWindow;
  }

  //! Get TAC coincidence window in ns
  double GetCoincidenceWindow() const
  {
    return m_CoincidenceWindow;
  }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 


  // protected methods:
 protected:

  
  // private methods:
 private:

  //! Apply TAC cuts to calibrated strip hits
  bool ApplyTACCuts(MReadOutAssembly* Event);


  // protected members:
 protected:

  // private members:
 private:

  //! TAC calibration parameter file name
  MString m_TACCalFile;

  //! Map DetID -> Side (LV=0, HV=1) -> Strip ID -> TAC calibration parameters
  unordered_map<int, vector<unordered_map<int, vector<double>>>> m_TACCal;

  //! Map characters representing detector sides to LV/HV indices
  unordered_map<char, int> m_SideToIndex;

  //! Vector of Detector IDs
  vector<unsigned int> m_DetectorIDs;

  //! TAC coincidence window in ns
  double m_CoincidenceWindow;

  //! Option to apply TAC cuts after TAC calibration
  bool m_ApplyTACCuts;

  //! TAC distribution and energy spectra
  MGUIExpoTACcut* m_ExpoTACcut;
  MGUIExpoPlotSpectrum* m_ExpoEnergySpectrum;


#ifdef ___CLING___
 public:
  ClassDef(MModuleTACcal, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
