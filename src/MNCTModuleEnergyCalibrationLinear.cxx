/*
 * MNCTModuleEnergyCalibrationLinear.cxx
 *
 *
 * Copyright (C) 2008-2009 by Mark Bandstra.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Mark Bandstra.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModuleEnergyCalibrationLinear
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleEnergyCalibrationLinear.h"

// Standard libs:
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

// ROOT libs:
#include "TGClient.h"
#include "MString.h"
#include "TFile.h"

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleEnergyCalibrationLinear)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEnergyCalibrationLinear::MNCTModuleEnergyCalibrationLinear() : MModule()
{
  // Construct an instance of MNCTModuleEnergyCalibrationLinear

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Linear Energy Calibration";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "EnergyCalibrationLinear";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_EnergyCalibration);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEnergyCalibrationLinear::~MNCTModuleEnergyCalibrationLinear()
{
  // Delete this instance of MNCTModuleEnergyCalibrationLinear
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEnergyCalibrationLinear::Initialize()
{
  // Initialize the module 

  m_DefaultGain = 0.225;
  m_DefaultFwhm = 2.0;

  for (int DetectorNumber=0; DetectorNumber<10; DetectorNumber++) {
    //mout << "Attempting to load linear energy calibration for D" << DetectorNumber << endl;

    // Construct the filename of the detector-specific calibration file
    string DetectorNumberString;
    stringstream temp;
    temp << setfill('0') << setw(2) << DetectorNumber;
    DetectorNumberString = temp.str();
    MString FileName = (MString)std::getenv ("NUCLEARIZER_CAL")
      +"/energy_linear_D"+ DetectorNumberString + ".csv";

    // Reset flags telling if calibration has been loaded
    m_IsCalibrationLoaded[DetectorNumber] = false;
    for (int i=0; i<37; i++) {
      m_IsCalibrationLoadedStrip[DetectorNumber][0][i] = false;
      m_IsCalibrationLoadedStrip[DetectorNumber][1][i] = false;
    }

    // Read the calibration coefficients line-by-line
    fstream File;
    File.open(FileName, ios_base::in);
    if (File.is_open() == false) {
      mout<<"***Warning: Unable to open file: "<<FileName<<endl
	  <<"   Is your NUCLEARIZER_CAL environment variable set?"<<endl;
    } else {
      MString Line;
      while(!File.eof()) {
	Line.ReadLine(File);
	if (Line.BeginsWith("#") == false) {
	  //mout << Line << endl;
	  char PosNeg;
	  int StripNumber, SideNumber;
	  double e_0, e_1, f_0, f_1, f_2;
	  //if (sscanf(Line.Data(), "%c,%d,%lf,%lf,%lf,%lf\n",
	  //	     &PosNeg,&StripNumber,&e_0,&e_1,&f_0,&f_1) == 6) {
	  if (sscanf(Line.Data(), "%c,%d,%lf,%lf,%lf,%lf,%lf\n",
		     &PosNeg,&StripNumber,&e_0,&e_1,&f_0,&f_1,&f_2) == 7) {
	    //mout << PosNeg << " " << StripNumber << " " 
	    // << e_0 << " " << e_1 << " " << f_0 << " " << f_1 << endl;
	    if (PosNeg == '+') { SideNumber = 0; } else { SideNumber = 1; }
	    m_CalibrationCoefficients[DetectorNumber][SideNumber][StripNumber-1][0] = e_0;
	    m_CalibrationCoefficients[DetectorNumber][SideNumber][StripNumber-1][1] = e_1;
	    m_FwhmCoefficients[DetectorNumber][SideNumber][StripNumber-1][0] = f_0;
	    m_FwhmCoefficients[DetectorNumber][SideNumber][StripNumber-1][1] = f_1;
	    m_FwhmCoefficients[DetectorNumber][SideNumber][StripNumber-1][2] = f_2;
	    m_IsCalibrationLoadedStrip[DetectorNumber][SideNumber][StripNumber-1] = true;
	  }
	}
      }
    }  // done reading from file

    // check to see if all strips have been calibrated; load defaults if not
    m_IsCalibrationLoaded[DetectorNumber] = true;
    for (int SideNumber=0; SideNumber<=1; SideNumber++) {
      for (int StripNumber=1; StripNumber<=37; StripNumber++) {
	if (m_IsCalibrationLoadedStrip[DetectorNumber][SideNumber][StripNumber-1] == false) {
	  // Unset calibration flag for entire detector
	  m_IsCalibrationLoaded[DetectorNumber] = false;
	  // Set default calibration for the uncalibrated strip
	  char PosNeg;
	  if (SideNumber == 0) { PosNeg='+'; } else { PosNeg='-'; }
	  mout << "***Warning: D" << DetectorNumber << " " << PosNeg << " Strip " << StripNumber
	       << ": No calibration data was found.  Using defaults." << endl;
	  m_CalibrationCoefficients[DetectorNumber][SideNumber][StripNumber-1][0] = 0.;
	  m_CalibrationCoefficients[DetectorNumber][SideNumber][StripNumber-1][1] = m_DefaultGain;
	  m_FwhmCoefficients[DetectorNumber][SideNumber][StripNumber-1][0] = m_DefaultFwhm*m_DefaultFwhm;
	  m_FwhmCoefficients[DetectorNumber][SideNumber][StripNumber-1][1] = 0.;
	  m_FwhmCoefficients[DetectorNumber][SideNumber][StripNumber-1][2] = 0.;
	}
      } // StripNumber
    } // SideNumber

    if (m_IsCalibrationLoaded[DetectorNumber] == true) {
      mout << "Linear energy calibration for D" << DetectorNumber << " successfully loaded!" << endl;
    } else {
      mout << "***Warning: Unable to fully load linear energy calibration for D" 
	   << DetectorNumber << ".  Defaults were used." << endl;
    }

  } // 'DetectorNumber' loop

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEnergyCalibrationLinear::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  unsigned int n = Event->GetNStripHits();
  //mout << "AnalyzeEvent: " << Event->GetID() << endl;

  for (unsigned int i=0; i<n; i++) {
    MNCTStripHit *SH = Event->GetStripHit(i);
    int DetectorNumber = SH->GetDetectorID();
    int SideNumber;
    if (SH->IsXStrip() == true) { SideNumber=0; } else { SideNumber=1; }
    int StripNumber = SH->GetStripID();
    double ADC = SH->GetADCUnits();
    double Energy = m_CalibrationCoefficients[DetectorNumber][SideNumber][StripNumber-1][0]
      + m_CalibrationCoefficients[DetectorNumber][SideNumber][StripNumber-1][1]*ADC;
    SH->SetEnergy(Energy);
    double EnergyFwhm = sqrt(m_FwhmCoefficients[DetectorNumber][SideNumber][StripNumber-1][0]
			     + m_FwhmCoefficients[DetectorNumber][SideNumber][StripNumber-1][1]*Energy
			     + m_FwhmCoefficients[DetectorNumber][SideNumber][StripNumber-1][2]*Energy*Energy);
    SH->SetEnergyResolution(EnergyFwhm/2.35);
    //mout << "StripHit: D" << DetectorNumber << " " << SideNumber << " " << StripNumber
    // << "  ADC: " << ADC << "  Energy: " << Energy << "  FWHM: " << EnergyFwhm << endl;
  }

  Event->SetEnergyCalibrated(true);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleEnergyCalibrationLinear::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleEnergyCalibrationLinear.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
