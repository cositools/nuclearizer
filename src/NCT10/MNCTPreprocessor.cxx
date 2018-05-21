/*
 * MNCTPreprocessor.cxx
 *
 *
 * Copyright (C) 2008-2009 by Mark Bandstra.
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
// MNCTPreprocessor
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTPreprocessor.h"

// Standard libs:
#include <string>
#include <iomanip>

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsTemplate.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNCTPreprocessor)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTPreprocessor::MNCTPreprocessor(MNCTFile* File)
{
  // Construct an instance of MNCTPreprocessor
  
  m_File = File;
  
  m_EndOfFile = false;
  
  m_NGoodEventsInFile=0;
  m_NStartEventsInFile=0;
  for (int j=1; j<=10; j++)
  {
    m_NEventsByNDetectors[j-1]=0;
    m_NSingleDetectorEvents[j-1]=0;
    m_NMultipleDetectorEvents[j-1]=0;
  }
  
  m_EventBufferMaxSize=1000;
  m_InitialReadNEvents=5000;
  m_CoincidenceTolerance=1.9;
  m_FindCoincidences = true;
  m_tempCL=0;
  
  // Set flags for first reading
  m_InitialRead = true;
  for (int det=0; det<10; det++)
  {
    m_DetectorActive[det]=false;
  }
  
  for (int det=0; det<10; det++)
  {
    m_EventBuffer[det].SetCoincidenceTolerance(m_CoincidenceTolerance);
  }
}


////////////////////////////////////////////////////////////////////////////////


MNCTPreprocessor::~MNCTPreprocessor()
{
  // Delete this instance of MNCTPreprocessor
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTPreprocessor::Initialize()
{
  // Initialize the module 
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* MNCTPreprocessor::GetNextEvent() 
{
  // Return the next event... or 0 if there are no more.
  // So remember to test for more events!
  
  MReadOutAssembly* Event;
  vector<MReadOutAssembly*> Events;
  bool Canceled = false;
  
  // if progress window has been canceled, exit.
  if (m_File->UpdateProgress() == false)
  {
    Canceled = true;
    Event = 0;
  }
  
  // If this is the first time we are reading events, fill up the buffers
  //  and determine which detectors are active in the file.
  if (m_InitialRead == true)
  {
    unsigned long NEvents=0;
    while ( (NEvents < m_InitialReadNEvents) && (m_EndOfFile != true) && !Canceled )
    {
      Event = m_File->GetNextEvent();
      if (Event!=0)
      {
        NEvents++;
        
        // If the event is in multiple detectors, break it up
        Events=EventByDetectors(Event);
        
        // add the events to buffers
        while(Events.size() > 0)
        {
          MReadOutAssembly* E = Events.back();
          Events.pop_back();
          int DetNum = E->GetStripHit(0)->GetDetectorID();
          if ( (0<=DetNum) && (DetNum<10) )
          {
            m_EventBuffer[DetNum].push_back(E);
          }
          else
          {
            delete E;
          }
        }
      }
      else
      {
        m_EndOfFile=true;
      }
    }
    m_InitialRead = false;
    // determine which detectors are active
    for (int det=0; det<10; det++)
    {
      if (m_EventBuffer[det].size() > 0)
      {
        m_DetectorActive[det] = true;
      }
      else
      {
        m_DetectorActive[det] = false;
      }
    }
  }
  
  // fill active detector buffers until that they contain events which are after the earliest event.
  
  // find largest and smallest buffer sizes
  unsigned long SmallestBufferSize=m_EventBufferMaxSize+1;
  unsigned long LargestBufferSize=0;
  for (int det=0; det<10; det++)
  {
    if ( (m_DetectorActive[det] == true) && (m_EventBuffer[det].size()<SmallestBufferSize) )
    {
      SmallestBufferSize=m_EventBuffer[det].size();
    }
    if ( m_EventBuffer[det].size() > LargestBufferSize )
    {
      LargestBufferSize=m_EventBuffer[det].size();
    }
  }
  
  // find earliest events at front and back of buffers
  double EarliestTI = 1.0e20;
  double EarliestCL;
  int EarliestDet = 0;
  double EarliestLastTI = 1.0e20;
  for (int det=0; det<10; det++)
  {
    if (m_EventBuffer[det].size() > 0)
    {
      if ((double)m_EventBuffer[det].front()->GetTI() < EarliestTI)
      {
        EarliestTI = (double)m_EventBuffer[det].front()->GetTI();
        EarliestDet = det;
      }
      if ((double)m_EventBuffer[det].back()->GetTI() < EarliestLastTI)
      {
        EarliestLastTI = (double)m_EventBuffer[det].back()->GetTI();
      }
    }
  }
  
  //load events with time condition (modified by Jau-Shian Liang)
  while ( (EarliestLastTI < EarliestTI+3.0)
    && (LargestBufferSize < m_EventBufferMaxSize) 
    && (m_EndOfFile != true) 
    && !Canceled )
  {
    Event = m_File->GetNextEvent();
    if (Event!=0)
    {
      // If the event is in multiple detectors, break it up
      Events=EventByDetectors(Event);
      
      // add the events to buffers
      while(Events.size() > 0)
      {
        MReadOutAssembly* E = Events.back();
        Events.pop_back();
        int DetNum = E->GetStripHit(0)->GetDetectorID();
        if ( (0<=DetNum) && (DetNum<10) )
        {
          m_EventBuffer[DetNum].push_back(E);
        }
        else
        {
          delete E;
        }
      }
    }
    else
    {
      m_EndOfFile=true;
    }
    // find largest and smallest buffer sizes
    SmallestBufferSize=m_EventBufferMaxSize+1;
    LargestBufferSize=0;
    for (int det=0; det<10; det++)
    {
      if ( (m_DetectorActive[det] == true) && (m_EventBuffer[det].size()<SmallestBufferSize) )
      {
        SmallestBufferSize=m_EventBuffer[det].size();
      }
      if ( m_EventBuffer[det].size() > LargestBufferSize )
      {
        LargestBufferSize=m_EventBuffer[det].size();
      }
    }
    // find earliest front and back events in buffers
    EarliestTI = 1.0e20;
    EarliestDet = 0;
    EarliestLastTI = 1.0e20;
    for (int det=0; det<10; det++)
    {
      if (m_EventBuffer[det].size() > 0)
      {
        if ((double)m_EventBuffer[det].front()->GetTI() < EarliestTI)
        {
          EarliestTI = (double)m_EventBuffer[det].front()->GetTI();
        }
        if ((double)m_EventBuffer[det].back()->GetTI() < EarliestLastTI)
        {
          EarliestLastTI = (double)m_EventBuffer[det].back()->GetTI();
        }
      }
    }
  } // while
  
  
  // if buffers are not empty, take event with earliest time and return it
  // match events with clock number.  (Jau-Shian Liang)
  while (LargestBufferSize > 0 && !Canceled)
  {
    bool FoundEarliest = false;
    EarliestCL = 5.0e30;
    EarliestDet = 0;
    for (int det=0; det<10; det++)
    {
      if (m_EventBuffer[det].size() > 0)
      {
        Event = m_EventBuffer[det].front();
        if ((double)Event->GetCL() < EarliestCL && Event->GetCL()>=m_tempCL)
        {
          EarliestCL = (double)Event->GetCL();
          EarliestDet = det;
          FoundEarliest = true;
        }
      }
    }
    
    // added by Jau-Shian
    if(FoundEarliest == false)
    {
      m_tempCL = 0;
      continue;
    }
    else
    {
      m_tempCL = (unsigned long long)EarliestCL;
    }
    
    // Keep track of earliest event
    Event = m_EventBuffer[EarliestDet].front();
    
    // Search for time coincidences with the first event in other buffers,
    //   and add them to the original event.
    int NCoinc = 0;
    if (m_FindCoincidences == true)
    {
      for (int det=0; det<10; det++)
      {
        if (det != EarliestDet)
        {
          NCoinc += m_EventBuffer[det].AddCoincidentEvents(Event, false, 1);
        }
      }
    }
    
    // Remove earliest event from buffer
    m_EventBuffer[EarliestDet].pop_front();
    
    // Count event statistics
    UpdateEventStatistics(Event);
    break;
  }
  
  if(LargestBufferSize <= 0 || Canceled)
  {
    // Buffers have been emptied, or analysis was canceled.  Print out statistics.
    Event = 0;
    mout << endl;
    mout << "MNCTPreprocessor::GetNextEvent.  Done reading events from file." << endl;
    mout << EventStatisticsString();
    mout << endl;
  }
  
  return Event;
}


////////////////////////////////////////////////////////////////////////////////


// Updates event statistics with given event
void MNCTPreprocessor::UpdateEventStatistics(MReadOutAssembly* Event)
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
string MNCTPreprocessor::EventStatisticsString()
{
  ostringstream out;
  
  out << "  ----------------------------------------------------------" << endl;
  out << "  PREPROCESSOR STATISTICS " << endl;
  out << "  ----------------------------------------------------------" << endl;
  out << "   Start events (SE) in file........." << setw(10) << m_NStartEventsInFile << endl;
  out << "   Valid single events in file......." << setw(10) << m_NGoodEventsInFile 
  << "  (" << setw(6) << setiosflags(ios::fixed) << setprecision(2)
  << 100.*(double)m_NGoodEventsInFile/(double)m_NStartEventsInFile << "%)  " << endl;
  out << "  ----------------------------------------------------------" << endl;
  if (m_FindCoincidences)
  {
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
  }
  return out.str();//.c_str();
}


////////////////////////////////////////////////////////////////////////////////


// break apart event into events by detector
vector<MReadOutAssembly*> MNCTPreprocessor::EventByDetectors(MReadOutAssembly* Event)
{
  vector<MReadOutAssembly*> Events;
  
  for (int det=0; det<10; det++)
  {
    if (Event->InDetector(det))
    {
      MReadOutAssembly* E=new MReadOutAssembly();
      E->SetID(Event->GetID());
      E->SetTI(Event->GetTI());
      E->SetCL(Event->GetCL());
      E->SetFC(Event->GetFC());
      for (unsigned int i=0; i<Event->GetNStripHits(); i++)
      {
        MNCTStripHit* SH = Event->GetStripHit(i);
        if (SH->GetDetectorID() == det)
        {
          E->AddStripHit(SH);
        }
      }
      Events.push_back(E);
    }
  }
  
  Event->Clear();
  delete Event;
  return Events;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTPreprocessor::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!
  
  //MGUIOptionsTemplate* Options = new MGUIOptionsTemplate(this);
  //Options->Create();
  //gClient->WaitForUnmap(Options);
}

// MNCTPreprocessor.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
