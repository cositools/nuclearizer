/*
 * MNCTDetectorResponse.cxx
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
// MNCTDetectorResponse.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTDetectorResponse.h"

// Standard libs:
#include <vector>
#include <string>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MAssert.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTDetectorResponse.h)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTDetectorResponse::MNCTDetectorResponse()
{
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MNCTDetectorResponse::~MNCTDetectorResponse()
{
  DeleteResponseArray();
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorResponse::Activate()
{
  // Initialize the module

  if (m_IsNCTDetectorArraySet == false) return false;

  SetParameters();
  LoadResponseArray();//<--need check

  m_Depth2TArr_n=(*m_Depth2TArr)[0][0].size();
  m_E2ADCArr_n=(*m_E2ADCArr)[0][0][0].size();
  m_NoiseArr_n=(*m_NoiseArr)[0][0][0].size();
  m_nonlinearNoiseArr_n=(*m_NoiseArr)[0][0][0].size();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorResponse::Clear()
{
  
  //create and setup Random object
  Jiggle = new TRandom();
  NoiseError = new TRandom();

  m_IsNCTDetectorArraySet = false;
  m_IsParametersSet = false;
  m_IsResponseArrayLoaded = false;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorResponse::LoadResponseArray()
{
  //load response files
  if(m_IsNCTDetectorArraySet && m_IsParametersSet && !m_IsResponseArrayLoaded)
  {
    cout << "Loading response array files...\n";

    m_DCalArr=&MNCTArray::csv_loader4("$(NUCLEARIZER)/resource/dee/TCal_Data2Sim.csv",10,37,37,2,"many");
    m_Depth2TArr=&MNCTArray::csv_loader3("$(NUCLEARIZER)/resource/dee/TCal_Z2Time.csv",10,2,6,"many");
    m_E2ADCArr=&MNCTArray::csv_loader4("$(NUCLEARIZER)/resource/dee/ECal_NCT09_Linear.csv",10,2,37,2,"many");
    m_nonlinearE2ADCArr=&MNCTArray::csv_loader4("$(NUCLEARIZER)/resource/dee/ECal_NCT09_Nonlinear.csv",10,2,37,5,"many");
    
    m_FastArr= new double_array4(boost::extents[10][2][37][2]);//Just a dummy!!
    m_SlowArr= new double_array4(boost::extents[10][2][37][2]);//Just a dummy!!
//    m_NoiseArr= new double_array3(boost::extents[2][2][37]);//Just a dummy!!
    
//    m_FastArr=  &MNCTArray::Array_loader4("thresholds_fast.arr",2,2,37,2);
//    m_SlowArr=  &MNCTArray::Array_loader4("thresholds_slow.arr",2,2,37,2);
    m_NoiseArr= &MNCTArray::csv_loader4("$(NUCLEARIZER)/resource/dee/ECal_NCT09_Noise.csv",10,2,37,3,"many");
    m_nonlinearNoiseArr= &MNCTArray::csv_loader4("$(NUCLEARIZER)/resource/dee/ECal_NCT09_Nonlinear_Noise.csv",10,2,37,3,"many");

    //cout << "debug: " << (*m_NoiseArr)[2][1][1][0] << '\n'; //debug
    cout << '\n';

    m_IsNCTDetectorArraySet=true;
  }
  else
  {
    cerr << "Can't load response array files!!!\n";
    exit(1);
  }
} 


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorResponse::DeleteResponseArray()
{
  if(m_IsResponseArrayLoaded)
  {
    delete m_DCalArr;
    delete m_Depth2TArr;
    delete m_E2ADCArr;
    delete m_nonlinearE2ADCArr;
    delete m_FastArr;
    delete m_SlowArr;
    delete m_NoiseArr;
    delete m_nonlinearNoiseArr;

    m_IsResponseArrayLoaded=false;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MNCTDetectorResponse::SetParameters()
{
  if(m_IsNCTDetectorArraySet)
  {
    cout << "Setting Parameters...\n";
    m_DetN = m_NCTDetectors->GetDetectorNum();
    cout << "detnum=" << m_DetN << '\n';
    m_StripN = m_NCTDetectors->GetStrips();
    cout << "stripnum=" << m_StripN << '\n';

    cout << "calculating potential for energy sharing\n\n";
    for(int i=0;i<(int)m_DetN;i++)
    {
      double Vd = 127.06*m_NCTDetectors->GetImpurity(i);
      double HV = abs(m_NCTDetectors->GetVoltage(i));
      e0.push_back(2.*Vd/1.5);
      e1.push_back((HV-Vd)/1.5);
      //cout << e0[i] << ' ' << e1[i]<< '\n';//debug
    }
    //cout << '\n';
    
    m_IsParametersSet=true;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTDetectorResponse::GuardringTriger(double energy)
{
  double grthreshold=40.0;//<-need check
  return (energy>=grthreshold);
}


////////////////////////////////////////////////////////////////////////////////


vector<MNCTStripEnergyDepth> MNCTDetectorResponse::noEnergySharing(const MNCTHitInVoxel& HitInVoxel, bool IsXStrip)
{
  int det=HitInVoxel.GetDetectorID();
  int strip;
  //double dx;
  double z=HitInVoxel.GetDisplace().GetZ();
  double energy=HitInVoxel.GetEnergy();
  double depth;
  vector<MNCTStripEnergyDepth> StripEDs;
  
  if(IsXStrip == false )
  {
    //dx=HitInVoxel.GetDisplace().GetY();
    strip=HitInVoxel.GetYStripID();
  }
  else
  {
    //dx=HitInVoxel.GetDisplace().GetX();
    strip=HitInVoxel.GetXStripID();
  }

  depth=(m_NCTDetectors->IsZPositive(det))? z+0.75 : 1.5-(z+0.75);
  
  MNCTStrip Strip(det,IsXStrip,strip);
  MNCTStrip XStrip(det,true,HitInVoxel.GetXStripID());
  MNCTStrip YStrip(det,false,HitInVoxel.GetYStripID());
  //update depth to timing
  double timing=Depth2Time(Strip, depth);
  //cout << "timing: " << timing << '\n';//debug
  timing=Time2TriggerTime(XStrip, YStrip, IsXStrip, timing);

  //return
  MNCTStripEnergyDepth sed_tmp(Strip,energy,timing);
  StripEDs.push_back(sed_tmp);

  return StripEDs;
}
  

////////////////////////////////////////////////////////////////////////////////


vector<MNCTStripEnergyDepth> MNCTDetectorResponse::EnergySharing(const MNCTHitInVoxel& HitInVoxel, bool IsXStrip)
{
  //Original IDL code: Zong-Kai
  //Date: May-15-2008

  //xpos: the posision relative to the center of strip to x or y direction
  //z: ??
  //xset=false :Y , xset=true :X, det=detector number(0/1) , 
  //flag: return energy on left strip or right strip (false:left,true:right)
  //can not apply on sharing of gaurdring and nomal strip

  int det=HitInVoxel.GetDetectorID();
  int strip;
  double dx;
  double z=HitInVoxel.GetDisplace().GetZ();
  double energy=HitInVoxel.GetEnergy();
  double depth=0,depth2=0;
  vector<MNCTStripEnergyDepth> StripEDs;

  
  if(IsXStrip == false)//<==x,y has been changed to +,- sides
  {
    dx=HitInVoxel.GetDisplace().GetY();
    strip=HitInVoxel.GetYStripID();
  }
  else
  {
    dx=HitInVoxel.GetDisplace().GetX();
    strip=HitInVoxel.GetXStripID();
  }

  //depth=distance to -side
  depth=(m_NCTDetectors->IsZPositive(det))? z+0.75 : 1.5-(z+0.75);
  
  MNCTStrip lstrip(det,IsXStrip,strip);
  MNCTStrip rstrip(det,IsXStrip,strip);

  double xtmp;
  if(dx > 0) 
  {
    xtmp=dx - 0.1;
    rstrip.SetStripID(strip+1);
  }
  else 
  {
    xtmp=dx + 0.1;
    lstrip.SetStripID(strip-1);
  }

  if(!IsXStrip)
  {//-side,h+
    depth2=(1.5/e0[det])*MNCTMath::alog10((e0[det]+e1[det])/(e0[det]*(1.-depth/1.5)+e1[det]));
  }
  else
  {//+side,e-
    depth2=-1.*(1.5/e0[det])*MNCTMath::alog10((e1[det])/(e0[det]*(1.-depth/1.5)+e1[det]));
  }
  
  //Parameters
  double rep_c=1.0765e-2;//It seems the same for each detector
  double dif_c=0.121158;//at 85K, (2*0.0253*T/293)^0.5
  double wf_mid=0.915038/2.;
  double xedge=0.0;//edge of strip [cm]
  double xdead=0.0;//dead region (charge losing)
  double sigmin=0.002;//~100 keV recoil electron range [cm]

  double sigma_rep = rep_c*pow((depth2*energy),(1./3.));
  double sigma_dif = dif_c*pow(depth2,0.5);
  double xsigma = sqrt(pow(sigmin,2.0) + pow(sigma_dif,2.0)+ pow(sigma_rep,2.0));

  double gxdum1=MNCTMath::gaussint((xedge-xdead/2.-xtmp)/xsigma);
  double gxdum2=MNCTMath::gaussint((xedge+xdead/2.-xtmp)/xsigma);
  
  double engl=0.0, engr=0.0;
  engl = energy*(gxdum1 + wf_mid*(gxdum2-gxdum1));
  engr = energy*(1.-gxdum2 + wf_mid*(gxdum2-gxdum1));

  //cout << dx << ' ' << xtmp << ' ' << xsigma << ' ' << energy << '\n';//debug
  //cout << gxdum1 << ' ' << gxdum2 << ' ' << engl << ' ' << engr << '\n';//debug
  //cout << energy << " -> " << engl << ' ' << engr << '\n';//debug

  //update depth to timing
  MNCTStrip Strip(det,IsXStrip,strip);
  MNCTStrip XStrip(det,true,HitInVoxel.GetXStripID());
  MNCTStrip YStrip(det,false,HitInVoxel.GetYStripID());
  double timing=Depth2Time(Strip, depth);
  //cout << "timing: " << timing << '\n';//debug
  timing=Time2TriggerTime(XStrip, YStrip, IsXStrip, timing);

  //return
  MNCTStripEnergyDepth sed_tmpl(lstrip,engl,timing);
  MNCTStripEnergyDepth sed_tmpr(rstrip,engr,timing);
  StripEDs.push_back(sed_tmpl);
  StripEDs.push_back(sed_tmpr);

  return StripEDs;
}


////////////////////////////////////////////////////////////////////////////////


double MNCTDetectorResponse::Depth2Time(MNCTStrip Strip, double depth)
{
  double time=0;
  for(int i=0; i<m_Depth2TArr_n; i++){
    time+=(*m_Depth2TArr)[Strip.GetDetectorID()][Strip.IsXStrip()][i]*pow(depth,(double)i);
  }
  return time;
}


////////////////////////////////////////////////////////////////////////////////


int MNCTDetectorResponse::Time2TriggerTime(MNCTStrip XStrip, MNCTStrip YStrip, bool Xstrip, double time)
{
  double trigger_time=0;
  if (Xstrip){
    trigger_time = (*m_DCalArr)[XStrip.GetDetectorID()][XStrip.GetStripID()][YStrip.GetStripID()][1]*time + 0.5*(*m_DCalArr)[XStrip.GetDetectorID()][XStrip.GetStripID()][YStrip.GetStripID()][0];
  }else{
    trigger_time = (*m_DCalArr)[XStrip.GetDetectorID()][XStrip.GetStripID()][YStrip.GetStripID()][1]*time - 0.5*(*m_DCalArr)[XStrip.GetDetectorID()][XStrip.GetStripID()][YStrip.GetStripID()][0];
  }

  return (int)trigger_time;
}


////////////////////////////////////////////////////////////////////////////////


double MNCTDetectorResponse::EnergyNoise(double energy, MNCTStrip Strip)
{
  int det=Strip.GetDetectorID();
  int xset=Strip.IsXStrip();
  int channel=Strip.GetStripID();
  double_array4& NoiseArr=*m_NoiseArr;

//  double fano=0.13;//###
//  double ep=0.00298;//###
//  double p=NoiseArr[det][xset][channel];
  double xmond=0;
  for(int power=0; power < m_NoiseArr_n; power++){
    xmond+=NoiseArr[det][xset][channel][power]*pow(energy,(double)power);
  }
 
  //double xmond=sqrt(1.*p*p + fano*ep*energy)/2.35;//1-sigma error from noise and random fluctuation
  return NoiseError->Gaus(0,sqrt(xmond)*MNCTMath::FWHM2sigma());
  //return NoiseError->Gaus(0,2.5);
}


////////////////////////////////////////////////////////////////////////////////


int MNCTDetectorResponse::Energy2ADC(double energy, MNCTStrip Strip)
{
  int det=Strip.GetDetectorID();
  int xset=Strip.IsXStrip();
  int channel=Strip.GetStripID();
  double_array4& E2ADCArr=*m_E2ADCArr;
  double term=0.;

  for(int power=0; power < m_E2ADCArr_n; power++){
    term+=E2ADCArr[det][xset][channel][power]*pow(energy,(double)power);
  }

  int adc = (int)(term + Jiggle->Uniform());
  return adc;
}


////////////////////////////////////////////////////////////////////////////////


double MNCTDetectorResponse::NonlinearEnergyNoise(double energy, MNCTStrip Strip)
{
  int det=Strip.GetDetectorID();
  int xset=Strip.IsXStrip();
  int channel=Strip.GetStripID();
  double_array4& NoiseArr=*m_nonlinearNoiseArr;

  double xmond=0;
  for(int power=0; power < m_nonlinearNoiseArr_n; power++){
    xmond+=pow(NoiseArr[det][xset][channel][power],2.0)*pow(energy,(double)power);
  }
 
  return NoiseError->Gaus(0,sqrt(xmond)*MNCTMath::FWHM2sigma());
}


////////////////////////////////////////////////////////////////////////////////


int MNCTDetectorResponse::NonlinearEnergy2ADC(double energy, MNCTStrip Strip)
{
  int det=Strip.GetDetectorID();
  int xset=Strip.IsXStrip();
  int channel=Strip.GetStripID();
  double_array4& E2ADCArr=*m_nonlinearE2ADCArr;
  double term0=0., sum=0.;
  double a[5];

  for(int i=0; i < 5; i++){//accept only 5 parameters
    a[i]=E2ADCArr[det][xset][channel][i];
  }
  
  term0 = (energy - a[0])/a[1];
  sum = term0;
  for(int i=1; i<=2; i++){//2nd order
    sum = term0 - delta(sum,a[2],a[3],a[4])/a[1];
  }

  int adc = (int)(sum + Jiggle->Uniform());
  return adc;
}


////////////////////////////////////////////////////////////////////////////////


double MNCTDetectorResponse::delta(double x, double a2, double a3, double a4)
{
  return a2*x*x+a3*exp(-pow(x/a4,3.0));
}


////////////////////////////////////////////////////////////////////////////////


double MNCTDetectorResponse::FastThreshold(MNCTStrip Strip)
{
  //int det=Strip.GetDetectorID();
  //int xstrip=Strip.IsXStrip();
  //int chan=Strip.GetStripID();
  //double_array4& FastArr=*m_FastArr;

// double fast=NoiseError->Gaus(FastArr[det][xstrip][chan][0],FastArr[det][xstrip][chan][1]);
  double fast=32.0;//default value
  return fast;
}


////////////////////////////////////////////////////////////////////////////////


double MNCTDetectorResponse::SlowThreshold(MNCTStrip Strip)
{
  //int det=Strip.GetDetectorID();
  //int xstrip=Strip.IsXStrip();
  //int chan=Strip.GetStripID();
  //double_array4& SlowArr=*m_SlowArr;

//  double slow=NoiseError->Gaus(SlowArr[det][xstrip][chan][0],SlowArr[det][xstrip][chan][1]);
  double slow=18.0;//default value
  return slow;
}


// MNCTDetectorResponse.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
