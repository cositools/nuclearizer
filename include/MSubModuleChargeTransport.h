/*
 * __MSubModuleChargeTransport__.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleChargeTransport__
#define __MSubModuleChargeTransport__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TSpline.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"
#include "MDStrip3D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Class handling the charge transport in the GeD detectors
//! End point is the energy in the individual strips and the guard ring
class MSubModuleChargeTransport : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleChargeTransport();

  //! No copy constructor
  MSubModuleChargeTransport(const MSubModuleChargeTransport&) = delete;
  //! No copy assignment
  MSubModuleChargeTransport& operator=(const MSubModuleChargeTransport&) = delete;
  //! No move constructors
  MSubModuleChargeTransport(MSubModuleChargeTransport&&) = delete;
  //! No move operators
  MSubModuleChargeTransport& operator=(MSubModuleChargeTransport&&) = delete;

  //! Set filename for CTD->Depth splines
  void SetDepthSplinesFileName( const MString& FileName) { m_DepthSplinesFileName = FileName; }
  //! Get filename for CTD->Depth splines
  MString GetDepthSplinesFileName() const {return m_DepthSplinesFileName; }
  //! Set filename for coefficients file
  void SetDepthCoefficientsFileName( const MString& FileName) { m_DepthCoefficientsFileName = FileName; }
  //! Get filename for coefficients file
  MString GetDepthCoefficientsFileName() const { return m_DepthCoefficientsFileName; }

  //! Default destructor
  virtual ~MSubModuleChargeTransport();

  //! Set geometry
  void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }

  //! Initialize the module
  virtual bool Initialize();

  //! Clear event data from the module
  virtual void Clear();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Finalize the module
  virtual void Finalize();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration(MXmlNode* Node);

  // protected methods:
 protected:

  //! Calculate charge fraction on a strip in local strip coordinates based on self-repulsion (η) and diffusion (σ)
  void RunChargeTransportForHit(MDEEStripHit& SH, bool isLV);

  // private methods:
 private:



  // protected members:
 protected:
  //! The geometry
  MDGeometryQuest* m_Geometry;

  //! The detector dimensions
  unordered_map<int, double> m_Thicknesses;
  unordered_map<int, int> m_NXStrips;
  unordered_map<int, int> m_NYStrips;
  unordered_map<int, double> m_XPitches;
  unordered_map<int, double> m_YPitches;
  unordered_map<int, double> m_XWidths;
  unordered_map<int, double> m_YWidths;
  unordered_map<int, double> m_Radii;
  unordered_map<int, MDDetector*> m_Detectors;
  vector<unsigned int> m_DetectorIDs;

  list<MDEEStripHit> m_ChargeTransportHits;

  //! Filename of the depth calibration coefficients (stretch, offset, timing noise, ...)
  MString m_DepthCoefficientsFileName;

  //! Map of the depth calibration coefficients
  unordered_map<int, vector<double>> m_Coeffs;

  //! Filename of CTD->Depth splines
  MString m_DepthSplinesFileName;

  // Analog of the CTD-to-depth splines in MModuleDepthCalibration:
  //! Map: detector ID (int) -> vector containing depth values
  unordered_map<int, vector<double>> m_DepthGrid;
  //! Map: detector ID (int) -> simulated electron drift times (+ electronics) for the depth values in m_DepthGrid
  unordered_map<int, vector<double>> m_ElectronDriftTimes;
  //! The corresponding electron-drift-time interpolation spline
  unordered_map<int, TSpline3*> m_ElectronDriftSplines;
  //! Map: detector ID (int) -> simulated hole drift times (+ electronics) for the depth values in m_DepthGrid
  unordered_map<int, vector<double>> m_HoleDriftTimes;
  //! The corresponding hole-drift-time interpolation spline
  unordered_map<int, TSpline3*> m_HoleDriftSplines;

  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MSubModuleChargeTransport, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
