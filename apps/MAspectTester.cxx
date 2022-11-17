
// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>

#include "Python.h"
#include "stdio.h"
#include "cstdio"
#include "cstdlib"
#include "MNCTAspectPacket.h"
#include "MNCTAspectReconstruction.h"
#include "MStreams.h"

#include<iostream>
#include<fstream>
#include <sstream>
#include<string>
#include<cmath>
#include<cstring>
#include<stdio.h>
#include<string.h>
#include <iomanip>
#include <locale>
#include <stdlib.h>
#include <stdint.h>
#include <vector>

using namespace std;


	


//**********************************************************************************************************
//*                 Declaring Structures that contain the parsed dGPS data                                 *
//**********************************************************************************************************

struct DSOStruct{
  uint32_t Seconds;
  double Heading;
  double Pitch;
  double Roll;
  double BRMS; //"baseline RMS error"
  uint8_t Attitude_Flag_1; //The manual only reserves one byte for Attitude Flag, but my parsing has shown that it's indeed two. Not sure which one is valid...
  uint8_t Attitude_Flag_2;
  double Latitude;
  double Longitude;
  double Altitude;
  uint16_t Checksum;
  uint32_t Clock;
};

struct PSAStruct{
  uint32_t Seconds;
  double ECEFX;
  double ECEFY;
  double ECEFZ;
  float ECEFX_vel;
  float ECEFY_vel;
  float ECEFZ_vel;
  float Clock_offset;
  float Freq_off;
  uint16_t PDOP;
  uint8_t NumSats;
  char PosMode;
  float Yaw;
  float Heading;
  float Pitch;
  float Roll;
  char AttState;
  char PosState;
  uint16_t Checksum;
};

struct BITStruct{
  char Batt_Test;
  char Param_Test;
  char Error_Test;
  char Inactive_Sens;
  //char Failed_blocks [];
  //uint16_t Checksum;
};

struct TSTStruct{
  char EPROM_Test;
  char RAM_Test;
  char Chan_Initialization_Test;
};

struct MagStruct{
  float Roll;
  float MagRoll;
  float Inclination;
  float MagTotal;
  float Azimuth;
  float Acceleration;
  float Temperature;
  float Voltage;
  //unit_8 Checksum;
  uint32_t Clock;
};




//*******************************************************************************************************
//*                  Functions to parse each data packet (Carolyn's packet, not an MNCTAspectPacket object) type.*
//*******************************************************************************************************

MNCTAspectReconstruction* AR = new MNCTAspectReconstruction();
MNCTAspectPacket GPS_Packet;
MNCTAspectPacket M_Packet;
MNCTAspectPacket Evil_Packet;

