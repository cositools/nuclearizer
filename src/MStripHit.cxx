/*
 * MStripHit.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MStripHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MStripHit.h"

// Standard libs:
#include <iomanip>
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MStripHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MStripHit::MStripHit()
{
  // Construct an instance of MStripHit

  m_ReadOutElement = new MReadOutElementDoubleStrip();
  
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MStripHit::~MStripHit()
{
  // Delete this instance of MStripHit
  
  delete m_ReadOutElement;
}


////////////////////////////////////////////////////////////////////////////////


void MStripHit::Clear()
{
  // Reset all data

  m_ReadOutElement->Clear();
  m_HasTriggered = false;
  m_UncorrectedADCUnits = 0;
  m_ADCUnits = 0;
  m_Energy = 0;
  m_EnergyResolution = 0;
  m_Timing = 0;
  m_PreampTemp = 0;
  m_Origins.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MStripHit::Parse(MString& Line, int Version)
{
	const char* line = Line.Data();
	if( line[0] == 'S' && line[1] == 'H' ){
		int det_id, strip_id, has_triggered, timing, un_adc, adc;
		float energy, energy_res;
		char pos_strip;
		sscanf(&line[3],"%d %c %d %d %d %d %d %f %f",&det_id,
			  															   &pos_strip,
																			&strip_id,
																			&has_triggered,
																			&timing,
																			&un_adc,
																			&adc,
																			&energy,
																			&energy_res);
		SetDetectorID(det_id);
		pos_strip == 'p' ? IsPositiveStrip(true) : IsPositiveStrip(false);
		SetStripID(strip_id);
		has_triggered == 0 ? HasTriggered(false) : HasTriggered(true);
		SetTiming((double)timing);
		SetUncorrectedADCUnits((double)un_adc);
		SetADCUnits((double)adc);
		SetEnergy(energy);
		SetEnergyResolution(energy_res);
		return true;
	} else {
		return false;
	}

  // to be written later 
/*
  vector<MString> tokens = Line.Tokenize(" ");
  if( tokens.size() >= 10 ){
	  SetDetectorID(tokens.at(1).ToInt());
	  tokens.at(2) == "p" ? IsPositiveStrip(true) : IsPositiveStrip(false);
	  SetStripID(tokens.at(3).ToInt());
	  tokens.at(4) == "0" ? HasTriggered(false) : HasTriggered(true);
	  SetTiming( tokens.at(5).ToDouble() );
	  SetUncorrectedADCUnits( tokens.at(6).ToDouble() );
	  SetADCUnits( tokens.at(7).ToDouble() );
	  SetEnergy( tokens.at(8).ToDouble() );
	  SetEnergyResolution( tokens.at(9).ToDouble() );
	  int det_id, pos_strip, strip_id
	  return true;
  } else {
	  return false;
  }
  */
}


////////////////////////////////////////////////////////////////////////////////


//! Set the origins from the simulations (take care of duplicates)
void MStripHit::AddOrigins(vector<int> Origins)
{
  m_Origins.insert(m_Origins.end(), Origins.begin(), Origins.end());
  sort(m_Origins.begin(), m_Origins.end());
  m_Origins.erase(unique(m_Origins.begin(), m_Origins.end()), m_Origins.end());
}

  
////////////////////////////////////////////////////////////////////////////////


bool MStripHit::StreamDat(ostream& S, int Version)
{
  //! Stream the content to an ASCII file 
  
  S<<"SH "
   <<m_ReadOutElement->GetDetectorID()<<" "
   <<((m_ReadOutElement->IsLowVoltageStrip() == true) ? "l" : "h")<<" "
   <<m_ReadOutElement->GetStripID()<<" "
   <<m_HasTriggered<<" "
   <<setprecision(9)<<m_Timing<<" "
   <<m_UncorrectedADCUnits<<" "
   <<m_ADCUnits<<" "
   <<m_Energy<<" "
   <<m_EnergyResolution<<endl;
 
  return true;
}




////////////////////////////////////////////////////////////////////////////////


void MStripHit::StreamRoa(ostream& S)
{
  //! Stream the content in MEGAlib's evta format 

  S<<"UH " 
   <<m_ReadOutElement->GetDetectorID()<<" "
   <<m_ReadOutElement->GetStripID()<<" "
   <<((m_ReadOutElement->IsLowVoltageStrip() == true) ? "l" : "h")<<" "
   <<m_ADCUnits<<" "
   <<m_Timing<<" "
   <<m_PreampTemp<<" ";
  for (unsigned int i = 0; i < m_Origins.size(); ++i) {
    if (i != 0) S<<";";
    S<<m_Origins[i]; 
  }
  S<<endl;
}


// MStripHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
