
#include "MString.h"
#include "MFile.h"

#include "magfld.h"
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <X11/Xlib.h>	// In order to include 'gse.h'
#include <sys/types.h>	// In order to include 'gse.h'
//#include "gse.h"

//***********************************************************************
// Private Variable Declarations
//***********************************************************************
#define WMM_MAXORD          12
//#ifdef NCT10DEV
  #define WMM_EPOCH           2010.
//#else
//  #define WMM_EPOCH           2005.
//#endif

//tolerances on lat and lon and altitude
#define WMM_LATTOL          0.2  /* degrees */
#define WMM_LONTOL          0.2
#define WMM_ALTTOL          10.     /* km */
#define DEG_TO_RAD          0.0174532925
#define RAD_TO_DEG          57.2957795

static float c[13][13],cd[13][13],tc[13][13],
            fn[13],fm[13],pp[13],k[13][13], epoch,snorm[169],
            // "o"ld values, to avoid unneccesary recomputation.
            oalt,olat,olon;
static float *p = snorm;

//extern hkp2data hkp2;
WMMmodel WMM;


// ***********************************************************************
// Private Function Declarations
// ***********************************************************************

void WMMInit(void);

float MagDec(float g_lat, float g_lon, float g_alt, float gcu_fracYear);


// ***********************************************************************
// Functions
// ***********************************************************************

double Magnitude(vect v)
{
	return sqrt( (v.x)*(v.x) + (v.y)*(v.y) + (v.z)*(v.z) );
}


// dot product
double DotProd(vect v1,vect v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}


// cross product
vect CrossProd(vect v1,vect v2)
{
	vect temp;
	temp.x = v1.y*v2.z - v1.z*v2.y;
	temp.y = v1.z*v2.x - v1.x*v2.z;
	temp.z = v1.x*v2.y - v1.y*v2.x;

	return temp;
}


// normal vector in direction of v
vect Normal(vect v)
{
	vect temp;
	double mag=Magnitude(v);

	temp.x = v.x/mag;
	temp.y = v.y/mag;
	temp.z = v.z/mag;

	return temp;
}


// v1 - v2
vect SubtractV(vect v1,vect v2)
{
	vect temp;

	temp.x = v1.x - v2.x;
	temp.y = v1.y - v2.y;
	temp.z = v1.z - v2.z;

	return temp;
}

// v1 + v2
vect AddV(vect v1,vect v2)
{
	vect temp;

	temp.x = v1.x + v2.x;
	temp.y = v1.y + v2.y;
	temp.z = v1.z + v2.z;

	return temp;
}

// c*v
vect ScalarMult(double c,vect v)
{
	vect temp;

	temp.x = c*v.x;
	temp.y = c*v.y;
	temp.z = c*v.z;

	return temp;
}




/*
 * A routine to interpolate the magnetic field vector
 * for the area around Ft. Sumner, NM for May 2005.
 */