//________________________________________________________________________________________________________
int DecodeDSO(struct DSOStruct * MyDSO, uint8_t * String, size_t Len){

  
  
  
  
  
  
  uint32_t MySeconds;
  MySeconds = 0;
  MySeconds = (((uint32_t)String[11] & 0xFF) << 24) | (((uint32_t)String[12] & 0xFF) << 16) | (((uint32_t)String[13] & 0xFF)  << 8) | ((uint32_t)String[14] & 0xFF);  
  //printf("%02x %02x %02x %02x - ", (String[11] & 0xFF),(String[12] & 0xFF),(String[13] & 0xFF),(String[14] & 0xFF));
  printf("DSO Packet: Milliseconds = %u, ",MySeconds);  //Again these are Carolyn's packets, not MNCTAspectPacket objects
  long intermediate_seconds = MySeconds;
  MySeconds = MyDSO->Seconds;


  uint64_t MyHeading_int = 0;
  double MyHeading = 0.0;
  MyHeading_int = (((uint64_t)String[15] & 0xFF) << 56) | (((uint64_t)String[16] & 0xFF) << 48) | (((uint64_t)String[17] & 0xFF) << 40) |  (((uint64_t)String[18] & 0xFF) << 32) |  (((uint64_t)String[19] & 0xFF) << 24) | (((uint64_t)String[20] & 0xFF) << 20) |  (((uint64_t)String[21] & 0xFF) << 8) | ((uint64_t)String[22] & 0xFF);
  MyHeading = *(double *) &MyHeading_int;
  printf("Heading = %4.2f, ", MyHeading); 
  GPS_Packet.heading =  MyHeading;
  MyHeading = MyDSO->Heading;


  uint64_t MyPitch_int = 0;
  double MyPitch = 0.0;
  MyPitch_int = (((uint64_t)String[23] & 0xFF) << 56) | (((uint64_t)String[24] & 0xFF) << 48) | (((uint64_t)String[25] & 0xFF) << 40) |  (((uint64_t)String[26] & 0xFF) << 32) |  (((uint64_t)String[27] & 0xFF) << 24) | (((uint64_t)String[28] & 0xFF) << 16) |  (((uint64_t)String[29] & 0xFF) << 8) | ((uint64_t)String[30] & 0xFF);
   MyPitch = *(double *) &MyPitch_int;
  printf("Pitch = %4.2f, ", MyPitch);  
  GPS_Packet.pitch =  MyPitch;
  MyPitch = MyDSO->Pitch;


  uint64_t MyRoll_int = 0;
  double MyRoll = 0.0;
  MyRoll_int = (((uint64_t)String[31] & 0xFF) << 56) | (((uint64_t)String[32] & 0xFF) << 48) | (((uint64_t)String[33] & 0xFF) << 40) |  (((uint64_t)String[34] & 0xFF) << 32) |  (((uint64_t)String[35] & 0xFF) << 24) | (((uint64_t)String[36] & 0xFF) << 16) |  (((uint64_t)String[37] & 0xFF) << 8) | ((uint64_t)String[38] & 0xFF);
  MyRoll = *(double *) &MyRoll_int;
  printf("Roll = %4.2f, ", MyRoll);  
  GPS_Packet.roll =  MyRoll;
  MyRoll = MyDSO->Roll;
  

  uint64_t MyBRMS_int = 0;
  double MyBRMS = 0.0;
  MyBRMS_int = (((uint64_t)String[39] & 0xFF) << 56) | (((uint64_t)String[40] & 0xFF) << 48) | (((uint64_t)String[41] & 0xFF) << 40) |  (((uint64_t)String[42] & 0xFF) << 32) |  (((uint64_t)String[43] & 0xFF) << 24) | (((uint64_t)String[44] & 0xFF) << 16) |  (((uint64_t)String[45] & 0xFF) << 8) | ((uint64_t)String[46] & 0xFF);
   MyBRMS = *(double *) &MyBRMS_int;
  printf("BRMS = %f, ", MyBRMS);  
  MyBRMS = MyDSO->BRMS;


  uint8_t MyAtt_flag_1;
  MyAtt_flag_1 = (uint8_t)String[47] & 0xFF;
  printf("Attitude Flag #1 = %u, ", MyAtt_flag_1);
  MyAtt_flag_1 = MyDSO->Attitude_Flag_1;


  uint8_t MyAtt_flag_2;
  MyAtt_flag_2 = (uint8_t)String[47] & 0xFF;
  printf("Attitude Flag #2 = %u, ", MyAtt_flag_2);
  MyAtt_flag_2 = MyDSO->Attitude_Flag_2;


  uint64_t MyLat_int = 0;
  double MyLat;
  MyLat_int = (((uint64_t)String[49] & 0xFF) << 56) | (((uint64_t)String[50] & 0xFF) << 48) | (((uint64_t)String[51] & 0xFF) << 40) |  (((uint64_t)String[52] & 0xFF) << 32) |  (((uint64_t)String[53] & 0xFF) << 24) | (((uint64_t)String[54] & 0xFF) << 16) |  (((uint64_t)String[55] & 0xFF) << 8) | ((uint64_t)String[56] & 0xFF);
  MyLat = *(double *) &MyLat_int;
  printf("Latitude = %4.2f, ", MyLat);  
  GPS_Packet.geographic_latitude =  MyLat;
  MyLat = MyDSO->Latitude;


  uint64_t MyLong_int = 0;
  double MyLong = 0.0;
  MyLong_int = (((uint64_t)String[57] & 0xFF) << 56) | (((uint64_t)String[58] & 0xFF) << 48) | (((uint64_t)String[59] & 0xFF) << 40) | (((uint64_t)String[60] & 0xFF) << 32) | (((uint64_t)String[61] & 0xFF) << 24) | (((uint64_t)String[62] & 0xFF) << 16) | (((uint64_t)String[63] & 0xFF) << 8) | ((uint64_t)String[64] & 0xFF);
  MyLong = *(double *) &MyLong_int;
  printf("Longitude = %4.2f, ", MyLong); 
  GPS_Packet.geographic_longitude =  MyLong;
  MyLong = MyDSO->Longitude;


  uint64_t MyAlt_int = 0;
  double MyAlt = 0.0;
  MyAlt_int = (((uint64_t)String[65] & 0xFF) << 56) | (((uint64_t)String[66] & 0xFF) << 48) | (((uint64_t)String[67] & 0xFF) << 40) |  (((uint64_t)String[68] & 0xFF) << 32) |  (((uint64_t)String[69] & 0xFF) << 24) | (((uint64_t)String[70] & 0xFF) << 16) |  (((uint64_t)String[71] & 0xFF) << 8) | ((uint64_t)String[72] & 0xFF);
  MyAlt = *(double *) &MyAlt_int;
  printf("Altitude = %4.2f, ", MyAlt); 
  GPS_Packet.elevation =  MyAlt; 
  MyAlt = MyDSO->Altitude;


  //NEEDS WORK!
  uint16_t MyChecksum;
  MyChecksum = (((uint16_t)String[73] & 0xFF) << 8) | ((uint16_t)String[74] & 0xFF);
  //printf("Hex Checksum = %c %c %c %c %c- ", (char)(String [72] & 0xFF), (char)(String[73] & 0xFF), (char)(String[74] & 0xFF), (char)(String[75] & 0xFF), (char)(String[76] & 0xFF));
  printf("MyChecksum? = %u, ", MyChecksum);
  MyChecksum = MyDSO->Checksum;

  uint32_t MyClock = 0;  //counting 10 MHz clock signal.
  MyClock = (((uint32_t)String[80] & 0xFF) << 24) | (((uint32_t)String[81] & 0xFF) << 16) | (((uint32_t)String[82] & 0xFF)  << 8) | ((uint32_t)String[83] & 0xFF);  
  printf("Clock =  %u. \n",MyClock);  
  MyClock = MyDSO->Clock;

  printf("\n");  
  GPS_Packet.GPS_or_magnetometer = 0;
  printf("Now, here is what's from GPS_Packet: \n");
  printf("Heading: \n"); 
  printf("%f\n",GPS_Packet.heading); 
  printf("Pitch: \n"); 
  printf("%f\n",GPS_Packet.pitch); 
  printf("Roll: \n"); 
  printf("%f\n",GPS_Packet.roll); 
  printf("Geographic Latitude: \n"); 
  printf("%f\n",GPS_Packet.geographic_latitude); 
  printf("Geographic Longitude: \n"); 
  printf("%f\n",GPS_Packet.geographic_longitude);  
  printf("Elevation: \n"); 
  printf("%f\n",GPS_Packet.elevation); 
  
  
  string slash = "/";
  string colon = ":";
  string twenty = "20";
  string space = " ";
  string zero = "0";
  

  long milliseconds = intermediate_seconds - 16000;  //this will only work for one day (date collected must be same date file with data was made because this program will trust that that date is correct)

    
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  unsigned int nanoseconds = 0;
  

  
  while(milliseconds > 86400000){
  	milliseconds = milliseconds - 86400000;
    }
  while(milliseconds > 3600000){
  	hours = hours +1;
  	milliseconds = milliseconds - 3600000;
    }
  while(milliseconds > 60000){
  	minutes = minutes +1;
  	milliseconds = milliseconds - 60000;
    }
  while(milliseconds > 1000){
  	seconds = seconds +1;
  	milliseconds = milliseconds - 1000;
    } 
    

    
  string Hours = to_string(hours);
  string Minutes = to_string(minutes);
  string Seconds = to_string(seconds);
  

  
  if(hours < 10){
  	Hours = zero + Hours;
    }
  if(minutes < 10){
  	Minutes = zero + Minutes;
    }
  if(seconds < 10){
  	Seconds = zero + Seconds;
    } 
 
   
  string date = Evil_Packet.date_and_time; 
   
  string date_and_time = date + space + Hours + colon + Minutes + colon + Seconds; 
  GPS_Packet.date_and_time = date_and_time;  
    
    
  nanoseconds = milliseconds * 1000000;
  GPS_Packet.nanoseconds = nanoseconds;
  
  
  
  printf("Date_and_Time: \n"); 
  
  cout << GPS_Packet.date_and_time << endl; 
  
  printf("Nanoseconds: \n"); 
  printf("%u\n",GPS_Packet.nanoseconds); 
  
  printf("\n"); 
  
  AR->AddAspectFrame(GPS_Packet);
  cout << "GPS Packet added!!!!!" << endl;
  printf("\n"); 


  return(0);
}
//____________________________________________________________________________________________________



