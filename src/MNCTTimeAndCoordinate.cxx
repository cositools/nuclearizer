/*
 * MNCTTimeAndCoordinate.cxx
 *
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Jau-Shian Liang.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */



////////////////////////////////////////////////////////////////////////////////
//
// MNCTTimeAndCoordinate
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTTimeAndCoordinate.h"

// Standard libs:
#include <iomanip>

// ROOT libs:
#include "TMath.h"
#include "TMatrixD.h"

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNCTTimeAndCoordinate)
#endif



////////////////////////////////////////////////////////////////////////////////

const double MNCTTimeAndCoordinate::c_Day2Second = 86400.;
const double MNCTTimeAndCoordinate::c_Second2Day = 1./86400.;
const double MNCTTimeAndCoordinate::c_MJDJD = 2400000.5; //constant for converting MJD to JD
const double MNCTTimeAndCoordinate::c_UnixTimeMJD = 40587.0; //MJD when Unix time starts

const double MNCTTimeAndCoordinate::c_TAI2TT = 32.184; //TT = TAI+32.184
const double MNCTTimeAndCoordinate::c_LeapSeconds = 0.0; // Unix Time and GPS Time = UT + LeapSeconds
const double MNCTTimeAndCoordinate::c_GAngle1 = 282.85; //J2000
const double MNCTTimeAndCoordinate::c_GAngle2 = 62.9;
const double MNCTTimeAndCoordinate::c_GAngle3 = -33.0;

const double MNCTTimeAndCoordinate::c_RA_g = 192.85; //J2000 RA of Galactic North Pole in degrees
const double MNCTTimeAndCoordinate::c_dec_g = 27.128333333; //J2000 dec of Galactic North Pole in degrees
const double MNCTTimeAndCoordinate::c_dec_c = -28.929656275; //J2000 dec of Galactic Center in degrees



////////////////////////////////////////////////////////////////////////////////

MNCTTimeAndCoordinate::MNCTTimeAndCoordinate()
{
  m_MJDZero = c_UnixTimeMJD;
  m_TimeSinceMJDZero = 0;
  m_Longitude = 0;
  m_Latitude = 0;
}

////////////////////////////////////////////////////////////////////////////////

MNCTTimeAndCoordinate::MNCTTimeAndCoordinate(double time, double MJDZero, double longitude, double latitude):
m_TimeSinceMJDZero(time), m_MJDZero(MJDZero), m_Longitude(longitude), m_Latitude(latitude)
{
}

////////////////////////////////////////////////////////////////////////////////

MNCTTimeAndCoordinate::~MNCTTimeAndCoordinate()
{
}

////////////////////////////////////////////////////////////////////////////////

vector<double> MNCTTimeAndCoordinate::Equatorial2Galactic(vector<double> radec)
{

  MVector dir;
  dir.SetMagThetaPhi(1, D2R(ELV2Zenith(radec[1])), D2R(radec[0]));
  dir.RotateZ(-D2R(c_GAngle1));
  dir.RotateX(-D2R(c_GAngle2));
  dir.RotateZ(-D2R(c_GAngle3));
  
  vector<double> galactic;
  galactic.push_back( R2D(dir.Phi()) );
  if(galactic[0]>=360.0)galactic[0]-=360.0;
  if(galactic[0]<0.0)galactic[0]+=360.0;  
  galactic.push_back( Zenith2ELV(R2D(dir.Theta())) );
  //cout << R2D(dir.Phi()) << ' ' << Zenith2ELV(R2D(dir.Theta())) <<'\n';

  return galactic;
}

////////////////////////////////////////////////////////////////////////////////

//Different conversion written by Carolyn Kierans August 2016 - give more precise results vs. above
vector<double> MNCTTimeAndCoordinate::Equatorial2Galactic2(vector<double> radec)
{

	double gal_lat = R2D(asin( sin(D2R(radec[1]))*sin(D2R(c_dec_g)) + cos(D2R(radec[1]))*cos(D2R(c_dec_g))*cos(D2R(c_RA_g-radec[0]))));
	double J = (sin(D2R(radec[1]))*cos(D2R(c_dec_g)) - cos(D2R(radec[1]))*sin(D2R(c_dec_g))*cos(D2R(radec[0]-c_RA_g)))/cos(D2R(gal_lat));
	double K = R2D(asin( cos(D2R(radec[1]))*sin(D2R(radec[0]-c_RA_g))/cos(D2R(gal_lat))));
	double Q = R2D(acos( sin(D2R(c_dec_c))/cos(D2R(c_dec_g))));

	double gal_long;
	if (J<0) {
		gal_long = Q+K -180;
	} else {
		gal_long = Q-K;
	}
	if (gal_long<0) gal_long = gal_long + 360;

	vector<double> galactic;
	galactic.push_back(gal_long);
	galactic.push_back( gal_lat);

	return galactic;
}


