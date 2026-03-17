/*
 * MAspectReconstruction.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MAspectReconstruction__
#define __MAspectReconstruction__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <deque>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAspect.h"
#include "MAspectPacket.h"
#include "MTimeAndCoordinate.h"
#include "MTIRecord.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MAspectReconstruction
{
	// public interface:
	public:
		//! Standard constructor
		MAspectReconstruction();
		//! Default destructor
		virtual ~MAspectReconstruction();
		//! Reset all data
		void Clear();
		//!Initialize the module
		virtual bool Initialize();
		//! Add and reconstruction one or more aspect frames - return false on error
		bool AddAspectFrame(MAspectPacket PacketA); 
		//! Get the aspect for the given time, return 0 if we do not have enough data for the given time
		MAspect* GetAspect_ares(MTime Time);
		MAspect* GetAspect(MTime Time, int GPS_Or_Magnetometer = 0);
		//! Get the aspect for the given time, return 0 if we do not have enough data for the given time
		MAspect* GetAspectGPS(MTime Time);
		//! Get the aspect for the given time, return 0 if we do not have enough data for the given time
		MAspect* GetAspectMagnetometer(MTime Time);
		//! Get the aspect for the given time, return 0 if we do not have enough data for the given time
		MAspect* GetPreviousGPS(MTime Time);
		//! Get the aspect for the given time, return 0 if we do not have enough data for the given time
		MAspect* GetPreviousMagnetometer(MTime Time);
		//! Set the is done flag, used in file mode to signify that there is not more data coming
		void SetIsDone(bool IsDone) {m_IsDone = IsDone;}
		//! Get the is done flag
		bool GetIsDone() {return m_IsDone;}
		//! Get a pointer to the TIRecord
		MTIRecord* GetTIRecord() { return &TIRecord; }

		//!The following are trig functions that work with degrees.  
		double sine(double sine_input);
		double arcsine(double arcsine_input);
		double cosine(double cosine_input);
		double arccosine(double arccosine_input);
		double tangent(double tangent_input);
		double arctangent(double arctangent_input);
		double arctangent2(double y, double x);
		//!The Spherical Vincenty Formula (used to compute exact great circle distance between two points on a sphere). 
		double Vincenty(double old_glat, double new_glat, double old_glon, double new_glon); 
		void SetLastAspectInDeque(deque<MAspect*> CurrentDeque);
		MAspect* GetLastAspectInDeque() const {return LastAspectInDeque;}

		MAspect * InterpolateAspect(MTime ReqTime, MAspect * AspectBefore, MAspect * AspectAfter);

	private:
		//! Internal lists of reconstructed aspects
		deque<MAspect*> m_Aspects_GPS;
		deque<MAspect*> m_Aspects_Magnetometer;
		MTIRecord TIRecord;

		//! Get the is done flag
		//bool GetIsDone() {return m_IsDone;}

		MAspect* LastAspectInDeque;  
		MTimeAndCoordinate m_TCCalculator;
		bool m_IsDone;




#ifdef ___CLING___
	public:
		ClassDef(MAspectReconstruction, 0) // no description
#endif

};

#endif
