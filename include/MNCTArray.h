/*
 * MNCTArray.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTArray__
#define __MNCTArray__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <fstream>
#include <iterator>
#include <boost/multi_array.hpp>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////




//!
typedef boost::multi_array<double, 4> double_array4;
typedef boost::multi_array<double, 3> double_array3;
typedef boost::multi_array<double, 2> double_array2;


class MNCTArray
{
 public:
  //! for loading a multi-dimension array from an ASCII file
  static double_array4& Array_loader4(char* filename,int a,int b,int c,int d);
  static double_array3& Array_loader3(char* filename,int a,int b,int c);
  static double_array2& Array_loader2(char* filename,int a,int b);

  static double_array4& csv_loader4(string filename,int a,int b,int c,int d, string mode="one");
  static double_array3& csv_loader3(string filename,int a,int b,int c, string mode="one");
  static double_array2& csv_loader2(string filename,int a,int b, string mode="one");

 protected:

 private:


#ifdef ___CINT___
 public:
  ClassDef(MNCTArray, 0) // no description
#endif
};



#endif


////////////////////////////////////////////////////////////////////////////////
