/*
 * MModuleLoaderMeasurementsTRA.cxx
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
// MModuleLoaderMeasurementsTRA
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleLoaderMeasurementsTRA.h"

// Standard libs:
#include <algorithm>

// ROOT libs:
#include "TGClient.h"

// MEGAlib libs:
#include "MGUIOptionsLoaderMeasurements.h"
#include "MPhysicalEventHit.h"
#include "MPhotoEvent.h"

// Nuclearizer libs:
#include "MAssembly.h"
#include "MHit.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleLoaderMeasurementsTRA)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsTRA::MModuleLoaderMeasurementsTRA() : MModuleLoaderMeasurements()
{
  // Construct an instance of MModuleLoaderMeasurementsTRA

  // Set the module name --- has to be unique
  m_Name = "Measurement loader for TRA files";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagMeasurementLoaderTRA";

  // Full pipeline
  AddModuleType(MAssembly::c_EventLoader);
  AddModuleType(MAssembly::c_EnergyCalibration);
  AddModuleType(MAssembly::c_TACCalibration);
  AddModuleType(MAssembly::c_StripPairing);
  AddModuleType(MAssembly::c_DepthCorrection);
  AddModuleType(MAssembly::c_PositionDetermiation);
  AddModuleType(MAssembly::c_EventReconstruction);

  // This is a special start module which can generate its own events
  m_IsStartModule = true;

  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleLoaderMeasurementsTRA::~MModuleLoaderMeasurementsTRA()
{
  // Delete this instance of MModuleLoaderMeasurementsTRA
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsTRA::Initialize()
{
  // Initialize the module

  if (Open(m_FileName, c_Read) == false) return false;

  m_NEventsInFile = 0;
  m_NGoodEventsInFile = 0;

  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsTRA::AnalyzeEvent(MReadOutAssembly* Event)
{
  // Main data analysis routine, which fills the read-out assembly with the next event from file

  Event->Clear();

  MPhysicalEvent* PhysicalEvent = m_TraFile.GetNextEvent();
  if (PhysicalEvent == nullptr) {
    if (g_Verbosity >= c_Info) cout<<m_Name<<": No more events!"<<endl;
    m_IsFinished = true;
    return false;
  }

  Event->SetID(PhysicalEvent->GetId());
  Event->SetTime(PhysicalEvent->GetTime());
  Event->SetTimeUTC(PhysicalEvent->GetTime());

  for (unsigned int b = 0; b < PhysicalEvent->GetNBadFlags(); ++b) {
    MString BadFlag = PhysicalEvent->GetBadFlag(b);
    MString Flag = BadFlag;
    MString Reason = "";

    size_t ReasonStart = BadFlag.Index(" (");
    if (ReasonStart != MString::npos) {
      Flag = BadFlag.GetSubString(0, ReasonStart);
      Reason = BadFlag.GetSubString(ReasonStart + 2);
      if (Reason.EndsWith(")") == true) {
        Reason.RemoveInPlace(Reason.Length() - 1);
      }
    }

    if (Flag == "EnergyCalibrationError") {
      Event->SetEnergyCalibrationError(Reason);
    } else if (Flag == "TACCalibrationError") {
      Event->SetTACCalibrationError(Reason);
    } else if (Flag == "StripPairingError") {
      Event->SetStripPairingError(Reason);
    } else if (Flag == "DepthCalibrationError") {
      Event->SetDepthCalibrationError(Reason);
    } else if (Flag == "EventReconstructionError") {
      Event->SetEventReconstructionError(Reason);
    } else if (Flag == "GR Veto") {
      Event->SetGuardRingVeto(true);
    } else if (Flag == "Shield Veto") {
      Event->SetShieldVeto(true);
    }
  }

  // Hits: Compton events carry their hit sequence, photo events a single position and energy
  if (PhysicalEvent->GetNHits() > 0) {
    for (unsigned int h = 0; h < PhysicalEvent->GetNHits(); ++h) {
      const MPhysicalEventHit& PhysicalHit = PhysicalEvent->GetHit(h);

      MHit* Hit = new MHit();
      Hit->SetPosition(PhysicalHit.GetPosition());
      Hit->SetPositionResolution(PhysicalHit.GetPositionUncertainty());
      Hit->SetEnergy(PhysicalHit.GetEnergy());
      Hit->SetEnergyResolution(PhysicalHit.GetEnergyUncertainty());
      Event->AddHit(Hit);
    }
  } else if (PhysicalEvent->GetType() == MPhysicalEvent::c_Photo) {
    MPhotoEvent* Photo = dynamic_cast<MPhotoEvent*>(PhysicalEvent);
    if (Photo != nullptr) {
      // The tra file has no uncertainties for photo events
      MHit* Hit = new MHit();
      Hit->SetPosition(Photo->GetPosition());
      Hit->SetPositionResolution(MVector(0.0, 0.0, 0.0));
      Hit->SetEnergy(Photo->GetEnergy());
      Hit->SetEnergyResolution(0.0);
      Event->AddHit(Hit);
    }
  }

  // The assembly stores its own copy
  Event->SetPhysicalEvent(PhysicalEvent);
  delete PhysicalEvent;

  m_NEventsInFile++;
  if (Event->IsGood() == true) {
    m_NGoodEventsInFile++;
  }

  Event->SetAnalysisProgress(MAssembly::c_EventLoader | MAssembly::c_EnergyCalibration | MAssembly::c_TACCalibration | MAssembly::c_StripPairing | MAssembly::c_DepthCorrection | MAssembly::c_PositionDetermiation | MAssembly::c_EventReconstruction);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsTRA::Finalize()
{
  // Finalize the module

  MModule::Finalize();

  if (g_Verbosity >= c_Info) {
    cout<<"MModuleLoaderMeasurementsTRA: "<<endl;
    cout<<"  * all events on file: "<<m_NEventsInFile<<endl;
    cout<<"  * good events on file: "<<m_NGoodEventsInFile<<endl;
  }

  m_TraFile.Close();
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsTRA::Open(MString FileName, unsigned int Way)
{
  // Open the tra file

  if (m_TraFile.IsOpen() == true) {
    m_TraFile.Close();
  }

  if (m_TraFile.Open(FileName) == false) {
    if (g_Verbosity >= c_Error) cout<<m_XmlTag<<": An error occured opening the file "<<FileName<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleLoaderMeasurementsTRA::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != nullptr) {
    m_FileName = FileNameNode->GetValue();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModuleLoaderMeasurementsTRA::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(nullptr, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);

  return Node;
}


///////////////////////////////////////////////////////////////////////////////


void MModuleLoaderMeasurementsTRA::ShowOptionsGUI()
{
  //! Show the options GUI

  MGUIOptionsLoaderMeasurements* Options = new MGUIOptionsLoaderMeasurements(this, "tra");
  Options->Create();
  gClient->WaitForUnmap(Options);
}


// MModuleLoaderMeasurementsTRA.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
