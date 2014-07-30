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
  //! Default destructor
  virtual ~MNCTAspect();

  //! Reset all data
  void Clear();

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
  bool Stream(ofstream& S, int Version);
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