//____________________________________________________________________________________________________
int DecodePSA(struct PSAStruct * MyPSA, uint8_t * String, size_t Len){

  uint32_t MySeconds;
  MySeconds = 0;
  MySeconds = (((uint32_t)String[11] & 0xFF) << 24) | (((uint32_t)String[12] & 0xFF) << 16) | (((uint32_t)String[13] & 0xFF)  << 8) | ((uint32_t)String[14] & 0xFF);  
  //printf("%02x %02x %02x %02x - ", (String[11] & 0xFF),(String[12] & 0xFF),(String[13] & 0xFF),(String[14] & 0xFF));
  printf("PSA Packet: Milliseconds = %u, ",MySeconds);  //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  MySeconds = MyPSA->Seconds;


  uint64_t MyECEFX_int = 0;
  double MyECEFX = 0.0;
  MyECEFX_int = (((uint64_t)String[15] & 0xFF) << 56) | (((uint64_t)String[16] & 0xFF) << 48) | (((uint64_t)String[17] & 0xFF) << 40) |  (((uint64_t)String[18] & 0xFF) << 32) |  (((uint64_t)String[19] & 0xFF) << 24) | (((uint64_t)String[20] & 0xFF) << 20) |  (((uint64_t)String[21] & 0xFF) << 8) | ((uint64_t)String[22] & 0xFF);
  MyECEFX = *(double *) &MyECEFX_int;
  printf("ECEF-X = %4.2f, ", MyECEFX);  
  MyECEFX = MyPSA->ECEFX;


  uint64_t MyECEFY_int = 0;
  double MyECEFY = 0.0;
  MyECEFY_int = (((uint64_t)String[23] & 0xFF) << 56) | (((uint64_t)String[24] & 0xFF) << 48) | (((uint64_t)String[25] & 0xFF) << 40) |  (((uint64_t)String[26] & 0xFF) << 32) |  (((uint64_t)String[27] & 0xFF) << 24) | (((uint64_t)String[28] & 0xFF) << 20) |  (((uint64_t)String[29] & 0xFF) << 8) | ((uint64_t)String[30] & 0xFF);
  MyECEFY = *(double *) &MyECEFY_int;
  printf("ECEF-Y = %4.2f, ", MyECEFY);  
  MyECEFY = MyPSA->ECEFY;


  uint64_t MyECEFZ_int = 0;
  double MyECEFZ = 0.0;
  MyECEFZ_int = (((uint64_t)String[31] & 0xFF) << 56) | (((uint64_t)String[32] & 0xFF) << 48) | (((uint64_t)String[33] & 0xFF) << 40) |  (((uint64_t)String[34] & 0xFF) << 32) |  (((uint64_t)String[35] & 0xFF) << 24) | (((uint64_t)String[36] & 0xFF) << 20) |  (((uint64_t)String[37] & 0xFF) << 8) | ((uint64_t)String[38] & 0xFF);
  MyECEFZ = *(double *) &MyECEFZ_int;
  printf("ECEF-Z = %4.2f, ", MyECEFZ);  
  MyECEFZ = MyPSA->ECEFZ;


  uint32_t MyECEFX_vel_int;
  float MyECEFX_vel = 0.0;
  MyECEFX_vel_int = (((uint32_t)String[39] & 0xFF) << 24) | (((uint32_t)String[40] & 0xFF) << 16) | (((uint32_t)String[41] & 0xFF)  << 8) | ((uint32_t)String[42] & 0xFF);  
  MyECEFX_vel = *(float *) &MyECEFX_vel_int;
  printf("ECEF-X velocity = %4.2f, ",MyECEFX_vel);  
  MyECEFX_vel = MyPSA->ECEFX_vel;


  uint32_t MyECEFY_vel_int;
  float MyECEFY_vel = 0.0;
  MyECEFY_vel_int = (((uint32_t)String[43] & 0xFF) << 24) | (((uint32_t)String[44] & 0xFF) << 16) | (((uint32_t)String[45] & 0xFF)  << 8) | ((uint32_t)String[46] & 0xFF);  
  MyECEFY_vel = *(float *) &MyECEFY_vel_int;
  printf("ECEF-Y velocity = %4.2f, ",MyECEFY_vel);  
  MyECEFY_vel = MyPSA->ECEFY_vel;


  uint32_t MyECEFZ_vel_int;
  float MyECEFZ_vel = 0.0;
  MyECEFZ_vel_int = (((uint32_t)String[47] & 0xFF) << 24) | (((uint32_t)String[48] & 0xFF) << 16) | (((uint32_t)String[49] & 0xFF)  << 8) | ((uint32_t)String[50] & 0xFF);  
  MyECEFZ_vel = *(float *) &MyECEFZ_vel_int;
  printf("ECEF-Z velocity = %4.2f, ",MyECEFZ_vel);  
  MyECEFZ_vel = MyPSA->ECEFZ_vel;


  uint32_t MyClock_off_int;
  float MyClock_off = 0.0;
  MyClock_off_int = (((uint32_t)String[51] & 0xFF) << 24) | (((uint32_t)String[52] & 0xFF) << 16) | (((uint32_t)String[53] & 0xFF)  << 8) | ((uint32_t)String[54] & 0xFF);  
  MyClock_off = *(float *) &MyClock_off_int;
  printf("Clock Offset = %4.2f, ",MyClock_off);  
  MyClock_off = MyPSA->Clock_offset;


  uint32_t MyFreq_off_int;
  float MyFreq_off = 0.0;
  MyFreq_off_int = (((uint32_t)String[55] & 0xFF) << 24) | (((uint32_t)String[56] & 0xFF) << 16) | (((uint32_t)String[57] & 0xFF)  << 8) | ((uint32_t)String[58] & 0xFF);  
  MyFreq_off = *(float *) &MyFreq_off_int;
  printf("Frequency Offset = %4.2f, ",MyFreq_off);  
  MyFreq_off = MyPSA->Freq_off;

  //DOESN'T SEEM RIGHT...
  uint16_t MyPDOP;
  MyPDOP = (((uint16_t)String[59] & 0xFF) << 8) | ((uint16_t)String[60] & 0xFF);
  printf("PDOP = %u, ", MyPDOP);
  MyPDOP = MyPSA->PDOP;

  uint8_t MyNumSats;
  MyNumSats = (uint8_t)(String[61] & 0xFF);
  printf("Number of Satellites = %u, ", MyNumSats);
  MyNumSats = MyPSA->NumSats;


  //NOT SURE ABOUT THIS ONE EITHER...
  uint8_t MyPosMode;
  MyPosMode = (uint8_t)(String[62] & 0xFF);
  printf("Postion Mode = %u, ", MyPosMode);
  MyPosMode = MyPSA->PosMode;


  uint32_t MyYaw_int;
  float MyYaw = 0.0;
  MyYaw_int = (((uint32_t)String[63] & 0xFF) << 24) | (((uint32_t)String[64] & 0xFF) << 16) | (((uint32_t)String[65] & 0xFF)  << 8) | ((uint32_t)String[66] & 0xFF);  
  MyYaw = *(float *) &MyYaw_int;
  printf("Yaw = %4.2f, ",MyYaw);  
  MyYaw = MyPSA->Yaw;


  uint32_t MyHeading_int;
  float MyHeading = 0.0;
  MyHeading_int = (((uint32_t)String[67] & 0xFF) << 24) | (((uint32_t)String[68] & 0xFF) << 16) | (((uint32_t)String[69] & 0xFF)  << 8) | ((uint32_t)String[70] & 0xFF);  
  MyHeading = *(float *) &MyHeading_int;
  printf("Heading = %4.2f, ",MyHeading);  
  MyHeading = MyPSA->Heading;


  uint32_t MyPitch_int;
  float MyPitch = 0.0;
  MyPitch_int = (((uint32_t)String[71] & 0xFF) << 24) | (((uint32_t)String[72] & 0xFF) << 16) | (((uint32_t)String[73] & 0xFF)  << 8) | ((uint32_t)String[74] & 0xFF);  
  MyPitch = *(float *) &MyPitch_int;
  printf("Pitch = %4.2f, ",MyPitch);  
  MyPitch = MyPSA->Pitch;


  uint32_t MyRoll_int;
  float MyRoll = 0.0;
  MyRoll_int = (((uint32_t)String[75] & 0xFF) << 24) | (((uint32_t)String[76] & 0xFF) << 16) | (((uint32_t)String[77] & 0xFF)  << 8) | ((uint32_t)String[78] & 0xFF);  
  MyRoll = *(float *) &MyRoll_int;
  printf("Roll = %4.2f, ",MyRoll);  
  MyRoll = MyPSA->Roll;


  uint8_t MyAttState;
  MyAttState = (uint8_t)(String[79] & 0xFF);
  printf("Attitude State = %u, ", MyAttState);
  MyAttState = MyPSA->AttState;


  uint8_t MyPosState;
  MyPosState = (uint8_t)(String[80] & 0xFF);
  printf("Postion State = %u, ", MyPosState);
  MyPosState = MyPSA->PosState;


  //NEEDS WORK!
  uint16_t MyChecksum;
  MyChecksum = (((uint16_t)String[81] & 0xFF) << 8) | ((uint16_t)String[82] & 0xFF);
  printf("MyChecksum? = %u \n", MyChecksum);
  MyChecksum = MyPSA->Checksum;



  return(0);
}
//______________________________________________________________________________________________________