////////////////////////////////////////////////////////////////////////////////

vector<double> MNCTTimeAndCoordinate::Horizon2Equatorial(double azi, double alt)
{
  double local_sidereal_time = LAST_degrees();
  MVector dir;

  dir.SetMagThetaPhi(1, D2R(ELV2Zenith(alt)), D2R(PositiveDegree(180-azi)));
  dir.RotateY( D2R(ELV2Zenith(m_Latitude)) );

  vector<double> radec;
  radec.push_back( local_sidereal_time + R2D(dir.Phi()) );
  if(radec[0]>=360.0)radec[0]-=360.0;
  if(radec[0]<0.0)radec[0]+=360.0;
  radec.push_back( Zenith2ELV(R2D(dir.Theta())) );

  //cout << local_sidereal_time + R2D(dir.Phi()) << ' ' << D2R(ELV2Zenith(alt)) << ' ' << ' ' << (R2D(dir.Theta())) <<'\n';
  return radec;
}

////////////////////////////////////////////////////////////////////////////////

//Different converion written by Carolyn Kierans August 2016 - gives idential results as above
vector<double> MNCTTimeAndCoordinate::Horizon2Equatorial2(double azi, double alt)
{
	double local_sidereal_time = LAST_degrees();
	double A = PositiveDegree(180-azi);//Convention to measure westward from *south*
	double phi = m_Latitude;
	double a = alt;

	double local_Hour_Angle = R2D( atan( sin(D2R(A)) / ( cos(D2R(A))*sin(D2R(phi)) + tan(D2R(a))*cos(D2R(phi)) ) ) );
	//need to make sure we end up in the right coordinate after using atan:
	if ( (sin(D2R(A)) < 0)  && (cos(D2R(A))*sin(D2R(phi)) + tan(D2R(a))*cos(D2R(phi)) < 0)) {
		local_Hour_Angle = local_Hour_Angle - 180;
	} else if ( (sin(D2R(A)) > 0) && (cos(D2R(A))*sin(D2R(phi)) + tan(D2R(a))*cos(D2R(phi)) < 0)) {
		local_Hour_Angle = local_Hour_Angle + 180;
	}
	
	vector<double> radec;
	radec.push_back( PositiveDegree(local_sidereal_time + local_Hour_Angle));
	double dec = R2D( asin( sin(D2R(a))*sin(D2R(phi)) - cos(D2R(a))*cos(D2R(phi))*cos(D2R(A)) ) );
	radec.push_back( dec );

	return radec;
}


////////////////////////////////////////////////////////////////////////////////


// Find MJD at previous UT midnight
double MNCTTimeAndCoordinate::MJD_at_previous_midnight()
{
  return TMath::Floor(GetMJD());
}

////////////////////////////////////////////////////////////////////////////////

// Find hours since previous UT midnight
double MNCTTimeAndCoordinate::Hours_since_previous_midnight()
{
  return 24.0*(GetMJD()-MJD_at_previous_midnight());
}

////////////////////////////////////////////////////////////////////////////////

// Calculate the equation of equinoxes (to convert mean to apparent sidereal time)
double MNCTTimeAndCoordinate::Equation_of_equinoxes_hours()
{
  double eqeq, D, DeltaPsi, Omega, L, epsilon;
  // days since Jan 1 2000 12:00 UT
  D = GetMJD()-51544.5;
  // longitude of ascending node of moon (deg)
  Omega = 125.04 - 0.052954*D;
  // mean longitude of the Sun (deg)
  L = 280.47 + 0.98565*D;
  // obliquity (deg)
  epsilon = 23.4393 - 0.0000004*D;
  // nutation longitude (hours)
  DeltaPsi = -0.000319*TMath::Sin(D2R(Omega)) - 0.000024*TMath::Sin(D2R(2.0*L));
  // equation of equinoxes, from USNO
  eqeq = DeltaPsi*TMath::Cos(D2R(epsilon));
  //mout << "   EqnOfEqs: " << eqeq << endl;
  return eqeq;
}

