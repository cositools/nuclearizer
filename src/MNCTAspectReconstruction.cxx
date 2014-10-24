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

	for (auto A: m_Aspects) {
		delete A;
	}
	m_Aspects.clear();
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
	int Python_or_Cplusplus = PacketA.Python_or_Cplusplus;
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
	
	MTimeA.Set(1411668301,2);
	
	//Please delete the above line and uncomment the below paragraph later!!!!!!!
	
	//uint64_t ClkModulo = PacketA.CorrectedClk % 10000000;
	//double ClkSeconds = (double) (PacketA.CorrectedClk - ClkModulo); ClkSeconds = ClkSeconds/10000000.;
	//double ClkNanoseconds = (double) ClkModulo * 100.0;
	//MTimeA.Set( ClkSeconds, ClkNanoseconds);


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

	/*This here is the interesting part (only used if our data is from the magnetometer). Here, we 
	  create a text file named "example1.txt" and write the longitude and latitude onto it. Then,
	  we activate python using PyRun_SimpleString() and use the python module known as geomag to
	  give us the magnetic declination, given our longitude and latitude. Then we have python create
	  another text file, named "example2.txt" and have it write down the magnetic declination onto it.
	  Then PyRunSimpleString() finishes. Back in C++, our code reads that new text file and finally
	  learns the magnetic declination.*/

	if (GPS_or_magnetometer == 1){
		ofstream myAfile;
		myAfile.open ("example1.txt");
		myAfile << geographic_latitude;
		myAfile << ",";
		myAfile << geographic_longitude;
		myAfile.close();		
		ofstream myBfile;
		myBfile.open ("example2.txt");
		myBfile.close();

		MString WMMFile("$(NUCLEARIZER)/resource/aspect/WMM.COF");
		MFile::ExpandFileName(WMMFile);

		ostringstream Cmd;
		Cmd<<"f = open('example1.txt')\n"
			"data = f.readline()\n"
			"split_data = data.split(',')\n"
			"geographic_latitude = float(split_data[0]) \n"
			"geographic_longitude = float(split_data[1]) \n"
			"f.close()\n"
			"import geomag\n"
			"gm = geomag.geomag.GeoMag('"<<WMMFile<<"')\n"
			"mag = gm.GeoMag(geographic_latitude,geographic_longitude)\n"
			"magnetic_declination = mag.dec\n"
			"g = open('example2.txt','w')\n"
			"g.write(str(magnetic_declination))\n"
			"g.close()\n"<<endl;
		PyRun_SimpleString(Cmd.str().c_str());

		ifstream myCFile;
		myCFile.open("example2.txt");
		string string_magnetic_declination;
		getline(myCFile,string_magnetic_declination);	
		magnetic_declination = ::atof(string_magnetic_declination.c_str());
		
if(test_or_not == 0){		
		
		printf("magnetic_declination is: %9.5f \n",magnetic_declination);
		
}


	}

	//Now that we've gone through that crazy ordeal and have found the magnetic declination, we
	//add it to the magnetic heading to find and record the true heading, as promised:

	double heading = magnetic_heading + magnetic_declination;

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
		Ypointing0[1][0] = 1.0;
		Ypointing0[2][0] = 0.0;	
		
		Xpointing0[0][0] = -1.0;
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

if(Python_or_Cplusplus == 1){

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
	
}

	
////////////////////////////////////////////////////////////////////////////	
	