vect MagField(double latitude, double longitude, double elevation)
{
	int i,j;
	vect B;

	/* elevation from ground in km (used for interpolation later) */
	static double z[2] = {0.,50.};
	/* north latitude */
	static double y[2] = {32.,36.};
	/* west longitude */
	// static double x[3] = {96.,104.,112.};

	/* Set up matrix for interpolation coefficients */
	/* This is the inverse of the matrix of longitude sampling values */
	/* A = {{1.,96.,96.*96.},{1.,104.,104.*104.},{1.,112.,112.*112}} */
	static double Ainv[3][3] = {{ 91.,-168.,78.},
		{-1.6875,3.25,-1.5625},
		{0.0078125,-0.015625,0.0078125}};

		/* set up matrix of data points */
		/* 5/1/05 data for the Ft. Sumner, NM area */
		static double Bx_t[2][2][3]={{{0.23819,0.24237,0.24615},   /*  0km, 32 deg N lat */
			{0.22082,0.22551,0.23043}},  /*  0km, 36 deg N lat */
			{{0.23227,0.23635,0.24008},   /* 50km, 32 deg N lat */
				{0.21537,0.21992,0.22472}}}; /* 50km, 36 deg N lat */

				static double By_t[2][2][3]={{{0.01784,0.03612,0.04930},   /*  0km, 32 deg N lat */
					{0.01614,0.03507,0.04919}},  /*  0km, 36 deg N lat */
					{{0.01719,0.03487,0.04768},   /* 50km, 32 deg N lat */
						{0.01554,0.03383,0.04752}}}; /* 50km, 36 deg N lat */

						static double Bz_t[2][2][3]={{{0.43705,0.42525,0.40786},   /*  0km, 32 deg N lat */
							{0.47238,0.46137,0.44446}},  /*  0km, 36 deg N lat */
							{{0.42602,0.41473,0.39804},   /* 50km, 32 deg N lat */
								{0.46039,0.44987,0.43366}}}; /* 50km, 36 deg N lat */

								double Bx_s[2][3],Bx_r[3],Bx_alpha[3];
								double By_s[2][3],By_r[3],By_alpha[3];
								double Bz_s[2][3],Bz_r[3],Bz_alpha[3];


								/* Linearly interpolate to the input elevation */
								for(i=0; i<2; i++)
								{
									for(j=0; j<3; j++)
									{
										Bx_s[i][j] = Bx_t[0][i][j] + (Bx_t[1][i][j]-Bx_t[0][i][j])*(elevation-z[0])/(z[1]-z[0]);
										By_s[i][j] = By_t[0][i][j] + (By_t[1][i][j]-By_t[0][i][j])*(elevation-z[0])/(z[1]-z[0]);
										Bz_s[i][j] = Bz_t[0][i][j] + (Bz_t[1][i][j]-Bz_t[0][i][j])*(elevation-z[0])/(z[1]-z[0]);
									}
								}

								/* Linearly interpolate to the input latitude */
								for (j=0; j<3; j++)
								{
									Bx_r[j] = Bx_s[0][j] + (Bx_s[1][j]-Bx_s[0][j])*(latitude-y[0])/(y[1]-y[0]);
									By_r[j] = By_s[0][j] + (By_s[1][j]-By_s[0][j])*(latitude-y[0])/(y[1]-y[0]);
									Bz_r[j] = Bz_s[0][j] + (Bz_s[1][j]-Bz_s[0][j])*(latitude-y[0])/(y[1]-y[0]);
								}

								/* Find the quadratic coefficients for the longitudinal dependence */
								/* This is a multiplication of the matrix A^(-1) with the vector rdec */

								for(i=0; i<3; i++)
								{
									Bx_alpha[i]=0.;
									By_alpha[i]=0.;
									Bz_alpha[i]=0.;
									for(j=0; j<3; j++)
									{
										Bx_alpha[i] += Ainv[i][j]*Bx_r[j];
										By_alpha[i] += Ainv[i][j]*By_r[j];
										Bz_alpha[i] += Ainv[i][j]*Bz_r[j];
									}
								}


								/* Quadratically interpolate to the input longitude */
								//  return alpha[0] + alpha[1]*longitude + alpha[2]*longitude*longitude;
								B.x = Bx_alpha[0] + Bx_alpha[1]*longitude + Bx_alpha[2]*longitude*longitude;
								B.y = By_alpha[0] + By_alpha[1]*longitude + By_alpha[2]*longitude*longitude;
								B.z = Bz_alpha[0] + Bz_alpha[1]*longitude + Bz_alpha[2]*longitude*longitude;

								return B;
}






vect MagField_Hat(double latitude, double longitude, double elevation)
{
	//vect B,B_Hat;
	//double BMag;

	//B = MagField(latitude,longitude,elevation);
	//BMag = Magnitude(&B);
	//B_Hat.x = B.x/BMag;
	//B_Hat.y = B.y/BMag;
	//B_Hat.z = B.z/BMag;

	return Normal(MagField(latitude,longitude,elevation));
}





/*
 * A routine to interpolate the magnetic field vector
 * at SSL for March 2005.
 */
