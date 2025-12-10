/*
 * MModuleEventFilter.h
 *
 * Copyright (C) by Andreas Zoglauer
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleEventFilter__
#define __MModuleEventFilter__


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


class MModuleEventFilter : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleEventFilter();
  //! Default destructor
  virtual ~MModuleEventFilter();
  
  //! Create a new object of this class 
  virtual MModuleEventFilter* Clone() { return new MModuleEventFilter(); }

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

  //! Set the minimum total energy
  void SetMinimumTotalEnergy(double MinimumTotalEnergy) { m_MinimumTotalEnergy = MinimumTotalEnergy; }
  //! Get the minimum total energy
  double GetMinimumTotalEnergy() const { return m_MinimumTotalEnergy; }

  //! Set the maximum total energy
  void SetMaximumTotalEnergy(double MaximumTotalEnergy) { m_MaximumTotalEnergy = MaximumTotalEnergy; }
  //! Get the maximum total energy
  double GetMaximumTotalEnergy() const { return m_MaximumTotalEnergy; }

  //! Set the minimum allowed number of HV strips
  void SetMinimumHVStrips(double MinimumHVStrips) { m_MinimumHVStrips = MinimumHVStrips; }
  //! Get the minimum allowed number of HV strips
  double GetMinimumHVStrips() const { return m_MinimumHVStrips; }

  //! Set the maximum allowed number of HV strips
  void SetMaximumHVStrips(double MaximumHVStrips) { m_MaximumHVStrips = MaximumHVStrips; }
  //! Get the maximum allowed number of HV strips
  double GetMaximumHVStrips() const { return m_MaximumHVStrips; }

  //! Set the minimum allowed number of LV strips
  void SetMinimumLVStrips(double MinimumLVStrips) { m_MinimumLVStrips = MinimumLVStrips; }
  //! Get the minimum allowed number of LV strips
  double GetMinimumLVStrips() const { return m_MinimumLVStrips; }

  //! Set the maximum allowed number of HV strips
  void SetMaximumLVStrips(double MaximumLVStrips) { m_MaximumLVStrips = MaximumLVStrips; }
  //! Get the maximum allowed number of HV strips
  double GetMaximumLVStrips() const { return m_MaximumLVStrips; }

  //! Set the minimum number of hits
  void SetMinimumHits(double MinimumHits) { m_MinimumHits = MinimumHits; }
  //! Get the minimum number of hits
  double GetMinimumHits() const { return m_MinimumHits; }

  //! Set the maximum number of hits
  void SetMaximumHits(double MaximumHits) { m_MaximumHits = MaximumHits; }
  //! Get the maximum number of hits
  double GetMaximumHits() const { return m_MaximumHits; }

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
  //! The maximum total energy
  double m_MaximumTotalEnergy;

  // The minimum number of HV strips
  unsigned int m_MinimumHVStrips;
  // The maximum number of HV strips
  unsigned int m_MaximumHVStrips;

  // The minimum number of LV strips
  unsigned int m_MinimumLVStrips;
  // The maximum number of LV strips
  unsigned int m_MaximumLVStrips;

  //! The minimum number of hits
  unsigned int m_MinimumHits;
  //! The maximum number of hits
  unsigned int m_MaximumHits;

  
#ifdef ___CLING___
 public:
  ClassDef(MModuleEventFilter, 0) // no description
#endif

};
#endif


////////////////////////////////////////////////////////////////////////////////
