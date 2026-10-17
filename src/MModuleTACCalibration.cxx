/*
 * MModuleTACCalibration.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer, Nicole Rodriguez Cavero
 * Sean Pike
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer, Nicole Rodriguez Cavero, Sean Pike.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MModuleTACCalibration
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleTACCalibration.h"
#include "MGUIExpoTACcut.h"
#include "MGUIExpoPlotSpectrum.h"
#include "MGUIOptionsTACCalibration.h"
#include "MModuleLoaderMeasurementsHDF.h"
#include "MModuleLoaderMeasurementsL0.h"
#include "MSupervisor.h"

// Standard libs:
#include <algorithm>
#include <limits>

// ROOT libs:


// MEGAlib libs:
#include "MModule.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleTACCalibration)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleTACCalibration::MModuleTACCalibration() : MModule()
{
  // Construct an instance of MModuleTACCalibration

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "TAC Calibration";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagTACCalibration";

  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);

  // Set all types this modules handles
  AddModuleType(MAssembly::c_TACCalibration);

  // Set all modules, which can follow this module
  AddSucceedingModuleType(MAssembly::c_StripPairing);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTACCalibration)
  // and implement all your GUI options

  // Can the program be run multi-threaded
  m_AllowMultiThreading = true;

  // Can we use multiple instances of this class
  m_AllowMultipleInstances = true;

  // Applying taccuts by default
  m_ApplyTACCuts = true;

  // Default coincidence window in ns
  m_CoincidenceWindow = 600.0;

  // Only used if no preceding HDF or L0 loader provides a strip map
  m_FileNameStripMap = "";

  m_SideToIndex = {{'l', 0}, {'h', 1}, {'0', 0}, {'1', 1}, {'p', 0}, {'n', 1}};

  // Determined in Initalize function, only true when TAC calibration/cut happens after Energy Calibration
  m_PlotEnergySpectrum = false;

}


////////////////////////////////////////////////////////////////////////////////


MModuleTACCalibration::~MModuleTACCalibration()
{
  // Delete this instance of MModuleTACCalibration
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleTACCalibration::Initialize()
{
  // Initialize the module 

  // The detector, side, and strip of each TAC calibration entry are taken from a strip map:
  // Reuse the one of a preceding HDF or L0 loader, and only load our own if there is none
  const MStripMap* StripMap = FindUpstreamStripMap();
  if (StripMap != nullptr) {
    if (m_FileNameStripMap != "") {
      if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Using the strip map of a preceding loader - ignoring the strip map file "<<m_FileNameStripMap<<endl;
    }
  } else {
    if (m_FileNameStripMap == "") {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: No strip map available - without a preceding HDF or L0 loader, a strip map file has to be given in the TAC calibration options"<<endl;
      return false;
    }
    if (m_StripMap.Open(m_FileNameStripMap) == false) {
      if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: Unable to read strip map "<<m_FileNameStripMap<<endl;
      return false;
    }
    StripMap = &m_StripMap;
  }

  if (LoadTACCalFile(m_TACCalFile, StripMap) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: TAC Calibration file could not be loaded."<<endl;
    return false;
  }

  // Some sanity checks:
  if (m_TACCal.size() == 0) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": The TAC calibration data set is empty"<<endl;
    return false;
  }

  // Only show the before/after energy spectrum if the energies are already
  // calibrated when they reach this module. Default to false
  m_PlotEnergySpectrum = false;
  MSupervisor* Supervisor = MSupervisor::GetSupervisor();
  //Interate through all modules in MSupervisor
  for (unsigned int m = 0; m < Supervisor->GetNModules(); ++m) {
    MModule* M = Supervisor->GetModule(m);
    if (M == this) break; // reaches current module
    if (M->ProvidesModuleType(MAssembly::c_EnergyCalibration) == true) {
      m_PlotEnergySpectrum = true;
      break;
    }
  }


  return MModule::Initialize();
}

////////////////////////////////////////////////////////////////////////////////

MModule* MModuleTACCalibration::FindStripMapProvider()
{
  // Return the closest preceding module which provides a strip map (HDF or L0 loader)

  MModule* Provider = nullptr;

  MSupervisor* Supervisor = MSupervisor::GetSupervisor();
  for (unsigned int m = 0; m < Supervisor->GetNModules(); ++m) {
    MModule* M = Supervisor->GetModule(m);
    // Spawned instances of this module are not in the module list, thus also stop at the TAC calibration module type
    if (M == this || M->ProvidesModuleType(MAssembly::c_TACCalibration) == true) break;

    if (dynamic_cast<MModuleLoaderMeasurementsHDF*>(M) != nullptr || dynamic_cast<MModuleLoaderMeasurementsL0*>(M) != nullptr) {
      Provider = M;
    }
  }

  return Provider;
}

////////////////////////////////////////////////////////////////////////////////

const MStripMap* MModuleTACCalibration::FindUpstreamStripMap()
{
  // Return the strip map of the closest preceding HDF or L0 loader.
  // Preceding modules are already initialized, and their strip maps are not modified afterwards

  MModule* Provider = FindStripMapProvider();

  if (MModuleLoaderMeasurementsHDF* HDF = dynamic_cast<MModuleLoaderMeasurementsHDF*>(Provider)) {
    return &HDF->GetStripMap();
  }
  if (MModuleLoaderMeasurementsL0* L0 = dynamic_cast<MModuleLoaderMeasurementsL0*>(Provider)) {
    return &L0->GetStripMap();
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

void MModuleTACCalibration::CreateExpos()
{
  if (HasExpos() == true) return;

  m_ExpoTACcut = new MGUIExpoTACcut(this);

  m_ExpoTACcut->SetTACHistogramArrangement(m_DetectorIDs);

  for (unsigned int i = 0; i < m_DetectorIDs.size(); ++i) {
    unsigned int DetID = m_DetectorIDs[i];
    m_ExpoTACcut->SetTACHistogramParameters(DetID, 200, 0, 6000);
  }
  m_Expos.push_back(m_ExpoTACcut);

  // Only create the energy spectra GUI if requested
  if (m_PlotEnergySpectrum == true) {
    m_ExpoEnergySpectrum = new MGUIExpoPlotSpectrum(this);
    m_Expos.push_back(m_ExpoEnergySpectrum);
  } else {
    m_ExpoEnergySpectrum = nullptr; 
  }

}

////////////////////////////////////////////////////////////////////////////////

void MModuleTACCalibration::ShowOptionsGUI()
{
  MGUIOptionsTACCalibration* Options =
    new MGUIOptionsTACCalibration(this);

  Options->Create();
  gClient->WaitForUnmap(Options);
}

////////////////////////////////////////////////////////////////////////////////

bool MModuleTACCalibration::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Check if GUI is active and if the event has already been energy calibrated
  if ((HasExpos() == true) && (m_ExpoEnergySpectrum != nullptr)) {
    if (Event->HasAnalysisProgress(MAssembly::c_EnergyCalibration) == true) {
      for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {

        MStripHit* SH = Event->GetStripHit(i);

        m_ExpoEnergySpectrum->AddEnergyInitial(
          SH->GetEnergy(),
          SH->IsNearestNeighbor(),
          SH->IsLowVoltageStrip()
        );
      }
    }
  }
  

  // Always apply TAC calibration
  if (ApplyTACCal(Event) == false){
    return false;
  }

  // Optionally apply TAC cuts
  if (m_ApplyTACCuts == true && Event->HasTACCalibrationError() == false) {
    if (ApplyTACCuts(Event) == false) {
      return false;
    }
  }

  if (HasExpos() == true) {
    for (unsigned int i = 0; i < Event->GetNStripHits(); ++i) {

      MStripHit* SH = Event->GetStripHit(i);

      // Only plot final energies if they are calibrated
      if (m_ExpoEnergySpectrum != nullptr && Event->HasAnalysisProgress(MAssembly::c_EnergyCalibration) == true) {
        m_ExpoEnergySpectrum->AddEnergyFinal(
          SH->GetEnergy(),
          SH->IsNearestNeighbor(),
          SH->IsLowVoltageStrip()
        );
      }

      // Plot the calibrated TAC in ns for strips with valid fast timing
      if ((SH->IsGuardRing() == false) && (SH->HasFastTiming() == true)) {
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

bool MModuleTACCalibration::ApplyTACCal(MReadOutAssembly* Event)
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
        if (g_Verbosity >= c_Warning) cout<<m_XmlTag
            <<": Error: DetID "<<DetID
            <<" has no TAC calibration entries - skipping event"
            <<endl;
        return false;
      }

      // Check that this side is understood
      if (m_SideToIndex.find(Side) == m_SideToIndex.end()) {
        if (g_Verbosity >= c_Warning) cout<<m_XmlTag
            <<": Error: Unable to identify Side "<<Side
            <<" - skipping event"
            <<endl;
        return false;
      }

      int SideIndex = m_SideToIndex[Side];

      auto TACCalibration = m_TACCal[DetID][SideIndex].find(StripID);
      if (TACCalibration == m_TACCal[DetID][SideIndex].end() || TACCalibration->second.size() <2){

        if (g_Verbosity >= c_Warning) {
          cout<< m_XmlTag
              <<": Warning: No valid TAC calibration for DETID"<< DetID
              <<", StripID "<< StripID
              <<", Side "<< Side
              <<endl;
        }

        Event->SetTACCalibrationError(
          "No valid TAC calibration for DetID " + to_string(DetID) + 
          ", StripID " + to_string(StripID)
        );

        continue;
      }

      // Raw TAC value
      double TAC_timing = SH->GetTAC();
      
      // Convert TAC value into timing in ns
      double ns_timing =
          TAC_timing*TACCalibration->second[0]
          + TACCalibration->second[1];

      // Store calibrated timing
      SH->SetTiming(ns_timing); 
    }
  }

  // Mark TAC calibration as completed for this event
  Event->SetAnalysisProgress(MAssembly::c_TACCalibration);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool MModuleTACCalibration::ApplyTACCuts(MReadOutAssembly* Event) 
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

  // 200ns appears to be the minimum acceptable timing value for all hits
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

  Event->SetAnalysisProgress(MAssembly::c_TACCalibration);

  return true;
}

////////////////////////////////////////////////////////////////////////////////


void MModuleTACCalibration::Finalize()
{
  m_TACCal.clear();
  m_DetectorIDs.clear();
  
  MModule::Finalize();
}

////////////////////////////////////////////////////////////////////////////////


bool MModuleTACCalibration::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* TACCalFileNameNode = Node->GetNode("TACCalFileName");
  if (TACCalFileNameNode != nullptr) {
    SetTACCalFileName(TACCalFileNameNode->GetValue());
  }

  MXmlNode* FileNameStripMapNode = Node->GetNode("FileNameStripMap");
  if (FileNameStripMapNode != nullptr) {
    SetFileNameStripMap(FileNameStripMapNode->GetValue());
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


MXmlNode* MModuleTACCalibration::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  
  new MXmlNode(Node, "TACCalFileName", m_TACCalFile);

  new MXmlNode(Node, "FileNameStripMap", m_FileNameStripMap);

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


bool MModuleTACCalibration::LoadTACCalFile(MString FName, const MStripMap* StripMap)
{
  // Read in the TAC Calibration file, which should contain for each strip:
  //  DetID, Side (h or l for high or low voltage), TAC cal, TAC cal error, TAC cal offset, TAC offset error
  // OR:
  // ReadOutID, Detector, Side, Strip, TAC cal, TAC cal error, TAC offset, TAC offset error
  // With a strip map, detector, side, and strip are taken from the strip map via the read-out ID,
  // and the detector, side, and strip columns of the file are only cross-checked
  MFile F;
  if (F.Open(FName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: failed to open TAC Calibration file."<<endl;
    return false;
  }

  if (StripMap == nullptr) {
    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": No strip map given - using the detector, side, and strip columns of the TAC calibration file "<<FName<<endl;
  }

  // Counters for the cross-check of the TAC calibration file against the strip map
  unsigned int NEntries = 0;
  unsigned int NSkippedEntries = 0;
  unsigned int NDetectorMismatches = 0;
  unsigned int NSideMismatches = 0;
  unsigned int NStripMismatches = 0;

  MString Line;
  while (F.ReadLine(Line)) {
    if (Line.BeginsWith("#") == false && Line.BeginsWith("STRIP_ID") == false) {
      std::vector<MString> Tokens = Line.Tokenize(",");
      if ((Tokens.size() == 7) || (Tokens.size() == 8)) {
        int IndexOffset = Tokens.size() % 7;
        int DetID = Tokens[0+IndexOffset].ToInt();
        MString SideString = Tokens[1+IndexOffset].Trim();
        char Side;
        if (SideString.Length()!=1) {
          if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: Expected 1 character Side, got string \""<<SideString<<"\" in TAC calibration file."<<endl;
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

        if (m_SideToIndex.find(Side) == m_SideToIndex.end()) {
          if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: Unable to identify Side \""<<Side<<"\" in TAC calibration file."<<endl;
          return false;
        }
        int SideIndex = m_SideToIndex[Side];

        // Take detector, side, and strip from the strip map via the read-out ID
        if (StripMap != nullptr) {
          if (Tokens.size() == 7) {
            if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": Error: The TAC calibration file "<<FName<<" has no read-out ID column, which is required to look up detector, side, and strip in the strip map."<<endl;
            return false;
          }
          ++NEntries;

          // Read-out IDs of detectors which are not part of the strip map, e.g. not enabled during the run, are skipped
          int ReadOutID = Tokens[0].ToInt();
          if (ReadOutID < 0 || StripMap->HasReadOutID(ReadOutID) == false) {
            ++NSkippedEntries;
            continue;
          }

          int MapDetID = StripMap->GetDetectorID(ReadOutID);
          int MapSideIndex = (StripMap->IsLowVoltage(ReadOutID) == true) ? 0 : 1;
          int MapStripID = StripMap->GetStripNumber(ReadOutID);

          if (MapDetID != DetID) ++NDetectorMismatches;
          if (MapSideIndex != SideIndex) ++NSideMismatches;
          if (MapStripID != StripID) ++NStripMismatches;

          DetID = MapDetID;
          SideIndex = MapSideIndex;
          StripID = MapStripID;
        }

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
        m_TACCal[DetID][SideIndex][StripID] = CalValues;
      }
    }
  }
  F.Close();
  sort(m_DetectorIDs.begin(), m_DetectorIDs.end());

  if (NDetectorMismatches > 0 || NSideMismatches > 0 || NStripMismatches > 0) {
    if (g_Verbosity >= c_Warning) cout<<m_XmlTag<<": Warning: The TAC calibration file "<<FName<<" disagrees with the strip map for "
                                      <<NDetectorMismatches<<" detector, "<<NSideMismatches<<" side, and "<<NStripMismatches<<" strip values out of "
                                      <<NEntries - NSkippedEntries<<" entries - using the strip map values"<<endl;
  }
  if (NSkippedEntries > 0) {
    if (g_Verbosity >= c_Info) cout<<m_XmlTag<<": Skipped "<<NSkippedEntries<<" of "<<NEntries<<" entries of the TAC calibration file "<<FName<<" whose read-out ID is not in the strip map"<<endl;
  }

  return true;
}

// MModuleTACCalibration.cxx: the end...
////////////////////////////////////////////////////////////////////////////////


