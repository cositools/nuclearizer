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
  
  
#include "Python.h"
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

using namespace std;

  
//Ares' 1st set of adjustments end here (there are more later).
  
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


#include "MNCTAspectReconstruction.h"

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
			
bool MNCTAspectReconstruction::AddAspectFrame(vector<uint8_t> Frame)
{
	
	
//Here we make a class of objects known as "Packets."
	
	
	class Packet{
		public:
			int GPS_or_magnetometer; //0=GPS;1=magnetometer;2=no good data at
			/*specified time (when the GetAspect, GetAspectGPS, and 
			GetAspectMagnetometer search functions fail to find an MNCTAspect
			object at a specified time they return a dummy MNCTAspect object
			with its m_GPS_Or_Magnetometer attribute equal to 2*/
			string date_and_time;
			unsigned int nanoseconds;
			double geographic_longitude;
			double geographic_latitude;
			double elevation;
			double heading;
			double pitch;
			double roll;
		};	
	
	
	
	Packet PacketA;
	
//Alex's code should go here. It should use the raw GPS and magnetometer data frames to
//create "Packets" like in the form specified above. When that is done, please delete
//the 10 dummy data packets below.	
	
//This for loop is temporary and used to create the 10 dummy data packets below. Please
//delete it later!!!!!!!	
	
for(int i = 0; i < 10; i++)	
{	
	
	
  MNCTAspect* Aspect = new MNCTAspect;

  
// Fill the Aspect object with data from Frame. The data from the frame must have
// been parsed and inserted into PacketA via Alex's code.
  

/*
If you want to test this program for a dummy GPS data instead of dummy magnetometer
data please uncomment out the batch of code below and comment out the 10 batches 
(and their for loop) underneath. When all is said and done, the GPS and magnetometer 
dummy data packets here will give two wildly different pointing directions. Do not 
be alarmed. This dummy data originates from an instance when the gps and magnetometer 
were not properly aligned during data collection. Hence the discrepancy. If the gps 
and magnetometer are aligned in flight, real data should produce pointing directions
that are the same for both.
*/
		
		
/*	Packet PacketA;
		PacketA.GPS_or_magnetometer = 0;	
		PacketA.date_and_time = "2014/6/19 21:33:59";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 377.85;
		PacketA.heading = 154.6264;
		PacketA.pitch = -0.71;
		PacketA.roll = -1.65; */

//The 10 dummy packets of magnetometer data begin below

if(i == 0){ 

			
	
		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:33:59";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 0.1;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}
		
if(i == 1){ 		
		
		
	
		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:34:00";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 100.0;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}
		
if(i == 2){ 		
		
	
	
		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:34:01";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 200.0;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}	
	
if(i == 3){ 		
		
		
		
	
		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:34:02";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 300.0;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}
		
if(i == 4){ 		
		
		
	
	
		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:34:03";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 400.0;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}
	
if(i == 9){ 
		
		
	
		
		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:34:04";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 500.0;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}
	
if(i == 8){ 	
		
		


		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:34:05";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 600.0;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}
	
if(i == 7){ 	
		
		


		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:34:06";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 700.0;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}
		
if(i == 6){ 		
		
		


		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:34:07";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 800.0;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}
	
if(i == 5){ 	
		
		


		PacketA.GPS_or_magnetometer = 1;
		PacketA.date_and_time = "2014/6/19 21:34:08";
		PacketA.nanoseconds = 0;
		PacketA.geographic_longitude = -122.2333333;
		PacketA.geographic_latitude = 37.8666667;
		PacketA.elevation = 900.0;
		PacketA.heading = 155.33698;
		PacketA.pitch = 4.54129;
		PacketA.roll = 355.76718;
	}
		
		
//The 10 packets of magnetometer data end here.

	
//Here we record the geographic longitude & latitude, as well as the height above sea
//level (known as "elevation" in here). We also record the date and time as well as 
//whether our data is from the GPS or magnetometer. 
	
	int GPS_or_magnetometer = PacketA.GPS_or_magnetometer;
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

	std::string myPoetry(date_and_time);
	std::istringstream icc(myPoetry);
	std::string tokenA;
	string vitamins[2];
	int k  = 0;
	while(getline(icc,tokenA, ' ')){
		vitamins[k] = tokenA;
		k = k + 1;
		}
	string m_Date = vitamins[0];
	string m_Time_Of_Day = vitamins[1];

	std::string myNovella(m_Date);
	std::istringstream idd(myNovella);
	std::string tokenB;
	string minerals[3];
	int l  = 0;
	while(getline(idd,tokenB, '/')){
		minerals[l] = tokenB;
		l = l + 1;
		}
	string string_m_Year = minerals[0];
	string string_m_Month = minerals[1];
	string string_m_Day = minerals[2];

	std::string myPsalm(m_Time_Of_Day);
	std::istringstream iee(myPsalm);
	std::string tokenC;
	string supplements[3];
	int m  = 0;
	while(getline(iee,tokenC, ':')){
		supplements[m] = tokenC;
		m = m + 1;
		}
	string string_m_Hour = supplements[0];
	string string_m_Minute = supplements[1];
	string string_m_Second = supplements[2];

	unsigned int m_Year = ::atof(string_m_Year.c_str());
	unsigned int m_Month = ::atof(string_m_Month.c_str());
	unsigned int m_Day = ::atof(string_m_Day.c_str());
	unsigned int m_Hour = ::atof(string_m_Hour.c_str());
	unsigned int m_Minute = ::atof(string_m_Minute.c_str());
	unsigned int m_Second = ::atof(string_m_Second.c_str());
	unsigned int m_NanoSecond = nanoseconds;

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
		printf("magnetic_declination is: %9.5f \n",magnetic_declination);
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
	
/*Here is the fun part. Or perhaps, the confusing part. Here, we use rotation matrices to convert
from heading, pitch, and roll to horizontal coordinates. These coordinates are azimuth and 
"altitude," with altitude being degrees above the horizon. This "altitude" will eventually become
the "elevation" in the MNCTAspect object that will be created at the end of this program. Again,
we use these naming conventions to match up with pyephem. Anyway, we again use a couple of text
files and PyRunSimpleString over here.*/

//Below, we record the heading, pitch, roll and whether we are using the GPS or magnetomter to
//the text file "example3.txt."
	
	ofstream myDfile;
		myDfile.open ("example3.txt");
		myDfile << heading;
 		myDfile << ",";
 		myDfile << pitch;
 		myDfile << ",";
 		myDfile << roll;
 		myDfile << ",";
 		myDfile << GPS_or_magnetometer;
		myDfile.close();		
		ofstream myEfile;
		myEfile.open ("example4.txt");
		myEfile.close();	
		//Py_Initialize();
		
		cout << "Preparing too use python" << endl;
		
		PyRun_SimpleString(
		
//Here python looks at the info on the text file. It also defines its own trig functions to
//work with degrees.
		
		"f = open('example3.txt')\n"
		"data = f.readline()\n"
		"split_data = data.split(',')\n"
		"heading = float(split_data[0]) \n"
		"pitch = float(split_data[1]) \n"
		"roll = float(split_data[2]) \n"
		"GPS_or_magnetometer = float(split_data[3]) \n"
		"f.close()\n"
		"import math\n"
		"def sine(x): return math.sin(math.radians(x))\n"
		"def arcsine(x): return math.degrees(math.asin(x))\n"
		"def cosine(x): return math.cos(math.radians(x))\n"
		"def arccosine(x): return math.degrees(math.acos(x))\n"
		"def tangent(x): return math.tan(math.radians(x))\n"
		"def arctangent(x): return math.degrees(math.atan(x))\n"
		
//In the first scenario (below), we are using GPS data.		

		"if GPS_or_magnetometer == 0:\n"
		
/*Note that the initial pointing vectors (ZYorXpointing0) for Z, Y, and X do not actually 
point in the Z, Y, and X directions. This is because these pointing vectors are based off 
of the coordinate system of the telescope itself, not based off of the coordinate system 
that the GPS uses to define heading, pitch, and roll. Within the GPS's coordinate system, 
thus, the three pointing directions do not appear to point in their proper directions.
Please note that we are using the GPS's coordinate system here, so get used to the idea of
Z, Y, and X, pointing towards something other than Z, Y, and X. Don't worry. At the end we 
will convert our final result from this coordinate system to the telescope's coordinate 
system.*/
		
		"	Zpointing0 = [[0.0],\n"
		"	[0.0],\n"
		"	[-1.0]]\n"
		"	Ypointing0 = [[0.0],\n"
		"	[1.0],\n"
		"	[0.0]]\n"
		"	Xpointing0 = [[-1.0],\n"
		"	[0.0],\n"
		"	[0.0]]\n"
		"	print ''\n"
		"	print 'Zpointing0 is: ', Zpointing0 \n"
		"	print 'Ypointing0 is: ', Ypointing0 \n"
		"	print 'Xpointing0 is: ', Xpointing0 \n"
		"	print ''\n"
		
/*These are the heading, pitch, and roll rotation matrices, named Z, Y, and X. They are named
as such because of the definition of heading, pitch, and roll. "Heading" is defined as the
counterclockwise rotation about the Z axis. "Pitch" is defined as the counterclockeise rotation
about the Y axis. "Roll" is defined as the counterclockwise rotation about the X axis. Hence, the
matrices are named Z, Y, and X.*/
		
		"	Z = [[cosine(heading),0.0-sine(heading),0.0],\n"
		"	[sine(heading),cosine(heading),0.0],\n"
		"	[0.0,0.0,1.0]]\n"
		"	Y = [[cosine(pitch),0.0,sine(pitch)],\n"
		"	[0.0,1.0,0.0],\n"
		"	[0.0-sine(pitch),0.0,cosine(pitch)]]\n"
		"	X = [[1.0,0.0,0.0],\n"
		"	[0.0,cosine(roll),0.0-sine(roll)],\n"
		"	[0.0,sine(roll),cosine(roll)]]\n"
		
/*Please note, universal pointing directions are specified using heading, pitch and roll by performing
3 intrinsic rotations in a specific order (remember: rotation matrices in more than 2 dimensions do
not commute, so the order must be specific). Heading first, then pitch, then roll. The fact that these
are intrinsic rotations means that each rotation after the first one is performed off of the new
coordinate system that originates from the first coordinate system after it has already been rotated
previously. So, first there is the heading rotation around the original Z axis of the GPS. Then,
the pitch rotation is performed after the new Y axis that is formed after the initial heading rotation.
Finally, the roll rotation is performed on the X axis that is formed after both the heading and pitch
rotations are finished. In order to describe intrinsic rotations performed on a specific pointing 
vector one must perform the same rotations with normal rotation matrices in reverse order. Thus, because
we have Z, then Y, then X intrinsic rotations in that order we must apply the standard X, then Y, then Z
rotation matrices in that order to our pointing vector to get the proper result. This is why we construct
the "ZYX" rotation matrix below.*/
		
		"	YX = [[sum(a*b for a,b in zip(Y_row,X_col)) for X_col in zip(*X)] for Y_row in Y]\n"
		"	ZYX = [[sum(a*b for a,b in zip(Z_row,YX_col)) for YX_col in zip(*YX)] for Z_row in Z]\n"
		
		"	print 'Z is: ', Z \n"
		"	print 'Y is: ', Y \n"
		"	print 'X is: ', X \n"
		"	print 'YX is: ', YX \n"
		"	print 'ZYX is: ', ZYX \n"
		
/*Here is where the action happens. Here we apply our ZYX matrix to our pointing vectors. These rotated
vectors are called ZYorXpointing1. Finally, we convert these vectors from their form in the GPS's coordinate
system to their form in the telescope's coordinate system. At this point, they are now called ZYorXpointing2.*/
		
		"	Zpointing1 = [[sum(a*b for a,b in zip(ZYX_row,Zpointing0_col)) for Zpointing0_col in zip(*Zpointing0)] for ZYX_row in ZYX]\n"
		"	print 'Zpointing1 is: ', Zpointing1 \n"
		"	Zpointing2 = [[0.0 - Zpointing1[1][0]],[0.0 - Zpointing1[0][0]],[0.0 - Zpointing1[2][0]]]\n"
		"	print 'Zpointing2 is: ', Zpointing2 \n"
		
		"	Ypointing1 = [[sum(a*b for a,b in zip(ZYX_row,Ypointing0_col)) for Ypointing0_col in zip(*Ypointing0)] for ZYX_row in ZYX]\n"
		"	print 'Ypointing1 is: ', Ypointing1 \n"
		"	Ypointing2 = [[0.0 - Ypointing1[1][0]],[0.0 - Ypointing1[0][0]],[0.0 - Ypointing1[2][0]]]\n"
		"	print 'Ypointing2 is: ', Ypointing2 \n"
		
		"	Xpointing1 = [[sum(a*b for a,b in zip(ZYX_row,Xpointing0_col)) for Xpointing0_col in zip(*Xpointing0)] for ZYX_row in ZYX]\n"
		"	print 'Xpointing1 is: ', Xpointing1 \n"
		"	Xpointing2 = [[0.0 - Xpointing1[1][0]],[0.0 - Xpointing1[0][0]],[0.0 - Xpointing1[2][0]]]\n"
		"	print 'Xpointing2 is: ', Xpointing2 \n"

//Okay now, here's the same thing over again, except this time we are in the second scenario (below) with
//the magnetometer.	

		"if GPS_or_magnetometer == 1:\n"
		
/*Note that the initial pointing vectors (ZYorXpointing0) for Z, Y, and X do not actually 
point in the Z, Y, and X directions once again. This is because, just like with the GPS,
these pointing vectors are based off of the coordinate system of the telescope itself, 
not based off of the coordinate system that the magnetometer uses to define heading, pitch,
and roll. Within the magnetometer's coordinate system, thus, the three pointing directions 
do not appear to point in their proper directions. Please note that we are using the 
magnetometer's coordinate system here, which is different than the GPS's coordinate system, 
so don't be shoked that the pointing vectors are different this time.*/		
		
		"	Zpointing0 = [[0.0],\n"
		"	[0.0],\n"
		"	[1.0]]\n"
		"	Ypointing0 = [[0.0],\n"
		"	[1.0],\n"
		"	[0.0]]\n"
		"	Xpointing0 = [[1.0],\n"
		"	[0.0],\n"
		"	[0.0]]\n"
		"	print ''\n"
		"	print 'Zpointing0 is: ', Zpointing0 \n"
		"	print 'Ypointing0 is: ', Ypointing0 \n"
		"	print 'Xpointing0 is: ', Xpointing0 \n"
		"	print ''\n"
		
/*These are the same heading, pitch, and roll rotation matrices, named Z, Y, and X, once again.
They are exactly the same as before.*/		
		
		
		"	Z = [[cosine(heading),0.0-sine(heading),0.0],\n"
		"	[sine(heading),cosine(heading),0.0],\n"
		"	[0.0,0.0,1.0]]\n"
		"	Y = [[cosine(pitch),0.0,sine(pitch)],\n"
		"	[0.0,1.0,0.0],\n"
		"	[0.0-sine(pitch),0.0,cosine(pitch)]]\n"
		"	X = [[1.0,0.0,0.0],\n"
		"	[0.0,cosine(roll),0.0-sine(roll)],\n"
		"	[0.0,sine(roll),cosine(roll)]]\n"
		
/*Here we build the exact same ZYX matrix as when we were dealing with the GPS.*/		
		
		"	YX = [[sum(a*b for a,b in zip(Y_row,X_col)) for X_col in zip(*X)] for Y_row in Y]\n"
		"	ZYX = [[sum(a*b for a,b in zip(Z_row,YX_col)) for YX_col in zip(*YX)] for Z_row in Z]\n"
		
		
		"	print 'Z is: ', Z \n"
		"	print 'Y is: ', Y \n"
		"	print 'X is: ', X \n"
		"	print 'YX is: ', YX \n"
		"	print 'ZYX is: ', ZYX \n"
		
/*Here is where the action happens, just as before. Here we apply our ZYX matrix to our pointing vectors. 
These rotated vectors are called ZYorXpointing1 again. Finally, we convert these vectors from their form
in the magnetometer's coordinate system to their form in the telescope's coordinate system. Again, at this point,
they are now called ZYorXpointing2.*/	
		
		"	Zpointing1 = [[sum(a*b for a,b in zip(ZYX_row,Zpointing0_col)) for Zpointing0_col in zip(*Zpointing0)] for ZYX_row in ZYX]\n"
		"	print 'Zpointing1 is: ', Zpointing1 \n"
		"	Zpointing2 = [[Zpointing1[1][0]],[Zpointing1[0][0]],[0.0 - Zpointing1[2][0]]]\n"
		"	print 'Zpointing2 is: ', Zpointing2 \n"
		
		"	Ypointing1 = [[sum(a*b for a,b in zip(ZYX_row,Ypointing0_col)) for Ypointing0_col in zip(*Ypointing0)] for ZYX_row in ZYX]\n"
		"	print 'Ypointing1 is: ', Ypointing1 \n"
		"	Ypointing2 = [[Ypointing1[1][0]],[Ypointing1[0][0]],[0.0 - Ypointing1[2][0]]]\n"
		"	print 'Ypointing2 is: ', Ypointing2 \n"
		
		"	Xpointing1 = [[sum(a*b for a,b in zip(ZYX_row,Xpointing0_col)) for Xpointing0_col in zip(*Xpointing0)] for ZYX_row in ZYX]\n"
		"	print 'Xpointing1 is: ', Xpointing1 \n"
		"	Xpointing2 = [[Xpointing1[1][0]],[Xpointing1[0][0]],[0.0 - Xpointing1[2][0]]]\n"
		"	print 'Xpointing2 is: ', Xpointing2 \n"

//Below we determine the azimuth for the Zpointing2 vector based off of knowing what its x and y components are.

		"a = arctangent(math.fabs(Zpointing2[1][0]/Zpointing2[0][0]))\n"
		"if Zpointing2[0][0] >= 0.0 and Zpointing2[1][0] >= 0.0:\n"
		"	Zazimuth = 90.0 - a\n"
		"if Zpointing2[0][0] >= 0 and Zpointing2[1][0] <= 0.0:\n"
		"	Zazimuth= 90.0 + a\n"
		"if Zpointing2[0][0] <= 0.0 and Zpointing2[1][0] <= 0.0:\n"
		"	Zazimuth = 270.0 - a\n"
		"if Zpointing2[0][0] <= 0.0 and Zpointing2[1][0] >=0.0 :\n"
		"	Zazimuth = 270.0 + a\n"
		"if Zpointing2[0][0] == 0.0 and Zpointing2[1][0] > 0.0:\n"
		"	Zazimuth = 90.0\n"
		"if Zpointing2[0][0] < 0.0 and Zpointing2[1][0] == 0.0:\n"
		"	Zazimuth = 180.0\n"
		"if Zpointing2[0][0] == 0.0 and Zpointing2[1][0] < 0.0:\n"
		"	Zazimuth = 270.0\n"
		"if Zpointing2[0][0] > 0.0 and Zpointing2[1][0] == 0.0:\n"
		"	Zazimuth = 0.0\n"
		"if Zpointing2[0][0] == 0.0 and Zpointing2[1][0] == 0.0:\n"
		"	Zazimuth = 0.0\n"
		
//Below we determine the azimuth for the Ypointing2 vector based off of knowing what its x and y components are.		

		"c = arctangent(math.fabs(Ypointing2[1][0]/Ypointing2[0][0]))\n"
		"if Ypointing2[0][0] >= 0 and Ypointing2[1][0] >= 0:\n"
		"	Yazimuth = 90 - c\n"
		"if Ypointing2[0][0] >= 0 and Ypointing2[1][0] <= 0:\n"
		"	Yazimuth= 90 + c\n"
		"if Ypointing2[0][0] <= 0 and Ypointing2[1][0] <= 0:\n"
		"	Yazimuth = 270 - c\n"
		"if Ypointing2[0][0] <= 0 and Ypointing2[1][0] >=0 :\n"
		"	Yazimuth = 270 + c\n"
		"if Ypointing2[0][0] == 0.0 and Ypointing2[1][0] > 0.0:\n"
		"	Yazimuth = 90.0\n"
		"if Ypointing2[0][0] < 0.0 and Ypointing2[1][0] == 0.0:\n"
		"	Yazimuth = 180.0\n"
		"if Ypointing2[0][0] == 0.0 and Ypointing2[1][0] < 0.0:\n"
		"	Yazimuth = 270.0\n"
		"if Ypointing2[0][0] > 0.0 and Ypointing2[1][0] == 0.0:\n"
		"	Yazimuth = 0.0\n"
		"if Ypointing2[0][0] == 0.0 and Ypointing2[1][0] == 0.0:\n"
		"	Yazimuth = 0.0\n"
		
//Below we determine the azimuth for the Xpointing2 vector based off of knowing what its x and y components are.
		
		"b = arctangent(math.fabs(Xpointing2[1][0]/Xpointing2[0][0]))\n"
		"if Xpointing2[0][0] >= 0 and Xpointing2[1][0] >= 0:\n"
		"	Xazimuth = 90 - b\n"
		"if Xpointing2[0][0] >= 0 and Xpointing2[1][0] <= 0:\n"
		"	Xazimuth= 90 + b\n"
		"if Xpointing2[0][0] <= 0 and Xpointing2[1][0] <= 0:\n"
		"	Xazimuth = 270 - b\n"
		"if Xpointing2[0][0] <= 0 and Xpointing2[1][0] >=0 :\n"
		"	Xazimuth = 270 + b\n"
		"if Xpointing2[0][0] == 0.0 and Xpointing2[1][0] > 0.0:\n"
		"	Xazimuth = 90.0\n"
		"if Xpointing2[0][0] < 0.0 and Xpointing2[1][0] == 0.0:\n"
		"	Xazimuth = 180.0\n"
		"if Xpointing2[0][0] == 0.0 and Xpointing2[1][0] < 0.0:\n"
		"	Xazimuth = 270.0\n"
		"if Xpointing2[0][0] > 0.0 and Xpointing2[1][0] == 0.0:\n"
		"	Xazimuth = 0.0\n"
		"if Xpointing2[0][0] == 0.0 and Xpointing2[1][0] == 0.0:\n"
		"	Xazimuth = 0.0\n"
		
//Below we determine the "altitude" for the pointing vectors based off of knowing what its x, y, and z 
//components are. Again, we are using pyephem's definition of altitude for now.	
		
		"Zhypot = math.hypot(Zpointing2[0][0],Zpointing2[1][0])\n"
		"Yhypot = math.hypot(Ypointing2[0][0],Ypointing2[1][0])\n"
		"Xhypot = math.hypot(Xpointing2[0][0],Xpointing2[1][0])\n"
		
		"Zaltitude = arctangent(math.fabs(Zpointing2[2][0]/Zhypot))\n"
		"Yaltitude = arctangent(math.fabs(Ypointing2[2][0]/Yhypot))\n"
		"Xaltitude = arctangent(math.fabs(Xpointing2[2][0]/Xhypot))\n"
		
//Okay, here we finish up are write down our azimuths and "altitudes" into a text file knwon as
//"example4.txt." Again, "altitude" is pyephem's altitude.
		
		"omega = ','\n"
		"g = open('example4.txt','w')\n"
		"g.write(str(Zazimuth))\n"
		"g.write(str(omega))\n"
		"g.write(str(Zaltitude))\n"
		"g.write(str(omega))\n"
		"g.write(str(Yazimuth))\n"
		"g.write(str(omega))\n"
		"g.write(str(Yaltitude))\n"
		"g.write(str(omega))\n"
		"g.write(str(Xazimuth))\n"
		"g.write(str(omega))\n"
		"g.write(str(Xaltitude))\n"
		"g.write(str(omega))\n"
		"g.close()\n");
		
/*Alright, now that we've used heading, pitch, and roll from the GPS or magnetometer to find the
equatorial coordinates (azimuth and what pyephem calls "altitude") we need to read them off of the
text file we just made, which we do below.*/
		
		ifstream myFFile;
		myFFile.open("example4.txt");
		string data1;
		getline(myFFile,data1);	
		
		std::string myText(data1);
		std::istringstream iww(myText);
		std::string token1;
		string elements[6];
		int n = 0;
		while(getline(iww, token1, ','))
		{
			elements[n] = token1;
			n = n+1;
		}
	
		string Zstring_azimuth = elements[0];
		string Zstring_altitude = elements[1];
		
		double Zazimuth = ::atof(Zstring_azimuth.c_str());
		double Zaltitude = ::atof(Zstring_altitude.c_str());
		
		string Ystring_azimuth = elements[2];
		string Ystring_altitude = elements[3];
		
		double Yazimuth = ::atof(Ystring_azimuth.c_str());
		double Yaltitude = ::atof(Ystring_altitude.c_str());
		
		string Xstring_azimuth = elements[4];
		string Xstring_altitude = elements[5];
		
		double Xazimuth = ::atof(Xstring_azimuth.c_str());
		double Xaltitude = ::atof(Xstring_altitude.c_str());
		
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
	
/*Now, we use pyephem to create an observer object that we will call COSI. We feed COSI the 
geographic longituude and latitude, what it calls the "elevation", the date and time, and the
azimuth and what it calls "altitude." This gives us the Equatorial coordinates (right ascension)
and declination. We then convert these coordinates to Galactic coordinates*/
	
	"COSI = ephem.Observer()\n"
	"COSI.lon = input1\n"
	"COSI.lat = input2\n"
	"COSI.elevation = input3\n"
	"COSI.date = input4\n"
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
	"g.close()\n"
	"f.close()\n");
	
//Phew. Okay, now we're finally done with messing with python for good. Now its time we read the
//Galactic coordinates data we just wrote down into a text file.
			
	ifstream myIFile; //define your file stream
	myIFile.open("example6.txt"); //open the file
	string data2; //define "data2" as a string
	getline(myIFile,data2);
	
	cout << "Now finished with python" <<endl;
	
//Now, its time we format this data by first making strings for it (below).
	
	std::string myWriting(data2);
	std::istringstream ixx(myWriting);
	std::string token2;
	string rudiments[6];
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
	
//Now its time (for the Z pointing direction) we convert the strings to doubles and print them. 
	
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
	double Zgalon = Zgalon_hours + (Zgalon_minutes/60.0) + (Zgalon_seconds/3600.0);
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
	double Zgalat = Zgalat_degrees + (Zgalat_arcminutes/60.0) + (Zgalat_arcseconds/3600.0);
	if (Zgalat < 0){
		Zgalat = Zgalat_degrees - (Zgalat_arcminutes/60.0) - (Zgalat_arcseconds/3600.0);
		}
	cout << "" << endl;
	cout << "Zgalactic latitude is: ";
	cout << Zgalat;
	cout << " degrees North" << endl;
	cout << "Zgalactic longitude is: ";
	cout << Zgalon;
	cout << " hours East" << endl;
	
//Now its time we do the same for the Y pointing direction.

	
	
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
	double Ygalon = Ygalon_hours + (Ygalon_minutes/60.0) + (Ygalon_seconds/3600.0);
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
	double Ygalat = Ygalat_degrees + (Ygalat_arcminutes/60.0) + (Ygalat_arcseconds/3600.0);
	if (Ygalat < 0){
		Ygalat = Ygalat_degrees - (Ygalat_arcminutes/60.0) - (Ygalat_arcseconds/3600.0);
		}
	cout << "" << endl;
	cout << "Ygalactic latitude is: ";
	cout << Ygalat;
	cout << " degrees North" << endl;
	cout << "Ygalactic longitude is: ";
	cout << Ygalon;
	cout << " hours East" << endl;
	
//Okay, again, same deal here except this time for the X pointing direction.

	
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
	double Xgalon = Xgalon_hours + (Xgalon_minutes/60.0) + (Xgalon_seconds/3600.0);
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
	double Xgalat = Xgalat_degrees + (Xgalat_arcminutes/60.0) + (Xgalat_arcseconds/3600.0);
	if (Xgalat < 0){
		Xgalat = Xgalat_degrees - (Xgalat_arcminutes/60.0) - (Xgalat_arcseconds/3600.0);
		}
	cout << "" << endl;
	cout << "Xgalactic latitude is: ";
	cout << Xgalat;
	cout << " degrees North" << endl;
	cout << "Xgalactic longitude is: ";
	cout << Xgalon;
	cout << " hours East" << endl;
	
//Alright, here comes the moment of truth. Now we take everything we just recorded about our aspect
//information and save it all into an MNCTAspect object.
		
		if (GPS_or_magnetometer == 0){
			Aspect->SetGPS_Or_Magnetometer(0);
 			}
		if (GPS_or_magnetometer == 1){
			Aspect->SetGPS_Or_Magnetometer(1);
 			}
		MTime MTimeA;
		MTimeA.Set(m_Year,m_Month,m_Day,m_Hour,m_Minute,m_Second,m_NanoSecond);
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
  
  struct less_than_m_Time{
    inline bool operator() (MNCTAspect* MNCTAspect1 = new MNCTAspect, MNCTAspect* MNCTAspect2 = new MNCTAspect){
        return (MNCTAspect1->GetTime() < MNCTAspect2->GetTime());
    	}
	};
	
  sort(m_Aspects.begin(), m_Aspects.end(), less_than_m_Time());
  
}  

cout << "finished sorting" << endl;
  
/*Now we're really done, right? Nope. Next, we must check with the previous batch of old aspect data 
we have and see if it is too different from the new data. If it is, we flag our data as untrustworthy.
Oh, by the way. That for loop we created at the beginning for the 10 batches of magnetometer data. It
just finished. Yep, at this point we've already added and sorted 10 MNCTAspect objects to the m_Aspects
vector. Thus, we must now create a new for loop to go through the vector we have just enlarged by 10 and,
MNCTAspect object, by MNCTAspect object slowly add in flags everywhere they are needed.*/


for (unsigned int i = 0; i < m_Aspects.size(); i++){
	
cout << "currently inside of suspicious for loop" << endl;
cout << "currentvalue of i is: "; 
cout << i << endl;
cout<<(long) m_Aspects[i] <<endl;
	
/*First things first. We retrieve our old GPS aspect data (if we are checking if a batch of data from
the magnetometer needs a flag, this old data will just go unused) using the GetPreviousGPS() function.
The way this function works will be described later.*/

double old_GPS_Xgalon = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGalacticPointingXAxisLongitude();
double old_GPS_Zgalon = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGalacticPointingZAxisLongitude();
double old_GPS_Xgalat = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGalacticPointingXAxisLatitude();
double old_GPS_Zgalat = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGalacticPointingZAxisLatitude();
double old_GPS_GPS_or_magnetometer = GetPreviousGPS(m_Aspects[i]->GetTime())->GetGPS_Or_Magnetometer();

cout << "currently inside of suspicious for loop" << endl;

/*Since we had to create a new for loop, we must retrieve the aspect information for each and every 
MNCTAspect object we inspect to see if it needs a flag. We do so below: */

double Xgalon = m_Aspects[i]->GetGalacticPointingXAxisLongitude();
double Zgalon = m_Aspects[i]->GetGalacticPointingZAxisLongitude();
double Xgalat = m_Aspects[i]->GetGalacticPointingXAxisLatitude();
double Zgalat = m_Aspects[i]->GetGalacticPointingZAxisLatitude();

double heading = m_Aspects[i]->GetHeading();
double pitch = m_Aspects[i]->GetPitch();
double roll = m_Aspects[i]->GetRoll();

/*Okay, now we retrieve our old magnetometer aspect data (if we are checking if a batch of data from
the GPS needs a flag, this old data will just go unused) using the GetPreviousMagnetometer() function.
The way this function works will be described later.*/


cout << "" << endl;
cout << "preparing to look for previous Xgalon" << endl;
cout << "" << endl;
double old_M_Xgalon = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGalacticPointingXAxisLongitude();
cout << "" << endl;
cout << "finished looking for previous Xgalon and found: ";
cout << old_M_Xgalon << endl;
cout << "" << endl;

cout << "" << endl;
cout << "preparing to look for previous Zgalon" << endl;
cout << "" << endl;
double old_M_Zgalon = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGalacticPointingZAxisLongitude();
cout << "" << endl;
cout << "finished looking for previous Zgalon and found: ";
cout << old_M_Zgalon << endl;
cout << "" << endl;

cout << "" << endl;
cout << "preparing to look for previous Xgalat" << endl;
cout << "" << endl;
double old_M_Xgalat = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGalacticPointingXAxisLatitude();
cout << "" << endl;
cout << "finished looking for previous Xgalat and found: ";
cout << old_M_Xgalat << endl;
cout << "" << endl;

cout << "" << endl;
cout << "preparing to look for previous Zgalat" << endl;
cout << "" << endl;
double old_M_Zgalat = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGalacticPointingZAxisLatitude();
cout << "" << endl;
cout << "finished looking for previous Zgalat and found: ";
cout << old_M_Zgalat << endl;
cout << "" << endl;

cout << "" << endl;
cout << "preparing to look for previous GPS_or_magnetometer" << endl;
cout << "" << endl;
double old_M_GPS_or_magnetometer = GetPreviousMagnetometer(m_Aspects[i]->GetTime())->GetGPS_Or_Magnetometer();
cout << "" << endl;
cout << "finished looking for previous GPS_or_magnetometer and found: ";
cout << old_M_GPS_or_magnetometer << endl;
cout << "" << endl;

//Good. Good. Now it's time for the actual inspection and flagging (if necessary). First, for the GPS:

	if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 0 && (abs(Vincenty(old_GPS_Zgalat,Zgalat,old_GPS_Zgalon,Zgalon)) > 10.0 || abs(Vincenty(old_GPS_Xgalat,Xgalat,old_GPS_Xgalon,Xgalon)) > 10.0 || heading >= 360.0 || heading <=-360.0 || pitch >= 360.0 || pitch <= -360.0 || roll >= 360.0 || roll <= -360.0)){
		m_Aspects[i]->SetFlag(1);
		}
	else{
		m_Aspects[i]->SetFlag(0);
		}
	if(old_GPS_GPS_or_magnetometer == 2){
		m_Aspects[i]->SetFlag(0);
		}	
	
//Alright, now for the magnetometer:

cout << "preparing to decide if a flag should be placed on element # ";
cout << i << endl;

	if(m_Aspects[i]->GetGPS_Or_Magnetometer() == 1 && (abs(Vincenty(old_M_Zgalat,Zgalat,old_M_Zgalon,Zgalon)) > 10.0 || abs(Vincenty(old_M_Xgalat,Xgalat,old_M_Xgalon,Xgalon)) > 10.0 || heading >= 360.0 || heading <=-360.0 || pitch >= 360.0 || pitch <= -360.0 || roll >= 360.0 || roll <= -360.0)){
		m_Aspects[i]->SetFlag(1);
		cout << "flag has been placed on element # ";
		cout << i << endl;
		}
	else{
		m_Aspects[i]->SetFlag(0);
		}
	if(old_M_GPS_or_magnetometer == 2 && heading < 360.0 && heading > -360.0 && pitch < 360.0 && pitch > -360.0 && roll < 360.0 && roll > -360.0){
		m_Aspects[i]->SetFlag(0);
		cout << "flag has been removed from element # ";
		cout << i;
		cout << " due to inability to find the preceding element for comparison" << endl;
		}				
		
cout << "finished deciding if a flag should be placed on element # ";
cout << i << endl;
	
} 

//Here ends the second for loop. Now, just as a precaution, we open a 3rd for loop to review which MNCTAspect objects
//had flags put on them.


cout << "preparing to list vector elements with flags" << endl;
for(unsigned int i = 0; i < m_Aspects.size(); i++){
	if(m_Aspects[i]->GetFlag() == 1){
		cout << i << endl;
		}
	}		
 
//Alright, we're done here... Just kidding! We still need to review all of the other functions in this file!
 
  return true;
  }
  


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



//Below we see various search functions useful for acquiring aspect information if given the time.


/*Okay, first off, we see the "GetAspect()" function. This function behaves like the GetAspectGPS()
function (more on that function later) except that if it is unable to find good GPS data for a given
time it will try to find good magnetometer data instead.*/

MNCTAspect* MNCTAspectReconstruction::GetAspect(MTime Time){
  MNCTAspect* Desired_MNCTAspect = new MNCTAspect();
  MNCTAspect* Desired_Candidate_MNCTAspect = new MNCTAspect();
  MNCTAspect* Lower_MNCTAspect = new MNCTAspect();
  MNCTAspect* Upper_MNCTAspect = new MNCTAspect();
  Desired_MNCTAspect->SetGPS_Or_Magnetometer(2); 
  MTime time_after_last_data;
  MTime time_until_next_data;
  for(unsigned int i = 0; i < m_Aspects.size(); i++){
  	cout << "flag value is: " << endl;
  	cout << m_Aspects[i]->GetFlag() << endl;
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
  	cout << "flag value is: " << endl;
  	cout << m_Aspects[i]->GetFlag() << endl;
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
  	cout << "flag value is: " << endl;
  	cout << m_Aspects[i]->GetFlag() << endl;
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
  	cout << "flag value is: " << endl;
  	cout << m_Aspects[i]->GetFlag() << endl;
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
	double great_circle_distance = arctangent2(sqrt(pow(cosine(new_glat) * sine(abs(new_glon - old_glon)),2) + pow(cosine(old_glat) * sine(old_glat) - sine(old_glat) * cosine(new_glat) * cos(abs(new_glon - old_glon)),2)),sine(old_glat)*sine(new_glat) + cosine(old_glat) * cosine(new_glat) * cosine(abs(new_glon - old_glon)));
	return great_circle_distance;
	}

//Okay, okay, okay... Now we're really, really done. Thanks for bearing with these comments from the beginning to
//the end. Hopefully, they thoroughly explained how this code works.


//Ares' 3rd (and last) set of adjustments end here.
  
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


// MNCTAspectReconstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////