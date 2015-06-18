/*
 * MGUIExpoAspectViewer.cxx
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
#include "MGUIExpoAspectViewer.h"

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
ClassImp(MGUIExpoAspectViewer)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIExpoAspectViewer::MGUIExpoAspectViewer(MModule* Module) : MGUIExpo(Module)
{
  // standard constructor

  // Set the new title of the tab here:
  m_TabTitle = "Heading";

  // Add all histograms and canvases below
  m_Heading = new TGraph();
  m_HeadingCanvas = 0;

  m_TimeCutOff = 60;
  
  // use hierarchical cleaning
  SetCleanup(kDeepCleanup);
}


////////////////////////////////////////////////////////////////////////////////


MGUIExpoAspectViewer::~MGUIExpoAspectViewer()
{
  // kDeepCleanup is activated 
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoAspectViewer::Reset()
{
  //! Reset the data in the UI

  m_Mutex.Lock();

  m_Heading->Set(0);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoAspectViewer::AddHeading(MTime Time, double Heading)
{
  // Add data to the heading histogram

  m_Mutex.Lock();

  // Ignore the data point if
  // (a) It is within 1 second of the old one AND
  // (b) the heading chaneg is the same as before...
  
  if (Time.GetAsSeconds() < m_Times.back().GetAsSeconds() + 1 && fabs(Heading - m_Headings.back()) < 0.1) {
    m_Mutex.UnLock();
    return;
  }
  
  m_Times.push_back(Time);
  m_Headings.push_back(Heading);
  
  while (m_Times.back().GetAsSeconds() > m_Times.front().GetAsSeconds() + m_TimeCutOff) {
    m_Times.pop_front();
    m_Headings.pop_front();
  }
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoAspectViewer::Export(const MString& FileName)
{
  // Add data to the energy histogram

  m_Mutex.Lock();

  m_HeadingCanvas->GetCanvas()->SaveAs(FileName);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoAspectViewer::Create()
{
  // Add the GUI options here

  // Do not create it twice!
  if (m_IsCreated == true) return;

  m_Mutex.Lock();
  
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,
                                                  2, 2, 2, 2);

  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, CanvasLayout);

  
  m_HeadingCanvas = new TRootEmbeddedCanvas("Heading", HFrame, 100, 100);
  HFrame->AddFrame(m_HeadingCanvas, CanvasLayout);

  m_HeadingCanvas->GetCanvas()->cd();
  m_HeadingCanvas->GetCanvas()->SetGridy();
  m_HeadingCanvas->GetCanvas()->SetGridx();
  m_Heading->Draw("AC*");
  m_HeadingCanvas->GetCanvas()->Update();
  
  m_IsCreated = true;
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoAspectViewer::Update()
{
  //! Update the frame

  m_Mutex.Lock();

  m_Heading->Set(m_Times.size());
  MTime Last = m_Times.back(); 
  
  int Counter = 0;
  auto T = m_Times.begin();
  auto H = m_Headings.begin();
  while (T != m_Times.end() && H != m_Headings.end()) {
    m_Heading->SetPoint(Counter, (*T).GetAsSeconds() - Last.GetAsSeconds(), (*H));
    ++Counter;
    ++H;
    ++T;
  }

  if (m_HeadingCanvas != 0) {
    m_HeadingCanvas->GetCanvas()->Modified();
    m_HeadingCanvas->GetCanvas()->Update();
  }
  
  m_Mutex.UnLock();
}


// MGUIExpoAspectViewer: the end...
////////////////////////////////////////////////////////////////////////////////
