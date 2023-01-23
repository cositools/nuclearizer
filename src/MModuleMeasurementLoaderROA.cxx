/*
 * MModuleMeasurementLoaderROA.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
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
// MModuleMeasurementLoaderROA
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleMeasurementLoaderROA.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsTemplate.h"
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MReadOutElementDoubleStrip.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutDataTiming.h"
#include "MReadOutDataOrigins.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleMeasurementLoaderROA)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleMeasurementLoaderROA::MModuleMeasurementLoaderROA() : MModuleMeasurementLoader()
{
  // Construct an instance of MModuleMeasurementLoaderROA
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Measurement loader for ROA files";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagMeasurementLoaderROA";
  
  // This is a special start module which can generate its own events
  m_IsStartModule = true;
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleMeasurementLoaderROA::~MModuleMeasurementLoaderROA()
{
  // Delete this instance of MModuleMeasurementLoaderROA
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleMeasurementLoaderROA::Initialize()
{
  // Initialize the module 
  
  // Clean:
  m_FileType = "Unknown";
  m_Detector = "Unknown";
  m_Version = -1;
  m_StartObservationTime = MTime(0);
  m_EndObservationTime = MTime(0);
  m_StartClock = numeric_limits<long>::max();
  m_EndClock = numeric_limits<long>::max();
  
  if (Open(m_FileName, c_Read) == false) return false;
  
  m_NEventsInFile = 0;
  m_NGoodEventsInFile = 0;
    
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleMeasurementLoaderROA::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.
    
  if (ReadNextEvent(Event) == false) {
    cout<<"MModuleMeasurementLoaderROA: No more events!"<<endl;
    m_IsFinished = true;
    return false;
  }
  
  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleMeasurementLoaderROA::Finalize()
{
  // Initialize the module 
  
  MModule::Finalize();
  
  cout<<"MModuleMeasurementLoaderROA: "<<endl;
  cout<<"  * all events on file: "<<m_NEventsInFile<<endl;
  cout<<"  * good events on file: "<<m_NGoodEventsInFile<<endl;

  m_ROAFile.Close();  
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleMeasurementLoaderROA::Open(MString FileName, unsigned int Way)
{
  // Open the file
  
  m_ROAFile.Open(FileName);
  
  return m_ROAFile.IsOpen();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleMeasurementLoaderROA::ReadNextEvent(MReadOutAssembly* Event)
{
  // Return next single event from file... or 0 if there are no more.
  
  Event->Clear();

  m_ROAFile.ReadNext(*Event);
  
  if (Event->GetNumberOfReadOuts() == 0) {
    cout<<m_Name<<": No more read-outs available in File"<<endl;
    return false;
  }
  
  m_NEventsInFile++;
  m_NGoodEventsInFile++;

  
  for (unsigned int r = 0; r < Event->GetNumberOfReadOuts(); ++r) {
    MReadOut RO = Event->GetReadOut(r);
    const MReadOutElementDoubleStrip* Strip = 
      dynamic_cast<const MReadOutElementDoubleStrip*>(&(RO.GetReadOutElement()));
      
    const MReadOutDataADCValue* ADC = 
      dynamic_cast<const MReadOutDataADCValue*>(RO.GetReadOutData().Get(MReadOutDataADCValue::m_TypeID));
    const MReadOutDataTiming* Timing = 
      dynamic_cast<const MReadOutDataTiming*>(RO.GetReadOutData().Get(MReadOutDataTiming::m_TypeID));
    const MReadOutDataOrigins* Origins = 
      dynamic_cast<const MReadOutDataOrigins*>(RO.GetReadOutData().Get(MReadOutDataOrigins::m_TypeID));
    
    
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(Strip->GetDetectorID());
    SH->IsXStrip(Strip->IsPositiveStrip());
    SH->SetStripID(Strip->GetStripID());
    
    SH->SetTiming(Timing->GetTiming());
    SH->SetADCUnits(ADC->GetADCValue());
    
    if (Origins != nullptr) {
      SH->AddOrigins(Origins->GetOrigins());
    }
    
    
    Event->AddStripHit(SH);
  }
  
  Event->SetTimeUTC(Event->GetTime());
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleMeasurementLoaderROA::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleMeasurementLoaderROA::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileName", m_FileName);
  
  return Node;
}


// MModuleMeasurementLoaderROA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
