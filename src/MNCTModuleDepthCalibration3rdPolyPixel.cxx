/*
 * MNCTModuleDepthCalibration3rdPolyPixel.cxx
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
// MNCTModuleDepthCalibration3rdPolyPixel
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDepthCalibration3rdPolyPixel.h"

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
ClassImp(MNCTModuleDepthCalibration3rdPolyPixel)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibration3rdPolyPixel::MNCTModuleDepthCalibration3rdPolyPixel() : MNCTModule()
{
  // Construct an instance of MNCTModuleDepthCalibration3rdPolyPixel
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "3rd order Polynomial Depth Calibration using pixel edge data";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DepthCalibration3rdPolyPixel";
  
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
  AddSucceedingModuleType(c_EventReconstruction);
  AddSucceedingModuleType(c_Aspect);
  AddSucceedingModuleType(c_Else);
  AddSucceedingModuleType(c_EventSaver);
  
  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
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
  // CTD(near -):-200. , CTD(near +):200.
  double default_ctd2z[4]={0.75,-0.00375,0.0,0.0};
  int N_parameter;
  
  ShareHitNumber0=0;
  ShareHitNumber1=0;
  SingleHitNumber=0;
  OtherHitNumber=0;
  ShareEventNumber0=0;
  ShareEventNumber1=0;
  SingleEventNumber=0;
  OtherEventNumber=0;
  
  for (N_parameter=0;N_parameter<4;N_parameter++) {
    m_Default_CTD2Depth[N_parameter]=default_ctd2z[N_parameter];
  }
  
  for (int DetectorNumber=0; DetectorNumber<10; DetectorNumber++) {
    mout << "Attempting to load linear depth calibration (by pixel) for D" << DetectorNumber << endl;
    
    // Construct the filename of the detector-specific calibration file
    string DetectorNumberString;
    stringstream temp;
    temp << setfill('0') << setw(2) << DetectorNumber;
    DetectorNumberString = temp.str();
    MString FileName = (MString)std::getenv ("NUCLEARIZER_CAL")
    +"/depth_3rdPoly_pixel_D"+ DetectorNumberString + ".csv";
    
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
          double ctd2z0, ctd2z1, ctd2z2, ctd2z3, FWHM_positive, FWHM_negative;
          if (sscanf(Line.Data(), "%d,%d,%lf,%lf,%lf,%lf,%lf,%lf\n",
            &PositiveStripNumber,&NegativeStripNumber,&ctd2z0,&ctd2z1,
            &ctd2z2,&ctd2z3,&FWHM_positive,&FWHM_negative) == 8) {
            //mout << "load parameter:[(s+),p0,p3,fwhm(+)]" << " " << PositiveStripNumber << " " 
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
    
    // check to see if all strips have been calibrated; load defaults if not
    m_IsCalibrationLoaded[DetectorNumber] = true;
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
      mout << "3rd order polynomial depth calibration (by pixel) for D" << DetectorNumber 
      << " successfully loaded!" << endl;
    } else {
      mout << "***Warning: Unable to fully load 3rd polynomial depth calibration (by pixel) for D" 
      << DetectorNumber << ".  Defaults were used for some or all strips." << endl;
    }
    
  } // 'DetectorNumber' loop
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibration3rdPolyPixel::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level 
  unsigned int EventTypeFlag0=0;
  unsigned int EventTypeFlag1=0;
  unsigned int EventTypeFlag2=0;
  unsigned int EventTypeFlag3=0;
  
  unsigned int NHits = Event->GetNHits();
  bool DepthCalibrated = false;
  //mout << "MNCTDepthCalibrationLinearPixel::AnalyzeEvent: Event ID = " << Event->GetID() << endl;
  
  //mout << endl << "Event ID: " << Event->GetID() << endl;
  for (unsigned int i_hit=0; i_hit<NHits; i_hit++) {
    MNCTHit *H = Event->GetHit(i_hit);
    unsigned int NStripHits = H->GetNStripHits();
    unsigned int NXStripHits = 0, NYStripHits = 0;
    int DetectorNumber;
    for (unsigned int i_sh=0; i_sh<NStripHits; i_sh++) {
      MNCTStripHit *SH = H->GetStripHit(i_sh);
      DetectorNumber = SH->GetDetectorID();
      if (SH->IsXStrip() == true) { NXStripHits++; } else { NYStripHits++; }
    }
    //Check for 1 pixel event and sharing event..
    int Flag_CanBeCalibrated;
    double XTiming,YTiming;
    int XStripNumber,YStripNumber;
    MNCTStripHit *SHX, *SHY;
    //int DetectorNumber = SHX->GetDetectorID();
    stringstream temp;
    temp << 'D' << setfill('0') << setw(2) << DetectorNumber;
    MString DetectorName = temp.str();
    
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
      //mout<<"EventID,"<<Event->GetID()<<"Single Hit,"<<SingleHitNumber<<endl;
      EventTypeFlag0=1;
    } else if (( (NStripHits == 3) && (((NXStripHits == 1)&&(NYStripHits == 2)) ||
      ((NXStripHits == 2)&&(NYStripHits == 1))) ) ||
      ((NStripHits == 4) && (NXStripHits == 2) && (NYStripHits == 2))) { 
      
      int i_sxhit=0;
    int i_syhit=0;
    int Tmp_XStrip[NXStripHits];
    int Tmp_YStrip[NYStripHits];
    double Tmp_XEnergy=-100.0;
    double Tmp_YEnergy=-100.0;
    //Take the timing on the larger energy strip
    for (unsigned int i_s_hit=0; i_s_hit < NStripHits; i_s_hit++){
      if (H->GetStripHit(i_s_hit)->IsXStrip() == true){
        Tmp_XStrip[i_sxhit] = H->GetStripHit(i_s_hit)->GetStripID();
        //mout<<"XStrip:"<<Tmp_XStrip[i_sxhit]<<",eNERGY:"<<H->GetStripHit(i_s_hit)->GetEnergy()
        //    <<",Tmp_E:"<<Tmp_XEnergy<<endl;
        if ( Tmp_XEnergy < H->GetStripHit(i_s_hit)->GetEnergy() ){
          Tmp_XEnergy = H->GetStripHit(i_s_hit)->GetEnergy();
          XTiming = H->GetStripHit(i_s_hit)->GetTiming();
          XStripNumber=Tmp_XStrip[i_sxhit];
        }
        i_sxhit++;
      } else{
        Tmp_YStrip[i_syhit] = H->GetStripHit(i_s_hit)->GetStripID();
        //mout<<"YStrip:"<<Tmp_YStrip[i_syhit]<<",eNERGY:"<<H->GetStripHit(i_s_hit)->GetEnergy()
        //    <<",Tmp_E:"<<Tmp_YEnergy<<endl;
        if ( Tmp_YEnergy < H->GetStripHit(i_s_hit)->GetEnergy() ){
          Tmp_YEnergy = H->GetStripHit(i_s_hit)->GetEnergy();
          YTiming = H->GetStripHit(i_s_hit)->GetTiming();
          YStripNumber=Tmp_YStrip[i_syhit];
        }
        i_syhit++;
      }
    }
    if (NXStripHits == 2 && NYStripHits== 1){
      if (((Tmp_XStrip[0]-Tmp_XStrip[1])==1) || ((Tmp_XStrip[0]-Tmp_XStrip[1])==-1)){
        Flag_CanBeCalibrated = 2;
        ShareHitNumber0++;
        //mout<<"EventID,"<<Event->GetID()<<"Share Hit, OK,"<<ShareHitNumber0<<endl;
        EventTypeFlag1=1;
      }
    } else if (NYStripHits == 2 && NXStripHits == 1){
      if (((Tmp_YStrip[0]-Tmp_YStrip[1])==1) || ((Tmp_YStrip[0]-Tmp_YStrip[1])==-1)){
        Flag_CanBeCalibrated = 2;
        ShareHitNumber0++;
        //mout<<"EventID,"<<Event->GetID()<<"Share Hit, OK,"<<ShareHitNumber0<<endl;
        EventTypeFlag1=1;
      }
    } else if (NYStripHits == 2 && NXStripHits == 2){
      if ( (((Tmp_YStrip[0]-Tmp_YStrip[1])==1) || ((Tmp_YStrip[0]-Tmp_YStrip[1])==-1)) &&
        (((Tmp_XStrip[0]-Tmp_XStrip[1])==1) || ((Tmp_XStrip[0]-Tmp_XStrip[1])==-1))){
        ShareHitNumber0++;
      //mout<<"EventID,"<<Event->GetID()<<"Share Hit, OK,"<<ShareHitNumber0<<endl;
        EventTypeFlag1=1;
        Flag_CanBeCalibrated = 2;
        }
    } else {
      Flag_CanBeCalibrated = 0;
      EventTypeFlag2=1;
      ShareHitNumber1++;
      //mout<<"EventID,"<<Event->GetID()<<"Share Hit, Bad,"<<ShareHitNumber1<<endl;
    }
      } else {
        Flag_CanBeCalibrated = 0;
        EventTypeFlag3=1;
        OtherHitNumber++;
        //mout<<"EventID,"<<Event->GetID()<<"Other Hit,"<<OtherHitNumber<<endl;
      }
      
      // Require 2 strip hits: 1 X strip and 1 Y strip
      //if ( (NStripHits == 2) && (NXStripHits == 1) && (NYStripHits == 1) ) 
      if (Flag_CanBeCalibrated == 1 || Flag_CanBeCalibrated == 2) {
        //mout<<"Flag:"<<Flag_CanBeCalibrated<<", ID:"<<Event->GetID()<<", XStrip/Timing:"<<
        //      XTiming<<"/"<<XStripNumber<<", YStrip/Timing:"<<YTiming<<"/"<<YStripNumber<<endl;
        
        
        // Calibration parameters
        int n_parameters;
        
        double ctd2z[4];
        //mout<<"DetID"<<DetectorNumber<<",X:"<<XStripNumber<<",Y:"<<YStripNumber<<endl;
        double FWHM_positive = m_Pixel_CTD_FWHM_Positive[DetectorNumber][XStripNumber-1][YStripNumber-1];
        double FWHM_negative = m_Pixel_CTD_FWHM_Negative[DetectorNumber][XStripNumber-1][YStripNumber-1];
        for (n_parameters=0;n_parameters<4;n_parameters++) {
          ctd2z[n_parameters]= m_Pixel_CTD2Depth[DetectorNumber][XStripNumber-1][YStripNumber-1][n_parameters];
        }
        // Calculate Z:	
        double CTD = XTiming-YTiming;
        double Z_Front,Z_Middle,Z_FWHM,CTD_tmp;
        double depth=0.0;
        double Z_tmp[2];
        for (n_parameters=0;n_parameters<4;n_parameters++) {
          depth=depth+ctd2z[n_parameters]*pow(CTD,n_parameters);
        }
        //The depth is the distance from the negative side, not always from front.
        //The negative side on D9 and D7 are at back, near dewar.
        if ( (DetectorNumber == 1) || (DetectorNumber == 3) || (DetectorNumber == 5)) {
          Z_Front = 1.5 - depth;
        } else {
          Z_Front = depth;
        }
        
        // Depth should be between 0 (front) and 1.5 (back)
        if (Z_Front < 0.) { Z_Front=0.; }
        if (Z_Front > 1.5) { Z_Front=1.5; }
        Z_Middle = 0.75-Z_Front;
        //output for debug:
        //mout<<"Det,Xstrip,Ystrip:"<<DetectorNumber<<", "<<XStripNumber<<", "<<YStripNumber<<", CTD:"<<CTD
        //    <<", Depth:"<<depth<<", Z_Front:"<<Z_Front<<endl;
        //output done
        
        //Calculate Z resoluation:
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
          //mout<<"CTD,CTD+-noise,depth+-noise:"<<CTD<<", "<<CTD_tmp<<" ,"<<Z_tmp[i_ct]<<endl;
          //output done
        }
        
        if (Z_tmp[0] >= Z_tmp[1]) {
          Z_FWHM=Z_tmp[0]-Z_tmp[1];
        } else {
          Z_FWHM=Z_tmp[1]-Z_tmp[0];
        }
        //output for debug
        //mout<<"calculated FWHM:"<<Z_FWHM<<endl;
        //output done
        // Calculate X and Y positions based on strip number.  These are referenced from the middle.
        double X_Middle=0., Y_Middle=0.;;
        if (DetectorNumber % 2 == 0) {
          // Positive side is in back, positive strips are vertical (i.e., give X position)
          X_Middle = -1.0*((double)XStripNumber - 19.0)*0.2;
          Y_Middle = -1.0*((double)YStripNumber - 19.0)*0.2;
        } else if (DetectorNumber == 9 || DetectorNumber == 7) {
          // Positive side is in front, positive strips are vertical (i.e., give X position)
          X_Middle = -((double)XStripNumber - 19.0)*0.2;
          Y_Middle = -1.0*((double)YStripNumber - 19.0)*0.2;
        } else {
          // Positive side is in back, positive strips are horizontal (so switch X and Y)
          X_Middle = -((double)YStripNumber - 19.0)*0.2;
          Y_Middle = -1.0*((double)XStripNumber - 19.0)*0.2;
        }
        
        // Set depth and depth resolution for the hit, relative to the center of the detector volume
        MVector PositionInDetector(X_Middle, Y_Middle, Z_Middle);
        //MVector PositionResolution(2.0/2.35, 2.0/2.35, Z_FWHM/2.35);
        MVector PositionResolution(2.0/2.35, Z_FWHM/2.35, 2.0/2.35);
        MVector PositionInGlobal = m_Geometry->GetGlobalPosition(PositionInDetector, DetectorName);
        //mout << "Pos in det:    " << PositionInDetector << endl;
        //mout << "Pos in global: " << PositionInGlobal << endl;
        H->SetPosition(PositionInGlobal);
        H->SetPositionResolution(PositionResolution);
        DepthCalibrated=true;
        //mout << "Hit: D" << DetectorNumber << " X:" << XStripNumber << " (" << X_Middle << " cm)  "
        //   << " Y:" << YStripNumber << " (" << Y_Middle << " cm)  "
        //   << " X Timing: " << XTiming << " Y Timing: " << YTiming 
        //   << " Z_X: " << Z_X << " Z_Y: " << Z_Y 
        //   << " Z: " << Z_Front << " cm  Z res.: " << Z_FWHM << " cm" << endl;
      } else {
        MVector PositionInGlobal(-9999.0,-9999.0,-9999.0);
        MVector PositionResolution(0.0,0.0,0.0);
        H->SetPosition(PositionInGlobal);
        H->SetPositionResolution(PositionResolution);
        //mout << "Linear depth calibration cannot calculate depth for hit.  "
        //   << "Doesn't contain exactly one X and one Y strip hit." << endl;
        //DepthCalibrated=false;
        //mout<<"Doesn't contain exactly one X and one Y strip hit. ID: "<<Event->GetID()
        //    <<" , NXStripHits:"<<NXStripHits<<", NYStripHits:"<<NYStripHits<<endl;
      }
  }
  //mout<<"DepthCal for this event is done..."<<endl;
  if (EventTypeFlag0==1 && EventTypeFlag1==0) {
    SingleEventNumber++;
    //mout<<"All pixel (?),"<<SingleEventNumber<<endl;
  } else if(EventTypeFlag0==1 && EventTypeFlag1==1){
    ShareEventNumber0++;
    //mout<<"pixel+share,"<<ShareEventNumber0<<endl;
  } 
  if (EventTypeFlag1==1){
    ShareEventNumber1++;
    //mout<<"with share(with pixel & without pixel),"<<ShareEventNumber1<<endl;
  }
  if (EventTypeFlag2==1){
    OtherEventNumber++;
    //mout<<"with strange,"<<OtherEventNumber<<endl;
  }
  
  Event->SetDepthCalibrated(DepthCalibrated);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleDepthCalibration3rdPolyPixel::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleDepthCalibration3rdPolyPixel.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
