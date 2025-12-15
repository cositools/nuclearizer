/*
 * MGUIOptionsDEESMEX.cxx
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
#include "MGUIOptionsDEESMEX.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleDEESMEX.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsDEESMEX)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDEESMEX::MGUIOptionsDEESMEX(MModule* Module)
    : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsDEESMEX::~MGUIOptionsDEESMEX()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsDEESMEX::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 10, 10);

  m_EnergyCalibrationFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select an energy calibration file:",
                                                          dynamic_cast<MModuleDEESMEX*>(m_Module)->GetEnergyCalibrationFileName());
  m_EnergyCalibrationFileSelector->SetFileType("Ecal file", "*.ecal");
  m_OptionsFrame->AddFrame(m_EnergyCalibrationFileSelector, LabelLayout);

  /*
  m_DeadtimeFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a deadtime parameters file:",
    dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetDeadtimeFileName());
  m_DeadtimeFileSelector->SetFileType("Deadtime file", "*.txt");
  m_OptionsFrame->AddFrame(m_DeadtimeFileSelector, LabelLayout);
  */

  /*
  m_DeadStripFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a dead strip file:",
    dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetDeadStripFileName());
  m_DeadStripFileSelector->SetFileType("Dead strips file", "*.txt");
  m_OptionsFrame->AddFrame(m_DeadStripFileSelector, LabelLayout);

  m_ThresholdFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a thresholds file:",
    dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetThresholdFileName());
  m_ThresholdFileSelector->SetFileType("Thresholds file", "*.dat");
  m_OptionsFrame->AddFrame(m_ThresholdFileSelector, LabelLayout);

  m_GuardRingThresholdFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a guard ring thresholds file:",
  dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetGuardRingThresholdFileName());
  m_GuardRingThresholdFileSelector->SetFileType("Thresholds file", "*.dat");
  m_OptionsFrame->AddFrame(m_GuardRingThresholdFileSelector, LabelLayout);

  m_ChargeSharingFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a charge sharing factor file:",
  dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetChargeSharingFileName());
  m_ChargeSharingFileSelector->SetFileType("Charge sharing file", "*.txt");
  m_OptionsFrame->AddFrame(m_ChargeSharingFileSelector, LabelLayout);

  m_CrosstalkFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a crosstalk coefficients file:",
  dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetCrosstalkFileName());
  m_CrosstalkFileSelector->SetFileType("Crosstalk file", "*.txt");
  m_OptionsFrame->AddFrame(m_CrosstalkFileSelector, LabelLayout);

  m_ChargeLossFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a charge loss coefficients file:",
  dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetChargeLossFileName());
  m_ChargeLossFileSelector->SetFileType("Charge loss file", "*.log");
  m_OptionsFrame->AddFrame(m_ChargeLossFileSelector, LabelLayout);

  m_DepthCalibrationCoeffsFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a depth calibration coefficients file:",
    dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetDepthCalibrationCoeffsFileName());
  m_DepthCalibrationCoeffsFileSelector->SetFileType("Coefficients file", "*.txt");
  m_OptionsFrame->AddFrame(m_DepthCalibrationCoeffsFileSelector, LabelLayout);

  m_DepthCalibrationTACCalFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a TAC calibration parameters file:",
    dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetDepthCalibrationTACCalFileName());
  m_DepthCalibrationTACCalFileSelector->SetFileType("TAC calibration file", "*.csv");
  m_OptionsFrame->AddFrame(m_DepthCalibrationTACCalFileSelector, LabelLayout);

  m_DepthCalibrationSplinesFileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select a depth calibration splines file:",
    dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetDepthCalibrationSplinesFileName());
  m_DepthCalibrationSplinesFileSelector->SetFileType("Splines file", "*.ctd");
  m_OptionsFrame->AddFrame(m_DepthCalibrationSplinesFileSelector, LabelLayout);
  */

  // shield energy correction file
  m_ShieldEnergyCorrectionFileSelector = new MGUIEFileSelector(m_OptionsFrame,
                                                               "Please select an energy correction file for the Shield DEE:",
                                                               dynamic_cast<MModuleDEESMEX*>(m_Module)->GetShieldEnergyCorrectionFileName());
  m_ShieldEnergyCorrectionFileSelector->SetFileType("Shield DEE energy correction file", "*.csv");
  m_OptionsFrame->AddFrame(m_ShieldEnergyCorrectionFileSelector, LabelLayout);

  /*
  m_ApplyFudgeFactorSelector = new TGCheckButton(m_OptionsFrame, "Apply fudge factor to better match fluxes", 1);
  m_ApplyFudgeFactorSelector->SetOn(dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->GetApplyFudgeFactor());
  m_OptionsFrame->AddFrame(m_ApplyFudgeFactorSelector, LabelLayout);
  */

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsDEESMEX::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    case kCM_CHECKBUTTON:
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


bool MGUIOptionsDEESMEX::OnApply()
{
  // Modify this to store the data in the module!

  // Shield options:
  dynamic_cast<MModuleDEESMEX*>(m_Module)->SetShieldEnergyCorrectionFileName(m_ShieldEnergyCorrectionFileSelector->GetFileName());

  // GeD options:
  dynamic_cast<MModuleDEESMEX*>(m_Module)->SetEnergyCalibrationFileName(m_EnergyCalibrationFileSelector->GetFileName());

  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetDeadtimeFileName(m_DeadtimeFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetThresholdFileName(m_ThresholdFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetGuardRingThresholdFileName(m_GuardRingThresholdFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetChargeSharingFileName(m_ChargeSharingFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetCrosstalkFileName(m_CrosstalkFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetChargeLossFileName(m_ChargeLossFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetDeadStripFileName(m_DeadStripFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetDepthCalibrationCoeffsFileName(m_DepthCalibrationCoeffsFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetDepthCalibrationTACCalFileName(m_DepthCalibrationTACCalFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetDepthCalibrationSplinesFileName(m_DepthCalibrationSplinesFileSelector->GetFileName());
  // ACS DEE energy correction
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetACSEnergyCorrectionFileName(m_ACSEnergyCorrectionFileSelector->GetFileName());
  //dynamic_cast<MModuleLoaderSimulationsSingleDet*>(m_Module)->SetApplyFudgeFactor(m_ApplyFudgeFactorSelector->IsOn());


  return true;
}


// MGUIOptionsDEESMEX: the end...
////////////////////////////////////////////////////////////////////////////////
