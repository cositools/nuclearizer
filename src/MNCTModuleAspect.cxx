/*
 * MNCTModuleAspect.cxx
 *
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Jau-Shian Liang.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MNCTModuleAspect
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleAspect.h"

// Standard libs:
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>

// ROOT libs:
#include "TGClient.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "MString.h"
#include "TFile.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MGUIOptionsAspect.h"
#include "MNCTModule.h"
#include "MNCTMath.h"
#include "MNCTTimeAndCoordinate.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleAspect)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleAspect::MNCTModuleAspect() : MNCTModule()
{
  // Construct an instance of MNCTModuleTemplate

  // Set all module relevant information

  // Set the module name --- has to be unique
  m_Name = "Update time and aspect information";

  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "XmlTagAspect";

  // Set all modules, which have to be done before this module
//  AddPreceedingModuleType(c_DetectorEffectsEngine);
//  AddPreceedingModuleType(c_EnergyCalibration);
//  AddPreceedingModuleType(c_ChargeSharingCorrection);
  AddPreceedingModuleType(c_DepthCorrection);
//  AddPreceedingModuleType(c_StripPairing);
//  AddPreceedingModuleType(c_EventReconstruction);

  // Set all types this modules handles
//  AddModuleType(c_DetectorEffectsEngine);
//  AddModuleType(c_EnergyCalibration);
//  AddModuleType(c_ChargeSharingCorrection);
//  AddModuleType(c_DepthCorrection);
//  AddModuleType(c_StripPairing);
  AddModuleType(c_Aspect);
//  AddModuleType(c_EventReconstruction);

  // Set all modules, which can follow this module
//  AddSucceedingModuleType(c_DetectorEffectsEngine);
  AddSucceedingModuleType(c_EnergyCalibration);
  AddSucceedingModuleType(c_ChargeSharingCorrection);
  AddSucceedingModuleType(c_DepthCorrection);
  AddSucceedingModuleType(c_StripPairing);
  AddSucceedingModuleType(c_EventReconstruction);
  AddSucceedingModuleType(c_Else);

  // Set if this module has an options GUI
  // Overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
  // If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleAspect::~MNCTModuleAspect()
{
  // Delete this instance of MNCTModuleTemplate
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleAspect::Initialize()
{
  // Initialize the module 
  //m_AspectFilename="aspect.txt";

  cout << endl << "Initializing MNCTModuleAspect..." << endl;
  m_GapTime=30.0;
  m_AspectDelay = 0.0;

  //LoadTimeParameterFile();
  //LoadAspectFile();
  // new aspect and corrections
  LoadSegmentCorrectionFile();
  LoadAspectFiles();

  m_NBadEvent=0;
	
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleAspect::AnalyzeEvent(MNCTEvent* Event) 
{
  // Main data analysis routine, which updates the event to a new level

  //update time (Unix time -> time after MJDZero)
  if(m_Verbose) mout << setprecision(14);
  if(m_Verbose) mout << "ID: " << Event->GetID() << endl;
  if(m_Verbose) mout << "    TI: " << Event->GetTI() 
		     << "    CL: " << Event->GetCL()
		     << "    FC: " << Event->GetFC() << endl;
  
  // mout << "GetTime: " << Event->GetTime() 
  //   << "   GetMJDZero: " << m_TCCalculator.GetMJDZero() 
  //   << "   GetUnixTime:" << m_TCCalculator.GetUnixTime() << endl;
  
  // prevent bad time values in input file
  if ((Event->GetTI()<1242000000) || (Event->GetCL()>4294967295UL))
    {
      // time information is wrong.  Veto the event
      Event->SetVeto(true);
      m_NBadEvent++;
      return true;
    }	
  
  // Find aspect information for new aspect
  
  // New event time correction
  FindSegment(Event);
  if (m_Segment==-1)
    {
      // no segment found.  Cannot correct the event.
      Event->SetVeto(true);
      m_NBadEvent++;
      return true;
    }
  
  double T_corrected=Event->GetTime();
  if (m_RunTimeCorrection)T_corrected = CorrectedUnixTime(Event);
  Event->SetTime(T_corrected);

  Event->SetTime(T_corrected+m_AspectDelay);
  Event->SetTI(Event->GetTI()+(int)m_AspectDelay);
  
  // get interpolated aspect data
  vector<double> AspectData = InterpolatedAspectData(Event);
  
  if (AspectData.size()!=6)
    {
      // some kind of failure in finding the aspect; veto the event
      Event->SetVeto(true);
      m_NBadEvent++;
      return true;
    }
  
  // Assign the GPS time to the event (GCU time had been used to interpolate the aspect)
  if(m_RunTimeCorrection)Event->SetTime(GPSTime(Event));
  
  double Lat  = AspectData[0];
  double Lon  = AspectData[1];
  double Alt  = AspectData[2];
  double dGPS_Heading  = AspectData[3];
  double dGPS_Pitch  = AspectData[4];
  double dGPS_Roll  = AspectData[5];
  if (m_Verbose) mout << "dGPS Angles : " << dGPS_Heading << "  " << dGPS_Pitch << "  " << dGPS_Roll << endl;

  // calculate local sidereal time
  m_TCCalculator.SetLocation(Lat,Lon);
  m_TCCalculator.SetUnixTime(Event->GetTime());
  //double LAST_deg = m_TCCalculator.LAST_degrees();
  //mout << "  LAST=" << LAST_deg << '\n';
  
  Event->SetLatitude(Lat);
  Event->SetLongitude(Lon);
  Event->SetAltitude(Alt);

  // determine elevation and azimuth angles of x and z
  vector<double> Cryo_X = m_TCCalculator.CryoX_to_Horizon(dGPS_Pitch, dGPS_Roll, dGPS_Heading);
  vector<double> Cryo_Z = m_TCCalculator.CryoZ_to_Horizon(dGPS_Pitch, dGPS_Roll, dGPS_Heading);

  //GZ
  vector<double> v_gz;
  v_gz.clear();
  v_gz = m_TCCalculator.Horizon2Equatorial(Cryo_Z[0], Cryo_Z[1]);
  if(m_Coordinate == 2)
    {
      v_gz = m_TCCalculator.Equatorial2Galactic(v_gz);
    }
  Event->SetGZ(v_gz);
  if(m_Verbose) mout << "Aspect: GZ " << v_gz[0] << ' ' << v_gz[1] << endl;
  
  //GX
  vector<double> v_gx;
  v_gx.clear();
  v_gx = m_TCCalculator.Horizon2Equatorial(Cryo_X[0], Cryo_X[1]);
  if(m_Coordinate == 2)
    {
      v_gx = m_TCCalculator.Equatorial2Galactic(v_gx);
    }
  Event->SetGX(v_gx);
  if(m_Verbose) mout << "Aspect: GX " << v_gx[0] << ' ' << v_gx[1] << endl;
  
  //HZ (cryostat Z expressed in local horizon coordinates (azi,alt))
  vector<double> v_hz;
  v_hz.clear();
  v_hz.push_back(-1.0*Cryo_Z[0]);
  v_hz.push_back(Cryo_Z[1]);
  Event->SetHZ(v_hz);
  if(m_Verbose) mout << "Aspect: HZ " << v_hz[0] << ' ' << v_hz[1] << endl;
  
  //HX (cryostat X expressed in local horizon coordinates (azi,alt))
  vector<double> v_hx;
  v_hx.clear();
  v_hx.push_back(-1.0*Cryo_X[0]);
  v_hx.push_back(Cryo_X[1]);
  Event->SetHX(v_hx);
  if(m_Verbose) mout << "Aspect: HX " << v_hx[0] << ' ' << v_hx[1] << endl << endl;
  
  //Update output time
  double time_after_MJDZero=(m_TCCalculator.GetMJD()-m_MJDZero)*86400.0;
  Event->SetTime(time_after_MJDZero);

  // We're done; update the event
  Event->SetAspectAdded(true);

  return true;
}
////////////////////////////////////////////////////////////////////////////////
MString MNCTModuleAspect::Report()
{
  MString string_tmp;
  string_tmp += "  Vetoed events: ";
  string_tmp += m_NBadEvent;
  string_tmp += "\n";
  
  string_tmp += "    impossible time value:";
  string_tmp += m_NBadEvent;
  string_tmp += "\n";

  return string_tmp;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleAspect::ShowOptionsGUI()
{
  //! Show the options GUI --- has to be overwritten!

  MGUIOptionsAspect* Options = new MGUIOptionsAspect(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
  
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleAspect::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

//  MXmlNode* AspectNode = Node->GetNode("AspectFilename");
  MXmlNode* MJDZeroNode = Node->GetNode("MJDZero");
  MXmlNode* AspectDelayNode = Node->GetNode("AspectDelay");
  MXmlNode* CoordinateNode = Node->GetNode("Coordinate");
  MXmlNode* TimeCorrectionNode = Node->GetNode("TimeCorrection");
  MXmlNode* VerboseNode = Node->GetNode("Verbose");

//  if (MJDZeroNode !=0) SetAspectFilename((const char*)AspectNode->GetValueAsString());
//  else SetAspectFilename("");
  
  if (MJDZeroNode !=0) SetMJDZero(MJDZeroNode->GetValueAsDouble());
  else SetMJDZero(40587.0);

  if (AspectDelayNode !=0) SetAspectDelay(AspectDelayNode->GetValueAsDouble());
  else SetAspectDelay(0.0);

  if (CoordinateNode !=0) SetCoordinate(CoordinateNode->GetValueAsInt());
  else SetCoordinate(2);

  if (TimeCorrectionNode !=0) m_RunTimeCorrection = TimeCorrectionNode->GetValueAsBoolean();
  else m_RunTimeCorrection = true;

  if (VerboseNode !=0) m_Verbose = VerboseNode->GetValueAsBoolean();
  else m_Verbose = false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MNCTModuleAspect::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);

//  new MXmlNode(Node, "AspectFilename", GetAspectFilename());
  new MXmlNode(Node, "MJDZero", GetMJDZero());  
  new MXmlNode(Node, "AspectDelay", GetAspectDelay());
  new MXmlNode(Node, "Coordinate", GetCoordinate());
  new MXmlNode(Node, "TimeCorrection", m_RunTimeCorrection);
  new MXmlNode(Node, "Verbose", m_Verbose);

  return Node;
}

////////////////////////////////////////////////////////////////////////////////

void MNCTModuleAspect::LoadTimeParameterFile()
{

  string Path=getenv("NUCLEARIZER_CAL");
  string filename = Path+"/TimeCorrectionParameter.csv";
  char buffer[512];
  string sbuffer;

  int l=0;
  double tmp_double=0;
  vector<double> tmp_TimePar;
  
  ifstream f1(filename.c_str());

  cout << "Loading time correction parameter file...\n";//check point
  cout << "filename: " << filename << '\n';
  if(!f1.is_open()){
    cerr << "file does not exist!!!\n";
    exit(1);
  }
  if(f1.eof()){
    cerr << "EOF!!!\n";//debug
    exit(1);
  }

  double Tzero=m_TCCalculator.GetUnixTime();
  while(1)
  {
    f1.getline(buffer,512);
    if(f1.eof())break;
    sbuffer=buffer;
    istringstream isbuffer(sbuffer);
    if(buffer[0]!='#')
    {
      tmp_TimePar.clear();
      
      while(!isbuffer.eof())
      {
        isbuffer >> tmp_double;
//	printf("%.9lf ",tmp_double);
        tmp_TimePar.push_back(tmp_double);
      }
//      cout << endl;

      //subtract Tzero
      tmp_TimePar[0]-=Tzero;
      tmp_TimePar[1]-=Tzero;
      tmp_TimePar[2]-=Tzero;
      
      m_TimeParTable.push_back(tmp_TimePar);
      l++;
    }
  }
  if(m_Verbose)cout << "lines:" << l << '\n';
}	

////////////////////////////////////////////////////////////////////////////////

void MNCTModuleAspect::LoadSegmentCorrectionFile()
{
  m_SegmentCorrectionTable.clear();
  MString Filename = (MString)std::getenv ("NUCLEARIZER_CAL")+"/SegmentCorrection.csv";
  mout << "Loading segment correction file..." << endl;
  mout << "filename: " << Filename << endl;

  // Read the calibration coefficients line-by-line
  fstream File;
  File.open(Filename, ios_base::in);
  if (File.is_open() == false)
    {
      mout << "***Warning: Unable to open file: " << Filename << endl
	   << "   Is your NUCLEARIZER_CAL environment variable set?" << endl;
    }
  else
    {
      vector<double> tmp_SegmentPar;
      TString Line;
      while(!File.eof())
	{
	  Line.ReadLine(File);
	  if (Line.BeginsWith("#") == false)
	    {
	      tmp_SegmentPar.clear();
	      //mout << "Line: " << Line << endl;
	      TObjArray* Data = Line.Tokenize(",");
	      TObjArrayIter Iter(Data);
	      TObjString* tmp_objstr;
	      while ((tmp_objstr = (TObjString*)Iter.Next()))
		{
		  //mout << "  single item from line " << ":  " << tmp_objstr->GetString() << endl;
		  tmp_SegmentPar.push_back((double)(tmp_objstr->GetString().Atof()));
		}
	      if (tmp_SegmentPar.size()==25)
		{
		  m_SegmentCorrectionTable.push_back(tmp_SegmentPar);
		}
	      else
		{
		  mout << "LoadSegmentCorrectionFile Warning: line not properly loaded:  " << Line << endl;
		}
	    }
	}
      mout << "Finished loading segment correction file." << endl;
    } // done reading from file
}	

////////////////////////////////////////////////////////////////////////////////

void MNCTModuleAspect::LoadAspectFiles()
{
  MString Filename_1sec;
  if(m_RunTimeCorrection){
    Filename_1sec = (MString)std::getenv ("NUCLEARIZER_CAL")
      +"/aspect_NCT09_flight_GCU_HK_1sec.csv";
  }else{
    Filename_1sec = (MString)std::getenv ("NUCLEARIZER_CAL")
      +"/aspect_NCT09_flight_GCU_HK_4sec.csv";
  }

  mout << "Loading aspect file (1 sec resolution)..." << endl;
  mout << "filename: " << Filename_1sec << endl;
  LoadAspectFile_(Filename_1sec, &m_AspectTable_1sec,
		  &m_GCUIndexList_1sec,
		  &m_GCUTimeTable_1sec);




  MString Filename_4sec = (MString)std::getenv ("NUCLEARIZER_CAL")
    +"/aspect_NCT09_flight_GCU_HK_4sec.csv";
  mout << "Loading aspect file (4 sec resolution)..." << endl;
  mout << "filename: " << Filename_4sec << endl;
  LoadAspectFile_(Filename_4sec, &m_AspectTable_4sec,
		  &m_GCUIndexList_4sec,
		  &m_GCUTimeTable_4sec);
}

////////////////////////////////////////////////////////////////////////////////

void MNCTModuleAspect::LoadAspectFile_(MString Filename,
				       vector< vector<double> > *AspectTable,
				       vector<int> *GCUIndexList,
				       vector< vector<double> > *GCUTimeTable)
{
  (*AspectTable).clear();
  (*GCUIndexList).clear();
  (*GCUTimeTable).clear();
  vector<double> GCUHKTable;

  // Read the aspect data line-by-line
  fstream File;
  File.open(Filename, ios_base::in);
  if (File.is_open() == false)
    {
      mout << "***Warning: Unable to open file: " << Filename << endl
	   << "   Is your NUCLEARIZER_CAL environment variable set?" << endl;
    }
  else
    {
      vector<double> tmp_AspectLine;
      TString Line;
      while(!File.eof())
	{
	  Line.ReadLine(File);
	  if (Line.BeginsWith("#") == false)
	    {
	      tmp_AspectLine.clear();
	      //mout << "Line: " << Line << endl;
	      TObjArray* Data = Line.Tokenize(",");
	      TObjArrayIter Iter(Data);
	      TObjString* tmp_objstr;
	      while ((tmp_objstr = (TObjString*)Iter.Next()))
		{
		  //mout << "  single item from line " << ":  " << tmp_objstr->GetString() << endl;
		  tmp_AspectLine.push_back((double)(tmp_objstr->GetString().Atof()));
		}
	      if (tmp_AspectLine.size()==17)
		{
		  (*AspectTable).push_back(tmp_AspectLine);
		  GCUHKTable.push_back(tmp_AspectLine[0]);
		}
	      else
		{
		  merr << "LoadAspectFile_ Error: line not properly loaded:  " << Line << endl;
		}
	    }
	}
      mout << "Finished loading aspect file.  Number of lines: " << (*AspectTable).size() << endl;
    } // done reading from file

  // Create tables of GCU Time for the different segments (requires loading of segment data!)
  // First, determine the GCUHK boundaries of each segment
  for (unsigned int segment=0; segment<m_SegmentCorrectionTable.size(); segment++)
    {
      double GCUHK_Start = m_SegmentCorrectionTable[segment][5];
      double GCUHK_End = m_SegmentCorrectionTable[segment][6];
      vector<double>::iterator iter_lower, iter_upper;

      // find the indices around that segment of the data (using GCUHK range)
      iter_lower = lower_bound(GCUHKTable.begin(), GCUHKTable.end(), GCUHK_Start);
      iter_upper = upper_bound(GCUHKTable.begin(), GCUHKTable.end(), GCUHK_End);
      iter_upper--;
      int index_lower = (int)(iter_lower-GCUHKTable.begin());
      int index_upper = (int)(iter_upper-GCUHKTable.begin());
      //mout << endl;
      //mout << "LoadAspectFile_:  Segment = " << segment << endl;
      //mout << "LoadAspectFile_:  GCUHK_Start = " << GCUHK_Start 
      //   << "   GCUHK_End = " << GCUHK_End << endl;
      //mout << "LoadAspectFile_:  index_lower = " << index_lower 
      //   << " index_upper = " << index_upper << endl;
      //mout << "LoadAspectFile_:  GCUHKTable.size() = " << GCUHKTable.size() << endl;
      //mout << "LoadAspectFile_:     GCUHK[" << index_lower << "] = " << GCUHKTable[index_lower]
      //   << "  GCUHK[" << index_upper << "] = " << GCUHKTable[index_upper] << endl;
      //mout << "LoadAspectFile_: PCTime_Start = " << (*AspectTable)[index_lower][7] 
      //   << "  PCTime_End = " << (*AspectTable)[index_upper][7] << endl;

      // Save the start indices of each segment
      //mout << "LoadAspectFile_: Start index = " << index_lower << endl;
      (*GCUIndexList).push_back(index_lower);

      // Now create a list of the GCU times within that segment
      vector<double> tmp_time;
      tmp_time.clear();
      for (int j=index_lower; j<=index_upper; j++)
	{
	  if(m_RunTimeCorrection)tmp_time.push_back((*AspectTable)[j][7]);
	  else tmp_time.push_back((*AspectTable)[j][8]);
	}
      //mout << "LoadAspectFile_: PCTime_Start = " << tmp_time[0] 
      //   << "  PCTime_End = " << tmp_time[tmp_time.size()-1] << endl;
      (*GCUTimeTable).push_back(tmp_time);
    }
}

////////////////////////////////////////////////////////////////////////////////

//Unavailable!!
/*
void MNCTModuleAspect::LoadAspectFile()
{
 
  char buffer[512];
  string sbuffer;

  int l=0;
  int tmp_time[6];
  double tmp_unixtime=0;
  double tmp_double;
  vector<double> tmp_aspect;
  
  ifstream f1(m_AspectFilename.c_str());

  mout << "Loading aspect file...\n";//check point
  mout << "filename: " << m_AspectFilename << '\n';
  if(!f1.is_open()){
    merr << "file does not exist!!!\n";
    exit(1);
  }
  if(f1.eof()){
    merr << "EOF!!!\n";//debug
    exit(1);
  }

  double Tzero=m_TCCalculator.GetUnixTime();
  while(1)
  {
    f1.getline(buffer,512);
    if(f1.eof())break;
  
    sbuffer=buffer;
    istringstream isbuffer(sbuffer);
    if(buffer[0]!='#')
    {
      tmp_aspect.clear();
      for(int i=0; i<6; i++)
      {
       isbuffer >> tmp_time[i];
      }
     
      //subtract Tzero and push_back time
      isbuffer >> tmp_unixtime;
      m_TimeList.push_back(tmp_unixtime - Tzero);
      tmp_aspect.push_back(tmp_unixtime - Tzero);

      //XXX
      //cout << tmp_unixtime << ' ' << Tzero << ' ' << tmp_unixtime - Tzero << '\n';
      
      while(!isbuffer.eof())
      {
        isbuffer >> tmp_double;
        tmp_aspect.push_back(tmp_double);
      }

      m_AspectTable.push_back(tmp_aspect);
      l++;
    }
  }
  if(m_Verbose) mout << "lines:" << l << '\n';
}
*/

