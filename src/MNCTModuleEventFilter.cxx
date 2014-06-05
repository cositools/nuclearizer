/*
 * MNCTModuleEventFilter.cxx
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

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"
#include "MGUIOptionsEventFilter.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleEventFilter)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEventFilter::MNCTModuleEventFilter() : MNCTModule()
{
  // Construct an instance of MNCTModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Event Filter";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagEventFilter";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(c_EventLoader);
  //  AddPreceedingModuleType(c_DetectorEffectsEngine);
//  AddPreceedingModuleType(c_EnergyCalibration);
//  AddPreceedingModuleType(c_ChargeSharingCorrection);
//  AddPreceedingModuleType(c_DepthCorrection);
//  AddPreceedingModuleType(c_StripPairing);
//  AddPreceedingModuleType(c_EventReconstruction);

  // Set all types this modules handles
//  AddModuleType(c_DetectorEffectsEngine);
//  AddModuleType(c_EnergyCalibration);
//  AddModuleType(c_ChargeSharingCorrection);
//  AddModuleType(c_DepthCorrection);
//  AddModuleType(c_StripPairing);
  AddModuleType(c_EventFilter);
//  AddModuleType(c_EventReconstruction);

  // Set all modules, which can follow this module
//  AddSucceedingModuleType(c_DetectorEffectsEngine);
  AddSucceedingModuleType(c_EnergyCalibration);
  AddSucceedingModuleType(c_CrosstalkCorrection);
  AddSucceedingModuleType(c_ChargeSharingCorrection);
  AddSucceedingModuleType(c_DepthCorrection);
  AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_Aspect);
  AddSucceedingModuleType(c_Else);
  AddSucceedingModuleType(c_EventSaver);
  AddSucceedingModuleType(c_EventReconstruction);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEventFilter::~MNCTModuleEventFilter()
{
  // Delete this instance of MNCTModuleTemplate
  
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventFilter::Initialize()
{
  // Initialize the module 
  cout << endl << "Initializing MNCTModuleEventFilter..." << endl;

  m_NEvent=0;
  m_NVeto=0;
  m_VetoList.clear();
//  m_VetoList.push_back(1);

  // parse veto setting string
  int int_tmp=-1;
  istringstream vetolist(m_VetoSetting);
  while(!vetolist.eof()){
    vetolist >> int_tmp;
    m_VetoList.push_back(int_tmp);
  }
  
  cout << "Veto list: ";
  for(unsigned int i=0;i<m_VetoList.size(); i++)
  {
    cout << m_VetoList[i] << ' ';
  }
  cout << endl << endl;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////
MString MNCTModuleEventFilter::Report()
{
  MString string_tmp;

  string_tmp += "  Total input events: ";
  string_tmp += m_NEvent;
  string_tmp += "\n";
  
  string_tmp += "  Vetoed events: "; 
  string_tmp += m_NVeto;
  string_tmp += '\n';
  
  string_tmp += "  Veto rate: ";
  string_tmp += m_NVeto/(double)m_NEvent*100.0 ;
  string_tmp += "%\n";

  return string_tmp;

}
////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventFilter::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level

  if(Event->GetNStripHits()<1)return true;

  m_NEvent++;
  vector<int>::iterator it;
  
  for(unsigned int i=0; i<Event->GetNStripHits(); i++){
    it=find(m_VetoList.begin(),m_VetoList.end(),Event->GetStripHit(i)->GetDetectorID());

    if(it!=m_VetoList.end()){
      Event->SetVeto();
      //cout << "Event:" << Event->GetID() << " D" << Event->GetStripHit(i)->GetDetectorID() <<"-->Veto!!\n";
      m_NVeto++;
      break;
    }

  }
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
  MXmlNode* VetoSettingNode = Node->GetNode("VetoSetting");

  if (VetoSettingNode !=0)SetVetoSetting(VetoSettingNode->GetValueAsString());
  else SetVetoSetting("");

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleEventFilter::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);

  new MXmlNode(Node, "VetoSetting", GetVetoSetting());

  return Node;
}

////////////////////////////////////////////////////////////////////////////////

// MNCTModuleEventFilter.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
