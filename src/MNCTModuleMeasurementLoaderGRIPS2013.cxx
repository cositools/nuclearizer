/*
 * MNCTModuleMeasurementLoaderGRIPS2013.cxx
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
// MNCTModuleMeasurementLoaderGRIPS2013
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleMeasurementLoaderGRIPS2013.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsTemplate.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleMeasurementLoaderGRIPS2013)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleMeasurementLoaderGRIPS2013::MNCTModuleMeasurementLoaderGRIPS2013() : MNCTModuleMeasurementLoader()
{
  // Construct an instance of MNCTModuleMeasurementLoaderGRIPS2013
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Measurement loader for GRIPS 2013";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagMeasurementLoaderGRIPS2013";
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleMeasurementLoaderGRIPS2013::~MNCTModuleMeasurementLoaderGRIPS2013()
{
  // Delete this instance of MNCTModuleMeasurementLoaderGRIPS2013
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderGRIPS2013::Initialize()
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
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderGRIPS2013::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level:
  // Here: Just read it.
  
  if (ReadNextEvent(Event) == false) {
    cout<<"MNCTModuleMeasurementLoaderGRIPS2013: No more events!"<<endl;
    return false;
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleMeasurementLoaderGRIPS2013::Finalize()
{
  // Initialize the module 
  
  cout<<"MNCTModuleMeasurementLoaderGRIPS2013: "<<endl;
  cout<<"  * all events on file: "<<m_NEventsInFile<<endl;
  cout<<"  * good events on file: "<<m_NGoodEventsInFile<<endl;
  
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderGRIPS2013::Open(MString FileName, unsigned int Way)
{
  // Open the file
  
  m_FileType = "DAT";
  if (MFile::Open(FileName, c_Read) == false) {
    return false;
  }
    
  bool Error = false;
  bool FoundVersion = false;
  bool FoundType = false;
  bool FoundTB = false;
  bool FoundCB = false;
  bool FoundTE = false;
  bool FoundCE = false;
  
  int Lines = 0;
  int MaxLines = 100;
  
  MString Line;
  while (IsGood()) {
    
    if (++Lines >= MaxLines) break;
    ReadLine(Line);
    
    if (FoundType == false) {
      if (Line.BeginsWith("Type") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read file type (should be "<<m_FileType<<")"<<endl;
          Error = true;
        } else {
          m_FileType = Tokens.GetTokenAtAsString(1);
          m_FileType.ToLower();
          FoundType = true;
          mout<<"Found dat file Type: "<<m_FileType<<endl;
        }
      }
    }
    if (FoundVersion == false) {
      if (Line.BeginsWith("Version") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 3) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read file version."<<endl;              
          Error = true;
        } else {
          m_Detector = Tokens.GetTokenAtAsString(1);
          m_Detector.ToLower();
          m_Version = Tokens.GetTokenAtAsInt(2);
          FoundVersion = true;
          mout<<"Found dat file version: "<<m_Version<<" for detector "<<m_Detector<<endl;
        }
      }
    }
    if (FoundTB == false) {
      if (Line.BeginsWith("TB") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read TB keyword"<<endl;              
          Error = true;
        } else {
          m_StartObservationTime = Tokens.GetTokenAtAsDouble(1);
          FoundTB = true;
        }
      }
    }
    if (FoundCB == false) {
      if (Line.BeginsWith("CB") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read file version."<<endl;              
          Error = true;
        } else {
          m_StartClock = Tokens.GetTokenAtAsDouble(1);
          FoundCB = true;
        }
      }
    }
  }
  MFile::Rewind();
  
  
  // Now go to the end of the file to find the TE, CE keywords
  Clear();
  if (GetFileLength() > (streampos) 10000) {
    Seek(GetFileLength() - streamoff(10000));
  } else {
    // start from the beginning...
    MFile::Rewind();
  }
  ReadLine(Line); // Ignore the first line
  while(IsGood()) {
    ReadLine(Line);
    if (Line.Length() < 2) continue;
    
    if (FoundTE == false) {
      if (Line[0] == 'T' && Line[1] == 'E') {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read TE keyword"<<endl;              
          Error = true;
        } else {
          m_EndObservationTime = Tokens.GetTokenAtAsDouble(1);
          FoundTE = true;
        }
      }
    }
    if (FoundCE == false) {
      if (Line[0] == 'C' && Line[1] == 'E') {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read CE keyword"<<endl;              
          Error = true;
        } else {
          m_EndClock = Tokens.GetTokenAtAsDouble(1);
          FoundCE = true;
        }
      }
    }
  }
  MFile::Rewind();
  
  // Now do the sanity checks:
  if (m_FileType != "dat") {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"The file type must be \"dat\" (case is ignored) - you have "<<m_FileType<<endl;              
    Error = true;
    return false;
  }
  if (m_Detector != "grips2013") {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"The detector must be GRIPS2013 (case is irgnored) - you have: "<<m_Detector<<endl;              
    Error = true;
    return false;
  }
  if (FoundTB == false) {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"Did not find the start time in the file (TB keyword)"<<endl;              
    Error = true;
  }
  if (FoundTE == false) {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"Did not find the end time in the file (TE keyword)"<<endl;              
    Error = true;
  }
  if (FoundCB == false) {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"Did not find the start clock in the file (CB keyword)"<<endl;              
    Error = true;
  }
  if (FoundCE == false) {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"Did not find the end clock in the file (CE keyword)"<<endl;              
    Error = true;
  }
  if (m_StartObservationTime > m_EndObservationTime) {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"The start of the observation time is larger than its end!"<<endl;              
    Error = true;
  }
  
  return !Error;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderGRIPS2013::ReadNextEvent(MReadOutAssembly* Event)
{
  // Return next single event from file... or 0 if there are no more.

  Event->Clear();
  
  bool Error = false;
  MString Line;
  
  // Read file line-by-line, returning 'Event' when it's read a complete, non-empty event.
  while (IsGood()) {
    ReadLine(Line);
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
          Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement);
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
    Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EventLoaderMeasurement);
    return !Error;
  }
  
  //cout<<"Returning 0"<<endl;
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleMeasurementLoaderGRIPS2013::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleMeasurementLoaderGRIPS2013::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration
  
  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileName", m_FileName);
  
  return Node;
}


// MNCTModuleMeasurementLoaderGRIPS2013.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
