/*
 * MNCTTimeAndCoordinate.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTTimeAndCoordinate__
#define __MNCTTimeAndCoordinate__

////////////////////////////////////////////////////////////////////////////////



// Standard libs:
#include <vector>

// ROOT libs:
#include "TMatrixD.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MNCTMath.h"


// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

class MNCTTimeAndCoordinate
{

 protected:
  //! some constants for time conversion
  static const double c_Day2Second;
  static const double c_Second2Day;
  static const double c_MJDJD; //constant for converting MJD to JD
  static const double c_UnixTimeMJD; //MJD when Unix time starts
  static const double c_TAI2TT; //TT = TAI+32.184
  static const double c_LeapSeconds; // Unix Time and GPS Time = UT + LeapSeconds
  //static const double c_LeapSeconds = 24.0; // Unix Time and GPS Time = UT + LeapSeconds
  //for 2009 and 2010, LeapSeconds = 24.0
  //If clock synchronized by using the Network Time Protocol, c_LeapSeconds should be zero
  //Ref. http://en.wikipedia.org/wiki/Leap_second
  //Ref. http://cr.yp.to/proto/utctai.html

  //! constants for convert J2000 to glactic coordinate
  //NOTE: the accuracy is about 0.1 degrees.
  static const double c_GAngle1;
  static const double c_GAngle2;
  static const double c_GAngle3;
  
	
 public:
  //!
  MNCTTimeAndCoordinate();

  //!
  MNCTTimeAndCoordinate(double time, double MJDZero = c_UnixTimeMJD, double longitude = 0, double latitude = 0);

  //!
  ~MNCTTimeAndCoordinate();

  //! set or get time zero as MJD
  void SetMJDZero(double MJDZero){m_MJDZero = MJDZero;}
  double GetMJDZero(){return m_MJDZero;}

  //!
  void SetTimeSinceMJDZero(double time){m_TimeSinceMJDZero = time;}
  double GetTimeSinceMJDZero(){return m_TimeSinceMJDZero;}
  
  //! set or get time as unix time
  void SetUnixTime(double time){m_TimeSinceMJDZero = time - c_LeapSeconds - (m_MJDZero-c_UnixTimeMJD)*c_Day2Second;}
  double GetUnixTime(){return m_TimeSinceMJDZero + c_LeapSeconds + (m_MJDZero-c_UnixTimeMJD)*c_Day2Second;}

  //!
  //void Date(string date);
  //void Date(vector<double> date);
  //vector<double> Date();

  //! set or get time as MJD
  void SetMJD(double mjd){m_TimeSinceMJDZero = (mjd - m_MJDZero) * c_Day2Second;}
  double GetMJD(){return (m_TimeSinceMJDZero * c_Second2Day)+m_MJDZero;}

  //! set or get location
  void SetLocation(double latitude, double longitude) {m_Longitude = longitude, m_Latitude = latitude;}
  double GetLongitude(){return m_Longitude;}
  double GetLatitude(){return m_Latitude;}

  //! Mean and Apparent Sidereal Time calculations
  // MST is good to only ~0.1 seconds.
  // AST has further corrections and 
  // Find MJD at previous UT midnight
  double MJD_at_previous_midnight();
  // Find hours since previous UT midnight
  double Hours_since_previous_midnight();
  // Equation of the equinoxes
  double Equation_of_equinoxes_hours();
  // Calculate GMST and GAST from MJD
  double GMST_hours();
  double GAST_hours();
  // Calculate GMST and GAST in degrees
  double GMST_degrees();
  double GAST_degrees();
  // Calculate LMST in degrees from MJD and longitude (already set elsewhere)
  double LMST_degrees();
  double LAST_degrees();

  //! astronomical coordinates conversion
  vector<double> Equatorial2Galactic(vector<double> radec);
  vector<double> Horizon2Galactic(double azi, double alt){return Equatorial2Galactic(Horizon2Equatorial(azi, alt));}
  vector<double> Horizon2Equatorial(double azi, double alt);

  //! Coordinate transformations from dGPS angles (pitch, roll, yaw) to Horizon coordinates
  // rotation matrix to convert vector in dGPS frame to vector in horizon coordinates
  TMatrixD dGPS_to_Horizon_Rotation(double pitch_deg, double roll_deg, double yaw_deg);
  // rotation matrix to convert vector in cryostat frame to vector in horizon coordinates
  TMatrixD Cryo_to_Horizon_Rotation(double pitch_deg, double roll_deg, double yaw_deg);
  // convert cryostat vector into Horizon (azi, alt) coordinates
  vector<double> CryoVector_to_Horizon(TMatrixD Vector_cryo, double pitch_deg, double roll_deg, double yaw_deg);
  // convert cryostat X axis into Horizon (azi, alt) coordinates
  vector<double> CryoX_to_Horizon(double pitch_deg, double roll_deg, double yaw_deg);
  // convert cryostat Z axis into Horizon (azi, alt) coordinates
  vector<double> CryoZ_to_Horizon(double pitch_deg, double roll_deg, double yaw_deg);

  //!
  //vector<double> Decimal2Sexagesimal(double d);
  //vector<double> Decimal2hhmmss(double d);
  //double Sexagesimal2Desimal(vector<double> dms);
  //double hhmmss2Decimal(vector<double>);
  
  //! conversion between elevation and azimuthal angle
  double ELV2Zenith(double ELV){return (90.0-ELV);}
  double Zenith2ELV(double azi){return (90.0-azi);}

  //! 0~360 -> -180~180
  double NegativeDegree(double positivedegree){return (positivedegree>180) ? (positivedegree - 360) : positivedegree;}
  double PositiveDegree(double negativedegree){return (negativedegree < 0) ? (negativedegree + 360) : negativedegree;}

  //! radian to degrees or inverse
  double R2D(double radian){return radian*TMath::RadToDeg();}
  double D2R(double degree){return degree*TMath::DegToRad();}

  // protected members:
 protected:

  //!
  double m_TimeSinceMJDZero; //in seconds
  
  //! time zero in MJD
  double m_MJDZero;

  //!
  double m_Longitude;
  double m_Latitude;

  // Rotation matrix for converting a vector in cryostat coordinates to dGPS coordinates
  TMatrixD m_Cryo_to_dGPS_Rotation;
  
#ifdef ___CINT___
 public:
  ClassDef(MNCTTimeAndCoordinate, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////

