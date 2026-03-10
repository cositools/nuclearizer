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
  double CalculateChargeFraction(double x, double Eta, double Sigma);

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


  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MSubModuleChargeTransport, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