////////////////////////////////////////////////////////////////////////////////

//Unavailable!!
/*
double MNCTModuleAspect::TimeCorrection(double ti, double cl)
{
	double cl_cycle=4294967296.0;
	double Ti_cycle=429.5; //assume frequency is 1e+7
	int interval=-1;
	int n;
	double nf;
	double correct_time;
	
	//find interval
	for(unsigned int i=0;i<m_TimeParTable.size();i++)
	{
		if (ti>m_TimeParTable[i][0] && ti<m_TimeParTable[i][1])
		{
			interval=1;
			break;
		}
	}
	
	if(interval==-1)
	{
	  //merr << "Warning: can't correct time information!!\n";
	  //	merr << "Event time: " << ti << endl;
		correct_time = ti;
	}
	else
	{
		//
		double t0=m_TimeParTable[interval][2];
		double t1=m_TimeParTable[interval][3];
		//double t_offset=m_TimeParTable[interval][4];
		n=(int)floor((ti-t0)/Ti_cycle);
		nf=(ti-t0)/Ti_cycle - n;

		//
		if(cl<1400000000 && nf > 0.70 )n+=1;
		if(cl>2900000000 && nf < 0.30 )n-=1;

		double mcl=cl+ n*cl_cycle;
		correct_time = t0 + t1*mcl;
	}
	return correct_time;
}
*/

