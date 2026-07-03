/*
 * MModuleTrappingCorrection.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleTrappingCorrection__
#define __MModuleTrappingCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <unordered_map>
#include <vector>
#include <numeric>
#include <cmath>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MModuleEnergyCalibration.h"
#include "MDStrip3D.h"
#include "MDShapeBRIK.h"
#include "MGUIExpoTrappingCorrection.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleTrappingCorrection : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleTrappingCorrection();
  //! Default destructor
  virtual ~MModuleTrappingCorrection();
  
  //! Create a new object of this class 
  virtual MModuleTrappingCorrection* Clone() { return new MModuleTrappingCorrection(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Create the expos
  virtual void CreateExpos();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Set filename for coefficients file
  void SetSimCCEFileName( const MString& FileName) { m_SimCCEFile = FileName; }
  //! Get filename for coefficients file
  MString GetSimCCEFileName() const { return m_SimCCEFile; }



  //! Read the XML configuration
  bool ReadXmlConfiguration(MXmlNode* Node);

  //! Create the XML configuration
  MXmlNode* CreateXmlConfiguration();

  //! Finalize
  void Finalize();

  // protected methods:
 protected:
  //! Returns the strip with most energy from vector Strips, also gives back the energy fraction
  MStripHit* GetDominantStrip(std::vector<MStripHit*>& Strips, double& EnergyFraction);
  
  //! Retrieve the appropriate Depth values given the DetID
  vector<double> GetDepth(int DetID);
  
  //! Retrieve the appropriate CTD values given the DetID and Grade
  vector<double> GetCTD(int DetID, int Grade);

  //! Determine the Grade (geometry of charge sharing) of the Hit
  int GetHitGrade(MHit* H);

  //! Load in the specified coefficients file
  bool LoadSimCCEFile(MString FName);



  // private methods
  private:


  // protected members:
 protected:

  unordered_map<int, vector<double>> m_SimCCE;
  double m_SimCCE_Energy;
  MString m_SimCCEFile;
 
  unordered_map<int, MDDetector*> m_Detectors;
  vector<unsigned int> m_DetectorIDs;
  MModuleEnergyCalibration* m_EnergyCalibration;
  MGUIExpoTrappingCorrection* m_ExpoTrappingCorrection;

  // The CTD Map maps each detector (int) to a 2D array of CTD values.
  // unordered_map<int, vector<vector<double>>> m_CTDMap;
  // unordered_map<int, vector<double>> m_DepthGrid;
  // unordered_map<int, vector<TSpline3*>> m_SplineMap;

  bool m_SimCCEFileIsLoaded;



  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MModuleTrappingCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
