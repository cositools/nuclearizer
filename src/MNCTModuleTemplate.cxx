/*
 * MNCTModuleTemplate.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
// MNCTModuleTemplate
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleTemplate.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MNCTModule.h"
#include "MGUIOptionsTemplate.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleTemplate)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleTemplate::MNCTModuleTemplate() : MNCTModule()
{
  // Construct an instance of MNCTModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Template";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagTemplate";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(c_DetectorEffectsEngine);
  AddPreceedingModuleType(c_EnergyCalibration);
  AddPreceedingModuleType(c_ChargeSharingCorrection);
  AddPreceedingModuleType(c_DepthCorrection);
  AddPreceedingModuleType(c_StripPairing);
  AddPreceedingModuleType(c_EventReconstruction);

  // Set all types this modules handles
  AddModuleType(c_DetectorEffectsEngine);
  AddModuleType(c_EnergyCalibration);
  AddModuleType(c_ChargeSharingCorrection);
  AddModuleType(c_DepthCorrection);
  AddModuleType(c_StripPairing);
  AddModuleType(c_EventReconstruction);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(c_DetectorEffectsEngine);
  AddSucceedingModuleType(c_EnergyCalibration);
  AddSucceedingModuleType(c_ChargeSharingCorrection);
  AddSucceedingModuleType(c_DepthCorrection);
  AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_Aspect);
  AddSucceedingModuleType(c_Else);
  AddSucceedingModuleType(c_EventReconstruction);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = false;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleTemplate::~MNCTModuleTemplate()
{
  // Delete this instance of MNCTModuleTemplate
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleTemplate::Initialize()
{
  // Initialize the module 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleTemplate::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleTemplate::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsTemplate* Options = new MGUIOptionsTemplate(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleTemplate::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  /*
  MXmlNode* SomeTagNode = Node->GetNode("SomeTag");
  if (SomeTagNode != 0) {
    m_SomeTagValue = SomeTagNode.GetValue();
  }
  */

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleTemplate::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MNCTModuleTemplate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
