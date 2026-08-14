/*
 * MModuleTACcal.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Nicole Rodriquez Cavero
 * Sean Pike
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer, Nicole Rodriquez Cavero, Sean Pike.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MModuleTACcal
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleTACcal.h"
#include "MGUIExpoTACcut.h"
#include "MGUIExpoPlotSpectrum.h"
#include "MGUIOptionsTACcal.h"

// Standard libs:
#include <algorithm>
#include <limits>

// ROOT libs:


// MEGAlib libs:
#include "MModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleTACcal)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleTACcal::MModuleTACcal() : MModule()
{
  // Construct an instance of MModuleTACcal

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "TAC Calibration";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagTACcal";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_TACcal);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_StripPairing);
  AddSucceedingModuleType(MAssembly::c_DepthCorrection);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTACcal)
  // and implement all your GUI options

  // Can the program be run multi-threaded
  m_AllowMultiThreading = true;

  // Can we use multiple instances of this class
  m_AllowMultipleInstances = true;

  // Applying taccuts by default
  m_ApplyTACCuts = true;

  // Default coincidence window in ns
  m_CoincidenceWindow = 600.0;

  m_SideToIndex = {{'l', 0}, {'h', 1}, {'0', 0}, {'1', 1}, {'p', 0}, {'n', 1}};

}


////////////////////////////////////////////////////////////////////////////////


MModuleTACcal::~MModuleTACcal()
{
  // Delete this instance of MModuleTACcal
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTACcal::Initialize()
{
  // Initialize the module 

  if (LoadTACCalFile(m_TACCalFile) == false) {
    cout<<m_XmlTag<<": Error: TAC Calibration file could not be loaded."<<endl;
    return false;
  }

  // Some sanity checks:
  if (m_TACCal.size() == 0) {
    cout<<m_XmlTag<<": The TAC calibration data set is empty"<<endl;
    return false;
  }

  return MModule::Initialize();
}

////////////////////////////////////////////////////////////////////////////////
void MModuleTACcal::CreateExpos()
{
  if (HasExpos() == true) return;

  m_ExpoTACcut = new MGUIExpoTACcut(this);

  m_ExpoTACcut->SetTACHistogramArrangement(m_DetectorIDs);

  for (unsigned int i = 0; i < m_DetectorIDs.size(); ++i) {
    unsigned int DetID = m_DetectorIDs[i];
    m_ExpoTACcut->SetTACHistogramParameters(DetID, 200, 0, 6000);
  }

  m_Expos.push_back(m_ExpoTACcut);

  m_ExpoEnergySpectrum = new MGUIExpoPlotSpectrum(this);
  m_Expos.push_back(m_ExpoEnergySpectrum);
}

////////////////////////////////////////////////////////////////////////////////

void MModuleTACcal::ShowOptionsGUI()
{
  MGUIOptionsTACcal* Options =
    new MGUIOptionsTACcal(this);

  Options->Create();
  gClient->WaitForUnmap(Options);
}

////////////////////////////////////////////////////////////////////////////////

bool MModuleTACcal::AnalyzeEvent(MReadOutAssembly* Event) 
{
  if (HasExpos()) {
    for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {

      MStripHit* SH = Event->GetStripHit(i);

      m_ExpoEnergySpectrum->AddEnergyInitial(
        SH->GetEnergy(),
        SH->IsNearestNeighbor(),
        SH->IsLowVoltageStrip()
      );
    }
  }

  // Always apply TAC calibration
  if (ApplyTACCal(Event) == false){
    return false;
  }

  // Optionally apply TAC cuts
  if (m_ApplyTACCuts == true) {
    if (ApplyTACCuts(Event) == false) {
      return false;
    }
  }

  if (HasExpos()) {
    for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {

      MStripHit* SH = Event->GetStripHit(i);

      m_ExpoEnergySpectrum->AddEnergyFinal(
        SH->GetEnergy(),
        SH->IsNearestNeighbor(),
        SH->IsLowVoltageStrip()
      );

      if ((SH->IsGuardRing() == false) &&
          (SH->HasFastTiming() == true)) {

        m_ExpoTACcut->AddTAC(
          SH->GetDetectorID(),
          SH->GetTiming()
        );
      }
    }
  }

  return true;
}
      
////////////////////////////////////////////////////////////////////////////////

bool MModuleTACcal::ApplyTACCal(MReadOutAssembly* Event)
{
  // Loop through all strip hits in the event
  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
    // Get the current strip hit
    MStripHit* SH = Event->GetStripHit(i);

    // Guard rings are intentionally not TAC calibrated
    if (SH->IsGuardRing() == false) {

      int DetID = SH->GetDetectorID();
      int StripID = SH->GetStripID();
      char Side = SH->IsLowVoltageStrip() ? 'l' : 'h';
      
      // Check that this detector exists in the TAC calibration
      if (m_TACCal.find(DetID) == m_TACCal.end()) {
        cout<<m_XmlTag
            <<": Error: DetID "<<DetID
            <<" has no TAC calibration entries - skipping event"
            <<endl;
        return false;
      }

      // Check that this side is understood
      if (m_SideToIndex.find(Side) == m_SideToIndex.end()) {
        cout<<m_XmlTag
            <<": Error: Unable to identify Side "<<Side
            <<" - skipping event"
            <<endl;
        return false;
      }

      int SideIndex = m_SideToIndex[Side];

      // Check that this strip has TAC calibration parameters
      if (m_TACCal[DetID][SideIndex].find(StripID) ==
          m_TACCal[DetID][SideIndex].end()) {

        cout<<m_XmlTag
            <<": Error: StripID "<<StripID
            <<" on side "<<Side
            <<" has no TAC calibration entries - skipping event"
            <<endl;
        return false;
      }

      // Need at least slope and offset
      if (m_TACCal[DetID][SideIndex][StripID].size() < 2) {
        cout<<m_XmlTag
            <<": Error: StripID "<<StripID
            <<" on side "<<Side
            <<" does not have enough TAC calibration parameters - skipping event"
            <<endl;
        return false;
      }

      // Raw TAC value
      double TAC_timing = SH->GetTAC();
      
      // Convert TAC value into timing in ns
      double ns_timing =
          TAC_timing*m_TACCal[DetID][SideIndex][StripID][0]
          + m_TACCal[DetID][SideIndex][StripID][1];

      // Store calibrated timing
      SH->SetTiming(ns_timing); 
    }
  }

  // Mark TAC calibration as completed for this event
  Event->SetAnalysisProgress(MAssembly::c_TACcal);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool MModuleTACcal::ApplyTACCuts(MReadOutAssembly* Event) 
{
  // Find the max timing value for non-NN hits of an event
  // This will be used for the coincidence window
  double MaxTAC = -numeric_limits<double>::max();

  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {
    MStripHit* SH = Event->GetStripHit(i);

    if ((SH->IsGuardRing() == false) && (SH->HasFastTiming() == true) && (SH ->IsNearestNeighbor()==false)){
        double ns_timing = SH->GetTiming();
      
      if (ns_timing> MaxTAC) {
        MaxTAC = ns_timing;
      }
    }
  }

  // 200ns appears to be the minimum acceptable timing value for Nearest Neighbor hits
  constexpr double c_FLNoiseCut = 200.0;

  // TotalOffset: Earliest time (in ns) after which valid timing hits can appear, start of the allowed timing window
  constexpr double TotalOffset = 3000.0;
  
  // Apply TAC cuts
  
  for (unsigned int i = 0; i < Event->GetNStripHits();) {
    MStripHit* SH = Event->GetStripHit(i);
    bool Passed = true;

    if (SH->IsGuardRing()==false) {
      double SHTiming = SH->GetTiming();
      
      // Nearest neighbor and direct hit with slow timing
      if (SH->HasFastTiming() == false) {
        if (SHTiming <= c_FLNoiseCut) {
          Passed = false;
        }
      
      //Fast-timing hits must satisfy true and chance coincidence cuts
      } else {
        if ((SHTiming < TotalOffset) || (SHTiming < MaxTAC - m_CoincidenceWindow)) {
          Passed = false;
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


void MModuleTACcal::Finalize()
{
  MModule::Finalize();
}

////////////////////////////////////////////////////////////////////////////////


bool MModuleTACcal::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* TACCalFileNameNode = Node->GetNode("TACCalFileName");
  if (TACCalFileNameNode != nullptr) {
    SetTACCalFileName(TACCalFileNameNode->GetValue());
  }

  MXmlNode* ApplyTACCutsNode = Node->GetNode("ApplyTACCuts");
  if (ApplyTACCutsNode != nullptr) {
    SetApplyTACCuts(ApplyTACCutsNode->GetValueAsBoolean());
  }

  MXmlNode* TACCutNode = Node->GetNode("TACCut");
  if (TACCutNode != nullptr) {

    MXmlNode* CoincidenceWindowNode =
      TACCutNode->GetNode("CoincidenceWindow");

    if (CoincidenceWindowNode != nullptr) {
      SetCoincidenceWindow(
        CoincidenceWindowNode->GetValueAsDouble()
      );
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleTACcal::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  new MXmlNode(Node, "TACCalFileName", m_TACCalFile);

  new MXmlNode(Node, "ApplyTACCuts", m_ApplyTACCuts);
  
  MXmlNode* TACCutNode =
    new MXmlNode(Node, "TACCut");

  new MXmlNode(
    TACCutNode,
    "CoincidenceWindow",
    m_CoincidenceWindow
  );

  return Node;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTACcal::LoadTACCalFile(MString FName)
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
          
          // If this detector has not been encountered yet, create LV and HV calibration maps for it
          if (m_TACCal.find(DetID) == m_TACCal.end()) {
            vector<unordered_map<int, vector<double>>> TempVector;
            unordered_map<int, vector<double>> TempMapLV;
            unordered_map<int, vector<double>> TempMapHV;
            m_TACCal[DetID] = TempVector;
            m_TACCal[DetID].push_back(TempMapLV);
            m_TACCal[DetID].push_back(TempMapHV);
          }

          // Keep track of detector IDs contained in the calibration
          if (find(m_DetectorIDs.begin(), m_DetectorIDs.end(), DetID) == m_DetectorIDs.end()) {
            m_DetectorIDs.push_back(DetID);
          }
          
          // Store the calibration parameters
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

// MModuleTACcal.cxx: the end...
////////////////////////////////////////////////////////////////////////////////


