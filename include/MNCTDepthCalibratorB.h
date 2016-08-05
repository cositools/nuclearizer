/*
 * MNCTDepthCalibratorB.h
 *
 * Copyright (C) by Alex Lowell.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTDepthCalibratorB__
#define __MNCTDepthCalibratorB__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <unordered_map>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTDepthCalibratorB
{
	public:
		//! Default constructor
		MNCTDepthCalibratorB();
		//! Load calibration data
		bool LoadLookupTables(MString Fname);
		//! Check if lookup table exists for a pixel
		bool PixelTableExists(int Pixel);
		//! Get Z from Pixel ID and measured CTD value
		bool GetZ(int Pixel, int CTD, double& Z);

	private:
		bool m_LookupTablesLoaded;
		unordered_map<int,unordered_map<int,double>*> m_PixelTables;



#ifdef ___CINT___
 public:
  ClassDef(MNCTDepthCalibratorB, 0) // no description
#endif

};

#endif