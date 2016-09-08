/*
 * MNCTAspectReconstruction.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTAspectReconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTAspectReconstruction.h"

// Standard libs:
#include "stdio.h"
#include "cstdio"
#include "cstdlib"
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cctype>
#include <map>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MString.h"
#include "MFile.h"
#include "MNCTAspectPacket.h"
#include "magfld.h"


////////////////////////////////////////////////////////////////////////////////

#ifdef ___CINT___
ClassImp(MNCTAspectReconstruction)
#endif


////////////////////////////////////////////////////////////////////////////////

bool MTimeSort( MNCTAspect* A1, MNCTAspect* A2 );

////////////////////////////////////////////////////////////////////////////////

MNCTAspectReconstruction::MNCTAspectReconstruction()
{
	// Construct an instance of MNCTAspectReconstruction
	Clear();
}

////////////////////////////////////////////////////////////////////////////////

MNCTAspectReconstruction::~MNCTAspectReconstruction()
{
	// Delete this instance of MNCTAspectReconstruction
}

////////////////////////////////////////////////////////////////////////////////


void MNCTAspectReconstruction::Clear()
{
	// Reset all data

	for (auto A: m_Aspects_GPS) {
		delete A;
	}
	m_Aspects_GPS.clear();



	for (auto A: m_Aspects_Magnetometer) {
		delete A;
	}

	LastAspectInDeque = 0;
	m_Aspects_Magnetometer.clear();	
	m_IsDone = false;

}


////////////////////////////////////////////////////////////////////////////////

bool MNCTAspectReconstruction::Initialize()
{
	//Initialize the module
	return true;
}

////////////////////////////////////////////////////////////////////////////////

// Add and reconstruction one or more aspect frames - return false on error

bool MNCTAspectReconstruction::AddAspectFrame(MNCTAspectPacket PacketA)
{


	MNCTAspect* Aspect = new MNCTAspect;
	double BRMS = PacketA.BRMS;
	uint16_t AttFlag = PacketA.AttFlag;
	int GPS_or_magnetometer = PacketA.GPS_or_magnetometer;
	int test_or_not = PacketA.test_or_not;
	double geographic_longitude = PacketA.geographic_longitude;
	double geographic_latitude = PacketA.geographic_latitude;
	double elevation = PacketA.elevation; //Note "elevation" here is height above sea

	if( GPS_or_magnetometer == 0 ){
		if( (PacketA.BRMS < 1.0E-6) || (PacketA.BRMS > 1.0) || (PacketA.AttFlag != 0) ){
			//just gonna return true here for now...
			delete Aspect;
			return true; //AWL commented out so we could test the absolute timing of events
		}
	}

	MTime UTCTime((long int)PacketA.UnixTime, (long int)(PacketA.GPSms % 1000)*1000000);
	MTime ClkTime((long int)(PacketA.PPSClk/10000000),(long int)((PacketA.PPSClk % 10000000)*100)); //start with PPS value
	ClkTime += MTime((long int)0,(long int)(PacketA.GPSms % 1000)*1000000); //then add offset corresponding to the number of ms into the current second

	double heading = PacketA.heading;
	double magnetic_declination = 0.0;


	//compute magnetic declination
	if (GPS_or_magnetometer == 1){
		float lon = geographic_longitude;
		float lat = geographic_latitude;
		float alt = elevation;
		MTime MagMTime;
		MagMTime.Set(UTCTime);
		double frac_Year = MagMTime.GetAsYears();
		float gcu_fracYear = frac_Year;
		//magfld* MF = new magfld();
		void WMMInit();
		float magdec_Cplusplus1 = MagDec(lat, lon, alt, gcu_fracYear);
		magnetic_declination = magdec_Cplusplus1;
		if (test_or_not == 0 && g_Verbosity >= c_Info) {		
			printf("According to C++, magnetic_declination is: %9.5f \n",magdec_Cplusplus1);	
	
		}
		if (g_Verbosity >= c_Info) { 
      		cout<<"magnetic declination = "<<magnetic_declination<<endl;
      		cout<<"lat = "<<lat<<" lon = "<<lon<<" altitude = "<<alt<<endl;
   	  	}
	}

	heading = heading + magnetic_declination; //if GPS packet, magnetic_declination = 0.

	heading = 360.0-heading;//The GPS system defines a positive heading angle as a clockwise rotation, and with the y direction as the forward facing vector, and the x direction as the right facing vector (when observing the system from above, down the z-axis), a 90 degree positve heading rotation would transform from the +y axis of the GPS to the +x axis of the GPS. The "360-heading" here transforms the heading angle into a right-handed coorindate system (a 90 degree postion heading rotation wouls transform the +x axis of the GPS to the +y axis of the GPS) so we can apply the traditional rotaion matrices.

	double pitch =  PacketA.pitch;
	if (GPS_or_magnetometer == 1){
		pitch = pitch - 90.0; //The magnetometer defines pitch with a 90 degree offset
	}

	double roll = PacketA.roll;
	if( GPS_or_magnetometer == 1 ){
		//magnetometer has a 180 degree offset with current mounting in the gondola.
		roll = roll + 180.0;
		if( roll > 360.0 ){
			roll = roll - 360.0;
		}
	}

   	if (g_Verbosity >= c_Info) {
    	cout<<"heading = "<<heading<<" pitch = "<<pitch<<" roll = "<<roll<<endl;
     	cout<<"GPS_or_magnetometer = "<<GPS_or_magnetometer<<endl;
   	}



	//definte the rotation matrices (https://en.wikipedia.org/wiki/Davenport_chained_rotations) to allow us to tranform from heading, pitch and roll, to the local GPS coordinates, and vise versa.

	double Rot_Z[3][3] = { {cosine(heading), -sine(heading), 0.0}, {sine(heading), cosine(heading), 0.0}, {0.0, 0.0, 1.0} };
	double Rot_Y[3][3] = { {cosine(roll), 0.0, sine(roll)}, {0.0, 1.0, 0.0}, {-sine(roll), 0.0, cosine(roll)} };
	double Rot_X[3][3] = { {1.0, 0.0, 0.0}, {0.0, cosine(pitch), -sine(pitch)}, {0.0, sine(pitch), cosine(pitch)} };

	double Rot_XY[3][3], Rot_ZXY[3][3];

	//multiply matricies together to get full rotation matrix from heading, pitch, and roll back to local GPS coordinates. The convention for Tait-Bryan chained rotations is to apply the rotations in this order: Yaw, Pitch, Roll. Here, the matricies are written in reverse to represent an extrinsic rotation. CK double check the output makes sense.
	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){   
			Rot_XY[i][j]=0;
			for(int k=0;k<3;k++){
				Rot_XY[i][j]=Rot_XY[i][j] + Rot_X[i][k]*Rot_Y[k][j];
			}
		}
	}

	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){   
			Rot_ZXY[i][j]=0;
			for(int k=0;k<3;k++){
				//Still some question about the order of this rotation here. CK is pretty convinced this is right, but it would be worth switching the order here if we're still having aspect problems down the line.
				Rot_ZXY[i][j]=Rot_ZXY[i][j] + Rot_Z[i][k]*Rot_XY[k][j];
			}
		}
	}


    //define the axes of the unrotated GPS coordinate system with unit vectors xhat, yhat and zhat. yhat points true north, and zhats points towards the zenith in this unrotated frame (i.e. 0 pitch, 0 roll, 0 heading).
    double Zhat[3] = {0.0, 0.0, 1.0};
    double Yhat[3] = {0.0, 1.0, 0.0};
    double Xhat[3] = {1.0, 0.0, 0.0};
    double temp[3];
	

	//Now, define the GPS pre-rotation matrix RotGPSCryo_ZXY, which is the rotation matrix between the cryostat coorindates, with +x pointing towards the front of the gondola (sun side), and the GPS coordinates, with +y pointing towards the back of the gonodla. To first order, this is just a -90 degree rotation around the z-axis.
	//The magnetometer doesn't require this extra rotation matrix because its axes are already aligned with the cryostat

	if( GPS_or_magnetometer == 0 ){

		//To second order, the GPS and cryostat are not perfectly aligned. Theodolite measurements were taken to better understand this offset, but have not yet been applied here. Once the Crab was detected during the 2016 flight, Alex worked towards getting the relative offsets that better centered the Crab, these are added below, but this is only preliminary and should be better defined.
		const double dh = 0.0;
		const double dp = 0.0;
		const double dr = 0.0;
		//Corrections from Crab observations 2016:
	/*	dh = 5.2;
		dp = -0.5;
		dr = -0.5;
	*/

		//define the three axis rotation matricies:
		double RotGPSCryo_Z[3][3] = { {cosine(-90 + dh), -sine(-90 + dh), 0.0}, {sine(-90 + dh), cosine(-90 + dh), 0.0}, {0.0, 0.0, 1.0} };
		double RotGPSCryo_Y[3][3] = { {cosine(dr), 0.0, sine(dr)}, {0.0, 1.0, 0.0}, {-sine(dr), 0.0, cosine(dr)} };
		double RotGPSCryo_X[3][3] = { {1.0, 0.0, 0.0}, {0.0, cosine(dp), -sine(dp)}, {0.0, sine(dp), cosine(dp)} };
		

		double RotGPSCryo_XY[3][3], RotGPSCryo_ZXY[3][3];

		//multiply the rotaion matricies to make one total 3x3 matrix that represents the 3-d rotation between cryostat coordinates and GPS coordinates. Using the same convention as above.
		for(int i=0;i<3;i++){
			for(int j=0;j<3;j++){   
		   	RotGPSCryo_XY[i][j]=0;
				for(int k=0;k<3;k++){
					RotGPSCryo_XY[i][j] = RotGPSCryo_XY[i][j] + RotGPSCryo_X[i][k]*RotGPSCryo_Y[k][j];
				}
			}
		}

		for(int i=0;i<3;i++){
			for(int j=0;j<3;j++){   
				RotGPSCryo_ZXY[i][j]=0;
				for(int k=0;k<3;k++){
					RotGPSCryo_ZXY[i][j] = RotGPSCryo_ZXY[i][j] + RotGPSCryo_Z[i][k]*RotGPSCryo_XY[k][j];
				}
			}
		}


	
	//Transform the xhat, yhat and zhat into the cryostat coordinate system. These unit vectors now represent the pointing of the cryostat axis in the GPS local coorindate system.

		//transform Xhat
		for(int i=0;i<3;i++){
			double Q = 0;
			for(int k=0;k<3;k++){
				Q += RotGPSCryo_ZXY[i][k] * Xhat[k];
			}
			temp[i] = Q;
		}
		Xhat[0] = temp[0]; Xhat[1] = temp[1]; Xhat[2] = temp[2];

        //transform Yhat
        for(int i=0;i<3;i++){
            double Q = 0; 
            for(int k=0;k<3;k++){
                Q += RotGPSCryo_ZXY[i][k] * Yhat[k];
            }    
            temp[i] = Q; 
        }    
        Yhat[0] = temp[0]; Yhat[1] = temp[1]; Yhat[2] = temp[2];

		//transform Zhat
		for(int i=0;i<3;i++){
			double Q = 0;
			for(int k=0;k<3;k++){
				Q += RotGPSCryo_ZXY[i][k] * Zhat[k];
			}
			temp[i] = Q;
		}
		Zhat[0] = temp[0]; Zhat[1] = temp[1]; Zhat[2] = temp[2];

	}


	//now Zhat, Yhat, and Xhat are represent cryostat's axis in the GPS local frame, apply the full ZXY rotation matrix to Zhat, Xhat, and Yhat to determine the true cryostat pointing taking into account heading, pitch and roll, in the unrotated GPS reference frame, with +y pointing north and +z pointing towards the zenith..

	//transform Xhat
	for(int i=0;i<3;i++){
		double Q = 0;
		for(int k=0;k<3;k++){
			Q += Rot_ZXY[i][k] * Xhat[k];
		}
		temp[i] = Q;
	}
	Xhat[0] = temp[0]; Xhat[1] = temp[1]; Xhat[2] = temp[2];

    //transform Yhat
    for(int i=0;i<3;i++){
        double Q = 0;
        for(int k=0;k<3;k++){
            Q += Rot_ZXY[i][k] * Yhat[k];
        }
        temp[i] = Q;
    }
    Yhat[0] = temp[0]; Yhat[1] = temp[1]; Yhat[2] = temp[2];

    //transform Zhat
    for(int i=0;i<3;i++){
        double Q = 0;
        for(int k=0;k<3;k++){
            Q += Rot_ZXY[i][k] * Zhat[k];
        }
        temp[i] = Q;
    }
    Zhat[0] = temp[0]; Zhat[1] = temp[1]; Zhat[2] = temp[2];


	m_TCCalculator.SetLocation(geographic_latitude,geographic_longitude);
	m_TCCalculator.SetUnixTime(UTCTime.GetAsSystemSeconds()); //AWL use the UnixTimeFromGPSTime

	//dot Zhat/Yhat/Xhat into (0,0,1) and take the arccos to get the zenith angle.  then convert this to elevation angle
	double Z_Elevation = 90.0 - arccosine( Zhat[2] );
	double Y_Elevation = 90.0 - arccosine( Yhat[2] );
    double X_Elevation = 90.0 - arccosine( Xhat[2] );

	//get azimuth of Zhat/Yhat/Xhat by projecting into X-Y plane, re-normalizing, and dotting into (0,1,0)
	double Z_proj[3]; Z_proj[0] = Zhat[0]; Z_proj[1] = Zhat[1]; Z_proj[2] = 0.0;
	double Y_proj[3]; Y_proj[0] = Yhat[0]; Y_proj[1] = Yhat[1]; Y_proj[2] = 0.0;
    double X_proj[3]; X_proj[0] = Xhat[0]; X_proj[1] = Xhat[1]; X_proj[2] = 0.0;

	double Znorm = sqrt( pow(Z_proj[0],2.0) + pow(Z_proj[1],2.0) );
	double Ynorm = sqrt( pow(Y_proj[0],2.0) + pow(Y_proj[1],2.0) );
    double Xnorm = sqrt( pow(X_proj[0],2.0) + pow(X_proj[1],2.0) );

  
	//IMPORTANT since the magnetometer reference frame has the X axis pointing at true north,
	//we need to look at the azimuth angle from the X axis rather than the y axis (from GPS)
	double Z_Azimuth,X_Azimuth,Y_Azimuth;
	
	//Azimuth calculation for GPS:
	if( GPS_or_magnetometer == 0 ){

		//for GPS, the azimuth angle is the angle between the projected vector and the y axis, so pass the y component 
		//of the projected vectors to the arccosine

		if( Znorm > 1.0E-9 ){
			Z_Azimuth = arccosine( Z_proj[1]/Znorm ); if( Z_proj[0] < 0.0 ) Z_Azimuth = 360.0 - Z_Azimuth;
		} else {
			Z_Azimuth = 0.0;
		}

		if( Ynorm > 1.0E-9 ){
			Y_Azimuth = arccosine( Y_proj[1]/Ynorm ); if( Y_proj[0] < 0.0 ) Y_Azimuth = 360.0 - Y_Azimuth;
		} else {
			Y_Azimuth = 0.0;
		}

        if( Xnorm > 1.0E-9 ){
            X_Azimuth = arccosine( X_proj[1]/Xnorm ); if( X_proj[0] < 0.0 ) X_Azimuth = 360.0 - X_Azimuth;
        } else {
            X_Azimuth = 0.0;
        }

        if (g_Verbosity >= c_Info){
			cout<<"X_Azimuth = "<<X_Azimuth<<" X_Elevation = "<<X_Elevation<<endl;
			cout<<"Y_Azimuth = "<<Y_Azimuth<<" Y_Elevation = "<<Y_Elevation<<endl;
			cout<<"Z_Azimuth = "<<Z_Azimuth<<" Z_Elevation = "<<Z_Elevation<<endl;
		}


	} else {

		//for magnetometer, the azimuth angle is the angle between the projected vector and the x axis, so pass the x component 
		//of the projected vectors to the arccosine

		if( Znorm > 1.0E-9 ){
			Z_Azimuth = arccosine( Z_proj[0]/Znorm ); if( Z_proj[1] > 0.0 ) Z_Azimuth = 360.0 - Z_Azimuth;
		} else {
			Z_Azimuth = 0.0;
		}

		if( Xnorm > 1.0E-9 ){
			X_Azimuth = arccosine( X_proj[0]/Xnorm ); if( X_proj[1] > 0.0 ) X_Azimuth = 360.0 - X_Azimuth;
		} else {
			X_Azimuth = 0.0;
		}

		if( Ynorm > 1.0E-9 ){
			Y_Azimuth = arccosine( Y_proj[0]/Ynorm ); if( Y_proj[1] > 0.0 ) Y_Azimuth = 360.0 - Y_Azimuth;
		} else {
			Y_Azimuth = 0.0;
		}
	
		if (g_Verbosity >= c_Info) {
			cout<<"X_Azimuth = "<<X_Azimuth<<" X_Elevation = "<<X_Elevation<<endl;
			cout<<"Y_Azimuth = "<<Y_Azimuth<<" Y_Elevation = "<<Y_Elevation<<endl;
			cout<<"Z_Azimuth = "<<Z_Azimuth<<" Z_Elevation = "<<Z_Elevation<<endl;		
		}

	}



	//Need to double check the TCCalculator used here!	
	vector<double> ZGalactic; 
	vector<double> ZEquatorial;
	ZEquatorial = m_TCCalculator.MNCTTimeAndCoordinate::Horizon2Equatorial(Z_Azimuth, Z_Elevation);
	ZGalactic = m_TCCalculator.MNCTTimeAndCoordinate::Equatorial2Galactic2(ZEquatorial);
	//Zra = ZEquatorial[0];
	//Zdec = ZEquatorial[1];
	double Zgalat = ZGalactic[1];
	double Zgalon = ZGalactic[0];
	if (g_Verbosity >= c_Info) cout<<"Z gal-lat = "<<Zgalat<<" Z gal-lon = "<<Zgalon<<endl;

	vector<double> YGalactic; 
	vector<double> YEquatorial;
	YEquatorial = m_TCCalculator.MNCTTimeAndCoordinate::Horizon2Equatorial(Y_Azimuth, Y_Elevation);
	YGalactic = m_TCCalculator.MNCTTimeAndCoordinate::Equatorial2Galactic2(YEquatorial);	
	//Yra = YEquatorial[0];
	//Ydec = YEquatorial[1];
	double Ygalat = YGalactic[1];
	double Ygalon = YGalactic[0];
	if (g_Verbosity >= c_Info) cout<<"Y gal-lat = "<<Ygalat<<" Y gal-lon = "<<Ygalon<<endl;

    vector<double> XGalactic; 
    vector<double> XEquatorial;
    XEquatorial = m_TCCalculator.MNCTTimeAndCoordinate::Horizon2Equatorial(X_Azimuth, X_Elevation);
	XGalactic = m_TCCalculator.MNCTTimeAndCoordinate::Equatorial2Galactic2(XEquatorial);
    //Xra = XEquatorial[0];
    //Xdec = XEquatorial[1];
    double Xgalat = XGalactic[1];
    double Xgalon = XGalactic[0];
    if (g_Verbosity >= c_Info) cout<<"X gal-lat = "<<Xgalat<<" X gal-lon = "<<Xgalon<<endl;


	Aspect->SetTime(ClkTime);
	Aspect->SetHeading(heading);
	Aspect->SetPitch(pitch);
	Aspect->SetRoll(roll);		
	Aspect->SetLatitude(geographic_latitude);
	Aspect->SetLongitude(geographic_longitude);
	Aspect->SetAltitude(elevation);
	Aspect->SetGalacticPointingXAxis(Xgalon, Xgalat);
	Aspect->SetGalacticPointingZAxis(Zgalon, Zgalat);
	Aspect->SetHorizonPointingXAxis(X_Azimuth, X_Elevation);
	Aspect->SetHorizonPointingZAxis(Z_Azimuth, Z_Elevation);
	Aspect->SetUTCTime(UTCTime);
	//cout << UnixTimeFromGPSTime.GetSeconds() << " ---> " << UnixTimeFromGPSTime.GetNanoSeconds() <<endl;
	//Aspect->SetGPSTime(GPSTime);
	Aspect->SetPPS(PacketA.PPSClk);
	//cout << "m_Aspect->GetPPS() returns " << Aspect->GetPPS() << endl; //this works
	Aspect->SetBRMS(BRMS);
	Aspect->SetAttFlag(AttFlag);
	Aspect->SetGPS_or_magnetometer(GPS_or_magnetometer);

	if (GPS_or_magnetometer == 0){

		m_Aspects_GPS.push_back(Aspect);
		sort(m_Aspects_GPS.begin(), m_Aspects_GPS.end(), MTimeSort);
		while(m_Aspects_GPS.size() > 256){
			auto x = m_Aspects_GPS.front();
			m_Aspects_GPS.pop_front();
			delete x;
		}
		SetLastAspectInDeque(m_Aspects_GPS); //For housekeeping file
	}


	if (GPS_or_magnetometer == 1){

		m_Aspects_Magnetometer.push_back(Aspect);
		sort(m_Aspects_Magnetometer.begin(), m_Aspects_Magnetometer.end(), MTimeSort);
		while(m_Aspects_Magnetometer.size() > 256){
			auto x = m_Aspects_Magnetometer.front();
			m_Aspects_Magnetometer.pop_front();
			delete x;
		}
		SetLastAspectInDeque(m_Aspects_Magnetometer); //For housekeeping file

	}

	return true;
}




