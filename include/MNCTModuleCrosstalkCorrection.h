/*
 * MNCTModuleCrosstalkCorrection.h
 *
 * Copyright (C) 2009-2009 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

#define MAXNSKIP 3

#ifndef __MNCTModuleCrosstalkCorrection__
#define __MNCTModuleCrosstalkCorrection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleCrosstalkCorrection : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleCrosstalkCorrection();
  //! Default destructor
  virtual ~MNCTModuleCrosstalkCorrection();
  
  //! Create a new object of this class 
  virtual MNCTModuleCrosstalkCorrection* Clone() { return new MNCTModuleCrosstalkCorrection(); }

  //! Set the calibration file name
  void SetFileName(const MString& FileName) { m_FileName = FileName; }
  //! Get the calibration file name
  MString GetFileName() const { return m_FileName; }



  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();



  // protected methods:
 protected:
  //! The calibration file name
  MString m_FileName;

  // private methods:
 private:
  // Method to make the cross-talk correction on a vector of strip hits
  virtual void CorrectCrosstalk(vector<MNCTStripHit*> StripHits, int det, unsigned int side);


  // protected members:
 protected:


  // private members:
 private:
  bool m_IsCalibrationLoadedDet[12];
  bool m_IsCalibrationLoaded[12][2][3];
  double m_CrosstalkCoeffs[12][2][3][2];

#ifdef ___CLING___
 public:
  ClassDef(MNCTModuleCrosstalkCorrection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
