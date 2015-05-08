/*
 * MNCTModuleDepthCalibration3rdPolyPixel.cxx
 *
 *
 * Copyright (C) by Carolyn Kierans, Mark Bandstra.
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
// MNCTModuleDepthCalibration3rdPolyPixel
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDepthCalibration3rdPolyPixel.h"

// // Standard libs:
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
#include "MFile.h"
#include "MNCTMath.h"
#include "MStreams.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleDepthCalibration3rdPolyPixel)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibration3rdPolyPixel::MNCTModuleDepthCalibration3rdPolyPixel() : MModule()
{
  // Construct an instance of MNCTModuleDepthCalibration3rdPolyPixel
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "3rd order Polynomial Depth Calibration using pixel edge data";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DepthCalibration3rdPolyPixel";
  
  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_StripPairing);
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_DepthCorrection);
  AddModuleType(MAssembly::c_PositionDetermiation);
  
  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  
  // Set the histogram display
  m_ExpoDepthCalibration = new MGUIExpoDepthCalibration(this);
  m_ExpoDepthCalibration->SetDepthHistogramArrangement(3, 4);
  //m_ExpoDepthCalibration->SetDepthHistogramParameters(75, -0.000001, 1.500001); //-0.7499, 0.7501);
  m_ExpoDepthCalibration->SetDepthHistogramParameters(75, -0.5, 2.0);
  m_Expos.push_back(m_ExpoDepthCalibration);  
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = true;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibration3rdPolyPixel::~MNCTModuleDepthCalibration3rdPolyPixel()
{
  // Delete this instance of MNCTModuleDepthCalibration3rdPolyPixel
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibration3rdPolyPixel::Initialize()
{
  // Initialize the module 
  
  // Definition:
  // Depth is the distance between interaction and cathode (negative side).
  // CTD is "Timing(+) - Timing(-)".
  // The relationship between CTD and depth is: Depth = CTD2Depth[i] * CTD^i, i=0~3
  // The CTD2Depth[i] is the parameters from 3rd polynomial fit.
  
  // These default parameters used linear curve from default CTD vaules:
  // CTD(near -):-200. , CTD(near +):200. // Actually, I think these should be +/- 320ns. This is what the CC were set to for the '14 flight, according to Alex
  double default_ctd2z[4]={0.75,-0.00375,0.0,0.0};
  int N_parameter;
 
  //Clean this part up... 
  ShareHitNumber0=0; //The number of charge sharing hits that have two adjacent strips on one side and only one strip on the other, or 2 adjacent strips on both sides 
  ShareHitNumber1=0; //Number of 3 or 4 strip hits that do not fall into the catagory above 
  SingleHitNumber=0; //Number of single pixel hits
  OtherHitNumber=0;  //The number of hits that have more that 4 strip hits
  ShareEventNumber0=0; //The number of events which include at least one charge sharing hit and a single pixel hit
  ShareEventNumber1=0; //The number of events which include at least one charge sharing hit and no single pixel hit
  SingleEventNumber=0; //The number of events which include only single pixel hits
  OtherEventNumber=0;
  LLDNumber=0;
  NotValidStripNumber=0;
  InvalidEventNumber=0; //The number of events that are flagged as uncalibratable
  //currently using InvalidEventNumber as a counter for pixel events that are out of bounds
  OutofBoundsDepth=0;
 
  for (N_parameter=0;N_parameter<4;N_parameter++) {
    m_Default_CTD2Depth[N_parameter]=default_ctd2z[N_parameter];
  }
  
  for (int DetectorNumber=0; DetectorNumber<12; DetectorNumber++) {   
    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Attempting to load depth calibration (by pixel) for D" << DetectorNumber << endl;
    
    // Construct the filename of the detector-specific calibration file
    string DetectorNumberString;
    stringstream temp;
    if (DetectorNumber < 10) {
      temp << setfill('0') << setw(2) << DetectorNumber;
      DetectorNumberString = temp.str();
    } else {
      temp << setw(2) << DetectorNumber;
      DetectorNumberString = temp.str();
    }
    MString FileName = "$(NUCLEARIZER)/resource/calibration/COSI14/Antarctica/depth_3rdPoly_pixel_CC"+ DetectorNumberString + ".csv";
    MFile::ExpandFileName(FileName);
    
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
      //if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: Unable to open file: "<<FileName<<endl
      cout<< "Error: Unable to open file: "<<FileName<<endl
		<<"   Is your NUCLEARIZER_CAL environment variable set?"<<endl;
      return false;
    } else {
      MString Line;
      while(!File.eof()) {
        Line.ReadLine(File);
        if (Line.BeginsWith("#") == false) {
          //cout << Line << endl;
          int PositiveStripNumber, NegativeStripNumber;
          double ctd2z0, ctd2z1, ctd2z2, ctd2z3, FWHM_positive, FWHM_negative;
          if (sscanf(Line.Data(), "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf\n",
            &PositiveStripNumber,&NegativeStripNumber,&ctd2z0,&ctd2z1,
            &ctd2z2,&ctd2z3,&FWHM_positive,&FWHM_negative) == 8) {
            //cout << "load parameter:[(s+),p0,p3,fwhm(+)]" << " " << PositiveStripNumber << " " 
            //<< ctd2z0 << " " << ctd2z3 << " " << FWHM_positive << endl;
            
            m_Pixel_CTD2Depth[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1][0] = ctd2z0;
            m_Pixel_CTD2Depth[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1][1] = ctd2z1;
            m_Pixel_CTD2Depth[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1][2] = ctd2z2;
            m_Pixel_CTD2Depth[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1][3] = ctd2z3;
            m_Pixel_CTD_FWHM_Positive[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = FWHM_positive;
            m_Pixel_CTD_FWHM_Negative[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = FWHM_negative;
            m_IsCalibrationLoadedPixel[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = true;
          }
        }
      }
    }  // done reading from file
    
    // check to see if all strips have been calibrated; load defaults if not. These defaults were passed down from previous NCT. I haven't gone through to see if they are still valid.
    m_IsCalibrationLoaded[DetectorNumber] = true;
    cout<<" Det #"<<DetectorNumber<<" is Calibrated?: "<<m_IsCalibrationLoaded[DetectorNumber]<<endl;
    for (int XStripNumber=1; XStripNumber<=37; XStripNumber++) {
      for (int YStripNumber=1; YStripNumber<=37; YStripNumber++) {
        if (m_IsCalibrationLoadedPixel[DetectorNumber][XStripNumber-1][YStripNumber-1] == false) {
          // Unset calibration flag for entire detector
          m_IsCalibrationLoaded[DetectorNumber] = false;
          // Set default calibration for the uncalibrated strip
          // Set default time resolution in FWHM
          m_Pixel_CTD_FWHM_Positive[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_FWHM;
          m_Pixel_CTD_FWHM_Negative[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_FWHM;
          // Set default times for front/back of detectors
          for (N_parameter=0;N_parameter<4;N_parameter++) {
            m_Pixel_CTD2Depth[DetectorNumber][XStripNumber-1][YStripNumber-1][N_parameter]
            = m_Default_CTD2Depth[N_parameter];
          } // set default parameter
        }
      } // YStripNumber
    } // XStripNumber
    
    if (m_IsCalibrationLoaded[DetectorNumber] == true) {
      if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": 3rd order polynomial depth calibration (by pixel) for D" << DetectorNumber 
      << " successfully loaded!" << endl;
    } else {
      if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": Warning: Unable to fully load 3rd polynomial depth calibration (by pixel) for D" 
      << DetectorNumber << ".  Defaults were used for some or all strips." << endl;
      // return false;
    }
    
  } // 'DetectorNumber' loop
  
  
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibration3rdPolyPixel::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 
  unsigned int EventTypeFlag0=0; //single pixel event
  unsigned int EventTypeFlag1=0; //charge sharing event - two adjacent strips on one or both sides
  unsigned int EventTypeFlag2=0; //Non-adjacent strips within the hit or 3 strip hits on one side and only one on the other - not calibrated and should not happen often.
  unsigned int EventTypeFlag3=0; //Cannot be calibrated - more that 4 strip hits
  unsigned int LLDEvents = 0;
  unsigned int EventStripNotValid = 0;
  unsigned int FlagUncali =0;
  unsigned int NoTiming_Event = 0;
  unsigned int OutofRange = 0;

  unsigned int NHits = Event->GetNHits();
  //bool DepthCalibrated = false;
  //cout << "MNCTDepthCalibration3rdPolyPixel::AnalyzeEvent: Event ID = " << Event->GetID() << endl;
  
  //cout << endl << "Event ID: " << Event->GetID() << endl;
  for (unsigned int i_hit=0; i_hit<NHits; i_hit++) {
    MNCTHit *H = Event->GetHit(i_hit);
    unsigned int NStripHits = H->GetNStripHits();
    unsigned int NXStripHits = 0, NYStripHits = 0;
    int DetectorNumber = -1;
    for (unsigned int i_sh=0; i_sh<NStripHits; i_sh++) {
      MNCTStripHit *SH = H->GetStripHit(i_sh);
      DetectorNumber = SH->GetDetectorID();	
      if (SH->IsXStrip() == true) { NXStripHits++; } else { NYStripHits++; }
    }
    //Check for 1 pixel event and sharing event..
    int Flag_CanBeCalibrated = -1; //Flag_CanBeCalibrated = 0 for uncalibratable events (LLD, false timing, too many strip hits, etc). Flag_CanBeCalibrated can = 1 or 2 for valid events. The difference between the two?:
	//int NoTiming_Event = 0; // = LLD Event
    double XTiming = 0.0, YTiming = 0.0;
    int XStripNumber = 0, YStripNumber = 0;
    MNCTStripHit *SHX = nullptr, *SHY = nullptr;
    //int DetectorNumber = SHX->GetDetectorID();
    MString DetectorName;
    int DisplayID = 0;
    MString DisplayName;
    
    //All of the "DetectorNumbers" within this file and elsewhere in Nuclearizer refer to CC #. When we call upon the geometry file later on here to determine the globale position of all the detector, we want to be calling the DetectorName not the CC #. Here is the conversion:

    if (DetectorNumber == 0) {
      DetectorName = "Detector0";
      DisplayID = 2;
      DisplayName = DetectorName + ": -x, -y, top";
    }
    else if (DetectorNumber == 1) {
      DetectorName = "Detector1";
      DisplayID = 1;
      DisplayName = DetectorName + ": -x, -y, middle";
    }
    else if (DetectorNumber == 2) {
      DetectorName = "Detector2";
      DisplayID = 0;
      DisplayName = DetectorName + ": -x, -y, bottom";
    }
    else if (DetectorNumber == 3) {
      DetectorName = "Detector3";
      DisplayID = 3;
      DisplayName = DetectorName + ": +x, -y, bottom";
    }
    else if (DetectorNumber == 4) {
      DetectorName = "Detector4";
      DisplayID = 4;
      DisplayName = DetectorName + ": +x, -y, middle";
    }
    else if (DetectorNumber == 5) {
      DetectorName = "Detector5";
      DisplayID = 5;
      DisplayName = DetectorName + ": +x, -y, top";
    }
    else if (DetectorNumber == 6) {
      DetectorName = "Detector6";
      DisplayID = 8;
      DisplayName = DetectorName + ": +x, +y, top";
    }
   else if (DetectorNumber == 7) {
      DetectorName = "Detector7";
      DisplayID = 7;
      DisplayName = DetectorName + ": +x, +y, middle";
    }
    else if (DetectorNumber == 8) {
      DetectorName = "Detector8";
      DisplayID = 6;
      DisplayName = DetectorName + ": +x, +y, bottom";
    }
    else if (DetectorNumber == 9) {
      DetectorName = "Detector9";
      DisplayID = 9;
      DisplayName = DetectorName + ": -x, +y, bottom";
    }
    else if (DetectorNumber == 10) {
      DetectorName = "Detector10";
      DisplayID = 10;
      DisplayName = DetectorName + ": -x, +y, middle";
    }
    else if (DetectorNumber == 11) {
      DetectorName = "Detector11";
      DisplayID = 11;
      DisplayName = DetectorName + ": -x, +y, top";
    }


	//if (DetectorNumber == 1 || DetectorNumber == 2 || DetectorNumber == 3) { //Used for debugging times...
    
    //Before moving on and trying to calibrate the hits, first we need to double check that the events are valid. This includes looking for false timing (LLD events get though to this point), checking to make sure that each hit has x and y strips and making sure the strip numbers are valid.


	//We can't calibrate LLD only events, so check first for those. But for multistrip events, one strip may have timing and one may not, but that event is still okay. Also, Alex and Alan have observed hits with weird timing values, like > 40, but the make is 36 (?), so I should check to for here too.
	double time_x = -1;
	double time_y = -1;
	int strip_x;
	int strip_y;

    for (unsigned int i_s_hit = 0; i_s_hit < NStripHits; i_s_hit++) {
	  //cout << "i_s_hit: " << i_s_hit << endl; 
	  if (H->GetStripHit(i_s_hit)->IsXStrip() == true && (time_x == -1 || time_x == 0 || time_x > 36)) {
		time_x = H->GetStripHit(i_s_hit)->GetTiming();
		//cout << "time_x: "<<time_x << endl;
	  	strip_x = H->GetStripHit(i_s_hit)->GetStripID();
	  }
	  if (H->GetStripHit(i_s_hit)->IsXStrip() == false && (time_y == -1 || time_y == 0 || time_y > 36)) {
		time_y = H->GetStripHit(i_s_hit)->GetTiming();
		//cout << "time_y: " << time_y << endl;
		strip_y = H->GetStripHit(i_s_hit)->GetStripID();
	  }	
	}
	if (time_x == -1 || time_y == -1) { //Then there isn't a valid hit on both sides of the detector 
	  Flag_CanBeCalibrated = 0;
	  //cout << "\n \n \n \n Only one active side: " << Event->GetID() << " \n \n \n \n " << endl;	
	} else if (time_x == 0 || time_x > 36 || time_y == 0 || time_y > 36) { //If the hit does not have a valid x and y timing, then we can't calibrate the depth and we count it as a LLD only event.
      LLDEvents++;
      NoTiming_Event = 1; //Should make a new and unique flag for LLD only event
      //cout << "Found LLD only event (" << Event->GetID() << ") in Det " << DetectorNumber << endl;
	  Event->SetLLDEvent(true);
	} else if (strip_x < 1 || strip_x > 37 || strip_y < 1 || strip_y > 37) {
	  //cout << " \n \n \n \n \n Found bad strip number: " << Event->GetID() << " \n \n \n \n " <<endl;
	  EventStripNotValid++;
	  Flag_CanBeCalibrated = 0;
	} 

    //Cout for debugging purposes, only printing out the info for the events that don't make it past the above cuts.
    /*
    if (Flag_CanBeCalibrated == 0) {
	  cout << "Bad Timing Event Details: " << endl;
	  cout << "Detector Number " << DetectorNumber << ", Event Number " << Event->GetID() << endl;
	  cout << "Number of Strip Hits " << NStripHits << endl;
	  for (int i = 0; i < NStripHits; i++ ) { 
	    if (H->GetStripHit(i)->IsXStrip() == true ) {
		  cout << "XStrip Number and Timing: " << H->GetStripHit(i)->GetStripID() << " " << H->GetStripHit(i)->GetTiming() << endl;
	    } else {
	   	  cout << "YStrip Number and Timing: " << H->GetStripHit(i)->GetStripID() << " " << H->GetStripHit(i)->GetTiming() << endl;
	    }
	  }
	}
    */

	if (Flag_CanBeCalibrated == 0) FlagUncali = 1;
	
    else if (Flag_CanBeCalibrated != 0) {

	  //-----------------------------------------------------------------------
	  //Single Pixel Events
	
      if ( (NStripHits == 2) && (NXStripHits == 1) && (NYStripHits == 1) ) {
        Flag_CanBeCalibrated = 1;
        SHX = H->GetStripHit(0);
        SHY = H->GetStripHit(1);
        if ( !SHX->IsXStrip() ) {
          SHX = H->GetStripHit(1);
          SHY = H->GetStripHit(0);
        }
        XStripNumber = SHX->GetStripID();
        YStripNumber = SHY->GetStripID();
        XTiming = SHX->GetTiming();
        YTiming = SHY->GetTiming();
        SingleHitNumber++;
        //cout<<"EventID, "<<Event->GetID()<<"Single Hit, "<<SingleHitNumber<<endl;
        EventTypeFlag0=1;


      //------------------------------------------------------------------------
	  //Hits with 3 or 4 active strips

      } else if (( (NStripHits == 3) && (((NXStripHits == 1)&&(NYStripHits == 2)) || ((NXStripHits == 2)&&(NYStripHits == 1))) ) || ((NStripHits == 4) && (NXStripHits == 2) && (NYStripHits == 2))) { 
      
        int i_sxhit=0;
        int i_syhit=0;
        int Tmp_XStrip[NXStripHits];
        int Tmp_YStrip[NYStripHits];
        double Tmp_XEnergy=-100.0;
        double Tmp_YEnergy=-100.0;
		//Figure out which strip to use in which hit to determine the depth...
        //Take the timing on the larger energy strip, but make sure that event actually has a valid time...
        for (unsigned int i_s_hit=0; i_s_hit < NStripHits; i_s_hit++){
          if (H->GetStripHit(i_s_hit)->IsXStrip() == true){
            Tmp_XStrip[i_sxhit] = H->GetStripHit(i_s_hit)->GetStripID();
            if (g_Verbosity >= c_Info) cout<<"XStrip:"<<Tmp_XStrip[i_sxhit]<<", ENERGY:"<<H->GetStripHit(i_s_hit)->GetEnergy()<<", Tmp_E:"<<Tmp_XEnergy<<endl;
            if ( Tmp_XEnergy < H->GetStripHit(i_s_hit)->GetEnergy() && (H->GetStripHit(i_s_hit)->GetTiming() > 0 && H->GetStripHit(i_s_hit)->GetTiming() < 37) ) {
	          Tmp_XEnergy = H->GetStripHit(i_s_hit)->GetEnergy();

		      //if (H->GetStripHit(i_s_hit)->GetTiming() != 0) {
                XTiming = H->GetStripHit(i_s_hit)->GetTiming();
                XStripNumber=Tmp_XStrip[i_sxhit];
	          //}
            }
            i_sxhit++;
          } else {
            Tmp_YStrip[i_syhit] = H->GetStripHit(i_s_hit)->GetStripID();
            //cout<<"YStrip:"<<Tmp_YStrip[i_syhit]<<",eNERGY:"<<H->GetStripHit(i_s_hit)->GetEnergy()
            //    <<",Tmp_E:"<<Tmp_YEnergy<<endl;
            if ( Tmp_YEnergy < H->GetStripHit(i_s_hit)->GetEnergy() && (H->GetStripHit(i_s_hit)->GetTiming() > 0 && H->GetStripHit(i_s_hit)->GetTiming() < 37) ) {
              Tmp_YEnergy = H->GetStripHit(i_s_hit)->GetEnergy();

		      //if (H->GetStripHit(i_s_hit)->GetTiming() != 0) {
                YTiming = H->GetStripHit(i_s_hit)->GetTiming();
                YStripNumber=Tmp_YStrip[i_syhit];
		      //}
            } 
            i_syhit++;
          }
        }


		//-----------------------------
		//Two x strips and 1 y strip
        if (NXStripHits == 2 && NYStripHits== 1){
          if (((Tmp_XStrip[0]-Tmp_XStrip[1])==1) || ((Tmp_XStrip[0]-Tmp_XStrip[1])==-1)){
            Flag_CanBeCalibrated = 2;
            ShareHitNumber0++;
            //cout<<"EventID,"<<Event->GetID()<<"Share Hit, OK,"<<ShareHitNumber0<<endl;
            EventTypeFlag1=1;
          }
	
		//-----------------------------
		//Two y strips and 1 x strip
        } else if (NYStripHits == 2 && NXStripHits == 1){
          if (((Tmp_YStrip[0]-Tmp_YStrip[1])==1) || ((Tmp_YStrip[0]-Tmp_YStrip[1])==-1)){
            Flag_CanBeCalibrated = 2;
            ShareHitNumber0++;
            //cout<<"EventID,"<<Event->GetID()<<"Share Hit, OK,"<<ShareHitNumber0<<endl;
            EventTypeFlag1=1;
          }

		//----------------------------
		//Two x and two y strips
        } else if (NYStripHits == 2 && NXStripHits == 2){
          if ( (((Tmp_YStrip[0]-Tmp_YStrip[1])==1) || ((Tmp_YStrip[0]-Tmp_YStrip[1])==-1)) &&
            (((Tmp_XStrip[0]-Tmp_XStrip[1])==1) || ((Tmp_XStrip[0]-Tmp_XStrip[1])==-1))){
            ShareHitNumber0++;
            //cout<<"EventID,"<<Event->GetID()<<"Share Hit, OK,"<<ShareHitNumber0<<endl;
            EventTypeFlag1=1;
            Flag_CanBeCalibrated = 2;
          }

		//If the hit has three or four strips but does not fit into one of the three categories above, then we ignore it. I should also include the possibility of 3 strips on one side  and 1 strip on the other.
		//Or, if the strips are not adjacent...
        } else {
          Flag_CanBeCalibrated = 0;
		  ShareHitNumber1++;
          //cout<<"EventID,"<<Event->GetID()<<"Share Hit, Bad,"<<ShareHitNumber1<<endl;
        }
       

	 //If the hit has more that 4 strip hits, then we don't calibrate it. We can, but there are very few and it would be complicated. This can be done later.
      } else {
        Flag_CanBeCalibrated = 0;
        OtherHitNumber++;
        //cout<<"EventID,"<<Event->GetID()<<"Other Hit,"<<OtherHitNumber<<endl;
      }
	}

