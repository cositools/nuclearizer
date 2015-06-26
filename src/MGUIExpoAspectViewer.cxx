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
#include <algorithm>
using namespace std;

// ROOT libs:
#include <TSystem.h>
#include <TString.h>
#include <TGLabel.h>
#include <TGResourcePool.h>
#include <TCanvas.h>
#include <TAxis.h>

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
  m_Heading_GPS = new TGraph();
  m_Heading_GPS->SetLineColor(kBlue);
  m_Heading_Mag = new TGraph();  
  m_Heading_Mag->SetLineColor(kRed);
  
  m_HeadingCanvas = 0;
  m_Label = 0;

  m_TimeCutOff = 3600; // seconds
  
  m_Update = 2;
 
  
  
  
  
  
  
  
  
  
  
  
  
  /*
  
  Label = new TGLabel(this, "Normal Label", fTextGC->GetGC(),
                       labelfont); 
  //AddFrame(Label,  new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  Label->SetTextColor(kGreen);
  
  */
  
  
  
  
  
  
  
  
  
  
  
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

  m_Heading_GPS->Set(0);
  m_Heading_Mag->Set(0);
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoAspectViewer::AddHeading(MTime Time, double Heading, int GPS_or_magnetometer, double BRMS, uint16_t AttFlag)
{
  // Add data to the heading histogram

  m_Mutex.Lock();

  // Ignore the data point if
  // (a) It is within 1 second of the old one AND
  // (b) the heading change is smaller than 0.1 degree
  
  ostringstream LabelStream;
  LabelStream<<"Heading: "<<Heading<<"   BRMS: "<<BRMS<<"   AttFlag: "<<AttFlag;
  
  /*
  string Word_Heading = "Heading: ";
  
  ostringstream StringStream_Heading;
  StringStream_Heading << Heading;
  string String_Heading = StringStream_Heading.str();
  
  string Word_BRMS = "  BRMS: ";  
  
  ostringstream StringStream_BRMS;
  StringStream_BRMS << BRMS;
  string String_BRMS = StringStream_BRMS.str();
  
  string Word_AttFlag = "  AttFlag: ";  
  
  ostringstream StringStream_AttFlag;
  StringStream_AttFlag << AttFlag;
  string String_AttFlag = StringStream_AttFlag.str();
  
  
  string String_Label = Word_Heading + String_Heading + Word_BRMS + String_BRMS + Word_AttFlag + String_AttFlag;
  //const char * ConstCharStar_Label = String_Label.c_str();
  */
  
  
  ////////////////////////
  
  
  if (m_Times_GPS.size() > 0 && Time.GetAsSeconds() < m_Times_GPS.back().GetAsSeconds() + 1 && fabs(Heading - m_Headings_GPS.back()) < 0.1 && GPS_or_magnetometer==0) {
  	//cout<<"Rejecting: "<<m_Times.size()<<endl;
    m_Mutex.UnLock();
    return;
  }
  //cout<<"Accepting"<<endl;


  ////////////////////////  
  
  if (m_Times_Mag.size() > 0 && Time.GetAsSeconds() < m_Times_Mag.back().GetAsSeconds() + 1 && fabs(Heading - m_Headings_Mag.back()) < 0.1 && GPS_or_magnetometer==1) {
  	//cout<<"Rejecting: "<<m_Times.size()<<endl;
    m_Mutex.UnLock();
    return;
  }
  //cout<<"Accepting"<<endl;
  
  
  ////////////////////////
  int a = 0;
  int b = 5;
  
  if (GPS_or_magnetometer==0){
  	m_Times_GPS.push_back(Time);
  	m_Headings_GPS.push_back(Heading);
  	m_Labels.push_back(LabelStream.str());
  	if(m_Update<5){
      //cout<<"Preparing to change text"<<endl;
  	  m_Update=m_Update+1;
    }
    /*if(m_Update==1){
      //cout<<"Preparing to change text"<<endl;
      //m_Label->ChangeText(LabelStream.str().c_str());
  	  m_Update=2;
    }*/
    else if(m_Update % b == a) {
      m_Label->SetText(LabelStream.str().c_str());
      m_Update = m_Update +1;
    }
    else{
    	m_Update = m_Update +1;
    }
    

    //else{
    //  cout<<"Preparing to change despite not being update time"<<endl;
    //  m_Label->ChangeText(LabelStream.str().c_str());
    //}
  	//cout<<"GPS: "<<Heading<<endl;
  }
  
  if (GPS_or_magnetometer==1){
  	m_Times_Mag.push_back(Time);
  	m_Headings_Mag.push_back(Heading);
  	m_Labels.push_back(LabelStream.str());
  	//m_Label->ChangeText(ConstCharStar_Label);
  	//cout<<"Mag:"<<Heading<<endl;
  }

  
  while (m_Times_GPS.back().GetAsSeconds() > m_Times_GPS.front().GetAsSeconds() + m_TimeCutOff) {
    m_Times_GPS.pop_front();
    m_Headings_GPS.pop_front();
    m_Labels.pop_front();
  }
  
  while (m_Times_Mag.back().GetAsSeconds() > m_Times_Mag.front().GetAsSeconds() + m_TimeCutOff) {
    m_Times_Mag.pop_front();
    m_Headings_Mag.pop_front();
    m_Labels.pop_front();
  }
  
  //cout<<"Size: "<<m_Times.size()<<endl;
  
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
  
  
  
  // label + horizontal line
  TGGC *fTextGC;
  const TGFont *font = gClient->GetFont("-*-times-bold-r-*-*-18-*-*-*-*-*-*-*");
  if (!font){
    font = gClient->GetResourcePool()->GetDefaultFont();
  }
  //FontStruct_t labelfont = font->GetFontStruct();
  GCValues_t   gval;
  gval.fMask = kGCBackground | kGCFont | kGCForeground;
  gval.fFont = font->GetFontHandle();
  gClient->GetColorByName("yellow", gval.fBackground);
  fTextGC = gClient->GetGC(&gval, kTRUE);


  ULong_t bcolor, ycolor;
  gClient->GetColorByName("yellow", ycolor);
  gClient->GetColorByName("blue", bcolor);
  
  
  
  
  m_Label = new TGLabel(this, "First Label") ; //, fTextGC->GetGC(), labelfont);
  AddFrame(m_Label,  new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  //m_Label->SetTextColor(ycolor);
  //m_Label->ChangeText("New Label");
  
  
  
  
  TGLayoutHints* CanvasLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX | kLHintsExpandY,
                                                  2, 2, 2, 2);

  TGHorizontalFrame* HFrame = new TGHorizontalFrame(this);
  AddFrame(HFrame, CanvasLayout);

  
  m_HeadingCanvas = new TRootEmbeddedCanvas("Heading", HFrame, 100, 100);
  HFrame->AddFrame(m_HeadingCanvas, CanvasLayout);

  m_HeadingCanvas->GetCanvas()->cd();
  m_HeadingCanvas->GetCanvas()->SetGridy();
  m_HeadingCanvas->GetCanvas()->SetGridx();
  m_Heading_GPS->SetTitle("Heading");
  m_Heading_GPS->Draw("AL");
  //m_Heading_Mag->Draw("L");
  
  
  //int_t DummyInt = 0;
  double_t DummyDouble = 0.0;
  m_Heading_GPS->SetPoint(0, DummyDouble,DummyDouble);
  m_Heading_Mag->SetPoint(0, DummyDouble, DummyDouble);
  
  m_HeadingCanvas->GetCanvas()->Update();
  
  m_IsCreated = true;
  
  m_Mutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


void MGUIExpoAspectViewer::Update()
{
  //! Update the frame

  cout<<"Now inside update function"<<endl;



  if (m_IsCreated == false){
  	//fNeedRedraw = true;
    cout<<"About to return"<<endl;  	
  	return;
  	cout<<"Return failed"<<endl;  
  }
  

  
  cout<<"m_Labels size: "<<m_Labels.size()<<endl;  
  
  cout<<"Now past if statement"<<endl;
  
  m_Mutex.Lock();
  m_Update=0;

/*

  //const char * Last_Label = m_Labels.back();
  if (m_Labels.size() > 0) {
  	fNeedRedraw = true;
  	m_Label->SetText(m_Labels.back().c_str());
  	m_Update=0;
  	
  	fNeedRedraw = true;
  	cout<<"Label text: "<<m_Labels.back().c_str()<<endl;
  }
  else{
  	m_Label->SetText("Second Label");
  	cout<<"Label text: "<<m_Labels.back().c_str()<<endl;
  }

  cout<<"Finished printing label"<<endl; */

  double Max = 0;
  double Min = 360;

  m_Heading_GPS->Set(m_Times_GPS.size());
  m_Heading_Mag->Set(m_Times_Mag.size());
  MTime Last_GPS = m_Times_GPS.back(); 
  MTime Last_Mag = m_Times_Mag.back(); 
  
  int Counter_GPS = 1;
  auto T_GPS = m_Times_GPS.begin();
  auto H_GPS = m_Headings_GPS.begin();
  while (T_GPS != m_Times_GPS.end() && H_GPS != m_Headings_GPS.end()) {
    m_Heading_GPS->SetPoint(Counter_GPS, (*T_GPS).GetAsSeconds() - Last_GPS.GetAsSeconds(), (*H_GPS));
    if ((*H_GPS) > Max) Max = (*H_GPS);
    if ((*H_GPS) < Min) Min = (*H_GPS);
    ++Counter_GPS;
    ++H_GPS;
    ++T_GPS;
  }
  
  
  int Counter_Mag = 1;
  auto T_Mag = m_Times_Mag.begin();
  auto H_Mag = m_Headings_Mag.begin();
  while (T_Mag != m_Times_Mag.end() && H_Mag != m_Headings_Mag.end()) {
    m_Heading_Mag->SetPoint(Counter_Mag, (*T_Mag).GetAsSeconds() - Last_Mag.GetAsSeconds(), (*H_Mag));
    if ((*H_Mag) > Max) Max = (*H_Mag);
    if ((*H_Mag) < Min) Min = (*H_Mag);
    ++Counter_Mag;
    ++H_Mag;
    ++T_Mag;
  }
  

  if (m_HeadingCanvas != 0) {
    if (m_Heading_GPS->GetHistogram() != nullptr && MString(m_Heading_GPS->GetHistogram()->GetXaxis()->GetTitle()).IsEmpty() == true) {
      m_Heading_GPS->GetHistogram()->GetXaxis()->SetTitle("Time [seconds]");
      m_Heading_GPS->GetHistogram()->GetYaxis()->SetTitle("Heading [degrees] (Blue=GPS, Red=Mag)");
    }
    m_Heading_GPS->SetMaximum(Max);
    m_Heading_GPS->SetMinimum(Min);
    
    m_HeadingCanvas->GetCanvas()->Modified();
    m_HeadingCanvas->GetCanvas()->Update();
  }
  
  m_Mutex.UnLock();
}


// MGUIExpoAspectViewer: the end...
////////////////////////////////////////////////////////////////////////////////
