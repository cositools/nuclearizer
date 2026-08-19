/*
 * MReadOutElementVoxel3D.cxx
 *
 * Copyright (C) by Andreas Zoglauer, Valentina Fioretti
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
      m_DetectorID(""),
      m_CrystalID(g_UnsignedIntNotDefined),
      m_VoxelXID(g_UnsignedIntNotDefined),
      m_VoxelYID(g_UnsignedIntNotDefined),
      m_VoxelZID(g_UnsignedIntNotDefined)
{
}


////////////////////////////////////////////////////////////////////////////////


//! Constructor for a crystal-only read-out element
MReadOutElementVoxel3D::MReadOutElementVoxel3D(
  const MString& DetectorID,
  unsigned int CrystalID)
    : MReadOutElement(),
      m_DetectorID(DetectorID),
      m_CrystalID(CrystalID),
      m_VoxelXID(g_UnsignedIntNotDefined),
      m_VoxelYID(g_UnsignedIntNotDefined),
      m_VoxelZID(g_UnsignedIntNotDefined)
{
}

////////////////////////////////////////////////////////////////////////////////

//! Parameterized constructor
MReadOutElementVoxel3D::MReadOutElementVoxel3D(
  const MString& DetectorID,
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
  if (IsCrystalOnly() == true) {
    return new MReadOutElementVoxel3D(m_DetectorID, m_CrystalID);
  }

  return new MReadOutElementVoxel3D(m_DetectorID, m_CrystalID, m_VoxelXID, m_VoxelYID, m_VoxelZID);
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutElementVoxel3D::Clear()
{
  MReadOutElement::Clear();
  m_DetectorID = "";
  m_CrystalID = g_UnsignedIntNotDefined;
  m_VoxelXID = g_UnsignedIntNotDefined;
  m_VoxelYID = g_UnsignedIntNotDefined;
  m_VoxelZID = g_UnsignedIntNotDefined;
}

////////////////////////////////////////////////////////////////////////////////


//! Return true if all voxel IDs are defined
bool MReadOutElementVoxel3D::HasVoxelIDs() const
{
  return m_VoxelXID != g_UnsignedIntNotDefined &&
         m_VoxelYID != g_UnsignedIntNotDefined &&
         m_VoxelZID != g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if only detector and crystal IDs are defined
bool MReadOutElementVoxel3D::IsCrystalOnly() const
{
  return m_VoxelXID == g_UnsignedIntNotDefined &&
         m_VoxelYID == g_UnsignedIntNotDefined &&
         m_VoxelZID == g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Clear voxel IDs and keep detector and crystal identification
void MReadOutElementVoxel3D::ClearVoxelIDs()
{
  m_VoxelXID = g_UnsignedIntNotDefined;
  m_VoxelYID = g_UnsignedIntNotDefined;
  m_VoxelZID = g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Set all voxel IDs
void MReadOutElementVoxel3D::SetVoxelIDs(
  unsigned int VoxelXID,
  unsigned int VoxelYID,
  unsigned int VoxelZID)
{
  m_VoxelXID = VoxelXID;
  m_VoxelYID = VoxelYID;
  m_VoxelZID = VoxelZID;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if this read-out element is of the given type
bool MReadOutElementVoxel3D::IsOfType(const MString& String) const
{
  if (String == "voxel3d") return true;

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
  const MReadOutElementVoxel3D* Other = dynamic_cast<const MReadOutElementVoxel3D*>(&R);
  if (Other == nullptr) {
    if (g_Verbosity >= c_Error) cout << "ERROR: Comparison with different read-out element type" << endl;
    return false;
  }

  if (m_DetectorID != Other->m_DetectorID) return false;
  if (m_CrystalID != Other->m_CrystalID) return false;

  const bool ThisHasVoxelIDs = HasVoxelIDs();
  const bool OtherHasVoxelIDs = Other->HasVoxelIDs();

  if (ThisHasVoxelIDs != OtherHasVoxelIDs) return false;
  if (IsCrystalOnly() == true && Other->IsCrystalOnly() == true) return true;
  if (ThisHasVoxelIDs == false) return false;

  if (m_VoxelXID != Other->m_VoxelXID) return false;
  if (m_VoxelYID != Other->m_VoxelYID) return false;
  if (m_VoxelZID != Other->m_VoxelZID) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Compare two voxel read-out elements for ordered containers
bool MReadOutElementVoxel3D::operator<(const MReadOutElementVoxel3D& R) const
{
  if (m_DetectorID != R.m_DetectorID) return m_DetectorID < R.m_DetectorID;
  if (m_CrystalID != R.m_CrystalID) return m_CrystalID < R.m_CrystalID;

  const bool ThisHasVoxelIDs = HasVoxelIDs();
  const bool OtherHasVoxelIDs = R.HasVoxelIDs();

  if (ThisHasVoxelIDs != OtherHasVoxelIDs) return ThisHasVoxelIDs < OtherHasVoxelIDs;
  if (IsCrystalOnly() == true && R.IsCrystalOnly() == true) return false;
  if (ThisHasVoxelIDs == false) {
    return m_VoxelXID < R.m_VoxelXID ||
           (m_VoxelXID == R.m_VoxelXID && m_VoxelYID < R.m_VoxelYID) ||
           (m_VoxelXID == R.m_VoxelXID && m_VoxelYID == R.m_VoxelYID && m_VoxelZID < R.m_VoxelZID);
  }

  if (m_VoxelXID != R.m_VoxelXID) return m_VoxelXID < R.m_VoxelXID;
  if (m_VoxelYID != R.m_VoxelYID) return m_VoxelYID < R.m_VoxelYID;
  return m_VoxelZID < R.m_VoxelZID;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of parsable elements
unsigned int MReadOutElementVoxel3D::GetNumberOfParsableElements() const
{
  return 5;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer
bool MReadOutElementVoxel3D::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + GetNumberOfParsableElements()) {
    if (g_Verbosity >= c_Error) cout << "ERROR: Not enough elements to parse. Number of tokens is "
                                     << T.GetNTokens() << " and less than 5" << endl;
    return false;
  }

  m_DetectorID = T.GetTokenAtAsString(StartElement);
  m_CrystalID = T.GetTokenAtAsUnsignedIntFast(StartElement + 1);
  m_VoxelXID = T.GetTokenAtAsUnsignedIntFast(StartElement + 2);
  m_VoxelYID = T.GetTokenAtAsUnsignedIntFast(StartElement + 3);
  m_VoxelZID = T.GetTokenAtAsUnsignedIntFast(StartElement + 4);

  if (m_DetectorID == "") {
    if (g_Verbosity >= c_Warning) cout << "WARNING: Parsed empty DetectorID (token index "
                                       << StartElement << ")." << endl;
  }

  if (m_CrystalID == g_UnsignedIntNotDefined ||
      m_VoxelXID == g_UnsignedIntNotDefined ||
      m_VoxelYID == g_UnsignedIntNotDefined ||
      m_VoxelZID == g_UnsignedIntNotDefined) {

    if (g_Verbosity >= c_Warning) cout << "WARNING: Parsed undefined ID(s): "
                                       << "Crystal = " << m_CrystalID
                                       << " Vx = " << m_VoxelXID
                                       << " Vy = " << m_VoxelYID
                                       << " Vz = " << m_VoxelZID << endl;
  }

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
  if (m_DetectorID == "" && g_Verbosity >= c_Warning) {
    cout << "WARNING: called an element with empty DetectorID" << endl;
  }

  if (m_CrystalID == g_UnsignedIntNotDefined && g_Verbosity >= c_Warning) {
    cout << "WARNING: called an element with undefined CrystalID" << endl;
  }

  const bool AnyVoxelIDDefined =
    m_VoxelXID != g_UnsignedIntNotDefined ||
    m_VoxelYID != g_UnsignedIntNotDefined ||
    m_VoxelZID != g_UnsignedIntNotDefined;

  if (AnyVoxelIDDefined == true && HasVoxelIDs() == false && g_Verbosity >= c_Warning) {
    cout << "WARNING: called an element with partially defined voxel IDs" << endl;
  }

  ostringstream OS;
  OS << "DetectorID: " << m_DetectorID << ", CrystalID: " << m_CrystalID;

  if (HasVoxelIDs() == true) {
    OS << ", VoxelID: (" << m_VoxelXID << ", " << m_VoxelYID << ", " << m_VoxelZID << ")";
  }

  return OS.str();
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

