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


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//Ares' 1st set of adjustments begin here (there are more later).


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

#include "Python.h"

using namespace std;


//Ares' 1st set of adjustments end here (there are more later).

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#include "MNCTAspectReconstruction.h"
#include "MNCTAspectPacket.h"
#include "magfld.h"



// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MString.h"
#include "MFile.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTAspectReconstruction)
#endif


	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////

	//Ares' 2nd set of adjustments begin here (there are more later).

	bool MTimeSort( MNCTAspect* A1, MNCTAspect* A2 );


MNCTAspectReconstruction::MNCTAspectReconstruction()
{
	// Construct an instance of MNCTAspectReconstruction
	Py_Initialize();
	Clear();
}


////////////////////////////////////////////////////////////////////////////////


MNCTAspectReconstruction::~MNCTAspectReconstruction()
{
	// Delete this instance of MNCTAspectReconstruction
	Py_Finalize();
}


//Ares' 2nd set of adjustments end here (there are more later).

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
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
	m_Aspects_Magnetometer.clear();	

}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



//Ares' 3rd (and last) set of adjustments begin here.



// Add and reconstruction one or more aspect frames - return false on error

bool MNCTAspectReconstruction::AddAspectFrame(MNCTAspectPacket PacketA)
{


	MNCTAspect* Aspect = new MNCTAspect;


	//Here we record the geographic longitude & latitude, as well as the height above sea
	//level (known as "elevation" in here). We also record the date and time as well as 
	//whether our data is from the GPS or magnetometer. 

	int GPS_or_magnetometer = PacketA.GPS_or_magnetometer;
	int test_or_not = PacketA.test_or_not;
	double geographic_longitude = PacketA.geographic_longitude;
	double geographic_latitude = PacketA.geographic_latitude;
	double elevation = PacketA.elevation; //Note "elevation" here is height above sea
	//level. This name was used to avoid confusion with pyephem (which defines
	//what it calls "elevation" that way). This "elevation" will eventually become the
	//"altitude" in the MNCTAspect object that will be created at the end of this program.

	//Below we look at the time data which should have been saved into packetA into 2
	//attributes: the string "date_and_time" and the unsigned int "nanoseconds." We use
	//these bits of info to build 7 separate unsigned ints of time information to be used
	//later when we finally build the MNCTAspect object


	string date_and_time = PacketA.date_and_time;
	unsigned int nanoseconds = PacketA.nanoseconds;

	MTime MTimeA;

	uint64_t ClkModulo = PacketA.CorrectedClk % 10000000;
	double ClkSeconds = (double) (PacketA.CorrectedClk - ClkModulo); ClkSeconds = ClkSeconds/10000000.;
	double ClkNanoseconds = (double) ClkModulo * 100.0;
	MTimeA.Set( ClkSeconds, ClkNanoseconds);
	double frac_Year = MTimeA.GetAsYears();
	float gcu_fracYear = frac_Year;


	//Here we record heading, pitch, and roll. It's a bit more complicated than you might think.

	double magnetic_heading = PacketA.heading;//Don't be alarmed that we are recording the 
	/*heading saved into PacketA (using Alex's code that parses data from the Frame) as
	  the double known as "magnetic_heading." In the case of the magnetometer, you see, the
	  "heading" measured by the magnetometer is actually the magnetic heading (or rather,
	  heading but with respect to magnetic north instead of true north). The difference
	  between what the magnetometer thinks is heading and what the true heading is, is thus
	  the difference between north and magnettic north, known as the "magnetic declination."
	  Thus our next step is to declare magnetic declination initially at 0, leave it alone
	  if we are using the GPS, or change it to what the magnetic declination actually is at
	  our current location if we are using the magnetometer. Then, we add the magnetic heading
	  and magnetic declination together to finally get the desired true heading.*/

	double magnetic_declination = 0.0; //This is the magnetic declination we were just discussing.


	if (GPS_or_magnetometer == 1){

		float lon = geographic_longitude;
		float lat = geographic_latitude;
		float alt = elevation;


		//magfld* MF = new magfld();
		void WMMInit();
		float magdec_Cplusplus1 = MagDec(lat, lon, alt, gcu_fracYear);
		magnetic_declination = magdec_Cplusplus1;


		if(test_or_not == 0){		
			printf("According to C++, magnetic_declination is: %9.5f \n",magdec_Cplusplus1);	
		}
	}

	//Now that we've gone through that crazy ordeal and have found the magnetic declination, we
	//add it to the magnetic heading to find and record the true heading, as promised:

	double heading = magnetic_heading + magnetic_declination;
	heading = 323.0;
	/*
		heading += 180.0;
		if( heading > 360.0 ){
		heading -= 360.0;
		}
	 */


	//Now, we record the pitch.	

	double pitch =  PacketA.pitch;
	if (GPS_or_magnetometer == 1){
		pitch = pitch - 90.0; //The magnetometer defines pitch with a 90 degree offset. That's
		//why we have no choice but to subtract by 90 in the event that the magnetometer is
		//being used.
	}
	pitch = 0;

	//Finally, we record the roll.

	double roll = PacketA.roll;
	roll = 0;


	//Here we print heading, pitch, and roll and announce which device is giving us the info. Please
	//note that even with the offsets above the heading, pitch and roll will not be the same for 
	//both devices because they are not aligned properly. This is taken into account later in the code.


	if(test_or_not == 0){

		cout << "" << endl;
		if (GPS_or_magnetometer == 0){
			cout << "The following is true according to the GPS: " << endl;
		}
		if (GPS_or_magnetometer == 1){
			cout << "The following is true according to the Magnetometer: " << endl;
		}
		cout << "" << endl;
		printf("heading is: %9.5f \n",heading);
		printf("pitch is: %9.5f \n",pitch);
		printf("roll is: %9.5f \n",roll);

	}

	////////////////////////////////////////////////////////////////////////////////



	//We'll leave this C++ version of finding the horizontal coordinates commented out for now, so other C++ versions of things written in python can be tested alone beforehand




	//Now we're going to be working with matrices. Here we declare all of the pointing vectors we will
	//use to find the azimuth and "altitude." We will also declare all of the appropriate rotation
	//matrices.



	double Zpointing0[3][1], Ypointing0[3][1], Xpointing0[3][1], Zpointing1[3][1], Ypointing1[3][1], Xpointing1[3][1], Zpointing2[3][1], Ypointing2[3][1], Xpointing2[3][1], Z[3][3], Y[3][3], X[3][3], YX[3][3], ZYX[3][3];

	//Here we build the heading, pitch, and roll rotation matrices, named "Z," "Y," and "X,"
	//respectively.

	Z[0][0] = cosine(heading);
	Z[0][1] = 0.0 - sine(heading);
	Z[0][2] = 0.0;

	Z[1][0] = sine(heading);
	Z[1][1] = cosine(heading);
	Z[1][2] = 0.0;

	Z[2][0] = 0.0;
	Z[2][1] = 0.0;
	Z[2][2] = 1.0;


	Y[0][0] = cosine(pitch);
	Y[0][1] = 0.0;
	Y[0][2] = sine(pitch);

	Y[1][0] = 0.0;
	Y[1][1] = 1.0;
	Y[1][2] = 0.0;

	Y[2][0] = 0.0 - sine(pitch);
	Y[2][1] = 0.0;
	Y[2][2] = cosine(pitch);


	X[0][0] = 1.0;
	X[0][1] = 0.0;
	X[0][2] = 0.0;

	X[1][0] = 0.0;
	X[1][1] = cosine(roll);
	X[1][2] = 0.0 - sine(roll);

	X[2][0] = 0.0;
	X[2][1] = sine(roll);
	X[2][2] = cosine(roll);

	//Here we multiply together the Z, Y, and X matrices to form the ZYX matrix. This matrix will later
	//be applied to the Z, Y, and X pointing vectors of the gondola to find the new pointing vector
	//after the heading, pitch, and roll rotations are performed.

	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){   
			YX[i][j]=0;
			for(int k=0;k<3;k++){
				YX[i][j]=YX[i][j]+Y[i][k]*X[k][j];
			}
		}
	}


	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){   
			ZYX[i][j]=0;
			for(int k=0;k<3;k++){
				ZYX[i][j]=ZYX[i][j]+Z[i][k]*YX[k][j];
			}
		}
	}

	//This is the specific case when we are dealing with the GPS.

	double Zhat[3];
	double Xhat[3];
	double Yhat[3];


	if (GPS_or_magnetometer == 0){

		Zhat[0] = 0.0;
		Zhat[1] = 0.0;
		Zhat[2] = 1.0;

		Xhat[0] = 1.0;
		Xhat[1] = 0.0;
		Xhat[2] = 0.0;

		Yhat[0] = 0.0;
		Yhat[1] = 1.0;
		Yhat[2] = 0.0;


		//transform Zhat
		for(int i=0;i<3;i++){
			double Q = 0;
			int k;
			for(k=0;k<3;k++){
				Q += Zhat[k] * ZYX[k][i];
			}
			Zhat[i] = Q;
		}

		//transform Yhat
		for(int i=0;i<3;i++){
			double Q = 0;
			int k;
			for(k=0;k<3;k++){
				Q += Yhat[k] * ZYX[k][i];
			}
			Yhat[i] = Q;
		}

		//transform Xhat
		for(int i=0;i<3;i++){
			double Q = 0;
			int k;
			for(k=0;k<3;k++){
				Q += Xhat[k] * ZYX[k][i];
			}
			Xhat[i] = Q;
		}

	}

	//This is the same thing as before except with the magnetometer.


	if (GPS_or_magnetometer == 1){

		Zpointing0[0][0] = 0.0;
		Zpointing0[1][0] = 0.0;
		Zpointing0[2][0] = 1.0;

		Ypointing0[0][0] = 0.0;
		Ypointing0[1][0] = 1.0;
		Ypointing0[2][0] = 0.0;	

		Xpointing0[0][0] = 1.0;
		Xpointing0[1][0] = 0.0;
		Xpointing0[2][0] = 0.0;


		for(int i=0;i<3;i++){
			for(int j=0;j<1;j++){   
				Zpointing1[i][j]=0;
				for(int k=0;k<3;k++){
					Zpointing1[i][j]=Zpointing1[i][j]+ZYX[i][k]*Zpointing0[k][j];
				}
			}
		}

		Zpointing2[0][0] = Zpointing1[1][0];
		Zpointing2[1][0] = Zpointing1[0][0];
		Zpointing2[2][0] = 0.0 - Zpointing1[2][0];		


		for(int i=0;i<3;i++){
			for(int j=0;j<1;j++){   
				Ypointing1[i][j]=0;
				for(int k=0;k<3;k++){
					Ypointing1[i][j]=Ypointing1[i][j]+ZYX[i][k]*Ypointing0[k][j];
				}
			}
		}

		Ypointing2[0][0] = Ypointing1[1][0];
		Ypointing2[1][0] = Ypointing1[0][0];
		Ypointing2[2][0] = 0.0 - Ypointing1[2][0];			


		for(int i=0;i<3;i++){
			for(int j=0;j<1;j++){   
				Xpointing1[i][j]=0;
				for(int k=0;k<3;k++){
					Xpointing1[i][j]=Xpointing1[i][j]+ZYX[i][k]*Xpointing0[k][j];
				}
			}
		}	

		Xpointing2[0][0] = Xpointing1[1][0];
		Xpointing2[1][0] = Xpointing1[0][0];
		Xpointing2[2][0] = 0.0 - Xpointing1[2][0];	

	}

	m_TCCalculator.SetLocation(geographic_latitude,geographic_longitude);

	m_TCCalculator.SetUnixTime(MTimeA.GetAsSeconds());

	vector<double> ZGalactic; 
	vector<double> ZEquatorial;

	//dot Zhat/Xhat into (0,0,1) and take the arcos to get the zenith angle.  then convert this to elevation angle
	double Z_Elevation = 90.0 - arccosine( Zhat[2] );
	double X_Elevation = 90.0 - arccosine( Xhat[2] );

	//get azimuth of Zhat/Xhat by projecting into X-Y plane, re-normalizing, and dotting into (0,1,0)
	double Z_proj[3]; Z_proj[0] = Zhat[0]; Z_proj[1] = Zhat[1]; Z_proj[2] = 0.0;
	double X_proj[3]; X_proj[0] = Xhat[0]; X_proj[1] = Xhat[1]; X_proj[2] = 0.0;

	double Znorm = sqrt( pow(Z_proj[0],2.0) + pow(Z_proj[1],2.0) );
	double Xnorm = sqrt( pow(X_proj[0],2.0) + pow(X_proj[1],2.0) );

	double Z_Azimuth = arccosine( Z_proj[1]/Znorm ); if( Z_proj[0] < 0.0 ) Z_Azimuth = 360.0 - Z_Azimuth;
	double X_Azimuth = arccosine( X_proj[1]/Xnorm ); if( X_proj[0] < 0.0 ) X_Azimuth = 360.0 - X_Azimuth;

	ZEquatorial = m_TCCalculator.MNCTTimeAndCoordinate::Horizon2Equatorial(Z_Azimuth, Z_Elevation);
	ZGalactic = m_TCCalculator.MNCTTimeAndCoordinate::Equatorial2Galactic(ZEquatorial);
	//Zra = ZEquatorial[0];
	//Zdec = ZEquatorial[1];
	double Zgalat = ZGalactic[1];
	double Zgalon = ZGalactic[0];
	cout<<"Z gal-lat = "<<Zgalat<<" Z gal-lon = "<<Zgalon<<endl;

	vector<double> XGalactic; 
	vector<double> XEquatorial;
	XEquatorial= m_TCCalculator.MNCTTimeAndCoordinate::Horizon2Equatorial(X_Azimuth, X_Elevation);
	XGalactic = m_TCCalculator.MNCTTimeAndCoordinate::Equatorial2Galactic(XEquatorial);	
	//Xra = XEquatorial[0];
	//Xdec = XEquatorial[1];
	double Xgalat = XGalactic[1];
	double Xgalon = XGalactic[0];
	cout<<"X gal-lat = "<<Xgalat<<" X gal-lon = "<<Xgalon<<endl;

	/*
	vector<double> YGalactic; 
	vector<double> YEquatorial;
	YEquatorial = m_TCCalculator.MNCTTimeAndCoordinate::Horizon2Equatorial(Yazimuth, Yaltitude);
	YGalactic = m_TCCalculator.MNCTTimeAndCoordinate::Equatorial2Galactic(YEquatorial);	
	Yra = YEquatorial[0];
	Ydec = YEquatorial[1];
	Ygalat = YGalactic[1];
	Ygalon = YGalactic[0];
	*/

	//MTimeA should already be filled out from earlier so the following paragraph is commented out;

	//MTime MTimeA;

	//Ares had this:
	//MTimeA.Set(m_Year,m_Month,m_Day,m_Hour,m_Minute,m_Second,m_NanoSecond);

	//Using this instead so that we can sort badsed on clock board time
	//MTimeA.Set(m_Year,m_Month,m_Day,m_Hour,m_Minute,m_Second,m_NanoSecond);



	Aspect->SetTime(MTimeA);
	Aspect->SetHeading(heading);
	Aspect->SetPitch(pitch);
	Aspect->SetRoll(roll);		
	Aspect->SetLatitude(geographic_latitude);
	Aspect->SetLongitude(geographic_longitude);
	Aspect->SetAltitude(elevation); // This is what we talked about. Our "elevation" 
	Aspect->SetGalacticPointingXAxis(Xgalon, Xgalat);
	Aspect->SetGalacticPointingZAxis(Zgalon, Zgalat);
	Aspect->SetHorizonPointingXAxis(X_Azimuth, X_Elevation);//Again here,
	Aspect->SetHorizonPointingZAxis(Z_Azimuth, Z_Elevation);//and here we see 

	if (GPS_or_magnetometer == 0){

		m_Aspects_GPS.push_back(Aspect);
		sort(m_Aspects_GPS.begin(), m_Aspects_GPS.end(), MTimeSort);
	}


	if (GPS_or_magnetometer == 1){

		m_Aspects_Magnetometer.push_back(Aspect);
		sort(m_Aspects_Magnetometer.begin(), m_Aspects_Magnetometer.end(), MTimeSort);

	}

	return true;
}