vect MagFieldSSL(double latitude, double longitude, double elevation)
{
	int i,j;
	vect B;

	/* elevation from ground in km (used for interpolation later) */
	static double z[2] = {0.,1.};
	/* north latitude */
	static double y[2] = {37.5,38.};
	/* west longitude */
	static double x[2] = {121.5,122.5};

	/* set up matrix of data points */
	/* 3/1/05 data for the area around SSL */
	static double Bx_t[2][2][2]={{{0.22986,0.23040},   /*  0km, 37.5 deg N lat */
		{0.22790,0.22847}},  /*  0km, 38.0 deg N lat */
		{{0.22974,0.23028},   /*  1km, 37.5 deg N lat */
			{0.22778,0.22835}}}; /*  1km, 38.0 deg N lat */

			static double By_t[2][2][2]={{{0.06001,0.06080},   /*  0km, 37.5 deg N lat */
				{0.06006,0.06086}},  /*  0km, 38.0 deg N lat */
				{{0.05997,0.06086},   /*  1km, 37.5 deg N lat */
					{0.06002,0.06082}}}; /*  1km, 38.0 deg N lat */

					static double Bz_t[2][2][2]={{{0.43359,0.43089},   /*  0km, 37.5 deg N lat */
						{0.43799,0.43529}},  /*  0km, 38.0 deg N lat */
						{{0.43338,0.43068},   /*  1km, 37.5 deg N lat */
							{0.43778,0.43508}}}; /*  1km, 38.0 deg N lat */

							double Bx_s[2][2],Bx_r[2];
							double By_s[2][2],By_r[2];
							double Bz_s[2][2],Bz_r[2];


							/* Linearly interpolate to the input elevation */
							for(i=0; i<2; i++)
							{
								for(j=0; j<2; j++)
								{
									Bx_s[i][j] = Bx_t[0][i][j] + (Bx_t[1][i][j]-Bx_t[0][i][j])*(elevation-z[0])/(z[1]-z[0]);
									By_s[i][j] = By_t[0][i][j] + (By_t[1][i][j]-By_t[0][i][j])*(elevation-z[0])/(z[1]-z[0]);
									Bz_s[i][j] = Bz_t[0][i][j] + (Bz_t[1][i][j]-Bz_t[0][i][j])*(elevation-z[0])/(z[1]-z[0]);
								}
							}

							/* Linearly interpolate to the input latitude */
							for (j=0; j<2; j++)
							{
								Bx_r[j] = Bx_s[0][j] + (Bx_s[1][j]-Bx_s[0][j])*(latitude-y[0])/(y[1]-y[0]);
								By_r[j] = By_s[0][j] + (By_s[1][j]-By_s[0][j])*(latitude-y[0])/(y[1]-y[0]);
								Bz_r[j] = Bz_s[0][j] + (Bz_s[1][j]-Bz_s[0][j])*(latitude-y[0])/(y[1]-y[0]);
							}

							/* Linearly interpolate to the input longitude */
							B.x = Bx_r[0] + (Bx_r[1]-Bx_r[0])*(longitude-x[0])/(x[1]-x[0]);
							B.y = By_r[0] + (By_r[1]-By_r[0])*(longitude-x[0])/(x[1]-x[0]);
							B.z = Bz_r[0] + (Bz_r[1]-Bz_r[0])*(longitude-x[0])/(x[1]-x[0]);

							return B;
}




vect MagFieldSSL_Hat(double latitude, double longitude, double elevation)
{
	/*  vect B,B_Hat;
	    double BMag;

	    B = MagFieldSSL(latitude,longitude,elevation);
	    BMag = Magnitude(&B);
	    B_Hat.x = B.x/BMag;
	    B_Hat.y = B.y/BMag;
	    B_Hat.z = B.z/BMag;

	    return B_Hat;*/
	return Normal(MagField(latitude,longitude,elevation));
}






// ***********************************************************************
//  Functions to calculate magnetic field vectors 
//  (revised from Eric Bellm's version) (Alan Chiu) 
// ***********************************************************************

