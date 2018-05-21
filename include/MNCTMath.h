

#ifndef __MNCTMath__
#define __MNCTMath__

// Standard libs:
#include <cmath>
#include <vector>
using namespace std;

// ROOT libs:
#include "Math/SpecFunc.h"
#include "TMath.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////



class MNCTMath
{
 public:
  MNCTMath() {};
  virtual ~MNCTMath() {};
   
  static double gaussint(double x);
//  static double interpol(const vector<double> &VectorY, const vector<double> &VectorX, double x);

  static double alog10(double x){return TMath::Log10(x);}

  //
  static double FWHM2sigma(){return 1./2.3548;}

  // discretize the input value into N*unit
  static double discretize(double input, double unit = 1, double error = 0);

  // restrict angle to range [0,360) by adding or subtracting 360
  static double angle360(double angle);

  // linear interpolation
  static double linear_interpolate(double a, double b, double fraction){return fraction*(b-a)+a;}
  static double linear_interpolate(double x_interpolate, double x_0, double x_1,
				   double y_0, double y_1);

  // linear interpolation of an angle where the angle is restricted to [0,360).
  static double linear_interpolate_angle360(double angle1, double angle2, double fraction);
  static double linear_interpolate_angle360(double x0, vector<double> x, vector<double> angles);
  static double linear_interpolate_angle360(double x_interpolate,
					    double x_0, double x_1,
					    double angle_0, double angle_1);

  /// Description
  static int fact(int m);

  /// Description
  static int comb(int n, int k);

 protected:

 private:


#ifdef ___CLING___
 public:
  ClassDef(MNCTMath, 0) // no description
#endif
};

#endif

