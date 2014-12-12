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

	//Finally, we record the roll.

	double roll = PacketA.roll;


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


	if (GPS_or_magnetometer == 0){


	//Here we start by building the initial Z, Y, and X pointing vectors of the gondola (in the GPS's
	//coordinate system) as they are before rotation.

		Zpointing0[0][0] = 0.0;
		Zpointing0[1][0] = 0.0;
		Zpointing0[2][0] = -1.0;
		
		Ypointing0[0][0] = 0.0;
		//Ypointing0[1][0] = 1.0;
		Ypointing0[1][0] = -1.0;
		Ypointing0[2][0] = 0.0;	
		
		//Xpointing0[0][0] = -1.0;
		Xpointing0[0][0] = 1.0;
		Xpointing0[1][0] = 0.0;
		Xpointing0[2][0] = 0.0;

		//Here we rotate the pointing vectors and then convert them from GPS coordinates to a coordinate
		//system that makes calculating the azimuth and "altitude" the easiest.

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

	//Here we 
	

if(test_or_not == 0){

	cout << "" << endl;
	
	cout << "Zpointing1 is: ";
	cout << "[[";
	cout << Zpointing1[0][0];
	cout << "], [";
	cout << Zpointing1[1][0];
	cout << "], [";
	cout << Zpointing1[2][0];	
	cout << "]]" << endl;
	cout << "Zpointing2 is: ";
	cout << "[[";
	cout << Zpointing2[0][0];
	cout << "], [";
	cout << Zpointing2[1][0];
	cout << "], [";
	cout << Zpointing2[2][0];	
	cout << "]]" << endl;
	
	cout << "Ypointing1 is: ";
	cout << "[[";
	cout << Ypointing1[0][0];
	cout << "], [";
	cout << Ypointing1[1][0];
	cout << "], [";
	cout << Ypointing1[2][0];	
	cout << "]]" << endl;
	cout << "Ypointing2 is: ";
	cout << "[[";
	cout << Ypointing2[0][0];
	cout << "], [";
	cout << Ypointing2[1][0];
	cout << "], [";
	cout << Ypointing2[2][0];	
	cout << "]]" << endl;
	
	cout << "Xpointing1 is: ";
	cout << "[[";
	cout << Xpointing1[0][0];
	cout << "], [";
	cout << Xpointing1[1][0];
	cout << "], [";
	cout << Xpointing1[2][0];	
	cout << "]]" << endl;
	cout << "Xpointing2 is: ";
	cout << "[[";
	cout << Xpointing2[0][0];
	cout << "], [";
	cout << Xpointing2[1][0];
	cout << "], [";
	cout << Xpointing2[2][0];	
	cout << "]]" << endl;
	cout << "" << endl;
	
}

	//Below we declare some doubles we are about to use.
	
	double a = 0.0;
	double c = 0.0;
	double b = 0.0;
	
	double Zhypot = 0.0;
	double Yhypot = 0.0;
	double Xhypot = 0.0;
	
	double Zazimuth = 0.0;
	double Yazimuth = 0.0;
	double Xazimuth = 0.0;
	
	double Zaltitude = 0.0;
	double Yaltitude = 0.0;
	double Xaltitude = 0.0; 
	
	//Below we determine the azimuth for the Zpointing2 vector based off of knowing what its x and y 
	//components are.
	
	
	if(Zpointing2[0][0] == 0.0 && Zpointing2[1][0] > 0.0){
		Zazimuth = 90.0;
	}
	else if(Zpointing2[0][0] < 0.0 && Zpointing2[1][0] == 0.0){
		Zazimuth = 180.0;
	}
	else if(Zpointing2[0][0] == 0.0 && Zpointing2[1][0] < 0.0){
		Zazimuth = 270.0;
	}
	else if(Zpointing2[0][0] > 0.0 && Zpointing2[1][0] == 0.0){
		Zazimuth = 0.0;
	}	
	else if(Zpointing2[0][0] == 0.0 && Zpointing2[1][0] == 0.0){
		Zazimuth = 0.0;
	}
	
	
	else{
		a = arctangent(abs(Zpointing2[1][0]/Zpointing2[0][0]));
		if(Zpointing2[0][0] >= 0.0 && Zpointing2[1][0] >= 0.0){
			Zazimuth = 90.0 - a;
		}
		if(Zpointing2[0][0] >= 0.0 && Zpointing2[1][0] <= 0.0){
			Zazimuth = 90.0 + a;
		}		
		if(Zpointing2[0][0] <= 0.0 && Zpointing2[1][0] <= 0.0){
			Zazimuth = 270.0 - a;
		}		
		if(Zpointing2[0][0] <= 0.0 && Zpointing2[1][0] >= 0.0){
			Zazimuth = 270.0 + a;
		}				
	}
	
	//Below we determine the azimuth for the Ypointing2 vector based off of knowing what its x and y 
	//components are.	
	
		
	if(Ypointing2[0][0] == 0.0 && Ypointing2[1][0] > 0.0){
		Yazimuth = 90.0;
	}
	else if(Ypointing2[0][0] < 0.0 && Ypointing2[1][0] == 0.0){
		Yazimuth = 180.0;
	}
	else if(Ypointing2[0][0] == 0.0 && Ypointing2[1][0] < 0.0){
		Yazimuth = 270.0;
	}
	else if(Ypointing2[0][0] > 0.0 && Ypointing2[1][0] == 0.0){
		Yazimuth = 0.0;
	}	
	else if(Ypointing2[0][0] == 0.0 && Ypointing2[1][0] == 0.0){
		Yazimuth = 0.0;
	}
	
	
	else{
		c = arctangent(abs(Ypointing2[1][0]/Ypointing2[0][0]));
		if(Ypointing2[0][0] >= 0.0 && Ypointing2[1][0] >= 0.0){
			Yazimuth = 90.0 - c;
		}
		if(Ypointing2[0][0] >= 0.0 && Ypointing2[1][0] <= 0.0){
			Yazimuth = 90.0 + c;
		}		
		if(Ypointing2[0][0] <= 0.0 && Ypointing2[1][0] <= 0.0){
			Yazimuth = 270.0 - c;
		}		
		if(Ypointing2[0][0] <= 0.0 && Ypointing2[1][0] >= 0.0){
			Yazimuth = 270.0 + c;
		}				
	}	
		
	//Below we determine the azimuth for the Xpointing2 vector based off of knowing what its x and y 
	//components are.	
	

		
	if(Xpointing2[0][0] == 0.0 && Xpointing2[1][0] > 0.0){
		Xazimuth = 90.0;
	}
	else if(Xpointing2[0][0] < 0.0 && Xpointing2[1][0] == 0.0){
		Xazimuth = 180.0;
	}
	else if(Xpointing2[0][0] == 0.0 && Xpointing2[1][0] < 0.0){
		Xazimuth = 270.0;
	}
	else if(Xpointing2[0][0] > 0.0 && Xpointing2[1][0] == 0.0){
		Xazimuth = 0.0;
	}	
	else if(Xpointing2[0][0] == 0.0 && Xpointing2[1][0] == 0.0){
		Xazimuth = 0.0;
	}
	
	
	else{
		b = arctangent(abs(Xpointing2[1][0]/Xpointing2[0][0]));
		if(Xpointing2[0][0] >= 0.0 && Xpointing2[1][0] >= 0.0){
			Xazimuth = 90.0 - b;
		}
		if(Xpointing2[0][0] >= 0.0 && Xpointing2[1][0] <= 0.0){
			Xazimuth = 90.0 + b;
		}		
		if(Xpointing2[0][0] <= 0.0 && Xpointing2[1][0] <= 0.0){
			Xazimuth = 270.0 - b;
		}		
		if(Xpointing2[0][0] <= 0.0 && Xpointing2[1][0] >= 0.0){
			Xazimuth = 270.0 + b;
		}				
	}	



	Zhypot = hypot(Zpointing2[0][0],Zpointing2[1][0]);
	if (Zhypot == 0.0){
		Zaltitude = 90.0;
	}	
	else{
		Zaltitude = arctangent(Zpointing2[2][0]/Zhypot);
	}	
		
		
	Yhypot = hypot(Ypointing2[0][0],Ypointing2[1][0]);
	if (Yhypot == 0.0){
		Yaltitude = 90.0;
	}	
	else{
		Yaltitude = arctangent(Ypointing2[2][0]/Yhypot);
	}	
		
		
	Xhypot = hypot(Xpointing2[0][0],Xpointing2[1][0]);
	if (Xhypot == 0.0){
		Xaltitude = 90.0;
	}	
	else{
		Xaltitude = arctangent(Xpointing2[2][0]/Xhypot);
	}	


if(test_or_not == 0){

	//This here next, is a whole bunch of printing. We print the azimuth, "altitude," longitude, 
	//latitude, and "elevation." We also print the date and time(as promised!).

	printf("Zazimuth  is: %12.10f \n",Zazimuth);
	printf("Zaltitude is: %12.10f \n",Zaltitude);//Remember that "altitude" is degrees above 
	//the horizon according to pyephem, and that's the naming convention we use here.
	printf("Yazimuth  is: %12.10f \n",Yazimuth);
	printf("Yaltitude is: %12.10f \n",Yaltitude);//Remember that "altitude" is degrees above 
	//the horizon according to pyephem, and that's the naming convention we use here.
	printf("Xazimuth  is: %12.10f \n",Xazimuth);
	printf("Xaltitude is: %12.10f \n",Xaltitude);//Remember that "altitude" is degrees above 
	//the horizon according to pyephem, and that's the naming convention we use here.
	printf("longitude is: %12.7f \n",geographic_longitude);
	printf("latitude is: %12.7f \n",geographic_latitude);
	printf("elevation is: %12.7f \n",elevation); //Remember that "elevation" is actually 
	//height above sea level in meters. Again, this is what pyephem calls it.
	cout << "date and time are: ";
	cout << date_and_time << endl;	

}
	
	////////////////////////////////////////////////////////////////////////////
	
	double Zra = 0.0;
	double Zdec = 0.0;
	double Zgalat = 0.0;
	double Zgalon =0.0;
	double Yra = 0.0;
	double Ydec = 0.0;
	double Ygalat = 0.0;
	double Ygalon =0.0;
	double Xra = 0.0;
	double Xdec = 0.0;
	double Xgalat = 0.0;
	double Xgalon =0.0;
	
	
	////////////////////////////////////////////////////////////////////////////



	m_TCCalculator.SetLocation(geographic_latitude,geographic_longitude);
	

	if(test_or_not == 0){
		cout << MTimeA.GetAsSeconds() << endl;
		printf("MTimeA is also: %9.5f \n",MTimeA.GetAsSeconds());
	}
	
	
	m_TCCalculator.SetUnixTime(MTimeA.GetAsSeconds());
	
	
	vector<double> ZGalactic; 
	vector<double> ZEquatorial;
	ZEquatorial = m_TCCalculator.MNCTTimeAndCoordinate::Horizon2Equatorial(Zazimuth, Zaltitude);
	ZGalactic = m_TCCalculator.MNCTTimeAndCoordinate::Equatorial2Galactic(ZEquatorial);
	Zra = ZEquatorial[0];
	Zdec = ZEquatorial[1];
	Zgalat = ZGalactic[1];
	Zgalon = ZGalactic[0];


	if(test_or_not == 0){

		cout << "" << endl;
		cout << "According to C+++++..." << endl;
	
		cout << "" << endl;
		cout << "Zdeclination is: ";
		cout << Zdec;
		cout << " degrees North" << endl;
		cout << "Zright ascension is: ";
		cout << Zra;
		cout << " degrees East" << endl;

		cout << "" << endl;
		cout << "Zgalactic latitude is: ";
		cout << Zgalat;
		cout << " degrees North" << endl;
		cout << "Zgalactic longitude is: ";
		cout << Zgalon;
		cout << " degrees East" << endl;
	}


	vector<double> YGalactic; 
	vector<double> YEquatorial;
	YEquatorial = m_TCCalculator.MNCTTimeAndCoordinate::Horizon2Equatorial(Yazimuth, Yaltitude);
	YGalactic = m_TCCalculator.MNCTTimeAndCoordinate::Equatorial2Galactic(YEquatorial);	
	Yra = YEquatorial[0];
	Ydec = YEquatorial[1];
	Ygalat = YGalactic[1];
	Ygalon = YGalactic[0];
	
if(test_or_not == 0){	
	
	
	cout << "" << endl;
	cout << "Ydeclination is: ";
	cout << Ydec;
	cout << " degrees North" << endl;
	cout << "Yright ascension is: ";
	cout << Yra;
	cout << " degrees East" << endl;
	
	cout << "" << endl;
	cout << "Ygalactic latitude is: ";
	cout << Ygalat;
	cout << " degrees North" << endl;
	cout << "Ygalactic longitude is: ";
	cout << Ygalon;
	cout << " degrees East" << endl;
	
}	
	
	
	vector<double> XGalactic; 
	vector<double> XEquatorial;
	XEquatorial= m_TCCalculator.MNCTTimeAndCoordinate::Horizon2Equatorial(Xazimuth, Xaltitude);
	XGalactic = m_TCCalculator.MNCTTimeAndCoordinate::Equatorial2Galactic(XEquatorial);	
	Xra = XEquatorial[0];
	Xdec = XEquatorial[1];
	Xgalat = XGalactic[1];
	Xgalon = XGalactic[0];
	
if(test_or_not == 0){
		
	
	cout << "" << endl;
	cout << "Xdeclination is: ";
	cout << Xdec;
	cout << " degrees North" << endl;
	cout << "Xright ascension is: ";
	cout << Xra;
	cout << " degrees East" << endl;
	
	cout << "" << endl;
	cout << "Xgalactic latitude is: ";
	cout << Xgalat;
	cout << " degrees North" << endl;
	cout << "Xgalactic longitude is: ";
	cout << Xgalon;
	cout << " degrees East" << endl;
	
}
	


	

	
////////////////////////////////////////////////////////////////////////////

	
	
		


	
	
	
	
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
	//here is saved as "altitude" in the MNCTAspect object.
	Aspect->SetGalacticPointingXAxis(Xgalon, Xgalat);
	Aspect->SetGalacticPointingZAxis(Zgalon, Zgalat);
	Aspect->SetHorizonPointingXAxis(Xazimuth, Xaltitude);//Again here,
	Aspect->SetHorizonPointingZAxis(Zazimuth, Zaltitude);//and here we see 
	//what we talked about. Our "altitudes" are saved as "elevations" in the 	
	//MNCTAspect object.

	//Okay, we're good. Our MNCTAspect object has basically all we need, so now we add it to our m_Aspects_GPS or m_Aspects_Magnetomter
	//vector and we're done, right? Yeah, we wish...


if (GPS_or_magnetometer == 0){

	m_Aspects_GPS.push_back(Aspect);
		//So, we still need to sort our vector after we add something to it:


	//Ares had this, which leaks memory:
	/*
	struct less_than_m_Time{
		inline bool operator() (MNCTAspect* MNCTAspect1 = new MNCTAspect, MNCTAspect* MNCTAspect2 = new MNCTAspect){
			return (MNCTAspect1->GetTime() < MNCTAspect2->GetTime());
	uint64_t int_ClkSeconds = (PacketA.CorrectedClk/10000000) * 10000000;
	double ClkSeconds = (double) int_ClkSeconds;
	uint64_t int_ClkNanoseconds = PacketA.CorrectedClk % 10000000;
	double ClkNanoseconds = ((double) int_ClkNanoseconds) *100.0; //clock board ticks are in units of 100 ns

		}
	};
	*/

	//i redefined the sort function at the bottom of the file

	sort(m_Aspects_GPS.begin(), m_Aspects_GPS.end(), MTimeSort);
}


if (GPS_or_magnetometer == 1){

	m_Aspects_Magnetometer.push_back(Aspect);
		//So, we still need to sort our vector after we add something to it:


	//Ares had this, which leaks memory:
	/*
	struct less_than_m_Time{
		inline bool operator() (MNCTAspect* MNCTAspect1 = new MNCTAspect, MNCTAspect* MNCTAspect2 = new MNCTAspect){
			return (MNCTAspect1->GetTime() < MNCTAspect2->GetTime());
	uint64_t int_ClkSeconds = (PacketA.CorrectedClk/10000000) * 10000000;
	double ClkSeconds = (double) int_ClkSeconds;
	uint64_t int_ClkNanoseconds = PacketA.CorrectedClk % 10000000;
	double ClkNanoseconds = ((double) int_ClkNanoseconds) *100.0; //clock board ticks are in units of 100 ns

		}
	};
	*/

	//i redefined the sort function at the bottom of the file

	sort(m_Aspects_Magnetometer.begin(), m_Aspects_Magnetometer.end(), MTimeSort);

}
		
	
	return true;






	/*Now we're really done, right? Nope. Next, we must check with the previous batch of old aspect data 
	  we have and see if it is too different from the new data. If it is, we flag our data as untrustworthy.
	  Oh, by the way. That for loop we created at the beginning for the 10 batches of magnetometer data. It
	  just finished. Yep, at this point we've already added and sorted 10 MNCTAspect objects to the m_Aspects
	  vector. Thus, we must now create a new for loop to go through the vector we have just enlarged by 10 and,
	  MNCTAspect object, by MNCTAspect object slowly add in flags everywhere they are needed.*/


	/* Commenting out Ares checks on the data -> causing a spin lock in nucleraizer

	for (unsigned int i = 0; i < m_Aspects.size(); i++){
		cout << "currently checking MNCTAspect object # ";
		cout << i << endl;
		cout << "GPS_or_magnetometer of this is: ";
		cout << m_Aspects[i]->GetGPS_Or_Magnetometer() << endl;

		First things first. We retrieve our old GPS aspect data (if we are checking if a batch of data from
		  the magnetometer needs a flag, this old data will just go unused) using the GetPreviousGPS() function.
		  The way this function works will be described later.

		double old_GPS_Xgalon = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGalacticPointingXAxisLongitude();
		cout << "finished looking for previous GPS Xgalon and found: ";
		cout << old_GPS_Xgalon << endl;
		double old_GPS_Zgalon = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGalacticPointingZAxisLongitude();
		cout << "finished looking for previous GPS Zgalon and found: ";
		cout << old_GPS_Zgalon << endl;
		double old_GPS_Xgalat = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGalacticPointingXAxisLatitude();
		cout << "finished looking for previous GPS Xgalat and found: ";
		cout << old_GPS_Xgalat << endl;
		double old_GPS_Zgalat = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGalacticPointingZAxisLatitude();
		cout << "finished looking for previous GPS Zgalat and found: ";
		cout << old_GPS_Zgalat << endl;
		double old_GPS_GPS_or_magnetometer = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGPS_Or_Magnetometer();
		cout << "finished looking for previous GPS GPS_or_magnetometer and found: ";
		cout << old_GPS_GPS_or_magnetometer << endl;


		Since we had to create a new for loop, we must retrieve the aspect information for each and every 
		  MNCTAspect object we inspect to see if it needs a flag. We do so below: 

		double Xgalon = m_Aspects[i]->GetGalacticPointingXAxisLongitude();
		double Zgalon = m_Aspects[i]->GetGalacticPointingZAxisLongitude();
		double Xgalat = m_Aspects[i]->GetGalacticPointingXAxisLatitude();
		double Zgalat = m_Aspects[i]->GetGalacticPointingZAxisLatitude();

		double heading = m_Aspects[i]->GetHeading();
		double pitch = m_Aspects[i]->GetPitch();
		double roll = m_Aspects[i]->GetRoll();

		Okay, now we retrieve our old magnetometer aspect data (if we are checking if a batch of data from
		  the GPS needs a flag, this old data will just go unused) using the GetPreviousMagnetometer() function.
		  The way this function works will be described later.


		double old_M_Xgalon = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGalacticPointingXAxisLongitude();
		cout << "finished looking for previous magnetometer Xgalon and found: ";
		cout << old_M_Xgalon << endl;

		double old_M_Zgalon = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGalacticPointingZAxisLongitude();
		cout << "finished looking for previous magnetometer Zgalon and found: ";
		cout << old_M_Zgalon << endl;

		double old_M_Xgalat = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGalacticPointingXAxisLatitude();
		cout << "finished looking for previous magnetometer Xgalat and found: ";
		cout << old_M_Xgalat << endl;

		double old_M_Zgalat = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGalacticPointingZAxisLatitude();
		cout << "finished looking for previous magnetometer Zgalat and found: ";
		cout << old_M_Zgalat << endl;

		double old_M_GPS_or_magnetometer = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGPS_Or_Magnetometer();
		cout << "finished looking for previous magnetometer GPS_or_magnetometer and found: ";
		cout << old_M_GPS_or_magnetometer << endl;

		//Good. Good. Now it's time for the actual inspection and flagging (if necessary). First, for the GPS:

		if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 0){

			if(abs(Vincenty(old_GPS_Zgalat,Zgalat,old_GPS_Zgalon,Zgalon)) > 10.0 || abs(Vincenty(old_GPS_Xgalat,Xgalat,old_GPS_Xgalon,Xgalon)) > 10.0 || heading >= 360.0 || heading <=-360.0 || pitch >= 360.0 || pitch <= -360.0 || roll >= 360.0 || roll <= -360.0 || heading == 0.0 || pitch == 0.0 || roll == 0.0){
				m_Aspects[i]->SetFlag(1);
				cout << "flag applied" << endl;
				if(old_GPS_GPS_or_magnetometer == 2 && heading < 360.0 && heading > -360.0 && pitch < 360.0 && pitch > -360.0 && roll < 360.0 && roll > -360.0 && heading != 0.0 && pitch != 0.0 && roll != 0.0){
					m_Aspects[i]->SetFlag(0);
					cout << "flag removed due to lack of any previous data at all" << endl;
				}	
				if(GetPreviousGPS(m_Aspects[i]->GetTime())->GetFlag() == 1 && heading < 360.0 && heading > -360.0 && pitch < 360.0 && pitch > -360.0 && roll < 360.0 && roll > -360.0 && heading != 0.0 && pitch != 0.0 && roll != 0.0){
					m_Aspects[i]->SetFlag(0);
					cout << "flag removed due to previous data having a flag (if previous data exists)" << endl;
				}
			}
			else{
				m_Aspects[i]->SetFlag(0);
				cout << i;
				cout << " passed the Vincenty test" << endl; 
				cout << "Here is the result: ";
				cout << abs(Vincenty(old_GPS_Zgalat,Zgalat,old_GPS_Zgalon,Zgalon)) << endl; 
			}
		}	
		//Alright, now for the magnetometer:


		if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 1){

			if(abs(Vincenty(old_M_Zgalat,Zgalat,old_M_Zgalon,Zgalon)) > 10.0 || abs(Vincenty(old_M_Xgalat,Xgalat,old_M_Xgalon,Xgalon)) > 10.0 || heading >= 360.0 || heading <=-360.0 || pitch >= 360.0 || pitch <= -360.0 || roll >= 360.0 || roll <= -360.0 || heading == 0.0 || pitch == 0.0 || roll == 0.0){
				m_Aspects[i]->SetFlag(1);
				cout << "flag applied" << endl;
				if(old_GPS_GPS_or_magnetometer == 2 && heading < 360.0 && heading > -360.0 && pitch < 360.0 && pitch > -360.0 && roll < 360.0 && roll > -360.0 && heading != 0.0 && pitch != 0.0 && roll != 0.0){
					m_Aspects[i]->SetFlag(0);
					cout << "flag removed due to lack of any previous data at all" << endl;
				}	
				if(GetPreviousGPS(m_Aspects[i]->GetTime())->GetFlag() == 1 && heading < 360.0 && heading > -360.0 && pitch < 360.0 && pitch > -360.0 && roll < 360.0 && roll > -360.0 && heading != 0.0 && pitch != 0.0 && roll != 0.0){
					m_Aspects[i]->SetFlag(0);
					cout << "flag removed due to previous data having a flag (if previous data exists)" << endl;
				}
			}
			else{
				m_Aspects[i]->SetFlag(0);
				cout << i;
				cout << " passed the Vincenty test" << endl; 
				cout << "Here is the result: ";
				cout << abs(Vincenty(old_M_Zgalat,Zgalat,old_M_Zgalon,Zgalon)) << endl; 
			}				
		}


	} 

	//Here ends the second for loop. Now, just as a precaution, we open a 3rd for loop to review which MNCTAspect objects
	//had flags put on them.


	cout << "preparing to list vector elements with flags" << endl;
	for(unsigned int i = 0; i < m_Aspects.size(); i++){
		if(m_Aspects[i]->GetFlag() == 1){
			cout << i << endl;
		}
	}		
	cout << "finished listing vector elements with flags" << endl;

	//Alright, we're done here... Just kidding! We still need to review all of the other functions in this file!


	*/

	//return true;
	
	
	
	
	
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
