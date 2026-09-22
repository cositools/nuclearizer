/*
 * MSubModuleShieldReadout.h
 *
 * Copyright (C) by Andreas Zoglauer, Valentina Fioretti.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleShieldReadout__
#define __MSubModuleShieldReadout__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <map>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"
#include "MReadOutElementVoxel3D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleShieldReadout : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleShieldReadout();

  //! No copy constructor
  MSubModuleShieldReadout(const MSubModuleShieldReadout&) = delete;
  //! No copy assignment
  MSubModuleShieldReadout& operator=(const MSubModuleShieldReadout&) = delete;
  //! No move constructor
  MSubModuleShieldReadout(MSubModuleShieldReadout&&) = delete;
  //! No move assignment
  MSubModuleShieldReadout& operator=(MSubModuleShieldReadout&&) = delete;

  //! Default destructor
  virtual ~MSubModuleShieldReadout();

  //! Set the shield readout calibration file name
  void SetShieldReadoutFileName(const MString& FileName)
  {
    m_ShieldReadoutFileName = FileName;
  }

  //! Set the shield readout calibration file name - deprecated spelling kept for compatibility
  void SetShieldRedoutFileName(const MString& FileName)
  {
    SetShieldReadoutFileName(FileName);
  }

  //! Return the shield readout calibration file name
  MString GetShieldReadoutFileName() const
  {
    return m_ShieldReadoutFileName;
  }

  //! Initialize the module
  virtual bool Initialize();

  //! Clear event data from the module
  virtual void Clear();

  //! Convert the shield hit energies into ADC values
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Finalize the module
  virtual void Finalize();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);

  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration(MXmlNode* Node);

  // protected methods:
 protected:
  //! Read and parse the shield readout calibration CSV file
  bool ParseShieldReadoutFile();

  //! Compute ADC from the calibration ADC = A*Energy^2 + B*Energy + C
  double GetADC(double Energy, const MString& DetectorID, unsigned int CrystalID);

  // private methods:
 private:


  // protected members:
 protected:
  //! Shield readout calibration file name
  MString m_ShieldReadoutFileName;

  // private members:
 private:
  //! Calibration data for ADC = A*Energy^2 + B*Energy + C
  struct ShieldCalibration
  {
    //! Quadratic coefficient A
    double m_QuadraticCoefficient = 0.0;
    //! Linear coefficient B
    double m_LinearCoefficient = 0.0;
    //! Constant coefficient C
    double m_ConstantCoefficient = 0.0;
  };

  //! Calibration indexed by crystal-only MReadOutElementVoxel3D keys (detector ID + crystal ID)
  std::map<MReadOutElementVoxel3D, ShieldCalibration> m_ADCCalibration;

  //! Number of columns expected in the NRL shield calibration CSV
  static constexpr unsigned int m_NumberOfCSVColumns = 16;

  //! CSV column containing the detector/crystal identifier
  static constexpr unsigned int m_ChannelIdentifierColumn = 2;

  //! CSV column containing the quadratic calibration coefficient
  static constexpr unsigned int m_QuadraticCoefficientColumn = 8;

  //! CSV column containing the linear calibration coefficient
  static constexpr unsigned int m_LinearCoefficientColumn = 10;

  //! CSV column containing the constant calibration coefficient
  static constexpr unsigned int m_ConstantCoefficientColumn = 12;

  //! Maximum value of the 14-bit ADC
  static constexpr double m_MaxADCRange = 16383.0;



#ifdef ___CLING___
 public:
  ClassDef(MSubModuleShieldReadout, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////