////////////////////////////////////////////////////////////////////////////////

//Unavailable!!
/*
int MNCTModuleAspect::FindRow(double time)
{
  //TODO:
  vector<double>::iterator up;
  
  up= upper_bound(m_TimeList.begin(), m_TimeList.end(), time);
  
  return (int)(up-m_TimeList.begin());
}
*/

////////////////////////////////////////////////////////////////////////////////

bool MNCTModuleAspect::FindSegment(MNCTEvent* E)
{
  m_Segment = -1;
  // Search through entire segment file and find a section matching the
  // event's TI (Unix time) and FC (frame counter).
  for (unsigned int j=0; j<m_SegmentCorrectionTable.size(); j++)
    {
      if ( (m_SegmentCorrectionTable[j][1] <= ((double)E->GetTI()))
	   && (((double)E->GetTI()) <= m_SegmentCorrectionTable[j][2]))
	{
	  if (m_RunTimeCorrection)
	    {
	      if((m_SegmentCorrectionTable[j][3] <= ((double)E->GetFC()))
		 && (((double)E->GetFC()) <= m_SegmentCorrectionTable[j][4]) )
		{
		  m_Segment = j;
		}
	    }
	  else
	    {
	      m_Segment = j;
	    }
	}
    }
  
  if (m_Verbose)
    {
      mout << "FindSegment: Event TI = " << E->GetTI() << "  FC = " << E->GetFC() << endl;
      if (m_Segment==-1)
	{
	  mout << "FindSegment: No segment found for event." << endl;
	}
      else
	{
	  mout << "FindSegment: Event found to be in segment " << m_Segment << endl;
	  mout << "FindSegment: TI_Start = " << m_SegmentCorrectionTable[m_Segment][1] 
	       << " TI_End = " << m_SegmentCorrectionTable[m_Segment][2] << endl;
	  mout << "FindSegment: FC_Start = " << m_SegmentCorrectionTable[m_Segment][3] 
	       << " FC_End = " << m_SegmentCorrectionTable[m_Segment][4] << endl;
	}
    }
  if (m_Segment==-1) return false;
  else return true;
}

