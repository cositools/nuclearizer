/*
 * MDepthCalibratorB.h
 *
 * Copyright (C) by Alex Lowell.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDepthCalibratorB__
#define __MDepthCalibratorB__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <unordered_map>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "TGraph.h"
#include "TRandom.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDepthCalibratorB
{
	public:
		//! Default constructor
		MDepthCalibratorB();
    //! Default destructor
    virtual ~MDepthCalibratorB() {};
		//! Load calibration data
		bool LoadLookupTables(MString Fname);
		//! Check if lookup table exists for a pixel
		bool PixelTableExists(int Pixel);
		//! Get Z from Pixel ID and measured CTD value.
		bool GetZ(int Pixel, double t, double& z, double R = 5.0);

	private:
		bool m_LookupTablesLoaded;
		unordered_map<int,TGraph*> m_PixelTables;
		TRandom m_Rand;



#ifdef ___CLING___
 public:
  ClassDef(MDepthCalibratorB, 0) // no description
#endif

};

#endif
