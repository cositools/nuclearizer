/*
* MNCTEventBuffer.cxx
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
// MNCTEventBuffer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTEventBuffer.h"

// Standard libs:
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MNCTMath.h" // needed for time comparison

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNCTEventBuffer)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTEventBuffer::MNCTEventBuffer()
{
  // Construct an instance of MNCTEventBuffer
  
}


////////////////////////////////////////////////////////////////////////////////


MNCTEventBuffer::~MNCTEventBuffer()
{
  // Delete this instance of MNCTEventBuffer
}


////////////////////////////////////////////////////////////////////////////////


int MNCTEventBuffer::AddCoincidentEvents(MReadOutAssembly* Event, bool SkipFirst, int NLimit)
{
  int NCoinc=0;
  double Time = Event->GetCL();
  
  if (NLimit==-1)
  {
    NLimit = m_EventBuffer.size();
  }
  
  Event_i = m_EventBuffer.begin();
  if ( SkipFirst == true )
  {
    Event_i++;
  }
  while( (Event_i != m_EventBuffer.end()) && (NCoinc < NLimit) )
  {
    // A coincidence is found when the times match, but the times are both nonzero
    if ( (fabs( Time-(*Event_i)->GetCL()) < m_CoincidenceTolerance)
      && !(Time <= 0.1) && !((*Event_i)->GetCL() <= 0.1) )
    {
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
      Event_i = m_EventBuffer.erase(Event_i);
      NCoinc++;
    }
    else
    {
      // move to next element in deque
      Event_i++;
    }
  }
  return NCoinc;
}


////////////////////////////////////////////////////////////////////////////////


// MNCTEventBuffer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
