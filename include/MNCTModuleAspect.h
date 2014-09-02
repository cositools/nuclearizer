/*
 * MNCTModuleAspect.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTModuleAspect__
#define __MNCTModuleAspect__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "MString.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MNCTTimeAndCoordinate.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTModuleAspect : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MNCTModuleAspect();
  //! Default destructor
  virtual ~MNCTModuleAspect();

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //!
  virtual MString Report();

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  //! 
  //virtual void SetAspectFilename(string FileName){m_AspectFilename = FileName;}
  //virtual string GetAspectFilename(){return m_AspectFilename;}

  //!
  virtual void SetCoordinate(int coordinate){m_Coordinate = coordinate;}
  virtual int GetCoordinate(){return m_Coordinate;}

  //!
  void SetMJDZero(double MJDZero){m_MJDZero=MJDZero;}
  double GetMJDZero(){return m_MJDZero;}

  //!
  void SetAspectDelay(double AD){m_AspectDelay=AD;}
  double GetAspectDelay(){return m_AspectDelay;}
	  

  //!
  void SetRunTimeCorrection(bool yn=true){m_RunTimeCorrection=yn;}
  bool GetRunTimeCorrection(){return m_RunTimeCorrection;}
  
  //!
  void SetVerbose(bool verbose=true){m_Verbose = verbose;}
  bool GetVerbose(){return m_Verbose;}

  // protected methods:
 protected:
  //!
  virtual void LoadTimeParameterFile();
  virtual void LoadSegmentCorrectionFile();

  // Load the old aspect file
  //virtual void LoadAspectFile();
  // Load the new aspect files
  virtual void LoadAspectFiles();
  virtual void LoadAspectFile_(MString Filename,
			       vector< vector<double> >* AspectTable,
			       vector<int>* GCUIndexList,
			       vector< vector<double> >* GCUTimeTable);
  // Calculate indices into the aspect files surrounding the event's position
  vector<int> AspectTableIndices(MReadOutAssembly* E,
				 vector< vector<double> >* AspectTable,
				 vector<int>* GCUIndexList,
				 vector< vector<double> >* GCUTimeTable);
  // calculate aspect information interpolated from aspect files
  vector<double> InterpolatedAspectData(MReadOutAssembly* E);

  //!
  //int FindRow(double time);
  //double TimeCorrection(double TI, double CL);
  
  // !
  bool FindSegment(MReadOutAssembly* E);
  double CorrectedUnixTime(MReadOutAssembly* E);
  // use both time corrections to calculate GPS (Universal) Time (in Unix Epoch)
  double GPSTime(MReadOutAssembly* E);
  
  // private methods:
 private:
  //!
  //vector<double> SexagesimalToDegrees();



  // protected members:
 protected:


  // private members:
 private:
  //!
  MNCTTimeAndCoordinate m_TCCalculator;

  //!
  double m_MJDZero;

  //!
  double m_AspectDelay;

  //!
  int m_Coordinate;

  //!
  bool m_RunTimeCorrection;
  
  //!
  //string m_AspectFilename;
  string m_AspectFilename_1sec;
  string m_AspectFilename_4sec;

  //!
  bool m_Verbose;

  //!
  double m_GapTime;

  //!
  int m_NBadEvent;

  //!
  vector< vector<double> > m_TimeParTable;
  vector< vector<double> > m_SegmentCorrectionTable;
  int m_Segment;

  //!
  //vector<double> m_TimeList;

  //!
  vector< vector<double> > m_AspectTable;
  vector< vector<double> > m_AspectTable_1sec;
  vector< vector<double> > m_AspectTable_4sec;
  vector<int> m_GCUIndexList_1sec;
  vector<int> m_GCUIndexList_4sec;
  vector< vector<double> > m_GCUTimeTable_1sec;
  vector< vector<double> > m_GCUTimeTable_4sec;


#ifdef ___CINT___
 public:
  ClassDef(MNCTModuleAspect, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
