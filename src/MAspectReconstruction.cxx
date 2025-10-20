/*
 * MAspectReconstruction.cxx
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
// MAspectReconstruction
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MAspectReconstruction.h"

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
#include "MParser.h"
#include "MString.h"
#include "MFile.h"
#include "MAspectPacket.h"
#include "magfld.h"
#include "MRotation.h"
#include "MQuaternion.h"

////////////////////////////////////////////////////////////////////////////////

#ifdef ___CLING___
ClassImp(MAspectReconstruction)
#endif


////////////////////////////////////////////////////////////////////////////////

bool MTimeSort( MAspect* A1, MAspect* A2 );

////////////////////////////////////////////////////////////////////////////////

MAspectReconstruction::MAspectReconstruction()
{
	// Construct an instance of MAspectReconstruction
	Clear();
}

////////////////////////////////////////////////////////////////////////////////

MAspectReconstruction::~MAspectReconstruction()
{
	// Delete this instance of MAspectReconstruction
}

////////////////////////////////////////////////////////////////////////////////


void MAspectReconstruction::Clear()
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

bool MAspectReconstruction::Initialize()
{
	//Initialize the module
	return true;
}

////////////////////////////////////////////////////////////////////////////////

// Add and reconstruction one or more aspect frames - return false on error

bool MAspectReconstruction::AddAspectFrame(MAspectPacket PacketA)
{


	MAspect* Aspect = new MAspect;
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



	//definte the rotation matrices (https://en.wikipedia.org/wiki/Davenport_chained_rotations) to allow us to tranform from heading, pitch and roll, to the local GPS coordinates, and vise versa. The ADU5 defines the X direction to be along the vector created by Antennas 3, 2, and 4, therefore defines the pitch angle, and the Y direction is along the vector defined by antennas 1 and 2, and therefore defines the roll direction. These three rotations should be applied in order: Yaw, Pitch, Roll.


	//CK made a mistake defining these matricies...
	MRotation ROT_Z(cos(heading*c_Rad), -sin(heading*c_Rad), 0.0, sin(heading*c_Rad), cos(heading*c_Rad), 0.0, 0.0, 0.0, 1.0);
	MRotation ROT_Y(cos(roll*c_Rad), 0.0, sin(roll*c_Rad), 0.0, 1.0, 0.0, -sin(roll*c_Rad),  0.0, cos(roll*c_Rad));
	MRotation ROT_X(1.0, 0.0, 0.0, 0.0, cos(pitch*c_Rad), -sin(pitch*c_Rad), 0.0, sin(pitch*c_Rad), cos(pitch*c_Rad));
	MRotation ROT_XY = ROT_X*ROT_Y;
	MRotation ROT_ZXY = ROT_Z*ROT_XY;
	
	//Now, define the GPS pre-rotation matrix RotGPSCryo, which is the rotation matrix between the cryostat coorindates, with +x pointing towards the front of the gondola (sun side), and the GPS coordinates, with +y pointing towards the back of the gonodla. To first order, this is just a -90 degree rotation around the z-axis.
    //The magnetometer doesn't require this extra rotation matrix because its axes are already aligned with the cryostat
	MRotation RotGPSCryo;


	if( GPS_or_magnetometer == 0 ){

	//To second order, the GPS and cryostat are not perfectly aligned. Theodolite measurements were taken to better understand this offset, but have not yet been applied here.

		const double dh = 0.0;
		const double dp = 0.0;
		const double dr = 0.0;

		MRotation RotGPSCryo_z(cos( (-90.0 + dh)*c_Rad), -sin( (-90.0 + dh)*c_Rad), 0.0, sin( (-90.0 + dh)*c_Rad), cos( (-90.0 + dh)*c_Rad), 0.0, 0.0, 0.0, 1.0);
		MRotation RotGPSCryo_y(cos(dr*c_Rad), 0.0, sin(dr*c_Rad), 0.0, 1.0, 0.0, -sin(dr*c_Rad), 0.0, cos(dr*c_Rad));
		MRotation RotGPSCryo_x(1.0, 0.0, 0.0, 0.0, cos(dp*c_Rad), -sin(dp*c_Rad), 0.0, sin(dp*c_Rad), cos(dp*c_Rad));
		RotGPSCryo = RotGPSCryo_z*RotGPSCryo_x*RotGPSCryo_y;

	}



	//Full Rotation includes the GPS pre-rotation and the GPS Rotation matrix defined above
	MRotation FullRot = ROT_ZXY*RotGPSCryo;

	
	//Calculate the Elevation
	double Z_Elevation = asin(FullRot.GetZZ())*c_Deg;
	double Y_Elevation = asin(FullRot.GetYZ())*c_Deg;
	double X_Elevation = asin(FullRot.GetXZ())*c_Deg;


	//Calculate the Azimuth, as measured clockwise from North, by projecting into X-Y plane and re-normalizing
	double Z_Azimuth, X_Azimuth, Y_Azimuth;
	MVector X_proj(FullRot.GetXX(), FullRot.GetXY(), 0);
	X_proj = X_proj.Unitize();
	MVector Y_proj(FullRot.GetYX(), FullRot.GetYY(), 0);
	Y_proj = Y_proj.Unitize();
	MVector Z_proj(FullRot.GetZX(), FullRot.GetZY(), 0);
	Z_proj = Z_proj.Unitize();
       

	//Azimuth calculation for GPS:
	if( GPS_or_magnetometer == 0 ){

		//for GPS, the azimuth angle is the angle between the projected vector and the y axis		
		X_Azimuth = acos(X_proj.GetY())*c_Deg;
		if (X_proj.GetX() < 0.0) X_Azimuth = 360.0 - X_Azimuth;

		Y_Azimuth = acos(Y_proj.GetY())*c_Deg;        
		if (Y_proj.GetX() < 0.0) Y_Azimuth = 360.0 - Y_Azimuth;

		Z_Azimuth = acos(Z_proj.GetY())*c_Deg;
		if (Z_proj.GetX() < 0.0) Z_Azimuth = 360.0 - Z_Azimuth;

	} else {
		//Azimuth calculation for Magnetometer:

		//for magnetometer, the azimuth angle is the angle between the projected vector and the x axis since the magnetometer reference frame has the X-axis pointing at true North
		X_Azimuth = acos(X_proj.GetX())*c_Deg;
		if (X_proj.GetY() > 0.0) X_Azimuth = 360.0 - X_Azimuth;

		Y_Azimuth = acos(Y_proj.GetX())*c_Deg;        
		if (Y_proj.GetY() > 0.0) Y_Azimuth = 360.0 - Y_Azimuth;

		Z_Azimuth = acos(Z_proj.GetX())*c_Deg;
		if (Z_proj.GetY() > 0.0) Z_Azimuth = 360.0 - Z_Azimuth;

	}

	
	if (g_Verbosity >= c_Info) {
		cout<<"X_Azimuth = "<<X_Azimuth<<" X_Elevation = "<<X_Elevation<<endl;
		cout<<"Y_Azimuth = "<<Y_Azimuth<<" Y_Elevation = "<<Y_Elevation<<endl;
		cout<<"Z_Azimuth = "<<Z_Azimuth<<" Z_Elevation = "<<Z_Elevation<<endl;
	}


	//Define appropriate paramters for the TimeAndCoordinate Calculatr:
	m_TCCalculator.SetLocation(geographic_latitude,geographic_longitude);
	m_TCCalculator.SetUnixTime(UTCTime.GetAsSystemSeconds()); //AWL use the UnixTimeFromGPSTime


	//Convert to Equatorial and Galactic
	vector<double> ZGalactic; 
	vector<double> ZEquatorial;
	ZEquatorial = m_TCCalculator.MTimeAndCoordinate::Horizon2Equatorial(Z_Azimuth, Z_Elevation);
	ZGalactic = m_TCCalculator.MTimeAndCoordinate::Equatorial2Galactic2(ZEquatorial);
	//Zra = ZEquatorial[0];
	//Zdec = ZEquatorial[1];
	double Zgalat = ZGalactic[1];
	double Zgalon = ZGalactic[0];
	if (g_Verbosity >= c_Info) cout<<"Z gal-lat = "<<Zgalat<<" Z gal-lon = "<<Zgalon<<endl;

	vector<double> YGalactic; 
	vector<double> YEquatorial;
	YEquatorial = m_TCCalculator.MTimeAndCoordinate::Horizon2Equatorial(Y_Azimuth, Y_Elevation);
	YGalactic = m_TCCalculator.MTimeAndCoordinate::Equatorial2Galactic2(YEquatorial);	
	//Yra = YEquatorial[0];
	//Ydec = YEquatorial[1];
	double Ygalat = YGalactic[1];
	double Ygalon = YGalactic[0];
	if (g_Verbosity >= c_Info) cout<<"Y gal-lat = "<<Ygalat<<" Y gal-lon = "<<Ygalon<<endl;

	vector<double> XGalactic; 
	vector<double> XEquatorial;
	XEquatorial = m_TCCalculator.MTimeAndCoordinate::Horizon2Equatorial(X_Azimuth, X_Elevation);
	XGalactic = m_TCCalculator.MTimeAndCoordinate::Equatorial2Galactic2(XEquatorial);
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

	if (GPS_or_magnetometer == 0) {

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

MAspect* MAspectReconstruction::GetAspect(MTime ReqTime, int GPS_Or_Magnetometer){

	MAspect* ReqAspect = 0;

	//Get Correct GPS packet for GPS and Interpolation
	if(GPS_Or_Magnetometer == 0 || GPS_Or_Magnetometer == 2){
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
					//If Interpolation...
					if (GPS_Or_Magnetometer == 2) {
						ReqAspect = InterpolateAspect(ReqTime, m_Aspects_GPS[i], m_Aspects_GPS[i+1]);
						break;
					} else {
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


//////////////////////////////////////////////////////////////////////////////

MAspect * MAspectReconstruction::InterpolateAspect(MTime ReqTime, MAspect * BeforeAspect, MAspect * AfterAspect)
{

		//BeforeAspect and AfterAspect are the two aspect packets that surround the time of the event

		//Get Absolute Time:
		double time_asdouble = BeforeAspect->GetUTCTime().GetAsDouble() + ( ReqTime.GetAsDouble() - BeforeAspect->GetTime().GetAsDouble());
		
		//Copy the BeforeAspect information. We'll use the same longitude, latitude, and PPS etc. for this event.
		MAspect* ReqAspect = BeforeAspect;

		//Initilize varibles
		map<int, double[3]> GPSPointing; //Heading, Pitch, Roll
		MTimeAndCoordinate m_TCCalculator;
		
		GPSPointing[0][0] = BeforeAspect->GetHeading();
		GPSPointing[0][1] = BeforeAspect->GetPitch();
		GPSPointing[0][2] = BeforeAspect->GetRoll();

                GPSPointing[1][0] = AfterAspect->GetHeading();
                GPSPointing[1][1] = AfterAspect->GetPitch();
                GPSPointing[1][2] = AfterAspect->GetRoll();

		//Define GPS Rotation Matrices
		MRotation RotGPSCryo(cos( (-90)*c_Rad), -sin( (-90)*c_Rad), 0.0, sin( (-90)*c_Rad), cos( (-90)*c_Rad), 0.0, 0.0, 0.0, 1.0);

		MRotation beforeRot_z(cos(GPSPointing[0][0]*c_Rad), -sin(GPSPointing[0][0]*c_Rad), 0.0, sin(GPSPointing[0][0]*c_Rad), cos(GPSPointing[0][0]*c_Rad), 0.0, 0.0, 0.0, 1.0);
		MRotation beforeRot_y(cos(GPSPointing[0][2]*c_Rad), 0.0, sin(GPSPointing[0][2]*c_Rad), 0.0, 1.0, 0.0, -sin(GPSPointing[0][2]*c_Rad),  0.0, cos(GPSPointing[0][2]*c_Rad));
		MRotation beforeRot_x(1.0, 0.0, 0.0, 0.0, cos(GPSPointing[0][1]*c_Rad), -sin(GPSPointing[0][1]*c_Rad), 0.0, sin(GPSPointing[0][1]*c_Rad), cos(GPSPointing[0][1]*c_Rad));
		MRotation beforeRot_xy = beforeRot_x*beforeRot_y;
		MRotation beforeRot = beforeRot_z*beforeRot_xy;
		beforeRot = beforeRot*RotGPSCryo;


		MRotation afterRot_z(cos(GPSPointing[1][0]*c_Rad), -sin(GPSPointing[1][0]*c_Rad), 0.0, sin(GPSPointing[1][0]*c_Rad), cos(GPSPointing[1][0]*c_Rad), 0.0, 0.0, 0.0, 1.0);
		MRotation afterRot_y(cos(GPSPointing[1][2]*c_Rad), 0.0, sin(GPSPointing[1][2]*c_Rad), 0.0, 1.0, 0.0, -sin(GPSPointing[1][2]*c_Rad),  0.0, cos(GPSPointing[1][2]*c_Rad));
		MRotation afterRot_x(1.0, 0.0, 0.0, 0.0, cos(GPSPointing[1][1]*c_Rad), -sin(GPSPointing[1][1]*c_Rad), 0.0, sin(GPSPointing[1][1]*c_Rad), cos(GPSPointing[1][1]*c_Rad));
		MRotation afterRot_xy = afterRot_x*afterRot_y;
		MRotation afterRot = afterRot_z*afterRot_xy;
		afterRot = afterRot*RotGPSCryo;

		//Convert to Quaternion for interpolation, then assign new rotation matrix to event
		MQuaternion qbefore(beforeRot);
		qbefore = qbefore.GetUnitQuaternion();

		MQuaternion qafter(afterRot);
		qafter = qafter.GetUnitQuaternion();



		//Interpolate between Quaternions:
		//Caluclate the fraction of time between the two aspect packets for interpolation. Should be between 0 and 1.
		double fact = (ReqTime.GetAsDouble() - BeforeAspect->GetTime().GetAsDouble())/(AfterAspect->GetTime().GetAsDouble() - BeforeAspect->GetTime().GetAsDouble());
		MQuaternion qinter;
		qinter = qinter.GetSlerp(qbefore,qafter,fact);

		//Convert back to Rotation Matrix
		MRotation interRot = qinter.GetRotation();


		//Define new Elevation angles
		double Z_Elevation = asin(interRot.GetZZ())*c_Deg;
		//double Y_Elevation = asin(interRot.GetYZ())*c_Deg;
		double X_Elevation = asin(interRot.GetXZ())*c_Deg;

		//Define new Azimuth angles
		double Z_Azimuth,X_Azimuth,Y_Azimuth;
		MVector X_proj(interRot.GetXX(), interRot.GetXY(), 0);
		X_proj = X_proj.Unitize();
		X_Azimuth = acos(X_proj.GetY())*c_Deg;
		if (X_proj.GetX() < 0.0) X_Azimuth = 360.0 - X_Azimuth;

		MVector Y_proj(interRot.GetYX(), interRot.GetYY(), 0);
		Y_proj = Y_proj.Unitize();
		Y_Azimuth = acos(Y_proj.GetY())*c_Deg;
		if (Y_proj.GetX() < 0.0) Y_Azimuth = 360.0 - Y_Azimuth;

		MVector Z_proj(interRot.GetZX(), interRot.GetZY(), 0);
		Z_proj = Z_proj.Unitize();
		Z_Azimuth = acos(Z_proj.GetY())*c_Deg;
		if (Z_proj.GetX() < 0.0) Z_Azimuth = 360.0 - Z_Azimuth;

	
		//Add the latitude and longitude and time to the TCCalculator
		m_TCCalculator.SetLocation(ReqAspect->GetLatitude(), ReqAspect->GetLongitude());
		m_TCCalculator.SetUnixTime(time_asdouble);


		vector<double> ZGalactic;
		vector<double> ZEquatorial;
		ZEquatorial = m_TCCalculator.MTimeAndCoordinate::Horizon2Equatorial(Z_Azimuth, Z_Elevation);
		ZGalactic = m_TCCalculator.MTimeAndCoordinate::Equatorial2Galactic2(ZEquatorial);
		//Zra = ZEquatorial[0];
		//Zdec = ZEquatorial[1];
		double Zgalat = ZGalactic[1];
		double Zgalon = ZGalactic[0];
		//cout<<"Z gal-lon = "<<Zgalon<<" Z gal-lat = "<<Zgalat<<endl;

		vector<double> XGalactic;
		vector<double> XEquatorial;
		XEquatorial = m_TCCalculator.MTimeAndCoordinate::Horizon2Equatorial(X_Azimuth, X_Elevation);
		XGalactic = m_TCCalculator.MTimeAndCoordinate::Equatorial2Galactic2(XEquatorial);
		//Xra = XEquatorial[0];
		//Xdec = XEquatorial[1];
		double Xgalat = XGalactic[1];
		double Xgalon = XGalactic[0];

		//Redefine the Galactic and Horizon Pointing with the interpolated values
		ReqAspect->SetGalacticPointingXAxis(Xgalon, Xgalat);
		ReqAspect->SetGalacticPointingZAxis(Zgalon, Zgalat);
		ReqAspect->SetHorizonPointingXAxis(X_Azimuth, X_Elevation);
		ReqAspect->SetHorizonPointingZAxis(Z_Azimuth, Z_Elevation);
		ReqAspect->SetGPS_or_magnetometer(0);

	return ReqAspect;

}

////////////////////////////////////////////////////////////////////////////////

//This is the Spherical Vincenty Formula. It is used to compute the exact great circle distance (in degrees)
//between two points on a sphere if given the longitude and latitude of each.
/*
double MAspectReconstruction::Vincenty(double old_glat, double new_glat, double old_glon, double new_glon){
	double great_circle_distance = arctangent2(sqrt(pow(cosine(new_glat) * sine(abs(new_glon - old_glon)),2) + pow(cosine(old_glat) * sine(new_glat) - sine(old_glat) * cosine(new_glat) * cosine(abs(new_glon - old_glon)),2)),sine(old_glat)*sine(new_glat) + cosine(old_glat) * cosine(new_glat) * cosine(abs(new_glon - old_glon)));
	return great_circle_distance;
}
*/
////////////////////////////////////////////////////////////////////////////////

bool MTimeSort( MAspect* A1, MAspect* A2 ){
	return A1->GetTime() < A2->GetTime();
}

//////////////////////////////////////////////////////////////////////////////

void MAspectReconstruction::SetLastAspectInDeque(deque<MAspect*> CurrentDeque){
	//Created for the housekeeping file.

	if (CurrentDeque.empty()) {
		LastAspectInDeque = 0;
	} else {
		LastAspectInDeque = CurrentDeque.back();
	}
}

// MAspectReconstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

