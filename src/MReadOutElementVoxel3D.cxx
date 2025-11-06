/*
 * MReadOutElementVoxel3D.cxx
 *
 * Copyright (C) by Andreas Zoglauer, Valentina Fioretti.
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
// MReadOutElementVoxel3D
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutElementVoxel3D.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutElementVoxel3D)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutElementVoxel3D::MReadOutElementVoxel3D()
    : MReadOutElement(),
      m_DetectorID(0),
      m_CrystalID(0),
      m_VoxelXID(g_UnsignedIntNotDefined),
      m_VoxelYID(g_UnsignedIntNotDefined),
      m_VoxelZID(g_UnsignedIntNotDefined)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Parameterized constructor
MReadOutElementVoxel3D::MReadOutElementVoxel3D(
  unsigned int DetectorID,
  unsigned int CrystalID,
  unsigned int VoxelXID,
  unsigned int VoxelYID,
  unsigned int VoxelZID)
    : MReadOutElement(),
      m_DetectorID(DetectorID),
      m_CrystalID(CrystalID),
      m_VoxelXID(VoxelXID),
      m_VoxelYID(VoxelYID),
      m_VoxelZID(VoxelZID)
{
}

////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutElementVoxel3D::~MReadOutElementVoxel3D()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this read-out element - the returned element must be deleted!
MReadOutElementVoxel3D* MReadOutElementVoxel3D::Clone() const
{
  MReadOutElementVoxel3D* R = new MReadOutElementVoxel3D(
    m_DetectorID,
    m_CrystalID,
    m_VoxelXID,
    m_VoxelYID,
    m_VoxelZID);
  return R;
}
////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutElementVoxel3D::Clear()
{
  MReadOutElement::Clear();
  m_DetectorID = g_UnsignedIntNotDefined;
  m_CrystalID = g_UnsignedIntNotDefined;
  m_VoxelXID = g_UnsignedIntNotDefined;
  m_VoxelYID = g_UnsignedIntNotDefined;
  m_VoxelZID = g_UnsignedIntNotDefined;
}

////////////////////////////////////////////////////////////////////////////////


//! Return true if this read-out element is of the given type
bool MReadOutElementVoxel3D::IsOfType(const MString& String) const
{
  if (String == "voxel3d")
    return true;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the type of this read-out element
MString MReadOutElementVoxel3D::GetType() const
{
  return "voxel3d";
}


////////////////////////////////////////////////////////////////////////////////


//! Test for equality
bool MReadOutElementVoxel3D::operator==(const MReadOutElement& R) const
{
  const MReadOutElementVoxel3D* S = dynamic_cast<const MReadOutElementVoxel3D*>(&R);
  if (S == 0)
    return false;

  if (m_DetectorID != S->m_DetectorID)
    return false;
  if (m_CrystalID != S->m_CrystalID)
    return false;

  if (m_VoxelXID != S->m_VoxelXID)
    return false;
  if (m_VoxelYID != S->m_VoxelYID)
    return false;
  if (m_VoxelZID != S->m_VoxelZID)
    return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Smaller than operator
bool MReadOutElementVoxel3D::operator<(const MReadOutElement& R) const
{
  const MReadOutElementVoxel3D* S = dynamic_cast<const MReadOutElementVoxel3D*>(&R);
  if (S == 0)
    return false;

  if (m_DetectorID < S->m_DetectorID)
    return true;

  if (m_DetectorID == S->m_DetectorID) {

    if (m_CrystalID < S->m_CrystalID)
      return true;
    if (m_CrystalID == S->m_CrystalID) {
      if (m_VoxelXID < S->m_VoxelXID)
        return true;
      if (m_VoxelXID == S->m_VoxelXID) {
        if (m_VoxelYID < S->m_VoxelYID)
          return true;
        if (m_VoxelYID == S->m_VoxelYID) {
          if (m_VoxelZID < S->m_VoxelZID)
            return true;
          if (m_VoxelZID == S->m_VoxelZID)
            return false;
        }
      }
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of parsable elements
unsigned int MReadOutElementVoxel3D::GetNumberOfParsableElements() const
{
  return 4;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer
bool MReadOutElementVoxel3D::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + 4) {
    merr << GetType() << ": Not enough elements to parse" << show;
    return false;
  }

  m_DetectorID = T.GetTokenAtAsUnsignedIntFast(StartElement);
  m_CrystalID = T.GetTokenAtAsUnsignedIntFast(StartElement + 1);
  m_VoxelXID = T.GetTokenAtAsUnsignedIntFast(StartElement + 2);
  m_VoxelYID = T.GetTokenAtAsUnsignedIntFast(StartElement + 3);
  m_VoxelZID = T.GetTokenAtAsUnsignedIntFast(StartElement + 4);

  return true;
}

////////////////////////////////////////////////////////////////////////////////


//! Return the data as parsable string
MString MReadOutElementVoxel3D::ToParsableString(bool WithDescriptor) const
{
  MString Return;
  if (WithDescriptor == true) {
    Return += "voxel3d ";
  }
  Return += m_DetectorID;
  Return += " ";
  Return += m_CrystalID;
  Return += " ";
  Return += m_VoxelXID;
  Return += " ";
  Return += m_VoxelYID;
  Return += " ";
  Return += m_VoxelZID;

  return Return;
}

////////////////////////////////////////////////////////////////////////////////


//! Convert content to a string
MString MReadOutElementVoxel3D::ToString() const
{
  ostringstream os;
  os << "DetectorID: " << m_DetectorID << ", CrystalID: " << m_CrystalID << ", VoxelID: (" << m_VoxelXID << ", " << m_VoxelYID << ", " << m_VoxelZID << ")";
  return os.str();
}

////////////////////////////////////////////////////////////////////////////////


//! Stream the data
ostream& operator<<(ostream& os, const MReadOutElementVoxel3D& R)
{
  os << R.ToString();
  return os;
}


// MReadOutElementVoxel3D.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
