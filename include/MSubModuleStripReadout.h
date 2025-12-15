/*
 * MSubModuleStripReadout.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSubModuleStripReadout__
#define __MSubModuleStripReadout__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSubModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSubModuleStripReadout : public MSubModule
{
  // public interface:
 public:
  //! Default constructor
  MSubModuleStripReadout();

  //! No copy constructor
  MSubModuleStripReadout(const MSubModuleStripReadout&) = delete;
  //! No copy assignment
  MSubModuleStripReadout& operator=(const MSubModuleStripReadout&) = delete;
  //! No move constructors
  MSubModuleStripReadout(MSubModuleStripReadout&&) = delete;
  //! No move operators
  MSubModuleStripReadout& operator=(MSubModuleStripReadout&&) = delete;

  //! Default destructor
  virtual ~MSubModuleStripReadout();

  //! Set energy calibration file name
  void SetEnergyCalibrationFileName(const MString& FileName) { m_EnergyCalibrationFileName = FileName; }
  //! Set energy calibration file name
  MString GetEnergyCalibrationFileName() const { return m_EnergyCalibrationFileName; }

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

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Energy calibration file name
  MString m_EnergyCalibrationFileName;



#ifdef ___CLING___
 public:
  ClassDef(MSubModuleStripReadout, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