////////////////////////////////////////////////////////////////////////////////

// Calculate Greenwich Mean Sidereal Time (GMST) from Modified Julian Date (MJD)
double MNCTTimeAndCoordinate::GMST_hours()
{
  double D, D0, H, T, GMST_hr;
  // days since Jan 1 2000 12:00 UT
  D = GetMJD()-51544.5;
  D0 = MJD_at_previous_midnight()-51544.5;
  // hours since previous midnight
  H = Hours_since_previous_midnight();
  // centuries since Jan 1 2000 12:00 UT
  T = D/36525.0;
  // formula from USNO, good to 0.1 sec
  GMST_hr = 6.697374558 + 0.06570982441908*D0 + 1.00273790935*H + 0.000026*T*T;
  //GMST_hr2 = 18.697374558 + 24.06570982441908*D;
  // convert GMST to range 0 to 24:
  GMST_hr = GMST_hr - 24.0*TMath::Floor(GMST_hr/24.0);
  //mout << "Calculation of Sidereal Time:" << endl;
  //mout << "   Unix:     " << setw(15) << setprecision(10) << GetUnixTime() << endl;
  //mout << "   MJD:      " << GetMJD() << endl;
  //mout << "   MJD0:     " << MJD_at_previous_midnight() << endl;
  //mout << "   H:        " << Hours_since_previous_midnight() << endl;
  //mout << "   D0:       " << D0 << endl;
  //mout << "   D:        " << D << endl;
  //mout << "   GMST(hr): " << GMST_hr << endl;
  return GMST_hr;
}

////////////////////////////////////////////////////////////////////////////////

// Calculate Greenwich Apparent Sidereal Time (GAST) in hours
double MNCTTimeAndCoordinate::GAST_hours()
{
  double GAST_hr;
  GAST_hr = GMST_hours() + Equation_of_equinoxes_hours();
  // ensure hour is in the range 0 to 24
  GAST_hr = GAST_hr - 24.0*TMath::Floor(GAST_hr/24.0);
  //mout << "   GAST(hr): " << GAST_hr << endl;
  return GAST_hr;
}

////////////////////////////////////////////////////////////////////////////////

// Calculate Greenwich Mean Sidereal Time (GMST) in degrees
double MNCTTimeAndCoordinate::GMST_degrees()
{
  double GMST_deg;
  GMST_deg = 15.0*GMST_hours();
  // ensure angle is in the range 0 to 360
  GMST_deg = GMST_deg - 360.0*TMath::Floor(GMST_deg/360.0);
  //mout << "   GMST(deg):" << GMST_deg << endl;
  return GMST_deg;
}

////////////////////////////////////////////////////////////////////////////////

// Calculate Greenwich Apparent Sidereal Time (GAST) in degrees
double MNCTTimeAndCoordinate::GAST_degrees()
{
  double GAST_deg;
  GAST_deg = 15.0*GAST_hours();
  // ensure angle is in the range 0 to 360
  GAST_deg = GAST_deg - 360.0*TMath::Floor(GAST_deg/360.0);
  //mout << "   GAST(deg):" << GAST_deg << endl;
  return GAST_deg;
}

////////////////////////////////////////////////////////////////////////////////

// Calculate Local Mean Sidereal Time (LMST) in degrees
double MNCTTimeAndCoordinate::LMST_degrees()
{
  double LMST_deg;
  LMST_deg = GMST_degrees() + m_Longitude;
  // ensure angle is in the range 0 to 360
  LMST_deg = LMST_deg - 360.0*TMath::Floor(LMST_deg/360.0);
  //mout << "   Lon(deg): " << m_Longitude << endl;
  //mout << "   LMST(deg):" << LMST_deg << endl;
  return LMST_deg;
}

////////////////////////////////////////////////////////////////////////////////

// Calculate Local Apparent Sidereal Time (LAST) in degrees
double MNCTTimeAndCoordinate::LAST_degrees()
{
  double LAST_deg;
  LAST_deg = GAST_degrees() + m_Longitude;
  // ensure angle is in the range 0 to 360
  LAST_deg = LAST_deg - 360.0*TMath::Floor(LAST_deg/360.0);
  //mout << "   Lon(deg): " << m_Longitude << endl;
  //mout << "   LAST(deg):" << LAST_deg << endl;
  return LAST_deg;
}

////////////////////////////////////////////////////////////////////////////////

// MNCTTimeAndCoordinate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