////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////// 


//Below we see various search functions useful for acquiring aspect information if given the time.


/*Okay, first off, we see the "GetAspect()" function. This function behaves like the GetAspectGPS()
  function (more on that function later) except that if it is unable to find good GPS data for a given
  time it will try to find good magnetometer data instead.*/

MNCTAspect* MNCTAspectReconstruction::GetAspect(MTime ReqTime, int GPS_Or_Magnetometer){

	MNCTAspect* ReqAspect = 0;

	if(GPS_Or_Magnetometer == 0){




		//check that there are aspect packets 
		if( m_Aspects_GPS.size() == 0 ){
			return 0;
		}

		//first need to check that event time is not older than the oldest Aspect we have
		//otherwise events won't get popped off of m_Events, since the first event will never get aspect info
		if( ReqTime < m_Aspects_GPS.front()->GetTime() ){
			//return an aspect that has a time of -1
			MNCTAspect* BadAspect = new MNCTAspect();
			BadAspect->SetTime( (double) -1.0 );
			return BadAspect;
		}

		//now check if event time is newer than the newest aspect ... if it is, then return null, we
		//need to wait for the newest aspect info to comein
		if( ReqTime > m_Aspects_GPS.back()->GetTime() ){
			return 0;
		}

		//this loop will only go if there are at least 2 aspects 
		for( int i = m_Aspects_GPS.size()-2; i > -1; --i ){
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

	if(GPS_Or_Magnetometer == 1){




		//check that there are aspect packets 
		if( m_Aspects_Magnetometer.size() == 0 ){
			return 0;
		}

		//first need to check that event time is not older than the oldest Aspect we have
		//otherwise events won't get popped off of m_Events, since the first event will never get aspect info
		if( ReqTime < m_Aspects_Magnetometer.front()->GetTime() ){
			//return an aspect that has a time of -1
			MNCTAspect* BadAspect = new MNCTAspect();
			BadAspect->SetTime( (double) -1.0 );
			return BadAspect;
		}

		//now check if event time is newer than the newest aspect ... if it is, then return null, we
		//need to wait for the newest aspect info to comein
		if( ReqTime > m_Aspects_Magnetometer.back()->GetTime() ){
			return 0;
		}

		//this loop will only go if there are at least 2 aspects 
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



	//if there is a flag for bad aspect, then still return the aspect.  the calling thread should check 
	//for this flag and set BD for the event

	return ReqAspect;

}

/*

	MNCTAspect* MNCTAspectReconstruction::GetAspect_ares(MTime Time){
	MNCTAspect* Desired_MNCTAspect = new MNCTAspect();
	MNCTAspect* Desired_Candidate_MNCTAspect = new MNCTAspect();
	uint64_t int_ClkSeconds = (PacketA.CorrectedClk/10000000) * 10000000;
	double ClkSeconds = (double) int_ClkSeconds;
	uint64_t int_ClkNanoseconds = PacketA.CorrectedClk % 10000000;
	double ClkNanoseconds = ((double) int_ClkNanoseconds) *100.0; //clock board ticks are in units of 100 ns

	MNCTAspect* Lower_MNCTAspect = new MNCTAspect();
	MNCTAspect* Upper_MNCTAspect = new MNCTAspect();
	Desired_MNCTAspect->SetGPS_Or_Magnetometer(2); 
	MTime time_after_last_data;
	MTime time_until_next_data;
	for(unsigned int i = 0; i < m_Aspects.size(); i++){
	if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 0 && m_Aspects[i]->GetTime() - Time < 0){
	time_after_last_data = Time - m_Aspects[i]->GetTime();
	Lower_MNCTAspect = m_Aspects[i];
	}
	else if (m_Aspects[i]->GetGPS_Or_Magnetometer() == 0 && m_Aspects[i]->GetTime() - Time >= 0){
	time_until_next_data = m_Aspects[i]->GetTime() - Time;
	Upper_MNCTAspect = m_Aspects[i];
	if(time_after_last_data < time_until_next_data){
	Desired_Candidate_MNCTAspect = Lower_MNCTAspect;
	if(Desired_Candidate_MNCTAspect->GetFlag() == 0){
	Desired_MNCTAspect = Desired_Candidate_MNCTAspect;
	}
	}
	else{
	Desired_Candidate_MNCTAspect = Upper_MNCTAspect;
	if(Desired_Candidate_MNCTAspect->GetFlag() == 0){
	Desired_MNCTAspect = Desired_Candidate_MNCTAspect;
	}
	}
	break;
	}	
	}
	if(Desired_MNCTAspect->GetGPS_Or_Magnetometer() == 2){
	MNCTAspect* Desired_MNCTAspect = new MNCTAspect();
	MNCTAspect* Desired_Candidate_MNCTAspect = new MNCTAspect();
	MNCTAspect* Lower_MNCTAspect = new MNCTAspect();
	MNCTAspect* Upper_MNCTAspect = new MNCTAspect();
	Desired_MNCTAspect->SetGPS_Or_Magnetometer(2); 
	MTime time_after_last_data;
	MTime time_until_next_data;
	for(unsigned int i = 0; i < m_Aspects.size(); i++){
	if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 1 && m_Aspects[i]->GetTime() - Time < 0){
	time_after_last_data = Time - m_Aspects[i]->GetTime();
	Lower_MNCTAspect = m_Aspects[i];
	}
	else if (m_Aspects[i]->GetGPS_Or_Magnetometer() == 1 && m_Aspects[i]->GetTime() - Time >= 0){
	time_until_next_data = m_Aspects[i]->GetTime() - Time;
	Upper_MNCTAspect = m_Aspects[i];
	if(time_after_last_data < time_until_next_data){
	Desired_Candidate_MNCTAspect = Lower_MNCTAspect;
	if(Desired_Candidate_MNCTAspect->GetFlag() == 0){
	Desired_MNCTAspect = Desired_Candidate_MNCTAspect;
	}
	}
	else{
	Desired_Candidate_MNCTAspect = Upper_MNCTAspect;
	if(Desired_Candidate_MNCTAspect->GetFlag() == 0){
	Desired_MNCTAspect = Desired_Candidate_MNCTAspect;
	}
	}
	break;
	}	
	}

	}
return Desired_MNCTAspect;
}

*/
//Okay, here's that GetAspectGPS() function we were talking about.


/*

	MNCTAspect* MNCTAspectReconstruction::GetAspectGPS(MTime Time){
	MNCTAspect* Desired_MNCTAspect = new MNCTAspect();
	MNCTAspect* Desired_Candidate_MNCTAspect = new MNCTAspect();
	MNCTAspect* Lower_MNCTAspect = new MNCTAspect();
	MNCTAspect* Upper_MNCTAspect = new MNCTAspect();
	Desired_MNCTAspect->SetGPS_Or_Magnetometer(2); 
	MTime time_after_last_data;
	MTime time_until_next_data;
	for(unsigned int i = 0; i < m_Aspects.size(); i++){
	if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 0 && m_Aspects[i]->GetTime() - Time < 0){
	time_after_last_data = Time - m_Aspects[i]->GetTime();
	Lower_MNCTAspect = m_Aspects[i];
	}
	else if (m_Aspects[i]->GetGPS_Or_Magnetometer() == 0 && m_Aspects[i]->GetTime() - Time >= 0){
	time_until_next_data = m_Aspects[i]->GetTime() - Time;
	Upper_MNCTAspect = m_Aspects[i];
	if(time_after_last_data < time_until_next_data){
	Desired_Candidate_MNCTAspect = Lower_MNCTAspect;
	if(Desired_Candidate_MNCTAspect->GetFlag() == 0){
	Desired_MNCTAspect = Desired_Candidate_MNCTAspect;
	}
	}
	else{
	Desired_Candidate_MNCTAspect = Upper_MNCTAspect;
	if(Desired_Candidate_MNCTAspect->GetFlag() == 0){
	Desired_MNCTAspect = Desired_Candidate_MNCTAspect;
	}
	}
	break;
	}	
	}
	return Desired_MNCTAspect;
	}

//Here's GetAspectMagnetometer(). It's just like its counterpart with the GPS.

MNCTAspect* MNCTAspectReconstruction::GetAspectMagnetometer(MTime Time){
MNCTAspect* Desired_MNCTAspect = new MNCTAspect();
MNCTAspect* Desired_Candidate_MNCTAspect = new MNCTAspect();
MNCTAspect* Lower_MNCTAspect = new MNCTAspect();
MNCTAspect* Upper_MNCTAspect = new MNCTAspect();
Desired_MNCTAspect->SetGPS_Or_Magnetometer(2); 
MTime time_after_last_data;
MTime time_until_next_data;
for(unsigned int i = 0; i < m_Aspects.size(); i++){
if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 1 && m_Aspects[i]->GetTime() - Time < 0){
time_after_last_data = Time - m_Aspects[i]->GetTime();
Lower_MNCTAspect = m_Aspects[i];
}
else if (m_Aspects[i]->GetGPS_Or_Magnetometer() == 1 && m_Aspects[i]->GetTime() - Time >= 0){
time_until_next_data = m_Aspects[i]->GetTime() - Time;
Upper_MNCTAspect = m_Aspects[i];
if(time_after_last_data < time_until_next_data){
Desired_Candidate_MNCTAspect = Lower_MNCTAspect;
if(Desired_Candidate_MNCTAspect->GetFlag() == 0){
Desired_MNCTAspect = Desired_Candidate_MNCTAspect;
}
}
else{
Desired_Candidate_MNCTAspect = Upper_MNCTAspect;
if(Desired_Candidate_MNCTAspect->GetFlag() == 0){
Desired_MNCTAspect = Desired_Candidate_MNCTAspect;
}
}
break;
}	
}
return Desired_MNCTAspect;
}

*/

/*This function, GetPreviousGPS(), was not created for the purposes of being used by people as it will 
  give you the previous GPS aspect data even it is bad data. The purpose of this function is to be used
  by the main chunk of this file to check and see if a piece of data should recieve a flag. Specifically,
  this function is used to retrieve old Aspect data to compare it to Aspect data being inspected for 
  flagging.*/

/*

	MNCTAspect* MNCTAspectReconstruction::GetPreviousGPS(MTime Time){
	MNCTAspect* Desired_MNCTAspect = new MNCTAspect();
	int Lower_Index = 0;
	int Upper_Index = 0;
	int Critical_Index = 0;
	Desired_MNCTAspect->SetGPS_Or_Magnetometer(2); 
	MTime time_after_last_data;
	MTime time_until_next_data;
	for (unsigned int i = 0; i < m_Aspects.size(); i++){
	if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 0 && m_Aspects[i]->GetTime() - Time < 0){
	time_after_last_data = Time - m_Aspects[i]->GetTime();
	Lower_Index = i;
	}
	else if (m_Aspects[i]->GetGPS_Or_Magnetometer() == 0 && m_Aspects[i]->GetTime() - Time >= 0){
	time_until_next_data = m_Aspects[i]->GetTime() - Time;
	Upper_Index = i;
	if(time_after_last_data < time_until_next_data){
	Critical_Index = Lower_Index;				
	}
	else{
	Critical_Index = Upper_Index;				
	}
	break;
	}	
	}  
	for (int i = Critical_Index - 1; i >= 0; i--){
	if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 0){
	Desired_MNCTAspect = m_Aspects[i];
	break;
	}
	}	
	return Desired_MNCTAspect;
	}

//This is the equivalent GetPrevious function, but for the magnetometer this time.

MNCTAspect* MNCTAspectReconstruction::GetPreviousMagnetometer(MTime Time){
MNCTAspect* Desired_MNCTAspect = new MNCTAspect();
int Lower_Index = 0;
int Upper_Index = 0;
int Critical_Index = 0;
Desired_MNCTAspect->SetGPS_Or_Magnetometer(2); 
MTime time_after_last_data;
MTime time_until_next_data;
for(unsigned int i = 0; i < m_Aspects.size(); i++){
if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 1 && m_Aspects[i]->GetTime() - Time < 0){
time_after_last_data = Time - m_Aspects[i]->GetTime();
Lower_Index = i;
}
else if (m_Aspects[i]->GetGPS_Or_Magnetometer() == 1 && m_Aspects[i]->GetTime() - Time >= 0){
time_until_next_data = m_Aspects[i]->GetTime() - Time;
Upper_Index = i;
if(time_after_last_data < time_until_next_data){
Critical_Index = Lower_Index;				
}
else{
Critical_Index = Upper_Index;				
}
break;
}	
}  
for (int i = Critical_Index - 1; i >= 0; i--){
if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 1){
Desired_MNCTAspect = m_Aspects[i];
break;
}
}	     
return Desired_MNCTAspect;
}



*/





////////////////////////////////////////////////////////////////////////////////
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

//Okay, okay, okay... Now we're really, really done. Thanks for bearing with these comments from the beginning to
//the end. Hopefully, they thoroughly explained how this code works.


//Ares' 3rd (and last) set of adjustments end here.

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool MTimeSort( MNCTAspect* A1, MNCTAspect* A2 ){
	return A1->GetTime() < A2->GetTime();
}


// MNCTAspectReconstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
