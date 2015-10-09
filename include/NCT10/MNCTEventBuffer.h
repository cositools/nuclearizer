/*
 * MNCTEventBuffer.h
 *
 * Copyright (C) 2008-2008 by Mark Bandstra.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTEventBuffer__
#define __MNCTEventBuffer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MReadOutAssembly.h"

// Standard libs
#include <deque>

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTEventBuffer
{
  // public interface:
 public:
  MNCTEventBuffer();
  virtual ~MNCTEventBuffer();

  // deque wrapping functions
  void clear() { m_EventBuffer.clear(); }
  unsigned long size() { return (unsigned long) m_EventBuffer.size(); }
  bool empty() { return m_EventBuffer.empty(); }
  void push_back(MReadOutAssembly *Event) { m_EventBuffer.push_back(Event); }
  void pop_front() { m_EventBuffer.pop_front(); }
  MReadOutAssembly* front() { return m_EventBuffer.front(); }
  MReadOutAssembly* back() { return m_EventBuffer.back(); }

  // Get/set coincidence tolerance in clock ticks (1 tick = 100 ns)
  void SetCoincidenceTolerance(double ct) { m_CoincidenceTolerance=ct; }
  double GetCoincidenceTolerance() { return m_CoincidenceTolerance; }

  // Add coincident events found in buffer to Event.
  //  (also includes options to skip the first event, and to set a limit to the
  //   number of coincidences found)
  //  Returns number of coincidences found.
  int AddCoincidentEvents(MReadOutAssembly* Event, bool SkipFirst=false, int NLimit=-1);

  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
  // Event buffer
  deque<MReadOutAssembly*> m_EventBuffer;
  // Tolerance (in clock ticks, one tick is 100 ns) of coincidence matching
  double m_CoincidenceTolerance;
  // internal iterator
  deque<MReadOutAssembly*>::iterator Event_i;
 
#ifdef ___CINT___
 public:
  ClassDef(MNCTEventBuffer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
