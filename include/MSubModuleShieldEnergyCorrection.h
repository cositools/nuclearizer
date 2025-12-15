/*
 * MSubModuleShieldEnergyCorrection.h
 *
 * Copyright (C) by Andreas Zoglauer, Valentina Fioretti.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleShieldEnergyCorrection__
#define __MSubModuleShieldEnergyCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TRandom3.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// based on MEGAlib library but created for Nuclearizer
#include "MReadOutElementVoxel3D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleShieldEnergyCorrection : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleShieldEnergyCorrection();

  //! No copy constructor
  MSubModuleShieldEnergyCorrection(const MSubModuleShieldEnergyCorrection&) = delete;
  //! No copy assignment
  MSubModuleShieldEnergyCorrection& operator=(const MSubModuleShieldEnergyCorrection&) = delete;
  //! No move constructors
  MSubModuleShieldEnergyCorrection(MSubModuleShieldEnergyCorrection&&) = delete;
  //! No move operators
  MSubModuleShieldEnergyCorrection& operator=(MSubModuleShieldEnergyCorrection&&) = delete;

  //! Default destructor
  virtual ~MSubModuleShieldEnergyCorrection();

  //! Set shield energy correction file name
  void SetShieldEnergyCorrectionFileName(const MString& FileName)
  {
    m_ShieldEnergyCorrectionFileName = FileName;
  }
  //! Get shield energy correction file name
  MString GetShieldEnergyCorrectionFileName() const
  {
    return m_ShieldEnergyCorrectionFileName;
  }

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
  //! Read in and parse the Shield energy correction file (fwhm and centroid)
  bool ParseShieldEnergyCorrectionFile();
  //! noise shield energy
  double NoiseShieldEnergyCentroid(double energy, int det_id, int crystal_id, int voxelx_id, int voxely_id, int voxelz_id);
  double NoiseShieldEnergyFWHM(double energy, int det_id, int crystal_id, int voxelx_id, int voxely_id, int voxelz_id);

  // private methods:
 private:
  // protected members:
 protected:
  //! Shield energy correction file name
  MString m_ShieldEnergyCorrectionFileName;

  // private members:
 private:
  //! Calibration map between Voxel3D read-out element and the energy resolution parameters
  map<MReadOutElementVoxel3D, TF1*> m_Centroid;
  map<MReadOutElementVoxel3D, TF1*> m_FWHM;

  //! The DEE internal random number generator
  TRandom3 m_Random;

#ifdef ___CLING___
 public:
  ClassDef(MSubModuleShieldEnergyCorrection, 0) // no description
#endif
};

#endif


////////////////////////////////////////////////////////////////////////////////