if(Python_or_Cplusplus == 0){	

	/*Here comes the tricky part. Again we will make use of python in the form of PyRunSimpleString.
	  What we do here we get all of the necessary inputs we need in order to find the galactic
	  longitude and latitude and write them all into a text file (known as "example5.txt"). Then, we open 
	  python, so we can finally use the fabled pyephem we have been talking about to finally get the
	  the galactic longitude and latitude. Python then writes these down in another text file 
	  ("example6.txt") and then shuts down. Afterwards (back in C++) we read that second text file 
	  and record the data.*/

	//Right here below, we record all the necessary	information that pyephem needs to find the
	//galactic longitude and latitude for us.

	ofstream myGfile;
	myGfile.open ("example5.txt");
	myGfile << geographic_longitude;
	myGfile << ",";
	myGfile << geographic_latitude;
	myGfile << ",";
	myGfile << elevation;
	myGfile << ",";
	myGfile << date_and_time;
	myGfile << ",";
	myGfile << Zazimuth;
	myGfile << ",";
	myGfile << Zaltitude;
	myGfile << ",";
	myGfile << Yazimuth;
	myGfile << ",";
	myGfile << Yaltitude;
	myGfile << ",";
	myGfile << Xazimuth;
	myGfile << ",";
	myGfile << Xaltitude;
	myGfile << ",";
	myGfile << test_or_not;
	myGfile.close();
	ofstream myHfile;
	myHfile.open ("example6.txt");
	myHfile.close();

	//And here's good old PyRunSimpleString once again (below)!

	PyRun_SimpleString(

			//Okay, first things first. We record all the data from the text file.

			"import ephem\n"
			"import math\n"
			"f = open('example5.txt')\n"
			"data = f.readline()\n"
			"split_data = data.split(',')\n"
			"input1 = str(float(split_data[0]))\n"
			"input2 = str(float(split_data[1]))\n"
			"input3 = float(split_data[2])\n"
			"input4 = split_data[3]\n"
			"input5 = math.radians(float(split_data[4]))\n"
			"input6 = math.radians(float(split_data[5]))\n"
			"input7 = math.radians(float(split_data[6]))\n"
			"input8 = math.radians(float(split_data[7]))\n"
			"input9 = math.radians(float(split_data[8]))\n"
			"input10 = math.radians(float(split_data[9]))\n"
			"input11 = math.radians(float(split_data[10]))\n"

			/*Now, we use pyephem to create an observer object that we will call COSI. We feed COSI the 
			  geographic longituude and latitude, what it calls the "elevation", the date and time, and the
			  azimuth and what it calls "altitude." This gives us the Equatorial coordinates (right ascension)
			  and declination. We then convert these coordinates to Galactic coordinates*/

			"COSI = ephem.Observer()\n"
			"COSI.lon = input1\n"
			"COSI.lat = input2\n"
			"COSI.elevation = input3\n"
			"COSI.date = input4\n"
			"if input11 == 0:\n"
			"	print COSI.date\n"
			"z = COSI.radec_of(input5,input6)\n"
			"z0 = str(z[0])\n"
			"z1 = str(z[1])\n"
			"Zeqco = ephem.Equatorial(z0,z1)\n"
			"Zgaco = ephem.Galactic(Zeqco)\n"
			"zz0 = str(Zgaco.lon)\n"
			"zz1 = str(Zgaco.lat)\n"
			"y = COSI.radec_of(input7,input8)\n"
			"y0 = str(y[0])\n"
			"y1 = str(y[1])\n"
			"Yeqco = ephem.Equatorial(y0,y1)\n"
			"Ygaco = ephem.Galactic(Yeqco)\n"
			"yy0 = str(Ygaco.lon)\n"
			"yy1 = str(Ygaco.lat)\n"
			"x = COSI.radec_of(input9,input10)\n"
			"x0 = str(x[0])\n"
			"x1 = str(x[1])\n"
			"Xeqco = ephem.Equatorial(x0,x1)\n"
			"Xgaco = ephem.Galactic(Xeqco)\n"
			"xx0 = str(Xgaco.lon)\n"
			"xx1 = str(Xgaco.lat)\n"

			//Now that we're done with that, we write the galactic coordinates we just find down into yet
			//another text file ("example6.txt").	

			"omega = ','\n"
			"g = open('example6.txt','w')\n"
			
			"g.write(zz0)\n"
			"g.write(omega)\n"
			"g.write(zz1)\n"
			"g.write(omega)\n"
			"g.write(yy0)\n"
			"g.write(omega)\n"
			"g.write(yy1)\n"
			"g.write(omega)\n"
			"g.write(xx0)\n"
			"g.write(omega)\n"
			"g.write(xx1)\n"
			"g.write(omega)\n"
			
			"g.write(z0)\n"
			"g.write(omega)\n"
			"g.write(z1)\n"
			"g.write(omega)\n"
			"g.write(y0)\n"
			"g.write(omega)\n"
			"g.write(y1)\n"
			"g.write(omega)\n"
			"g.write(x0)\n"
			"g.write(omega)\n"
			"g.write(x1)\n"
			
			"g.close()\n"
			"f.close()\n");

	//Phew. Okay, now we're finally done with messing with python for good. Now its time we read the
	//Galactic coordinates data we just wrote down into a text file.

	ifstream myIFile; //define your file stream
	myIFile.open("example6.txt"); //open the file
	string data2; //define "data2" as a string
	getline(myIFile,data2);
	
if(test_or_not == 0){


	cout << "Now finished with python" <<endl;
	
}


	//Now, its time we format this data by first making strings for it (below).

	std::string myWriting(data2);
	std::istringstream ixx(myWriting);
	std::string token2;
	string rudiments[12];
	int o = 0;
	while(getline(ixx, token2, ','))
	{
		rudiments[o] = token2;
		o = o+1;
	}

	string Zgalon_data = rudiments[0];
	string Zgalat_data = rudiments[1];

	string Ygalon_data = rudiments[2];
	string Ygalat_data = rudiments[3];

	string Xgalon_data = rudiments[4];
	string Xgalat_data = rudiments[5];
	
	string Zra_data = rudiments[6];
	string Zdec_data = rudiments[7];

	string Yra_data = rudiments[8];
	string Ydec_data = rudiments[9];

	string Xra_data = rudiments[10];
	string Xdec_data = rudiments[11];

	//Now its time (for the Z pointing direction) we convert the strings to doubles and print them. 


		
	std::string myZProse_e(Zra_data);
	std::istringstream iff(myZProse_e);
	std::string Ztoken3_e;
	string Zcondiments_e[3];
	int Zp_e = 0;
	while(getline(iff, Ztoken3_e, ':'))
	{
		Zcondiments_e[Zp_e] = Ztoken3_e;
		Zp_e = Zp_e+1;
	}
	string Zstring_ra_hours = Zcondiments_e[0];
	double Zra_hours = ::atof(Zstring_ra_hours.c_str());
	string Zstring_ra_minutes = Zcondiments_e[1];
	double Zra_minutes = ::atof(Zstring_ra_minutes.c_str());
	string Zstring_ra_seconds = Zcondiments_e[2];
	double Zra_seconds = ::atof(Zstring_ra_seconds.c_str());
	Zra = (Zra_hours + (Zra_minutes/60.0) + (Zra_seconds/3600.0))*15;
	
	
	std::string myZEpistle_e(Zdec_data);
	std::istringstream igg(myZEpistle_e);
	std::string Ztoken4_e;
	string Zcompliments_e[3];
	int Zq_e = 0;
	while(getline(igg, Ztoken4_e, ':'))
	{
		Zcompliments_e[Zq_e] = Ztoken4_e;
		Zq_e = Zq_e+1;
	}
	string Zstring_dec_degrees = Zcompliments_e[0];
	double Zdec_degrees = ::atof(Zstring_dec_degrees.c_str());
	string Zstring_dec_arcminutes = Zcompliments_e[1];
	double Zdec_arcminutes = ::atof(Zstring_dec_arcminutes.c_str());
	string Zstring_dec_arcseconds = Zcompliments_e[2];
	double Zdec_arcseconds = ::atof(Zstring_dec_arcseconds.c_str());
	Zdec = Zdec_degrees + (Zdec_arcminutes/60.0) + (Zdec_arcseconds/3600.0);
	if (Zdec < 0){
		Zdec = Zdec_degrees - (Zdec_arcminutes/60.0) - (Zdec_arcseconds/3600.0);
	}
	
	if(test_or_not == 0){
	
	
	cout << "" << endl;
	cout << "According to Python..." << endl;
	
	cout << "" << endl;
	cout << "Zdeclination is: ";
	cout << Zdec;
	cout << " degrees North" << endl;
	cout << "Zright ascension is: ";
	cout << Zra;
	cout << " degrees East" << endl;
	
}
	
		
	std::string myZProse(Zgalon_data);
	std::istringstream iss(myZProse);
	std::string Ztoken3;
	string Zcondiments[3];
	int Zp = 0;
	while(getline(iss, Ztoken3, ':'))
	{
		Zcondiments[Zp] = Ztoken3;
		Zp = Zp+1;
	}
	string Zstring_galon_hours = Zcondiments[0];
	double Zgalon_hours = ::atof(Zstring_galon_hours.c_str());
	string Zstring_galon_minutes = Zcondiments[1];
	double Zgalon_minutes = ::atof(Zstring_galon_minutes.c_str());
	string Zstring_galon_seconds = Zcondiments[2];
	double Zgalon_seconds = ::atof(Zstring_galon_seconds.c_str());
	Zgalon = Zgalon_hours + (Zgalon_minutes/60.0) + (Zgalon_seconds/3600.0);
	if (Zgalon < 0){
		Zgalon = Zgalon_hours - (Zgalon_minutes/60.0) - (Zgalon_seconds/3600.0);
	}
	std::string myZEpistle(Zgalat_data);
	std::istringstream itt(myZEpistle);
	std::string Ztoken4;
	string Zcompliments[3];
	int Zq = 0;
	while(getline(itt, Ztoken4, ':'))
	{
		Zcompliments[Zq] = Ztoken4;
		Zq = Zq+1;
	}
	string Zstring_galat_degrees = Zcompliments[0];
	double Zgalat_degrees = ::atof(Zstring_galat_degrees.c_str());
	string Zstring_galat_arcminutes = Zcompliments[1];
	double Zgalat_arcminutes = ::atof(Zstring_galat_arcminutes.c_str());
	string Zstring_galat_arcseconds = Zcompliments[2];
	double Zgalat_arcseconds = ::atof(Zstring_galat_arcseconds.c_str());
	Zgalat = Zgalat_degrees + (Zgalat_arcminutes/60.0) + (Zgalat_arcseconds/3600.0);
	if (Zgalat < 0){
		Zgalat = Zgalat_degrees - (Zgalat_arcminutes/60.0) - (Zgalat_arcseconds/3600.0);
	}
	
if(test_or_not == 0){
	
	
	cout << "" << endl;
	cout << "Zgalactic latitude is: ";
	cout << Zgalat;
	cout << " degrees North" << endl;
	cout << "Zgalactic longitude is: ";
	cout << Zgalon;
	cout << " degrees East" << endl;
	
}
		
	//Now its time we do the same for the Y pointing direction.


	std::string myYProse_e(Yra_data);
	std::istringstream ihh(myYProse_e);
	std::string Ytoken3_e;
	string Ycondiments_e[3];
	int Yp_e = 0;
	while(getline(ihh, Ytoken3_e, ':'))
	{
		Ycondiments_e[Yp_e] = Ytoken3_e;
		Yp_e = Yp_e+1;
	}
	string Ystring_ra_hours = Ycondiments_e[0];
	double Yra_hours = ::atof(Ystring_ra_hours.c_str());
	string Ystring_ra_minutes = Ycondiments_e[1];
	double Yra_minutes = ::atof(Ystring_ra_minutes.c_str());
	string Ystring_ra_seconds = Ycondiments_e[2];
	double Yra_seconds = ::atof(Ystring_ra_seconds.c_str());
	Yra = (Yra_hours + (Yra_minutes/60.0) + (Yra_seconds/3600.0))*15;


	std::string myYEpistle_e(Ydec_data);
	std::istringstream ijj(myYEpistle_e);
	std::string Ytoken4_e;
	string Ycompliments_e[3];
	int Yq_e = 0;
	while(getline(ijj, Ytoken4_e, ':'))
	{
		Ycompliments_e[Yq_e] = Ytoken4_e;
		Yq_e = Yq_e+1;
	}
	string Ystring_dec_degrees = Ycompliments_e[0];
	double Ydec_degrees = ::atof(Ystring_dec_degrees.c_str());
	string Ystring_dec_arcminutes = Ycompliments_e[1];
	double Ydec_arcminutes = ::atof(Ystring_dec_arcminutes.c_str());
	string Ystring_dec_arcseconds = Ycompliments_e[2];
	double Ydec_arcseconds = ::atof(Ystring_dec_arcseconds.c_str());
	Ydec = Ydec_degrees + (Ydec_arcminutes/60.0) + (Ydec_arcseconds/3600.0);
	if (Ydec < 0){
		Ydec = Ydec_degrees - (Ydec_arcminutes/60.0) - (Ydec_arcseconds/3600.0);
	}
	
if(test_or_not == 0){
	
	
	cout << "" << endl;
	cout << "Ydeclination is: ";
	cout << Ydec;
	cout << " degrees North" << endl;
	cout << "Yright ascension is: ";
	cout << Yra;
	cout << " degrees East" << endl;

}


	std::string myYProse(Ygalon_data);
	std::istringstream iuu(myYProse);
	std::string Ytoken3;
	string Ycondiments[3];
	int Yp = 0;
	while(getline(iuu, Ytoken3, ':'))
	{
		Ycondiments[Yp] = Ytoken3;
		Yp = Yp+1;
	}
	string Ystring_galon_hours = Ycondiments[0];
	double Ygalon_hours = ::atof(Ystring_galon_hours.c_str());
	string Ystring_galon_minutes = Ycondiments[1];
	double Ygalon_minutes = ::atof(Ystring_galon_minutes.c_str());
	string Ystring_galon_seconds = Ycondiments[2];
	double Ygalon_seconds = ::atof(Ystring_galon_seconds.c_str());
	Ygalon = Ygalon_hours + (Ygalon_minutes/60.0) + (Ygalon_seconds/3600.0);
	if (Ygalon < 0){
		Ygalon = Ygalon_hours - (Ygalon_minutes/60.0) - (Ygalon_seconds/3600.0);
	}
	std::string myYEpistle(Ygalat_data);
	std::istringstream ivv(myYEpistle);
	std::string Ytoken4;
	string Ycompliments[3];
	int Yq = 0;
	while(getline(ivv, Ytoken4, ':'))
	{
		Ycompliments[Yq] = Ytoken4;
		Yq = Yq+1;
	}
	string Ystring_galat_degrees = Ycompliments[0];
	double Ygalat_degrees = ::atof(Ystring_galat_degrees.c_str());
	string Ystring_galat_arcminutes = Ycompliments[1];
	double Ygalat_arcminutes = ::atof(Ystring_galat_arcminutes.c_str());
	string Ystring_galat_arcseconds = Ycompliments[2];
	double Ygalat_arcseconds = ::atof(Ystring_galat_arcseconds.c_str());
	Ygalat = Ygalat_degrees + (Ygalat_arcminutes/60.0) + (Ygalat_arcseconds/3600.0);
	if (Ygalat < 0){
		Ygalat = Ygalat_degrees - (Ygalat_arcminutes/60.0) - (Ygalat_arcseconds/3600.0);
	}
	
	if(test_or_not == 0){
	
	
	cout << "" << endl;
	cout << "Ygalactic latitude is: ";
	cout << Ygalat;
	cout << " degrees North" << endl;
	cout << "Ygalactic longitude is: ";
	cout << Ygalon;
	cout << " degrees East" << endl;
	
}

	//Okay, again, same deal here except this time for the X pointing direction.


	std::string myXProse_e(Xra_data);
	std::istringstream ikk(myXProse_e);
	std::string Xtoken3_e;
	string Xcondiments_e[3];
	int Xp_e = 0;
	while(getline(ikk, Xtoken3_e, ':'))
	{
		Xcondiments_e[Xp_e] = Xtoken3_e;
		Xp_e = Xp_e+1;
	}
	string Xstring_ra_hours = Xcondiments_e[0];
	double Xra_hours = ::atof(Xstring_ra_hours.c_str());
	string Xstring_ra_minutes = Xcondiments_e[1];
	double Xra_minutes = ::atof(Xstring_ra_minutes.c_str());
	string Xstring_ra_seconds = Xcondiments_e[2];
	double Xra_seconds = ::atof(Xstring_ra_seconds.c_str());
	Xra = (Xra_hours + (Xra_minutes/60.0) + (Xra_seconds/3600.0))*15;


	std::string myXEpistle_e(Xdec_data);
	std::istringstream ill(myXEpistle_e);
	std::string Xtoken4_e;
	string Xcompliments_e[3];
	int Xq_e = 0;
	while(getline(ill, Xtoken4_e, ':'))
	{
		Xcompliments_e[Xq_e] = Xtoken4_e;
		Xq_e = Xq_e+1;
	}
	string Xstring_dec_degrees = Xcompliments_e[0];
	double Xdec_degrees = ::atof(Xstring_dec_degrees.c_str());
	string Xstring_dec_arcminutes = Xcompliments_e[1];
	double Xdec_arcminutes = ::atof(Xstring_dec_arcminutes.c_str());
	string Xstring_dec_arcseconds = Xcompliments_e[2];
	double Xdec_arcseconds = ::atof(Xstring_dec_arcseconds.c_str());
	Xdec = Xdec_degrees + (Xdec_arcminutes/60.0) + (Xdec_arcseconds/3600.0);
	if (Xdec < 0){
		Xdec = Xdec_degrees - (Xdec_arcminutes/60.0) - (Xdec_arcseconds/3600.0);
	}
	
	if(test_or_not == 0){
		
		
	cout << "" << endl;
	cout << "Xdeclination is: ";
	cout << Xdec;
	cout << " degrees North" << endl;
	cout << "Xright ascension is: ";
	cout << Xra;
	cout << " degrees East" << endl;

}


	std::string myXProse(Xgalon_data);
	std::istringstream iyy(myXProse);
	std::string Xtoken3;
	string Xcondiments[3];
	int Xp = 0;
	while(getline(iyy, Xtoken3, ':'))
	{
		Xcondiments[Xp] = Xtoken3;
		Xp = Xp+1;
	}
	string Xstring_galon_hours = Xcondiments[0];
	double Xgalon_hours = ::atof(Xstring_galon_hours.c_str());
	string Xstring_galon_minutes = Xcondiments[1];
	double Xgalon_minutes = ::atof(Xstring_galon_minutes.c_str());
	string Xstring_galon_seconds = Xcondiments[2];
	double Xgalon_seconds = ::atof(Xstring_galon_seconds.c_str());
	Xgalon = Xgalon_hours + (Xgalon_minutes/60.0) + (Xgalon_seconds/3600.0);
	if (Xgalon < 0){
		Xgalon = Xgalon_hours - (Xgalon_minutes/60.0) - (Xgalon_seconds/3600.0);
	}
	std::string myXEpistle(Xgalat_data);
	std::istringstream izz(myXEpistle);
	std::string Xtoken4;
	string Xcompliments[3];
	int Xq = 0;
	while(getline(izz, Xtoken4, ':'))
	{
		Xcompliments[Xq] = Xtoken4;
		Xq = Xq+1;
	}
	string Xstring_galat_degrees = Xcompliments[0];
	double Xgalat_degrees = ::atof(Xstring_galat_degrees.c_str());
	string Xstring_galat_arcminutes = Xcompliments[1];
	double Xgalat_arcminutes = ::atof(Xstring_galat_arcminutes.c_str());
	string Xstring_galat_arcseconds = Xcompliments[2];
	double Xgalat_arcseconds = ::atof(Xstring_galat_arcseconds.c_str());
	Xgalat = Xgalat_degrees + (Xgalat_arcminutes/60.0) + (Xgalat_arcseconds/3600.0);
	if (Xgalat < 0){
		Xgalat = Xgalat_degrees - (Xgalat_arcminutes/60.0) - (Xgalat_arcseconds/3600.0);
	}
	
	if(test_or_not == 0){
	
	
	cout << "" << endl;
	cout << "Xgalactic latitude is: ";
	cout << Xgalat;
	cout << " degrees North" << endl;
	cout << "Xgalactic longitude is: ";
	cout << Xgalon;
	cout << " degrees East" << endl;

}
	
}	
	
