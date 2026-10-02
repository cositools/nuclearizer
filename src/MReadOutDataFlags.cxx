/*
 * MReadOutDataFlags.cxx
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
// MReadOutDataFlags
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutDataFlags.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutDataFlags)
#endif


////////////////////////////////////////////////////////////////////////////////


//! The type name --- must be unique
const MString MReadOutDataFlags::m_Type = "flags";
//! The type name ID --- must be unique
const long MReadOutDataFlags::m_TypeID = m_Type.GetHash();


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutDataFlags::MReadOutDataFlags() : MReadOutData(nullptr), m_Flags(0)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor given the data
MReadOutDataFlags::MReadOutDataFlags(MReadOutData* Data) : MReadOutData(Data), m_Flags(0)
{
}
  
////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutDataFlags::~MReadOutDataFlags()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this element
MReadOutDataFlags* MReadOutDataFlags::Clone() const
{
  MReadOutDataFlags* ROD = new MReadOutDataFlags();
  ROD->SetFlags(m_Flags);
  if (m_Wrapped != 0) {
    ROD->SetWrapped(m_Wrapped->Clone());
  }
  return ROD;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutDataFlags::Clear()
{
  MReadOutData::Clear();
  m_Flags = 0;
}


////////////////////////////////////////////////////////////////////////////////

 
//! Return the number of parsable elements
unsigned int MReadOutDataFlags::GetNumberOfParsableElements() const
{
  return MReadOutData::GetNumberOfParsableElements() + 1;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer 
bool MReadOutDataFlags::Parse(const MTokenizer& T, unsigned int StartElement)
{
  // Go deep first:
  if (MReadOutData::Parse(T, StartElement) == false) return false;
  
  // Then here:
  m_Flags = T.GetTokenAtAsUnsignedIntFast(StartElement + MReadOutData::GetNumberOfParsableElements());
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MReadOutDataFlags::ToString() const
{
  ostringstream os;
  os<<MReadOutData::ToString()<<m_Flags<<" ";
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a parsable string
MString MReadOutDataFlags::ToParsableString(bool WithDescriptor) const
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
ostream& operator<<(ostream& os, const MReadOutDataFlags& R)
{
  os<<R.ToString();
  return os;
}

  
// MReadOutDataFlags.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
