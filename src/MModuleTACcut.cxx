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
  AddModuleType(MAssembly::c_TACcut);


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
  m_MaximumTAC = 20000;
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

  vector<MDDetector*> detectors = m_Geometry->GetDetectorList();

  for(unsigned int i = 0; i < detectors.size(); ++i){
    // For now, DetID is in order of detectors, which puts contraints on how the geometry file should be written.
    unsigned int DetID = i;
    MDDetector* det = detectors[i];
    if (det->GetNNamedDetectors() > 0){
      MString det_name = det->GetNamedDetectorName(0);
      cout << "Found detector " << det_name << " corresponding to DetID=" << DetID << "." << endl;
      m_DetectorIDs.push_back(DetID);
    }
  }

  if( LoadTACCalFile(m_TACCalFile) == false ){
    cout << "TAC Calibration file could not be loaded." << endl;
    return false;
  }

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
    m_ExpoTACcut->SetTACHistogramParameters(DetID, 120, 0, 20000);
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

    if ( HasExpos()==true ){
      m_ExpoTACcut->AddTAC(SH->GetDetectorID(), SH->GetTiming());
    }
    // takes inputted min and max TAC values from the GUI module to make cuts 
    if ( SH->GetTiming() < m_MinimumTAC || SH->GetTiming() > m_MaximumTAC) {
      // cout<<"HACK: Removing strip ht due to TAC "<<SH->GetTiming()<<" cut or energy "<<SH->GetEnergy()<<endl;
      Event->RemoveStripHit(i);
      delete SH;
    } else {
      ++i;
    }
  }

  for (unsigned int i = 0; i < Event->GetNStripHits(); ++i){
    MStripHit* SH = Event->GetStripHit(i);
    double TAC_timing = SH->GetTiming();
    double ns_timing;
    int DetID = SH->GetDetectorID();
    int StripID = SH->GetStripID();
    if ( SH->IsLowVoltageStrip() == true ){
      ns_timing = TAC_timing*m_LVTACCal[DetID][StripID][0] + m_LVTACCal[DetID][StripID][1];
    }
    else{
      ns_timing = TAC_timing*m_HVTACCal[DetID][StripID][0] + m_HVTACCal[DetID][StripID][1];
    }
    SH->SetTiming(ns_timing);
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

bool MModuleTACcut::LoadTACCalFile(MString FName)
{
  // Read in the TAC Calibration file, which should contain for each strip:
  //  DetID, h or l for high or low voltage, TAC cal, TAC cal error, TAC cal offset, TAC offset error
  MFile F;
  if( F.Open(FName) == false ){
    cout << "MModuleTACcut: failed to open TAC Calibration file." << endl;
    return false;
  } else {
    for(unsigned int i = 0; i < m_DetectorIDs.size(); ++i){
      unordered_map<int, vector<double>> temp_map_HV;
      m_HVTACCal[i] = temp_map_HV;
      unordered_map<int, vector<double>> temp_map_LV;
      m_LVTACCal[i] = temp_map_LV;
    }
    MString Line;
    while( F.ReadLine( Line ) ){
      if( !Line.BeginsWith("#") ){
        std::vector<MString> Tokens = Line.Tokenize(",");
        if( Tokens.size() == 7 ){
          int DetID = Tokens[0].ToInt();
          int StripID = Tokens[2].ToInt();
          double taccal = Tokens[3].ToDouble();
          double taccal_err = Tokens[4].ToDouble();
          double offset = Tokens[5].ToDouble();
          double offset_err = Tokens[6].ToDouble();
          vector<double> cal_vals;
          cal_vals.push_back(taccal); cal_vals.push_back(offset); cal_vals.push_back(taccal_err); cal_vals.push_back(offset_err);
          if ( Tokens[1] == "l" ){
            m_LVTACCal[DetID][StripID] = cal_vals;
          }
          else if ( Tokens[1] == "h" ){
            m_HVTACCal[DetID][StripID] = cal_vals;
          }
        }
      }
    }
    F.Close();
  }

  return true;

}


// MModuleTACcut.cxx: the end...
