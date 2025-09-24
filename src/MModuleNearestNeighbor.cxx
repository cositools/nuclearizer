/*
 * MModuleNearestNeighbor.cxx
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
// MModuleNearestNeighbor
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleNearestNeighbor.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleNearestNeighbor)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleNearestNeighbor::MModuleNearestNeighbor() : MModule()
{
  // Construct an instance of MModuleNearestNeighbor

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "NearestNeighbor";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagNearestNeighbor";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_NearestNeighbor);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsNearestNeighbor)
  // and implement all your GUI options

  // Can the program be run multi-threaded
  m_AllowMultiThreading = true;

  // Can we use multiple instances of this class
  m_AllowMultipleInstances = true;
}


////////////////////////////////////////////////////////////////////////////////


MModuleNearestNeighbor::~MModuleNearestNeighbor()
{
  // Delete this instance of MModuleNearestNeighbor
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleNearestNeighbor::Initialize()
{
  // Initialize the module 

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleNearestNeighbor::AnalyzeEvent(MReadOutAssembly* Event)
{
  // For the time being remove all next neighbor hits

  for (unsigned int i = 0; i < Event->GetNStripHits();) {
    MStripHit* SH = Event->GetStripHit(i);
    if (SH->IsNearestNeighbor() == true) {
      Event->RemoveStripHit(i);
      delete SH;
    } else {
      ++i;
    }
  }

  Event->SetAnalysisProgress(MAssembly::c_NearestNeighbor);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleNearestNeighbor::Finalize()
{
  // Finalize the analysis - do all cleanup, i.e., undo Initialize() 

  MModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


void MModuleNearestNeighbor::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  /*
  MGUIOptionsNearestNeighbor* Options = new MGUIOptionsNearestNeighbor(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
  */
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleNearestNeighbor::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  /*
  MXmlNode* SomeTagNode = Node->GetNode("SomeTag");
  if (SomeTagNode != 0) {
    m_SomeTagValue = SomeTagNode->GetValue();
  }
  */

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleNearestNeighbor::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MModuleNearestNeighbor.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
