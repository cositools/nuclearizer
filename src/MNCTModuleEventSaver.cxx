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
#include "MTime.h"
#include "MGUIOptionsEventSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleEventSaver)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleEventSaver::MNCTModuleEventSaver() : MModule()
{
  // Construct an instance of MNCTModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Save events (roa, dat, or evta format)";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagEventSaver";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_EventSaver);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_NoRestriction);
  
  // Set if this module has an options GUI
  m_HasOptionsGUI = true;
  
  m_Mode = c_EvtaFile;
  m_FileName = "MyEvtaFileForRevan.evta";
  m_InternalFileName = "";
  m_SaveBadEvents = true;
  m_AddTimeTag = false;
  
  m_SplitFile = true;
  m_SplitFileTime.Set(60*10); // seconds
  m_SubFileStart.Set(0);
  
  m_NAllowedWorkerThreads = 1;
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
  
  m_SubFileStart.Set(0);  

  m_InternalFileName = m_FileName;
  
  MString Suffix = m_InternalFileName;
  if (Suffix.Last('.') != MString::npos) {
    Suffix.RemoveInPlace(0, Suffix.Last('.'));
    cout<<"Suf:"<<Suffix<<endl;
    if (Suffix == ".dat" || Suffix == ".roa" || Suffix == ".evta") {
      m_InternalFileName.RemoveInPlace(m_InternalFileName.Last('.'));
    }
  }
  cout<<"Internal: "<<m_InternalFileName<<endl;
  
  if (m_AddTimeTag == true) {
    MTime Now;
    m_InternalFileName += ".";
    m_InternalFileName += Now.GetShortString();
  }

  // Add the right tag
  if (m_Mode == c_DatFile) {
    m_InternalFileName += ".dat";
  } else if (m_Mode == c_EvtaFile) {
    m_InternalFileName += ".evta";
  } else if (m_Mode == c_RoaFile) {
    m_InternalFileName += ".roa";
  } else {
    cout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
    return false;
  }
  
  m_Out.open(m_InternalFileName);
  if (m_Out.is_open() == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open file: "<<m_InternalFileName<<endl;
    return false;
  }
 
  ostringstream Header;
  if (m_Mode == c_DatFile) {
    Header<<endl;
    Header<<"Version 1"<<endl;
    Header<<"Type DAT"<<endl;
    Header<<endl;
  } else if (m_Mode == c_EvtaFile) {
    Header<<endl;
    Header<<"Version 21"<<endl;
    Header<<"Type EVTA"<<endl;
    Header<<endl;
  } else if (m_Mode == c_RoaFile) {
    Header<<endl;
    Header<<"TYPE ROA"<<endl;
    Header<<"UF doublesidedstrip adcwithtiming"<<endl;
    Header<<endl;
  } else {
    cout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
    return false;
  }

  m_Header = Header.str();
  m_Out<<m_Header<<endl;
  
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventSaver::StartSubFile()
{
  //! Start a new sub-file

  if (m_SubFileOut.is_open() == true) {
    m_SubFileOut<<"EN"<<endl;
    m_SubFileOut.close();
  }
  
  MString SubName = m_InternalFileName;  
  if (m_Mode == c_DatFile) {
    SubName.ReplaceAllInPlace(".dat", "");
    SubName += ".";
    SubName += m_SubFileStart.GetAsSystemSeconds();
    SubName += ".dat";
  } else if (m_Mode == c_EvtaFile) {
    SubName.ReplaceAllInPlace(".evta", "");
    SubName += ".";
    SubName += m_SubFileStart.GetAsSystemSeconds();
    SubName += ".evta";
  } else if (m_Mode == c_RoaFile) {
    SubName.ReplaceAllInPlace(".roa", "");
    SubName += ".";
    SubName += m_SubFileStart.GetAsSystemSeconds();
    SubName += ".roa";
  } else {
    cout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
    return false;
  }
  
  m_SubFileOut.open(SubName);
  if (m_SubFileOut.is_open() == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open file: "<<SubName<<endl;
    return false;
  }
  m_SubFileOut<<m_Header<<endl;
  
  if (SubName.Last('/') != MString::npos) {
    SubName.RemoveInPlace(0, SubName.Last('/')+1); 
  }
  m_Out<<"IN "<<SubName<<endl;
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


void MNCTModuleEventSaver::Finalize()
{
  // Initialize the module 

  MModule::Finalize();
  
  if (m_SubFileOut.is_open() == true) {
    m_SubFileOut<<"EN"<<endl;
    m_SubFileOut.close();
  }

  m_Out<<"EN"<<endl;
  m_Out.close();
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleEventSaver::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Write the event to disk
 
  if (m_SaveBadEvents == false) {
    if (Event->IsBad() == true) return true;
  }

  ofstream* Choosen = 0; // Wish C++ would allow unassigned references...
  if (m_SplitFile == true) {
    MTime Current = Event->GetTime();
    if (Current > m_SubFileStart + m_SplitFileTime) {
      m_SubFileStart = Current;
      if (StartSubFile() == false) {
        m_IsOK = false;
        return false;
      }
    }
    Choosen = &m_SubFileOut;
  } else {
    Choosen = &m_Out; 
  }
      
  if (m_Mode == c_EvtaFile) {
    Event->StreamEvta(*Choosen);
  } else if (m_Mode == c_DatFile) {
    Event->StreamDat(*Choosen);    
  } else if (m_Mode == c_RoaFile) {
    Event->StreamRoa(*Choosen);
  }
  
  Event->SetAnalysisProgress(MAssembly::c_EventSaver);

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
  MXmlNode* AddTimeTagNode = Node->GetNode("AddTimeTag");
  if (AddTimeTagNode != 0) {
    m_AddTimeTag = AddTimeTagNode->GetValueAsBoolean();
  }
  MXmlNode* SplitFileNode = Node->GetNode("SplitFile");
  if (SplitFileNode != 0) {
    m_SplitFile = SplitFileNode->GetValueAsBoolean();
  }
  MXmlNode* SplitFileTimeNode = Node->GetNode("SplitFileTime");
  if (SplitFileTimeNode != 0) {
    m_SplitFileTime.Set(SplitFileTimeNode->GetValueAsInt());
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
  new MXmlNode(Node, "AddTimeTag", m_AddTimeTag);
  new MXmlNode(Node, "SplitFile", m_SplitFile);
  new MXmlNode(Node, "SplitFileTime", m_SplitFileTime.GetAsSystemSeconds());

  return Node;
}


// MNCTModuleEventSaver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
