/*
* MNCTDetectorArray.cxx
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
// MNCTDetectorArray
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTDetectorArray.h"

// Standard libs:
#include <iostream>
#include <fstream>

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTDetectorArray)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTDetectorArray::MNCTDetectorArray()
{
  // Construct an instance of MNCTHit
  Initialize();
  
  
}


////////////////////////////////////////////////////////////////////////////////


MNCTDetectorArray::~MNCTDetectorArray()
{
  // Delete this instance of MNCTHit
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::Initialize()
{
  //!
  m_Geometry=0;
  m_DetectorFile="";
  m_DeadStripFile="";
  m_DetectorNumber=0;
  m_LoadDeadStrip=true;
  m_LoadCoincidence=false;
  m_LoadAntiCoincidence=false;
  
  //! Detector parameters
  m_Width=1.5;//thickness
  m_Length=8.04;//full length of edge
  m_CutLength=1.138;//triangle cut at corner(cut down length of edge)
  
  //! Strip parameters
  m_Strips=37;//strip number
  m_FirstStripID=1;//the first strip ID
  m_Pitch=0.2;//strip pitch
  m_Gap=0.02;//gap width between strips
  m_GuardringWidth=0.32;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::Activate()
{
  DumpParameters();
  
  FindAllDetectorVolumes();
  cout << m_AllDetectorVolumes.size() << " detector volumes have been found in geometry file..."<<endl<<endl;
  
  Load_Detector(m_DetectorFile);
  cout << "Detector number=" << GetDetectorNum() << '\n';
  DumpDetectors(); 
  
  if(m_LoadDeadStrip){
    Load_DeadStrip(m_DeadStripFile);
    DumpDeadStrips();
  }
  
  if(m_LoadCoincidence){
    Load_Coincidence(m_CoinFile);
    DumpCoincidence();
  }
  
  if(m_LoadAntiCoincidence){
    Load_AntiCoincidence(m_AntiCoinFile);
    DumpAntiCoincidence();
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorArray::IsNCTDetector(const MString& Name)
{
  for (unsigned int i = 0; i < m_NCTDetectors.size(); ++i) {
    if (Name == m_NCTDetectors[i].DetectorName) {
      return true;
    }
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorArray::IsDeadStrip(const MNCTStrip& Strip)
{
  for(unsigned int i=0;i<m_DeadStrips.size();i++)
  {
    if(m_DeadStrips[i]==Strip)return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorArray::IsDeadStrip(int DetectorID, bool XStrip, int StripID)
{
  MNCTStrip Strip(DetectorID,XStrip,StripID);
  return IsDeadStrip(Strip);
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorArray::IsCoincidence(const MDVolume* Vol)
{
  for(unsigned int i=0;i<m_CoinVol.size();i++)
  {
    if((m_CoinVol[i]).GetVolume()==Vol)return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorArray::IsAntiCoincidence(const MDVolume* Vol)
{
  for(unsigned int i=0;i<m_AntiCoinVol.size();i++)
  {
    if((m_AntiCoinVol[i]).GetVolume()==Vol)return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


MNCTCoincidenceVolume* MNCTDetectorArray::GetCoincidenceVolume(const MDVolume* Vol)
{
  for(unsigned int i=0;i<m_CoinVol.size();i++)
  {
    if((m_CoinVol[i]).GetVolume()==Vol)return &m_CoinVol[i];
  }
  return NULL;
}


////////////////////////////////////////////////////////////////////////////////


MNCTCoincidenceVolume* MNCTDetectorArray::GetAntiCoincidenceVolume(const MDVolume* Vol)
{
  for(unsigned int i=0;i<m_AntiCoinVol.size();i++)
  {
    if((m_AntiCoinVol[i]).GetVolume()==Vol)return &m_AntiCoinVol[i];
  }
  return NULL;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorArray::IsSensitive(const MVector& PositionInVolume)
{
  //Only for NCT Germanium Detector
  double x,y;
  double diagonal_criterion;
  double xy_criterion;
  
  x=fabs(PositionInVolume.GetX());
  y=fabs(PositionInVolume.GetY());
  
  diagonal_criterion = m_Length - m_CutLength - m_GuardringWidth*sqrt(2.0);
  xy_criterion = m_Length/2.0 - m_GuardringWidth;
  
  if(x <= xy_criterion && y <= xy_criterion && (x+y) <= diagonal_criterion)return true;
  else return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorArray::IsGuardring(const MVector& PositionInVolume)
{
  //Only for NCT Germanium Detector
  double x,y;
  double diagonal_criterion;
  double xy_criterion;
  
  x=fabs(PositionInVolume.GetX());
  y=fabs(PositionInVolume.GetY());
  
  diagonal_criterion = m_Length - m_CutLength - m_GuardringWidth*sqrt(2.0);
  xy_criterion = m_Length/2.0 - m_GuardringWidth;
  
  if(x > xy_criterion || y > xy_criterion || (x+y) > diagonal_criterion)return true;
  else return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorArray::IsNearGuardring(const MVector& PositionInVolume)
{
  //Only for NCT Germanium Detector
  double x,y;
  double diagonal_criterion;
  double xy_criterion;
  
  x=fabs(PositionInVolume.GetX());
  y=fabs(PositionInVolume.GetY());
  
  diagonal_criterion = m_Length - m_CutLength - (m_Pitch/2.0 + m_GuardringWidth)*sqrt(2.0);
  xy_criterion = m_Length/2.0 - m_GuardringWidth - m_Pitch/2.0;
  
  if(x > xy_criterion || y > xy_criterion || (x+y) > diagonal_criterion)return true;
  else return false;
}


////////////////////////////////////////////////////////////////////////////////



MNCTHitInVoxel MNCTDetectorArray::PositionInVolume2Voxel(int DetectorID, MVector PositionInVolume, double Energy)
{
  double x,y;
  MVector displace;
  MNCTHitInVoxel HitInVoxel;
  
  //  if(!IsSensitive(PositionInVolume)) return HitInVoxel;//debug
  
  HitInVoxel.SetDetectorID(DetectorID);
  
  x=PositionInVolume.GetX();
  y=PositionInVolume.GetY();
  
  if(IsXStripInverse(DetectorID))x=-x;
  if(IsYStripInverse(DetectorID))y=-y;
  if(!XYStripSwap(DetectorID)^IsXPositive(DetectorID)){  //x,y sides have been changed to +,- sides
    double tmp=x;
    x=y;
    y=tmp;
  }
  
  x += (m_Length/2.0-m_GuardringWidth);
  y += (m_Length/2.0-m_GuardringWidth);
  
  HitInVoxel.SetXStripID((int)floor(x/m_Pitch));
  HitInVoxel.SetYStripID((int)floor(y/m_Pitch));
  
  displace.SetX(fmod(x,m_Pitch) - m_Pitch/2.0);
  displace.SetY(fmod(y,m_Pitch) - m_Pitch/2.0);
  //cout << "posiX: " << x << ' ' << displace.GetX() << " energy=" << Energy <<'\n';//debug
  //cout << "posiX: " << y << ' ' << displace.GetY() << " energy=" << Energy <<'\n';//debug
  
  displace.SetZ(PositionInVolume.GetZ());
  HitInVoxel.SetDisplace(displace);
  
  HitInVoxel.SetEnergy(Energy);
  
  return HitInVoxel;
}


////////////////////////////////////////////////////////////////////////////////


MNCTDetectorArray::Detector* MNCTDetectorArray::FindID(int DetectorID)
{
  for(unsigned int i = 0; i < m_NCTDetectors.size(); ++i) {
    if (m_NCTDetectors[i].DetectorID == DetectorID) return &m_NCTDetectors[i];
  }
  return 0;
}


////////////////////////////////////////////////////////////////////////////////

/*
MNCTDetectorArray::Detector* MNCTDetectorArray::FindVolume(const MDVolume* DetectorVolume)
{
  for(unsigned int i=0;i<m_NCTDetectors.size();i++)
  {
    if(m_NCTDetectors[i].DetectorVolume==DetectorVolume)return &m_NCTDetectors[i];
  }
  return 0;
}
*/

