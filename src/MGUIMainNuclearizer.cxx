/*
* MGUINuclearizerMain.cxx
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
#include "MGUIMainNuclearizer.h"

// Standard libs:

// ROOT libs:
#include <KeySymbols.h>
#include <TApplication.h>
#include <TGPicture.h>
#include <TStyle.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGWindow.h>
#include <TGFrame.h>
#include <TGClient.h>
#include <TGResourcePool.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MGUIDefaults.h"
#include "MGUIAbout.h"
#include "MGUIGeometry.h"
#include "MGUIModuleSelector.h"
#include "MGUIEFileSelector.h"

// Nuclearizer libs:
#include "MNCTModule.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUINuclearizerMain)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUINuclearizerMain::MGUINuclearizerMain(MInterfaceNuclearizer* Interface,
                                        MNCTData* Data)
  : TGMainFrame(gClient->GetRoot(), 350, 300, kVerticalFrame),
    m_Interface(Interface), m_Data(Data)
{
  gStyle->SetPalette(1, 0);

  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);

  BindKey(this, gVirtualX->KeysymToKeycode(kKey_L), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_S), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_E), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Return), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Enter), kAnyModifier);
  BindKey(this, gVirtualX->KeysymToKeycode(kKey_Escape), kAnyModifier);

  Create();
}


////////////////////////////////////////////////////////////////////////////////


MGUINuclearizerMain::~MGUINuclearizerMain()
{
  // Deep Cleanup automatically deletes all used GUI elements
}


////////////////////////////////////////////////////////////////////////////////


void MGUINuclearizerMain::Create()
{
  // Create the main window

  // We start with a name and an icon...
  SetWindowName("Nuclearizer");  

  double FontScaler = MGUIDefaults::GetInstance()->GetFontScaler();

  // In the beginning we build the menus and define their layout, ... 
  TGLayoutHints* MenuBarItemLayoutLeft = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  //TGLayoutHints* MenuBarItemLayoutRight = new TGLayoutHints(kLHintsTop | kLHintsRight, 0, 0, 0, 0);
  
  // We continue with the menu bar and its layout ...
  TGLayoutHints* MenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 0, 0, 0, 0);
  
  TGMenuBar* MenuBar = new TGMenuBar(this, 1, 1, kHorizontalFrame | kRaisedFrame);
  AddFrame(MenuBar, MenuBarLayout);

  TGPopupMenu* MenuOptions = new TGPopupMenu(gClient->GetRoot());
  MenuOptions->AddLabel("Configuration file");
  MenuOptions->AddEntry("Open", c_LoadConfig);
  MenuOptions->AddEntry("Save As", c_SaveConfig);
  MenuOptions->AddSeparator();
  MString Geo = MString("Geometry file");
  if (m_Data->GetGeometry() != 0) {
    Geo += " (current: ";
    Geo += m_Data->GetGeometry()->GetName(); 
    Geo += ")";
  }
  MenuOptions->AddLabel(Geo);
  MenuOptions->AddEntry("Open", c_Geometry);
  MenuOptions->AddSeparator();
  MenuOptions->AddEntry("Exit", c_Exit);
  MenuOptions->Associate(this);
  MenuBar->AddPopup("Options", MenuOptions, MenuBarItemLayoutLeft);

  TGPopupMenu* MenuInfo = new TGPopupMenu(fClient->GetRoot());
  MenuInfo->AddEntry("About", c_About);
  MenuInfo->Associate(this);
  MenuBar->AddPopup("Info", MenuInfo, MenuBarItemLayoutLeft);


  // Main label
  MString TitleIconName("$(NUCLEARIZER)/resource/Nuclearizer.xpm");
  MFile::ExpandFileName(TitleIconName);
  
  TGLayoutHints* TitleIconLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsCenterX, 2, 2, 10, 0);
  if (MFile::Exists(TitleIconName) == true) {
    const TGPicture* TitlePicture = fClient->GetPicture(TitleIconName, FontScaler*300, FontScaler*300/5);
    if (TitlePicture == 0) {
      mgui<<"Can't find picture "<<TitleIconName<<"! Aborting!"<<error;
      return;
    }
    TGIcon* TitleIcon = new TGIcon(this, TitlePicture, TitlePicture->GetWidth()+2, TitlePicture->GetHeight()+2);
    AddFrame(TitleIcon, TitleIconLayout);
  } else {
    const TGFont* lFont = gClient->GetFont("-*-helvetica-bold-r-*-*-24-*-*-*-*-*-iso8859-1");
    if (!lFont) lFont = gClient->GetResourcePool()->GetDefaultFont();
    FontStruct_t LargeFont = lFont->GetFontStruct();

    TGLabel* MainLabel = new TGLabel(this, "The Nuclearizer");
    MainLabel->SetTextFont(LargeFont);
    TGLayoutHints* MainLabelLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 30, 0);
    AddFrame(MainLabel, MainLabelLayout);
  }


  // Sub-title
  FontStruct_t ItalicFont = MGUIDefaults::GetInstance()->GetItalicMediumFont()->GetFontStruct();

  TGLabel* SubTitle = new TGLabel(this, "The NCT & GRIPS measurement and simulation calibrator");
  SubTitle->SetTextFont(ItalicFont);
  TGLayoutHints* SubTitleLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 0, FontScaler*12);
  AddFrame(SubTitle, SubTitleLayout);

  
  
  
  
  TGLayoutHints* SectionLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 10, 10, 0, 10);
  TGLayoutHints* FileSelectorLayout = new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX, 0, 0, 5, 5);

  /*
  // Section 1: File loading
  TGGroupFrame* Loading = new TGGroupFrame(this, "Section: File loading");
  AddFrame(Loading, SectionLayout);

  m_FileSelectorLoad = new MGUIEFileSelector(Loading, "Select a simulation or data file:", 
                                            m_Data->GetLoadFileName());
  m_FileSelectorLoad->SetFileType("Simulation file", "*.sim");
  m_FileSelectorLoad->SetFileType("Real data file", "*.dat");
  Loading->AddFrame(m_FileSelectorLoad, FileSelectorLayout);

  m_FileSelectorGeometry = new MGUIEFileSelector(Loading, "Select a geometry file:", 
                                                m_Data->GetGeometryFileName());
  m_FileSelectorGeometry->SetFileType("Geometry file", "*.geo.setup");
  Loading->AddFrame(m_FileSelectorGeometry, FileSelectorLayout);
  */

  // Section 2: Modules
  m_ModuleFrame = new TGGroupFrame(this, "Choose the module sequence for your detector setup");
  AddFrame(m_ModuleFrame, SectionLayout);

  m_ModuleLayout = new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 0, 0, 3, 3);
  UpdateModules();

  /*
  // Section 3: File saving
  TGGroupFrame* Saving = new TGGroupFrame(this, "Section: File saving");
  AddFrame(Saving, SectionLayout);

  m_FileSelectorSave = new MGUIEFileSelector(Saving, "", 
                                            m_Data->GetSaveFileName());
  m_FileSelectorSave->SetFileType("Events file", "*.evta");
  Saving->AddFrame(m_FileSelectorSave, FileSelectorLayout);
  */

  // Start & Exit buttons
  // Frame around the buttons:
  TGHorizontalFrame* ButtonFrame = new TGHorizontalFrame(this, 150, 25);
  TGLayoutHints* ButtonFrameLayout =	new TGLayoutHints(kLHintsBottom | kLHintsExpandX | kLHintsCenterX, 10, 10, 10, 10);
  AddFrame(ButtonFrame, ButtonFrameLayout);
  
  // The buttons itself
  TGTextButton*	StartButton = new TGTextButton(ButtonFrame, "Start", c_Start); 
  StartButton->Associate(this);
  TGLayoutHints* StartButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX, 40, 40, 0, 0);
  ButtonFrame->AddFrame(StartButton, StartButtonLayout);
  
  /*
  TGTextButton* ExitButton = new TGTextButton(ButtonFrame, "     Exit     ", c_Exit); 
  ExitButton->Associate(this);
  TGLayoutHints* ExitButtonLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 0, 0);
  ButtonFrame->AddFrame(ExitButton, ExitButtonLayout);
  */
  
  // Give this element the default size of its content:
  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  MapSubwindows();
  MapWindow();  
  Layout();

  return;
}