////////////////////////////////////////////////////////////////////////////////

MNCTAspect* MNCTAspectReconstruction::GetAspect(MTime ReqTime, int GPS_Or_Magnetometer){

	MNCTAspect* ReqAspect = 0;

	if(GPS_Or_Magnetometer == 0){
		//check that there are aspect packets 
		if( m_Aspects_GPS.size() == 0 ){
			return 0;
		} else if( ReqTime < m_Aspects_GPS.front()->GetTime() ){
			ReqAspect = m_Aspects_GPS.front();
		} else if( ReqTime > m_Aspects_GPS.back()->GetTime() ){
			if(m_IsDone){
				ReqAspect = m_Aspects_GPS.back();
			} else {
				ReqAspect = 0;
			}
		} else {
			for( int i = m_Aspects_GPS.size()-2; i > -1; --i ){//this loop will only go if there are at least 2 aspects 
				if( ReqTime > m_Aspects_GPS[i]->GetTime() ){ //found the lower bracketing value
					if( (ReqTime - m_Aspects_GPS[i]->GetTime()) <= (m_Aspects_GPS[i+1]->GetTime() - ReqTime) ){ //check which bracketing value is closer
						ReqAspect = m_Aspects_GPS[i];
						break;
					} else {
						ReqAspect = m_Aspects_GPS[i+1];
						break;
					}
				}
			}
		}

	} else if(GPS_Or_Magnetometer == 1){

		//check that there are aspect packets 
		if( m_Aspects_Magnetometer.size() == 0 ){
			return 0;
		} else if( ReqTime < m_Aspects_Magnetometer.front()->GetTime() ){
			ReqAspect = m_Aspects_Magnetometer.front();
		} else if( ReqTime > m_Aspects_Magnetometer.back()->GetTime() ){
			if(m_IsDone){
				ReqAspect = m_Aspects_Magnetometer.back();
			} else {
				ReqAspect = 0;
			}
		} else {
			for( int i = m_Aspects_Magnetometer.size()-2; i > -1; --i ){
				if( ReqTime > m_Aspects_Magnetometer[i]->GetTime() ){ //found the lower bracketing value
					if( (ReqTime - m_Aspects_Magnetometer[i]->GetTime()) <= (m_Aspects_Magnetometer[i+1]->GetTime() - ReqTime) ){ //check which bracketing value is closer
						ReqAspect = m_Aspects_Magnetometer[i];
						break;
					} else {
						ReqAspect = m_Aspects_Magnetometer[i+1];
						break;
					}
				}
			}
		}
	}

	return ReqAspect;
}