//    } else {
//	  Event->SetTimeIncomplete(true);
//	}


      if (Flag_CanBeCalibrated == 1 || Flag_CanBeCalibrated == 2 || NoTiming_Event == 1) {
        //cout<<"Flag: "<<Flag_CanBeCalibrated<<", ID: "<<Event->GetID()<<", XStrip/Timing: "<<
        //        XStripNumber<<"/"<<XTiming<<", YStrip/Timing: "<<YStripNumber<<"/"<<YTiming<<endl;
        
        
        // Calibration parameters
        int n_parameters;
        
        double ctd2z[4];
        //cout<<"DetID "<<DetectorNumber<<", X: "<<XStripNumber<<", Y: "<<YStripNumber<<endl;
        double FWHM_positive = m_Pixel_CTD_FWHM_Positive[DetectorNumber][XStripNumber-1][YStripNumber-1];
        double FWHM_negative = m_Pixel_CTD_FWHM_Negative[DetectorNumber][XStripNumber-1][YStripNumber-1];
        for (n_parameters=0;n_parameters<4;n_parameters++) {
          ctd2z[n_parameters]= m_Pixel_CTD2Depth[DetectorNumber][XStripNumber-1][YStripNumber-1][n_parameters];
        }
        // Calculate Z:	
        double CTD = (XTiming-YTiming)*10;
        double Z_Front,Z_Middle,CTD_tmp;
		double Z_FWHM = 0.0;
		//Z_Front is defined as the distance from the +z side of the GeD in geomega, where the +z side is determined by the roation of the GeD in the GeD_!2Stack.geo geometry file. We confert the timing into depth measurements (where depth is definied by the distance from the negative/DC side), but then to get the position of the interaction within the detector/ the gobal mass model, we convert the depth to a measurement of Z_Front. Because of the way the GeDs are rotated in geomega, Z_Front is always (1.5 - depth).

        double depth=0.0;
        double Z_tmp[2];
        for (n_parameters=0;n_parameters<4;n_parameters++) {
          depth=depth+ctd2z[n_parameters]*pow(CTD,n_parameters);
        }
        
        //COSI 14 - Det 1-3 (CC 01, 02, 03) have DC on top, 
        //Det 4-6 (CC 00, 06, 04) have DC on bottom, 
        //Det 7-9 (CC 05, 07, 10) have DC on top, 
        //Det 10-12 (CC 11, 08, 09) have DC on bottom 
        //But the rotation of the GeDs in GeD_12Stack.geo take this into account, so Z_Front is always equal to the depth
        //Z_Front = depth;
		Z_Front = 1.5 - depth; //This is true for MassModel_1.1        

        // Depth should be between 0 (front) and 1.5 (back) - but maybe we should only do this if the calculated Z_Front is within a reasonable distance from the edge of the wafer. For badly calibrated strips or weird events we might get a depth that's way outside of the physical range of the GeD wafer, this hit should be ignored, not just placed into the 0 or 1.5 cm bin. At least it seems that way to me...
        if (Z_Front < -0.05 && NoTiming_Event != 1) {//{ Z_Front=0.;  - don't knock out the LLD events
		  //cout << "Event #: " << Event->GetID() <<," Number of strip hits: "<< NStripHits << endl;
		  //cout << "Out of bounds hit: "<< Z_Front << " with timing: " << XTiming << " " << YTiming << ", CTD = " << CTD << endl;	
		  //cout << "Detector: " << DetectorName << ", Strips: " << XStripNumber << " " << YStripNumber << endl;
		  //cout << "D: " << DetectorName << ", CTD: " << CTD << ", Z_Front: " <<  Z_Front << endl;
		  if (NStripHits == 2) {
			InvalidEventNumber++;
		   }
//cout << "Number of out of bounds hits: " << OutofBoundsDepth << " Number of out of bounds pixel hits: "<< InvalidEventNumber <<  endl;  
		  Event->SetDepthCalibration_OutofRange(true);
		  OutofRange = 1;
		} 
		if (Z_Front < 0 && Z_Front > -0.05) {
		  Z_Front = 0.;
		}
        if (Z_Front > 1.55 && NoTiming_Event != 1) { //Z_Front=1.5; }
		  //cout << " Event #: " << Event->GetID() << " Number of strip hits: " <<  NStripHits << endl;
		  //cout << " Out of bounds hit: " << Z_Front << " with timing: " << XTiming << " " << YTiming << ", CTD = " << CTD << endl;
		  //cout << " Detector: " << DetectorName << ", Strips: " << XStripNumber << " " << YStripNumber << endl;
		  //cout << "D: " << DetectorName << ", CTD: " << CTD << ", Z_Front: " <<  Z_Front << endl;
		  if (NStripHits == 2) {
			InvalidEventNumber++;
		  }
//cout << "Number of out of bounds hits: " << OutofBoundsDepth << " Number of out of bounds pixel hits: "<< InvalidEventNumber <<  endl;  
		  Event->SetDepthCalibration_OutofRange(true);
		  OutofRange = 1;
		}
		if (Z_Front > 1.5 && Z_Front < 1.55) {
		  Z_Front = 1.5;
		}
        Z_Middle = 0.75-Z_Front; //Z_Middle is used because the position of the interaction is determined frist relative to the center of the detector in geomega (in the individual detector coordinates, which are upside down for all of the GeDs with the DC side up), then the detector position is used to determine the global coordinates of the interaction.


        //output for debug:
        //cout<<"Det,Xstrip,Ystrip:"<<DetectorNumber<<", "<<XStripNumber<<", "<<YStripNumber<<", CTD:"<<CTD
           //<<", Depth:"<<depth<<", Z_Front:"<<Z_Front<<endl;
        //output done
        
        //Calculate Z resolution:
        for (int i_ct=0;i_ct<2;i_ct++){
          depth=0.0;
          CTD_tmp=CTD + pow(-1.,i_ct)*0.25*(FWHM_negative+FWHM_positive);
          for (n_parameters=0;n_parameters<4;n_parameters++) {
            depth=depth+ctd2z[n_parameters]*pow(CTD_tmp,n_parameters);
          }
          if (depth < 0. ) {depth=0. ;}
          if (depth > 1.5) {depth=1.5;}
          Z_tmp[i_ct]=depth;
          //output for debug:
          //cout<<"CTD,CTD+-noise,depth+-noise:"<<CTD<<", "<<CTD_tmp<<" ,"<<Z_tmp[i_ct]<<endl;
          //output done
        }
        
        if (Z_tmp[0] >= Z_tmp[1]) {
          Z_FWHM=Z_tmp[0]-Z_tmp[1];
        } else {
          Z_FWHM=Z_tmp[1]-Z_tmp[0];
        }
        //output for debug
        //cout<<"calculated FWHM:"<<Z_FWHM<<endl;
        //output done


        // Calculate X and Y positions based on strip number.  These are referenced from the middle of the GeD reference frame. In the GeD_DetectorBuild.geo, which defines the GeD geometry, The +x direction corresponds to the y-strips (ystrips are always on the LV side) and the +y direction corresponds to the x-strips.
        double X_Middle = ((double)YStripNumber - 19.0)*(-0.2); //edited for MassModel_1.1
        double Y_Middle = ((double)XStripNumber - 19.0)*(-0.2); //edited for MassModel_1.1
        
		//cout<<"X_Middle: "<<X_Middle<<", Y_Middle: "<<Y_Middle<<", Z_Middle: "<<Z_Middle<<endl;
		
        // Set depth and depth resolution for the hit, relative to the center of the detector volume

		//if (NoTiming_Event == 1) {  //There was talk about making the LLD only hits have a position in the center of the detector with a large depth resolution. But for now, we're completely igoring all events with an LLD only hit. We'll come back to this...
		//  Z_Middle = 0;
		//  Z_FWHM = 1.5/sqrt(12.0)*2.35;
		//}
        MVector PositionInDetector(X_Middle, Y_Middle, Z_Middle);
        //MVector PositionResolution(2.0/2.35, 2.0/2.35, Z_FWHM/2.35)

        //MVector PositionResolution(2.0/2.35, Z_FWHM/2.35, 2.0/2.35); //What? Why are the z and y resolutions wapped? Does the 2.35 come from FWHM -> sigma?
		MVector PositionResolution(0.2/sqrt(12.0), 0.2/sqrt(12.0), Z_FWHM/2.35);
        MVector PositionInGlobal = m_Geometry->GetGlobalPosition(PositionInDetector, DetectorName);
        //cout << "Pos in det:    " << PositionInDetector << endl;
        //if (g_Verbosity >= c_Info) 
		//cout << "Event: " << Event->GetID() << ", Pos in global (Det="<<DetectorName<<"): " << PositionInGlobal << endl;
		//cout << "Resolution: " << PositionResolution << endl;
        H->SetPosition(PositionInGlobal);
        H->SetPositionResolution(PositionResolution);
        if (g_Verbosity >= c_Info) {
          cout << "Hit: D" << DetectorNumber << " X:" << XStripNumber << " (" << X_Middle << " cm)  "
               << " Y:" << YStripNumber << " (" << Y_Middle << " cm)  "
               << " X Timing: " << XTiming << " Y Timing: " << YTiming 
               << " Z: " << Z_Front << " cm  Z res.: " << Z_FWHM << " cm \n" << endl;
        }


	    //Add the hit depth to the histograms in Nuclearizer
		if ((DetectorNumber == 3) || (DetectorNumber == 4) || (DetectorNumber == 5) ||   (DetectorNumber == 9) || (DetectorNumber == 10) || (DetectorNumber == 11)) {
          //m_ExpoDepthCalibration->AddDepth(DisplayID, Z_Front); //Changed for MassModel_1.1
		  m_ExpoDepthCalibration->AddDepth(DisplayID, 1.5 - Z_Front);
        } else {
          //m_ExpoDepthCalibration->AddDepth(DisplayID, 1.5-Z_Front); //Changed for MassModel_1.1
		  m_ExpoDepthCalibration->AddDepth(DisplayID, Z_Front);
        }
        m_ExpoDepthCalibration->SetDepthHistogramName(DisplayID, DisplayName);       



      } else {  //closes "if (Flag_CanBeCalibrated == 1 || Flag_CanBeCalibrated == 2) {"

		FlagUncali = 1;

		//Should flag as BD event? Does setting this position to -9999 really matter if we have BD flagged? - Oh, one reason not to flag it as BD is cause then we can still use it as a photoevent with the appropriate senergy
		//cout<<"Bad depth calibration - Det "<<DetectorNumber<<endl;
		MVector PositionInGlobal(0,0,0);
        MVector PositionResolution(0.0,0.0,0.0);
        H->SetPosition(PositionInGlobal);
        H->SetPositionResolution(PositionResolution);
        if (g_Verbosity >= c_Error) cout << "Depth calibration cannot calculate depth for hit.  "
          << "Doesn't contain exactly one X and one Y strip hit." << endl;
        Event->SetDepthCalibrationIncomplete(true);
      }
  }
  //cout<<"DepthCal for this event is done..."<<endl;


  //Now let's gather some statistics about the events. Remember, at this point, the event includes one or more hits within a detector in the same instant in time. And each hit can include multiple strip hits, which have already passed through the strip pairing algorithim. Throughout the depth calibration above, we've flag particular events if they've had single hits, charge sharing hits, and LLD only hits. 
  if (EventTypeFlag0==1 && EventTypeFlag1==0) { //the number of events that have only two-strip hits
    SingleEventNumber++;
    //cout<<"All events including only pixel hits, "<<SingleEventNumber<<endl;
  } else if(EventTypeFlag0==1 && EventTypeFlag1==1){ //the number of events that have both two-strip hits and three or four strip hits (charge sharing).
    ShareEventNumber0++;
    //cout<<"All events including both pixel and charge sharing hits, "<<ShareEventNumber0<<endl;
  } 
  if (EventTypeFlag1==1 && EventTypeFlag0 == 0){ //the number of events that have charge sharing hits and no hits with only two-strips
    ShareEventNumber1++;
    //cout<<"All events including only charge sharing hits, "<<ShareEventNumber1<<endl;
  }
  if (EventTypeFlag2==1){  //the number of events which include hits with non-adjacent strip hits (shouldn't happen unless the strip pairing module screws up) or 4 strip events with three strips on one side and only one on the other
	OtherEventNumber++;
	//cout<<"EventTypeFlag2: "<<OtherEventNumber<<endl;
    //cout<<"All events with 3-4 strip hits that are not adjacent or not 1&2 or 2&2,"<<OtherEventNumber<<endl;
  }
  if (EventTypeFlag3 ==1){  //the number of events which include hits with more that four strip hits
    OtherHitNumber++;
    //cout<<"EventTypeFlag3: "<<OtherHitNumber<<"/"<<Event->GetID()<<endl;
	//cout<<"All events with with more that 4 strip hits,"<<OtherHitNumber<<endl;
  }	
  if (NoTiming_Event == 1){ //the number of events which contain an LLD only hit. For now, these are given a BD flag and won't continue through the analysis. But we can take care and sort through these to find which events also have other good hits.
	LLDNumber++;
	//cout<<"All events which contain an LLD only hit: "<<LLDNumber<<"/"<<Event->GetID()<<endl;
  }
  if (FlagUncali == 1  || NotValidStripNumber == 1 || NoTiming_Event == 1) {
//	InvalidEventNumber++;
  }
  if (OutofRange == 1) {
	OutofBoundsDepth++;
  }  


  //Output for debugging:
  /*
  cout << "Event Types: SinglePixel - " << SingleEventNumber << ", MultistripHit - " << ShareEventNumber0 << " & "<< ShareEventNumber1 << ", LLD Events - " << LLDNumber << ", Uncalibratable due to number of hits or non-adjacent hits - " << OtherEventNumber+OtherHitNumber << endl;
  cout << "Number of uncalibrated events: " << InvalidEventNumber << ", Number of calibratable events (doesn't discount LLD hits in a valid event): " << SingleEventNumber + ShareEventNumber1 + ShareEventNumber0 << " Total number of events: " << Event->GetID() << endl;
  cout << "Number of out of bounds hits: " << OutofBoundsDepth << " Number of out of bounds pixel hits: "<< InvalidEventNumber <<  endl;  
  cout << " \n " << endl;
  */  

  Event->SetAnalysisProgress(MAssembly::c_DepthCorrection | MAssembly::c_PositionDetermiation);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleDepthCalibration3rdPolyPixel::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleDepthCalibration3rdPolyPixel.cxx: the end...
///////////////////////////////////////////////////////////////////////////////