//Reads in WMM coefficients from file and stores them.
void WMMInit(void)
{

	int n,m,j,D1,D2;
	float gnm,hnm,dgnm,dhnm,flnmj;
	char model[20], c_str[81];

//	char MAXPATH=50;
	char wmmcoffilespec[MAXPATH];
	FILE *wmmdat;

	//inititialize constants
	*p = 1.0;

//#ifdef NCT10DEV
    MString WMMFile("$(NUCLEARIZER)/resource/aspect/WMM.COF");
	MFile::ExpandFileName(WMMFile);
	strncpy(wmmcoffilespec,WMMFile.Data(),MAXPATH-1);
//#else
	/*  WMM2005-2010.COF is identical to the released WMM.COF,
		but has the 9999 file ending removed */
	//sprintf(s,"%s/config/WMM2005-2010.COF",getenv("HOME"));
//	strncpy(wmmcoffilespec,"WMM2005-2010.COF",MAXPATH-1);
//#endif
	wmmcoffilespec[MAXPATH-1] = '\0';
	if ((wmmdat = fopen(wmmcoffilespec,"r")) == NULL)
	{
		printf(" Error opening file %s \n",wmmcoffilespec);
		strncpy(wmmcoffilespec,"NOFILE",MAXPATH - 1); /* kill the name */
		printf(" <WMM> Initializing..... failure! \n");
	}
	else
	{

		/* READ WORLD MAGNETIC MODEL SPHERICAL HARMONIC COEFFICIENTS */
		c[0][0] = 0.0;
		cd[0][0] = 0.0;

		if (fgets(c_str, 80, wmmdat) == NULL) {
      printf(" Error reading first data set \n");
      return;
    }
		sscanf(c_str,"%f%s",&epoch,model);

		while (fgets(c_str, 80, wmmdat) != NULL) {

			/* END OF FILE NOT ENCOUNTERED, GET VALUES */
			sscanf(c_str,"%d%d%f%f%f%f",&n,&m,&gnm,&hnm,&dgnm,&dhnm);

			//m and n need to be checked against the array size...

			if (m <= n) {
				c[m][n] = gnm;
				cd[m][n] = dgnm;
				if (m != 0) {
					c[n][m-1] = hnm;
					cd[n][m-1] = dhnm;
				}
			}
		}


		/* CONVERT SCHMIDT NORMALIZED GAUSS COEFFICIENTS TO UNNORMALIZED */
		*snorm = 1.0;
		for (n=1; n<=WMM_MAXORD; n++) {
			*(snorm+n) = *(snorm+n-1)*(float)(2*n-1)/(float)n;
			j = 2;
			for (m=0,D1=1,D2=(n-m+D1)/D1; D2>0; D2--,m+=D1) {
				k[m][n] = (float)(((n-1)*(n-1))-(m*m))/(float)((2*n-1)*(2*n-3));
				if (m > 0) {
					flnmj = (float)((n-m+1)*j)/(float)(n+m);
					*(snorm+n+m*13) = *(snorm+n+(m-1)*13)*sqrt(flnmj);
					j = 1;
					c[n][m-1] = *(snorm+n+m*13)*c[n][m-1];
					cd[n][m-1] = *(snorm+n+m*13)*cd[n][m-1];
				}
				c[m][n] = *(snorm+n+m*13)*c[m][n];
				cd[m][n] = *(snorm+n+m*13)*cd[m][n];
			}
			fn[n] = (float)(n+1);
			fm[n] = (float)n;
		}
		k[1][1] = 0.0;

		//Initialize these "last computed values" to unphysical values to
		//ensure we do all the computations the first time.
		oalt = olat = olon = -1000.0;
		fclose(wmmdat);

		printf(" <WMM> Initializing..... Succeed! \n");
	}
	return;
}

//Wrapper for WMMCalc that returns best current estimate of the declination
//  Only recomputes if we've exceeded our lat/lon tolerance
//float MagDec(void)
float MagDec(float g_lat, float g_lon, float g_alt, float gcu_fracYear)
{

    static float odec;
    float dec,fracYear;
    //POSVECT loc;
	float loc_lat,loc_lon,loc_alt;
    char errDec=1;  //default to keeping old value

    // Grab the curent lat, lon, and alt
    // should be degrees w/ N, E positive, alt in km
	loc_lat = g_lat;
	loc_lon = g_lon;
	loc_alt = g_alt/1000.;   //convert to km
	fracYear = gcu_fracYear;
	//printf(" loc_lat=%f, loc_lon=%f, loc_alt=%f, gcu_fracYear=%f \n",loc_lat,loc_lon,loc_alt,gcu_fracYear);

	// Only when the position is offset to a certain level did we do the calculation again 
    if (fabs(loc_lon - olon) > WMM_LONTOL || fabs(loc_lat - olat) > WMM_LATTOL
            || fabs(loc_alt - oalt) > WMM_ALTTOL) 
	{
        errDec = WMMCalc(loc_alt,loc_lat,loc_lon,fracYear,&dec);
    }
	//printf(" <magfld> MagDec=%f \n",dec);

    //keep new value only if WMMCalc doesn't throw an error
    //here's where we'd apply a running correction from the dGPS...
    if (errDec == 0) {
        odec = dec;
        // this is not sem-protected, so it could get munged.
        //StatGCU.Data.SubCom.SubCom3.MagDec = (short)(odec * 100.);
    }

    return(odec);
}