////////////////////////////////////////////////////////////////////////////////


//Here we make trig functions that work with degrees.

double MNCTAspectReconstruction::sine(double sine_input){
	double sine_output = sin((sine_input * 3.14159265359)/180);
	return sine_output;
}
double MNCTAspectReconstruction::arcsine(double arcsine_input){
	double arcsine_output = ((asin(arcsine_input))*180)/3.14159265359;
	return arcsine_output;
}
double MNCTAspectReconstruction::cosine(double cosine_input){
	double cosine_output = cos((cosine_input * 3.14159265359)/180);
	return cosine_output;
}
double MNCTAspectReconstruction::arccosine(double arccosine_input){
	double arccosine_output = ((acos(arccosine_input))*180)/3.14159265359;
	return arccosine_output;
}
double MNCTAspectReconstruction::tangent(double tangent_input){
	double tangent_output = tan((tangent_input * 3.14159265359)/180);
	return tangent_output;
}
double MNCTAspectReconstruction::arctangent(double arctangent_input){
	double angle_in_degrees = ((atan(arctangent_input))*180)/3.14159265359;
	return angle_in_degrees;
}	
double MNCTAspectReconstruction::arctangent2(double y, double x){
	double angle_in_degrees = ((atan2(y,x))*180)/3.14159265359;
	return angle_in_degrees;
}

