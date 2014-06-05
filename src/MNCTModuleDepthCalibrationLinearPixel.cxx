/*
 * MNCTModuleDepthCalibrationLinearPixel.cxx
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
// MNCTModuleDepthCalibrationLinearPixel
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDepthCalibrationLinearPixel.h"

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
#include "MNCTModule.h"
#include "MNCTMath.h"
#include "MStreams.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleDepthCalibrationLinearPixel)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibrationLinearPixel::MNCTModuleDepthCalibrationLinearPixel() : MNCTModule()
{
  // Construct an instance of MNCTModuleDepthCalibrationLinearPixel

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Linear Depth Calibration using pixel edge data";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DepthCalibrationLinearPixel";

  // Set all modules, which have to be done before this module
  //AddPreceedingModuleType(c_DetectorEffectsEngine);
  AddPreceedingModuleType(c_EnergyCalibration);
  //AddPreceedingModuleType(c_ChargeSharingCorrection);
  //AddPreceedingModuleType(c_DepthCorrection);
  AddPreceedingModuleType(c_StripPairing);
  //AddPreceedingModuleType(c_EventReconstruction);

  // Set all types this modules handles
  //AddModuleType(c_DetectorEffectsEngine);
  //AddModuleType(c_EnergyCalibration);
  //AddModuleType(c_ChargeSharingCorrection);
  AddModuleType(c_DepthCorrection);
  //AddModuleType(c_StripPairing);
  //AddModuleType(c_EventReconstruction);

  // Set all modules, which can follow this module
  //AddSucceedingModuleType(c_DetectorEffectsEngine);
  //AddSucceedingModuleType(c_EnergyCalibration);
  //AddSucceedingModuleType(c_ChargeSharingCorrection);
  //AddSucceedingModuleType(c_DepthCorrection);
  //AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_Aspect);
  AddSucceedingModuleType(c_EventReconstruction);
  AddSucceedingModuleType(c_Else);
  AddSucceedingModuleType(c_EventSaver);

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibrationLinearPixel::~MNCTModuleDepthCalibrationLinearPixel()
{
  // Delete this instance of MNCTModuleDepthCalibrationLinearPixel
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibrationLinearPixel::Initialize()
{
  // Initialize the module 

  // These defaults can be 'front' or 'back' of detector, depending on the detector
  m_Default_CTD_Negative = -200.;
  m_Default_CTD_Positive =  200.;

  for (int DetectorNumber=0; DetectorNumber<10; DetectorNumber++) {
    mout << "Attempting to load linear depth calibration (by pixel) for D" << DetectorNumber << endl;

    // Construct the filename of the detector-specific calibration file
    string DetectorNumberString;
    stringstream temp;
    temp << setfill('0') << setw(2) << DetectorNumber;
    DetectorNumberString = temp.str();
    MString FileName = (MString)std::getenv ("NUCLEARIZER_CAL")
      +"/depth_linear_pixel_D"+ DetectorNumberString + ".csv";

    // Reset flags telling if calibration has been loaded
    m_IsCalibrationLoaded[DetectorNumber] = false;
    for (int strip0=0; strip0<37; strip0++) {
      for (int strip1=0; strip1<37; strip1++) {
	m_IsCalibrationLoadedPixel[DetectorNumber][strip0][strip1] = false;
	m_IsCalibrationLoadedPixel[DetectorNumber][strip0][strip1] = false;
      }
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
	  int PositiveStripNumber, NegativeStripNumber;
	  double CTD_Front, CTD_Back, FWHM_Front, FWHM_Back;
	  if (sscanf(Line.Data(), "%d,%d,%lf,%lf,%lf,%lf\n",
		     &PositiveStripNumber,&NegativeStripNumber,&CTD_Front,&CTD_Back,
		     &FWHM_Front,&FWHM_Back) == 6) {
	    //mout << PosNeg << " " << StripNumber << " " 
	    //<< Timing_Front << " " << Timing_Back << " " << TimingFWHM_Front << " " << TimingFWHM_Back << endl;
	    m_Pixel_CTD_Front[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = CTD_Front;
	    m_Pixel_CTD_Back[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = CTD_Back;
	    m_Pixel_CTD_FWHM_Front[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = FWHM_Front;
	    m_Pixel_CTD_FWHM_Back[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = FWHM_Back;
	    m_IsCalibrationLoadedPixel[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = true;
	  }
	}
      }
    }  // done reading from file

    // check to see if all strips have been calibrated; load defaults if not
     m_IsCalibrationLoaded[DetectorNumber] = true;
     for (int XStripNumber=1; XStripNumber<=37; XStripNumber++) {
       for (int YStripNumber=1; YStripNumber<=37; YStripNumber++) {
 	if (m_IsCalibrationLoadedPixel[DetectorNumber][XStripNumber-1][YStripNumber-1] == false) {
 	  // Unset calibration flag for entire detector
 	  m_IsCalibrationLoaded[DetectorNumber] = false;
 	  // Set default calibration for the uncalibrated strip
 	  // Set default time resolution in FWHM
 	  m_Pixel_CTD_FWHM_Front[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_FWHM;
 	  m_Pixel_CTD_FWHM_Back[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_FWHM;
 	  // Set default times for front/back of detectors
 	  if ( (DetectorNumber == 7) || (DetectorNumber == 9) ) {
 	    // Positive strips are in front on these detectors, so use the positive default CTD
 	    // for the front CTD, and negative default CTD for the back CTD.
	    m_Pixel_CTD_Front[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_Positive;
	    m_Pixel_CTD_Back[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_Negative;
 	  } else {
 	    // Positive strips are in back on these detectors, so use the positive default CTD
 	    // for the back CTD, and negative default CTD for the front CTD.
	    m_Pixel_CTD_Front[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_Negative;
	    m_Pixel_CTD_Back[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_Positive;
 	  }
 	}
       } // YStripNumber
     } // XStripNumber

    if (m_IsCalibrationLoaded[DetectorNumber] == true) {
      mout << "Linear depth calibration (by pixel) for D" << DetectorNumber 
	   << " successfully loaded!" << endl;
    } else {
      mout << "***Warning: Unable to fully load linear depth calibration (by pixel) for D" 
	   << DetectorNumber << ".  Defaults were used for some or all strips." << endl;
    }

  } // 'DetectorNumber' loop


  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibrationLinearPixel::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  unsigned int NHits = Event->GetNHits();
  bool DepthCalibrated = true;
  //mout << "MNCTDepthCalibrationLinearPixel::AnalyzeEvent: Event ID = " << Event->GetID() << endl;

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
      double CTD_Front = m_Pixel_CTD_Front[DetectorNumber][XStripNumber-1][YStripNumber-1];
      double CTD_Back = m_Pixel_CTD_Back[DetectorNumber][XStripNumber-1][YStripNumber-1];
      double FWHM_Front = m_Pixel_CTD_FWHM_Front[DetectorNumber][XStripNumber-1][YStripNumber-1];
      double FWHM_Back = m_Pixel_CTD_FWHM_Back[DetectorNumber][XStripNumber-1][YStripNumber-1];

      // Calculate Z and the depth resolution!	
      double CTD = XTiming-YTiming;
      double Z_Front;
      double Z_FWHM = 0.1;
      double Z_Middle;
      Z_Front = 1.5*(CTD-CTD_Front)/(CTD_Back-CTD_Front);
      Z_FWHM = 1.5*0.5*(FWHM_Front+FWHM_Back)/(CTD_Back-CTD_Front);

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
	X_Middle = ((double)XStripNumber - 19.0)*0.2;
	Y_Middle = -((double)YStripNumber - 19.0)*0.2;
      } else {
	// Positive side is in back, positive strips are horizontal (so switch X and Y)
	X_Middle = ((double)YStripNumber - 19.0)*0.2;
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

  Event->SetDepthCalibrated(DepthCalibrated);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleDepthCalibrationLinearPixel::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleDepthCalibrationLinearPixel.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
