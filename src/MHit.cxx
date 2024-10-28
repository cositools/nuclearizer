/*
 * MHit.cxx
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
// MHit
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MHit.h"

// Standard libs:
#include <algorithm>

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MHit)
#endif


////////////////////////////////////////////////////////////////////////////////


MHit::MHit()
{
  // Construct an instance of MHit

  Clear();
}


////////////////////////////////////////////////////////////////////////////////


MHit::~MHit()
{
  // Delete this instance of MHit
  
  // This strip hits are not deleted since they where not generated here
}


////////////////////////////////////////////////////////////////////////////////


void MHit::Clear()
{
  // Reset all data

  m_Position = g_VectorNotDefined;
  m_Energy = g_DoubleNotDefined;
  m_PositionResolution = g_VectorNotDefined;
  m_EnergyResolution = g_DoubleNotDefined;

  m_StripHits.clear();
  m_Origins.clear();
}


////////////////////////////////////////////////////////////////////////////////


MStripHit* MHit::GetStripHit(unsigned int i) 
{ 
  //! Return strip hit i

  if (i < m_StripHits.size()) {
    return m_StripHits[i];
  }

  merr<<"Index out of bounds!"<<show;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MHit::RemoveStripHit(unsigned int i)
{
  //! Remove a strip hit
  if (i < m_StripHits.size()) {
    m_StripHits.erase(m_StripHits.begin()+i);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MHit::RemoveStripHit(MStripHit* StripHit)
{
  //! Remove a strip hit
  
  vector<MStripHit*>::iterator I = find(m_StripHits.begin(), m_StripHits.end(), StripHit);
  if (I != m_StripHits.end()) {
    m_StripHits.erase(I); 
  }
}

////////////////////////////////////////////////////////////////////////////////


bool MHit::StreamDat(ostream& S, int Version)
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


void MHit::StreamEvta(ostream& S)
{
  //! Stream the content to an ASCII file 
  
  // Assemble the origin information;
  vector<int> Origins;
  
  // Fix the origins: only those existing both on x and y strips count
  vector<int> xOrigins;
  vector<int> yOrigins;
  for (unsigned int s = 0; s < GetNStripHits(); ++s) {
    //GetStripHit(s)->StreamRoa(cout);
    vector<int> NewOrigins = GetStripHit(s)->GetOrigins();
    if (GetStripHit(s)->IsLowVoltageStrip() == true) {
      for (int o: NewOrigins) {
        xOrigins.push_back(o);
      }
    } else {
      for (int o: NewOrigins) {
        yOrigins.push_back(o);
      }
    }
  }
  
  sort(xOrigins.begin(), xOrigins.end());
  xOrigins.erase(unique(xOrigins.begin(), xOrigins.end()), xOrigins.end());
  sort(yOrigins.begin(), yOrigins.end());
  yOrigins.erase(unique(yOrigins.begin(), yOrigins.end()), yOrigins.end());
  
  set_intersection(xOrigins.begin(), xOrigins.end(),
                   yOrigins.begin(), yOrigins.end(),
                   std::back_inserter(Origins));

  if ((xOrigins.size() != 0 || yOrigins.size() != 0) && Origins.size() == 0) {
    // This is the case when the strip pairing got screwed up completely, and the hits got mixed
    // In this case we need to keep the mixing:
    for (unsigned int s = 0; s < GetNStripHits(); ++s) {
      vector<int> NewOrigins = GetStripHit(s)->GetOrigins();
      for (int o: NewOrigins) {
        Origins.push_back(o);
      }
    }
    sort(Origins.begin(), Origins.end());
    Origins.erase(unique(Origins.begin(), Origins.end()), Origins.end());
  }
  
  S<<"HT 3;"<<m_Position.GetX()<<";"<<m_Position.GetY()<<";"<<m_Position.GetZ()<<";"<<m_Energy
       <<";"<<m_PositionResolution.GetX()<<";"<<m_PositionResolution.GetY()<<";"<<m_PositionResolution.GetZ()<<";"<<m_EnergyResolution;
  for (unsigned int i = 0; i < Origins.size(); ++i) {
    S<<";"<<Origins[i]; 
  }
  S<<endl;

}


////////////////////////////////////////////////////////////////////////////////


bool MHit::Parse(MString &Line, int Version){

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


////////////////////////////////////////////////////////////////////////////////


//! Set the origins from the simulations (take care of duplicates)
void MHit::AddOrigins(vector<int> Origins)
{
  m_Origins.insert(m_Origins.end(), Origins.begin(), Origins.end());
  sort(m_Origins.begin(), m_Origins.end());
  m_Origins.erase(unique(m_Origins.begin(), m_Origins.end()), m_Origins.end());
}







// MHit.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
