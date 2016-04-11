/*
 * MGUIExpoReceiver.cxx
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
#include "MGUIExpoReceiver.h"

// Standard libs:

// ROOT libs:
#include <TSystem.h>
#include <TString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TCanvas.h>

// MEGAlib libs:
#include "MStreams.h"



////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIExpoReceiver)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoReceiver::MGUIExpoReceiver(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "Receiver";
  
  m_TimeReceived.Set(0);
  m_BytesReceived = 0;
  m_RawFramesParsed = 0;
  m_ComptonFramesParsed = 0;
  m_AspectFramesParsed = 0;
  m_OtherFramesParsed = 0;
  
  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoReceiver::~MGUIExpoReceiver()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::Reset()
{
  //! Reset the data in the UI

  m_Mutex.Lock();
  m_TimeReceived.Set(0);
  m_BytesReceived = 0;
  m_RawFramesParsed = 0;
  m_ComptonFramesParsed = 0;
  m_AspectFramesParsed = 0;
  m_OtherFramesParsed = 0;
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::SetBytesReceived(long Bytes) 
{ 
  //! Set the amount of data received
  
  m_Mutex.Lock();
  m_BytesReceived = Bytes; 
  m_Mutex.UnLock();  
}
  

////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::SetTimeReceived(MTime Time) 
{ 
  // Set the time the last event was received
  
  m_Mutex.Lock();
  m_TimeReceived = Time; 
  m_Mutex.UnLock();   
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::SetRawFramesParsed(long Frames)
{
  //! Set the number of raw data frames received

  m_Mutex.Lock();
  m_RawFramesParsed = Frames;
  m_Mutex.UnLock();   
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::SetComptonFramesParsed(long Frames)
{
  //! Set the number of Compton data frames received

  m_Mutex.Lock();
  m_ComptonFramesParsed = Frames;
  m_Mutex.UnLock();   
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::SetAspectFramesParsed(long Frames)
{
  //! Set the number of aspect data frames received

  m_Mutex.Lock();
  m_AspectFramesParsed = Frames;
  m_Mutex.UnLock();   
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::SetOtherFramesParsed(long Frames)
{
  //! Set thenumber of other data frames received

  m_Mutex.Lock();
  m_OtherFramesParsed = Frames;
  m_Mutex.UnLock();   
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::Create()
{
  // Add the GUI options here

  // Do not create it twice!
  if (m_IsCreated == true) return;
  
  m_Mutex.Lock();
  
  TGLayoutHints* ModuleFrameLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 40, 40, 30, 30);

  TGHorizontalFrame* ModuleFrame = new TGHorizontalFrame(this);
  //ModuleFrame->ChangeOptions(kSunkenFrame);
  AddFrame(ModuleFrame, ModuleFrameLayout);


  ModuleFrame->SetLayoutManager(new TGMatrixLayout(ModuleFrame, 6, 2, 5));
  
  TGLabel* TimeTextLabel = new TGLabel(ModuleFrame, "Time last data was received: ");
  ModuleFrame->AddFrame(TimeTextLabel);
  m_TimeLabel = new TGLabel(ModuleFrame, "1970-01-01 00:00:00:0000000000");
  //m_TimeLabel->ChangeOptions(kSunkenFrame);
  ModuleFrame->AddFrame(m_TimeLabel);

  TGLabel* BytesReceivedTextLabel = new TGLabel(ModuleFrame, "Bytes received: ");
  ModuleFrame->AddFrame(BytesReceivedTextLabel);
  m_BytesReceivedLabel = new TGLabel(ModuleFrame, "                              0                              ");
  //m_BytesReceivedLabel->ChangeOptions(kSunkenFrame);
  ModuleFrame->AddFrame(m_BytesReceivedLabel);

  TGLabel* RawFramesTextLabel = new TGLabel(ModuleFrame, "Raw frames parsed: ");
  ModuleFrame->AddFrame(RawFramesTextLabel);
  m_RawFramesLabel = new TGLabel(ModuleFrame, "                              0                              ");
  //m_RawFramesLabel->ChangeOptions(kSunkenFrame);
  ModuleFrame->AddFrame(m_RawFramesLabel);

  TGLabel* ComptonFramesTextLabel = new TGLabel(ModuleFrame, "Compton frames parsed: ");
  ModuleFrame->AddFrame(ComptonFramesTextLabel);
  m_ComptonFramesLabel = new TGLabel(ModuleFrame, "                              0                              ");
  //m_ComptonFramesLabel->ChangeOptions(kSunkenFrame);
  ModuleFrame->AddFrame(m_ComptonFramesLabel);

  TGLabel* AspectFramesTextLabel = new TGLabel(ModuleFrame, "Aspect frames parsed: ");
  ModuleFrame->AddFrame(AspectFramesTextLabel);
  m_AspectFramesLabel = new TGLabel(ModuleFrame, "                              0                              ");
  //m_AspectFramesLabel->ChangeOptions(kSunkenFrame);
  ModuleFrame->AddFrame(m_AspectFramesLabel);

  TGLabel* OtherFramesTextLabel = new TGLabel(ModuleFrame, "Other frames parsed: ");
  ModuleFrame->AddFrame(OtherFramesTextLabel);
  m_OtherFramesLabel = new TGLabel(ModuleFrame, "                              0                              ");
  //m_OtherFramesLabel->ChangeOptions(kSunkenFrame);
  ModuleFrame->AddFrame(m_OtherFramesLabel);
  
  
  m_IsCreated = true;

  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::Update()
{
  //! Update the frame

  m_Mutex.Lock();

  if (m_IsCreated == true) {
    m_TimeLabel->SetText(m_TimeReceived.GetUTCString());

    MString Text;
    Text += m_BytesReceived;
    m_BytesReceivedLabel->SetText(Text);

    Text.Clear();
    Text += m_RawFramesParsed;
    m_RawFramesLabel->SetText(Text);

    Text.Clear();
    Text += m_ComptonFramesParsed;
    m_ComptonFramesLabel->SetText(Text);

    Text.Clear();
    Text += m_AspectFramesParsed;
    m_AspectFramesLabel->SetText(Text);

    Text.Clear();
    Text += m_OtherFramesParsed;
    m_OtherFramesLabel->SetText(Text);    
  }
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoReceiver::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  m_Mutex.UnLock();
}


// MGUIExpoReceiver: the end...
////////////////////////////////////////////////////////////////////////////////
