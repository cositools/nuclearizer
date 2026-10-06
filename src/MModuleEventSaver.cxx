/*
 * MModuleEventSaver.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer
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
// MModuleEventSaver
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleEventSaver.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MTime.h"
#include "MGUIOptionsEventSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleEventSaver)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleEventSaver::MModuleEventSaver() : MModule()
{
  // Construct an instance of MModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Save events (roa, dat, evta, or tra format)";

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
  m_Zip = false;
  m_SaveBadEvents = true;
  m_SavePoorQualityEvents = true;
  m_SaveVetoEvents = true;
  m_AddTimeTag = false;
    
  m_RoaWithADCs = true;
  m_RoaWithTACs = true;
  m_RoaWithEnergies = false;
  m_RoaWithTimings = false;
  m_RoaWithTemperatures = false;
  m_RoaWithFlags = false;
  m_RoaWithOrigins = false;
  m_RoaWithNearestNeighbors = true;
  m_SplitByDetectorSide = false;

  m_SplitFile = true;
  m_SplitFileTime.Set(60*10); // seconds
  m_SubFileStart.Set(0);
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
  
  m_StartAreaFarField = 0.0;
  m_NumberOfSimulatedEvents = 0;
}


////////////////////////////////////////////////////////////////////////////////


MModuleEventSaver::~MModuleEventSaver()
{
  // Destructor
  
  CloseDetectorSideFiles();
  m_Out.Close();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEventSaver::Initialize()
{
  // Initialize the module
  
  m_SubFileStart.Set(0);  
  CloseDetectorSideFiles();

  if (m_SplitByDetectorSide == true && m_Mode != c_RoaFile) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Splitting by detector and side is only supported for roa files"<<endl;
    return false;
  }

  m_InternalFileName = m_FileName;
  
  m_Zip = false;
  if (m_InternalFileName.EndsWith(".gz") == true) {
    m_Zip = true;
    m_InternalFileName.RemoveInPlace(m_InternalFileName.Length() - 3);
  }
  
  MString Suffix = m_InternalFileName;
  if (Suffix.Last('.') != MString::npos) {
    Suffix.RemoveInPlace(0, Suffix.Last('.'));
    if (Suffix == ".dat" || Suffix == ".roa" || Suffix == ".evta" || Suffix == ".tra") {
      m_InternalFileName.RemoveInPlace(m_InternalFileName.Last('.'));
    }
  }
  
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
  } else if (m_Mode == c_TraFile) {
    m_InternalFileName += ".tra";
  } else if (m_Mode == c_RoaFile) {
    m_InternalFileName += ".roa";
  } else {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
    return false;
  }
  
  if (m_Zip == true) {
    m_InternalFileName += ".gz";
  }
  
  
  // When splitting by detector and side, the files are opened on demand and there is no main file
  if (m_SplitByDetectorSide == false) {
    m_Out.Open(m_InternalFileName, MFile::c_Write);
    if (m_Out.IsOpen() == false) {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open file: "<<m_InternalFileName<<endl;
      return false;
    }
  } else if (m_SplitFile == true) {
    if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": Splitting by detector and side is enabled - ignoring the time-based file splitting"<<endl;
  }
 
  ostringstream Header;
  if (m_Mode == c_DatFile) {
    Header<<endl;
    Header<<"Version 3"<<endl;
    Header<<"Type DAT"<<endl;
    Header<<endl;
  } else if (m_Mode == c_EvtaFile) {
    Header<<endl;
    Header<<"Version 200"<<endl;
    Header<<"Type EVTA"<<endl;
    Header<<endl;
  } else if (m_Mode == c_TraFile) {
    Header<<endl;
    Header<<"Version 1"<<endl;
    Header<<"TYPE TRA"<<endl;
    Header<<endl;
  } else if (m_Mode == c_RoaFile) {
    Header<<endl;
    Header<<"TYPE ROA"<<endl;

    // Prepare the read-out data flags for strip and cystal hits
    vector<MString> StripData;
    vector<MString> CrystalData;
    if (m_RoaWithADCs == true) {
      StripData.push_back("adc");
      CrystalData.push_back("adc");
    }
    if (m_RoaWithTACs == true) {
      StripData.push_back("tac");
    }
    if (m_RoaWithEnergies == true) {
      StripData.push_back("energy");
      CrystalData.push_back("energy");
    }
    if (m_RoaWithTimings == true) {
      StripData.push_back("timing");
    }
    if (m_RoaWithFlags == true) {
      StripData.push_back("flags");
      CrystalData.push_back("flags");
    }
    if (m_RoaWithOrigins == true) {
      StripData.push_back("origins");
      CrystalData.push_back("origins");
    }

    MString StripType;
    for (unsigned int d = 0; d < StripData.size(); ++d) {
      if (d != 0) StripType += "-";
      StripType += StripData[d];
    }
    MString CrystalType;
    for (unsigned int d = 0; d < CrystalData.size(); ++d) {
      if (d != 0) CrystalType += "-";
      CrystalType += CrystalData[d];
    }

    m_RoaFileFormat.Clear();
    if (m_RoaFileFormat.AddReadOutUnit("UH", "doublesidedstrip", StripType) == false) {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to define the roa read-out unit of the strip hits"<<endl;
      return false;
    }
    // The detector side files only contain strip hits, thus they have no crystal read-out unit
    if (m_SplitByDetectorSide == false) {
      if (CrystalType != "") {
        if (m_RoaFileFormat.AddReadOutUnit("UC", "voxel3d", CrystalType) == false) {
          if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to define the roa read-out unit of the crystal hits"<<endl;
          return false;
        }
      } else {
        // Crystal hits are always written as UC lines, so they need a declared read-out unit
        if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": The roa options select no read-out data the crystal hits have - enable the ADCs or the energies"<<endl;
        return false;
      }
    }
    for (unsigned int u = 0; u < m_RoaFileFormat.GetNumberOfReadOutUnits(); ++u) {
      Header<<m_RoaFileFormat.GetUFLine(u)<<endl;
    }
    Header<<endl;
  } else {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
    return false;
  }
  if (m_StartAreaFarField != 0.0) {
    Header<<"StartAreaFarField "<<m_StartAreaFarField<<endl;
    Header<<endl;
    Header<<"TB 0"<<endl;
    Header<<endl;
  }
  
  m_Header = Header.str();

  if (m_Out.IsOpen() == true) {
    m_Out.Write(m_Header);
  }
  
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEventSaver::StartSubFile()
{
  //! Start a new sub-file

  if (m_SubFileOut.IsOpen() == true) {
    m_SubFileOut.Write("EN");
    m_SubFileOut.Close();
  }
  
  MString SubName = m_InternalFileName;
  if (SubName.EndsWith(".gz") == true) {
    SubName.RemoveInPlace(SubName.Length() - 3);
  }
  
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
  } else if (m_Mode == c_TraFile) {
    SubName.ReplaceAllInPlace(".tra", "");
    SubName += ".";
    SubName += m_SubFileStart.GetAsSystemSeconds();
    SubName += ".tra";
  } else if (m_Mode == c_RoaFile) {
    SubName.ReplaceAllInPlace(".roa", "");
    SubName += ".";
    SubName += m_SubFileStart.GetAsSystemSeconds();
    SubName += ".roa";
  } else {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unsupported mode: "<<m_Mode<<endl;
    return false;
  }
  
  if (m_Zip == true) {
    SubName += ".gz";
  }
  
  m_SubFileOut.Open(SubName, MFile::c_Write);
  if (m_SubFileOut.IsOpen() == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open file: "<<SubName<<endl;
    return false;
  }
  m_SubFileOut.Write(m_Header);
  
  if (SubName.Last('/') != MString::npos) {
    SubName.RemoveInPlace(0, SubName.Last('/')+1); 
  }
  m_Out.Write("IN ");
  m_Out.Write(SubName);
  m_Out.Write('\n');
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


void MModuleEventSaver::Finalize()
{
  // Initialize the module 

  MModule::Finalize();
  
  if (m_SubFileOut.IsOpen() == true) {
    m_SubFileOut.Write("EN\n");
    m_SubFileOut.Close();
  }

  for (auto& F: m_DetectorSideFiles) {
    F.second.WriteLine("EN");
    F.second.WriteLine();
    if (m_NumberOfSimulatedEvents > 0) {
      F.second.WriteLine(MString("TS ") + m_NumberOfSimulatedEvents);
      F.second.WriteLine();
    }
  }
  CloseDetectorSideFiles();

  if (m_Out.IsOpen() == true) {
    m_Out.WriteLine("EN");
    m_Out.WriteLine();
    if (m_NumberOfSimulatedEvents > 0) {
      m_Out.WriteLine(MString("TS ") + m_NumberOfSimulatedEvents);
      m_Out.WriteLine();
    }
    m_Out.Close();
  }
  
  return;
}


////////////////////////////////////////////////////////////////////////////////


MFile* MModuleEventSaver::GetDetectorSideFile(unsigned int DetectorID, bool LowVoltageSide)
{
  //! Return the file for the given detector side, open it if it does not exist yet

  auto Iter = m_DetectorSideFiles.find(make_pair(DetectorID, LowVoltageSide));
  if (Iter != m_DetectorSideFiles.end()) {
    return &(Iter->second);
  }

  // Name: <base>.det<ID>.<lv|hv>.roa[.gz]
  MString Name = m_InternalFileName;
  if (Name.EndsWith(".gz") == true) {
    Name.RemoveInPlace(Name.Length() - 3);
  }
  if (Name.EndsWith(".roa") == true) {
    Name.RemoveInPlace(Name.Length() - 4);
  }
  Name += ".det";
  Name += DetectorID;
  Name += (LowVoltageSide == true) ? ".lv" : ".hv";
  Name += ".roa";
  if (m_Zip == true) {
    Name += ".gz";
  }

  // Construct the file in place, since MFile cannot be copied
  MFile& File = m_DetectorSideFiles[make_pair(DetectorID, LowVoltageSide)];
  File.Open(Name, MFile::c_Write);
  if (File.IsOpen() == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Unable to open file: "<<Name<<endl;
    m_DetectorSideFiles.erase(make_pair(DetectorID, LowVoltageSide));
    return nullptr;
  }
  File.Write(m_Header);

  return &File;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEventSaver::CloseDetectorSideFiles()
{
  //! Close all detector side files

  // The MFile destructor closes the files
  m_DetectorSideFiles.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEventSaver::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Write the event to disk
 
  if (m_SaveBadEvents == false) {
    if (Event->IsBad() == true) return true;
  }

  if (m_SavePoorQualityEvents == false) {
    if (Event->IsPoorQuality() == true) return true;
  }

  if (m_SaveVetoEvents == false) {
    if (Event->IsVeto() == true) return true;
  }


  if (m_SplitByDetectorSide == true) {
    // Find all detector sides with hits in this event (set keeps the files in a deterministic order)
    set<pair<unsigned int, bool>> DetectorSides;
    for (unsigned int h = 0; h < Event->GetNStripHits(); ++h) {
      MStripHit* SH = Event->GetStripHit(h);
      if (m_RoaWithNearestNeighbors == false && SH->IsNearestNeighbor() == true) continue;
      DetectorSides.insert(make_pair(SH->GetDetectorID(), SH->IsLowVoltageStrip()));
    }
    for (const pair<unsigned int, bool>& DS: DetectorSides) {
      MFile* File = GetDetectorSideFile(DS.first, DS.second);
      if (File == nullptr) {
        m_IsOK = false;
        return false;
      }
      ostringstream Out;
      Event->StreamRoaDetectorSide(Out, DS.first, DS.second, m_RoaWithADCs, m_RoaWithTACs, m_RoaWithEnergies, m_RoaWithTimings, m_RoaWithFlags, m_RoaWithOrigins, m_RoaWithNearestNeighbors);
      File->Write(Out);
    }

    Event->SetAnalysisProgress(MAssembly::c_EventSaver);

    return true;
  }

  MFile* Choosen = 0; // Wish C++ would allow unassigned references...
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

  ostringstream Out;
  if (m_Mode == c_EvtaFile) {
    Event->StreamEvta(Out);
  } else if (m_Mode == c_TraFile) {
    Event->StreamTra(Out);
  } else if (m_Mode == c_DatFile) {
    Event->StreamDat(Out, 3);
  } else if (m_Mode == c_RoaFile) {
    Event->StreamRoa(Out, m_RoaWithADCs, m_RoaWithTACs, m_RoaWithEnergies, m_RoaWithTimings, m_RoaWithTemperatures, m_RoaWithFlags, m_RoaWithOrigins, m_RoaWithNearestNeighbors);
  }
  Choosen->Write(Out);
  
  Event->SetAnalysisProgress(MAssembly::c_EventSaver);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleEventSaver::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsEventSaver* Options = new MGUIOptionsEventSaver(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleEventSaver::ReadXmlConfiguration(MXmlNode* Node)
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
  MXmlNode* SavePoorQualityEventsNode = Node->GetNode("SavePoorQualityEvents");
  if (SavePoorQualityEventsNode != 0) {
    m_SavePoorQualityEvents = SavePoorQualityEventsNode->GetValueAsBoolean();
  }
  MXmlNode* SaveVetoEventsNode = Node->GetNode("SaveVetoEvents");
  if (SaveVetoEventsNode != 0) {
    m_SaveVetoEvents = SaveVetoEventsNode->GetValueAsBoolean();
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

  MXmlNode* SplitByDetectorSideNode = Node->GetNode("SplitByDetectorSide");
  if (SplitByDetectorSideNode != nullptr) {
    m_SplitByDetectorSide = SplitByDetectorSideNode->GetValueAsBoolean();
  }

  MXmlNode* RoaWithADCsNode = Node->GetNode("RoaWithADCs");
  if (RoaWithADCsNode != nullptr) {
    m_RoaWithADCs = RoaWithADCsNode->GetValueAsBoolean();
  }
  MXmlNode* RoaWithTACsNode = Node->GetNode("RoaWithTACs");
  if (RoaWithTACsNode != nullptr) {
    m_RoaWithTACs = RoaWithTACsNode->GetValueAsBoolean();
  }
  MXmlNode* RoaWithEnergiesNode = Node->GetNode("RoaWithEnergies");
  if (RoaWithEnergiesNode != nullptr) {
    m_RoaWithEnergies = RoaWithEnergiesNode->GetValueAsBoolean();
  }
  MXmlNode* RoaWithTimingsNode = Node->GetNode("RoaWithTimings");
  if (RoaWithTimingsNode != nullptr) {
    m_RoaWithTimings = RoaWithTimingsNode->GetValueAsBoolean();
  }
  MXmlNode* RoaWithTemperaturesNode = Node->GetNode("RoaWithTemperatures");
  if (RoaWithTemperaturesNode != nullptr) {
    m_RoaWithTemperatures = RoaWithTemperaturesNode->GetValueAsBoolean();
  }
  MXmlNode* RoaWithFlagsNode = Node->GetNode("RoaWithFlags");
  if (RoaWithFlagsNode != nullptr) {
    m_RoaWithFlags = RoaWithFlagsNode->GetValueAsBoolean();
  }
  MXmlNode* RoaWithOriginsNode = Node->GetNode("RoaWithOrigins");
  if (RoaWithOriginsNode != nullptr) {
    m_RoaWithOrigins = RoaWithOriginsNode->GetValueAsBoolean();
  }
  MXmlNode* RoaWithNearestNeighborsNode = Node->GetNode("RoaWithNearestNeighbors");
  if (RoaWithNearestNeighborsNode != nullptr) {
    m_RoaWithNearestNeighbors = RoaWithNearestNeighborsNode->GetValueAsBoolean();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleEventSaver::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);  
  new MXmlNode(Node, "FileName", m_FileName);
  new MXmlNode(Node, "Mode", m_Mode);
  new MXmlNode(Node, "SaveBadEvents", m_SaveBadEvents);
  new MXmlNode(Node, "SavePoorQualityEvents", m_SavePoorQualityEvents);
  new MXmlNode(Node, "SaveVetoEvents", m_SaveVetoEvents);
  new MXmlNode(Node, "AddTimeTag", m_AddTimeTag);
  new MXmlNode(Node, "SplitFile", m_SplitFile);
  new MXmlNode(Node, "SplitFileTime", m_SplitFileTime.GetAsSystemSeconds());
  new MXmlNode(Node, "SplitByDetectorSide", m_SplitByDetectorSide);
  new MXmlNode(Node, "RoaWithADCs", m_RoaWithADCs);
  new MXmlNode(Node, "RoaWithTACs", m_RoaWithTACs);
  new MXmlNode(Node, "RoaWithEnergies", m_RoaWithEnergies);
  new MXmlNode(Node, "RoaWithTimings", m_RoaWithTimings);
  new MXmlNode(Node, "RoaWithTemperatures", m_RoaWithTemperatures);
  new MXmlNode(Node, "RoaWithFlags", m_RoaWithFlags);
  new MXmlNode(Node, "RoaWithOrigins", m_RoaWithOrigins);
  new MXmlNode(Node, "RoaWithNearestNeighbors", m_RoaWithNearestNeighbors);

  return Node;
}


// MModuleEventSaver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
