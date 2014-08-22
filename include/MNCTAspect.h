/*
 * MNCTAspect.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTAspect__
#define __MNCTAspect__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTAspect
{
  // public interface:
 public:
  //! Standard constructor
  MNCTAspect();
  //! Copy constructor
  MNCTAspect(const MNCTAspect& Aspect);
  //! Default destructor
  virtual ~MNCTAspect();

  //! Reset all data
  void Clear();

  //! Assignment operator 
  const MNCTAspect& operator= (const MNCTAspect& A);

////////////////////////////////////////////////////////////////////////////////
  
  //Ares' 1st set of adjustments begin here (there are more later).
  
  
  //! Set the m_GPS_or_magnetometer
  void SetTime(MTime tim) { m_Time = tim; }
  //! Get the m_GPS_or_magnetometer
  MTime GetTime() const { return m_Time; }
  
  
  //! Set the m_GPS_or_magnetometer
  void SetFlag(int fla) { m_Flag = fla; }
  //! Get the m_GPS_or_magnetometer
  int GetFlag() const { return m_Flag; }
  //! Set the m_GPS_or_magnetometer
  void SetGPS_Or_Magnetometer(int GPS) { m_GPS_Or_Magnetometer = GPS; }
  //! Get the m_GPS_or_magnetometer
  int GetGPS_Or_Magnetometer() const { return m_GPS_Or_Magnetometer; }
 
  //! Set the heading
  void SetHeading(double hea) { m_Heading = hea; }
  //! Get the heading
  double GetHeading() const { return m_Heading; }
  //! Set the pitch
  void SetPitch(double pit) { m_Pitch = pit; }
  //! Get the pitch
  double GetPitch() const { return m_Pitch; }
  //! Set the roll
  void SetRoll(double rol) { m_Roll = rol; }
  //! Get the roll
  double GetRoll() const { return m_Roll; }
    
  
  //Ares' 1st set of adjustments end here (there are more later).
  
////////////////////////////////////////////////////////////////////////////////
  

  //! Set the latitude
  void SetLatitude(double lat) { m_Latitude = lat; }
  //! Get the latitude
  double GetLatitude() const { return m_Latitude; }
  //! Set the longitude
  void SetLongitude(double lon) { m_Longitude = lon; }
  //! Get the longitude
  double GetLongitude() const { return m_Longitude; }
  //! Set the altitude
  void SetAltitude(double alt) { m_Altitude = alt; }
  //! Get the altitide
  double GetAltitude() const { return m_Altitude; }

  
  //! Set the x-axis of the galactic coordinate system - input is in degrees
  void SetGalacticPointingXAxis(const double Longitude, const double Latitude) { m_GalacticPointingXAxisLongitude = Longitude; m_GalacticPointingXAxisLatitude = Latitude; }
  //! Set the z-axis of the galactic coordinate system - input is in degrees
  void SetGalacticPointingZAxis(const double Longitude, const double Latitude) { m_GalacticPointingZAxisLongitude = Longitude; m_GalacticPointingZAxisLatitude = Latitude; }

  //! Get the x-axis of the galactic coordinate system - longitude in degrees
  double GetGalacticPointingXAxisLongitude() const { return m_GalacticPointingXAxisLongitude; }
  //! Get the z-axis of the galactic coordinate system - longitude in degrees
  double GetGalacticPointingZAxisLongitude() const { return m_GalacticPointingZAxisLongitude; }

  //! Get the x-axis of the galactic coordinate system - latitude in degrees
  double GetGalacticPointingXAxisLatitude() const { return m_GalacticPointingXAxisLatitude; }
  //! Get the z-axis of the galactic coordinate system - latitude in degrees
  double GetGalacticPointingZAxisLatitude() const { return m_GalacticPointingZAxisLatitude; }

  
  //! Set the x-axis of the horizon coordinate system - input is in degrees
  void SetHorizonPointingXAxis(const double AzimuthNorth, const double Elevation) { m_HorizonPointingXAxisAzimuthNorth = AzimuthNorth;  m_HorizonPointingXAxisElevation = Elevation; }
  //! Set the z-axis of the horizon coordinate system - input is in degrees
  void SetHorizonPointingZAxis(const double AzimuthNorth, const double Elevation) { m_HorizonPointingZAxisAzimuthNorth = AzimuthNorth; m_HorizonPointingZAxisElevation = Elevation; }

  //! Get the x-axis of the horizon coordinate system - azimuth north in degrees
  double GetHorizonPointingXAxisAzimuthNorth() const { return m_HorizonPointingXAxisAzimuthNorth; }
  //! Get the z-axis of the horizon coordinate system - azimuth north in degrees
  double GetHorizonPointingZAxisAzimuthNorth() const { return m_HorizonPointingZAxisAzimuthNorth; }

  //! Get the x-axis of the horizon coordinate system - Elevation in degrees
  double GetHorizonPointingXAxisElevation() const { return m_HorizonPointingXAxisElevation; }
  //! Get the z-axis of the horizon coordinate system - Elevation in degrees
  double GetHorizonPointingZAxisElevation() const { return m_HorizonPointingZAxisElevation; }

  
  //! Dump the content into a file stream
  bool StreamDat(ostream& S, int Version = 1);
  //! Stream the content in MEGAlib's evta format 
  void StreamEvta(ostream& S);

  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Time when this aspect was measured
  MTime m_Time;
  
  
////////////////////////////////////////////////////////////////////////////////
  
  //Ares' 2nd (and last) set of adjustments begin here.
  
  
  //! 0=No Problem;1=Data is not trustworthy.
  int m_Flag;

  //! 0=GPS;1=magnetometer
  int m_GPS_Or_Magnetometer;

  //! This heading is used to determine whether or not data should be flagged as untrustworthy.
  double m_Heading;
  //! This pitch is used to determine whether or not data should be flagged as untrustworthy.
  double m_Pitch;
  //! This roll is used to determine whether or not data should be flagged as untrustworthy.
  double m_Roll;
 
  
  //Ares' 2nd (and last) set of adjustments end here.
  
////////////////////////////////////////////////////////////////////////////////
  
  
  //! Latitude of the measurement
  double m_Latitude;
  //! Longitude of the measurement
  double m_Longitude;
  //! Altutude of the measurement
  double m_Altitude;

  //! Pointing of the detector in galactic coordinates - x-axis longitude
  double m_GalacticPointingXAxisLongitude;
  //! Pointing of the detector in galactic coordinates - x-axis latitude
  double m_GalacticPointingXAxisLatitude;
  //! Pointing of the detector in galactic coordinates - z-axis longitude
  double m_GalacticPointingZAxisLongitude;
  //! Pointing of the detector in galactic coordinates - z-axis latitude
  double m_GalacticPointingZAxisLatitude;
  
  //! Pointing of the detector in the horizon coordinate system - X axis azimuth from north
  double m_HorizonPointingXAxisAzimuthNorth;
  //! Pointing of the detector in the horizon coordinate system - X axis elevation
  double m_HorizonPointingXAxisElevation;
  //! Pointing of the detector in the horizon coordinate system - Z axis azimuth from north
  double m_HorizonPointingZAxisAzimuthNorth;
  //! Pointing of the detector in the horizon coordinate system - Z axis elevation
  double m_HorizonPointingZAxisElevation;
  

#ifdef ___CINT___
 public:
  ClassDef(MNCTAspect, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
