/*
 * MNCTModuleChargeSharingCorrection.cxx
 *
 *
 * Copyright (C) 2009-2009 by Mark Bandstra.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Mark Bandstra.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModuleChargeSharingCorrection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleChargeSharingCorrection.h"

// Standard libs:
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>

// ROOT libs:
#include "TGClient.h"
#include "MString.h"
#include "TFile.h"

// MEGAlib libs:
#include "MNCTMath.h"
#include "MStreams.h"
#include "MVector.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleChargeSharingCorrection)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleChargeSharingCorrection::MNCTModuleChargeSharingCorrection() : MModule()
{
  // Construct an instance of MNCTModuleChargeSharingCorrection

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Charge sharing correction";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "ChargeSharingCorrection";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  AddPreceedingModuleType(MAssembly::c_StripPairing);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_ChargeSharingCorrection);

  // Set all modules, which can follow this module

  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleChargeSharingCorrection::~MNCTModuleChargeSharingCorrection()
{
  // Delete this instance of MNCTModuleChargeSharingCorrection
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleChargeSharingCorrection::Initialize()
{
  // Initialize the module 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleChargeSharingCorrection::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 
  
  unsigned int NHits = Event->GetNHits();

  for (unsigned int i_hit=0; i_hit<NHits; i_hit++)
    {
      // Count number of X and Y strips
      MNCTHit *H = Event->GetHit(i_hit);
      unsigned int NStripHits = H->GetNStripHits();
      unsigned int NXStripHits = 0, NYStripHits = 0;
      int DetectorNumber;
      for (unsigned int i_sh=0; i_sh<NStripHits; i_sh++)
	{
	  MNCTStripHit *SH = H->GetStripHit(i_sh);
	  DetectorNumber = SH->GetDetectorID();
	  if (SH->IsXStrip() == true) { NXStripHits++; } else { NYStripHits++; }
	}

      //Check for 1 pixel event and sharing event..
      MNCTStripHit *SHX, *SHY;
      if ( (NStripHits==2) && (NXStripHits==1) && (NYStripHits==1) )
	{
	  SHX = H->GetStripHit(0);
	  SHY = H->GetStripHit(1);
	  if ( !SHX->IsXStrip() )
	    {
	      SHX = H->GetStripHit(1);
	      SHY = H->GetStripHit(0);
	    }
	  // assign negative side energy to this hit
	  H->SetEnergy(SHY->GetEnergy());
	  H->SetEnergyResolution(SHY->GetEnergyResolution());
	}
      else if (( (NStripHits==3) && (((NXStripHits==1)&&(NYStripHits==2)) ||
                                       ((NXStripHits==2)&&(NYStripHits==1))) ) ||
	       ( (NStripHits==4) && (NXStripHits==2) && (NYStripHits==2)))
	{ 
	  int i_sxhit=0;
	  int i_syhit=0;
	  int XStripID[NXStripHits];
	  int YStripID[NYStripHits];
	  double XEnergy=0.0,XEnergyVar=0.0;
	  double YEnergy=0.0,YEnergyVar=0.0;
	  // Get the X and Y strip numbers, and calculate the total energy for each side
	  for (unsigned int i_s_hit=0; i_s_hit < NStripHits; i_s_hit++)
	    {
	      if (H->GetStripHit(i_s_hit)->IsXStrip() == true)
		{
		  XStripID[i_sxhit] = H->GetStripHit(i_s_hit)->GetStripID();
		  XEnergy += H->GetStripHit(i_s_hit)->GetEnergy();
		  XEnergyVar += (H->GetStripHit(i_s_hit)->GetEnergyResolution())
		    *(H->GetStripHit(i_s_hit)->GetEnergyResolution());
		  i_sxhit++;
		}
	      else
		{
		  YStripID[i_syhit] = H->GetStripHit(i_s_hit)->GetStripID();
		  YEnergy += H->GetStripHit(i_s_hit)->GetEnergy();
		  YEnergyVar += (H->GetStripHit(i_s_hit)->GetEnergyResolution())
		    *(H->GetStripHit(i_s_hit)->GetEnergyResolution());
		  i_syhit++;
		}
	    }
	  if (NXStripHits==2 && NYStripHits==1)
	    {
	      if (((XStripID[0]-XStripID[1])==1) || ((XStripID[0]-XStripID[1])==-1))
		{
		  // two adjacent positive strips; use the negative energy
		  H->SetEnergy(YEnergy);
		  H->SetEnergyResolution(sqrt(YEnergyVar));
		}
	    }
	  else if (NYStripHits==2 && NXStripHits==1)
	    {
	      if (((YStripID[0]-YStripID[1])==1) || ((YStripID[0]-YStripID[1])==-1))
		{
		  // two adjacent negative strips; use the negative energy
		  H->SetEnergy(YEnergy);
		  H->SetEnergyResolution(sqrt(YEnergyVar));
		}
	    }
	  else if (NYStripHits==2 && NXStripHits==2)
	    {
	      if ( (((YStripID[0]-YStripID[1])==1) || ((YStripID[0]-YStripID[1])==-1)) &&
		   (((XStripID[0]-XStripID[1])==1) || ((XStripID[0]-XStripID[1])==-1)))
		{
		  // two adjacent positive strips and two adjacent negative strips
		  H->SetEnergy(YEnergy);
		  H->SetEnergyResolution(sqrt(YEnergyVar));
		}
	    } 
	  else
	    {
	      // can't do anything to help this hit
	    }
	}
      else
	{
	  // can't do anything to help this hit
	}
    }
  
  Event->SetChargeSharingCorrected(true);
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleChargeSharingCorrection::ShowOptionsGUI()
{
  // Show the options GUI - or do nothing
}


// MNCTModuleChargeSharingCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