////////////////////////////////////////////////////////////////////////////

	
	
		

	//Alright, here comes the moment of truth. Now we take everything we just recorded about our aspect
	//information and save it all into an MNCTAspect object.

	if (GPS_or_magnetometer == 0){
		Aspect->SetGPS_Or_Magnetometer(0);
	}
	if (GPS_or_magnetometer == 1){
		Aspect->SetGPS_Or_Magnetometer(1);
	}
	
	
	
	
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

	//Okay, we're good. Our MNCTAspect object has basically all we need, so now we add it to our m_Aspects
	//vector and we're done, right? Yeah, we wish...

	m_Aspects.push_back(Aspect);

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



	sort(m_Aspects.begin(), m_Aspects.end(), MTimeSort);
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

MNCTAspect* MNCTAspectReconstruction::GetAspect(MTime ReqTime){

	MNCTAspect* ReqAspect = 0;

	//check that there are aspect packets 
	if( m_Aspects.size() == 0 ){
		return 0;
	}

	//first need to check that event time is not older than the oldest Aspect we have
	//otherwise events won't get popped off of m_Events, since the first event will never get aspect info
	if( ReqTime < m_Aspects.front()->GetTime() ){
		//return an aspect that has a time of -1
		MNCTAspect* BadAspect = new MNCTAspect();
		BadAspect->SetTime( (double) -1.0 );
		return BadAspect;
	}

	//now check if event time is newer than the newest aspect ... if it is, then return null, we
	//need to wait for the newest aspect info to comein
	if( ReqTime > m_Aspects.back()->GetTime() ){
		return 0;
	}

	//this loop will only go if there are at least 2 aspects 
	for( int i = m_Aspects.size()-2; i > -1; --i ){
		if( ReqTime > m_Aspects[i]->GetTime() ){ //found the lower bracketing value
			if( (ReqTime - m_Aspects[i]->GetTime()) <= (m_Aspects[i+1]->GetTime() - ReqTime) ){ //check which bracketing value is closer
				ReqAspect = m_Aspects[i];
				break;
			} else {
				ReqAspect = m_Aspects[i+1];
				break;
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

/*This function, GetPreviousGPS(), was not created for the purposes of being used by people as it will 
  give you the previous GPS aspect data even it is bad data. The purpose of this function is to be used
  by the main chunk of this file to check and see if a piece of data should recieve a flag. Specifically,
  this function is used to retrieve old Aspect data to compare it to Aspect data being inspected for 
  flagging.*/


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
