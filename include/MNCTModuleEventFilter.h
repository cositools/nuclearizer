/*
 * MNCTModuleEventFilter.h
 *
 * Copyright (C) by Andreas Zoglauer
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleEventFilter__
#define __MNCTModuleEventFilter__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <fstream>

// ROOT libs:
#include "MString.h"
#include "TH3.h"
#include "TTree.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleEventFilter : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleEventFilter();
  //! Default destructor
  virtual ~MNCTModuleEventFilter();
  
  //! Create a new object of this class 
  virtual MNCTModuleEventFilter* Clone() { return new MNCTModuleEventFilter(); }

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

  //! Set the accepted detector list - if empty all are accepted!
  void SetDetectorList(vector<int> DetectorIDs) { m_DetectorIDs = DetectorIDs; }
  //! Get the accepted detector list - if empty all are accepted!
  vector<int> GetDetectorList() const { return m_DetectorIDs; }

  //! Set the minimum total energy!
  void SetMinimumTotalEnergy(double MinimumTotalEnergy) { m_MinimumTotalEnergy = MinimumTotalEnergy; }
  //! Get the minimum total energy!
  double GetMinimumTotalEnergy() const { return m_MinimumTotalEnergy; }

  //! Set the maximum total energy!
  void SetMaximumTotalEnergy(double MaximumTotalEnergy) { m_MaximumTotalEnergy = MaximumTotalEnergy; }
  //! Get the maximum total energy!
  double GetMaximumTotalEnergy() const { return m_MaximumTotalEnergy; }

  //!AWL compute kinemaics
  double kinematics(MVector SourcePosition, MVector Site1, MVector Site2, double E0);

  //!AWL dump the CTD data
  void DumpCTDs(void);
  

  // protected methods:
 protected:
  
  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The list of detector to use
  vector<int> m_DetectorIDs;

  //! The minimum total energy
  double m_MinimumTotalEnergy;
  //! The minimum total energy
  double m_MaximumTotalEnergy;
  //CTD histogram output file
  ofstream m_CTDOut;
  //CTD histograms
  TH3I* m_CTDHist;
  //Source position vector
  MVector* m_SourceVector;
  //!number of 2 strip events
  unsigned int m_Good2Strip, m_Bad2Strip;
  unsigned int m_Good4Strip, m_Bad4Strip;
  unsigned int m_BadNStrips;
  double m_EOff;
  unsigned int m_Bad2StripEnergy, m_Bad2StripSH;
  unsigned int m_Bad4StripEnergy, m_Bad4StripSH;
  unsigned int m_2StripGoodEnergyBadSH;
  TH1D *m_CTD17, *m_CTD18;
  TTree* EventTree;
  TFile* RootFile;
  float m_YEnergy, m_X17Energy, m_X18Energy;
  float m_YTiming, m_X17Timing, m_X18Timing;


  
#ifdef ___CLING___
 public:
  ClassDef(MNCTModuleEventFilter, 0) // no description
#endif

};
#endif


////////////////////////////////////////////////////////////////////////////////
