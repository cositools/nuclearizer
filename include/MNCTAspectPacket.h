


#ifndef __MNCTAspectPacket__
#define __MNCTAspectPacket__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <deque>
using namespace std;

// ROOT libs:

// MEGAlib libs:

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////	


class MNCTAspectPacket{
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
		uint64_t PPSClk;
		uint64_t CorrectedClk;// approx the clock board value using the GPS milliseconds + PPS latch info
		bool Error;

		//From the GPS
		uint32_t GPSMilliseconds;
		double BRMS;
		uint16_t AttFlag;



	
	
  
#ifdef ___CINT___
 public:
  ClassDef(MNCTAspectReconstruction, 0) // no description
#endif

};

#endif



////////////////////////////////////////////////////////////////////////////////
	