//This is the Spherical Vincenty Formula. It is used to compute the exact great circle distance (in degrees)
//between two points on a sphere if given the longitude and latitude of each.

double MNCTAspectReconstruction::Vincenty(double old_glat, double new_glat, double old_glon, double new_glon){
	double great_circle_distance = arctangent2(sqrt(pow(cosine(new_glat) * sine(abs(new_glon - old_glon)),2) + pow(cosine(old_glat) * sine(new_glat) - sine(old_glat) * cosine(new_glat) * cosine(abs(new_glon - old_glon)),2)),sine(old_glat)*sine(new_glat) + cosine(old_glat) * cosine(new_glat) * cosine(abs(new_glon - old_glon)));
	return great_circle_distance;
}

////////////////////////////////////////////////////////////////////////////////

bool MTimeSort( MNCTAspect* A1, MNCTAspect* A2 ){
	return A1->GetTime() < A2->GetTime();
}

//////////////////////////////////////////////////////////////////////////////

void MNCTAspectReconstruction::SetLastAspectInDeque(deque<MNCTAspect*> CurrentDeque){
	//Created for the housekeeping file.

	if (CurrentDeque.empty()) {
		LastAspectInDeque = 0;
	} else {
		LastAspectInDeque = CurrentDeque.back();
	}
}

// MNCTAspectReconstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
