/*
 * MNCTData.cxx
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
// MNCTData
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTData.h"

// Standard libs:

// ROOT libs:
#include "TSystem.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MXmlDocument.h"
#include "MXmlNode.h"
#include "MNCTModule.h"
#include "MNCTModuleMeasurementLoaderROA.h"
#include "MNCTModuleMeasurementLoaderNCT2009.h"
#include "MNCTModuleMeasurementLoaderGRIPS2013.h"
#include "MNCTModuleReceiverCOSI2014.h"
#include "MNCTModuleSimulationLoader.h"
#include "MNCTModuleEnergyCalibration.h"
#include "MNCTModuleEnergyCalibrationUniversal.h"
#include "MNCTModuleEnergyCalibrationLinear.h"
#include "MNCTModuleEnergyCalibrationNonlinear.h"
#include "MNCTModuleCrosstalkCorrection.h"
#include "MNCTModuleChargeSharingCorrection.h"
#include "MNCTModuleDepthAndStripCalibration.h"
#include "MNCTModuleDepthCalibration.h"
#include "MNCTModuleDepthCalibrationLinearStrip.h"
#include "MNCTModuleDepthCalibrationLinearPixel.h"
#include "MNCTModuleDepthCalibration3rdPolyPixel.h"
#include "MNCTModuleStripPairing.h"
#include "MNCTModuleStripPairingGreedy.h"
#include "MNCTModuleStripPairingGreedy_a.h"
#include "MNCTModuleStripPairingGreedy_b.h"
#include "MNCTModuleAspect.h"
#include "MNCTModuleEventFilter.h"
#include "MNCTModuleDumpEvent.h"
#include "MNCTModuleEventSaver.h"
#include "MNCTModuleTransmitterRealta.h"
//#include "MNCTModuleEventReconstruction.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTData)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTData::MNCTData()
{
  // Construct an instance of MNCTData

  m_Geometry = 0;
  Clear();

  // Add in this list all available modules:
  m_AvailableModules.push_back(new MNCTModuleSimulationLoader());
  m_AvailableModules.push_back(new MNCTModuleMeasurementLoaderROA());
  m_AvailableModules.push_back(new MNCTModuleMeasurementLoaderGRIPS2013());
  m_AvailableModules.push_back(new MNCTModuleMeasurementLoaderNCT2009());
  m_AvailableModules.push_back(new MNCTModuleReceiverCOSI2014());
  
  m_AvailableModules.push_back(new MNCTModuleEventFilter());
  //m_AvailableModules.push_back(new MNCTModuleEnergyCalibration());
  m_AvailableModules.push_back(new MNCTModuleEnergyCalibrationUniversal());
  m_AvailableModules.push_back(new MNCTModuleEnergyCalibrationLinear());
  m_AvailableModules.push_back(new MNCTModuleEnergyCalibrationNonlinear());
  m_AvailableModules.push_back(new MNCTModuleCrosstalkCorrection());
  m_AvailableModules.push_back(new MNCTModuleChargeSharingCorrection());
  //m_AvailableModules.push_back(new MNCTModuleDepthAndStripCalibration());
  //m_AvailableModules.push_back(new MNCTModuleDepthCalibration());
  //m_AvailableModules.push_back(new MNCTModuleDepthCalibrationLinearStrip());
  m_AvailableModules.push_back(new MNCTModuleDepthCalibrationLinearPixel());
  m_AvailableModules.push_back(new MNCTModuleDepthCalibration3rdPolyPixel());
  //m_AvailableModules.push_back(new MNCTModuleStripPairing());
  //m_AvailableModules.push_back(new MNCTModuleStripPairingGreedy());
  m_AvailableModules.push_back(new MNCTModuleStripPairingGreedy_a());
	m_AvailableModules.push_back(new MNCTModuleStripPairingGreedy_b());
  m_AvailableModules.push_back(new MNCTModuleAspect());
  m_AvailableModules.push_back(new MNCTModuleDumpEvent());
  m_AvailableModules.push_back(new MNCTModuleEventSaver());
  m_AvailableModules.push_back(new MNCTModuleTransmitterRealta());
  //m_AvailableModules.push_back(new MNCTModuleEventReconstruction());

  // All the rest:
}


////////////////////////////////////////////////////////////////////////////////


void MNCTData::Clear()
{
  //! Reset all data

  m_Modules.clear();
  m_GeometryFileName = "";
  delete m_Geometry;
  m_Geometry = 0;
  m_LoadFileName = "";
  m_SaveFileName = "";
}


////////////////////////////////////////////////////////////////////////////////


MNCTData::~MNCTData()
{
  // Delete this instance of MNCTData

  for (unsigned int m = 0; m < m_AvailableModules.size(); ++m) {
    delete m_AvailableModules[m];
  }

  delete m_Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModule* MNCTData::GetModule(unsigned int i) 
{ 
  //! Return the modules at position i in the current sequence --- no error checks are performed  

  if (i < m_Modules.size()) {
    return m_Modules[i];
  }

  return 0; 
}


////////////////////////////////////////////////////////////////////////////////


MNCTModule* MNCTData::GetAvailableModule(unsigned int i) 
{ 
  //! Return the modules at position i in the current sequence --- no error checks are performed  

  if (i < m_AvailableModules.size()) {
    return m_AvailableModules[i]; 
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModule* MNCTData::GetAvailableModule(MString Name) 
{ 
  //! Return the modules at position i in the current sequence --- no error checks are performed  

  for (unsigned int m = 0; m < m_AvailableModules.size(); ++m) {
    if (m_AvailableModules[m]->GetName() == Name) {
      return m_AvailableModules[m];
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


vector<MNCTModule*> MNCTData::ReturnPossibleVolumes(unsigned int Position)
{
  vector<MNCTModule*> Previous;
  if (Position > 0) {
    for (unsigned int i = 0; i < Position; ++i) {
      Previous.push_back(m_Modules[i]);
    }
  }

  return ReturnPossibleVolumes(Previous);
}


////////////////////////////////////////////////////////////////////////////////


vector<MNCTModule*> MNCTData::ReturnPossibleVolumes(vector<MNCTModule*>& Previous)
{
  vector<MNCTModule*> List;
  
  for (unsigned int i = 0; i < m_AvailableModules.size(); ++i) {
    //cout<<"Module: "<<m_AvailableModules[i]->GetName()<<endl;

    bool Passed = true;

    // (1) Check if the requirement on previous modules can be passed 
    for (unsigned int m = 0; m < m_AvailableModules[i]->GetNPreceedingModuleTypes(); ++m) {
      bool Found = false;
      for (unsigned int p = 0; p < Previous.size(); ++p) {
        for (unsigned int t = 0; t < Previous[p]->GetNModuleTypes(); ++t) {
          if (Previous[p]->GetModuleType(t) == m_AvailableModules[i]->GetPreceedingModuleType(m)) {
            Found = true;
            break;
          }
        }
        if (Found == true) break;
      }
      if (Found == false) {
        //mout<<"Failed Previous requirement"<<endl;
        Passed = false;
        break;
      }
    }

    // (2) If any of it is already in the list ignore it:
    if (Passed == true) {
      bool AlreadyInList = false;
      for (unsigned int p = 0; p < m_Modules.size(); ++p) {
        if (m_Modules[p] == m_AvailableModules[i]) {
          AlreadyInList = true;
          break;
        }
      }
      if (AlreadyInList == true) {
        //mout<<"Failed Already in list requirement"<<endl;
        Passed = false;
      }
    }

    // (3) If any of the module types are already in the list, ignore it:
    if (Passed == true) {
      bool AlreadyInList = false;
      for (unsigned int p = 0; p < Previous.size(); ++p) {
        for (unsigned int t = 0; t < Previous[p]->GetNModuleTypes(); ++t) {
          for (unsigned int at = 0; at < m_AvailableModules[i]->GetNModuleTypes(); ++at) {
            if (Previous[p]->GetModuleType(t) == m_AvailableModules[i]->GetModuleType(at)) {
              AlreadyInList = true;
              break;
            }
          }
        }
      }
      if (AlreadyInList == true) {
        //mout<<"Failed module type already in list requirement"<<endl;
        Passed = false;
      }
    }


    // (4) If any of the modules which can follow are already in the list, also ignore it
//     if (Passed == true) {
//       bool AlreadyInList = false;
//       for (unsigned int at = 0; at < m_AvailableModules[i]->GetNSucceedingModuleTypes(); ++at) {
//         for (unsigned int p = 0; p < Previous.size(); ++p) {
//           for (unsigned int t = 0; t < Previous[p]->GetNModuleTypes(); ++t) {
//             if (Previous[p]->GetModuleType(t) == m_AvailableModules[i]->GetSucceedingModuleType(at)) {
//               AlreadyInList = true;
//               break;
//             }
//           }
//         }
//       }
//       if (AlreadyInList == true) {
//         //mout<<"Failed Successor requirement"<<endl;        
//         Passed = false;
//       }
//     }
      
    if (Passed == true) {
      //mout<<"Passed :)"<<endl;
      List.push_back(m_AvailableModules[i]);
    }
  }

  return List;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTData::SetModule(MNCTModule* Module, unsigned int i) 
{
  // Set a module at a specific position - return false if other modules had to be eliminated  

  if (m_Modules.size() > i) {
    m_Modules[i] = Module;
  } else if (m_Modules.size() == i) {
    m_Modules.push_back(Module);
  } else {
    merr<<"Unable to add module"<<endl;
  }

  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTData::RemoveModule(unsigned int i)
{
  //! Remove module at a specific position - return false if other modules had to be eliminated  

  if (i < m_Modules.size()) {
    m_Modules.erase(m_Modules.begin() + i);
  } else {
    merr<<"Unable to remove module"<<endl;
  }

  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTData::Validate()
{
  // Validate

  unsigned int ValidUntil = m_Modules.size();

  // (1) Make sure no module appears twice:
  for (unsigned int m1 = 0; m1 < m_Modules.size(); ++m1) {
    for (unsigned int m2 = m1+1; m2 < m_Modules.size(); ++m2) {
      if (m_Modules[m1] == m_Modules[m2]) {
        mout<<"Module: "<<m_Modules[m1]->GetName()<<" appears twice: "<<m1<<" & "<<m2<<endl;
        if (m2 < ValidUntil) {
          ValidUntil = m2;
        }
      }
    }
  }

  // (2) Make sure all predecessor requirements are fulfilled
  vector<int> PredecessorTypes;
  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    for (unsigned int t = 0; t < m_Modules[m]->GetNPreceedingModuleTypes(); ++t) {
      bool Found = false;
      for (unsigned int p = 0; p < PredecessorTypes.size(); ++p) {
        if (PredecessorTypes[p] == m_Modules[m]->GetPreceedingModuleType(t)) {
          Found = true;
        }
      }
      if (Found == false) {
        mout<<"Predecessor requirements for module "<<m_Modules[m]->GetName()<<" are not fullfilled"<<endl;
        if (m < ValidUntil) {
          ValidUntil = m;
        }
        break;
      }
    }
    if (ValidUntil == m) break;

    for (unsigned int t = 0; t < m_Modules[m]->GetNModuleTypes(); ++t) {
      PredecessorTypes.push_back(m_Modules[m]->GetModuleType(t));
    }
  }

  // (3) Make sure all succecessor requirements are fulfilled
  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    for (unsigned int s = m+1; s < m_Modules.size(); ++s) {
      for (unsigned int st = 0; st < m_Modules[s]->GetNModuleTypes(); ++st) {
        bool Found = false;
        for (unsigned int mt = 0; mt < m_Modules[m]->GetNSucceedingModuleTypes(); ++mt) {
          if (m_Modules[m]->GetSucceedingModuleType(mt) == m_Modules[s]->GetModuleType(st)) {
            Found = true;
            break;
          }
        }
        if (Found == false) {
          mout<<"Succecessor requirements for module "<<m_Modules[m]->GetName()<<" are not fullfilled"<<endl;
          if (s < ValidUntil) {
            ValidUntil = s;
          }
          break;
        }
        if (ValidUntil == s) break;
      }
      if (ValidUntil == s) break;
    }
  }

  // Many possible validations are missing

  //cout<<"Valid until: "<<ValidUntil<<endl;
  while (ValidUntil < m_Modules.size()) {
    cout<<"Erasing some modules!"<<endl;
    m_Modules.erase(m_Modules.begin()+ValidUntil);
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTData::Load(MString FileName)
{
  // Load all data from a file

  Clear();

  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument();
  Document->Load(FileName);

  int Version = 1;
  MXmlNode* VersionNode = Document->GetNode("Version");
  if (VersionNode != 0) {
    Version = VersionNode->GetValueAsInt();
  }

  MXmlNode* ModuleSequence = Document->GetNode("ModuleSequence");
  if (ModuleSequence != 0) {
    for (unsigned int m = 0; m < ModuleSequence->GetNNodes(); ++m) {
      MNCTModule* M = GetAvailableModule(ModuleSequence->GetNode(m)->GetValue());
      if (M != 0) {
        m_Modules.push_back(M);
      } else {
        mout<<"Error: Cannot find a module with name: "<<ModuleSequence->GetNode(m)->GetValue()<<endl;
      }
    }
  }

  MXmlNode* LoadFileName = Document->GetNode("LoadFileName");
  if (LoadFileName != 0) {
    m_LoadFileName = LoadFileName->GetValue();
  }
  MXmlNode* SaveFileName = Document->GetNode("SaveFileName");
  if (SaveFileName != 0) {
    m_SaveFileName = SaveFileName->GetValue();
  }
  MXmlNode* GeometryFileName = Document->GetNode("GeometryFileName");
  if (GeometryFileName != 0) {
    m_GeometryFileName = GeometryFileName->GetValue();
  }

  MXmlNode* ModuleOptions = Document->GetNode("ModuleOptions");
  if (ModuleOptions != 0) {
    for (unsigned int a = 0; a < m_AvailableModules.size(); ++a) {
      MXmlNode* ModuleNode = ModuleOptions->GetNode(m_AvailableModules[a]->GetXmlTag());
      if (ModuleNode != 0) {
        m_AvailableModules[a]->ReadXmlConfiguration(ModuleNode);
      }
    }
  }

  delete Document;

  return true;
} 


////////////////////////////////////////////////////////////////////////////////


bool MNCTData::Save(MString FileName)
{
  //! Save all data to a file

  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument("NuclearizerData");

  // Version string: 1
  new MXmlNode(Document, "Version", 1);

  MXmlNode* ModuleSequence = new MXmlNode(Document, "ModuleSequence");
  // Store the module sequence
  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    new MXmlNode(ModuleSequence, "ModuleSequenceItem", m_Modules[m]->GetName());
  }

  // Store the file names
  new MXmlNode(Document, "LoadFileName", m_LoadFileName);
  new MXmlNode(Document, "SaveFileName", m_SaveFileName);
  new MXmlNode(Document, "GeometryFileName", m_GeometryFileName);

  MXmlNode* ModuleOptions = new MXmlNode(Document, "ModuleOptions");
  for (unsigned int a = 0; a < m_AvailableModules.size(); ++a) {
    MXmlNode* ModuleNode = m_AvailableModules[a]->CreateXmlConfiguration();
    ModuleOptions->AddNode(ModuleNode);
  }

  // Store the module content
  Document->Save(FileName);

  delete Document;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTData::LoadGeometry()
{
  //! Load the geometry and transfer it to all modules

  m_Geometry = new MDGeometryQuest();

  if (m_Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    mlog<<"Geometry "<<m_Geometry->GetName()<<" loaded!"<<endl;
    m_Geometry->ActivateNoising(false);
    m_Geometry->SetGlobalFailureRate(0.0);
  } else {
    mlog<<"Loading of geometry "<<m_GeometryFileName<<" failed!!"<<endl;
    delete m_Geometry;
    m_Geometry = 0;
    return false;
  }  

  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    m_Modules[m]->SetGeometry(m_Geometry);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MNCTData::GetHighestAnalysisLevel() const
{
  //! Get highest analysis level

  bool FoundEventReconstruction = false;
  bool FoundStripPairing = false;
  bool FoundDepthCorrection = false;
  bool FoundChargeSharingCorrection = false;
  bool FoundCrosstalkCorrection = false;
  bool FoundEnergyCalibration = false;
  bool FoundAspect = false;
  bool FoundElse = false;
  bool FoundDetectorEffectsEngine = false;
  bool FoundEventFilter = false;

  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    for (unsigned int t = 0; t < m_Modules[m]->GetNModuleTypes(); ++t) {
      if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_EventReconstruction) {
        FoundEventReconstruction = true;
      } else if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_StripPairing) {
        FoundStripPairing = true;
      } else if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_DepthCorrection) {
        FoundDepthCorrection = true;
      } else if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_ChargeSharingCorrection) {
        FoundChargeSharingCorrection = true;
      } else if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_EnergyCalibration) {
        FoundEnergyCalibration = true;
      } else if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_Aspect){
        FoundAspect = true;
      } else if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_Else){
        FoundElse = true;
      } else if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_EventFilter){
        FoundEventFilter = true;
      } else if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_DetectorEffectsEngine) {
        FoundDetectorEffectsEngine = true;
      } else if (m_Modules[m]->GetModuleType(t) == MNCTModule::c_CrosstalkCorrection) {
        FoundCrosstalkCorrection = true;
      } else {
        merr<<"Not implemented module type: "<<m_Modules[m]->GetModuleType(t)<<endl;
      }
    }
  }

  if (FoundEventReconstruction == true) {
    return c_DataReconstructed;
  } else if (FoundEnergyCalibration == true &&
	     //             FoundChargeSharingCorrection == true &&
             FoundDepthCorrection == true &&
             FoundStripPairing == true) {
    return c_DataCalibrated;
  } else if (FoundDetectorEffectsEngine == true){
    return c_DataSimed;
  }

  return c_DataRaw;
}


// MNCTData.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
