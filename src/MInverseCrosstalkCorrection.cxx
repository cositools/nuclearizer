/*
 * MInverseCrosstalkCorrection.cxx
 *
 *
 * Copyright (C) 2009-2009 by Mark Bandstra.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Mark Bandstra.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MInverseCrosstalkCorrection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MInverseCrosstalkCorrection.h"

// Standard libs:
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

// ROOT libs:
#include "TGClient.h"
#include "TFile.h"
#include "TMatrixD.h"

// MEGAlib libs:
//#include "MMath.h"
#include "MStreams.h"
#include "MVector.h"
#include "MString.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MInverseCrosstalkCorrection)
#endif


////////////////////////////////////////////////////////////////////////////////


MInverseCrosstalkCorrection::MInverseCrosstalkCorrection()
{
  // Construct an instance of MInverseCrosstalkCorrection
}


////////////////////////////////////////////////////////////////////////////////


MInverseCrosstalkCorrection::~MInverseCrosstalkCorrection()
{
  // Delete this instance of MInverseCrosstalkCorrection
}


////////////////////////////////////////////////////////////////////////////////


bool MInverseCrosstalkCorrection::Initialize()
{
  // Initialize the module

  for (int DetectorNumber=0; DetectorNumber<10; DetectorNumber++)
    {
      mout << "Attempting to load energy crosstalk data for D" << DetectorNumber << endl;

      // Construct the filename of the detector-specific calibration file
      string DetectorNumberString;
      stringstream temp;
      temp << setfill('0') << setw(1) << DetectorNumber;
      DetectorNumberString = temp.str();
      MString FileName = (MString)std::getenv ("NUCLEARIZER_CAL")
	+"/crosstalk_D"+ DetectorNumberString + ".csv";

      // Set calibration to default
      for (int i=0; i<=1; i++)
	{
	  for (unsigned int j=0; j<=MAXNSKIP; j++)
	    {
	      m_CrosstalkCoeffs[DetectorNumber][i][j][0] = 0.;
	      m_CrosstalkCoeffs[DetectorNumber][i][j][1] = 0.;
	    }
	}

      // Reset flags telling if calibration has been loaded
      for (unsigned int i=0; i<=MAXNSKIP; i++)
	{
	  m_IsCalibrationLoaded[DetectorNumber][0][i] = false;
	  m_IsCalibrationLoaded[DetectorNumber][1][i] = false;
	}
      
      // Read the calibration coefficients line-by-line
      fstream File;
      File.open(FileName, ios_base::in);
      if (File.is_open() == false)
	{
	  mout<<"***Warning: Unable to open file: "<<FileName<<endl
	      <<"   Is your NUCLEARIZER_CAL environment variable set?"<<endl;
	} 
      else
	{
	  MString Line;
	  while(!File.eof())
	    {
	      Line.ReadLine(File);
	      if (Line.BeginsWith("#") == false)
		{
		  //mout << Line << endl;
		  char PosNeg;
		  int SideNumber, NSkip;
		  double a, b;
		  if (sscanf(Line.Data(), "%c,%d,%lf,%lf\n",
			     &PosNeg,&NSkip,&a,&b) == 4)
		    {
		      //mout << PosNeg << " " << NSkip << " " << a << " " << b << endl;
		      if (PosNeg == '+') { SideNumber = 0; } else { SideNumber = 1; }
		      if ((0<=NSkip) and (NSkip<=MAXNSKIP))
			{
			  m_CrosstalkCoeffs[DetectorNumber][SideNumber][NSkip][0] = a;
			  m_CrosstalkCoeffs[DetectorNumber][SideNumber][NSkip][1] = b/1333.;
			  m_IsCalibrationLoaded[DetectorNumber][SideNumber][NSkip] = true;
			}
		    }
		}
	    }
	}  // done reading from file

      // Check if data has been properly loaded
      m_IsCalibrationLoadedDet[DetectorNumber] = true;
      for (int side=0; side<2; side++)
	{
	  for (unsigned int nskip=0; nskip<=MAXNSKIP; nskip++)
	    {
	      if (m_IsCalibrationLoaded[DetectorNumber][side][nskip]==false)
		{
		  m_IsCalibrationLoadedDet[DetectorNumber] = false;
		}
	    }
	}
      if (m_IsCalibrationLoadedDet[DetectorNumber] == true)
	{
	  mout << "Cross-talk data for D" << DetectorNumber << " successfully loaded!" << endl;
	}
      else
	{
	  mout << "***Warning: Unable to fully load cross-talk data for D" 
	       << DetectorNumber << ".  Defaults were used." << endl;
	  // Set calibration to default
	  for (int i=0; i<=1; i++)
	    {
	      for (int j=0; i<MAXNSKIP; i++)
		{
		  m_CrosstalkCoeffs[DetectorNumber][i][j][0] = 0.;
		  m_CrosstalkCoeffs[DetectorNumber][i][j][1] = 0.;
		}
	    }
	}

    } // 'DetectorNumber' loop

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool compare_striphits2(MStripHit* SH1, MStripHit* SH2)
{
  int det1 = SH1->GetDetectorID();
  int det2 = SH2->GetDetectorID();
  int side1 = 0, side2 = 0;
  int strip1 = SH1->GetStripID();
  int strip2 = SH2->GetStripID();
  if (not SH1->IsLowVoltageStrip()) side1 = 1;
  if (not SH2->IsLowVoltageStrip()) side2 = 1;
  if (det1 != det2) return (det1<det2);
  else if ((det1==det2) && (side1!=side2)) return (side1<side2);
  else if ((det1==det2) && (side1==side2)) return (strip1<strip2);
  else
    {
      mout << "############# Problem in compare_striphits!" << endl;
      return false;
    }
}


////////////////////////////////////////////////////////////////////////////////

/*
bool MInverseCrosstalkCorrection::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 
  
  unsigned int NStripHits = Event->GetNStripHits();
  vector<MStripHit*> StripHits;
  bool debug=false;

  if (debug)
    {
      mout << endl;
      mout << "#######################################" << endl;
      mout << "Event " << Event->GetID() << endl;
    }
  // Loop over all detectors
  for (int i_det=0; i_det<10; i_det++)
    {
      // Loop over detector sides
      for (unsigned int i_side=0; i_side<=1; i_side++)
	{
	  StripHits.clear();
	  // Extract strip hits from the given side of the given detector
	  for (unsigned int i_sh=0; i_sh<NStripHits; i_sh++)
	    {
	      // Count number of X and Y strips
	      MStripHit *SH = Event->GetStripHit(i_sh);
	      if ((SH->GetDetectorID()==i_det) 
		  && (((SH->IsLowVoltageStrip()==true) && (i_side==0)) 
		      || ((SH->IsLowVoltageStrip()==false) && (i_side==1))))
		{
		  StripHits.push_back(SH);
		}
	    }
	  if (StripHits.size()>=2)
	    {
	      // Perform the cross-talk correction!
	      ApplyCrosstalk(StripHits, i_det, i_side);
	    }
	}
    }
  return true;
}
*/

