/*
 * MNCTModuleCrosstalkCorrection.cxx
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
// MNCTModuleCrosstalkCorrection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTModuleCrosstalkCorrection.h"
#include "MGUIOptionsCrosstalkCorrection.h"

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
#include "MStreams.h"
#include "MVector.h"
#include "MString.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTModuleCrosstalkCorrection)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTModuleCrosstalkCorrection::MNCTModuleCrosstalkCorrection() : MModule()
{
  // Construct an instance of MNCTModuleCrosstalkCorrection
  
  // Set all module relevant information
  
  // Set the module name --- has to be unique
  m_Name = "Cross-talk energy correction";
  
  // Set the XML tag --- has to be unique --- no spaces allowed
  m_XmlTag = "CrosstalkCorrection";
  
  // Set all modules, which have to be done before this module
  AddPreceedingModuleType(MAssembly::c_EventLoader);
  AddPreceedingModuleType(MAssembly::c_EnergyCalibration);
  
  // Set all types this modules handles
  AddModuleType(MAssembly::c_CrosstalkCorrection);
  
  // Set all modules, which can follow this module
	AddSucceedingModuleType(MAssembly::c_NoRestriction); 
 
  // Set if this module has an options GUI
  // If true, overwrite ShowOptionsGUI() with the call to the GUI!
  m_HasOptionsGUI = true;
// If true, you have to derive a class from MGUIOptions (use MGUIOptionsTemplate)
  // and implement all your GUI options
  
  // Allow the use of multiple threads and instances
  m_AllowMultiThreading = true;
  m_AllowMultipleInstances = true;
}


////////////////////////////////////////////////////////////////////////////////


MNCTModuleCrosstalkCorrection::~MNCTModuleCrosstalkCorrection()
{
  // Delete this instance of MNCTModuleCrosstalkCorrection
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTModuleCrosstalkCorrection::Initialize()
{
  // Initialize the module


	
  
  //for (int DetectorNumber=0; DetectorNumber<10; DetectorNumber++) {
   // mout << "Attempting to load energy crosstalk data for D" << DetectorNumber << endl;
    
    // Construct the filename of the detector-specific calibration file
   // string DetectorNumberString;
   // stringstream temp;
   // temp << setfill('0') << setw(1) << DetectorNumber;
   // DetectorNumberString = temp.str();
   // MString FileName = (MString)std::getenv ("NUCLEARIZER_CAL")
   // +"/crosstalk_D"+ DetectorNumberString + ".csv";
    
    // Set calibration to default
	for (int detnum=0; detnum < 12; detnum++) {
		for (int side = 0; side <= 1; side++) {
			for (unsigned int skip = 0; skip <= 2; skip++) {
				m_CrosstalkCoeffs[detnum][side][skip][0] = 0.;
        m_CrosstalkCoeffs[detnum][side][skip][1] = 0.;
      }
    }
	}


    // Reset flags telling if calibration has been loaded
	for (int detnum=0; detnum < 12; detnum++) {
		for (unsigned int skip=0; skip <= 2; skip++) {
			m_IsCalibrationLoaded[detnum][0][skip] = false;
			m_IsCalibrationLoaded[detnum][1][skip] = false;
		}
	}


//	Read in the file from the Nuclearizer GUI
	fstream File;
	File.open(m_FileName, ios_base::in);
	// Read the calibration coefficients line-by-line
	if (File.is_open() == false) {
		mout<<"***Warning: Unable to open file for crosstalk calibration"<<endl;	
	} else {
	
		MString Line;
		while(!File.eof()) {
			Line.ReadLine(File);
			if (Line.BeginsWith("#") == false) {
				//mout << Line << endl;
				int DetNum, PosSide;
        int NSkip;
        double a0, a1;
        if (sscanf(Line.Data(), "%d %d %d %lf %lf\n", &DetNum,&PosSide,&NSkip,&a0,&a1) == 5) {
					//mout << DetNum <<" "<< PosSide << " " << NSkip << " " << a0 << " " << a1 << endl;
          m_CrosstalkCoeffs[DetNum][PosSide][NSkip][0] = a0;
          m_CrosstalkCoeffs[DetNum][PosSide][NSkip][1] = a1;
          m_IsCalibrationLoaded[DetNum][PosSide][NSkip] = true;

        }
			}
    }
  }  // done reading from file



  // Check if data has been properly loaded
	for (int detnum = 0; detnum < 12; detnum++) {
		m_IsCalibrationLoadedDet[detnum] = true;
 		for (int side=0; side<2; side++) {
			for (unsigned int nskip=0; nskip<=2; nskip++) {
				if (m_IsCalibrationLoaded[detnum][side][nskip]==false) {
					m_IsCalibrationLoadedDet[detnum] = false;
				}
			}
		}

  	if (m_IsCalibrationLoadedDet[detnum] == true)	{
			//mout << "Cross-talk data for D" << detnum << " successfully loaded!" << endl;
  	} else {
			mout << "***Warning: Unable to fully load cross-talk data for D" << detnum << ".  Defaults were used." << endl;
    	// Set calibration to default
			for (int i=0; i<=1; i++) {
				for (int j=0; j<=2; j++) {
					m_CrosstalkCoeffs[detnum][i][j][0] = 0.;
        	m_CrosstalkCoeffs[detnum][i][j][1] = 0.;
      	}
			}
		}
	} // 'DetectorNumber' loop
  
  
  return MModule::Initialize();
}


////////////////////////////////////////////////////////////////////////////////


bool compare_striphits(MNCTStripHit* SH1, MNCTStripHit* SH2)
{
  int det1 = SH1->GetDetectorID();
  int det2 = SH2->GetDetectorID();
  int side1 = 0, side2 = 0;
  int strip1 = SH1->GetStripID();
  int strip2 = SH2->GetStripID();
  if (not SH1->IsXStrip()) side1 = 1;
  if (not SH2->IsXStrip()) side2 = 1;
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


bool MNCTModuleCrosstalkCorrection::AnalyzeEvent(MReadOutAssembly* Event) 
{
  // Main data analysis routine, which updates the event to a new level 
  
 //for (unsigned int sh=0; sh < Event->GetNHits(); sh++) {
 	unsigned int NStripHits = Event->GetNStripHits();
  vector<MNCTStripHit*> StripHits;
  bool debug=false;
  
  if (debug)
  {
    mout << endl;
    mout << "#######################################" << endl;
    mout << "Event " << Event->GetID() << endl;
  }
  // Loop over all detectors
  for (int i_det=0; i_det<12; i_det++)
  {
    // Loop over detector sides
    for (unsigned int i_side=0; i_side<=1; i_side++)
    {
      StripHits.clear();
      // Extract strip hits from the given side of the given detector
      for (unsigned int i_sh=0; i_sh<NStripHits; i_sh++)
      {
        // Count number of X and Y strips
        MNCTStripHit *SH = Event->GetStripHit(i_sh);
        if ((SH->GetDetectorID()==i_det) 
          && (((SH->IsXStrip()==true) && (i_side==0)) 
          || ((SH->IsXStrip()==false) && (i_side==1))))
        {
          StripHits.push_back(SH);
        }
      }
      if (StripHits.size()>=2)
      {
        // Perform the cross-talk correction!
        CorrectCrosstalk(StripHits, i_det, i_side);
      }
    }
  }
	
	
  // Remove any strips that have negative energy after the correction FROM the Hits -- we still keep them around
  // in the strip hits
  const double RemovalEneryLimit = 5.0;
  for (unsigned int h = 0; h < Event->GetNHits(); ++h) {
    unsigned int i = 0;
    while (i < Event->GetHit(h)->GetNStripHits()) {
      if (Event->GetHit(h)->GetStripHit(i)->GetEnergy() <= RemovalEneryLimit) {
        if (g_Verbosity >= c_Info) {
          mout << "Removing strip hit " << i << " for having low or negative energy: " 
               << Event->GetHit(h)->GetStripHit(i)->GetEnergy() << " keV" << endl;
        }
        Event->GetHit(h)->RemoveStripHit(i);
      } else {  
        ++i;
      }
    }
  }
  // Now remove hits with no strip hits
  unsigned int hit = 0;
  while (hit < Event->GetNHits()) {
    if (Event->GetHit(hit)->GetNStripHits() == 0) {
      Event->RemoveHit(hit); 
    } else {
      ++hit;
    }
  }
  
  // Recalculate the total hit energy now the the individual strips have been corrected.
  for (unsigned int sh = 0; sh < Event->GetNHits(); sh++) {
    double energy = 0;
  	//if y strip was hit multiple times, need to use the x energy
		if (Event->GetHit(sh)->GetStripHitMultipleTimesY()){
			for (unsigned int sh_i = 0; sh_i < Event->GetHit(sh)->GetNStripHits(); sh_i++){
				if (Event->GetHit(sh)->GetStripHit(sh_i)->IsXStrip() == true){
					energy = energy + Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergy();
				}
			}
		}
		else{
	  	for (unsigned int sh_i = 0; sh_i < Event->GetHit(sh)->GetNStripHits(); sh_i++) {	
	      //for now, just define the hit energy as the sum of the y strip hits. This could later be modifided to take an average of the two sides.
	      if (Event->GetHit(sh)->GetStripHit(sh_i)->IsXStrip() == false) {
	        energy = energy + Event->GetHit(sh)->GetStripHit(sh_i)->GetEnergy();
	      }
	    }
		}

    Event->GetHit(sh)->SetEnergy(energy);
  }

  Event->SetAnalysisProgress(MAssembly::c_CrosstalkCorrection);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


// Method to make the cross-talk correction on a vector of strip hits
// StripHits is a vector of StripHits from one side of one detector
void MNCTModuleCrosstalkCorrection::CorrectCrosstalk(vector<MNCTStripHit*> StripHits, 
                                                     int det, unsigned int side)
{
  bool debug=false;
  bool debug_matrices=false;
  unsigned int N = StripHits.size();
  // Sort the strip hits
  sort(StripHits.begin(), StripHits.end(), compare_striphits);
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
      << !StripHits[j]->IsXStrip() << " "
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
        Constant[i] += a0/2.;
        Constant[j] += a0/2.;
      }
      // Skip-1 neighbor contributions
      if (StripHits[j]->GetStripID()==(StripHits[i]->GetStripID()+2))
      {
        Matrix[i][j] += b1;
        Matrix[j][i] += b1;
        Constant[i] += a1/2.;
        Constant[j] += a1/2.;
      }
      // Skip-2 neighbor contributions
      if (StripHits[j]->GetStripID()==(StripHits[i]->GetStripID()+3))
      {
        Matrix[i][j] += b2;
        Matrix[j][i] += b2;
        Constant[i] += a2/2.;
        Constant[j] += a2/2.;
      }
    }
  }
  if (debug_matrices && N>=2) Constant.Print();
  if (debug_matrices && N>=2) Matrix.Print();
  TMatrixD Inv = Matrix.Invert();
  if (debug_matrices && N>=2) Inv.Print();
  // Calculate final corrected energies
  TMatrixD FinalEnergies = TMatrixD(Inv,TMatrixD::kMult,Energies+Constant); //ck changed this to Energies + Constant
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
      << !StripHits[j]->IsXStrip() << " "
      << StripHits[j]->GetStripID() << " "
      << StripHits[j]->GetEnergy() << endl;
    }
    mout << "++++++++++++++++++++++" << endl;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MNCTModuleCrosstalkCorrection::ShowOptionsGUI()
{
  // Show the options GUI

  MGUIOptionsCrosstalkCorrection* Options = new MGUIOptionsCrosstalkCorrection(this);
  Options->Create();
  gClient->WaitForUnmap(Options);
}

////////////////////////////////////////////////////////////////////////////////

bool MNCTModuleCrosstalkCorrection::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  MXmlNode* FileNameNode = Node->GetNode("FileName");
  if (FileNameNode != 0) {
	m_FileName = FileNameNode->GetValue();
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

MXmlNode* MNCTModuleCrosstalkCorrection::CreateXmlConfiguration()
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);
  new MXmlNode(Node, "FileName", m_FileName);

  return Node;

}


// MNCTModuleCrosstalkCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
