/*
 * MGUIExpoReceiver.h
 *    
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIExpoReceiver__
#define __MGUIExpoReceiver__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGLabel.h>
#include <TString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MGUIExpo.h"

// Nuclearizer libs

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIExpoReceiver : public MGUIExpo
{
  // public Session:
 public:
  //! Default constructor
  MGUIExpoReceiver(MModule* Module);
  //! Default destructor
  virtual ~MGUIExpoReceiver();

  //! The creation part which gets overwritten
  virtual void Create();

  //! Update the frame
  virtual void Update();

  //! Reset the data in the UI
  virtual void Reset();

  //! Export the data in the UI
  virtual void Export(const MString& FileName);
  
  //! Set the time the last event was received
  void SetTimeReceived(MTime Time);

  //! Set the amount of data received
  void SetBytesReceived(long Bytes);

  //! Set the number of raw data frames parsed
  void SetRawFramesParsed(long Frames);

  //! Set the number of Compton data frames parsed
  void SetComptonFramesParsed(long Frames);

  //! Set the number of aspect data frames parsed
  void SetAspectFramesParsed(long Frames);

  //! Set thenumber of other data frames parsed
  void SetOtherFramesParsed(long Frames);


  // protected methods:
 protected:


  // protected members:
 protected:

  // private members:
 private:
  //! The time the last data was received
  MTime m_TimeReceived;
  //! The label showing the time received
  TGLabel* m_TimeLabel;
  
  //! The total amount of data received
  long m_BytesReceived;
  //! The label showing the amount received
  TGLabel* m_BytesReceivedLabel;
  
  //! The total number of raw data frames parsed
  long m_RawFramesParsed;
  //! The label showing the number of raw data frames parsed
  TGLabel* m_RawFramesLabel;
  
  //! The total number of Compton data frames parsed
  long m_ComptonFramesParsed;
  //! The label showing the number of Compton data frames parsed
  TGLabel* m_ComptonFramesLabel;
  
  //! The total number of aspect data frames parsed
  long m_AspectFramesParsed;
  //! The label showing the number of aspect data frames parsed
  TGLabel* m_AspectFramesLabel;
  
  //! The total number of other data frames parsed
  long m_OtherFramesParsed;
  //! The label showing the number of other data frames parsed
  TGLabel* m_OtherFramesLabel;

  
#ifdef ___CINT___
 public:
  ClassDef(MGUIExpoReceiver, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