////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::Load_Detector(MString filename)
{
  char buffer[512];
  string sbuffer;
  Detector tmp_det;
  
  string tmp_detectorname;
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);
  
  cout << "Loading detector file...\n";//check point
  cout << "filename: " << filename << '\n';
  if(!f1.is_open()){
    cerr << "file does not exist!!!\n";
    exit(1);
  }
  if(f1.eof()){
    cerr << "EOF!!!\n";//debug
    exit(1);
  }
  
  while(1)
  {
    f1.getline(buffer,512);
    if (f1.eof()) break;
    
    sbuffer=buffer;
    //cout << "#debug: " << sbuffer << '\n';//debug
    istringstream isbuffer(sbuffer);
    
    if (buffer[0] != '#') {
      tmp_det.Clear();
      isbuffer >> tmp_det.DetectorID
      >> tmp_det.DetectorName
      >> tmp_det.XStripInverse
      >> tmp_det.YStripInverse
      >> tmp_det.XYStripSwap
      >> tmp_det.XStripUp
      >> tmp_det.DetectorON
      >> tmp_det.XStripHighVoltage
      >> tmp_det.Voltage
      >> tmp_det.Impurity
      >> tmp_det.DeadRegion;
      
      if (tmp_det.DetectorID != -1) {
        m_NCTDetectors.push_back(tmp_det);
        //cout << ' ' << tmp_volumename << '\n';//debug
        m_DetectorNumber++;
      }
    }
    //cout << m_DetectorNumber;//debug
  }
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::Load_DeadStrip(MString filename)
{
  char buffer[512];
  string sbuffer;
  MNCTStrip tmp_strip;
  int detectorid;
  bool xstrip;
  int stripid;
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);
  
  cout << "Loading dead strip file...\n";//check point
  cout << "filename: " << filename << '\n';
  if(!f1.is_open()){
    cerr << "file does not exist!!!\n";
    exit(1);
  }
  if(f1.eof()){
    cerr << "EOF!!!\n";//debug
    exit(1);
  }
  
  while(1)
  {
    f1.getline(buffer,512);
    if(f1.eof())break;
    sbuffer=buffer;
    istringstream isbuffer(sbuffer);
    if(buffer[0]!='#')
    {
      tmp_strip.Clear();
      isbuffer >> detectorid
      >> xstrip
      >> stripid;
      
      tmp_strip.SetDetectorID(detectorid);
      tmp_strip.IsXStrip(xstrip);
      tmp_strip.SetStripID(stripid);
      
      if(tmp_strip.GetDetectorID()!=g_IntNotDefined)
      {
        m_DeadStrips.push_back(tmp_strip);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::Load_Coincidence(MString filename)
{
  char buffer[512];
  string sbuffer;
  MNCTCoincidenceVolume tmp_coin;
  string vol;
  double threshold;
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);
  
  cout << "Loading coincidence file...\n";//check point
  cout << "filename: " << filename << '\n';
  if(!f1.is_open()){
    cerr << "file does not exist!!!\n";
    exit(1);
  }
  if(f1.eof()){
    cerr << "EOF!!!\n";//debug
    exit(1);
  }
  
  while(1)
  {
    f1.getline(buffer,512);
    if(f1.eof())break;
    sbuffer=buffer;
    istringstream isbuffer(sbuffer);
    if(buffer[0]!='#')
    {
      tmp_coin.Clear();
      isbuffer >> vol
      >> threshold;
      
      tmp_coin.SetVolume(m_Geometry->MDGeometry::GetVolume(vol));
      tmp_coin.SetThreshold(threshold);
      
      if(tmp_coin.GetVolume()!=NULL)
      {
        m_CoinVol.push_back(tmp_coin);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::Load_AntiCoincidence(MString filename)
{
  char buffer[512];
  string sbuffer;
  MNCTCoincidenceVolume tmp_coin;
  string vol;
  double threshold;
  
  MFile::ExpandFileName(filename);
  ifstream f1(filename);
  
  cout << "Loading anti-coincidence file...\n";//check point
  cout << "filename: " << filename << '\n';
  if(!f1.is_open()){
    cerr << "file does not exist!!!\n";
    exit(1);
  }
  if(f1.eof()){
    cerr << "EOF!!!\n";//debug
    exit(1);
  }
  
  while(1)
  {
    f1.getline(buffer,512);
    if(f1.eof())break;
    sbuffer=buffer;
    istringstream isbuffer(sbuffer);
    if(buffer[0]!='#')
    {
      tmp_coin.Clear();
      isbuffer >> vol
      >> threshold;
      
      tmp_coin.SetVolume(m_Geometry->MDGeometry::GetVolume(vol));
      tmp_coin.SetThreshold(threshold);
      
      if(tmp_coin.GetVolume()!=NULL)
      {
        m_AntiCoinVol.push_back(tmp_coin);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::FindAllDetectorVolumes()
{
  MDVolume* WorldV = m_Geometry->GetWorldVolume();
  FindDetectorVolumes(WorldV);
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::FindDetectorVolumes(MDVolume* V)
{
  if(V->GetDetector() != 0) m_AllDetectorVolumes.push_back(V);
  if(V->GetNDaughters() > 0)
  {
    for(unsigned int i=0;i<V->GetNDaughters();i++)
    {
      FindDetectorVolumes(V->GetDaughterAt(i));
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


int MNCTDetectorArray::FindVolumeByName(MString name)
{
  for (unsigned int i = 0; i < m_AllDetectorVolumes.size(); ++i) {
    cout<<m_AllDetectorVolumes[i]->GetName()<<endl;
    if (m_AllDetectorVolumes[i]->GetName() == name) return i;
  }
  
  return -1;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::DumpParameters()
{
  cout << "\nDetector Parameters(defaults):\n"
  << " DetectorNumber=" << m_DetectorNumber << '\n'
  << " Width=" << m_Width << '\n'
  << " Length=" << m_Length << '\n'
  << " CutLength=" << m_CutLength << '\n'
  << " StripNumber=" << m_Strips << '\n'
  << " StripPitch=" << m_Pitch << '\n'
  << " StripGapWidth=" << m_Gap << '\n'
  << " GuardringWidth=" << m_GuardringWidth << "\n\n";
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::DumpDetectors()
{
  cout << "Detector detail information:\n";
  cout << "(ID Vol XInv YInv XYSwap XUp ON XHi HV Impur Dead)\n";
  for (unsigned int i = 0; i < m_NCTDetectors.size(); ++i) {
    cout << m_NCTDetectors[i].DetectorID << ' '
    << m_NCTDetectors[i].DetectorName << ' '
    << m_NCTDetectors[i].XStripInverse << ' '
    << m_NCTDetectors[i].YStripInverse << ' '
    << m_NCTDetectors[i].XYStripSwap << ' '
    << m_NCTDetectors[i].XStripUp << ' '
    << m_NCTDetectors[i].DetectorON << ' '
    << m_NCTDetectors[i].XStripHighVoltage << ' '
    << m_NCTDetectors[i].Voltage << ' '
    << m_NCTDetectors[i].Impurity << ' '
    << m_NCTDetectors[i].DeadRegion << '\n';
  }
  cout << '\n';
}


////////////////////////////////////////////////////////////////////////////////


//! Return the detector ID using the detector name
int MNCTDetectorArray::GetID(MString Name) const
{
  int ID = -1;
  for (unsigned int i = 0; i < m_NCTDetectors.size(); ++i) {
    if (m_NCTDetectors[i].DetectorName == Name) {
      ID = m_NCTDetectors[i].DetectorID;
      break;
    }
  }
  return ID;
}

  
////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::DumpDeadStrips()
{
  cout << "Dead Strips:\n";
  cout << "(ID +:1 Strip)\n";
  for(unsigned int i=0; i<m_DeadStrips.size(); i++)
  {
    cout << m_DeadStrips[i].GetDetectorID() << ' '
    << m_DeadStrips[i].IsXStrip() << ' '
    << m_DeadStrips[i].GetStripID() << '\n';
  }
  cout << '\n';
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::DumpCoincidence()
{
  cout << "Coincidence Volume:\n";
  cout << "(Volume Threshold)\n";
  for(unsigned int i=0; i<m_CoinVol.size(); i++)
  {
    cout << m_CoinVol[i].GetVolume()->GetName() << ' '
    << m_CoinVol[i].GetThreshold() << '\n';
  }
  cout << '\n';
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::DumpAntiCoincidence()
{
  cout << "Anti-coincidence Volume:\n";
  cout << "(Volume Threshold)\n";
  for(unsigned int i=0; i<m_AntiCoinVol.size(); i++)
  {
    cout << m_AntiCoinVol[i].GetVolume()->GetName() << ' '
    << m_AntiCoinVol[i].GetThreshold() << '\n';
  }
  cout << '\n';
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorArray::Detector::Clear()
{
  DetectorName = "";
  DetectorID = g_IntNotDefined;
  
  XStripInverse = false;
  YStripInverse = false;
  XYStripSwap = false;
  XStripUp = true;
  DetectorON = true;
  XStripHighVoltage = false;
  Voltage = g_DoubleNotDefined;
  Impurity = g_DoubleNotDefined;
  DeadRegion = g_DoubleNotDefined;
}


// MNCTDetectorArray.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
