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
  m_Name = "TAC Calibration";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagTACcut";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  //AddPreceedingModuleType(MAssembly::c_EnergyCalibration);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_TACcut);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_StripPairing);
  AddSucceedingModuleType(MAssembly::c_DepthCorrection);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTACcut)
  // and implement all your GUI options

  // Can the program be run multi-threaded
  m_AllowMultiThreading = true;

  // Can we use multiple instances of this class
  m_AllowMultipleInstances = true;

  m_SideToIndex = {{'l', 0}, {'h', 1}, {'0', 0}, {'1', 1}, {'p', 0}, {'n', 1}};
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

  if (LoadTACCalFile(m_TACCalFile) == false) {
    cout<<m_XmlTag<<": Error: TAC Calibration file could not be loaded."<<endl;
    return false;
  }

  if (LoadTACCutFile(m_TACCutFile) == false) {
    cout<<m_XmlTag<<": Error: TAC Cut file could not be loaded."<<endl;
    return false;
  }

  // Some sanity checks:
  if (m_TACCal.size() == 0) {
    cout<<m_XmlTag<<": The TAC calibration data set is empty"<<endl;
    return false;
  }
  if (m_TACCut.size() == 0) {
    cout<<m_XmlTag<<": The TAC cut data set is empty"<<endl;
    return false;
  }

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MModuleTACcut::CreateExpos()
{
  // Create all expos

  if (HasExpos() == true) return;

  // Set the histogram display
  m_ExpoTACcut = new MGUIExpoTACcut(this);
  m_ExpoTACcut->SetTACHistogramArrangement(m_DetectorIDs);
  for (unsigned int i = 0; i < m_DetectorIDs.size(); ++i) {
    unsigned int DetID = m_DetectorIDs[i];
    m_ExpoTACcut->SetTACHistogramParameters(DetID, 200, 0, 6000);
  }
  m_Expos.push_back(m_ExpoTACcut);
}

////////////////////////////////////////////////////////////////////////////////


