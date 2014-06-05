/*
 * MNCTFileEventsDat.cxx
 *
 *
 * Copyright (C) 2008-2008 by Mark Bandstra.
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
// MNCTFileEventsDat
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTFileEventsDat.h"

// Standard libs:
#include <string>
#include <iomanip>
using namespace std;

// ROOT libs:
#include "Rtypes.h"
#include "TObjArray.h"
#include "TObjString.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MTokenizer.h"
#include "MNCTMath.h" // needed for time comparison

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTFileEventsDat)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTFileEventsDat::MNCTFileEventsDat(MString GeometryFileName) : MFileEvents(), m_GeometryFileName(GeometryFileName)
{
  // Construct an instance of MNCTFileEventsDat

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MNCTFileEventsDat::~MNCTFileEventsDat()
{
  // Delete this instance of MNCTFileEventsDat
}


////////////////////////////////////////////////////////////////////////////////


void MNCTFileEventsDat::Init()
{
  // Construct an instance of MNCTFileEventsDat

  m_EventId = c_NoId;
  m_IsFirstEvent = true;

  m_NGoodEventsInFile=0;
  m_NStartEventsInFile=0;
  for (int j=1; j<=10; j++)
    {
      m_NEventsByNDetectors[j-1]=0;
      m_NSingleDetectorEvents[j-1]=0;
      m_NMultipleDetectorEvents[j-1]=0;
    }

  //mout << "Test of event statistics:"<<endl;
  //mout << EventStatisticsString() << endl;

  m_FileType = "dat";
  //m_GeometryName = "";

  m_EventBufferMaxSize=1000;
  m_InitialReadNEvents=5000;
  m_CoincidenceTolerance=0.9;
  m_FindCoincidences = false;

  // Set flags for first reading
  m_InitialRead = true;
  for (int det=0; det<10; det++)
    {
      m_DetectorActive[det]=false;
    }

  m_EventBuffer_single.SetCoincidenceTolerance(m_CoincidenceTolerance);
  for (int det=0; det<10; det++)
    {
      m_EventBuffer[det].SetCoincidenceTolerance(m_CoincidenceTolerance);
    }

  // load segment correction file so that we can correct cardcage coincidences
  if (LoadSegmentCorrectionFile() == false) {
    mout<<"MNCTFileEventsDat: Unable to initialize event file object correctly"<<endl;
  }  
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTFileEventsDat::Open(MString FileName, unsigned int Way)
{
  // Open the file

  if (MFileEvents::Open(FileName, c_Read) == false) {
    return false;
  }

  m_EventId = -1;
  m_IsFirstEvent = true;

  bool FoundVersion = false;
  bool FoundType = false;
  bool FoundGeometry = false;

  int Lines = 0;
  int MaxLines = 100;

  MString Line;
  while(!m_File.eof()) {

    if (++Lines >= MaxLines) break;
    Line.ReadLine(m_File);
      
    if (FoundType == false) {
      if (Line.BeginsWith("Type") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read file type (should be "<<m_FileType<<")"<<endl;              
        } else {
          m_FileType = Tokens.GetTokenAtAsString(1);
          FoundType = true;
          mout<<"Found dat file Type: "<<m_FileType<<endl;
        }
      }
    }
    if (FoundVersion == false) {
      if (Line.BeginsWith("Version") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read file version."<<endl;              
        } else {
          m_Version = Tokens.GetTokenAtAsInt(1);
          FoundVersion = true;
          mout<<"Found dat file version: "<<m_Version<<endl;
        }
      }
    }
    if (FoundGeometry == false) {
      if (Line.BeginsWith("Geometry") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() < 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read geometry name."<<endl;              
        } else {
          // Disabled for now, until a real NCT geometry accompanies the dat files. ~MSB
          //m_GeometryFileName = Tokens.GetTokenAfterAsString(1);
          //FoundGeometry = true;
          //mout<<"Found dat file geometry: "<<m_GeometryFileName<<endl;
        }
      }
    }
  }
  MFile::Rewind();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MNCTEvent* MNCTFileEventsDat::ReadNextEvent()
{
  // Return next single event from file... or 0 if there are no more.

  MNCTEvent* Event = new MNCTEvent();

  MString Line;
  // Read file line-by-line, returning 'Event' when it's read a complete, non-empty event.
  while(!m_File.eof()) {
    Line.ReadLine(m_File);
    //mout << Line << endl;

    if ( Line.BeginsWith("Type") || Line.BeginsWith("Version") || Line.BeginsWith("Geometry") ) {
      // do nothing
    } else if (Line.BeginsWith("SE") || Line.BeginsWith("ID") 
               || Line.BeginsWith("TI") || Line.BeginsWith("SH") 
	       || Line.BeginsWith("CL") || Line.BeginsWith("FC")) {
      
      // Case 1: The event is completed.  Check to see if we're at the following "SE".
      if (Line.BeginsWith("SE")) {
        // If the event is empty, then we ignore it and prepare for the next event:
        //mout << "MNCTFileEventsDat::ReadNextEvent: Done reading event" << endl;
        m_NStartEventsInFile++;
        if (Event->GetNStripHits() == 0) {
          delete Event;
          Event = new MNCTEvent();
          m_IsFirstEvent = false;
        } else {
          // Done reading a non-empty event.  Return it:
          //mout << "MNCTFileEventsDat::ReadNextEvent: Returning good event" << endl;
          m_NGoodEventsInFile++;
          return Event;
          m_IsFirstEvent = false;
        }
      } // SE
      
      // Case 2: Parse other keywords in the event
      unsigned int eventID;
      unsigned int framecounter;
      unsigned long long clock_TI;
      unsigned long clock_CL;
      int detid=-1;
      char posneg='0';
      int stripid=-1;
      int adc=-1;
      int timing=-1;
      int adcactive=0;
      int timingactive=0;
      
      //mout << "MNCTFileEventsDat::GetNextEvent: Line = " << Line << endl;
      
      if (Line.BeginsWith("ID")) {
        if (sscanf(Line.Data(), "ID %ud\n", &eventID) == 1) {
          Event->SetID(eventID);
          //mout << "MNCTFileEventsDat::ReadNextEvent:    " << eventID << endl;
        } else {
          //mout << "MNCTFileEventsDat::ReadNextEvent: *** Unable to parse event ID: " 
          //   << Line << endl;
        }
      } else if (Line.BeginsWith("TI")) {
        if (sscanf(Line.Data(), "TI %llu\n", &clock_TI) == 1) {
          Event->SetTI(clock_TI);
	  Event->SetTime(clock_TI);
	  //mout << "MNCTFileEventsDat::ReadNextEvent:  ID: " << eventID 
	  //   << " TI: " << clock_TI << " " << Event->GetTI() << endl;
        } else {
          mout << "MNCTFileEventsDat::ReadNextEvent: *** Unable to parse event time: " 
	       << Line << endl;
        }
      } else if (Line.BeginsWith("CL")) {
        if (sscanf(Line.Data(), "CL %lu\n", &clock_CL) == 1) {
          //Event->SetCL(clock_CL % (4294967295UL));
          Event->SetCL(clock_CL & 0xffffffff);
	  //mout << "MNCTFileEventsDat::ReadNextEvent:  ID: " << eventID 
	  //   << " CL: " << clock_CL << endl;
        } else {
        }
      } else if (Line.BeginsWith("FC")) {
        if (sscanf(Line.Data(), "FC %ud\n", &framecounter) == 1) {
          Event->SetFC(framecounter);
        } else {
        }
      } else if (Line.BeginsWith("SH")) {
        if (sscanf(Line.Data(), "SH %d;%c;%d;%d;%d;%d;%d;\n", 
                   &detid, &posneg, &stripid, &adc, &timing, &adcactive, &timingactive) == 7) {
          // for now, ignore strip hits without both energy and timing info
          if( adcactive == 1 && timingactive == 1) {
            MNCTStripHit* SH = new MNCTStripHit();
            SH->SetDetectorID(detid);
            SH->IsXStrip(posneg == '+');
            SH->SetStripID(stripid);
            SH->SetADCUnits((double)adc);
            SH->SetTiming((double)timing*10.0); // convert to nanoseconds with factor of 10.0
            Event->AddStripHit(SH);
          } else {
            // mout << "MNCTFileEventsDat::ReadNextEvent:  " 
            //      << "SH: ADC and timing are not both active" << endl;
          }
          //mout << "MNCTFileEventsDat::ReadNextEvent:    " << detid << " " << posneg 
          //    << " " << stripid 
          //    << " " << adc << " " << timing
          //    << " " << adcactive << " " << timingactive << endl;
        } else {
          mout << "MNCTFileEventsDat::ReadNextEvent: *** Unable to parse strip hit: " 
               << Line << endl;
        }
      }
    } else {
      // Keyword is not recognized
      mout << "MNCTFileEventsDat::ReadNextEvent: *** Unknown event keyword: " << Line << endl;
    }
    
  } // End of while(!m_File.eof())

  // Done reading.  No more new events.
  if (Event->GetNStripHits() == 0) {
    delete Event;
    return 0;
  } else {
    // Done reading a non-empty event.  Return it:
    // mout << "MNCTFileEventsDat::GetNextEvent: Returning good event" << endl;
    m_NGoodEventsInFile++;
    return Event;
  }
  m_IsFirstEvent = true;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MNCTEvent* MNCTFileEventsDat::GetNextEvent()
{
  // Return the next event... or 0 if there are no more.
  // So remember to test for more events!

  MNCTEvent* Event;
  bool Canceled = false;
  unsigned long CL_period = 4294967295UL;

  // if progress window has been canceled, exit.
  if (UpdateProgress() == false)
    {
      Canceled = true;
      Event = 0;
    }
  else
    {
      Event=ReadNextEvent();
    }

  if (Event != 0)
    {
      // handle special case of flight data where there was no sync
      if (Event->GetNStripHits()>0)
	{
	  if (FindSegment(Event))
	    {
	      // Apply the CL offset appropriate for the given detector
	      unsigned long CL = Event->GetCL();
	      int Detector = Event->GetStripHit(0)->GetDetectorID();
	      if ( (0<=Detector) and (Detector<=9) )
		{
		  double CL_offset = m_SegmentCorrectionTable[m_Segment][15+Detector];
		  if (0)
		    {
		      mout << setprecision(14);
		      mout << "FileEventsDat: CL offset correction: Original CL: " << CL << endl;
		      mout << "FileEventsDat: CL offset correction: CL offset:   " << CL_offset << " (D" << Detector << ")" << endl;
		    }
		  CL = (unsigned long)((double)CL - CL_offset);
		  // ensure CL is still between 0 and 2^32
		  CL = (CL % CL_period);
		  if (0)
		    {
		      mout << "FileEventsDat: CL offset correction: Final CL:    " << CL << endl;
		    }
		  Event->SetCL(CL);
		}
	    }
	}

      // Count event statistics
      UpdateEventStatistics(Event);
    }
  else
    {
      // Done reading file.  Print out statistics.
      mout << endl;
      //mout << "MNCTFileEventsDat::GetNextEvent.  Done reading events from file." << endl;
      mout << EventStatisticsString();
      mout << endl;
    }

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


MNCTEvent* MNCTFileEventsDat::GetNextEvent_buffers()
{
  // Return the next event... or 0 if there are no more.
  // So remember to test for more events!

  MNCTEvent* Event;
  bool EndOfFile = false;
  bool Canceled = false;

  // if progress window has been canceled, exit.
  if (UpdateProgress() == false)
    {
      Canceled = true;
      Event = 0;
    }

  // If this is the first time we are reading events, fill up the buffers
  //  and determine which detectors are active in the file.
  if (m_InitialRead == true)
    {
      unsigned long NEvents=0;
      while ( (NEvents < m_InitialReadNEvents) && (EndOfFile != true) && !Canceled )
        {
          Event = ReadNextEvent();
          if (Event!=0)
            {
              NEvents++;
              int DetNum = Event->GetStripHit(0)->GetDetectorID();
              if ( (0<=DetNum) && (DetNum<10) )
                {
                  m_EventBuffer[DetNum].push_back(Event);
                }
              else
                {
                  delete Event;
                }
            }
          else
            {
              EndOfFile=true;
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

  // find earliest front and back events in buffers
  double EarliestTime = 5.0e9;
  int EarliestDet = 0;
  double EarliestLastTime = 5.0e9;
  for (int det=0; det<10; det++)
    {
      if (m_EventBuffer[det].size() > 0)
        {
          if (m_EventBuffer[det].front()->GetCL() < EarliestTime)
            {
              EarliestTime = m_EventBuffer[det].front()->GetCL();
              EarliestDet = det;
            }
          if (m_EventBuffer[det].back()->GetCL() < EarliestLastTime)
            {
              EarliestLastTime = m_EventBuffer[det].back()->GetCL();
            }
        }
    }

  while ( (EarliestLastTime < EarliestTime+m_CoincidenceTolerance)
          && (LargestBufferSize < m_EventBufferMaxSize) 
          && (EndOfFile != true) 
          && !Canceled )
    {
      Event = ReadNextEvent();
      if (Event!=0)
        {
          int DetNum = Event->GetStripHit(0)->GetDetectorID();
          if ( (0<=DetNum) && (DetNum<10) )
            {
              m_EventBuffer[DetNum].push_back(Event);
            }
          else
            {
              delete Event;
            }
        }
      else
        {
          EndOfFile=true;
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
      EarliestTime = 5.0e9;
      EarliestDet = 0;
      EarliestLastTime = 5.0e9;
      for (int det=0; det<10; det++)
        {
          if (m_EventBuffer[det].size() > 0)
            {
              if (m_EventBuffer[det].front()->GetCL() < EarliestTime)
                {
                  EarliestTime = m_EventBuffer[det].front()->GetCL();
                  EarliestDet = det;
                }
              if (m_EventBuffer[det].back()->GetCL() < EarliestLastTime)
                {
                  EarliestLastTime = m_EventBuffer[det].back()->GetCL();
                }
            }
        }
    } // while

  // if buffers are not empty, take event with earliest time and return it
  if (LargestBufferSize > 0 && !Canceled)
    {
      EarliestTime = 5.0e9;
      EarliestDet = 0;
      for (int det=0; det<10; det++)
        {
          if (m_EventBuffer[det].size() > 0)
            {
              Event = m_EventBuffer[det].front();
              if (Event->GetCL() < EarliestTime)
                {
                  EarliestTime = Event->GetCL();
                  EarliestDet = det;
                }
            }
        }

      //       // buffer debugging output
      //       for (int det=0; det<10; det++) { mout << setw(13) << m_EventBuffer[det].size(); }
      //       mout << endl;
      //       for (int det=0; det<10; det++) { mout << setw(13) << m_NSingleDetectorEvents[det]; }
      //       mout << endl;
      //       for (int det=0; det<10; det++) {
      // 	  if (m_EventBuffer[det].size() > 0) {
      // 	    mout << setw(13) << setprecision(10) 
      // 		 << m_EventBuffer[det].back()->GetTime()-EarliestTime;
      // 	  }
      // 	  else {
      // 	    mout << setw(13) << " ";
      // 	  }
      //       }
      //       mout << endl;
      //       for (int det=0; det<10; det++) {
      // 	if (m_EventBuffer[det].size() > 0) {
      // 	  mout << setw(13) << setprecision(10) 
      // 	       << m_EventBuffer[det].front()->GetTime()-EarliestTime;
      // 	}
      // 	else {
      // 	  mout << setw(13) << " ";
      // 	}
      //       }
      //       mout << endl;
      //       if (EarliestDet > 0) {
      // 	for (int det=0; det<=EarliestDet-1; det++) { mout << setw(13) << " "; }
      //       }
      //       mout << setw(13) << "**********" << endl;
      //       mout << endl << endl;

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

    }
  else
    {
      // Done reading file.  Print out statistics.
      mout << endl;
      mout << "MNCTFileEventsDat::GetNextEvent.  Done reading events from file." << endl;
      mout << EventStatisticsString();
      mout << endl;
    }

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


MNCTEvent* MNCTFileEventsDat::GetNextEvent_singlebuffer()
{
  // Return the next event... or 0 if there are no more.
  // So remember to test for more events!

  MNCTEvent* Event;
  bool EndOfFile = false;
  bool Canceled = false;

  // if progress window has been canceled, exit.
  if (UpdateProgress() == false)
    {
      Canceled = true;
      Event = 0;
    }

  // fill buffer to its maximum size if possible
  while ( (m_EventBuffer_single.size() < m_EventBufferMaxSize) && (EndOfFile != true) && !Canceled)
    {
      // read another event and put it in the buffer
      Event = ReadNextEvent();
      if (Event!=0) { m_EventBuffer_single.push_back(Event); }
      else { EndOfFile=true; }
    }

  // if buffer is not empty, take first event off and return it
  if (m_EventBuffer_single.size()>0 && !Canceled)
    {
      // Keep track of first event
      Event = m_EventBuffer_single.front();

      // Search for time coincidences with the first event and add them to it.
      int NCoinc=0;
      if (m_FindCoincidences==true)
        {
          NCoinc = m_EventBuffer_single.AddCoincidentEvents(Event, true, -1);
        }

      // Remove first event from buffer
      m_EventBuffer_single.pop_front();

      // Count event statistics
      UpdateEventStatistics(Event);

    }
  else if (Event == 0)
    {
      // Done reading file.  Print out statistics.
      mout << endl;
      //mout << "MNCTFileEventsDat::GetNextEvent_singlebuffer.  Done reading events from file." << endl;
      mout << EventStatisticsString();
      mout << endl;
    }

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


MNCTEvent* MNCTFileEventsDat::GetNextEvent_deque()
{
  // Return the next event... or 0 if there are no more.
  // So remember to test for more events!

  MNCTEvent* Event;
  bool EndOfFile = false;
  bool Canceled = false;

  // if progress window has been canceled, exit.
  if (UpdateProgress() == false)
    {
      Canceled = true;
      Event = 0;
    }

  // fill buffer to its maximum size if possible
  while ( (m_EventBuffer_deque.size() < m_EventBufferMaxSize) && (EndOfFile != true) && !Canceled)
    {
      // read another event and put it in the buffer
      Event = ReadNextEvent();
      if (Event!=0) { m_EventBuffer_deque.push_back(Event); }
      else { EndOfFile=true; }
    }

  // if buffer is not empty, take first event off and return it
  if (m_EventBuffer_deque.size()>0 && !Canceled)
    {
      // Keep track of first event
      Event = m_EventBuffer_deque.front();

      // Search for time coincidences with the first event
      int NCoinc = 0;

      if (m_FindCoincidences == true)
        {
          deque<MNCTEvent*>::iterator Event_i;
          Event_i=m_EventBuffer_deque.begin()+1;
          while( Event_i != m_EventBuffer_deque.end() )
            {
              if ( fabs( Event->GetCL() - (*Event_i)->GetCL()) < m_CoincidenceTolerance )
                {
                  // Time match found
                  // 	      mout << "Time match found!" << endl;
                  // 	      mout << "  1: ID " << Event->GetID() 
                  // 		   << " Time: " << setw(15) << setprecision(10) << Event->GetTime() <<endl;
                  // 	      mout << "  2: ID " << (*Event_i)->GetID() 
                  // 		   << " Time: " << setw(15) << setprecision(10) << (*Event_i)->GetTime() <<endl;
                  // 	      mout << endl;
                  // 	      mout << "  Before combining: " << endl;
                  // 	      mout << "   ID:   " << Event->GetID() << endl;
                  // 	      mout << "   Time: " << setw(15) << Event->GetTime() << endl;
                  // 	      for (unsigned int sh=0; sh<Event->GetNStripHits(); sh++)
                  // 		{
                  // 		  mout << "         "
                  // 		       << Event->GetStripHit(sh)->GetDetectorID()
                  // 		       << "  " << Event->GetStripHit(sh)->IsXStrip()
                  // 		       << "  " << Event->GetStripHit(sh)->GetStripID() << endl;
                  // 		}
                  // 	      mout << endl;
                  // 	      mout << "   ID:   " << (*Event_i)->GetID() << endl;
                  // 	      mout << "   Time: " << setw(15) << setprecision(10) << (*Event_i)->GetTime() << endl;
                  // 	      for (unsigned int sh=0; sh<(*Event_i)->GetNStripHits(); sh++)
                  // 		{
                  // 		  mout << "         "
                  // 		       << (*Event_i)->GetStripHit(sh)->GetDetectorID()
                  // 		       << "  " << (*Event_i)->GetStripHit(sh)->IsXStrip()
                  // 		       << "  " << (*Event_i)->GetStripHit(sh)->GetStripID() << endl;
                  // 		}

                  // Move strip hits over to first event
                  for (unsigned int sh=0; sh<(*Event_i)->GetNStripHits(); sh++)
                    {
                      Event->AddStripHit((*Event_i)->GetStripHit(sh));
                    }

                  // Remove event from memory and from deque.
                  // Important: must clear event before deleting, otherwise the strip hits we
                  //   tranferred over are deleted!
                  (*Event_i)->Clear();
                  delete *Event_i;
                  Event_i = m_EventBuffer_deque.erase(Event_i);
                  NCoinc++;

                  // 	      mout << endl;
                  // 	      mout << "  After combining:" << endl;
                  // 	      mout << "   ID:   " << Event->GetID() << endl;
                  // 	      mout << "   Time: " << setw(15) << setprecision(10) << Event->GetTime() << endl;
                  // 	      for (unsigned int sh=0; sh<Event->GetNStripHits(); sh++)
                  // 		{
                  // 		  mout << "         "
                  // 		       << Event->GetStripHit(sh)->GetDetectorID()
                  // 		       << "  " << Event->GetStripHit(sh)->IsXStrip()
                  // 		       << "  " << Event->GetStripHit(sh)->GetStripID() << endl;
                  // 		}
                  // 	      mout << endl;
                }
              else
                {
                  // move to next element in deque
                  Event_i++;
                }
            }
        }

      // Remove first event from buffer
      m_EventBuffer_deque.pop_front();

      // Count event statistics
      UpdateEventStatistics(Event);

    }
  else if (Event == 0)
    {
      // Done reading file.  Print out statistics.
      mout << endl;
      mout << "MNCTFileEventsDat::GetNextEvent_deque.  Done reading events from file." << endl;
      mout << EventStatisticsString();
      mout << endl;
    }

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


// Updates event statistics with given event
void MNCTFileEventsDat::UpdateEventStatistics(MNCTEvent* Event)
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
string MNCTFileEventsDat::EventStatisticsString()
{
  ostringstream out;

  out << "  ----------------------------------------------------------" << endl;
  out << "  DAT FILE STATISTICS " << endl;
  out << "  ----------------------------------------------------------" << endl;
  //out << "  File Statistics:" << endl;
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


bool MNCTFileEventsDat::LoadSegmentCorrectionFile()
{
  char* Env = std::getenv("NUCLEARIZER_CAL");
  if (Env == 0) {
    mout<<"Error: Unable to find the Calibration environment variable: NUCLEARIZER_CAL"<<endl; 
    return false;
  }
  
  m_SegmentCorrectionTable.clear();
  MString Filename = MString(Env) + "/SegmentCorrection.csv";
  mout << "MNCTFileEventsDat: Loading segment correction file..." << endl;
  mout << "filename: " << Filename << endl;

  // Read the calibration coefficients line-by-line
  fstream File;
  File.open(Filename, ios_base::in);
  if (File.is_open() == false)
    {
      mout << "***Warning: Unable to open file: " << Filename << endl
	   << "   Is your NUCLEARIZER_CAL environment variable set?" << endl;
    }
  else
    {
      vector<double> tmp_SegmentPar;
      TString Line;
      while(!File.eof())
	{
	  Line.ReadLine(File);
	  if (Line.BeginsWith("#") == false)
	    {
	      tmp_SegmentPar.clear();
	      //mout << "Line: " << Line << endl;
	      TObjArray* Data = Line.Tokenize(",");
	      TObjArrayIter Iter(Data);
	      TObjString* tmp_objstr;
	      while ((tmp_objstr = (TObjString*)Iter.Next()))
		{
		  //mout << "  single item from line " << ":  " << tmp_objstr->GetString() << endl;
		  tmp_SegmentPar.push_back((double)(tmp_objstr->GetString().Atof()));
		}
	      if (tmp_SegmentPar.size()==25)
		{
		  m_SegmentCorrectionTable.push_back(tmp_SegmentPar);
		}
	      else
		{
		  mout << "LoadSegmentCorrectionFile Warning: line not properly loaded:  " << Line << endl;
		}
	    }
	}
      mout << "Finished loading segment correction file." << endl;
    } // done reading from file
}	

////////////////////////////////////////////////////////////////////////////////

bool MNCTFileEventsDat::FindSegment(MNCTEvent* E)
{
  m_Segment = -1;
  // Search through entire segment file and find a section matching the
  // event's TI (Unix time) and FC (frame counter).
  for (unsigned int j=0; j<m_SegmentCorrectionTable.size(); j++)
    {
      if ( (m_SegmentCorrectionTable[j][1] <= ((double)E->GetTI()))
	   && (((double)E->GetTI()) < m_SegmentCorrectionTable[j][2])
	   && (m_SegmentCorrectionTable[j][3] <= ((double)E->GetFC()))
	   && (((double)E->GetFC()) < m_SegmentCorrectionTable[j][4]) )
	{
	  m_Segment = j;
	}
    }
  if (0)
    {
      mout << "FindSegment: Event TI = " << E->GetTI() << "  FC = " << E->GetFC() << endl;
      if (m_Segment==-1)
	{
	  mout << "FindSegment: No segment found for event." << endl;
	}
      else
	{
	  mout << "FindSegment: Event found to be in segment " << m_Segment << endl;
	  mout << "FindSegment: TI_Start = " << m_SegmentCorrectionTable[m_Segment][1] 
	       << " TI_End = " << m_SegmentCorrectionTable[m_Segment][2] << endl;
	  mout << "FindSegment: FC_Start = " << m_SegmentCorrectionTable[m_Segment][3] 
	       << " FC_End = " << m_SegmentCorrectionTable[m_Segment][4] << endl;
	}
    }
  if (m_Segment==-1) return false;
  else return true;
}

////////////////////////////////////////////////////////////////////////////////

// MNCTFileEventsDat.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
