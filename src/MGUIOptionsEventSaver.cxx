/*
 * MGUIOptionsEventSaver.cxx
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
#include "MGUIOptionsEventSaver.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <MString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MModuleEventSaver.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MGUIOptionsEventSaver)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventSaver::MGUIOptionsEventSaver(MModule* Module)
    : MGUIOptions(Module)
{
  // standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MGUIOptionsEventSaver::~MGUIOptionsEventSaver()
{
  // kDeepCleanup is activated
}


////////////////////////////////////////////////////////////////////////////////


void MGUIOptionsEventSaver::Create()
{
  PreCreate();

  m_MainTab = new TGTab(m_OptionsFrame, 300, 300);
  TGLayoutHints* MainTabLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY, 0, 0, 0, 0);
  m_OptionsFrame->AddFrame(m_MainTab, MainTabLayout);

  TGCompositeFrame* TypeFrame = m_MainTab->AddTab("File");
  TGCompositeFrame* GeneralFrame = m_MainTab->AddTab("General Options");
  TGCompositeFrame* RoaFrame = m_MainTab->AddTab("Roa Options");
  //TGCompositeFrame* Evtarame = m_MainTab->AddTab("Evta Options");


  TGLayoutHints* FirstLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 20, 10);
  TGLayoutHints* LabelLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 0, 20);
  TGLayoutHints* TightButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 10, 0, 10);


  // File type frame

  m_Mode = new MGUIERBList(TypeFrame, "Please select an output file format:");
  m_Mode->Add("*.roa file to use with melinator / nulcearizer");
  m_Mode->Add("*.dat file containing all information for debugging");
  m_Mode->Add("*.evta file to use with revan");
  m_Mode->SetSelected(dynamic_cast<MModuleEventSaver*>(m_Module)->GetMode());
  m_Mode->Create();
  TypeFrame->AddFrame(m_Mode, FirstLabelLayout);


  m_FileSelector = new MGUIEFileSelector(TypeFrame, "Please select an output file:",
                                         dynamic_cast<MModuleEventSaver*>(m_Module)->GetFileName());
  m_FileSelector->SetFileType("roa file (read-out assemlies)", "*.roa");
  m_FileSelector->SetFileType("dat file (all info)", "*.dat");
  m_FileSelector->SetFileType("evta file (evta file)", "*.evta");
  TypeFrame->AddFrame(m_FileSelector, LabelLayout);


  // General frame

  m_SaveBadEvents = new TGCheckButton(GeneralFrame, "Save events which are flagged bad (BD)", 1);
  m_SaveBadEvents->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetSaveBadEvents());
  GeneralFrame->AddFrame(m_SaveBadEvents, FirstLabelLayout);

  m_SaveVetoEvents = new TGCheckButton(GeneralFrame, "Save guard ring and shield veto events (Veto)", 1);
  m_SaveVetoEvents->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetSaveVetoEvents());
  GeneralFrame->AddFrame(m_SaveVetoEvents, TightButtonLayout);

  m_AddTimeTag = new TGCheckButton(GeneralFrame, "Add a unique time tag", 3);
  m_AddTimeTag->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetAddTimeTag());
  GeneralFrame->AddFrame(m_AddTimeTag, TightButtonLayout);


  m_SplitFile = new TGCheckButton(GeneralFrame, "Split the file", 2);
  m_SplitFile->Associate(this);
  m_SplitFile->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetSplitFile());
  GeneralFrame->AddFrame(m_SplitFile, TightButtonLayout);

  TGLayoutHints* SplitFileTimeLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 30, 10, 0, 10);
  m_SplitFileTime = new MGUIEEntry(GeneralFrame, "Split the file after this time [sec]:", false,
                                   dynamic_cast<MModuleEventSaver*>(m_Module)->GetSplitFileTime().GetAsSystemSeconds(), true, 0l);
  if (m_SplitFile->IsOn() == false) {
    m_SplitFileTime->SetEnabled(false);
  }
  GeneralFrame->AddFrame(m_SplitFileTime, SplitFileTimeLayout);


  // Roa frame

  TGLabel* ROAOptionsLabel = new TGLabel(RoaFrame, "Special options for roa files:");
  RoaFrame->AddFrame(ROAOptionsLabel, FirstLabelLayout);

  m_RoaWithADCs = new TGCheckButton(RoaFrame, "Include ADCs", 3);
  m_RoaWithADCs->Associate(this);
  m_RoaWithADCs->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetRoaWithADCs());
  RoaFrame->AddFrame(m_RoaWithADCs, TightButtonLayout);

  m_RoaWithTACs = new TGCheckButton(RoaFrame, "Include TACs", 3);
  m_RoaWithTACs->Associate(this);
  m_RoaWithTACs->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetRoaWithTACs());
  RoaFrame->AddFrame(m_RoaWithTACs, TightButtonLayout);

  m_RoaWithEnergies = new TGCheckButton(RoaFrame, "Include energies", 3);
  m_RoaWithEnergies->Associate(this);
  m_RoaWithEnergies->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetRoaWithEnergies());
  RoaFrame->AddFrame(m_RoaWithEnergies, TightButtonLayout);

  m_RoaWithTimings = new TGCheckButton(RoaFrame, "Include timings", 3);
  m_RoaWithTimings->Associate(this);
  m_RoaWithTimings->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetRoaWithTimings());
  RoaFrame->AddFrame(m_RoaWithTimings, TightButtonLayout);

  m_RoaWithTemperatures = new TGCheckButton(RoaFrame, "Include temperatures", 3);
  m_RoaWithTemperatures->Associate(this);
  m_RoaWithTemperatures->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetRoaWithTemperatures());
  RoaFrame->AddFrame(m_RoaWithTemperatures, TightButtonLayout);

  m_RoaWithFlags = new TGCheckButton(RoaFrame, "Include flags", 3);
  m_RoaWithFlags->Associate(this);
  m_RoaWithFlags->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetRoaWithFlags());
  RoaFrame->AddFrame(m_RoaWithFlags, TightButtonLayout);

  m_RoaWithOrigins = new TGCheckButton(RoaFrame, "Include origins", 3);
  m_RoaWithOrigins->Associate(this);
  m_RoaWithOrigins->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetRoaWithOrigins());
  RoaFrame->AddFrame(m_RoaWithOrigins, TightButtonLayout);

  m_RoaWithNearestNeighbors = new TGCheckButton(RoaFrame, "Include nearest neighbor Hits", 3);
  m_RoaWithNearestNeighbors->Associate(this);
  m_RoaWithNearestNeighbors->SetOn(dynamic_cast<MModuleEventSaver*>(m_Module)->GetRoaWithNearestNeighbors());
  RoaFrame->AddFrame(m_RoaWithNearestNeighbors, TightButtonLayout);


  PostCreate();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIOptionsEventSaver::ProcessMessage(long Message, long Parameter1, long Parameter2)
{
  // Modify here if you have more buttons

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      break;
    case kCM_CHECKBUTTON:
      if (Parameter1 == 2) {
        m_SplitFileTime->SetEnabled(m_SplitFile->IsOn());
      }
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


bool MGUIOptionsEventSaver::OnApply()
{
  // Modify this to store the data in the module!

  dynamic_cast<MModuleEventSaver*>(m_Module)->SetMode(m_Mode->GetSelected());

  dynamic_cast<MModuleEventSaver*>(m_Module)->SetFileName(m_FileSelector->GetFileName());

  dynamic_cast<MModuleEventSaver*>(m_Module)->SetSaveBadEvents(m_SaveBadEvents->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetSaveVetoEvents(m_SaveVetoEvents->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetAddTimeTag(m_AddTimeTag->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetSplitFile(m_SplitFile->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetSplitFileTime(MTime(m_SplitFileTime->GetAsInt()));

  dynamic_cast<MModuleEventSaver*>(m_Module)->SetRoaWithADCs(m_RoaWithADCs->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetRoaWithTACs(m_RoaWithTACs->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetRoaWithEnergies(m_RoaWithEnergies->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetRoaWithTimings(m_RoaWithTimings->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetRoaWithTemperatures(m_RoaWithTemperatures->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetRoaWithFlags(m_RoaWithFlags->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetRoaWithOrigins(m_RoaWithOrigins->IsOn());
  dynamic_cast<MModuleEventSaver*>(m_Module)->SetRoaWithNearestNeighbors(m_RoaWithNearestNeighbors->IsOn());

  return true;
}


// MGUIOptionsEventSaver: the end...
////////////////////////////////////////////////////////////////////////////////
