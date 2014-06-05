/*
 * MNCTMath.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
// MNCTMath
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTMath.h"

// Standard libs:
#include <cmath>


// ROOT libs:
#include "Math/SpecFunc.h"
#include "TRandom.h"
#include "TMatrixD.h"
//#include "Math/Interpolator.h" //MathMore library
//#include "Math/InterpolationTypes.h" //MathMore library

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTMath)
#endif



////////////////////////////////////////////////////////////////////////////////

double MNCTMath::gaussint(double x){return (0.5 + 0.5*ROOT::Math::erf(x/sqrt(2.0)));}

////////////////////////////////////////////////////////////////////////////////

double MNCTMath::discretize(double input, double unit, double error)
{
  double x=0,e=0;
  e = (error==0)?0:gRandom->Uniform(-error/2./unit,error/2./unit);
  x = input/unit + e;
  return (int)x * unit;
}

////////////////////////////////////////////////////////////////////////////////


int MNCTMath::fact(int m) 
{  
  // m is a positive integer. The function returns its factorial
  int lcv;    // loop control variable
  int p;      // set to the product of the first lcv positive integers
  
  for(p=1, lcv=2; lcv <= m; p=p*lcv, lcv++);  
  return p;  
}  


////////////////////////////////////////////////////////////////////////////////


int MNCTMath::comb(int n, int k) 
{   
  // n and k are positive integers. The function returns the he number of k-combinations (each of size k) from a set S with n elements (size n)
  int c_nk;       
  
  c_nk=fact(n)/(fact(k)*fact(n-k));  
  return c_nk;  
}  

////////////////////////////////////////////////////////////////////////////////

// restrict angle to range [0,360) by adding or subtracting 360
double MNCTMath::angle360(double angle)
{
  double angle_trunc = angle;
  while (angle_trunc>=360.)
    {
      angle_trunc -= 360.;
    }
  while (angle_trunc<0.)
    {
      angle_trunc += 360.;
    }
  return angle_trunc;
}

////////////////////////////////////////////////////////////////////////////////

double MNCTMath::linear_interpolate(double x_interpolate, double x_0, double x_1,
				    double y_0, double y_1)
{
  if (x_0 != x_1)
    {
      return y_0+(y_1-y_0)/(x_1-x_0)*(x_interpolate-x_0);
    }
  else
    {
      return 0.5*(y_0+y_1);
    }
}

////////////////////////////////////////////////////////////////////////////////

// linear interpolation of an angle where the angle is restricted to [0,360).
double MNCTMath::linear_interpolate_angle360(double angle_1, double angle_2, double fraction)
{
  double angle=0.;
  // make sure angles are in range [0,360)
  double angle1 = MNCTMath::angle360(angle_1);
  double angle2 = MNCTMath::angle360(angle_2);

  if (angle2-angle1<=-180.)
    {
      // angles go from 360 to 0
      angle = MNCTMath::linear_interpolate(angle1, angle2+360., fraction);      
    }
  else if (angle2-angle1>=180.)
    {
      // angles go from 0 to 360
      angle = MNCTMath::linear_interpolate(angle1, angle2-360., fraction);      
    }
  else
    {
      // case where there is no rollover
      angle = MNCTMath::linear_interpolate(angle1, angle2, fraction);      
    }
  // ensure angle is still in range
  angle = MNCTMath::angle360(angle);

  return angle;
}

////////////////////////////////////////////////////////////////////////////////

double MNCTMath::linear_interpolate_angle360(double x_interpolate,
					     double x_0, double x_1,
					     double angle_0, double angle_1)
{
  // ensure angles are in range [0,360)
  double angle0 = MNCTMath::angle360(angle_0);
  double angle1 = MNCTMath::angle360(angle_1);

  // interpolate the angle
  double angle=0.;
  if (angle1-angle0<=-180.)
    {
      // angles go from 360 to 0
      angle = MNCTMath::linear_interpolate(x_interpolate, x_0, x_1,
					   angle0, angle1+360.);
    }
  else if (angle1-angle0>=180.)
    {
      // angles go from 0 to 360
      angle = MNCTMath::linear_interpolate(x_interpolate, x_0, x_1,
					   angle0, angle1-360.);
    }
  else
    {
      // case where there is no rollover
      angle = MNCTMath::linear_interpolate(x_interpolate, x_0, x_1,
					   angle0, angle1);
    }

  // ensure angle is still in range [0, 360)
  angle = MNCTMath::angle360(angle);

  return angle;
}

////////////////////////////////////////////////////////////////////////////////

// MNCTMath.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
