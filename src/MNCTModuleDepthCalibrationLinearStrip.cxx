/*
 * MNCTModuleDepthCalibrationLinearStrip.cxx
 *
 *
 * Copyright (C) 2008-2008 by Mark Bandstra.
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
// MNCTModuleDepthCalibrationLinearStrip
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDepthCalibrationLinearStrip.h"

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
#include "MNCTMath.h"
#include "MStreams.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNCTModuleDepthCalibrationLinearStrip)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibrationLinearStrip::MNCTModuleDepthCalibrationLinearStrip() : MModule()
{
  // Construct an instance of MNCTModuleDepthCalibrationLinearStrip

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Linear Depth Calibration using strip edge data (very crude)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DepthCalibrationLinearStrip";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_StripPairing);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_DepthCorrection);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibrationLinearStrip::~MNCTModuleDepthCalibrationLinearStrip()
{
  // Delete this instance of MNCTModuleDepthCalibrationLinearStrip
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibrationLinearStrip::Initialize()
{
  // Initialize the module 

  // These defaults can be 'front' or 'back' of detector, depending on the detector
  m_DefaultTiming_Low = 100.;
  m_DefaultTiming_High = 250.;

  for (int DetectorNumber=0; DetectorNumber<10; DetectorNumber++) {
    mout << "Attempting to load linear depth calibration (by strip) for D" << DetectorNumber << endl;

    // Construct the filename of the detector-specific calibration file
    string DetectorNumberString;
    stringstream temp;
    temp << setfill('0') << setw(2) << DetectorNumber;
    DetectorNumberString = temp.str();
    MString FileName = (MString)std::getenv ("NUCLEARIZER_CAL")
      +"/depth_linear_strip_D"+ DetectorNumberString + ".csv";

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
	  double Timing_Front, Timing_Back, TimingFWHM_Front, TimingFWHM_Back;
	  if (sscanf(Line.Data(), "%c,%d,%lf,%lf,%lf,%lf\n",
		     &PosNeg,&StripNumber,&Timing_Front,&Timing_Back,&TimingFWHM_Front,&TimingFWHM_Back) == 6) {
	    //mout << PosNeg << " " << StripNumber << " " 
	    //<< Timing_Front << " " << Timing_Back << " " << TimingFWHM_Front << " " << TimingFWHM_Back << endl;
	    if (PosNeg == '+') { SideNumber = 0; } else { SideNumber = 1; }
	    m_StripTiming_Front[DetectorNumber][SideNumber][StripNumber-1] = Timing_Front;
	    m_StripTiming_Back[DetectorNumber][SideNumber][StripNumber-1] = Timing_Back;
	    m_StripTimingFWHM_Front[DetectorNumber][SideNumber][StripNumber-1] = TimingFWHM_Front;
	    m_StripTimingFWHM_Back[DetectorNumber][SideNumber][StripNumber-1] = TimingFWHM_Back;
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
	  //mout << "***Warning: D" << DetectorNumber << " " << PosNeg << " Strip " << StripNumber
	  //     << ": No linear depth calibration data was found.  Using defaults." << endl;
	  // Set default time resolution in FWHM
	  m_StripTimingFWHM_Front[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTimingFWHM;
	  m_StripTimingFWHM_Back[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTimingFWHM;
	  // Set default times for front/back of detectors
	  if ( (DetectorNumber == 7) || (DetectorNumber == 9) ) {
	    // Positive strips are in front on these detectors, so use the low default time
	    // for the positive side front (side 0), and high default time for the negative 
	    // side front (side 1).
	    if (SideNumber == 0) {
	      m_StripTiming_Front[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTiming_Low;
	      m_StripTiming_Back[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTiming_High;
	    } else {
	      m_StripTiming_Front[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTiming_High;
	      m_StripTiming_Back[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTiming_Low;
	    }
	  } else {
	    // Positive strips are in back on all other detectors, so use the high default time
	    // for the positive side front (side 0), and low default time for the negative 
	    // side front (side 1).
	    if (SideNumber == 0) {
	      m_StripTiming_Front[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTiming_High;
	      m_StripTiming_Back[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTiming_Low;
	    } else {
	      m_StripTiming_Front[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTiming_Low;
	      m_StripTiming_Back[DetectorNumber][SideNumber][StripNumber-1] = m_DefaultTiming_High;
	    }
	  }
	}
      } // StripNumber
    } // SideNumber

    if (m_IsCalibrationLoaded[DetectorNumber] == true) {
      mout << "Linear depth calibration (by strip) for D" << DetectorNumber 
	   << " successfully loaded!" << endl;
    } else {
      mout << "***Warning: Unable to fully load linear depth calibration (by strip) for D" 
	   << DetectorNumber << ".  Defaults were used for some or all strips." << endl;
    }

  } // 'DetectorNumber' loop


  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibrationLinearStrip::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  unsigned int NHits = Event->GetNHits();
  bool DepthCalibrated = true;
  //mout << "MNCTDepthCalibrationLinearStrip::AnalyzeEvent: Event ID = " << Event->GetID() << endl;

  //mout << endl << "Event ID: " << Event->GetID() << endl;
  for (unsigned int i_hit=0; i_hit<NHits; i_hit++) {
    MNCTHit *H = Event->GetHit(i_hit);
    unsigned int NStripHits = H->GetNStripHits();
    unsigned int NXStripHits = 0, NYStripHits = 0;
    for (unsigned int i_sh=0; i_sh<NStripHits; i_sh++) {
      MNCTStripHit *SH = H->GetStripHit(i_sh);
      if (SH->IsXStrip() == true) { NXStripHits++; } else { NYStripHits++; }
    }
    // Require 2 strip hits: 1 X strip and 1 Y strip
    if ( (NStripHits == 2) && (NXStripHits == 1) && (NYStripHits == 1) ) {
      MNCTStripHit *SHX, *SHY;
      SHX = H->GetStripHit(0);
      SHY = H->GetStripHit(1);
      if ( !SHX->IsXStrip() ) {
	SHX = H->GetStripHit(1);
	SHY = H->GetStripHit(0);
      }
      // X and Y Strip hit info
      int DetectorNumber = SHX->GetDetectorID();
      stringstream temp;
      temp << 'D' << setfill('0') << setw(2) << DetectorNumber;
      MString DetectorName = temp.str();
      int XStripNumber = SHX->GetStripID();
      int YStripNumber = SHY->GetStripID();
      double XTiming = SHX->GetTiming();
      double YTiming = SHY->GetTiming();

      // Calibration parameters
      double XTiming_Front = m_StripTiming_Front[DetectorNumber][0][XStripNumber-1];
      double XTiming_Back = m_StripTiming_Back[DetectorNumber][0][XStripNumber-1];
      double YTiming_Front = m_StripTiming_Front[DetectorNumber][1][YStripNumber-1];
      double YTiming_Back = m_StripTiming_Back[DetectorNumber][1][YStripNumber-1];
      double XTimingFWHM_Front = m_StripTimingFWHM_Front[DetectorNumber][0][XStripNumber-1];
      double XTimingFWHM_Back = m_StripTimingFWHM_Back[DetectorNumber][0][XStripNumber-1];
      double YTimingFWHM_Front = m_StripTimingFWHM_Front[DetectorNumber][1][YStripNumber-1];
      double YTimingFWHM_Back = m_StripTimingFWHM_Back[DetectorNumber][1][YStripNumber-1];

      // Calculate Z and the depth resolution!	
      double Z_Front = 0.75;
      double Z_FWHM = 0.1;
      double Z_X, Z_Y;
      double Z_Middle;
      Z_X = 1.5*(XTiming-XTiming_Front)/(XTiming_Back-XTiming_Front);
      Z_Y = 1.5*(YTiming-YTiming_Front)/(YTiming_Back-YTiming_Front);
      Z_Front = 0.5*(Z_X + Z_Y);
      Z_FWHM = 1.5*0.25*sqrt( pow((XTimingFWHM_Front+XTimingFWHM_Back)/(XTiming_Back-XTiming_Front), 2.0) 
      		      + pow((YTimingFWHM_Front+YTimingFWHM_Back)/(YTiming_Back-YTiming_Front),2.0) );
      // Depth should be between 0 (front) and 1.5 (back)
      if (Z_Front < 0.) { Z_Front=0.; }
      if (Z_Front > 1.5) { Z_Front=1.5; }
      Z_Middle = 0.75-Z_Front;

      // Calculate X and Y positions based on strip number.  These are referenced from the middle.
      double X_Middle=0., Y_Middle=0.;;
      if (DetectorNumber % 2 == 0) {
	// Positive side is in back, positive strips are vertical (i.e., give X position)
	X_Middle = -((double)XStripNumber - 19.0)*0.2;
	Y_Middle = -((double)YStripNumber - 19.0)*0.2;
      } else if (DetectorNumber == 9 || DetectorNumber == 7) {
	// Positive side is in front, positive strips are vertical (i.e., give X position)
	X_Middle =  ((double)XStripNumber - 19.0)*0.2;
	Y_Middle = -((double)YStripNumber - 19.0)*0.2;
      } else {
	// Positive side is in back, positive strips are horizontal (so switch X and Y)
	X_Middle =  ((double)YStripNumber - 19.0)*0.2;
	Y_Middle = -((double)XStripNumber - 19.0)*0.2;
      }

      // Set depth and depth resolution for the hit, relative to the center of the detector volume
      MVector PositionInDetector(X_Middle, Y_Middle, Z_Middle);
      MVector PositionResolution(2.0/2.35, 2.0/2.35, Z_FWHM/2.35);
      MVector PositionInGlobal = m_Geometry->GetGlobalPosition(PositionInDetector, DetectorName);
      //mout << "Pos in det:    " << PositionInDetector << endl;
      //mout << "Pos in global: " << PositionInGlobal << endl;
      H->SetPosition(PositionInGlobal);
      H->SetPositionResolution(PositionResolution);
      //mout << "Hit: D" << DetectorNumber << " X:" << XStripNumber << " (" << X_Middle << " cm)  "
      //   << " Y:" << YStripNumber << " (" << Y_Middle << " cm)  "
      //   << " X Timing: " << XTiming << " Y Timing: " << YTiming 
      //   << " Z_X: " << Z_X << " Z_Y: " << Z_Y 
      //   << " Z: " << Z_Front << " cm  Z res.: " << Z_FWHM << " cm" << endl;
    } else {
      //mout << "Linear depth calibration cannot calculate depth for hit.  "
      //   << "Doesn't contain exactly one X and one Y strip hit." << endl;
      DepthCalibrated=false;
    }
  }

  Event->SetAnalysisProgress(MAssembly::c_DepthCorrection | MAssembly::c_PositionDetermiation);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleDepthCalibrationLinearStrip::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleDepthCalibrationLinearStrip.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
