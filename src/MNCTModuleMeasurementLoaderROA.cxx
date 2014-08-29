/*
 * MNCTModuleMeasurementLoaderROA.cxx
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
// MNCTModuleMeasurementLoaderROA
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleMeasurementLoaderROA.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"
#include "MGUIOptionsTemplate.h"
#include "MReadOut.h"
#include "MReadOutSequence.h"
#include "MReadOutElementDoubleStrip.h"
#include "MReadOutDataADCValueWithTiming.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleMeasurementLoaderROA)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleMeasurementLoaderROA::MNCTModuleMeasurementLoaderROA() : MNCTModuleMeasurementLoader()
{
  // Construct an instance of MNCTModuleMeasurementLoaderROA
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Measurement loader for ROA files";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagMeasurementLoaderROA";
  
  // This is a special start module which can generate its own events
  m_IsStartModule = true;
  
  m_NAllowedWorkerThreads = 1;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleMeasurementLoaderROA::~MNCTModuleMeasurementLoaderROA()
{
  // Delete this instance of MNCTModuleMeasurementLoaderROA
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderROA::Initialize()
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
  
  m_IsOK = true;
  
  return MNCTModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderROA::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.
    
  if (ReadNextEvent(Event) == false) {
    cout<<"MNCTModuleMeasurementLoaderROA: No more events!"<<endl;
    m_IsOK = false;
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleMeasurementLoaderROA::Finalize()
{
  // Initialize the module 
  
  MNCTModule::Finalize();
  
  cout<<"MNCTModuleMeasurementLoaderROA: "<<endl;
  cout<<"  * all events on file: "<<m_NEventsInFile<<endl;
  cout<<"  * good events on file: "<<m_NGoodEventsInFile<<endl;

  m_ROAFile.Close();  
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderROA::Open(MString FileName, unsigned int Way)
{
  // Open the file
  
  m_ROAFile.Open(FileName);
  
  return m_ROAFile.IsOpen();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderROA::ReadNextEvent(MNCTEvent* Event)
{
  // Return next single event from file... or 0 if there are no more.
  
  Event->Clear();

  MReadOutSequence ROS;
  m_ROAFile.ReadNext(ROS);

  if (ROS.GetNumberOfReadOuts() == 0) {
    cout<<m_Name<<": No more read-outs available in File"<<endl;
    return false;
  }
  
  m_NEventsInFile++;
  m_NGoodEventsInFile++;
  
  Event->SetID(ROS.GetID());
  Event->SetTime(ROS.GetTime());
  Event->SetCL(ROS.GetClock());
  
  
  for (unsigned int r = 0; r < ROS.GetNumberOfReadOuts(); ++r) {
    MReadOut RO = ROS.GetReadOut(r);
    const MReadOutElementDoubleStrip* Strip = 
      dynamic_cast<const MReadOutElementDoubleStrip*>(&(RO.GetReadOutElement()));
    const MReadOutDataADCValueWithTiming* ADC = 
      dynamic_cast<const MReadOutDataADCValueWithTiming*>(&(RO.GetReadOutData()));
    
    
    MNCTStripHit* SH = new MNCTStripHit();
    SH->SetDetectorID(Strip->GetDetectorID());
    SH->IsXStrip(Strip->IsPositiveStrip());
    SH->SetStripID(Strip->GetStripID());
    
    SH->SetTiming(ADC->GetTiming());
    SH->SetADCUnits(ADC->GetADCValue());
    Event->AddStripHit(SH);
  }
  
  Event->SetDataRead();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderROA::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleMeasurementLoaderROA::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileName", m_FileName);
  
  return Node;
}


// MNCTModuleMeasurementLoaderROA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