////////////////////////////////////////////////////////////////////////////////


  // Method to make the cross-talk correction on a vector of strip hits
  // StripHits is a vector of StripHits from one side of one detector
void MInverseCrosstalkCorrection::ApplyCrosstalk(vector<MStripHit*> StripHits, 
						     int det, unsigned int iside)
{
  bool debug=false;
  bool debug_matrices=false;
  const double a_threshold=32.0;
  unsigned int N = StripHits.size();
  unsigned int side=!iside;//side definition is different from other place....
  // Sort the strip hits
  sort(StripHits.begin(), StripHits.end(), compare_striphits2);
  // Cross-talk coefficients
  double a0 = m_CrosstalkCoeffs[det][side][0][0];
  double b0 = m_CrosstalkCoeffs[det][side][0][1];
  double a1 = m_CrosstalkCoeffs[det][side][1][0];
  double b1 = m_CrosstalkCoeffs[det][side][1][1];
  double a2 = m_CrosstalkCoeffs[det][side][2][0];
  double b2 = m_CrosstalkCoeffs[det][side][2][1];

  // Print out the strips, check their order
  if (debug && StripHits.size()>0)
    {
      mout << "++++++++++++++++++++++" << endl;
      for (unsigned int j=0; j<StripHits.size(); j++)
	{
	  mout << StripHits[j]->GetDetectorID() << " "
	       << !StripHits[j]->IsLowVoltageStrip() << " "
	       << StripHits[j]->GetStripID() << " "
	       << StripHits[j]->GetEnergy() << endl;
	}
    }

  // Make a matrix vector of energies from each strip
  TMatrixD Energies(N,1);
  double Energy_Total = 0.;
  for (unsigned int j=0; j<StripHits.size(); j++)
    {
      Energies[j][0] = StripHits[j]->GetEnergy();
      Energy_Total += StripHits[j]->GetEnergy();
    }
  if (debug_matrices && N>=2) Energies.Print();

  // Make a big matrix for the cross-talk corrections
  TMatrixD Matrix(N,N);
  TMatrixD Constant(N,1);
  for (unsigned int i=0; i<StripHits.size(); i++)
    {
      for (unsigned int j=i; j<StripHits.size(); j++)
	{
	  // Self-contribution
	  if (i==j)
	    {
	      Matrix[i][j] += 1.0;
	    }
	  // Nearest-neighbor contributions
	  if (StripHits[j]->GetStripID()==(StripHits[i]->GetStripID()+1))
	    {
	      Matrix[i][j] += b0;
	      Matrix[j][i] += b0;
	      if(StripHits[j]->GetEnergy()>a_threshold && StripHits[i]->GetEnergy()>a_threshold)
	      {
	        Constant[i] += a0/2.;
	        Constant[j] += a0/2.;
	      }
	    }
	  // Skip-1 neighbor contributions
	  if (StripHits[j]->GetStripID()==(StripHits[i]->GetStripID()+2))
	    {
	      Matrix[i][j] += b1;
	      Matrix[j][i] += b1;
	      if(StripHits[j]->GetEnergy()>a_threshold && StripHits[i]->GetEnergy()>a_threshold)
	      {
	        Constant[i] += a1/2.;
	        Constant[j] += a1/2.;
	      }
	    }
	  // Skip-2 neighbor contributions
	  if (StripHits[j]->GetStripID()==(StripHits[i]->GetStripID()+3))
	    {
	      Matrix[i][j] += b2;
	      Matrix[j][i] += b2;
	      if(StripHits[j]->GetEnergy()>a_threshold && StripHits[i]->GetEnergy()>a_threshold)
	      {
	        Constant[i] += a2/2.;
	        Constant[j] += a2/2.;
	      }
	    }
	}
    }
  if (debug_matrices && N>=2) Constant.Print();
  if (debug_matrices && N>=2) Matrix.Print();
  //  TMatrixD Inv = Matrix.Invert();
  //if (debug_matrices && N>=2) Inv.Print();
  // Calculate final energies with cross-talk
  TMatrixD FinalEnergies = TMatrixD(Matrix, TMatrixD::kMult, Energies) + Constant;
  if (debug_matrices && N>=2) FinalEnergies.Print();
  for (unsigned int j=0; j<StripHits.size(); j++)
    {
      StripHits[j]->SetEnergy(FinalEnergies[j][0]);
    }


  // Print out the strips again, check their order and energies
  if (debug && StripHits.size()>0)
    {
      mout << "----------------------" << endl;
      for (unsigned int j=0; j<StripHits.size(); j++)
	{
	  mout << StripHits[j]->GetDetectorID() << " "
	       << !StripHits[j]->IsLowVoltageStrip() << " "
	       << StripHits[j]->GetStripID() << " "
	       << StripHits[j]->GetEnergy() << endl;
	}
      mout << "++++++++++++++++++++++" << endl;
    }
}



// MInverseCrosstalkCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