bool MModuleTACcut::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Start with sanity checks:
  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
    MStripHit* SH = Event->GetStripHit(i);

    int DetID = SH->GetDetectorID();
    int StripID = SH->GetStripID();
    char Side = SH->IsLowVoltageStrip() ? 'l' : 'h';

    if (DetID >= m_TACCal.size()) {
      cout<<m_XmlTag<<": Error: DetID "<<DetID<<" is not in TACCal (max det ID: "<<m_TACCal.size()-1<<") - skipping event"<<endl;
      return false;
    }
    if (m_TACCal[DetID][m_SideToIndex[Side]][StripID].size() == 0 && SH->IsGuardRing() == false) {
      cout<<m_XmlTag<<": Error: StripID "<<StripID<<" on side "<<Side<<" has no calibration entries) - skipping event"<<endl;
      return false;
    }
    if ((StripID >= m_TACCal[DetID][m_SideToIndex[Side]].size()) && (SH->IsGuardRing()==false)) {
      cout<<m_XmlTag<<": Error: StripID "<<StripID<<" on side "<<Side<<" is not in TACCal (max strip ID: "<<m_TACCal[DetID][m_SideToIndex[Side]].size()-1<<") - skipping event"<<endl;
      return false;
    }
    if (DetID >= m_TACCut.size()) {
      cout<<m_XmlTag<<": Error: DetID "<<DetID<<" is not in TACCut (max det ID: "<<m_TACCut.size()-1<<") - skipping event"<<endl;
      return false;
    }
    if (m_TACCut[DetID][m_SideToIndex[Side]][StripID].size() == 0 && SH->IsGuardRing()==false) {
      cout<<m_XmlTag<<": Error: StripID "<<StripID<<" on side "<<Side<<" has no entries) - skipping event"<<endl;
      return false;
    }
    if ((StripID >= m_TACCut[DetID][m_SideToIndex[Side]].size()) && (SH->IsGuardRing()==false)) {
      cout<<m_XmlTag<<": Error: StripID "<<StripID<<" on side "<<Side<<" is not in TACCut (max strip ID: "<<m_TACCut[DetID][m_SideToIndex[Side]].size()-1<<") - skipping event"<<endl;
      return false;
    }
  }

  double MaxTAC = -numeric_limits<double>::max();
  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
    MStripHit* SH = Event->GetStripHit(i);
    double TAC_timing = SH->GetTAC();
    double ns_timing = 0;
    int DetID = SH->GetDetectorID();
    int StripID = SH->GetStripID();
    char Side = SH->IsLowVoltageStrip() ? 'l' : 'h';
    if ((TAC_timing == 0) || (SH->IsGuardRing() == true)) {
      SH->HasCalibratedTiming(false);
    } else {
      ns_timing = TAC_timing*m_TACCal[DetID][m_SideToIndex[Side]][StripID][0] + m_TACCal[DetID][m_SideToIndex[Side]][StripID][1];
      SH->HasCalibratedTiming(true);
    } 
    SH->SetTiming(ns_timing);
    if ((SH->HasCalibratedTiming()==true) && (ns_timing > MaxTAC) && (SH->HasFastTiming()==true) && (SH->IsNearestNeighbor()==false)) {
      MaxTAC = ns_timing;
    }
  }

  for (unsigned int i = 0; i < Event->GetNStripHits();) {
    MStripHit* SH = Event->GetStripHit(i);
    bool Passed = true;
    if ((SH->HasCalibratedTiming() == true) && (SH->IsGuardRing()==false)) {
      double SHTiming = SH->GetTiming();
      int DetID = SH->GetDetectorID();
      int StripID = SH->GetStripID();
      char Side = SH->IsLowVoltageStrip() ? 'l' : 'h';
      double FLNoiseCut = m_TACCut[DetID][m_SideToIndex[Side]][StripID][4];
      if ((SHTiming < FLNoiseCut)) {
        Passed = false;
      } else if (SH->HasFastTiming()==true) {
        double ShapingOffset = m_TACCut[DetID][m_SideToIndex[Side]][StripID][0];
        double CoincidenceWindow = m_TACCut[DetID][m_SideToIndex[Side]][StripID][1];
        double DisableTime = m_TACCut[DetID][m_SideToIndex[Side]][StripID][2];
        double FlagToEnDelay = m_TACCut[DetID][m_SideToIndex[Side]][StripID][3];
        double FlagDelay = m_TACCut[DetID][m_SideToIndex[Side]][StripID][5];
        double TotalOffset = ShapingOffset + DisableTime + FlagToEnDelay + FlagDelay;
        if ((SHTiming > TotalOffset + CoincidenceWindow) || (SHTiming < TotalOffset) || (SHTiming < MaxTAC - CoincidenceWindow)) {
          Passed = false;
        } else if (HasExpos()==true) {
          m_ExpoTACcut->AddTAC(DetID, SHTiming);
        }
      }
    }
    if (Passed == true) {
      ++i;
    } else {
      Event->RemoveStripHit(i);
      delete SH;
    }
  }

  Event->SetAnalysisProgress(MAssembly::c_TACcut);

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

  MXmlNode* TACCalFileNameNode = Node->GetNode("TACCalFileName");
  if (TACCalFileNameNode != nullptr) {
    SetTACCalFileName(TACCalFileNameNode->GetValue());
  }

  MXmlNode* TACCutFileNameNode = Node->GetNode("TACCutFileName");
  if (TACCutFileNameNode != nullptr) {
    SetTACCutFileName(TACCutFileNameNode->GetValue());
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleTACcut::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  new MXmlNode(Node, "TACCalFileName", m_TACCalFile);
  new MXmlNode(Node, "TACCutFileName", m_TACCutFile);

  return Node;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTACcut::LoadTACCalFile(MString FName)
{
  // Read in the TAC Calibration file, which should contain for each strip:
  //  DetID, Side (h or l for high or low voltage), TAC cal, TAC cal error, TAC cal offset, TAC offset error
  // OR:
  // ReadOutID, Detector, Side, Strip, TAC cal, TAC cal error, TAC offset, TAC offset error
  MFile F;
  if (F.Open(FName) == false) {
    cout<<m_XmlTag<<": Error: failed to open TAC Calibration file."<<endl;
    return false;
  } else {
    MString Line;
    while (F.ReadLine(Line)) {
      if (!Line.BeginsWith("#")) {
        std::vector<MString> Tokens = Line.Tokenize(",");
        if ((Tokens.size() == 7) || (Tokens.size() == 8)) {
          int IndexOffset = Tokens.size() % 7;
          int DetID = Tokens[0+IndexOffset].ToInt();
          MString SideString = Tokens[1+IndexOffset].Trim();
          char Side;
          if (SideString.Length()!=1) {
            cout<<m_XmlTag<<": Error: Expected 1 character Side, got string \""<<SideString<<"\" in TAC calibration file."<<endl;
            return false;
          }
          else {
            Side = SideString[0];
          }
          int StripID = Tokens[2+IndexOffset].ToInt();
          double TACCal = Tokens[3+IndexOffset].ToDouble();
          double TACCalError = Tokens[4+IndexOffset].ToDouble();
          double Offset = Tokens[5+IndexOffset].ToDouble();
          double OffsetError = Tokens[6+IndexOffset].ToDouble();
          vector<double> CalValues;
          CalValues.push_back(TACCal); CalValues.push_back(Offset); CalValues.push_back(TACCalError); CalValues.push_back(OffsetError);
          
          if (m_TACCal.find(DetID) == m_TACCal.end()) {
            vector<unordered_map<int, vector<double>>> TempVector;
            unordered_map<int, vector<double>> TempMapLV;
            unordered_map<int, vector<double>> TempMapHV;
            m_TACCal[DetID] = TempVector;
            m_TACCal[DetID].push_back(TempMapLV);
            m_TACCal[DetID].push_back(TempMapHV);
          }

          if (find(m_DetectorIDs.begin(), m_DetectorIDs.end(), DetID) == m_DetectorIDs.end()) {
            m_DetectorIDs.push_back(DetID);
          }
          
          if (m_SideToIndex.find(Side) != m_SideToIndex.end()) {
            m_TACCal[DetID][m_SideToIndex[Side]][StripID] = CalValues;
          } else {
            cout<<m_XmlTag<<": Error: Unable to identify Side \""<<Side<<"\" in TAC calibration file."<<endl;
            return false;
          }
        }
      }
    }
    F.Close();
    sort(m_DetectorIDs.begin(), m_DetectorIDs.end());
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTACcut::LoadTACCutFile(MString FName)
{
  // Read in the TAC Cut file, which should contain for each strip:
  //  DetID, h or l for high or low voltage, StripID, shaping offset, coincidence window
  // or new format:
  //  DetID, h or l for high or low voltage, StripID, shaping offset, coincidence window, disable time, flag-to-enable delay, FL noise cut, flag delay
  MFile F;
  bool OldFormatMessage = false;
  if (F.Open(FName) == false) {
    cout<<m_XmlTag<<": Error: failed to open TAC Cut file."<<endl;
    return false;
  } else {
    MString Line;
    while (F.ReadLine(Line)) {
      if (!Line.BeginsWith("#")) {
        std::vector<MString> Tokens = Line.Tokenize(",");
        if (Tokens.size() >= 5) {
          int DetID = Tokens[0].ToInt();
          MString SideString = Tokens[1];
          char Side;
          if (SideString.Length()!=1) {
            cout<<m_XmlTag<<": Error: Expected 1 character Side, got string:\""<<SideString<<"\" in TAC cut file."<<endl;
            return false;
          } else {
            Side = SideString[0];
          }
          int StripID = Tokens[2].ToInt();
          double ShapingOffset = Tokens[3].ToDouble();
          double CoincidenceWindow = Tokens[4].ToDouble();
          if ((Tokens.size() == 5) && (OldFormatMessage==false)) {
            cout<<m_XmlTag<<": Error: TAC cut file is using the old format. Using default values for FL noise cut, flag delay, disable time, and flag-to-enable delay."<<endl;
            OldFormatMessage = true;
          }
          double DisableTime = 1400;
          double FlagToEnDelay = 104;
          double FLNoiseCut = 0;
          double FlagDelay = 0;
          if (Tokens.size()==9) {
            DisableTime = Tokens[5].ToDouble();
            FlagToEnDelay = Tokens[6].ToDouble();
            FLNoiseCut = Tokens[7].ToDouble();
            FlagDelay = Tokens[8].ToDouble();
          } else if (Tokens.size()!=5) {
            cout<<m_XmlTag<<": Error: Unrecognized TAC cut file format. Number of parameters is "<<Tokens.size()<<"."<<endl;
            return false;
          }
          vector<double> CutParams;
          CutParams.push_back(ShapingOffset); CutParams.push_back(CoincidenceWindow); CutParams.push_back(DisableTime); CutParams.push_back(FlagToEnDelay); CutParams.push_back(FLNoiseCut); CutParams.push_back(FlagDelay);
          
          if (m_TACCut.find(DetID) == m_TACCut.end()) {
            vector<unordered_map<int, vector<double>>> TempVector;
            unordered_map<int, vector<double>> TempMapLV;
            unordered_map<int, vector<double>> TempMapHV;
            m_TACCut[DetID] = TempVector;
            m_TACCut[DetID].push_back(TempMapLV);
            m_TACCut[DetID].push_back(TempMapHV);
          }

          if (find(m_DetectorIDs.begin(), m_DetectorIDs.end(), DetID) == m_DetectorIDs.end()) {
            m_DetectorIDs.push_back(DetID);
          }
          
          if (m_SideToIndex.find(Side) != m_SideToIndex.end()) {
            m_TACCut[DetID][m_SideToIndex[Side]][StripID] = CutParams;
          } else {
            cout<<m_XmlTag<<": Error: Unable to identify Side "<<Side<<" In TAC cut file."<<endl;
            return false;
          }
        }
      }
    }
    F.Close();
    sort(m_DetectorIDs.begin(), m_DetectorIDs.end());
  }

  return true;

}

// MModuleTACcut.cxx: the end...
////////////////////////////////////////////////////////////////////////////////


