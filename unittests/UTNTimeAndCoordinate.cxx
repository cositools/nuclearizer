/*
 * UTNTimeAndCoordinate.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <vector>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"

// Nuclearizer:
#include "MTimeAndCoordinate.h"


//! Unit test class for MTimeAndCoordinate
class UTNTimeAndCoordinate : public MUnitTest
{
 public:
  //! Default constructor
  UTNTimeAndCoordinate() : MUnitTest("UTNTimeAndCoordinate") {}
  //! Default destructor
  virtual ~UTNTimeAndCoordinate() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the default and the full constructor
  bool TestConstruction();
  //! Test the time setters/getters and their conversions
  bool TestTimeConversions();
  //! Test the location setters/getters
  bool TestLocation();
  //! Test the small angle helpers
  bool TestAngleHelpers();
  //! Test the sidereal time functions against reference values
  bool TestSiderealTime();
  //! Test Equatorial2Galactic() against catalog positions
  bool TestEquatorial2Galactic();
  //! Test Horizon2Equatorial() and Horizon2Galactic()
  bool TestHorizon2Equatorial();
  //! Test the hour angle quadrant handling and the right ascension range of Horizon2Equatorial()
  bool TestHorizon2EquatorialEdgeCases();

  //! Return the absolute difference of two angles in degrees taking the 360 degree wrap into account
  double AngleDifference(double A, double B);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNTimeAndCoordinate::Run()
{
  bool Passed = true;

  Passed = TestConstruction() && Passed;
  Passed = TestTimeConversions() && Passed;
  Passed = TestLocation() && Passed;
  Passed = TestAngleHelpers() && Passed;
  Passed = TestSiderealTime() && Passed;
  Passed = TestEquatorial2Galactic() && Passed;
  Passed = TestHorizon2Equatorial() && Passed;
  Passed = TestHorizon2EquatorialEdgeCases() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


double UTNTimeAndCoordinate::AngleDifference(double A, double B)
{
  double D = fmod(fabs(A - B), 360.0);
  if (D > 180.0) D = 360.0 - D;
  return D;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNTimeAndCoordinate::TestConstruction()
{
  bool Passed = true;

  MTimeAndCoordinate D;
  Passed = EvaluateNear("MTimeAndCoordinate()", "default", "The default MJD zero is the Unix epoch MJD 40587", D.GetMJDZero(), 40587.0, 1e-12) && Passed;
  Passed = EvaluateNear("MTimeAndCoordinate()", "default", "The default time since MJD zero is 0", D.GetTimeSinceMJDZero(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MTimeAndCoordinate()", "default", "The default longitude is 0", D.GetLongitude(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MTimeAndCoordinate()", "default", "The default latitude is 0", D.GetLatitude(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("MTimeAndCoordinate()", "default", "The default Unix time is 0", D.GetUnixTime(), 0.0, 1e-12) && Passed;

  MTimeAndCoordinate F(10.0, 51544.5, -122.25, 37.875);
  Passed = EvaluateNear("MTimeAndCoordinate(...)", "full constructor", "The time since MJD zero is 10 s", F.GetTimeSinceMJDZero(), 10.0, 1e-12) && Passed;
  Passed = EvaluateNear("MTimeAndCoordinate(...)", "full constructor", "The MJD zero is 51544.5", F.GetMJDZero(), 51544.5, 1e-12) && Passed;
  Passed = EvaluateNear("MTimeAndCoordinate(...)", "full constructor", "The longitude is -122.25", F.GetLongitude(), -122.25, 1e-12) && Passed;
  Passed = EvaluateNear("MTimeAndCoordinate(...)", "full constructor", "The latitude is 37.875", F.GetLatitude(), 37.875, 1e-12) && Passed;
  Passed = EvaluateNear("MTimeAndCoordinate(...)", "full constructor", "The Unix time is 946728010 s", F.GetUnixTime(), 946728010.0, 1e-6) && Passed;

  MTimeAndCoordinate U(100.0);
  Passed = EvaluateNear("MTimeAndCoordinate(double)", "time only", "The MJD zero defaults to the Unix epoch", U.GetMJDZero(), 40587.0, 1e-12) && Passed;
  Passed = EvaluateNear("MTimeAndCoordinate(double)", "time only", "The Unix time is 100 s", U.GetUnixTime(), 100.0, 1e-9) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNTimeAndCoordinate::TestTimeConversions()
{
  bool Passed = true;

  MTimeAndCoordinate T;
  // 2000-01-01 12:00:00 UTC
  T.SetUnixTime(946728000.0);
  Passed = EvaluateNear("SetUnixTime/GetMJD", "J2000 epoch", "Unix time 946728000 is MJD 51544.5", T.GetMJD(), 51544.5, 1e-9) && Passed;
  Passed = EvaluateNear("SetUnixTime/GetUnixTime", "J2000 epoch", "GetUnixTime returns the set Unix time", T.GetUnixTime(), 946728000.0, 1e-6) && Passed;
  Passed = EvaluateNear("SetUnixTime/GetTimeSinceMJDZero", "J2000 epoch", "The time since MJD zero equals the Unix time for the default MJD zero", T.GetTimeSinceMJDZero(), 946728000.0, 1e-6) && Passed;

  T.SetMJD(60310.0);
  Passed = EvaluateNear("SetMJD/GetUnixTime", "2024-01-01", "MJD 60310 is Unix time 1704067200", T.GetUnixTime(), 1704067200.0, 1e-4) && Passed;
  Passed = EvaluateNear("SetMJD/GetMJD", "2024-01-01", "GetMJD returns the set MJD", T.GetMJD(), 60310.0, 1e-9) && Passed;

  T.SetTimeSinceMJDZero(43200.0);
  Passed = EvaluateNear("SetTimeSinceMJDZero/GetTimeSinceMJDZero", "half a day", "GetTimeSinceMJDZero returns 43200 s", T.GetTimeSinceMJDZero(), 43200.0, 1e-12) && Passed;
  Passed = EvaluateNear("SetTimeSinceMJDZero/GetMJD", "half a day", "Half a day after the Unix epoch is MJD 40587.5", T.GetMJD(), 40587.5, 1e-12) && Passed;

  // Changing MJD zero changes the reference, the Unix time is set relative to the new reference
  MTimeAndCoordinate R;
  R.SetMJDZero(51544.5);
  Passed = EvaluateNear("SetMJDZero/GetMJDZero", "51544.5", "GetMJDZero returns 51544.5", R.GetMJDZero(), 51544.5, 1e-12) && Passed;
  R.SetUnixTime(946728000.0 + 60.0);
  Passed = EvaluateNear("SetUnixTime()", "MJD zero at J2000", "The time since MJD zero is 60 s", R.GetTimeSinceMJDZero(), 60.0, 1e-6) && Passed;
  Passed = EvaluateNear("GetMJD()", "MJD zero at J2000", "The MJD is J2000 plus one minute", R.GetMJD(), 51544.5 + 60.0/86400.0, 1e-9) && Passed;
  R.SetMJD(51545.5);
  Passed = EvaluateNear("SetMJD()", "MJD zero at J2000", "One day after MJD zero is 86400 s", R.GetTimeSinceMJDZero(), 86400.0, 1e-6) && Passed;

  // Times before the epoch
  MTimeAndCoordinate N;
  N.SetUnixTime(-86400.0);
  Passed = EvaluateNear("SetUnixTime/GetMJD", "one day before the Unix epoch", "Unix time -86400 is MJD 40586", N.GetMJD(), 40586.0, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNTimeAndCoordinate::TestLocation()
{
  bool Passed = true;

  // SetLocation takes latitude first, then longitude
  MTimeAndCoordinate T;
  T.SetLocation(37.875, -122.25);
  Passed = EvaluateNear("SetLocation/GetLatitude", "37.875, -122.25", "GetLatitude returns the first argument", T.GetLatitude(), 37.875, 1e-12) && Passed;
  Passed = EvaluateNear("SetLocation/GetLongitude", "37.875, -122.25", "GetLongitude returns the second argument", T.GetLongitude(), -122.25, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNTimeAndCoordinate::TestAngleHelpers()
{
  bool Passed = true;

  MTimeAndCoordinate T;
  Passed = EvaluateNear("ELV2Zenith()", "30", "An elevation of 30 deg is a zenith angle of 60 deg", T.ELV2Zenith(30.0), 60.0, 1e-12) && Passed;
  Passed = EvaluateNear("ELV2Zenith()", "-10", "An elevation of -10 deg is a zenith angle of 100 deg", T.ELV2Zenith(-10.0), 100.0, 1e-12) && Passed;
  Passed = EvaluateNear("Zenith2ELV()", "60", "A zenith angle of 60 deg is an elevation of 30 deg", T.Zenith2ELV(60.0), 30.0, 1e-12) && Passed;
  Passed = EvaluateNear("Zenith2ELV(ELV2Zenith())", "17.5", "The elevation round trip is the identity", T.Zenith2ELV(T.ELV2Zenith(17.5)), 17.5, 1e-12) && Passed;

  Passed = EvaluateNear("NegativeDegree()", "270", "270 deg maps to -90 deg", T.NegativeDegree(270.0), -90.0, 1e-12) && Passed;
  Passed = EvaluateNear("NegativeDegree()", "180", "180 deg stays 180 deg", T.NegativeDegree(180.0), 180.0, 1e-12) && Passed;
  Passed = EvaluateNear("NegativeDegree()", "45", "45 deg stays 45 deg", T.NegativeDegree(45.0), 45.0, 1e-12) && Passed;
  Passed = EvaluateNear("NegativeDegree()", "359.5", "359.5 deg maps to -0.5 deg", T.NegativeDegree(359.5), -0.5, 1e-12) && Passed;
  Passed = EvaluateNear("PositiveDegree()", "-90", "-90 deg maps to 270 deg", T.PositiveDegree(-90.0), 270.0, 1e-12) && Passed;
  Passed = EvaluateNear("PositiveDegree()", "0", "0 deg stays 0 deg", T.PositiveDegree(0.0), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("PositiveDegree()", "-180", "-180 deg maps to 180 deg", T.PositiveDegree(-180.0), 180.0, 1e-12) && Passed;
  Passed = EvaluateNear("PositiveDegree(NegativeDegree())", "200", "The degree round trip is the identity", T.PositiveDegree(T.NegativeDegree(200.0)), 200.0, 1e-12) && Passed;

  Passed = EvaluateNear("R2D()", "pi", "pi rad is 180 deg", T.R2D(TMath::Pi()), 180.0, 1e-12) && Passed;
  Passed = EvaluateNear("D2R()", "90", "90 deg is pi/2 rad", T.D2R(90.0), TMath::Pi()/2, 1e-12) && Passed;
  Passed = EvaluateNear("R2D(D2R())", "123.456", "The angle round trip is the identity", T.R2D(T.D2R(123.456)), 123.456, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNTimeAndCoordinate::TestSiderealTime()
{
  bool Passed = true;

  // J2000.0: 2000-01-01 12:00 UT, GMST = 18.697374558 h (USNO)
  MTimeAndCoordinate J;
  J.SetUnixTime(946728000.0);
  Passed = EvaluateNear("MJD_at_previous_midnight()", "J2000", "The previous midnight is MJD 51544", J.MJD_at_previous_midnight(), 51544.0, 1e-12) && Passed;
  Passed = EvaluateNear("Hours_since_previous_midnight()", "J2000", "12 hours passed since midnight", J.Hours_since_previous_midnight(), 12.0, 1e-6) && Passed;
  Passed = EvaluateNear("GMST_hours()", "J2000", "GMST is 18.697374558 h", J.GMST_hours(), 18.697374558, 1e-6) && Passed;
  Passed = EvaluateNear("GMST_degrees()", "J2000", "GMST is 280.46061837 deg", J.GMST_degrees(), 280.46061837, 1e-5) && Passed;

  double EqEq = J.Equation_of_equinoxes_hours();
  Passed = EvaluateTrue("Equation_of_equinoxes_hours()", "J2000", "The equation of the equinoxes is smaller than 1.2 s", fabs(EqEq) < 1.2/3600.0) && Passed;
  Passed = EvaluateNear("GAST_hours()", "J2000", "GAST is GMST plus the equation of the equinoxes", J.GAST_hours(), J.GMST_hours() + EqEq, 1e-9) && Passed;
  Passed = EvaluateNear("GAST_degrees()", "J2000", "GAST in degrees is 15 times GAST in hours", J.GAST_degrees(), 15.0*J.GAST_hours(), 1e-9) && Passed;

  // At midnight the hours since midnight are 0
  MTimeAndCoordinate M;
  M.SetMJD(60310.0);
  Passed = EvaluateNear("Hours_since_previous_midnight()", "2024-01-01 00:00", "0 hours passed since midnight", M.Hours_since_previous_midnight(), 0.0, 1e-6) && Passed;
  // 2024-01-01 0h UT: GMST = 6h 40m 36.6s
  Passed = EvaluateNear("GMST_hours()", "2024-01-01 00:00", "GMST is 6.67684 h", M.GMST_hours(), 6.67684, 1e-4) && Passed;
  Passed = EvaluateTrue("GMST_hours()", "2024-01-01 00:00", "GMST is not negative", M.GMST_hours() >= 0) && Passed;
  Passed = EvaluateTrue("GMST_hours()", "2024-01-01 00:00", "GMST is smaller than 24 h", M.GMST_hours() < 24) && Passed;

  // Local sidereal times
  MTimeAndCoordinate L;
  L.SetUnixTime(946728000.0);
  L.SetLocation(37.875, -122.25);
  double ExpectedLMST = J.GMST_degrees() - 122.25;
  if (ExpectedLMST < 0) ExpectedLMST += 360.0;
  Passed = EvaluateNear("LMST_degrees()", "J2000, longitude -122.25", "LMST is GMST plus the longitude", L.LMST_degrees(), ExpectedLMST, 1e-9) && Passed;
  double ExpectedLAST = J.GAST_degrees() - 122.25;
  if (ExpectedLAST < 0) ExpectedLAST += 360.0;
  Passed = EvaluateNear("LAST_degrees()", "J2000, longitude -122.25", "LAST is GAST plus the longitude", L.LAST_degrees(), ExpectedLAST, 1e-9) && Passed;

  MTimeAndCoordinate E;
  E.SetUnixTime(946728000.0);
  E.SetLocation(0.0, 100.0);
  Passed = EvaluateNear("LMST_degrees()", "J2000, longitude 100", "LMST wraps into [0, 360)", E.LMST_degrees(), 280.46061837 + 100.0 - 360.0, 1e-5) && Passed;

  // One sidereal day later the GMST is the same
  MTimeAndCoordinate S;
  S.SetUnixTime(946728000.0 + 86164.0905);
  Passed = EvaluateNear("GMST_hours()", "J2000 plus one sidereal day", "GMST repeats after one sidereal day", S.GMST_hours(), 18.697374558, 1e-4) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNTimeAndCoordinate::TestEquatorial2Galactic()
{
  bool Passed = true;

  // J2000 catalog values: right ascension, declination, galactic longitude, galactic latitude
  vector<MString> Names = { "Crab", "Cyg X-1", "Vela pulsar", "Sco X-1", "Cas A", "Galactic center", "Galactic anti-center" };
  vector<vector<double>> Catalog = {
    { 83.63308, 22.01450, 184.5575, -5.7843 },
    { 299.59032, 35.20161, 71.3350, 3.0668 },
    { 128.83588, -45.17635, 263.5520, -2.7873 },
    { 244.97955, -15.64022, 359.0942, 23.7840 },
    { 350.85000, 58.81500, 111.7340, -2.1300 },
    { 266.40499, -28.93617, 0.0, 0.0 },
    { 86.40499, 28.93617, 180.0, 0.0 }
  };

  MTimeAndCoordinate T;
  for (unsigned int s = 0; s < Names.size(); ++s) {
    vector<double> LB = T.Equatorial2Galactic({ Catalog[s][0], Catalog[s][1] });
    Passed = EvaluateSize("Equatorial2Galactic()", Names[s], "The result has two elements", LB.size(), 2) && Passed;
    if (LB.size() == 2) {
      // The documented accuracy is 0.01 degrees
      Passed = EvaluateNear("Equatorial2Galactic()", Names[s], "The galactic longitude matches the catalog value within 0.02 deg", AngleDifference(LB[0], Catalog[s][2]), 0.0, 0.02) && Passed;
      Passed = EvaluateNear("Equatorial2Galactic()", Names[s], "The galactic latitude matches the catalog value within 0.02 deg", LB[1], Catalog[s][3], 0.02) && Passed;
      Passed = EvaluateTrue("Equatorial2Galactic()", Names[s], "The galactic longitude is not negative", LB[0] >= 0.0) && Passed;
      Passed = EvaluateTrue("Equatorial2Galactic()", Names[s], "The galactic longitude is smaller than 360 deg", LB[0] < 360.0) && Passed;
    }
  }

  // At the galactic north pole the longitude is arbitrary but must be a number
  vector<double> NGP = T.Equatorial2Galactic({ 192.85, 27.128333333 });
  Passed = EvaluateSize("Equatorial2Galactic()", "galactic north pole", "The result has two elements", NGP.size(), 2) && Passed;
  if (NGP.size() == 2) {
    Passed = EvaluateNear("Equatorial2Galactic()", "galactic north pole", "The galactic latitude is 90 deg", NGP[1], 90.0, 1e-6) && Passed;
    Passed = EvaluateTrue("Equatorial2Galactic()", "galactic north pole", "The galactic longitude is finite", std::isfinite(NGP[0])) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNTimeAndCoordinate::TestHorizon2Equatorial()
{
  bool Passed = true;

  MTimeAndCoordinate T;
  T.SetUnixTime(946728000.0);
  T.SetLocation(37.0, -122.0);
  double LAST = T.LAST_degrees();

  // Zenith: declination equals latitude, right ascension equals LAST
  vector<double> Zenith = T.Horizon2Equatorial(0.0, 90.0);
  Passed = EvaluateSize("Horizon2Equatorial()", "zenith", "The result has two elements", Zenith.size(), 2) && Passed;
  if (Zenith.size() == 2) {
    Passed = EvaluateNear("Horizon2Equatorial()", "zenith", "The declination of the zenith is the latitude", Zenith[1], 37.0, 1e-6) && Passed;
    Passed = EvaluateNear("Horizon2Equatorial()", "zenith", "The right ascension of the zenith is LAST", AngleDifference(Zenith[0], LAST), 0.0, 1e-6) && Passed;
  }

  // Meridian south at altitude 40: hour angle 0, dec = alt + lat - 90
  vector<double> South = T.Horizon2Equatorial(180.0, 40.0);
  Passed = EvaluateSize("Horizon2Equatorial()", "south meridian, alt 40", "The result has two elements", South.size(), 2) && Passed;
  if (South.size() == 2) {
    Passed = EvaluateNear("Horizon2Equatorial()", "south meridian, alt 40", "The declination is -13 deg", South[1], -13.0, 1e-6) && Passed;
    Passed = EvaluateNear("Horizon2Equatorial()", "south meridian, alt 40", "The right ascension is LAST", AngleDifference(South[0], LAST), 0.0, 1e-6) && Passed;
  }

  // Meridian north at altitude 50: dec = 90 - (alt - lat) = 77, hour angle 0
  vector<double> North = T.Horizon2Equatorial(0.0, 50.0);
  Passed = EvaluateSize("Horizon2Equatorial()", "north meridian, alt 50", "The result has two elements", North.size(), 2) && Passed;
  if (North.size() == 2) {
    Passed = EvaluateNear("Horizon2Equatorial()", "north meridian, alt 50", "The declination is 77 deg", North[1], 77.0, 1e-6) && Passed;
    Passed = EvaluateNear("Horizon2Equatorial()", "north meridian, alt 50", "The right ascension is LAST", AngleDifference(North[0], LAST), 0.0, 1e-6) && Passed;
  }

  // General case, azimuth measured from north through east, compared to the standard formulae
  const vector<double> Azimuths = { 30.0, 120.0, 210.0, 300.0 };
  for (double Az : Azimuths) {
    double Alt = 25.0;
    double Phi = T.D2R(37.0);
    double A = T.D2R(Az);
    double H = T.D2R(Alt);
    double Dec = asin(sin(H)*sin(Phi) + cos(H)*cos(Phi)*cos(A));
    double HA = atan2(-sin(A)*cos(H), cos(Phi)*sin(H) - sin(Phi)*cos(H)*cos(A));
    double RA = T.PositiveDegree(fmod(LAST - T.R2D(HA) + 360.0, 360.0));

    vector<double> R = T.Horizon2Equatorial(Az, Alt);
    MString Input = MString("azimuth ") + Az + ", alt 25";
    Passed = EvaluateSize("Horizon2Equatorial()", Input, "The result has two elements", R.size(), 2) && Passed;
    if (R.size() == 2) {
      Passed = EvaluateNear("Horizon2Equatorial()", Input, "The declination matches the standard formula", R[1], T.R2D(Dec), 1e-6) && Passed;
      Passed = EvaluateNear("Horizon2Equatorial()", Input, "The right ascension matches the standard formula", AngleDifference(R[0], RA), 0.0, 1e-6) && Passed;
      Passed = EvaluateTrue("Horizon2Equatorial()", Input, "The right ascension is not negative", R[0] >= 0.0) && Passed;
      Passed = EvaluateTrue("Horizon2Equatorial()", Input, "The right ascension is smaller than 360 deg", R[0] < 360.0) && Passed;
    }
  }

  // Horizon2Galactic is the concatenation of both conversions
  vector<double> Eq = T.Horizon2Equatorial(120.0, 25.0);
  vector<double> GalExpected = T.Equatorial2Galactic(Eq);
  vector<double> Gal = T.Horizon2Galactic(120.0, 25.0);
  Passed = EvaluateSize("Horizon2Galactic()", "azimuth 120, alt 25", "The result has two elements", Gal.size(), 2) && Passed;
  Passed = EvaluateSize("Equatorial2Galactic()", "azimuth 120, alt 25", "The reference result has two elements", GalExpected.size(), 2) && Passed;
  if (Gal.size() == 2 && GalExpected.size() == 2) {
    Passed = EvaluateNear("Horizon2Galactic()", "azimuth 120, alt 25", "Horizon2Galactic longitude equals Equatorial2Galactic(Horizon2Equatorial())", Gal[0], GalExpected[0], 1e-9) && Passed;
    Passed = EvaluateNear("Horizon2Galactic()", "azimuth 120, alt 25", "Horizon2Galactic latitude equals Equatorial2Galactic(Horizon2Equatorial())", Gal[1], GalExpected[1], 1e-9) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNTimeAndCoordinate::TestHorizon2EquatorialEdgeCases()
{
  bool Passed = true;

  // Due south (azimuth exactly 180) below the celestial pole: the result must be continuous with azimuths 179.999 and 180.001
  MTimeAndCoordinate T;
  T.SetUnixTime(946728000.0);
  T.SetLocation(-45.0, 170.0);
  vector<double> South = T.Horizon2Equatorial(180.0, 20.0);
  vector<double> Left = T.Horizon2Equatorial(179.999, 20.0);
  vector<double> Right = T.Horizon2Equatorial(180.001, 20.0);
  Passed = EvaluateSize("Horizon2Equatorial()", "latitude -45, azimuth 180", "The result has two elements", South.size(), 2) && Passed;
  Passed = EvaluateSize("Horizon2Equatorial()", "latitude -45, azimuth 179.999", "The result has two elements", Left.size(), 2) && Passed;
  Passed = EvaluateSize("Horizon2Equatorial()", "latitude -45, azimuth 180.001", "The result has two elements", Right.size(), 2) && Passed;
  if (South.size() == 2 && Left.size() == 2 && Right.size() == 2) {
    Passed = ExpectedToFail(EvaluateNear("Horizon2Equatorial()", "latitude -45, alt 20, azimuth 180 vs 179.999", "The right ascension is continuous at azimuth 180", AngleDifference(South[0], Left[0]), 0.0, 0.01)) && Passed;
    Passed = ExpectedToFail(EvaluateNear("Horizon2Equatorial()", "latitude -45, alt 20, azimuth 180 vs 180.001", "The right ascension is continuous at azimuth 180", AngleDifference(South[0], Right[0]), 0.0, 0.01)) && Passed;
  }

  // The right ascension is always in [0, 360), for all sidereal times and azimuths
  bool InRange = true;
  MString FirstOutOfRange;
  MTimeAndCoordinate R;
  R.SetLocation(34.0, -118.0);
  for (unsigned int h = 0; h < 24; ++h) {
    R.SetUnixTime(946728000.0 + 3600.0*h);
    for (double Az = 0.0; Az < 360.0; Az += 30.0) {
      vector<double> RD = R.Horizon2Equatorial(Az, 30.0);
      if (RD.size() != 2 || RD[0] < 0.0 || RD[0] >= 360.0) {
        if (InRange == true) FirstOutOfRange = MString("hour ") + h + ", azimuth " + Az + ": RA " + (RD.size() == 2 ? RD[0] : -1.0);
        InRange = false;
      }
    }
  }
  if (InRange == false) mout<<"Horizon2Equatorial(): first right ascension out of range at "<<FirstOutOfRange<<endl;
  Passed = ExpectedToFail(EvaluateTrue("Horizon2Equatorial()", "24 sidereal times x 12 azimuths", "The right ascension is always in [0, 360)", InRange)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNTimeAndCoordinate Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNTimeAndCoordinate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
