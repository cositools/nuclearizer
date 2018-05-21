/*
 * MGUIOptionsSimulationLoader.cxx
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


// Include the header:
#include "MGUIOptionsSimulationLoader.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModuleSimulationLoader.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsSimulationLoader)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsSimulationLoader::MGUIOptionsSimulationLoader(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsSimulationLoader::~MGUIOptionsSimulationLoader()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsSimulationLoader::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_SimulationFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a simulations file:",
    dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetSimulationFileName());
  m_SimulationFileSelector->SetFileType("Sim file", "*.sim");
  m_SimulationFileSelector->SetFileType("Sim file (gzip'ed)", "*.sim.gz");
  m_OptionsFrame->AddFrame(m_SimulationFileSelector, LabelLayout);

  m_EnergyCalibrationFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select an energy calibration file:",
    dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetEnergyCalibrationFileName());
  m_EnergyCalibrationFileSelector->SetFileType("Ecal file", "*.ecal");
  m_OptionsFrame->AddFrame(m_EnergyCalibrationFileSelector, LabelLayout);

  m_DeadStripFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a dead strip file:",
    dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetDeadStripFileName());
  m_DeadStripFileSelector->SetFileType("Dead strips file", "*.txt");
  m_OptionsFrame->AddFrame(m_DeadStripFileSelector, LabelLayout);

  m_ThresholdFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a thresholds file:",
    dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetThresholdFileName());
  m_ThresholdFileSelector->SetFileType("Thresholds file", "*.dat");
  m_OptionsFrame->AddFrame(m_ThresholdFileSelector, LabelLayout);

  m_DepthCalibrationCoeffsFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a depth calibration coefficients file:",
    dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetDepthCalibrationCoeffsFileName());
  m_DepthCalibrationCoeffsFileSelector->SetFileType("Coefficients file", "*.txt");
  m_OptionsFrame->AddFrame(m_DepthCalibrationCoeffsFileSelector, LabelLayout);

  m_DepthCalibrationSplinesFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a depth calibration splines file:",
    dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->GetDepthCalibrationSplinesFileName());
  m_DepthCalibrationSplinesFileSelector->SetFileType("Splines file", "*.ctd");
  m_OptionsFrame->AddFrame(m_DepthCalibrationSplinesFileSelector, LabelLayout);
  
  
  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsSimulationLoader::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  bool Status = true;
  
  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
  
  if (Status == false) {
    return false;
  }

  // Call also base class
  return MGUIOptions::ProcessMessage(Message, Parameter1, Parameter2);
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsSimulationLoader::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetSimulationFileName(m_SimulationFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetEnergyCalibrationFileName(m_EnergyCalibrationFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetThresholdFileName(m_ThresholdFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetDeadStripFileName(m_DeadStripFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetDepthCalibrationCoeffsFileName(m_DepthCalibrationCoeffsFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader*>(m_Module)->SetDepthCalibrationSplinesFileName(m_DepthCalibrationSplinesFileSelector->GetFileName());
  
  return true;
}


// MGUIOptionsSimulationLoader: the end...
////////////////////////////////////////////////////////////////////////////////