////////////////////////////////////////////////////////////////////////////////

double MNCTModuleAspect::CorrectedUnixTime(MNCTEvent* E)
{
  unsigned long CL_period=4294967295UL;
  double TI_period=429.4967; //assume frequency is 1e+7
  double T_corrected;

  // Ensure a segment has been searched for
  if (m_Segment==-1)
    {
      FindSegment(E);
    }

  // see if segment is valid
  if (m_Segment==-1)
    {
      merr << "CorrectedUnixTime: Warning: Segment of the event not found.  Cannot correct the time!" << endl;
      return (double)E->GetTI();
    }
  // also exclude events where there are no hits (need a detector number for the corrections)
  else if (E->GetNStripHits()==0)
    {
      merr << "CorrectedUnixTime: Warning: Event has no hits.  Cannot correct the time!" << endl;
      return (double)E->GetTI();
    }
  else
    {
      unsigned long CL = (unsigned long)E->GetCL();
      // No longer apply the CL offset here; moved to FileEventsDat so that coincidences can be found
//       // Apply the CL offset appropriate for the given detector
//        int Detector=E->GetStripHit(0)->GetDetectorID();
//        double CL_offset = m_SegmentCorrectionTable[m_Segment][14+Detector];
//        if (m_Verbose)
//  	{
//  	  mout << setprecision(14);
//  	  mout << "CorrectedUnixTime: CL offset correction: Original CL: " << CL << endl;
//  	  mout << "CorrectedUnixTime: CL offset correction: CL offset:   " << CL_offset << " (D" << Detector << ")" << endl;
//  	}
//        CL = (unsigned long)((double)CL - CL_offset);
//        // ensure CL is still between 0 and 2^32
//        //CL = fmod(CL,CL_period);
//        CL = (CL % CL_period);
//        if (m_Verbose)
//  	{
//  	  mout << "CorrectedUnixTime: CL offset correction: Final CL:    " << CL << endl;
//  	}

      // Calculate the unrolled CL time:
      double TI = (double) E->GetTI();
      double TI_0 = m_SegmentCorrectionTable[m_Segment][7];
      double CL_0 = m_SegmentCorrectionTable[m_Segment][8];
      int n_rollovers;
      double n, f;
      n = (TI-TI_0)/TI_period + CL_0/((double)CL_period);
      n_rollovers = (int)floor(n);
      f = n - n_rollovers;
      if( CL<1400000000UL && f>0.70 ) n_rollovers += 1;
      if( CL>2900000000UL && f<0.30 ) n_rollovers -= 1;
      CL += (unsigned long)(n_rollovers*((double)CL_period));
      if (m_Verbose)
	{
	  mout << "CorrectedUnixTime: CL rollover correction: TI_0         " << TI_0 << endl;
	  mout << "CorrectedUnixTime: CL rollover correction: CL_0:      " << CL_0 << endl;
	  mout << "CorrectedUnixTime: CL rollover correction: f:           " << f << endl;
	  mout << "CorrectedUnixTime: CL rollover correction: n rollovers: " << n_rollovers << endl;
	  mout << "CorrectedUnixTime: CL rollover correction: Final CL:  " << CL << endl;
	}

      // Calculate the corrected Unix time
      double A_0 = m_SegmentCorrectionTable[m_Segment][9];
      double A_1 = m_SegmentCorrectionTable[m_Segment][10];
      T_corrected = TI_0 + A_0 + A_1*((double)CL-CL_0)/1.e7;
      if (m_Verbose)
	{
	  mout << setprecision(20);
	  mout << "CorrectedUnixTime: Unix time correction: A_0:      " << A_0 << endl;
	  mout << "CorrectedUnixTime: Unix time correction: A_1:      " << A_1 << endl;
	  mout << "CorrectedUnixTime: Unix time correction: Final TI:        " << T_corrected << endl;
	}
      if (fabs(TI-T_corrected)>2.5)
	{
	  mout << "CorrectedUnixTime: Warning!" 
	       << "  Corrected Unix time is more than 2.5 seconds from TI." << endl;
	  mout << setprecision(10);
	  mout << "   ID: " << E->GetID()
	       << " CL = " << CL << " (nr=" << n_rollovers << ")"
	       << " TI = " << TI << ", Corrected time = " << T_corrected
	       << ", diff = " << (T_corrected-TI) 
	       << "  Segment #" << m_Segment << endl;
	}
      return T_corrected;
    }
}

