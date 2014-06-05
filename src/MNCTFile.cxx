/*
 * MNCTFile.cxx
 *
 *
 * Copyright (C) 1998-2008 by Andreas Zoglauer.
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
// MNCTFile
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTFile.h"

// Standard libs:
#include <iomanip>

// ROOT libs:

// MEGAlib:
#include "MStreams.h"
#include "MAssert.h"
#include "MNCTData.h"
#include "MNCTEvent.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTFile)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTFile::MNCTFile(MString GeometryFileName) : MFileEvents(), m_GeometryFileName(GeometryFileName)
{
  // Construct an instance of MNCTFile

  m_ReadSimFile = false;
  m_ReadDatFile = false;
  m_Geometry = 0;
}


////////////////////////////////////////////////////////////////////////////////


MNCTFile::~MNCTFile()
{
  // Delete this instance of MNCTFile
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTFile::Open(MString FileName, unsigned int Way, int HighestAnalysisLevel)
{
  // Open the file, check if the type is correct

  // We have to distingush three cases:
  // (a) Read a sim file
  // (b) Read a NCT data file
  // (c) Save an events file
  if (Way == MFile::c_Read) {
    if (FileName.EndsWith(".sim") == true) {
      // Case (a)

      // Load the geometry
      m_Geometry = new MDGeometryQuest();

      if (m_Geometry->ScanSetupFile(m_GeometryFileName) == true) {
        mlog<<"Geometry "<<m_Geometry->GetName()<<" loaded!"<<endl;
        m_Geometry->ActivateNoising(false);
        m_Geometry->SetGlobalFailureRate(0.0);
      } else {
        mlog<<"Loading of geometry "<<FileName<<" failed!!"<<endl;
        return false;
      }        
      m_SimFile.SetGeometry(m_Geometry);

      // Open a MEGAlib sim file
      m_SimFile.Open(FileName);
      m_ReadSimFile = true;

    } else if (FileName.EndsWith(".dat") == true) {
      // Case (b)
      // Open raw NCT data file
      m_DatFile.Open(FileName, Way);
      m_ReadDatFile = true;

    }
  } else if (Way == MFile::c_Create) {
    // Case (c)
    MFile::Open(FileName, Way);
    if (HighestAnalysisLevel == MNCTData::c_DataCalibrated) {
      m_File<<endl;
      m_File<<"Version 21"<<endl;
      m_File<<"Type EVTA"<<endl;
      m_File<<endl;
    } else if (HighestAnalysisLevel == MNCTData::c_DataReconstructed) {
      m_File<<endl;
      m_File<<"Version 1"<<endl;
      m_File<<"Type TRA"<<endl;
      m_File<<endl;
    } else if (HighestAnalysisLevel == MNCTData::c_DataSimed) {
      m_File<<endl;
      m_File<<"Version 1"<<endl;
      m_File<<"Type DAT"<<endl;
      m_File<<endl;
    } else {
      cout<<"Writing of analysis level "<<HighestAnalysisLevel<<" not yet implemented"<<endl; 
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTFile::Close()
{
  // Dump some statistics and close the file

  return MFileEvents::Close();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTFile::IsShowProgress()
{
  // Handle progress bar for two cases: sim or dat file
  if (m_ReadSimFile == true) {
    return m_SimFile.IsShowProgress();
  }
  else if (m_ReadDatFile == true) {
    return m_DatFile.IsShowProgress();
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTFile::ShowProgress(bool Show)
{
  // Handle progress bar for two cases: sim or dat file
  if (m_ReadSimFile == true) {
    m_SimFile.ShowProgress(Show);
  }
  else if (m_ReadDatFile == true) {
    m_DatFile.ShowProgress(Show);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTFile::UpdateProgress()
{
  // Handle progress bar for two cases: sim or dat file
  if (m_ReadSimFile == true) {
    return m_SimFile.UpdateProgress();
  }
  else if (m_ReadDatFile == true) {
    return m_DatFile.UpdateProgress();
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTFile::SetProgressTitle(MString Main, MString Sub)
{
  // Handle progress bar for two cases: sim or dat file
  if (m_ReadSimFile == true) {
    m_SimFile.SetProgressTitle(Main, Sub);
  }
  else if (m_ReadDatFile == true) {
    m_DatFile.SetProgressTitle(Main, Sub);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MNCTFile::SetProgress(MGUIProgressBar* PB, int Level)
{
  // Handle progress bar for two cases: sim or dat file
  if (m_ReadSimFile == true) {
    m_SimFile.SetProgress(PB, Level);
  }
  else if (m_ReadDatFile == true) {
    m_DatFile.SetProgress(PB, Level);
  }
}


////////////////////////////////////////////////////////////////////////////////


MNCTEvent* MNCTFile::GetNextEvent()
{
  // Return the next event... or 0 if it is the last one
  // So remember to test for more events!

  MNCTEvent* Event = 0;

  if (m_ReadSimFile == true) {
    Event = new MNCTEvent(); // deleted by caller
    MSimEvent* SimEvent = m_SimFile.GetNextEvent();
    if (SimEvent == 0) {
      // No more events
      return 0;
    }

    // Copy the data to the event:

    // Basics:
    Event->SetID(SimEvent->GetEventNumber());
    Event->SetTime(SimEvent->GetTime());

    mimp<<"Missing: Set event time, orientation, etc."<<show;

    // Strip hits:
    for (unsigned int h = 0; h < SimEvent->GetNHTs(); ++h) {
      MNCTHit* Hit = new MNCTHit(); // deleted on destruction of event
      Hit->SetEnergy(SimEvent->GetHTAt(h)->GetEnergy());
      Hit->SetPosition(SimEvent->GetHTAt(h)->GetPosition());

      mimp<<"Adding dummy position resolution"<<show;
      Hit->SetEnergyResolution(2.0);
      Hit->SetPositionResolution(MVector(0.2/sqrt(12.0), 0.2/sqrt(12.0), 0.1));

      Event->AddHit(Hit);
    }

    // Guardring hits
    for (unsigned int g = 0; g < SimEvent->GetNGRs(); ++g) {
      MNCTGuardringHit* GuardringHit = new MNCTGuardringHit(); // deleted on destruction of event
      GuardringHit->SetADCUnits(SimEvent->GetGRAt(g)->GetEnergy());
      GuardringHit->SetPosition(SimEvent->GetGRAt(g)->GetPosition());
      GuardringHit->SetDetectorID(99);

      Event->AddGuardringHit(GuardringHit);
    }
    

    Event->SetDataRead();

    delete SimEvent;

  } else if (m_ReadDatFile == true) {
    // Read event from NCT data file
    Event = m_DatFile.GetNextEvent();
    if (Event == 0) {
      // no more events
      return 0;
    } else {
      Event->SetDataRead();
    }
  } else {
    merr << "reading erroe!!!\n";
  }

  return Event;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTFile::Write(MNCTEvent* Event, int HighestAnalysisLevel)
{
  // Write one event to file

  if (m_Way == MFile::c_Create) {
    ostringstream out;
    if (HighestAnalysisLevel == MNCTData::c_DataCalibrated) {
      out << "SE"  << endl;
      out << "ID " << Event->GetID() << endl;
//      out << fixed;
//      out << "TI " << Event->GetTI() << '.' << setw(9) << setfill('0') << Event->GetCL() << endl;
      out << "TI " << setprecision(20) << Event->GetTime() << endl;
      out << setw(0) << setfill(' ');
  //    out.unsetf(ios_base::fixed);
      out << "PQ " << setprecision(7) << Event->GetEventQuality() << endl;

      if(Event->IsAspectAdded())
	{
	  out << "LT " << setprecision(6) << Event->GetLatitude() << endl;
	  out << "LN " << setprecision(7) << Event->GetLongitude() << endl;
	  out << "AL " << setprecision(5) << Event->GetAltitude() << endl;
	  out << "GX " << setprecision(6) << (Event->GetGX())[0] << ' ' << (Event->GetGX())[1] <<endl;
	  out << "GZ " << setprecision(6) << (Event->GetGZ())[0] << ' ' << (Event->GetGZ())[1] <<endl;
	  out << "HX " << setprecision(6) << (Event->GetHX())[0] << ' ' << (Event->GetHX())[1] <<endl;
	  out << "HZ " << setprecision(6) << (Event->GetHZ())[0] << ' ' << (Event->GetHZ())[1] <<endl;
	}

      for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
        MNCTHit* H = Event->GetHit(h);
	// Write the hit to file
//         out << "HT 3;"
// 	  << H->GetPosition().X() << ";"
// 	  << H->GetPosition().Y() << ";"
// 	  << H->GetPosition().Z() << ";"
// 	  << H->GetEnergy()
// 	  <<endl;
	// Write the hit to file, with energy and position resolutions, too
	if (H->GetPosition().X() != -9999.0){
        out << "HT 3;"
	    << setiosflags(ios::fixed) << setprecision(4) << H->GetPosition().X() << ";"
	    << setiosflags(ios::fixed) << setprecision(4) << H->GetPosition().Y() << ";"
	    << setiosflags(ios::fixed) << setprecision(4) << H->GetPosition().Z() << ";"
	    << setiosflags(ios::fixed) << setprecision(3) << H->GetEnergy()       << ";"
	    << setiosflags(ios::fixed) << setprecision(4) << H->GetPositionResolution().X() << ";"
	    << setiosflags(ios::fixed) << setprecision(4) << H->GetPositionResolution().Y() << ";"
	    << setiosflags(ios::fixed) << setprecision(4) << H->GetPositionResolution().Z() << ";"
	    << setiosflags(ios::fixed) << setprecision(3) << H->GetEnergyResolution()       << ";OK"
	    << endl;
	}else {
	}
      }
      m_File<<out.str();
    } else if (HighestAnalysisLevel == MNCTData::c_DataReconstructed) {
      m_File << "SE" << endl;
      Event->GetPhysicalEvent()->Stream(m_File, 1, false);
    } else if (HighestAnalysisLevel == MNCTData::c_DataSimed){
      m_File << "SE" << endl;
      m_File << "ID " << Event->GetID() << endl;
      m_File << fixed;
      m_File << "TI " << setprecision(9) << Event->GetTime() << endl;
      m_File.unsetf(ios_base::fixed);
      m_File << "CL " << Event->GetCL() << endl;
      for(unsigned int i=0; i<Event->GetNStripHits();i++)
      {
        MNCTStripHit* SH = Event->GetStripHit(i);
        m_File << "SH " << SH->GetDetectorID() << ";"
	       << (SH->IsXStrip() ? '+':'-') << ";"
	       << SH->GetStripID() << ";"
	       << SH->GetADCUnits() << ";"
	       << (SH->GetTiming()/10) << ";"
	       << 1 << ";"
	       << 1 << ";" <<endl; //<--need check
      }
    } else {
      cout<<"Writing of analysis level "<<HighestAnalysisLevel<<" not yet implemented"<<endl; 
    }
  } else {
    return false;
  }

  return true;
}


// MNCTFile.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
