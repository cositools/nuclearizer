/*
 * MNCTModuleEventSaver.cxx
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
// MNCTModuleEventSaver
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleEventSaver.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MNCTModule.h"
#include "MGUIOptionsEventSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleEventSaver)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEventSaver::MNCTModuleEventSaver() : MNCTModule()
{
  // Construct an instance of MNCTModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Save events (dat or evta format)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagEventSaver";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(c_EventLoader);
  
  // Set all types this modules handles
  AddModuleType(c_EventSaver);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(c_NoRestriction);
  
  // Set if this module has an options GUI
  m_HasOptionsGUI = true;
  
  m_Mode = c_EvtaFile;
  m_FileName = "MyEvtaFileForRevan.evta";
  m_SaveBadEvents = true;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEventSaver::~MNCTModuleEventSaver()
{
  // Delete this instance of MNCTModuleTemplate
  m_Out.close();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventSaver::Initialize()
{
  // Initialize the module
  
  m_Out.open(m_FileName);
  if (m_Out.is_open() == false) {
    if (m_Verbosity >= c_Error) mout<<m_XmlTag<<": Unable to open file: "<<m_FileName<<endl;
    return false;
  }
  
  if (m_Mode == c_DatFile) {
    m_Out<<endl;
    m_Out<<"Version 1"<<endl;
    m_Out<<"Type DAT"<<endl;
    m_Out<<endl;
  } else if (m_Mode == c_EvtaFile) {
    m_Out<<endl;
    m_Out<<"Version 21"<<endl;
    m_Out<<"Type EVTA"<<endl;
    m_Out<<endl;
  } else {
    mout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleEventSaver::Finalize()
{
  // Initialize the module 

  m_Out<<"EN"<<endl;
  m_Out.close();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventSaver::AnalyzeEvent(MNCTEvent* Event) 
{
  // Write the event to disk
 
  if (m_SaveBadEvents == false) {
    if (Event->IsBad() == true) return true;
  }

  if (m_Mode == c_EvtaFile) {
    Event->StreamEvta(m_Out);
  } else if (m_Mode == c_DatFile) {
    Event->StreamDat(m_Out);    
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleEventSaver::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsEventSaver* Options = new MGUIOptionsEventSaver(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventSaver::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node
  
  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
    m_FileName = FileNameNode->GetValue();
  }
  MXmlNode* ModeNode = Node->GetNode("Mode");
  if (ModeNode != 0) {
    m_Mode = ModeNode->GetValueAsUnsignedInt();
  }
  MXmlNode* SaveBadEventsNode = Node->GetNode("SaveBadEvents");
  if (SaveBadEventsNode != 0) {
    m_SaveBadEvents = SaveBadEventsNode->GetValueAsBoolean();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleEventSaver::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileName", m_FileName);
  new MXmlNode(Node, "Mode", m_Mode);
  new MXmlNode(Node, "SaveBadEvents", m_SaveBadEvents);

  return Node;
}


// MNCTModuleEventSaver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