////////////////////////////////////////////////////////////////////////////////

double MNCTModuleAspect::GPSTime(MNCTEvent* E)
{
  // Ensure a segment has been searched for
  if (m_Segment==-1)
    {
      FindSegment(E);
    }
  // see if segment is valid
  if (m_Segment==-1)
    {
      merr << "Event_UT: Warning: Segment of the event not found.  Cannot correct the time!" << endl;
      return (double)E->GetTI();
    }
  else
    {
      // Calculate the corrected Unix time
      double TI_0_GPS = m_SegmentCorrectionTable[m_Segment][11];
      double B_0 = m_SegmentCorrectionTable[m_Segment][12];
      double B_1 = m_SegmentCorrectionTable[m_Segment][13];
      double B_2 = m_SegmentCorrectionTable[m_Segment][14];
      double TC = CorrectedUnixTime(E);
      double UT_Unix = TC + B_0 + B_1*(TC-TI_0_GPS) + B_2*(TC-TI_0_GPS)*(TC-TI_0_GPS);
      if (m_Verbose)
	{
	  //mout << "GPSTime: Unix to dGPS time correction: B_0:      " << B_0 << endl;
	  //mout << "GPSTime: Unix to dGPS time correction: B_1:      " << B_1 << endl;
	  //mout << "GPSTime: Unix to dGPS time correction: B_2:      " << B_2 << endl;
	  mout << "GPSTime: Unix to dGPS time correction: Original Unix time:          " << TC << endl;
	  mout << "GPSTime: Unix to dGPS time correction: Final GPS time (Unix Epoch): " << UT_Unix << endl;
	}
      return UT_Unix;
    }
}

