/*
 * MNCTAspect.cxx
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
// MNCTAspect
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTAspect.h"

// Standard libs:
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTAspect)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTAspect::MNCTAspect()
{
  // Construct an instance of MNCTAspect

  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MNCTAspect::~MNCTAspect()
{
  // Delete this instance of MNCTAspect
}


////////////////////////////////////////////////////////////////////////////////


void MNCTAspect::Clear()
{
  // Reset all data

  m_Time.Set(0);
  
  
////////////////////////////////////////////////////////////////////////////////
  
  //Ares' adjustments begin here.
  
  
  m_Flag = 1;

  m_Heading = 0;
  m_Pitch = 0;
  m_Roll = 0;

  m_GPS_Or_Magnetometer = 0;
    
  
  //Ares' adjustments end here.
  
////////////////////////////////////////////////////////////////////////////////
  
  
  m_Latitude = 0;
  m_Longitude = 0;
  m_Altitude = 0;
  
  m_GalacticPointingXAxisLongitude = 0;
  m_GalacticPointingXAxisLatitude = 0;
  m_GalacticPointingZAxisLongitude = 0;
  m_GalacticPointingZAxisLatitude = 0;
  
  m_HorizonPointingXAxisAzimuthNorth = 0;
  m_HorizonPointingXAxisElevation = 0;
  m_HorizonPointingZAxisAzimuthNorth = 0;
  m_HorizonPointingZAxisElevation = 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTAspect::StreamDat(ostream& S, int Version)
{
  //! Stream the content to an ASCII file 
       
  S<<"LT "<<setprecision(8)<<m_Latitude<<endl;
  S<<"LN "<<setprecision(8)<<m_Longitude<<endl;
  S<<"AL "<<setprecision(8)<<m_Altitude<<endl;
  S<<"GX "<<setprecision(8)<<m_GalacticPointingXAxisLongitude<<" "<<m_GalacticPointingXAxisLatitude<<endl;
  S<<"GZ "<<setprecision(8)<<m_GalacticPointingZAxisLongitude<<" "<<m_GalacticPointingZAxisLatitude<<endl;
  S<<"HX "<<setprecision(8)<<m_HorizonPointingXAxisAzimuthNorth<<" "<<m_HorizonPointingXAxisElevation<<endl;
  S<<"HZ "<<setprecision(8)<<m_HorizonPointingZAxisAzimuthNorth<<" "<<m_HorizonPointingZAxisElevation<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTAspect::StreamEvta(ostream& S)
{
  // Stream the content in MEGAlib's evta format 
       
  S<<"GX "<<setprecision(8)<<m_GalacticPointingXAxisLongitude<<" "<<m_GalacticPointingXAxisLatitude<<endl;
  S<<"GZ "<<setprecision(8)<<m_GalacticPointingZAxisLongitude<<" "<<m_GalacticPointingZAxisLatitude<<endl;
  S<<"HX "<<setprecision(8)<<m_HorizonPointingXAxisAzimuthNorth<<" "<<m_HorizonPointingXAxisElevation<<endl;
  S<<"HZ "<<setprecision(8)<<m_HorizonPointingZAxisAzimuthNorth<<" "<<m_HorizonPointingZAxisElevation<<endl;
}


// MNCTAspect.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
