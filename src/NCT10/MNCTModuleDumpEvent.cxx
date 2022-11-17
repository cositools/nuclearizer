/*
 * MNCTModuleDumpEvent.cxx
 *
 *
 * Copyright (C) 2008-2010 by Jau-Shian Liang.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Jau-Shian Liang.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModuleDumpEvent
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleDumpEvent.h"

// Standard libs:
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
//#include "MGUIOptionsDumpEvent.h"
//#include "MNCTTimeAndCoordinate.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MNCTModuleDumpEvent)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDumpEvent::MNCTModuleDumpEvent() : MModule()
{
  // Construct an instance of MNCTModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Dump raw events (NCT format)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagDumpEvent";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventSaver);

  // Set all modules, which can follow this module

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleDumpEvent::~MNCTModuleDumpEvent()
{
  // Delete this instance of MNCTModuleTemplate
  m_OutFile.close();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDumpEvent::Initialize()
{
  // Initialize the module 
  cout << endl << "Initializing MNCTModuleDumpEvent..." << endl;

  m_OutFile.open("rawEvents.dump");
  WriteHeader();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDumpEvent::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level
  m_OutFile << "SE" << endl;
  DumpBasic(Event);

  if(Event->GetNHitsSim()>0){
    for(unsigned int i=0; i<Event->GetNHitsSim(); i++)DumpHitsSim(Event->GetHitSim(i));
  }

  if(Event->GetNStripHits()>0){
    for(unsigned int i=0; i<Event->GetNStripHits(); i++)DumpStripHits(Event->GetStripHit(i));
  }

  if(Event->GetNHits()>0){
    for(unsigned int i=0; i<Event->GetNHits(); i++)DumpHits(Event->GetHit(i));
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleDumpEvent::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

//  MGUIOptionsDumpEvent* Options = new MGUIOptionsDumpEvent(this);
//  Options->Create();
//  gClient->WaitForUnmap(Options);
  
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleDumpEvent::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleDumpEvent::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);

  return Node;
}

////////////////////////////////////////////////////////////////////////////////
void MNCTModuleDumpEvent::WriteHeader()
{
  m_OutFile << "# This file contains raw information of events" << endl
	  << "#" << endl
	  << "# Format:" << endl
	  << "# SE //start event" << endl
	  << "# ID <Event ID>" << endl
	  << "# TI <Event Time>" << endl
	  << "# CL <Event clock> //Event clock or time float digits" << endl
	  << "# TG <Triggered flag> <Veto flag>" << endl
	  << "# AN <Analyzed flags>" << endl
	  << "# DT <Detecter trigger flags>" << endl
	  << "# NH <HTSim number> <SH number> <HT number>" << endl
	  << "# HTSim <x> <y> <z> <Energy> //simulation hits" << endl
	  << "# SH <Det> <Positive?> <strip number> <Timing> <ADC> <E> //strip hits" << endl
	  << "# HT <x> <y> <z> <Energy> //reconstructed hits" << endl
	  << "#" << endl
	  << "#######################################################################" << endl;

}

////////////////////////////////////////////////////////////////////////////////

void MNCTModuleDumpEvent::DumpBasic(MReadOutAssembly* Event)
{
  m_OutFile << "ID " << Event->GetID() << endl
	  << "TI " << Event->GetTime() << endl
	  << "CL " << Event->GetCL() << endl
	  << "TG " << Event->GetTrigger() << ' ' << Event->GetVeto() << endl
	  << "AN " << Event->IsEnergyCalibrated()
	  << Event->IsCrosstalkCorrected()
	  << Event->IsChargeSharingCorrected()
	  << Event->IsStripsPaired()
	  << Event->IsDepthCalibrated() << endl;

  //assumed 10 detectors
  m_OutFile << "DT ";
  for(int i=0; i<10; i++){ m_OutFile << Event->InDetector(i);}
  m_OutFile << endl;

  m_OutFile << "NH " << Event->GetNHitsSim()
	  << ' ' << Event->GetNStripHits()
	  << ' ' << Event->GetNHits() << endl;

}

////////////////////////////////////////////////////////////////////////////////
void MNCTModuleDumpEvent::DumpHitsSim(MNCTHit* HitSim)
{
 m_OutFile << "HTSim"
	 << ' ' << HitSim->GetPosition().GetX()
	 << ' ' << HitSim->GetPosition().GetY()
	 << ' ' << HitSim->GetPosition().GetZ()
	 << ' ' << HitSim->GetEnergy() << endl;

}

////////////////////////////////////////////////////////////////////////////////
void MNCTModuleDumpEvent::DumpStripHits(MNCTStripHit* StrpHit)
{
 m_OutFile << "SH"
	 << ' ' << StrpHit->GetDetectorID()
	 << ' ' << StrpHit->IsXStrip()
	 << ' ' << StrpHit->GetStripID()
	 << ' ' << StrpHit->GetTiming()
	 << ' ' << StrpHit->GetADCUnits()
	 << ' ' << StrpHit->GetEnergy()<< endl;
}

////////////////////////////////////////////////////////////////////////////////
void MNCTModuleDumpEvent::DumpHits(MNCTHit* Hit)
{
 m_OutFile << "HT"
	 << ' ' << Hit->GetPosition().GetX()
	 << ' ' << Hit->GetPosition().GetY()
	 << ' ' << Hit->GetPosition().GetZ()
	 << ' ' << Hit->GetEnergy() << endl;

}

////////////////////////////////////////////////////////////////////////////////

// MNCTModuleTemplate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