//______________________________________________________________________________________________________
int DecodeBIT(struct BITStruct * MyBIT, uint8_t * String, size_t Len){

  char MyBatt_Test;
  MyBatt_Test = String[11];  
  printf("BIT Packet: Battery Test = %c, ",MyBatt_Test);  //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  MyBatt_Test = MyBIT->Batt_Test;


  char MyParam_Test;
  MyParam_Test = String[13];  
  printf("Parameter Test = %c, ",MyParam_Test);  
  MyParam_Test = MyBIT->Param_Test;


  char MyError_Test;
  MyError_Test = String[15];  
  printf("Fatal Error Test = %c, ",MyError_Test);  
  MyError_Test = MyBIT->Error_Test;


  char MyInactive_Sens;
  MyInactive_Sens = String[17];  
  printf("Number of Inactive Sensors = %c, ",MyInactive_Sens);  
  MyInactive_Sens = MyBIT->Inactive_Sens;


  //THIS ONE DOESN'T WORK YET
  char MyFailed_blocks [3];
  MyFailed_blocks[0] = String[19];  
  MyFailed_blocks[1] = String[20];  
  MyFailed_blocks[2] = String[21];  
  printf("Percentage Failed Data Blocks = %s \n",MyFailed_blocks);  
  //MyFailed_blocks = MyBIT->Failed_blocks;



  return(0);
}
//_________________________________________________________________________________________________


