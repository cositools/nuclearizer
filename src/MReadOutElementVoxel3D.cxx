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
#include <cctype>
#include <limits>

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
    return new MReadOutElementVoxel3D(*this);
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
  const MReadOutElementVoxel3D* O = dynamic_cast<const MReadOutElementVoxel3D*>(&R);
  if (O == nullptr) return false;
  return m_DetectorID == O->m_DetectorID && m_CrystalID == O->m_CrystalID &&
         m_VoxelXID == O->m_VoxelXID && m_VoxelYID == O->m_VoxelYID && m_VoxelZID == O->m_VoxelZID;
}

//! Compare two voxel read-out elements for ordered containers
bool MReadOutElementVoxel3D::operator<(const MReadOutElement& R) const
{
  const MReadOutElementVoxel3D* O = dynamic_cast<const MReadOutElementVoxel3D*>(&R);
  if (O == nullptr) return GetType() < R.GetType();   // keeps ordering across types consistent
  if (m_DetectorID != O->m_DetectorID) return m_DetectorID < O->m_DetectorID;
  if (m_CrystalID != O->m_CrystalID) return m_CrystalID < O->m_CrystalID;
  if (m_VoxelXID != O->m_VoxelXID) return m_VoxelXID < O->m_VoxelXID;
  if (m_VoxelYID != O->m_VoxelYID) return m_VoxelYID < O->m_VoxelYID;
  return m_VoxelZID < O->m_VoxelZID;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of parsable elements
unsigned int MReadOutElementVoxel3D::GetNumberOfParsableElements() const
{
  return 5;
}


////////////////////////////////////////////////////////////////////////////////


//! Placeholder token written for an empty detector ID, so that the parsable string always has 5 tokens
const char* const MReadOutElementVoxel3D::c_EmptyDetectorIDToken = "-";

////////////////////////////////////////////////////////////////////////////////


//! Return true if the detector ID can be written as a single parsable token
bool MReadOutElementVoxel3D::IsWritableDetectorID(const MString& DetectorID)
{
  const char* C = DetectorID.Data();
  if (C[0] == '#' || (C[0] == '/' && C[1] == '/')) return false; // would start a comment in MTokenizer
  if (DetectorID == c_EmptyDetectorIDToken) return false;         // would be read back as empty
  for (; *C != '\0'; ++C) {
    if (isspace(static_cast<unsigned char>(*C)) != 0) return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////


//! Convert a token into an unsigned int - return false if it is not a plain decimal number in range
bool MReadOutElementVoxel3D::ParseUnsignedInt(const MString& Token, unsigned int& Value)
{
  const char* C = Token.Data();
  if (*C == '\0') return false;

  unsigned long long V = 0;
  for (; *C != '\0'; ++C) {
    if (*C < '0' || *C > '9') return false;
    V = 10*V + static_cast<unsigned long long>(*C - '0');
    if (V > numeric_limits<unsigned int>::max()) return false;
  }

  Value = static_cast<unsigned int>(V);
  return true;
}

////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer
bool MReadOutElementVoxel3D::Parse(const MTokenizer& T, unsigned int StartElement)
{
  const unsigned int Required = StartElement + GetNumberOfParsableElements();
  if (T.GetNTokens() < Required) {
    if (g_Verbosity >= c_Error) cout<<"ERROR: Not enough elements to parse a voxel3d read-out element: "
                                    <<"need "<<Required<<" tokens (start element "<<StartElement<<" + "
                                    <<GetNumberOfParsableElements()<<"), but got "<<T.GetNTokens()<<endl;
    return false;
  }

  // Parse into temporaries first, so that a failed parse leaves this element unchanged
  MString DetectorID = T.GetTokenAtAsString(StartElement);
  if (DetectorID == c_EmptyDetectorIDToken) DetectorID = "";

  unsigned int IDs[4];
  const char* Names[4] = { "crystal", "voxel X", "voxel Y", "voxel Z" };
  for (unsigned int i = 0; i < 4; ++i) {
    MString Token = T.GetTokenAtAsString(StartElement + 1 + i);
    if (ParseUnsignedInt(Token, IDs[i]) == false) {
      if (g_Verbosity >= c_Error) cout<<"ERROR: The "<<Names[i]<<" ID \""<<Token
                                      <<"\" (token "<<StartElement + 1 + i<<") is not an unsigned integer"<<endl;
      return false;
    }
  }

  m_DetectorID = DetectorID;
  m_CrystalID = IDs[0];
  m_VoxelXID = IDs[1];
  m_VoxelYID = IDs[2];
  m_VoxelZID = IDs[3];

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

  if (m_DetectorID == "") {
    Return += c_EmptyDetectorIDToken;
  } else {
    if (IsWritableDetectorID(m_DetectorID) == false && g_Verbosity >= c_Error) {
      cout<<"ERROR: Detector ID \""<<m_DetectorID<<"\" cannot be parsed back correctly (whitespace, comment start or placeholder)"<<endl;
    }
    Return += m_DetectorID;
  }
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


//! Convert content to a string - undefined IDs are shown as "?"
MString MReadOutElementVoxel3D::ToString() const
{
  auto ID = [](ostringstream& OS, unsigned int Value) -> ostringstream& {
    if (Value == g_UnsignedIntNotDefined) OS<<"?"; else OS<<Value;
    return OS;
  };

  ostringstream OS;
  OS<<"DetectorID: "<<(m_DetectorID == "" ? MString("?") : m_DetectorID)<<", CrystalID: ";
  ID(OS, m_CrystalID);

  if (IsCrystalOnly() == false) {
    OS<<", VoxelID: (";
    ID(OS, m_VoxelXID)<<", ";
    ID(OS, m_VoxelYID)<<", ";
    ID(OS, m_VoxelZID)<<")";
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


