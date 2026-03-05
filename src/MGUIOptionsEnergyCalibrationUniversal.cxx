/*
 * MGUIOptionsEnergyCalibrationUniversal.cxx
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
#include "MGUIOptionsEnergyCalibrationUniversal.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModule.h"
#include "MModuleEnergyCalibrationUniversal.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsEnergyCalibrationUniversal)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEnergyCalibrationUniversal::MGUIOptionsEnergyCalibrationUniversal(MModule* Module) 
  : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEnergyCalibrationUniversal::~MGUIOptionsEnergyCalibrationUniversal()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsEnergyCalibrationUniversal::Create()
{
  PreCreate();

  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 10, 10);
  TGLayoutHints* FileSelectorLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 10, 10, 10, 10);

  // File loader for energy calibration file
  m_FileSelector = new MGUIEFileSelector(m_OptionsFrame, "Please select an energy calibration file:",
    dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("Energy calibration file", "*.ecal");
  m_OptionsFrame->AddFrame(m_FileSelector, FileSelectorLayout);


  TGLabel* SlowThresholdLabel = new TGLabel(m_OptionsFrame, "Please choose how to handle the slow threshold cut:");
  m_OptionsFrame->AddFrame(SlowThresholdLabel, LabelLayout);


  TGLayoutHints* RBLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 40, 10, 2, 0);
  TGLayoutHints* RBOptionLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop, 60, 10, 2, 0);
  TGLayoutHints* RBOptionStretchLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 60, 10, 2, 0);

  m_SlowThresholdCutRBIgnore = new TGRadioButton(m_OptionsFrame, "Do not apply a slow threshold cut", c_SlowThresholdIgnore);
  m_SlowThresholdCutRBIgnore->Associate(this);
  m_OptionsFrame->AddFrame(m_SlowThresholdCutRBIgnore, RBLayout);


  m_SlowThresholdCutRBFixed = new TGRadioButton(m_OptionsFrame, "Use one uniform slow threshold cut for all strips", c_SlowThresholdFixed);
  m_SlowThresholdCutRBFixed->Associate(this);
  m_OptionsFrame->AddFrame(m_SlowThresholdCutRBFixed, RBLayout);

  m_SlowThresholdCutFixedValue = new MGUIEEntry(m_OptionsFrame, "Set threshold value [keV]:", false, dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetSlowThresholdCutFixedValue(), true, 0.0);
  m_OptionsFrame->AddFrame(m_SlowThresholdCutFixedValue, RBOptionLayout);


  m_SlowThresholdCutRBFile = new TGRadioButton(m_OptionsFrame, "Read slow threshold cuts from file (unique to each strip)", c_SlowThresholdFile);
  m_SlowThresholdCutRBFile->Associate(this);
  m_OptionsFrame->AddFrame(m_SlowThresholdCutRBFile, RBLayout);

  m_SlowThresholdCutFileSelector = new MGUIEFileSelector(m_OptionsFrame, "", dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetSlowThresholdCutFileName());
  m_SlowThresholdCutFileSelector->SetFileType("Slow threshold cut per strip file", "*.csv");
  m_OptionsFrame->AddFrame(m_SlowThresholdCutFileSelector, RBOptionStretchLayout);


  // Toggle the right button
  MSlowThresholdCutModes SlowThresholdCutMode = dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetSlowThresholdCutMode();
  ToggleRadioButtons(static_cast<int>(SlowThresholdCutMode));
  
  
  // Nearest Neighbor threshold cuts!
  TGLabel* NearestNeighborLabel = new TGLabel(m_OptionsFrame, "Please choose how to handle the threshold cut for Nearest Neighbors:");
  m_OptionsFrame->AddFrame(NearestNeighborLabel, LabelLayout);

  m_SlowThresholdCutNearestNeighborRBIgnore = new TGRadioButton(m_OptionsFrame, "Do not apply a slow threshold cut to Nearest Neighbors", c_SlowThresholdNearestNeighborIgnore);
  m_SlowThresholdCutNearestNeighborRBIgnore->Associate(this);
  m_OptionsFrame->AddFrame(m_SlowThresholdCutNearestNeighborRBIgnore, RBLayout);
      
  m_SlowThresholdCutNearestNeighborRBFixed = new TGRadioButton(m_OptionsFrame, "Use one uniform slow threshold cut for all Nearest Neighbors", c_SlowThresholdNearestNeighborFixed);
  m_SlowThresholdCutNearestNeighborRBFixed->Associate(this);
  m_OptionsFrame->AddFrame(m_SlowThresholdCutNearestNeighborRBFixed, RBLayout);
      
  m_SlowThresholdCutNearestNeighborFixedValue = new MGUIEEntry(m_OptionsFrame, "Set threshold value [keV] for Nearest Neighbors:", false, dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetNearestNeighborThreshold());
  m_OptionsFrame->AddFrame(m_SlowThresholdCutNearestNeighborFixedValue, RBOptionLayout);
  
  MNearestNeighborCutModes NearestNeighborCutMode = dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetNearestNeighborCutMode();
  ToggleRadioButtons(static_cast<int>(NearestNeighborCutMode));
  
  
  // Plot spectrum options 
  TGLabel* PlotSpectrumLabel = new TGLabel(m_OptionsFrame, "Please choose a spectrum plotting and memory option:");
  m_OptionsFrame->AddFrame(PlotSpectrumLabel, LabelLayout);
    
  // Don't plot
  m_PlotSpectrumNoneRB = new TGRadioButton(m_OptionsFrame, "Do not plot spectrum", c_PlotSpectrumNone);
  m_PlotSpectrumNoneRB->Associate(this);
  m_OptionsFrame->AddFrame(m_PlotSpectrumNoneRB, RBLayout);

  // Plot without the buffer
  m_PlotSpectrumNoBufferRB = new TGRadioButton(m_OptionsFrame, "Plot spectrum without buffering data (uses less memory)", c_PlotSpectrumNoBuffer);
  m_PlotSpectrumNoBufferRB->Associate(this);
  m_OptionsFrame->AddFrame(m_PlotSpectrumNoBufferRB, RBLayout);
    
  // Plot with the buffer
  m_PlotSpectrumWithBufferRB = new TGRadioButton(m_OptionsFrame, "Plot spectrum with buffered data (warning: uses x2 memory!)", c_PlotSpectrumWithBuffer);
  m_PlotSpectrumWithBufferRB->Associate(this);
  m_OptionsFrame->AddFrame(m_PlotSpectrumWithBufferRB, RBLayout);

  int PlotMode = dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->GetPlotSpectrumMode();
  ToggleRadioButtons(PlotMode);
  

  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEnergyCalibrationUniversal::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    case kCM_RADIOBUTTON:
      ToggleRadioButtons(Parameter1);
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


void MGUIOptionsEnergyCalibrationUniversal::ToggleRadioButtons(int WidgetID)
{
  // Toggle the radio buttons and the entry fields

  if (WidgetID == m_SlowThresholdCutRBIgnore->WidgetId()) {
    m_SlowThresholdCutRBIgnore->SetState(kButtonDown);
    m_SlowThresholdCutRBFixed->SetState(kButtonUp);
    m_SlowThresholdCutFixedValue->SetEnabled(false);
    m_SlowThresholdCutRBFile->SetState(kButtonUp);
    m_SlowThresholdCutFileSelector->SetEnabled(false);
  } else if (WidgetID == m_SlowThresholdCutRBFixed->WidgetId()) {
    m_SlowThresholdCutRBIgnore->SetState(kButtonUp);
    m_SlowThresholdCutRBFixed->SetState(kButtonDown);
    m_SlowThresholdCutFixedValue->SetEnabled(true);
    m_SlowThresholdCutRBFile->SetState(kButtonUp);
    m_SlowThresholdCutFileSelector->SetEnabled(false);
  } else if (WidgetID == m_SlowThresholdCutRBFile->WidgetId()) {
    m_SlowThresholdCutRBIgnore->SetState(kButtonUp);
    m_SlowThresholdCutRBFixed->SetState(kButtonUp);
    m_SlowThresholdCutFixedValue->SetEnabled(false);
    m_SlowThresholdCutRBFile->SetState(kButtonDown);
    m_SlowThresholdCutFileSelector->SetEnabled(true);
  }
  
  // Nearest Neighbors
  if (WidgetID == c_SlowThresholdNearestNeighborIgnore || WidgetID == c_SlowThresholdNearestNeighborFixed) {
    if (WidgetID == c_SlowThresholdNearestNeighborIgnore) {
      m_SlowThresholdCutNearestNeighborRBIgnore->SetState(kButtonDown);
      m_SlowThresholdCutNearestNeighborRBFixed->SetState(kButtonUp);
      m_SlowThresholdCutNearestNeighborFixedValue->SetEnabled(false);
    } else if (WidgetID == c_SlowThresholdNearestNeighborFixed) {
      m_SlowThresholdCutNearestNeighborRBIgnore->SetState(kButtonUp);
      m_SlowThresholdCutNearestNeighborRBFixed->SetState(kButtonDown);
      m_SlowThresholdCutNearestNeighborFixedValue->SetEnabled(true);
    }
  }
  
  // Plot spectrum
  if (WidgetID == c_PlotSpectrumNone) {
    m_PlotSpectrumNoneRB->SetState(kButtonDown);
    m_PlotSpectrumNoBufferRB->SetState(kButtonUp);
    m_PlotSpectrumWithBufferRB->SetState(kButtonUp);
  } else if (WidgetID == c_PlotSpectrumNoBuffer) {
    m_PlotSpectrumNoneRB->SetState(kButtonUp);
    m_PlotSpectrumNoBufferRB->SetState(kButtonDown);
    m_PlotSpectrumWithBufferRB->SetState(kButtonUp);
  } else if (WidgetID == c_PlotSpectrumWithBuffer) {
    m_PlotSpectrumNoneRB->SetState(kButtonUp);
    m_PlotSpectrumNoBufferRB->SetState(kButtonUp);
    m_PlotSpectrumWithBufferRB->SetState(kButtonDown);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEnergyCalibrationUniversal::OnApply()
{
 // Modify this to store the data in the module!

  dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetFileName(m_FileSelector->GetFileName());

  if (m_SlowThresholdCutRBIgnore->GetState() == kButtonDown) {
    dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetSlowThresholdCutMode(MSlowThresholdCutModes::e_Ignore);
  } else if (m_SlowThresholdCutRBFixed->GetState() == kButtonDown) {
    dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetSlowThresholdCutMode(MSlowThresholdCutModes::e_Fixed);
  }  if (m_SlowThresholdCutRBFile->GetState() == kButtonDown) {
    dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetSlowThresholdCutMode(MSlowThresholdCutModes::e_File);
  }

  dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetSlowThresholdCutFixedValue(m_SlowThresholdCutFixedValue->GetAsDouble());
  dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetSlowThresholdCutFileName(m_SlowThresholdCutFileSelector->GetFileName());
  
  // Nearest Neighbors
  if (m_SlowThresholdCutNearestNeighborRBIgnore->GetState() == kButtonDown) {
    dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetNearestNeighborCutMode(MNearestNeighborCutModes::e_Ignore);
  } else {
    dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetNearestNeighborCutMode(MNearestNeighborCutModes::e_Fixed);
  }

  dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetNearestNeighborThreshold(m_SlowThresholdCutNearestNeighborFixedValue->GetAsDouble());
  
  // Plot spectrum
  if (m_PlotSpectrumNoneRB->GetState() == kButtonDown) {
      dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetPlotSpectrumMode(MEnergyCalibrationPlotSpectrumModes::e_PlotNone);
    } else if (m_PlotSpectrumNoBufferRB->GetState() == kButtonDown) {
      dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetPlotSpectrumMode(MEnergyCalibrationPlotSpectrumModes::e_PlotNoBuffer);
    } else if (m_PlotSpectrumWithBufferRB->GetState() == kButtonDown) {
      dynamic_cast<MModuleEnergyCalibrationUniversal*>(m_Module)->SetPlotSpectrumMode(MEnergyCalibrationPlotSpectrumModes::e_PlotWithBuffer);
    }

  return true;
}


// MGUIOptionsEnergyCalibrationUniversal: the end...
////////////////////////////////////////////////////////////////////////////////