//_________________________________________________________________________________________________
int DecodeTST(struct TSTStruct * MyTST, uint8_t * String, size_t Len){

  char MyEPROM_Test;
  MyEPROM_Test = String[11];  
  printf("TST Packet: EPROM Test = %c, ",MyEPROM_Test);  // 'EPROM = 0' //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  MyEPROM_Test = MyTST->EPROM_Test;


  char MyRAM_Test;
  MyRAM_Test = String[12];  
  printf("RAM Test = %c, ",MyRAM_Test);  
  MyRAM_Test = MyTST->RAM_Test;


  char MyChan_Init_Test;
  MyChan_Init_Test = String[13];  
  printf("Channel Initialization Test = %c \n",MyChan_Init_Test);  
  MyChan_Init_Test = MyTST->Chan_Initialization_Test;

  return(0);
}
//________________________________________________________________________________________________



//_________________________________________________________________________________________________
int DecodeMag(struct MagStruct * MyMag, uint8_t* String, size_t Len){

  //  printf(" %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n", String[0] & 0xFF, String[1] & 0xFF, String[2] & 0xFF, String[3] & 0xFF, String[4] & 0xFF, String[5] & 0xFF, String[6] & 0xFF, String[7] & 0xFF, String[8] & 0xFF, String[9] & 0xFF, String[10] & 0xFF, String[11] & 0xFF, String[12] & 0xFF, String[13] & 0xFF, String[14] & 0xFF, String[15] & 0xFF, String[16] & 0xFF, String[17] & 0xFF, String[18] & 0xFF, String[19] & 0xFF, String[20] & 0xFF, String[21] & 0xFF, String[22] & 0xFF, String[23] & 0xFF);

  int16_t MyRoll_int;
  float MyRoll = 0.0;
  MyRoll_int = (((int16_t)String[4] & 0xFF) << 8) | ((int16_t)String[5] & 0xFF);  
  MyRoll = MyRoll_int/10.0;
  printf("Magnetometer Packet: Roll = %4.2f, ",MyRoll); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  M_Packet.roll =  MyRoll; 
  MyRoll = MyMag->Roll;


  int16_t MyMagRoll_int;
  float MyMagRoll = 0.0;
  MyMagRoll_int = (((int16_t)String[6] & 0xFF) << 8) | ((int16_t)String[7] & 0xFF);  
  MyMagRoll = MyMagRoll_int/10.0;
  printf("Mag Roll = %4.2f, ",MyMagRoll);  
  MyMagRoll = MyMag->MagRoll;


  int16_t MyInclination_int;
  float MyInclination = 0.0;
  MyInclination_int = (((int16_t)String[8] & 0xFF) << 8) | ((int16_t)String[9] & 0xFF);  
  MyInclination = MyInclination_int/10.0;
  printf("Inclination = %4.2f, ",MyInclination); 
  M_Packet.pitch =  MyInclination; 
  MyInclination = MyMag->Inclination;


  int16_t MyMagTot_int;
  float MyMagTot = 0.0;
  MyMagTot_int = (((int16_t)String[10] & 0xFF) << 8) | ((int16_t)String[11] & 0xFF);  
  MyMagTot = MyMagTot_int/10000.0;
  printf("Mag Total = %4.2f, ",MyMagTot);  
  MyMagTot = MyMag->MagTotal;


  int16_t MyAzi_int;
  float MyAzi = 0.0;
  MyAzi_int = (((int16_t)String[12] & 0xFF) << 8) | ((int16_t)String[13] & 0xFF);  
  MyAzi = MyAzi_int/10.0;
  printf("Azimuth = %4.2f, ",MyAzi);  
  M_Packet.heading =  MyAzi; 
  MyAzi = MyMag->Azimuth;


  int16_t MyAccel_int;
  float MyAccel = 0.0;
  MyAccel_int = (((int16_t)String[14] & 0xFF) << 8) | ((int16_t)String[15] & 0xFF);  
  MyAccel = MyAccel_int/10000.0;
  printf("Acceleration = %4.2f, ",MyAccel);  
  MyAccel = MyMag->Acceleration;


  int16_t MyTemp_int;
  float MyTemp = 0.0;
  MyTemp_int = (((int16_t)String[16] & 0xFF) << 8) | ((int16_t)String[17] & 0xFF);  
  MyTemp = MyTemp_int/100.0;
  printf("Temperature = %4.2f, ",MyTemp);  
  MyTemp = MyMag->Temperature;


  int16_t MyVolt_int;
  float MyVolt = 0.0;
  MyVolt_int = (((int16_t)String[18] & 0xFF) << 8) | ((int16_t)String[19] & 0xFF);  
  MyVolt = MyVolt_int/100.0;
  printf("Voltage = %4.2f, ",MyVolt);  
  MyVolt = MyMag->Voltage;


  //CHECKSUM!


  uint32_t MyClock = 0;  //counting 10 MHz clock signal.
  MyClock = (((uint32_t)String[-4] & 0xFF) << 24) | (((uint32_t)String[-3] & 0xFF) << 16) | (((uint32_t)String[-2] & 0xFF)  << 8) | ((uint32_t)String[-1] & 0xFF);  
  printf("Clock =  %u. \n",MyClock);  
  MyClock = MyMag->Clock;
  
  
  
  printf("\n");  
  M_Packet.GPS_or_magnetometer = 1;
  printf("Now, here is what's from M_Packet: \n");
  printf("Heading: \n"); 
  printf("%f\n",M_Packet.heading); 
  printf("Pitch: \n"); 
  printf("%f\n",M_Packet.pitch); 
  printf("Roll: \n"); 
  printf("%f\n",M_Packet.roll); 
  
  M_Packet.geographic_latitude = GPS_Packet.geographic_latitude;
  M_Packet.geographic_longitude = GPS_Packet.geographic_longitude;
  M_Packet.elevation = GPS_Packet.elevation;
  
  printf("This is GPS stuff we put in the M_Packet: \n");
  printf("Geographic Latitude: \n"); 
  printf("%f\n",M_Packet.geographic_latitude); 
  printf("Geographic Longitude: \n"); 
  printf("%f\n",M_Packet.geographic_longitude);  
  printf("Elevation: \n"); 
  printf("%f\n",M_Packet.elevation); 
  
  M_Packet.date_and_time = GPS_Packet.date_and_time;
  M_Packet.nanoseconds = GPS_Packet.nanoseconds;  
  
  printf("Date_and_Time: \n");
  cout << M_Packet.date_and_time << endl;  
  printf("Nanoseconds: \n"); 
  printf("%u\n",M_Packet.nanoseconds);  
  printf("\n"); 
  
  
  AR->AddAspectFrame(M_Packet);
  cout << "Magnetometer Packet added!!!!!" << endl;
  printf("\n"); 
  
  return(0);
}




