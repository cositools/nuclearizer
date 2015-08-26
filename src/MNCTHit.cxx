/*
 * MNCTHit.cxx
 *
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
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
// MNCTHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MNCTHit.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MNCTHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MNCTHit::MNCTHit()
{
  // Construct an instance of MNCTHit

  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MNCTHit::~MNCTHit()
{
  // Delete this instance of MNCTHit
  
  // This strip hits are not deleted since they where not generated here
}


////////////////////////////////////////////////////////////////////////////////


void MNCTHit::Clear()
{
  // Reset all data

  m_Position = g_VectorNotDefined;
  m_Energy = g_DoubleNotDefined;
  m_PositionResolution = g_VectorNotDefined;
  m_EnergyResolution = g_DoubleNotDefined;

  m_StripHits.clear();
}


////////////////////////////////////////////////////////////////////////////////


MNCTStripHit* MNCTHit::GetStripHit(unsigned int i) 
{ 
  //! Return strip hit i

  if (i < m_StripHits.size()) {
    return m_StripHits[i];
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTHit::StreamDat(ostream& S, int Version)
{
  //! Stream the content to an ASCII file 
  
  if( Version == 1 ){
     S<<"HT "<<m_Position.GetX()<<" "<<m_Position.GetY()<<" "<<m_Position.GetZ()<<" "<<m_Energy<<endl;
  } else if( Version == 2 ){
	  //stream the hit information, then stream the strip hit info for this hit so that 
	  //we will know which strip hits were associated with which hits
     S<<"HT "<<m_Position.GetX()<<" "<<m_Position.GetY()<<" "<<m_Position.GetZ()<<" "<<m_Energy<<endl;
	  for( auto SH : m_StripHits ){
		  SH->StreamDat(S,0);
	  }
  }

 
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MNCTHit::StreamEvta(ostream& S)
{
  //! Stream the content to an ASCII file 
  
  S<<"HT 3;"<<m_Position.GetX()<<";"<<m_Position.GetY()<<";"<<m_Position.GetZ()<<";"<<m_Energy
       <<";"<<m_PositionResolution.GetX()<<";"<<m_PositionResolution.GetY()<<";"<<m_PositionResolution.GetZ()<<";"<<m_EnergyResolution<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MNCTHit::Parse(MString &Line, int Version){

	//check that line begins with HT
	const char* line = Line.Data();
	if( line[0] == 'H' && line[1] == 'T' ){
		float X,Y,Z,E;
		sscanf(&line[3],"%f %f %f %f",&X, &Y, &Z, &E);
		m_Position.SetX(X);
		m_Position.SetY(Y);
		m_Position.SetZ(Z);
		m_Energy = E;
		return true;
	} else {
		return false;
	}
	
	/*
	if( Line.BeginsWith("HT") ){
		vector<MString> tokens = Line.Tokenize(" ");
		if( tokens.size() >= 5 ){
			m_Position.SetX( tokens.at(1).ToDouble() );
			m_Position.SetY( tokens.at(2).ToDouble() );
			m_Position.SetZ( tokens.at(3).ToDouble() );
			m_Energy = tokens.at(4).ToDouble();
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
	*/

}







// MNCTHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