////////////////////////////////////////////////////////////////////////////////

// Calculate indices into the aspect files surrounding the event's position
vector<int> MNCTModuleAspect::AspectTableIndices(MNCTEvent* E,
						 vector< vector<double> > *AspectTable,
						 vector<int> *GCUIndexList,
						 vector< vector<double> > *GCUTimeTable)
{
  vector<int> tmp_position;
  tmp_position.clear();

  if (m_Segment!=-1)
    {
      double T = E->GetTime();
      // lower keeps the last entry before the event
      // higher keeps the next entry after the event
      int index_lower, index_upper, index_center;
      double T_lower, T_upper;

      // Find the lower and upper indices within that segment using Unix Time information
      //mout << "Segment: " << m_Segment << endl;
      //mout << setprecision(15);
      //mout << "Time:    " << T << endl;
      //mout << "GCUTimeTable start/end: " << (*GCUTimeTable)[m_Segment][0] << endl;
      //mout << "GCUTimeTable size: " << ((*GCUTimeTable)[m_Segment]).size() << endl;
      vector<double>::iterator iter_time_lower, iter_time_upper;
      iter_time_lower = lower_bound(((*GCUTimeTable)[m_Segment]).begin(), ((*GCUTimeTable)[m_Segment]).end(), T);
      iter_time_upper = upper_bound(((*GCUTimeTable)[m_Segment]).begin(), ((*GCUTimeTable)[m_Segment]).end(), T);
      if (iter_time_lower != ((*GCUTimeTable)[m_Segment]).begin())
	{
	  iter_time_lower--;
	}
      //mout << "GCUTimeTable bounds: " << *iter_time_lower << "  " << *iter_time_upper << endl;
      //mout << "AspectTableIndices:  index_lower = " << (int)(iter_time_lower-((*GCUTimeTable)[m_Segment]).begin())
      //   << "   index_upper = " << (int)(iter_time_upper-((*GCUTimeTable)[m_Segment]).begin()) << endl;
      //mout << "GCUIndexList[] = " << (*GCUIndexList)[m_Segment] << endl;
      index_lower = (*GCUIndexList)[m_Segment] + (int)(iter_time_lower-((*GCUTimeTable)[m_Segment]).begin());
      index_upper = (*GCUIndexList)[m_Segment] + (int)(iter_time_upper-((*GCUTimeTable)[m_Segment]).begin());
      //mout << "AspectTableIndices:  index_lower = " << index_lower 
      //   << "   index_upper = " << index_upper << endl;
      //mout << "AspectTableIndices:      T_lower = " << (*AspectTable)[index_lower][7]
      //   << "       T_upper = " << (*AspectTable)[index_upper][7] << endl;
      index_center = index_lower;

      // Now make sure that we are not using vetoed data
      while ( ((*AspectTable)[index_lower][16] != 11001.0) && (index_lower>=0) )
	{
	  index_lower--;
	}
      while ( ((*AspectTable)[index_upper][16] != 11001.0) && (index_upper<(int)AspectTable->size()) )
	{
	  index_upper++;
	}

      // If we were successful, check what we have found
      if ( (index_lower>=0) && (index_upper<(int)AspectTable->size()) )
	{
	  if (m_RunTimeCorrection)T_lower = (*AspectTable)[index_lower][7];
	  else T_lower = (*AspectTable)[index_lower][8];
	  
	  if (m_RunTimeCorrection)T_upper = (*AspectTable)[index_upper][7];
	  else T_upper = (*AspectTable)[index_upper][8];
	  //mout << "AspectTableIndices: " << T_lower << " " << T << " " << T_upper << endl;
	  //mout << "AspectTableIndices:  index_lower = " << index_lower 
	  //   << "   index_upper = " << index_upper << endl;

	  // Have we found a valid position in the file? (i.e., surrounding T, close enough to T)
	  if ( (T_lower<=T) && (T<=T_upper) 
	       && (T_lower>=T-15.) and (T_upper<=T+15.) 
	       && (index_center-index_lower<8) && (index_upper-index_center<8) )
	    {
	      if (m_Verbose)
		{
		  //mout << "AspectTableIndices:  Position in aspect file found!" << endl;
		  mout << "AspectTableIndices: " << endl;
		  mout << "     Lower index: " << index_lower 
		       << "  Upper index: " << index_upper << endl;
		  mout << "     Event time:  " << T << endl;
		  mout << "     Lower time:  " << (*AspectTable)[index_lower][7] 
		       << "  Upper time:  " << (*AspectTable)[index_upper][7] << endl;
		}
	      // finish
	      tmp_position.clear();
	      tmp_position.push_back(index_lower);
	      tmp_position.push_back(index_upper);
	      return tmp_position;
	    }
	  else
	    {
	      // return no positions because no good information near the event
	      mout << "AspectTableIndices:  No aspect information found for event"
		   << " (no valid aspect within 15 sec)." << endl 
		   << "   ID=" << E->GetID() << " FC=" << E->GetFC()
		   << " TI=" << E->GetTI() << endl;
	      mout << T_lower << ' ' << T << ' ' << T_upper << '\n';
	      return tmp_position;
	    }
	}
      else
	{
	  // return no positions because went outside bounds of segment
	  mout << "AspectTableIndices:  No aspect information found for event"
	       << " (outside bounds of segment #" << m_Segment << ")." << endl
	       << "    ID=" << E->GetID() << " FC=" << E->GetFC()
	       << " TI=" << E->GetTI() << endl;
	  return tmp_position;
	}
    }
  else
    {
      // return no positions because segment is unknown
      mout << "AspectTableIndices:  No aspect information found for event"
	   << " (unknown segment)." << endl
	   << "    ID=" << E->GetID() << " FC=" << E->GetFC()
	   << " TI=" << E->GetTI() << endl;
      return tmp_position;
    }
}

