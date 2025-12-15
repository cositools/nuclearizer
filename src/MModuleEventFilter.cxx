/*
 * MModuleEventFilter.cxx
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
// MModuleEventFilter
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleEventFilter.h"

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


#ifdef ___CLING___
ClassImp(MModuleEventFilter)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleEventFilter::MModuleEventFilter() : MModule()
{
  // Construct an instance of MModuleTemplate

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

  m_MinimumHVStrips = 0;
  m_MaximumHVStrips = 20;

  m_MinimumLVStrips = 0;
  m_MaximumLVStrips = 20;

  m_MinimumHits = 0;
  m_MaximumHits = 100;
    
  m_MinimumReducedChiSquare = -1;
  m_MaximumReducedChiSquare = numeric_limits<double>::max();
}


////////////////////////////////////////////////////////////////////////////////


MModuleEventFilter::~MModuleEventFilter()
{
  // Delete this instance of MModuleTemplate
  
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEventFilter::Initialize()
{
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEventFilter::Finalize()
{
  MModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEventFilter::AnalyzeEvent(MReadOutAssembly* Event) 
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
  
  unsigned int NHVStrips = 0;
  unsigned int NLVStrips = 0;
  for (unsigned int sh = 0; sh < Event->GetNStripHits(); ++sh) {
    if (Event->GetStripHit(sh)->IsLowVoltageStrip() == true) {
      ++NLVStrips;
    } else {
      ++NHVStrips;
    }
  }
  if (NHVStrips < m_MinimumHVStrips || NHVStrips > m_MaximumHVStrips || NLVStrips < m_MinimumLVStrips || NLVStrips > m_MaximumLVStrips) {
    FilteredOut = true;
  }

  if (Event->GetNHits() < m_MinimumHits || Event->GetNHits() > m_MaximumHits) {
    FilteredOut = true;
  }
    
  // Apply Chi^2 filter (as calculated in strip pairing)
    
    if (Event->GetReducedChiSquare() < m_MinimumReducedChiSquare || Event->GetReducedChiSquare() > m_MaximumReducedChiSquare) {
        FilteredOut = true;
    }


  if (FilteredOut == true) {
    Event->SetFilteredOut(true);
  }
  
  Event->SetAnalysisProgress(MAssembly::c_EventFilter);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEventFilter::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsEventFilter* Options = new MGUIOptionsEventFilter(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
  
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEventFilter::ReadXmlConfiguration(MXmlNode* Node)
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

  MXmlNode* MinimumHVStripsNode = Node->GetNode("MinimumHVStrips");
  if (MinimumHVStripsNode != 0) {
    m_MinimumHVStrips = MinimumHVStripsNode->GetValueAsUnsignedInt();
  }
  MXmlNode* MaximumHVStripsNode = Node->GetNode("MaximumHVStrips");
  if (MaximumHVStripsNode != 0) {
    m_MaximumHVStrips = MaximumHVStripsNode->GetValueAsUnsignedInt();
  }

  MXmlNode* MinimumLVStripsNode = Node->GetNode("MinimumLVStrips");
  if (MinimumLVStripsNode != 0) {
    m_MinimumLVStrips = MinimumLVStripsNode->GetValueAsUnsignedInt();
  }
  MXmlNode* MaximumLVStripsNode = Node->GetNode("MaximumLVStrips");
  if (MaximumLVStripsNode != 0) {
    m_MaximumLVStrips = MaximumLVStripsNode->GetValueAsUnsignedInt();
  }

  MXmlNode* MinimumHitsNode = Node->GetNode("MinimumHits");
  if (MinimumHitsNode != 0) {
    m_MinimumHits = MinimumHitsNode->GetValueAsUnsignedInt();
  }
  MXmlNode* MaximumHitsNode = Node->GetNode("MaximumHits");
  if (MaximumHitsNode != 0) {
    m_MaximumHits = MaximumHitsNode->GetValueAsUnsignedInt();
  }
    
  MXmlNode* MinimumReducedChiSquareNode = Node->GetNode("MinimumReducedChiSquare");
  if (MinimumReducedChiSquareNode != 0) {
    m_MinimumReducedChiSquare = MinimumReducedChiSquareNode->GetValueAsDouble();
  }
  MXmlNode* MaximumReducedChiSquareNode = Node->GetNode("MaximumReducedChiSquare");
  if (MaximumReducedChiSquareNode != 0) {
    m_MaximumReducedChiSquare = MaximumReducedChiSquareNode->GetValueAsDouble();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleEventFilter::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);

  new MXmlNode(Node, "MinimumTotalEnergy", m_MinimumTotalEnergy);
  new MXmlNode(Node, "MaximumTotalEnergy", m_MaximumTotalEnergy);

  new MXmlNode(Node, "MinimumHVStrips", m_MinimumHVStrips);
  new MXmlNode(Node, "MaximumHVStrips", m_MaximumHVStrips);

  new MXmlNode(Node, "MinimumLVStrips", m_MinimumLVStrips);
  new MXmlNode(Node, "MaximumLVStrips", m_MaximumLVStrips);

  new MXmlNode(Node, "MinimumHits", m_MinimumHits);
  new MXmlNode(Node, "MaximumHits", m_MaximumHits);
    
  new MXmlNode(Node, "MinimumReducedChiSquare", m_MinimumReducedChiSquare);
  new MXmlNode(Node, "MaximumReducedChiSquare", m_MaximumReducedChiSquare);

  return Node;
}


// MModuleEventFilter.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
