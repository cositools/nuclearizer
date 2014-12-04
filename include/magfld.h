//
// Functions for calculating the magnetic field vector.
// Based on data from NOAA
// (http://www.ngdc.noaa.gov/seg/geomag/jsp/struts/igrfPointZip).
// The field components are:
//   Bx: north component, north is positive
//   By: east component, east is positive
//   Bz: vertical component, down is positive
// The _Hat versions give a unit vector in the direction of the field
// MEB, 2/15/05, based on SEB's magdec.pro code
//

#define MAXPATH 128

typedef struct {
   float Bx;
   float By;
   float Bz;
   float Bh;
   float Bf;
   float B_inc;
   float B_dec;
} WMMmodel;

// vector type
typedef struct {
	double x,y,z;
} vect;

// Magnitude of a vector
double Magnitude(vect v);

// dot product
double DotProd(vect v1,vect v2);

// cross product
vect CrossProd(vect v1,vect v2);

// normal vector in direction of v
vect Normal(vect v);

// v1 - v2
vect SubtractV(vect v1,vect v2);

// v1 + v2
vect AddV(vect v1,vect v2);

// c*v
vect ScalarMult(double c,vect v);


/*
 * A routine to interpolate the magnetic field vector
 * for the area around Ft. Sumner, NM for May 2005.
 */
vect MagField(double latitude, double longitude, double elevation);

vect MagField_Hat(double latitude, double longitude, double elevation);


/*
 * A routine to interpolate the magnetic field vector
 * at SSL for March 2005.
 */
vect MagFieldSSL(double latitude, double longitude, double elevation);

vect MagFieldSSL_Hat(double latitude, double longitude, double elevation);


float MagDec(float g_lat, float g_lon, float g_alt, float gcu_fracYear);

char WMMCalc(float alt,float glat,float glon, float fracYear, float *dec);
