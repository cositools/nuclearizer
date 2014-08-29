/*
 * MNCTModule.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModule
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModule.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModule)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Global function to start the thread
void* MNCTModuleKickstartThread(void* ClassDerivedFromMNCTModule)
{
  // dynamic_cast<MNCTModule*>(ClassDerivedFromMNCTModule)->AnalysisLoop();
  ((MNCTModule*) ClassDerivedFromMNCTModule)->AnalysisLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModule::MNCTModule()
{
  // Construct an instance of MNCTModule

  m_Name = "Base class...";
  m_XmlTag = "BaseClass"; // No spaces allowed

  m_HasOptionsGUI = false;
  
  m_IsStartModule = false;
  
  m_IsOK = true;
  m_IsReady = true;
  
  m_Interrupt = false;
  
  m_UseMultiThreading = false;
  m_NAllowedWorkerThreads = 0;
  m_Thread = 0;
  m_IsThreadRunning = false;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModule::~MNCTModule()
{
  // Delete this instance of MNCTModule
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModule::AddEvent(MNCTEvent* Event)
{
  //! Add an event to the incoming event list

  m_IncomingEventsMutex.Lock();
  m_IncomingEvents.push_back(Event);
  m_IncomingEventsMutex.UnLock();
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MNCTModule::HasAddedEvents()
{
  //! Check if there are events in the incoming event list

  bool HasEvents = false;

  m_IncomingEventsMutex.Lock();
  HasEvents = m_IncomingEvents.begin() != m_IncomingEvents.end(); // faster for deque than size if filled!
  m_IncomingEventsMutex.UnLock();

  return HasEvents;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MNCTModule::HasAnalyzedEvents()
{
  //! Check if there are events in the outgoing event list

  bool HasEvents = false;

  m_OutgoingEventsMutex.Lock();
  HasEvents = m_OutgoingEvents.begin() != m_OutgoingEvents.end(); // faster for deque than size if filled!
  m_OutgoingEventsMutex.UnLock();

  return HasEvents;
}
  

////////////////////////////////////////////////////////////////////////////////


MNCTEvent* MNCTModule::GetAnalyzedEvent()
{
  //! Check if there are events in the outgoing event list

  MNCTEvent* E = 0;
  
  m_OutgoingEventsMutex.Lock();
  if (m_OutgoingEvents.begin() != m_OutgoingEvents.end()) {
    E = m_OutgoingEvents.front();
    m_OutgoingEvents.pop_front();
  }
  m_OutgoingEventsMutex.UnLock();
  
  return E;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModule::Initialize()
{
  m_IsOK = true;
  
  for (auto E: m_Expos) {
    E->Reset(); 
  }
  
  if (m_UseMultiThreading == true && m_NAllowedWorkerThreads > 0) {
    m_IsThreadRunning = false;
  
    delete m_Thread;
    m_Thread = new TThread(m_XmlTag + "-Thread ", 
                           (void(*) (void *)) &MNCTModuleKickstartThread, 
                           (void*) this);
    m_Thread->SetPriority(TThread::kHighPriority);
    m_Thread->Run();
    
    while (m_IsThreadRunning == false) {
      gSystem->Sleep(10);
    }
  }
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


void MNCTModule::AnalysisLoop()
{
  m_IsThreadRunning = true;
  
  while (m_Interrupt == false) {
    
    if (DoSingleAnalysis() == false) {
      gSystem->Sleep(20);
    }
  }

  m_IsThreadRunning = false;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MNCTModule::DoSingleAnalysis()
{
  // First check if we are ready:
  if (IsReady() == false) return false;
  
  MNCTEvent* E = 0;
  // If this is a module which does not generate the events, grab one from the incoming list
  if (m_IsStartModule == false) { 
    m_IncomingEventsMutex.Lock();
    if (m_IncomingEvents.begin() != m_IncomingEvents.end()) {
      E = m_IncomingEvents.front();
      m_IncomingEvents.pop_front();
    }
    m_IncomingEventsMutex.UnLock();
  }
  // If we got one from the incoming list, or if this is a start module which generates them:
  if (E == 0 && m_IsStartModule == true) {
    E = new MNCTEvent();
  }
  
  if (E != 0) {
    AnalyzeEvent(E);
    m_OutgoingEventsMutex.Lock();
    m_OutgoingEvents.push_back(E);
    m_OutgoingEventsMutex.UnLock();
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModule::Finalize()
{
  if (m_Thread != 0) {
    m_Interrupt = true;
    while (m_IsThreadRunning == true) {
      gSystem->Sleep(20);
    }
    if (m_Thread != 0) m_Thread->Kill();
    m_Thread = 0;
  }
  
  if (HasAddedEvents() > 0) {
    for (auto E: m_IncomingEvents) {
      delete E; 
    }
    m_IncomingEvents.clear();
  }
  if (HasAnalyzedEvents() > 0) {
    for (auto E: m_OutgoingEvents) {
      delete E; 
    }
    m_OutgoingEvents.clear();
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModule::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModule::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);

  return Node;
}


// MNCTModule.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
