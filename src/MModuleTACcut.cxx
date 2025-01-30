/*
 * MModuleTACcut.cxx
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
// MModuleTACcut
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleTACcut.h"

// Standard libs:

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MModule.h"
#include "MGUIOptionsTACcut.h"
#include "MGUIExpoTACcut.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleTACcut)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleTACcut::MModuleTACcut() : MModule()
{
  // Construct an instance of MModuleTACcut

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "TAC Cut";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagTACcut";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  //AddPreceedingModuleType(MAssembly::c_DetectorEffectsEngine);

  // AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  // AddPreceedingModuleType(MAssembly::c_ChargeSharingCorrection);
  // AddPreceedingModuleType(MAssembly::c_DepthCorrection);
  // AddPreceedingModuleType(MAssembly::c_StripPairing);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventFilter);


  // Set all modules, which can follow this module

  AddSucceedingModuleType(MAssembly::c_EnergyCalibration);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTACcut)
  // and implement all your GUI options

  // Can the program be run multi-threaded
  m_AllowMultiThreading = true;

  // Can we use multiple instances of this class
  m_AllowMultipleInstances = false;

  //initialize a min and max TAC value 
  m_MinimumTAC = 0;
  m_MaximumTAC = 10000;
}


////////////////////////////////////////////////////////////////////////////////


MModuleTACcut::~MModuleTACcut()
{
  // Delete this instance of MModuleTACcut
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTACcut::Initialize()
{
  // Initialize the module 

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void MModuleTACcut::CreateExpos()
{
  // Create all expos

  if (HasExpos() == true) return;

  // Set the histogram display
  m_ExpoTACcut = new MGUIExpoTACcut(this);
  m_ExpoTACcut->SetTACHistogramArrangement(&m_DetectorIDs);
  for(unsigned int i = 0; i < m_DetectorIDs.size(); ++i){
    unsigned int DetID = m_DetectorIDs[i];
    double thickness = m_Thicknesses[DetID];
    m_ExpoTACcut->SetTACHistogramParameters(DetID, 120, -thickness/2.,thickness/2.);
  }
  m_Expos.push_back(m_ExpoTACcut);
}

////////////////////////////////////////////////////////////////////////////////


bool MModuleTACcut::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 

  // Apply cuts to the TAC values:
   for (unsigned int i = 0; i < Event->GetNStripHits(); ) {
    MStripHit* SH = Event->GetStripHit(i);
    // takes inputted min and max TAC values from the GUI module to make cuts 
    if ( SH->GetTiming() < m_MinimumTAC || SH->GetTiming() > m_MaximumTAC) {
      cout<<"HACK: Removing strip ht due to TAC "<<SH->GetTiming()<<" cut or energy "<<SH->GetEnergy()<<endl;
      Event->RemoveStripHit(i);
      delete SH;
    } else {
      ++i;
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleTACcut::Finalize()
{
  MModule::Finalize();
}


////////////////////////////////////////////////////////////////////////////////


void MModuleTACcut::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsTACcut* Options = new MGUIOptionsTACcut(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTACcut::ReadXmlConfiguration(MXmlNode* Node)
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


MXmlNode* MModuleTACcut::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  /*
  MXmlNode* SomeTagNode = new MXmlNode(Node, "SomeTag", "SomeValue");
  */

  return Node;
}


// MModuleTACcut.cxx: the end...
