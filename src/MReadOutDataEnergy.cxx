/*
 * MReadOutDataEnergy.cxx
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
// MReadOutDataEnergy
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataEnergy.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutDataEnergy)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The type name --- must be unique
const MString MReadOutDataEnergy::m_Type = "energy";
//! The type name ID --- must be unique
const long MReadOutDataEnergy::m_TypeID = m_Type.GetHash();


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataEnergy::MReadOutDataEnergy() : MReadOutData(nullptr), m_Energy(0.)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataEnergy::MReadOutDataEnergy(MReadOutData* Data) : MReadOutData(Data), m_Energy(0.)
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataEnergy::~MReadOutDataEnergy()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this element
MReadOutDataEnergy* MReadOutDataEnergy::Clone() const
{
  MReadOutDataEnergy* ROD = new MReadOutDataEnergy();
  ROD->SetEnergy(m_Energy);
  if (m_Wrapped != 0) {
    ROD->SetWrapped(m_Wrapped->Clone());
  }
  return ROD;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataEnergy::Clear()
{
  MReadOutData::Clear();
  m_Energy = 0.;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataEnergy::GetNumberOfParsableElements() const
{
  return MReadOutData::GetNumberOfParsableElements() + 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataEnergy::Parse(const MTokenizer& T, unsigned int StartElement)
{
  // Go deep first:
  if (MReadOutData::Parse(T, StartElement) == false) return false;
  
  // Then here:
  m_Energy = T.GetTokenAtAsDouble(StartElement + MReadOutData::GetNumberOfParsableElements());
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataEnergy::ToString() const
{
  ostringstream os;
  os<<MReadOutData::ToString()<<m_Energy<<" ";
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a parsable string
MString MReadOutDataEnergy::ToParsableString(bool WithDescriptor) const
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
ostream& operator<<(ostream& os, const MReadOutDataEnergy& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataEnergy.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
