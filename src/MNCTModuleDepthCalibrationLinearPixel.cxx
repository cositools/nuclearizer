/*
 * MNCTModuleDepthCalibrationLinearPixel.cxx
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
// MNCTModuleDepthCalibrationLinearPixel
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDepthCalibrationLinearPixel.h"
#include "MGUIOptionsDepthCalibrationLinearPixel.h"

// // Standard libs:
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>

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
ClassImp(MNCTModuleDepthCalibrationLinearPixel)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibrationLinearPixel::MNCTModuleDepthCalibrationLinearPixel() : MModule()
{
  // Construct an instance of MNCTModuleDepthCalibrationLinearPixel
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Linear Depth Calibration using pixel edge data";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DepthCalibrationLinearPixel";
  
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
  m_HasOptionsGUI = true;
  
  // Set the histogram display
  m_ExpoDepthCalibration = new MGUIExpoDepthCalibration(this);
  m_ExpoDepthCalibration->SetDepthHistogramArrangement(3, 4);
  //m_ExpoDepthCalibration->SetDepthHistogramParameters(75, -0.000001, 1.500001); //-0.7499, 0.7501);
  m_ExpoDepthCalibration->SetDepthHistogramParameters(100, -0.5, 2.);
  m_Expos.push_back(m_ExpoDepthCalibration);  
  
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDepthCalibrationLinearPixel::~MNCTModuleDepthCalibrationLinearPixel()
{
  // Delete this instance of MNCTModuleDepthCalibration3rdPolyPixel
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibrationLinearPixel::Initialize()
{
// Initialize the Module


 // Definition:
  // Depth is the distance between interaction and cathode (negative side).
  // CTD is "Timing(+) - Timing(-)".
  // The relationship between CTD and depth is linear. I used Martin's pixel edges the assumed these were at 0cm and 1.5cm with a linear relationship in between.
 

  DetectorMapping DMap;

//Read in the DetectorMap
  MParser Parser;
  if (Parser.Open(m_FileName, MFile::c_Read) == false) {
    cout<<"\n \n \n Didn't get the Map.txt file open \n \n \n"<<endl;
	return false;
  }

  for (unsigned int i = 0; i < Parser.GetNLines(); ++i) {
	unsigned int NTokens = Parser.GetTokenizerAt(i)->GetNTokens();
	if (NTokens != 8) continue; //the tabs in the .txt files count as a Token
    for (unsigned int detnum = 0; detnum < 12; ++detnum) {  
		//cout<<Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(0)<<endl;
		DMap.CCNumber = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(0);
  	    DMap.DetectorNumber = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(1);
		DMap.DetectorName = Parser.GetTokenizerAt(i)->GetTokenAtAsString(2);
		DMap.DisplayID = Parser.GetTokenizerAt(i)->GetTokenAtAsUnsignedInt(3);
		DMap.DisplayName = Parser.GetTokenizerAt(i)->GetTokenAtAsString(4)+' '+Parser.GetTokenizerAt(i)->GetTokenAtAsString(5)+' '+Parser.GetTokenizerAt(i)->GetTokenAtAsString(6)+' '+Parser.GetTokenizerAt(i)->GetTokenAtAsString(7);
		DetMap[DMap.CCNumber] = DMap;
	}
  }


 
  // The default parameters
  m_Default_CTD_Front = -150.;
  m_Default_CTD_Back =  100.;
  m_Default_CTD_FWHM = 2.35*15.;	
 

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
  
  for (int DetectorNumber=0; DetectorNumber<12; DetectorNumber++) {   
    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Attempting to load depth calibration (by pixel) for D" << DetectorNumber << endl;
    
    // Construct the filename of the detector-specific calibration file
    string DetectorNumberString;
    stringstream temp;
    if (DetectorNumber < 10) {
      temp << setw(1) << DetectorNumber;
      DetectorNumberString = temp.str();
    } else {
      temp << setw(2) << DetectorNumber;
      DetectorNumberString = temp.str();
    }

// Use the DetectorMap chosen through the Nuclearizer GUI to chose which calibraiton files to use...ie, Palestine or Antarctica
    string FileName = m_FileName.substr(0,m_FileName.length()-15)+"depth_linear_Det"+DetectorNumberString+".out";
   
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
          double depth, CTD_Negative, CTD_Positive, Sigma_Negative, Sigma_Positive, Counts;
		  if (Line.BeginsWith("1.5") == true) {
          	if (sscanf(Line.Data(), "%lf %d %d %lf %lf %lf \n",
			  &depth,&PositiveStripNumber,&NegativeStripNumber,&CTD_Negative,&Sigma_Negative,&Counts) == 6) {
			  if (Counts > 100) {
          	    m_Pixel_CTD_Front[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = CTD_Negative;
          	    m_Pixel_CTD_FWHM_Front[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = 2.35*Sigma_Negative;
                m_IsCalibrationLoadedPixel[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = true;
			  }
			}
		  }
		  if (Line.BeginsWith("0.0") == true) {
			if (sscanf(Line.Data(), "%lf %d %d %lf %lf %lf \n",
		      &depth,&PositiveStripNumber,&NegativeStripNumber,&CTD_Positive,&Sigma_Positive,&Counts) == 6) {
			  if (Counts > 100) {
			    m_Pixel_CTD_Back[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = CTD_Positive;
			    m_Pixel_CTD_FWHM_Back[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = 2.35*Sigma_Positive;
			    m_IsCalibrationLoadedPixel[DetectorNumber][PositiveStripNumber-1][NegativeStripNumber-1] = true;
			  }
			}
          }
        }
      }
    }  // done reading from file
    
    // check to see if all strips have been calibrated; load defaults if not.
    m_IsCalibrationLoaded[DetectorNumber] = true;
    //cout<<" Det #"<<DetectorNumber<<" is Calibrated?: "<<m_IsCalibrationLoaded[DetectorNumber]<<endl;
    for (int XStripNumber=1; XStripNumber<=37; XStripNumber++) {
      for (int YStripNumber=1; YStripNumber<=37; YStripNumber++) {
		int x = XStripNumber;
		int y = YStripNumber;
		if ( ( (x == 1 && (y == 1 || y == 2 || y == 3 || y == 35 || y == 36 || y == 37)) || (x == 2 && (y == 1 || y == 2 || y == 36 || y == 37)) || (x == 3 && (y == 1 || y == 37)) || (x == 35 && (y == 1 || y == 37)) || (x == 36 && (y == 1 || y == 2 || y == 36 || y == 37) ) || (x == 37 && ( y == 1 || y == 2 || y == 3 || y == 35 || y == 36 || y == 37)) ) == false ) {
        if (m_IsCalibrationLoadedPixel[DetectorNumber][XStripNumber-1][YStripNumber-1] == false) {
		  //cout << "Det " << DetectorNumber << " pixel (" << XStripNumber << ", " << YStripNumber << ") is not calibrated." <<endl;
          // Set default calibration for the uncalibrated strip
          m_Pixel_CTD_FWHM_Front[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_FWHM;
          m_Pixel_CTD_FWHM_Back[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_FWHM;
          m_Pixel_CTD_Front[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_Front;
		  m_Pixel_CTD_Back[DetectorNumber][XStripNumber-1][YStripNumber-1] = m_Default_CTD_Back;
        }
        }
      } // YStripNumber
    } // XStripNumber
    
    if (m_IsCalibrationLoaded[DetectorNumber] == true) {
      if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Linear depth calibration (by pixel) for D" << DetectorNumber 
      << " successfully loaded!" << endl;
    } else {
      if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": Warning: Unable to fully load Linear depth calibration (by pixel) for D" 
      << DetectorNumber << ".  Defaults were used for some or all strips." << endl;
      // return false;
    }


  } // 'DetectorNumber' loop
  

  // Seed the random number generator for dithering the CTD values in ::AnalyzeEvent
  srand(1);  

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibrationLinearPixel::AnalyzeEvent(MReadOutAssembly* Event) 
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
 
    
    //Before moving on and trying to calibrate the hits, first we need to double check that the events are valid. This includes looking for false timing (LLD events get though to this point), checking to make sure that each hit has x and y strips and making sure the strip numbers are valid.

	//We can't calibrate LLD only events, so check first for those. But for multistrip events, one strip may have timing and one may not, but that event is still okay. Also, Alex and Alan have observed hits with weird timing values, like > 40, but the make is 36 (?), so I should check to for here too.
	double time_x = -1;
	double time_y = -1;
	int strip_x = -1;
	int strip_y = -1;

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

      if (Flag_CanBeCalibrated == 1 || Flag_CanBeCalibrated == 2 || NoTiming_Event == 1) {
	     
    	//Calibration parameters
		double CTD_Front = m_Pixel_CTD_Front[DetectorNumber][XStripNumber-1][YStripNumber-1];
		double CTD_Back = m_Pixel_CTD_Back[DetectorNumber][XStripNumber-1][YStripNumber-1];    
        double FWHM_Front = m_Pixel_CTD_FWHM_Front[DetectorNumber][XStripNumber-1][YStripNumber-1];
        double FWHM_Back = m_Pixel_CTD_FWHM_Back[DetectorNumber][XStripNumber-1][YStripNumber-1];

        // Calculate Z:	
        double CTD = (XTiming-YTiming)*10;
		//Add a random number between +/- 5 to dither the discritized CTD signal
		CTD = CTD + (rand() % 10 -5);

        double Z_Front,Z_Middle;
		double Z_FWHM = 0.0;
		//Z_Front is defined as the distance from the +z side of the GeD in geomega, where the +z side is determined by the roation of the GeD in the GeD_!2Stack.geo geometry file. We confert the timing into depth measurements (where depth is definied by the distance from the negative/DC side), but then to get the position of the interaction within the detector/ the gobal mass model, we convert the depth to a measurement of Z_Front. Because of the way the GeDs are rotated in geomega, Z_Front is always (1.5 - depth).

		Z_Front = 1.4*(CTD-CTD_Front)/(CTD_Back-CTD_Front)+0.05;
		Z_FWHM = 1.4*0.5*(FWHM_Front+FWHM_Back)/(CTD_Back-CTD_Front);
        
        if (Z_Front < -0.05 && NoTiming_Event != 1) {
		  if (NStripHits == 2) {
			InvalidEventNumber++;
		   }
		  Event->SetDepthCalibration_OutofRange(true);
		  OutofRange = 1;
		} 
		if (Z_Front < 0.0 && Z_Front > -0.05) {
		  Z_Front = 0.;
		}
        if (Z_Front > 1.55 && NoTiming_Event != 1) {
		  if (NStripHits == 2) {
		  	InvalidEventNumber++;
		  }
		  Event->SetDepthCalibration_OutofRange(true);
		  OutofRange = 1;
		}
		if (Z_Front > 1.5 && Z_Front < 1.55) {
		  Z_Front = 1.5;
		}

        Z_Middle = 0.75-Z_Front; //Z_Middle is used because the position of the interaction is determined frist relative to the center of the detector in geomega (in the individual detector coordinates, which are upside down for all of the GeDs with the DC side up), then the detector position is used to determine the global coordinates of the interaction.

        // Calculate X and Y positions based on strip number.  These are referenced from the middle of the GeD reference frame. In the GeD_DetectorBuild.geo, which defines the GeD geometry, The +x direction corresponds to the y-strips (ystrips are always on the LV side) and the +y direction corresponds to the x-strips.
        double X_Middle = ((double)YStripNumber - 19.0)*(-0.2); //edited for MassModel_1.1
        double Y_Middle = ((double)XStripNumber - 19.0)*(-0.2); //edited for MassModel_1.1

		
        // Set depth and depth resolution for the hit, relative to the center of the detector volume
        MVector PositionInDetector(X_Middle, Y_Middle, Z_Middle);
		MVector PositionResolution(0.2/sqrt(12.0), 0.2/sqrt(12.0), Z_FWHM/2.35);
        MVector PositionInGlobal = m_Geometry->GetGlobalPosition(PositionInDetector, DetMap[DetectorNumber].DetectorName);
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
	    if (Flag_CanBeCalibrated != 0) {
		if ((DetMap[DetectorNumber].DetectorNumber == 3) || (DetMap[DetectorNumber].DetectorNumber == 4) || (DetMap[DetectorNumber].DetectorNumber == 5) ||   (DetMap[DetectorNumber].DetectorNumber == 9) || (DetMap[DetectorNumber].DetectorNumber == 10) || (DetMap[DetectorNumber].DetectorNumber == 11)) {
          //m_ExpoDepthCalibration->AddDepth(DisplayID, Z_Front); //Changed for MassModel_1.1
		  m_ExpoDepthCalibration->AddDepth(DetMap[DetectorNumber].DisplayID, 1.5 - Z_Front);
        } else {
          //m_ExpoDepthCalibration->AddDepth(DisplayID, 1.5-Z_Front); //Changed for MassModel_1.1
		  m_ExpoDepthCalibration->AddDepth(DetMap[DetectorNumber].DisplayID, Z_Front);
        }
        m_ExpoDepthCalibration->SetDepthHistogramName(DetMap[DetectorNumber].DisplayID, DetMap[DetectorNumber].DisplayName);       
		}


      } else {  //closes "if (Flag_CanBeCalibrated == 1 || Flag_CanBeCalibrated == 2) {"

		FlagUncali = 1;

		//MVector PositionInGlobal(0,0,0);
        //MVector PositionResolution(0.0,0.0,0.0);
        //H->SetPosition(PositionInGlobal);
        //H->SetPositionResolution(PositionResolution);
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


void MNCTModuleDepthCalibrationLinearPixel::ShowOptionsGUI()
{
  // Show the options GUI

  MGUIOptionsDepthCalibrationLinearPixel* Options = new MGUIOptionsDepthCalibrationLinearPixel(this);
  Options->Create();
  gClient->WaitForUnmap(Options);

}


///////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDepthCalibrationLinearPixel::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
	m_FileName = FileNameNode->GetValue();
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleDepthCalibrationLinearPixel::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);

  return Node;

}






// MNCTModuleDepthCalibrationLinearPixel.cxx: the end...
///////////////////////////////////////////////////////////////////////////////




