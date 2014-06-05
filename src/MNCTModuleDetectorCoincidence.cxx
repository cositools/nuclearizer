/*
 * MNCTModuleDetectorCoincidence.cxx
 *
 *
 * Copyright (C) 2009-2009 by Mark Bandstra
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
// MNCTModuleDetectorCoincidence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDetectorCoincidence.h"

// Standard libs:
#include <string>
#include <iomanip>

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"
//#include "MGUIOptionsTemplate.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleDetectorCoincidence)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDetectorCoincidence::MNCTModuleDetectorCoincidence() : MNCTModule()
{
  // Construct an instance of MNCTModuleDetectorCoincidence

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Coincident detector event combiner (still being tested!!)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "DetectorCoincidence";

  // Set all modules, which have to be done before this module
  //AddPreceedingModuleType(c_DetectorEffectsEngine);
  //AddPreceedingModuleType(c_CoincidentEventCombiner);
  //AddPreceedingModuleType(c_EnergyCalibration);
  //AddPreceedingModuleType(c_ChargeSharingCorrection);
  //AddPreceedingModuleType(c_DepthCorrection);
  //AddPreceedingModuleType(c_StripPairing);
  //AddPreceedingModuleType(c_EventReconstruction);

  // Set all types this modules handles
  //AddModuleType(c_DetectorEffectsEngine);
  AddModuleType(c_CoincidentEventCombiner);
  //AddModuleType(c_EnergyCalibration);
  //AddModuleType(c_ChargeSharingCorrection);
  //AddModuleType(c_DepthCorrection);
  //AddModuleType(c_StripPairing);
  //AddModuleType(c_EventReconstruction);

  // Set all modules, which can follow this module
  //AddSucceedingModuleType(c_DetectorEffectsEngine);
  //AddSucceedingModuleType(c_CoincidentEventCombiner);
  AddSucceedingModuleType(c_EnergyCalibration);
  AddSucceedingModuleType(c_ChargeSharingCorrection);
  AddSucceedingModuleType(c_DepthCorrection);
  AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_EventReconstruction);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDetectorCoincidence::~MNCTModuleDetectorCoincidence()
{
  // Delete this instance of MNCTModuleDetectorCoincidence
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDetectorCoincidence::Initialize()
{
  // Initialize the module 
  m_NGoodEventsInFile=0;
  m_NStartEventsInFile=0;
  for (int j=1; j<=10; j++)
    {
      m_NEventsByNDetectors[j-1]=0;
      m_NSingleDetectorEvents[j-1]=0;
      m_NMultipleDetectorEvents[j-1]=0;
    }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDetectorCoincidence::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  if (Event != 0) {
    UpdateEventStatistics(Event);
  } else {
    mout << EventStatisticsString() << endl;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


// Updates event statistics with given event
void MNCTModuleDetectorCoincidence::UpdateEventStatistics(MNCTEvent* Event)
{
  // determine if single-detector event or coincidence
  int ndets=0;
  int det;
  for (det=0; det<10; det++)
    {
      if (Event->InDetector(det)==true) { ndets++; }
    }
  for (det=0; det<10; det++)
    {
      if ( (ndets==1) && (Event->InDetector(det)==true) )
	{
	  // single-detector event
	  m_NSingleDetectorEvents[det]++;
	}
      else if ( (ndets>1) && (Event->InDetector(det)==true) )
	{
	  // multiple-detector event
	  m_NMultipleDetectorEvents[det]++;
	}
    }
  if ( (ndets>=1) && (ndets<=10) )
    {
      m_NEventsByNDetectors[ndets-1]++;
    }
}


////////////////////////////////////////////////////////////////////////////////


// Prints a summary of all event statistics
string MNCTModuleDetectorCoincidence::EventStatisticsString()
{
  ostringstream out;

  out << "  ----------------------------------------------------------" << endl;
  out << "  EVENT COINCIDENCE STATISTICS " << endl;
  out << "  ----------------------------------------------------------" << endl;
  out << "  Single-detector and multiple-detector events listed by" << endl;
  out << "    detector number (a multiple-detector event is counted" << endl;
  out << "    in all the detectors that comprise it):" << endl;
  out << "      Detector        Single               Multiple" << endl;
  for (int det=0; det<=9; det++)
    {
      double total = (double)(m_NSingleDetectorEvents[det]+m_NMultipleDetectorEvents[det]);
      out << "        D" << setw(2) << det << "    " 
	  << setw(10) << m_NSingleDetectorEvents[det] 
	  << "  (" << setw(6) << setiosflags(ios::fixed) << setprecision(2)
	  << 100.*(double)m_NSingleDetectorEvents[det]/(double)total << "%) "
	  << setw(10) << m_NMultipleDetectorEvents[det] 
	  << "  (" << setw(6) << setiosflags(ios::fixed) << setprecision(2)
	  << 100.*(double)m_NMultipleDetectorEvents[det]/(double)total << "%)"
	  << endl;
    }
  out << "  ----------------------------------------------------------" << endl;
  out << "  Number of events by # of detectors they span:" << endl;
  unsigned long total=0, total_gt1=0;
  for (int ndet=1; ndet<=10; ndet++)
    {
      total += m_NEventsByNDetectors[ndet-1];
      if (ndet>1) { total_gt1 += m_NEventsByNDetectors[ndet-1]; }
    }
  for (int ndet=1; ndet<=10; ndet++)
    {
      out << "   " << setw(2) << ndet << " detectors......................" 
	  << setw(10) << m_NEventsByNDetectors[ndet-1]
	  << "  (" << setw(6) << setiosflags(ios::fixed) << setprecision(2)
	  << 100.*(double)m_NEventsByNDetectors[ndet-1]/(double)total << "%)" << endl;
    }
  out << "  ----------------------------------------------------------" << endl;
  out << "  Event Totals:" << endl;
  out << "   Single-detector events............" 
      << setw(10) << m_NEventsByNDetectors[0]
      << "  (" << setw(6) << setiosflags(ios::fixed) << setprecision(2) 
      << 100.*(double)m_NEventsByNDetectors[0]/(double)total << "%)" << endl;
  out << "   Multiple-detector events.........." 
      << setw(10) << total_gt1
      << "  (" << setw(6) << setiosflags(ios::fixed) << setprecision(2) 
      << 100.*(double)total_gt1/(double)total << "%)" << endl;
  out << "   Total events processed............" << setw(10) << total << endl;
  out << "  ----------------------------------------------------------" << endl;

  return out.str();//.c_str();
}


////////////////////////////////////////////////////////////////////////////////

// MNCTModuleDetectorCoincidence.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
