/*
 * MNCTModuleMeasurementLoaderNCT2009.cxx
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
// MNCTModuleMeasurementLoaderNCT2009
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleMeasurementLoaderNCT2009.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"
#include "MGUIOptionsTemplate.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleMeasurementLoaderNCT2009)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleMeasurementLoaderNCT2009::MNCTModuleMeasurementLoaderNCT2009() : MNCTModuleMeasurementLoader()
{
  // Construct an instance of MNCTModuleMeasurementLoaderNCT2009
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Measurement loader for NCT 2009 data";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagMeasurementLoaderNCT2009";
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleMeasurementLoaderNCT2009::~MNCTModuleMeasurementLoaderNCT2009()
{
  // Delete this instance of MNCTModuleMeasurementLoaderNCT2009
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderNCT2009::Initialize()
{
  // Initialize the module 
/*  
  // Load file
  MNCTFile* Reader = new MNCTFile(m_Data->GetGeometryFileName());
  Reader->Open(m_Data->GetLoadFileName(), MFile::c_Read);
  
  // set up a progress bar
  Reader->ShowProgress(m_UseGui);
  Reader->SetProgressTitle("Progress", "Progress of processing events...");
  
  // Initialize the preprocessor
  mout << "Read dat file: " << m_Data->GetLoadFileName() << endl;
  m_Preprocessor = new MNCTPreprocessor(Reader);
  m_Preprocessor->Initialize();
  */
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderNCT2009::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.

  /*
  MNCTEvent* PreprocessorEvent = Preprocessor->GetNextEvent();

  if (PreprocessorEvent == 0) {
    cout<<"MNCTModuleMeasurementLoaderNCT2009: No more events!"<<endl;
    return false;
  }
  */
  
  // Copy the key data to event and delete
  //Event->
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleMeasurementLoaderNCT2009::Finalize()
{
  // Initialize the module 
  
  cout<<"MNCTModuleMeasurementLoaderNCT2009: "<<endl;
  cout<<"  * all events on file: "<<m_NEventsInFile<<endl;
  cout<<"  * good events on file: "<<m_NGoodEventsInFile<<endl;
  
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderNCT2009::Open(MString FileName, unsigned int Way)
{
  // Open the file
  
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderNCT2009::ReadNextEvent(MNCTEvent* Event)
{
  // Return next single event from file... or 0 if there are no more.

  Event->Clear();
  
  bool Error = false;
  MString Line;
  
  // Read file line-by-line, returning 'Event' when it's read a complete, non-empty event.
  while (!m_File.eof()) {
    Line.ReadLine(m_File);
    //mout<<Line<<endl;
    
    if (Line.BeginsWith("SE") || Line.BeginsWith("ID") || Line.BeginsWith("TI") || Line.BeginsWith("SH") || Line.BeginsWith("CL")) {
      
      // Case 1: The event is completed.  Check to see if we're at the following "SE".
      if (Line.BeginsWith("SE")) {
        // If the event is empty, then we ignore it and prepare for the next event:
        //mout << "MNCTFileEventsDat::ReadNextEvent: Done reading event" << endl;
        m_NEventsInFile++;
        if (Event->GetNStripHits() == 0) {
          Event->Clear();
        } else {
          // Done reading a non-empty event.  Return it:
          //mout<<"MNCTFileEventsDat::ReadNextEvent: Returning good event: "<<long(Event)<<endl;
          m_NGoodEventsInFile++;
          if (Error == true) {
            mout<<"An error occured during reading the event with ID "<<Event->GetID()<<endl;
            mout<<"(If the error is really bad, then there might event not be an ID)"<<endl;
            mout<<"I pass the event on anyway."<<endl;
          }
          Event->SetDataRead();
          return !Error;
        }
      } // SE
      
      // Case 2: Parse other keywords in the event
      if (Line.BeginsWith("ID")) {
        unsigned long ID = 0;
        if (sscanf(Line.Data(), "ID %lu\n", &ID) == 1) {
          Event->SetID(ID);
        } else {
          Error = true;
        }
      } else if (Line.BeginsWith("TI")) {
        MTime T;
        if (T.Set(MString(Line)) == true) {
          Event->SetTime(T);
        } else {
          Error = true;
        }
      } else if (Line.BeginsWith("CL")) {
        unsigned long Clock = 0;
        if (sscanf(Line.Data(), "CL %lu\n", &Clock) == 1) {
          Event->SetCL(Clock);
        } else {
          Error = true;
        }
      } else if (Line.BeginsWith("SH")) {
        int DetectorID = 0;
        char PosOrNeg = 'a';
        int StripID = 0;
        int Triggered = 0;
        unsigned long Timing = 0;
        long UncorrectedADC = 0;
        long CorrectedADC = 0;
        if (sscanf(Line.Data(), "SH %d %c %d %d %lu %li %li\n", &DetectorID, &PosOrNeg, &StripID, &Triggered, &Timing, &UncorrectedADC, &CorrectedADC) == 7) {
          MNCTStripHit* SH = new MNCTStripHit();
          SH->SetDetectorID(DetectorID);
          SH->IsXStrip(PosOrNeg == 'p');
          SH->SetStripID(StripID);
          SH->SetTiming((double) Timing);
          SH->HasTriggered(Triggered == 1);
          SH->SetUncorrectedADCUnits((double)UncorrectedADC);
          SH->SetADCUnits((double) CorrectedADC);
          Event->AddStripHit(SH);
        } else {
          Error = true;
        }
      }
    } else {
      // Keyword is not recognized
      //mout << "MNCTFileEventsDat::ReadNextEvent: *** Unknown event keyword: " << Line << endl;
    }
    
  } // End of while(!m_File.eof())
  
  // Done reading.  No more new events.
  if (Event->GetNStripHits() == 0) {
    Event->Clear();
  } else {
    // Done reading a non-empty event.  Return it:
    //mout << "MNCTFileEventsDat::GetNextEvent: Returning good event (at end of function)" << endl;
    m_NGoodEventsInFile++;
    if (Error == true) {
      mout<<"An error occured during reading the event with ID "<<Event->GetID()<<endl;
      mout<<"(If the error is really bad, then there might event not be an ID)"<<endl;
      mout<<"I pass the event on anyway."<<endl;
    }
    Event->SetDataRead();
    return !Error;
  }
  
  //cout<<"Returning 0"<<endl;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderNCT2009::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleMeasurementLoaderNCT2009::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  new MXmlNode(Node, "FileName", m_FileName);
  
  return Node;
}


// MNCTModuleMeasurementLoaderNCT2009.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
