/*
 * MNCTModuleEventFilter.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer
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
// MNCTModuleEventFilter
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleEventFilter.h"

// Standard libs:
#include <cstdio>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsEventFilter.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleEventFilter)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEventFilter::MNCTModuleEventFilter() : MModule()
{
  // Construct an instance of MNCTModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Event Filter";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagEventFilter";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventFilter);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
  
  m_MinimumTotalEnergy = 0;
  m_MaximumTotalEnergy = 10000;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEventFilter::~MNCTModuleEventFilter()
{
  // Delete this instance of MNCTModuleTemplate
  
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventFilter::Initialize()
{
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleEventFilter::Finalize()
{
  MModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventFilter::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level

  bool FilteredOut = false;
  
  // Apply the energy filter:
  if (Event->GetPhysicalEvent() != 0) {
    MPhysicalEvent* E = Event->GetPhysicalEvent();
    if (E->Ei() < m_MinimumTotalEnergy || E->Ei() > m_MaximumTotalEnergy) {
      FilteredOut = true; 
    }
  } else if (Event->GetNHits() > 0) {
    double Total = 0;
    for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
      Total += Event->GetHit(h)->GetEnergy(); 
    }
    if (Total < m_MinimumTotalEnergy || Total > m_MaximumTotalEnergy) {
      FilteredOut = true; 
    }   
  } else if (Event->GetNStripHits() > 0) {
    double Total = 0;
    for (unsigned int h = 0; h < Event->GetNStripHits(); ++h) {
      Total += Event->GetStripHit(h)->GetEnergy(); 
    }
    if (Total < m_MinimumTotalEnergy || Total > m_MaximumTotalEnergy) {
      FilteredOut = true; 
    }   
  }
  
  if (FilteredOut == true) {
    Event->SetFilteredOut(true);
  }
  
  Event->SetAnalysisProgress(MAssembly::c_EventFilter);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleEventFilter::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsEventFilter* Options = new MGUIOptionsEventFilter(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
  
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventFilter::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* MinimumTotalEnergyNode = Node->GetNode("MinimumTotalEnergy");
  if (MinimumTotalEnergyNode != 0) {
    m_MinimumTotalEnergy = MinimumTotalEnergyNode->GetValueAsDouble();
  }
  MXmlNode* MaximumTotalEnergyNode = Node->GetNode("MaximumTotalEnergy");
  if (MaximumTotalEnergyNode != 0) {
    m_MaximumTotalEnergy = MaximumTotalEnergyNode->GetValueAsDouble();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleEventFilter::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);

  new MXmlNode(Node, "MinimumTotalEnergy", m_MinimumTotalEnergy);
  new MXmlNode(Node, "MaximumTotalEnergy", m_MaximumTotalEnergy);

  return Node;
}


// MNCTModuleEventFilter.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