////////////////////////////////////////////////////////////////////////////////


void MGUINuclearizerMain::UpdateModules()
{
  // Remove all existing modules:
  for (unsigned int m = m_Modules.size()-1; m < m_Modules.size(); --m) {
    m_ModuleFrame->RemoveFrame(m_Modules[m]);
    m_Modules[m]->UnmapWindow();
    delete m_Modules[m];
  }
  m_Modules.clear();

  for (unsigned int m = 0; m < m_Data->GetNModules(); ++m) {
    MGUIEModule* GuiModule = new MGUIEModule(m_ModuleFrame, m, m_Data->GetModule(m));
    GuiModule->Associate(this);
    m_ModuleFrame->AddFrame(GuiModule, m_ModuleLayout);
    m_Modules.push_back(GuiModule);
  }  

  if (m_Data->GetNModules() == 0 || m_Data->GetModule(m_Data->GetNModules()-1)->GetNSucceedingModuleTypes() > 0) {
    MGUIEModule* GuiModule = new MGUIEModule(m_ModuleFrame, m_Data->GetNModules());
    GuiModule->Associate(this);
    m_ModuleFrame->AddFrame(GuiModule, m_ModuleLayout);
    m_Modules.push_back(GuiModule);
  }

  Resize(GetDefaultWidth(), GetDefaultHeight()); 

  MapSubwindows();
  MapWindow();  
  Layout();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::HandleKey(Event_t* Event)
{
  // Here we handle all keys...

  char   tmp[10];
  unsigned int keysym;

  // Test if we have a key release:
  if (Event->fType != kKeyRelease) return false;

  // First we get the key...
  gVirtualX->LookupString(Event, tmp, sizeof(tmp), keysym);
  
  // ... and than we do what we need to do...
  
  // The following keys need an initialized hardware
  switch ((EKeySym) keysym) {
  case kKey_Escape:
    OnExit();
    break;
  case kKey_Return:
  case kKey_Enter:
    OnStart();
    break;
  case kKey_l:
  case kKey_L:
    OnLoadConfiguration();
    break;
  case kKey_s:
  case kKey_S:
    OnSaveConfiguration();
    break;
  default:
    break;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::ProcessMessage(long Message, long Parameter1, 
                                        long Parameter2)
{
  // Process the messages for this application

  bool Status = true;

  switch (GET_MSG(Message)) {
  case kC_COMMAND:
    switch (GET_SUBMSG(Message)) {
    case kCM_BUTTON:
      if (Parameter1 >= c_Change && Parameter1 <= c_Change+99) {
        OnChange(Parameter1-c_Change);
      } else if (Parameter1 >= c_Remove && Parameter1 <= c_Remove+99) {
        OnRemove(Parameter1-c_Remove);
      } else if (Parameter1 >= c_Options && Parameter1 <= c_Options+99) {
        OnOptions(Parameter1-c_Options);
      }
      switch (Parameter1) {
      case c_Exit:
        Status = OnExit();
        break;

      case c_Start:
        Status = OnStart();
        break;

      default:
        break;
      }
    case kCM_MENU:
      switch (Parameter1) {

      case c_LoadConfig:
        Status = OnLoadConfiguration();
        break;

      case c_SaveConfig:
        Status = OnSaveConfiguration();
        break;

      case c_Geometry:
        Status = OnGeometry();
        break;

      case c_Exit:
        Status = OnExit();
        break;

      case c_About:
        Status = OnAbout();
        break;

      default:
        break;
      }
    default:
      break;
    }
  default:
    break;
  }
  
  return Status;
}


////////////////////////////////////////////////////////////////////////////////


void MGUINuclearizerMain::CloseWindow()
{
  // Call exit for controlled good-bye

  OnExit();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnChange(unsigned int ModuleID)
{
  MGUIModuleSelector* S = new MGUIModuleSelector(m_Data, ModuleID);
  gClient->WaitForUnmap(S);
  
  UpdateModules();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnRemove(unsigned int ModuleID)
{
  m_Data->RemoveModule(ModuleID);
  
  UpdateModules();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnOptions(unsigned int ModuleID)
{
  m_Data->GetModule(ModuleID)->ShowOptionsGUI();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnExit()
{
  OnApply();

  m_Interface->Exit();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnStart()
{
  if (OnApply() == false) return false;

  m_Interface->Analyze();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnApply()
{
  /*
  if (MFile::Exists(m_FileSelectorLoad->GetFileName()) == false) {
    mgui<<"The data file \""<<m_FileSelectorLoad->GetFileName()<<"\" does not exist."<<show;
    return false;
  }
  */
  /*
  if (MFile::Exists(m_FileSelectorGeometry->GetFileName()) == false) {
    mgui<<"The geometry file \""<<m_FileSelectorGeometry->GetFileName()<<"\" does not exist."<<show;
    return false;
  }
  */
  
  /*
  m_Data->SetLoadFileName(m_FileSelectorLoad->GetFileName());
  m_Data->SetSaveFileName(m_FileSelectorSave->GetFileName());
  m_Data->SetGeometryFileName(m_FileSelectorGeometry->GetFileName());
  */
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnLoadConfiguration()
{
  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;


  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &Info);
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_Data->Load(MString(Info.fFilename));

    m_FileSelectorLoad->SetFileName(m_Data->GetLoadFileName());
    m_FileSelectorSave->SetFileName(m_Data->GetSaveFileName());
    m_FileSelectorGeometry->SetFileName(m_Data->GetGeometryFileName());

    UpdateModules();
  } 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnSaveConfiguration()
{
  // Save a configuration file...

  if (OnApply() == false) return false;

  const char** Types = new const char*[4];
  Types[0] = "Configuration file";
  Types[1] = "*.cfg";
  Types[2] = 0;
  Types[3] = 0;
  

  TGFileInfo Info;
  Info.fFileTypes = (const char **) Types;
  //Info.fIniDir = StrDup(gSystem->DirName(m_Data->GetCurrentFile()));
  new TGFileDialog(gClient->GetRoot(), this, kFDSave, &Info);
  
  // Get the filename ...
  if ((char *) Info.fFilename != 0) {
    m_Data->Save(MString(Info.fFilename));
  } 

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnGeometry()
{
  // Show the geometry dialog
  // Returns the geometry file name

  MGUIGeometry* Geo = new MGUIGeometry(gClient->GetRoot(), this, m_Data->GetGeometryFileName());
  gClient->WaitForUnmap(Geo);
  MString Name = Geo->GetGeometryFileName();
  delete Geo;
  for (unsigned int i = 0; i < 100; ++i) {
    gSystem->ProcessEvents();
  }

  m_Data->SetGeometryFileName(Name);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUINuclearizerMain::OnAbout()
{
  // Launch the about dialog

  MGUIAbout* About = new MGUIAbout(gClient->GetRoot(), this);
  About->SetProgramName("Nuclearizer");
  //About->SetIconPath(g_MEGAlibPath + "/resource/icons/mimrec/Small.xpm");
  //About->SetLeadProgrammer("Andreas Zoglauer");
  About->SetCopyright("(C) by Andreas Zoglauer, Mark Bandstra,\nJau-Shian Liang, and Daniel Perez-Becker\nAll rights reserved");
  //About->SetReference("Implementation details of the imaging approach", 
  //                    "A. Zoglauer et al., \"Design, implementation, and optimization of MEGAlib's image reconstruction tool Mimrec \", NIM A 652, 2011");
  //About->SetReference("A detailed description of list-mode likelihood image reconstruction - in German", 
  //                    "A. Zoglauer, \"Methods of image reconstruction for the MEGA Compton telescope\", Diploma thesis, TU Munich, 2000");
  //About->SetReference("Chapter 5: List-mode image reconstruction applied to the MEGA telecope", 
  //                    "A. Zoglauer, \"First Light for the Next Generation of Compton and Pair Telescopes\", Doctoral thesis, TU Munich, 2005");
  About->Create();
  
  return true;
}


// MGUINuclearizerMain: the end...
////////////////////////////////////////////////////////////////////////////////
