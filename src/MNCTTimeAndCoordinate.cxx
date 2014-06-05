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


#ifdef ___CINT___
ClassImp(MNCTTimeAndCoordinate)
#endif



////////////////////////////////////////////////////////////////////////////////


const double MNCTTimeAndCoordinate::c_Day2Second = 86400.;
const double MNCTTimeAndCoordinate::c_Second2Day = 1./86400.;
const double MNCTTimeAndCoordinate::c_MJDJD = 2400000.5; //constant for converting MJD to JD
const double MNCTTimeAndCoordinate::c_UnixTimeMJD = 40587.0; //MJD when Unix time starts
const double MNCTTimeAndCoordinate::c_TAI2TT = 32.184; //TT = TAI+32.184
const double MNCTTimeAndCoordinate::c_LeapSeconds = 0.0; // Unix Time and GPS Time = UT + LeapSeconds
const double MNCTTimeAndCoordinate::c_GAngle1 = 282.85;
const double MNCTTimeAndCoordinate::c_GAngle2 = 62.9;
const double MNCTTimeAndCoordinate::c_GAngle3 = -33.0;


////////////////////////////////////////////////////////////////////////////////

MNCTTimeAndCoordinate::MNCTTimeAndCoordinate()
{
  m_MJDZero = c_UnixTimeMJD;
  m_TimeSinceMJDZero = 0;
  m_Longitude = 0;
  m_Latitude = 0;

  // initialize cryostat to dGPS rotation matrix
  m_Cryo_to_dGPS_Rotation.ResizeTo(3,3);
  m_Cryo_to_dGPS_Rotation[0][0] =  0.67165431;
  m_Cryo_to_dGPS_Rotation[0][1] = -0.74314688;
  m_Cryo_to_dGPS_Rotation[0][2] =  0.12958872;
  m_Cryo_to_dGPS_Rotation[1][0] =  0.74084079;
  m_Cryo_to_dGPS_Rotation[1][1] =  0.6461111;
  m_Cryo_to_dGPS_Rotation[1][2] = -0.1159381;
  m_Cryo_to_dGPS_Rotation[2][0] = -0.00116692;
  m_Cryo_to_dGPS_Rotation[2][1] =  0.17390276;
  m_Cryo_to_dGPS_Rotation[2][2] =  0.98451351;

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

//  cout << R2D(dir.Phi()) << ' ' << Zenith2ELV(R2D(dir.Theta())) <<'\n';

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

// Calculate the equation of equinoxes (converts mean to apparent sidereal time)
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

// Calculate GMST from MJD
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

// Calculate GAST in hours
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

// Calculate GMST in degrees
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

// Calculate GAST in degrees
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

// Calculate LMST in degrees
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

// Calculate LAST in degrees
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

// coordinate rotation from dGPS to Horizon coordinates
TMatrixD MNCTTimeAndCoordinate::dGPS_to_Horizon_Rotation(double pitch_deg, double roll_deg, double yaw_deg)
{
  TMatrixD PRY_Rotation(3,3);
  double sin_th = TMath::Sin(D2R(pitch_deg));
  double cos_th = TMath::Cos(D2R(pitch_deg));
  double sin_ps = TMath::Sin(D2R(roll_deg));
  double cos_ps = TMath::Cos(D2R(roll_deg));
  double sin_ph = TMath::Sin(D2R(yaw_deg));
  double cos_ph = TMath::Cos(D2R(yaw_deg));

  
  PRY_Rotation[0][0] = cos_th*cos_ph;
  PRY_Rotation[0][1] = sin_ps*sin_th*cos_ph + cos_ps*sin_ph;
  PRY_Rotation[0][2] = -cos_ps*sin_th*cos_ph + sin_ps*sin_ph;;
  PRY_Rotation[1][0] = -cos_th*sin_ph;
  PRY_Rotation[1][1] = -sin_ps*sin_th*sin_ph + cos_ps*cos_ph;;
  PRY_Rotation[1][2] = cos_ps*sin_th*sin_ph + sin_ps*cos_ph;;
  PRY_Rotation[2][0] = sin_th;
  PRY_Rotation[2][1] = -cos_th*sin_ps;
  PRY_Rotation[2][2] = cos_th*cos_ps;
  

/*  
  PRY_Rotation[0][0] = cos_th*cos_ph;
  PRY_Rotation[0][1] = cos_th*sin_ph;
  PRY_Rotation[0][2] =   -1.0*sin_th;
  PRY_Rotation[1][0] = sin_ps*sin_th*cos_ph - cos_ps*sin_ph;
  PRY_Rotation[1][1] = sin_ps*sin_th*sin_ph + cos_ps*cos_ph;
  PRY_Rotation[1][2] = cos_th*sin_ps;
  PRY_Rotation[2][0] = cos_ps*sin_th*cos_ph + sin_ps*sin_ph;
  PRY_Rotation[2][1] = cos_ps*sin_th*sin_ph - sin_ps*cos_ph;
  PRY_Rotation[2][2] = cos_th*cos_ps;
*/
  
  return PRY_Rotation;
}

////////////////////////////////////////////////////////////////////////////////

// coordinate rotation from dGPS to Horizon coordinates
TMatrixD MNCTTimeAndCoordinate::Cryo_to_Horizon_Rotation(double pitch_deg, double roll_deg, double yaw_deg)
{
  TMatrixD Full_Rotation = dGPS_to_Horizon_Rotation(pitch_deg, roll_deg, yaw_deg)*m_Cryo_to_dGPS_Rotation;
  return Full_Rotation;
}

////////////////////////////////////////////////////////////////////////////////

// convert cryostat vector into Horizon (azi, alt) coordinates
vector<double> MNCTTimeAndCoordinate::CryoVector_to_Horizon(TMatrixD Vector_cryo, double pitch_deg, double roll_deg, double yaw_deg)
{
  TMatrixD Vector_horizon = Cryo_to_Horizon_Rotation(pitch_deg,roll_deg,yaw_deg) * Vector_cryo;
  double azi = R2D(TMath::ATan2(-1.0*Vector_horizon[1][0], Vector_horizon[0][0]));
  double alt = R2D(TMath::ATan(Vector_horizon[2][0]
			       /TMath::Hypot(Vector_horizon[0][0],Vector_horizon[1][0])));
  vector<double> Coord_horizon;
  Coord_horizon.push_back(azi);
  Coord_horizon.push_back(alt);
  return Coord_horizon;
}

////////////////////////////////////////////////////////////////////////////////

// convert cryostat X axis into Horizon (azi, alt) coordinates
vector<double> MNCTTimeAndCoordinate::CryoX_to_Horizon(double pitch_deg, double roll_deg, double yaw_deg)
{
  TMatrixD CryoX(3,1);
  CryoX[0][0] = 1.;
  CryoX[1][0] = 0.;
  CryoX[2][0] = 0.;
  return CryoVector_to_Horizon(CryoX, pitch_deg, roll_deg, yaw_deg);
}

////////////////////////////////////////////////////////////////////////////////

// convert cryostat Z axis into Horizon (azi, alt) coordinates
vector<double> MNCTTimeAndCoordinate::CryoZ_to_Horizon(double pitch_deg, double roll_deg, double yaw_deg)
{
  TMatrixD CryoZ(3,1);
  CryoZ[0][0] = 0.;
  CryoZ[1][0] = 0.;
  CryoZ[2][0] = 1.;
  return CryoVector_to_Horizon(CryoZ, pitch_deg, roll_deg, yaw_deg);
}

////////////////////////////////////////////////////////////////////////////////

// MNCTTimeAndCoordinate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
