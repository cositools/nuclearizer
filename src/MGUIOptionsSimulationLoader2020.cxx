/*
 * MGUIOptionsSimulationLoader2020.cxx
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
#include "MGUIOptionsSimulationLoader2020.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MNCTModuleSimulationLoader2020.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsSimulationLoader2020)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsSimulationLoader2020::MGUIOptionsSimulationLoader2020(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsSimulationLoader2020::~MGUIOptionsSimulationLoader2020()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsSimulationLoader2020::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_SimulationFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a simulations file:",
    dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetSimulationFileName());
  m_SimulationFileSelector->SetFileType("Sim file", "*.sim");
  m_SimulationFileSelector->SetFileType("Sim file (gzip'ed)", "*.sim.gz");
  m_OptionsFrame->AddFrame(m_SimulationFileSelector, LabelLayout);

  m_EnergyCalibrationFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select an energy calibration file:",
    dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetEnergyCalibrationFileName());
  m_EnergyCalibrationFileSelector->SetFileType("Ecal file", "*.ecal");
  m_OptionsFrame->AddFrame(m_EnergyCalibrationFileSelector, LabelLayout);

  m_DeadStripFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a dead strip file:",
    dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetDeadStripFileName());
  m_DeadStripFileSelector->SetFileType("Dead strips file", "*.txt");
  m_OptionsFrame->AddFrame(m_DeadStripFileSelector, LabelLayout);

  m_ThresholdFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a thresholds file:",
    dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetThresholdFileName());
  m_ThresholdFileSelector->SetFileType("Thresholds file", "*.dat");
  m_OptionsFrame->AddFrame(m_ThresholdFileSelector, LabelLayout);

	m_ChargeSharingFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a charge sharing factor file:",
		dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetChargeSharingFileName());
	m_ChargeSharingFileSelector->SetFileType("Charge sharing file", "*.txt");
	m_OptionsFrame->AddFrame(m_ChargeSharingFileSelector, LabelLayout);

	m_CrosstalkFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a crosstalk coefficients file:",
		dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetCrosstalkFileName());
	m_CrosstalkFileSelector->SetFileType("Crosstalk file", "*.txt");
	m_OptionsFrame->AddFrame(m_CrosstalkFileSelector, LabelLayout);

	m_ChargeLossFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a charge loss coefficients file:",
		dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetChargeLossFileName());
	m_ChargeLossFileSelector->SetFileType("Charge loss file", "*.log");
	m_OptionsFrame->AddFrame(m_ChargeLossFileSelector, LabelLayout);

  m_DepthCalibrationCoeffsFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a depth calibration coefficients file:",
    dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetDepthCalibrationCoeffsFileName());
  m_DepthCalibrationCoeffsFileSelector->SetFileType("Coefficients file", "*.txt");
  m_OptionsFrame->AddFrame(m_DepthCalibrationCoeffsFileSelector, LabelLayout);

  m_DepthCalibrationSplinesFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a depth calibration splines file:",
    dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetDepthCalibrationSplinesFileName());
  m_DepthCalibrationSplinesFileSelector->SetFileType("Splines file", "*.ctd");
  m_OptionsFrame->AddFrame(m_DepthCalibrationSplinesFileSelector, LabelLayout);
  
  m_ApplyFudgeFactorSelector = new TGCheckButton(m_OptionsFrame, "Apply fudge factor to better match fluxes", 1);
	m_ApplyFudgeFactorSelector->SetOn(dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->GetApplyFudgeFactor());
	m_OptionsFrame->AddFrame(m_ApplyFudgeFactorSelector, LabelLayout);

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsSimulationLoader2020::ProcessMessage(long Message, long Parameter1, long Parameter2)
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


bool MGUIOptionsSimulationLoader2020::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetSimulationFileName(m_SimulationFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetEnergyCalibrationFileName(m_EnergyCalibrationFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetThresholdFileName(m_ThresholdFileSelector->GetFileName());
	dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetChargeSharingFileName(m_ChargeSharingFileSelector->GetFileName());
	dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetCrosstalkFileName(m_CrosstalkFileSelector->GetFileName());
	dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetChargeLossFileName(m_ChargeLossFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetDeadStripFileName(m_DeadStripFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetDepthCalibrationCoeffsFileName(m_DepthCalibrationCoeffsFileSelector->GetFileName());
  dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetDepthCalibrationSplinesFileName(m_DepthCalibrationSplinesFileSelector->GetFileName());
	dynamic_cast<MNCTModuleSimulationLoader2020*>(m_Module)->SetApplyFudgeFactor(m_ApplyFudgeFactorSelector->IsOn());
 
  return true;
}


// MGUIOptionsSimulationLoader2020: the end...
////////////////////////////////////////////////////////////////////////////////