//Computes magnetic declination given input coordinates
char WMMCalc(float alt, float glat, float glon, float fracYear, float *dec)
{
    /*  Input units:
     *  Altitude:  km above mean sea level (WGS84)
     *  Latitude:  decimal degrees, North Postive
     *  Longitude: decimal degrees, East Positive
     *  Time:  decimal year (ie, 2009.0)
     *
     *  Returns:
     *  Declination:  degrees
     */

    int n,m;
    char D3,D4;
//  float a,b,RE, A2,B2,C2,A4,B4,C4;
    float dt,rlon,rlat,srlon,srlat,crlon,crlat,srlat2,
                 crlat2,q,q1,q2,ct,st,r2,r,d,ca,sa,aor,ar,br,bt,bp,bpp,
                 par,temp1,temp2,parp,bx,by,bz,bh,ti,dip;
    float dp[13][13],sp[13],cp[13];
    float warn_H_val, warn_H_strong_val;
    char warn_H, warn_H_strong, warn_P;


    warn_H = 0;
    warn_H_val = 99999.0;
    warn_H_strong = 0;
    warn_H_strong_val = 99999.0;
    warn_P = 0;

    //If WMM data has not been loaded, do so.
    if (fabs(epoch - WMM_EPOCH) > 0.1) WMMInit();

    dt = fracYear - epoch;
    /*  We'll skip the epoch check.
    if (otime < 0.0 && (dt < 0.0 || dt > 6.0)) {
        printf("\n\n WARNING - TIME EXTENDS BEYOND MODEL 5-YEAR LIFE SPAN");
        printf("\n         Web: http://www.ngdc.noaa.gov/seg/WMM/");
        printf("\n\n EPOCH  = %.3lf",epoch);
        printf("\n TIME   = %.3lf",time);
    }
    */

    /* INITIALIZE CONSTANTS */
    sp[0] = 0.0;
    cp[0] = *p = pp[0] = 1.0;
    dp[0][0] = 0.0;
/*      a = 6378.137;
    b = 6356.7523142;
    RE = 6371.2;
    A2 = a*a;
    B2 = b*b;
    C2 = A2-B2;
    A4 = A2*A2;
    B4 = B2*B2;
    C4 = A4 - B4;  */
//these are more digits of precision than the floats can use
#define A2  40680631.590769
#define B2  40408299.984087
#define C2  272331.606682
#define A4  1654913786623872.5
#define B4  632830707603970.
#define C4  22083079019902.5
#define RE  6371.2


    rlon = glon*DEG_TO_RAD;
    rlat = glat*DEG_TO_RAD;
    srlon = sin(rlon);
    srlat = sin(rlat);
    crlon = cos(rlon);
    crlat = cos(rlat);
    srlat2 = srlat*srlat;
    crlat2 = crlat*crlat;
    sp[1] = srlon;
    cp[1] = crlon;

    /* CONVERT FROM GEODETIC COORDS. TO SPHERICAL COORDS. */
    //if (alt != oalt || glat != olat) {
    q = sqrt(A2-C2*srlat2);
    q1 = alt*q;
    q2 = ((q1+A2)/(q1+B2))*((q1+A2)/(q1+B2));
    ct = srlat/sqrt(q2*crlat2+srlat2);
    st = sqrt(1.0-(ct*ct));
    r2 = (alt*alt)+2.0*q1+(A4-C4*srlat2)/(q*q);
    r = sqrt(r2);
    d = sqrt(A2*crlat2+B2*srlat2);
    ca = (alt+d)/r;
    sa = C2*crlat*srlat/(r*d);
    //}
    //if (glon != olon) {
    for (m=2; m<=WMM_MAXORD; m++) {
        sp[m] = sp[1]*cp[m-1]+cp[1]*sp[m-1];
        cp[m] = cp[1]*cp[m-1]-sp[1]*sp[m-1];
    }
    //}
    aor = RE/r;
    ar = aor*aor;
    br = bt = bp = bpp = 0.0;
    for (n=1; n<=WMM_MAXORD; n++) {
        ar = ar*aor;
        for (m=0,D3=1,D4=(n+m+D3)/D3; D4>0; D4--,m+=D3) {
            // COMPUTE UNNORMALIZED ASSOCIATED LEGENDRE POLYNOMIALS
            // AND DERIVATIVES VIA RECURSION RELATIONS
            //if (alt != oalt || glat != olat) {
            if (n == m) {
                *(p+n+m*13) = st**(p+n-1+(m-1)*13);
                dp[m][n] = st*dp[m-1][n-1]+ct**(p+n-1+(m-1)*13);
            }
            if (n == 1 && m == 0) {
                *(p+n+m*13) = ct**(p+n-1+m*13);
                dp[m][n] = ct*dp[m][n-1]-st**(p+n-1+m*13);
            }
            if (n > 1 && n != m) {
                if (m > n-2) *(p+n-2+m*13) = 0.0;
                if (m > n-2) dp[m][n-2] = 0.0;
                *(p+n+m*13) = ct**(p+n-1+m*13)-k[m][n]**(p+n-2+m*13);
                dp[m][n] = ct*dp[m][n-1] - st**(p+n-1+m*13)-k[m][n]*dp[m][n-2];
                }
            //}

            // TIME ADJUST THE GAUSS COEFFICIENTS
            //ECB another float comparison
            //if (fracYear != otime) {
            tc[m][n] = c[m][n]+dt*cd[m][n];
            if (m != 0) tc[n][m-1] = c[n][m-1]+dt*cd[n][m-1];
            //}

            // ACCUMULATE TERMS OF THE SPHERICAL HARMONIC EXPANSIONS
            par = ar**(p+n+m*13);
            if (m == 0) {
                temp1 = tc[m][n]*cp[m];
                temp2 = tc[m][n]*sp[m];
            } else {
                temp1 = tc[m][n]*cp[m]+tc[n][m-1]*sp[m];
                temp2 = tc[m][n]*sp[m]-tc[n][m-1]*cp[m];
            }
            bt = bt-ar*temp1*dp[m][n];
            bp += (fm[m]*temp2*par);
            br += (fn[n]*temp1*par);

            //  SPECIAL CASE:  NORTH/SOUTH GEOGRAPHIC POLES
            //  if st is consistent with zero, we're at the pole.
            //  absolute comparison with FLT_EPSILON is not strictly
            //  numerically correct, but it's close enough here.
            if (fabs(st) < FLT_EPSILON && m == 1) {
                if (n == 1) pp[n] = pp[n-1];
                else pp[n] = ct*pp[n-1]-k[m][n]*pp[n-2];
                parp = ar*pp[n];
                bpp += (fm[m]*temp2*parp);
            }
        }
    }
    if (fabs(st) < FLT_EPSILON) bp = bpp;
    else bp /= st;
    /*
       ROTATE MAGNETIC VECTOR COMPONENTS FROM SPHERICAL TO
       GEODETIC COORDINATES
     */
    WMM.Bx = bx = -bt*ca-br*sa;
    WMM.By = by = bp;
    WMM.Bz = bz = bt*sa-br*ca;
    /*
       COMPUTE DECLINATION (DEC), INCLINATION (DIP) AND
       TOTAL INTENSITY (TI)
     */
    WMM.Bh = bh = sqrt((bx*bx)+(by*by));
    WMM.Bf = ti = sqrt((bh*bh)+(bz*bz));
    WMM.B_inc = dip = atan2(bz,bh)/DEG_TO_RAD;
    WMM.B_dec = *dec = atan2(by,bx)/DEG_TO_RAD;
	//printf(" <WMM> Bx=%f, By=%f, Bz=%f, Bh=%f, Bf=%f, B_inc=%f, B_dec=%f \n",WMM.Bx,WMM.By,WMM.Bz,WMM.Bh,WMM.Bf,WMM.B_inc,WMM.B_dec);


    //For NCT08 flying from Australia, we should not be at |lat| > 55 degrees
    /*
       COMPUTE MAGNETIC GRID VARIATION IF THE CURRENT
       GEODETIC POSITION IS IN THE ARCTIC OR ANTARCTIC
       (I.E. GLAT > +55 DEGREES OR GLAT < -55 DEGREES)

       OTHERWISE, SET MAGNETIC GRID VARIATION TO -999.0
     */
    /*
    *gv = -999.0;
    if (fabs(glat) >= 55.) {
        if (glat > 0.0 && glon >= 0.0) *gv = *dec-glon;
        if (glat > 0.0 && glon < 0.0) *gv = *dec+fabs(glon);
        if (glat < 0.0 && glon >= 0.0) *gv = *dec+glon;
        if (glat < 0.0 && glon < 0.0) *gv = *dec-fabs(glon);
        if (*gv > +180.0) *gv -= 360.0;
        if (*gv < -180.0) *gv += 360.0;
    }
    */

    /* deal with geographic and magnetic poles */

    /* at geographic poles */
    /*  This won't be an issue unless we're in Antartica, in which case
        we need grid variation anyway. */
    /*
    if (90.0-fabs(dlat) <= 0.001) {
        dec = NAN;
        warn_P = 1;
        warn_H = 0;
        warn_H_strong = 0;
    }
    */

    /* at magnetic poles */
    //if (bh < 100.0) *dec = NAN;
    if (bh < 100.0) *dec = 0;

    if (bh < 1000.0) {
        warn_H = 0;
        warn_H_strong = 1;
        warn_H_strong_val = bh;
    } else if (bh < 5000.0 && !warn_H_strong) {
        warn_H = 1;
        warn_H_val = bh;
    }

    oalt = alt;
    olat = glat;
    olon = glon;

    //ECB  pass error codes here, but make sure to return a real value!
    //  the pointing table needs numbers, not error codes
    //  check the map--are we expecting these much?

    if (warn_H) {
        printf("\n\nWarning: The horizontal field strength at this location is only %6.1f nT\n",warn_H_val);
        printf("         Compass readings have large uncertainties in areas where H is\n");
        printf("         smaller than 5000 nT\n");
    }
    if (warn_H_strong) {
        printf("\n\nWarning: The horizontal field strength at this location is only %6.1f nT\n",warn_H_strong_val);
        printf("         Compass readings have VERY LARGE uncertainties in areas where H is\n");
        printf("         smaller than 1000 nT\n");
    }
    /*
    if (warn_P) {
        printf("\n\nWarning: Location is at geographic pole where X, Y, and Decl are undefined\n");
    }
    */

#ifdef PRINTALL
    printf("\n Results For \n");
    if (glat < 0)
        printf("\n LATITUDE:     %7.2fS",-glat);
    else
        printf("\n LATITUDE:     %7.2fN",glat);
    if (glon < 0)
        printf("\n LONGITUDE:    %7.2fW",-glon);
    else
        printf("\n LONGITUDE:    %7.2fE",glon);

    printf("\n ALTITUDE:    %8.2f METERS AMSL (WGS84)",alt*1000.);
    printf("\n DATE:         %6.1f\n",fracYear);

    printf("\n     Main Field");

    printf("\n F      =    %-9.1f nT",ti);
    if (isnan(bh))
        printf("\n H      =    NAN");
    else
        printf("\n H      =    %-9.1f nT",bh);
    if (isnan(bx))
        printf("\n X      =    NAN");
    else
        printf("\n X      =    %-9.1f nT",bx);
    if (isnan(by))
        printf("\n Y      =    NAN");
    else
        printf("\n Y      =    %-9.1f nT",by);
    printf("\n Z      =    %-9.1f nT",bz);
    if (isnan(*dec))
        printf("\n D      =    NAN");
    else
        printf("\n D      = %f Deg ",*dec);
    printf("\n I      = %f Deg \n",dip);
#endif

    return(warn_H || warn_H_strong || warn_P || isnan(*dec));
}


