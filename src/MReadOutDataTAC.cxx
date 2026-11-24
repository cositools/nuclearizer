/*
 * MReadOutDataTAC.cxx
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
// MReadOutDataTAC
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataTAC.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutDataTAC)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The type name --- must be unique
const MString MReadOutDataTAC::m_Type = "tac";
//! The type name ID --- must be unique
const long MReadOutDataTAC::m_TypeID = m_Type.GetHash();


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataTAC::MReadOutDataTAC() : MReadOutData(nullptr), m_TAC(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataTAC::MReadOutDataTAC(MReadOutData* Data) : MReadOutData(Data), m_TAC(0)
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataTAC::~MReadOutDataTAC()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this element
MReadOutDataTAC* MReadOutDataTAC::Clone() const
{
  MReadOutDataTAC* ROD = new MReadOutDataTAC();
  ROD->SetTAC(m_TAC);
  if (m_Wrapped != 0) {
    ROD->SetWrapped(m_Wrapped->Clone());
  }
  return ROD;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataTAC::Clear()
{
  MReadOutData::Clear();
  m_TAC = 0;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataTAC::GetNumberOfParsableElements() const
{
  return MReadOutData::GetNumberOfParsableElements() + 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataTAC::Parse(const MTokenizer& T, unsigned int StartElement)
{
  // Go deep first:
  if (MReadOutData::Parse(T, StartElement) == false) return false;
  
  // Then here:
  m_TAC = T.GetTokenAtAsUnsignedIntFast(StartElement + MReadOutData::GetNumberOfParsableElements());
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataTAC::ToString() const
{
  ostringstream os;
  os<<MReadOutData::ToString()<<m_TAC<<" ";
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a parsable string
MString MReadOutDataTAC::ToParsableString(bool WithDescriptor) const
{
  ostringstream os;
  if (WithDescriptor == true) {
    os<<GetCombinedType()<<" ";
  }
  os<<ToString();
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutDataTAC& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataTAC.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