//*********************************************************************************************************
//*      Main program to parse the flight computer data to interpret dGPS messages.                       *
//*********************************************************************************************************



int main() {
	


  
  FILE *fp;
  uint8_t buffer[8192];
  size_t Len;  
  unsigned int buffer_index;
  int num_aspect_packets = 0; //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  int num_DSO = 0;
  int num_PSA = 0;
  int num_TST = 0;
  int num_BIT = 0;
  uint16_t size;     
  unsigned int packet_index;    //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  int Len_DSO = 84;    //Carolyn is still alittle unsure about these.
  int Len_PSA = 114;
  int Len_BIT = 32;
  int Len_TST = 24;
  int Len_Mag = 22;

  
  string slash = "/";
  string colon = ":";
  string twenty = "20";
  string space = " ";
  string zero = "0";
  
  string filename = "COSIa080814_133925.dat";
  
  string month = filename.substr(5,2);
  if(month[0] == '0'){
  	month = month.substr(1,1);
    }
  string day = filename.substr(7,2);
  if(day[0] == '0'){
  	day = day.substr(1,1);
    }
  string year = twenty + filename.substr(9,2);
  
  string date = year + slash + month + slash + day;
  
  
  Evil_Packet.date_and_time = date;


  

  
  
  fp = fopen("COSIa080814_133925.dat", "r");
  
  if( fp == NULL ) {
    perror("Error while opening file");
    exit(EXIT_FAILURE);
  }

  do{
    Len = fread(buffer, 1, 8192, fp);

    for(buffer_index = 0; buffer_index < sizeof(buffer); ++buffer_index) { 
 
    if (buffer[buffer_index] == 0xeb && buffer[buffer_index+1] == 0x90 && buffer[buffer_index+2] == 0x05) {
	size = ( (uint16_t)buffer[buffer_index+8] << 8) | ( (uint16_t)buffer[buffer_index+9]);

	for(packet_index = buffer_index; packet_index < (size + buffer_index); ++packet_index) //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	  {
	    if (strncmp((char *) &buffer[packet_index],"$PASHR,DSO",10) == 0 ) { //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	      if ( (packet_index+Len_DSO) > sizeof(buffer)) { //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
		break;
	      }
	      struct DSOStruct CarolynsDSO;
	      int err;
	      err = DecodeDSO(&CarolynsDSO, &buffer[packet_index], Len_DSO); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	        if (err != 0){    
		perror("Error while filling DSOStruct");
		exit(EXIT_FAILURE);
	      }

		if (strncmp((char *) &buffer[packet_index + 84], "$M",2) == 0) { //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
		  struct MagStruct CarolynsMag;
		  int err;
		  err = DecodeMag(&CarolynsMag, &buffer[packet_index + 84], Len_Mag); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
		  if (err != 0){    
		    perror("Error while filling MagStruct");
		    exit(EXIT_FAILURE);
		  }
		}
		else {
		  printf("\n No magnetometer packet found! \n\n"); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
		}

	      packet_index = packet_index + Len_DSO -1; //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	      num_DSO = num_DSO + 1;
	    }


	    if (strncmp((char *) &buffer[packet_index],"$PASHR,PSA",10) == 0 ) { //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	      if ( (packet_index+Len_PSA) > sizeof(buffer)) { //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
		break;
	      }
	      struct PSAStruct CarolynsPSA;
	      int err;
	      err = DecodePSA(&CarolynsPSA, &buffer[packet_index], Len_PSA); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	        if (err != 0){    
		perror("Error while filling PSAStruct");
		exit(EXIT_FAILURE);
	      }
		packet_index = packet_index + Len_PSA -1; //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	      num_PSA = num_PSA + 1;
	    }


	    if (strncmp((char *) &buffer[packet_index],"$PASHR,BIT",10) == 0 ) {
	      if ( (packet_index+Len_BIT) > sizeof(buffer)) { //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
		break;
	      }
	      struct BITStruct CarolynsBIT;
	      int err;
	      err = DecodeBIT(&CarolynsBIT, &buffer[packet_index], Len_BIT); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	        if (err != 0){    
		perror("Error while filling BITStruct");
		exit(EXIT_FAILURE);
	      }
		packet_index = packet_index + Len_BIT -1; //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	      num_BIT = num_BIT + 1;
	    }


	    if (strncmp((char *) &buffer[packet_index],"$PASHR,TST",10) == 0 ) { //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	      if ( (packet_index+Len_TST) > sizeof(buffer)) { //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
		break;
	      }
	      struct TSTStruct CarolynsTST;
	      int err;
	      err = DecodeTST(&CarolynsTST, &buffer[packet_index], Len_TST); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	        if (err != 0){    
		perror("Error while filling TSTStruct");
		exit(EXIT_FAILURE);
	      }
		packet_index = packet_index + Len_TST -1; //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	      num_TST = num_TST + 1;
	    }



	  }
	num_aspect_packets = num_aspect_packets+1; //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
	buffer_index = buffer_index + size;
      }
    }
  } while(Len == 8192);


  printf("\n Total number of aspect packets found is %d\n", num_aspect_packets); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  printf("     number of DSO packets = %d\n", num_DSO); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  printf("     number of PSA packets = %d\n", num_PSA); //Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  printf("     number of BIT packets = %d\n", num_BIT);//Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects
  printf("     number of TST packets = %d\n", num_TST);//Once again, let me be clear, these are Carolyn's packets, not MNCTAspectPacket objects

  fclose(fp);	
	
	

  cout << "" << endl;
  cout << "Preparing to declare MTime..." << endl;
  cout << "Still preparing to declare MTime..." << endl;
  cout << "" << endl;
	
  MTime example_time;
  example_time.Set(2014,8,8,18,43,0,600000000);
  cout << AR->MNCTAspectReconstruction::GetAspectMagnetometer(example_time)->GetAltitude() << endl;
  cout << "Program finished..." << endl;	
  return 1;
  }
