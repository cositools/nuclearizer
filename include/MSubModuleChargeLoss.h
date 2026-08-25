/*
 * MSubModuleChargeLoss.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleChargeLoss__
#define __MSubModuleChargeLoss__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Charge-loss correction for adjacent same-side strips in the GeD detectors.
//! When one interaction shares its charge across two adjacent strips on the same side,
//! their summed energy droops below the true energy. This submodule subtracts that loss
//! using a per-detector/side parabola read from a CSV grid:
//!   loss = a * (trueSum^2 - diff^2)
//! applied in the edge (|diff| >= boundary) with edge_a, and in the core (|diff| < boundary)
//! with core_a. a and boundary are interpolated in (energy, depth).
class MSubModuleChargeLoss : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleChargeLoss();

  //! No copy constructor
  MSubModuleChargeLoss(const MSubModuleChargeLoss&) = delete;
  //! No copy assignment
  MSubModuleChargeLoss& operator=(const MSubModuleChargeLoss&) = delete;
  //! No move constructors
  MSubModuleChargeLoss(MSubModuleChargeLoss&&) = delete;
  //! No move operators
  MSubModuleChargeLoss& operator=(MSubModuleChargeLoss&&) = delete;

  //! Default destructor
  virtual ~MSubModuleChargeLoss();

  //! Set the charge-loss CSV file name
  void SetChargeLossFileName( const MString& FileName) { m_ChargeLossFileName = FileName; }
  //! Get the charge-loss CSV file name
  MString GetChargeLossFileName() const { return m_ChargeLossFileName; }

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

  //! Clamped linear interpolation of the value grid over energy; holds the endpoint outside the grid.
  double ChargeLossInterp1D(double chargeLossEnergy, const vector<double>& chargeLossEnergyGrid,
                            const vector<double>& chargeLossValueGrid);
  //! Bilinear interpolation of chargeLossValueGrid[iDepth][iEnergy] at (energy, depth); clamps both axes.
  double ChargeLossInterp2D(double chargeLossEnergy, double chargeLossDepth,
                            const vector<double>& chargeLossEnergyGrid, const vector<double>& chargeLossDepthGrid,
                            const vector<vector<double> >& chargeLossValueGrid);

  // private methods:
 private:



  // protected members:
 protected:
  //! COSI constants
  static const int nDets = 12;
  static const int nSides = 2;
  //! Charge-loss energy range in keV (the CSV grid max; matches the charge-sharing 0-2000 domain)
  static const int ChargeLossMaxEnergy = 2000;

  //! Name of the charge-loss CSV (columns: detector, side, depth_cm, energy_keV, boundary_keV, edge_a, core_a)
  MString m_ChargeLossFileName;

  //! Charge-loss curves per detector/side, loaded from the CSV (dense grid). loss = a*(trueSum^2 - diff^2):
  //! edge_a in the edge (|diff| >= boundary), core_a in the core (|diff| < boundary). Depth is a future axis.
  vector<double> m_ChargeLossEnergy[nDets][nSides];          //!< energy-axis grid
  vector<double> m_ChargeLossDepth[nDets][nSides];           //!< depth-axis grid (1 entry until calibrated)
  vector<double> m_ChargeLossBoundary[nDets][nSides];        //!< boundary(E), energy-only
  vector<vector<double> > m_ChargeLossEdgeA[nDets][nSides];  //!< edge_a[iDepth][iEnergy]  (|diff| >= boundary)
  vector<vector<double> > m_ChargeLossCoreA[nDets][nSides];  //!< core_a[iDepth][iEnergy]  (|diff| <  boundary)

  //! Count of strip pairs that actually received a correction (diagnostic, printed in Finalize)
  unsigned long m_ChargeLossCounter;

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MSubModuleChargeLoss, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