////////////////////////////////////////////////////////////////////////////////

vector<double> MNCTModuleAspect::InterpolatedAspectData(MNCTEvent* E)
{
  vector<double> tmp_aspect;
  tmp_aspect.clear();

  // Ensure a segment has been found
  if (m_Segment==-1)
    {
      merr << "AspectInformation: Warning: Segment of the event not found.  Cannot calculate the aspect!" << endl;
      return tmp_aspect;
    }
  else
    {
      vector<int> ind_1sec = AspectTableIndices(E, &m_AspectTable_1sec,
						&m_GCUIndexList_1sec,
						&m_GCUTimeTable_1sec);
      vector<int> ind_4sec = AspectTableIndices(E, &m_AspectTable_4sec,
						&m_GCUIndexList_4sec,
						&m_GCUTimeTable_4sec);
      if ( (ind_1sec.size()==2) && (ind_4sec.size()==2) )
	{
	  // Found good data surrounding the event in the aspect files.
	  double T=E->GetTime();
	  int t_col_n=0;
	  if (m_RunTimeCorrection)t_col_n=7;
	  else t_col_n=8;
	  double T_0_1sec = m_AspectTable_1sec[ind_1sec[0]][t_col_n];
	  double T_1_1sec = m_AspectTable_1sec[ind_1sec[1]][t_col_n];
	  double T_0_4sec = m_AspectTable_4sec[ind_4sec[0]][t_col_n];
	  double T_1_4sec = m_AspectTable_4sec[ind_4sec[1]][t_col_n];
	  
	  // interpolate!
	  double lat = MNCTMath::linear_interpolate(T, T_0_4sec, T_1_4sec,
						    m_AspectTable_4sec[ind_4sec[0]][10],
						    m_AspectTable_4sec[ind_4sec[1]][10]);
	  double lon = MNCTMath::linear_interpolate(T, T_0_4sec, T_1_4sec,
						    m_AspectTable_4sec[ind_4sec[0]][11],
						    m_AspectTable_4sec[ind_4sec[1]][11]);
	  double alt = MNCTMath::linear_interpolate(T, T_0_4sec, T_1_4sec,
						    m_AspectTable_4sec[ind_4sec[0]][12],
						    m_AspectTable_4sec[ind_4sec[1]][12]);
	  double heading = MNCTMath::linear_interpolate_angle360(T, T_0_1sec, T_1_1sec,
							m_AspectTable_1sec[ind_1sec[0]][13],
							m_AspectTable_1sec[ind_1sec[1]][13]);
	  double pitch = MNCTMath::linear_interpolate(T, T_0_4sec, T_1_4sec,
						      m_AspectTable_4sec[ind_4sec[0]][14],
						      m_AspectTable_4sec[ind_4sec[1]][14]);
	  double roll = MNCTMath::linear_interpolate(T, T_0_4sec, T_1_4sec,
						     m_AspectTable_4sec[ind_4sec[0]][15],
						     m_AspectTable_4sec[ind_4sec[1]][15]);
	  tmp_aspect.push_back(lat);
	  tmp_aspect.push_back(lon);
	  tmp_aspect.push_back(alt);
	  tmp_aspect.push_back(heading);
	  tmp_aspect.push_back(pitch);
	  tmp_aspect.push_back(roll);
	  return tmp_aspect;
	}
      else
	{
	  // Did not find good data surrounding the event in the aspect files.
	  return tmp_aspect;
	}
    }
}

////////////////////////////////////////////////////////////////////////////////


// MNCTModuleTemplate.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
