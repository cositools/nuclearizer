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
      m_DetectorName(""),
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
      m_DetectorName(DetectorID),
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
      m_DetectorName(DetectorID),
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
  m_DetectorName = "";
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
  if (String == "voxel3d") {
    return true;
  }

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
  if (O == nullptr) {
    return false;
  }
  return m_DetectorName == O->m_DetectorName && m_CrystalID == O->m_CrystalID &&
         m_VoxelXID == O->m_VoxelXID && m_VoxelYID == O->m_VoxelYID && m_VoxelZID == O->m_VoxelZID;
}

//! Compare two voxel read-out elements for ordered containers
bool MReadOutElementVoxel3D::operator<(const MReadOutElement& R) const
{
  const MReadOutElementVoxel3D* O = dynamic_cast<const MReadOutElementVoxel3D*>(&R);
  if (O == nullptr) {
    return GetType() < R.GetType(); // keeps ordering across types consistent
  }
  if (m_DetectorName != O->m_DetectorName) {
    return m_DetectorName < O->m_DetectorName;
  }
  if (m_CrystalID != O->m_CrystalID) {
    return m_CrystalID < O->m_CrystalID;
  }
  if (m_VoxelXID != O->m_VoxelXID) {
    return m_VoxelXID < O->m_VoxelXID;
  }
  if (m_VoxelYID != O->m_VoxelYID) {
    return m_VoxelYID < O->m_VoxelYID;
  }
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
  // MTokenizer treats a token starting with "#" or "//" as the start of a comment
  if (DetectorID.BeginsWith("#") == true || DetectorID.BeginsWith("//") == true) {
    return false;
  }

  // The placeholder token is read back as an empty detector ID
  if (DetectorID == c_EmptyDetectorIDToken) {
    return false;
  }

  // White space would split the detector ID into several tokens
  for (size_t Position = 0; Position < DetectorID.Length(); ++Position) {
    // isspace() requires an unsigned char value
    if (isspace(static_cast<unsigned char>(DetectorID[Position])) != 0) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Convert a token into an unsigned int - return false if it is not a plain decimal number in range
bool MReadOutElementVoxel3D::ParseUnsignedInt(const MString& Token, unsigned int& Value)
{
  // Only a non-empty sequence of decimal digits is accepted: no sign, no white space, no exponent
  if (Token.IsEmpty() == true) {
    return false;
  }

  // The number is accumulated in a wider type, so that values too large for an unsigned int are detected
  const unsigned long long DecimalBase = 10;
  unsigned long long Number = 0;
  for (size_t Position = 0; Position < Token.Length(); ++Position) {
    const char Digit = Token[Position];
    // isdigit() requires an unsigned char value
    if (isdigit(static_cast<unsigned char>(Digit)) == 0) {
      return false;
    }

    Number = DecimalBase * Number + static_cast<unsigned long long>(Digit - '0');
    if (Number > numeric_limits<unsigned int>::max()) {
      return false;
    }
  }

  // Value is only changed if the whole token is valid
  Value = static_cast<unsigned int>(Number);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer
bool MReadOutElementVoxel3D::Parse(const MTokenizer& T, unsigned int StartElement)
{
  const unsigned int Required = StartElement + GetNumberOfParsableElements();
  if (T.GetNTokens() < Required) {
    if (g_Verbosity >= c_Error) cout << "ERROR: Not enough elements to parse a voxel3d read-out element: "
                                     << "need " << Required << " tokens (start element " << StartElement << " + "
                                     << GetNumberOfParsableElements() << "), but got " << T.GetNTokens() << endl;
    return false;
  }

  // Parse into temporaries first, so that a failed parse leaves this element unchanged
  MString DetectorID = T.GetTokenAtAsString(StartElement);
  if (DetectorID == c_EmptyDetectorIDToken) {
    DetectorID = "";
  }

  unsigned int IDs[4];
  const char* Names[4] = { "crystal", "voxel X", "voxel Y", "voxel Z" };
  for (unsigned int i = 0; i < 4; ++i) {
    MString Token = T.GetTokenAtAsString(StartElement + 1 + i);
    if (ParseUnsignedInt(Token, IDs[i]) == false) {
      if (g_Verbosity >= c_Error) cout << "ERROR: The " << Names[i] << " ID \"" << Token
                                       << "\" (token " << StartElement + 1 + i << ") is not an unsigned integer" << endl;
      return false;
    }
  }

  m_DetectorName = DetectorID;
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

  if (m_DetectorName == "") {
    Return += c_EmptyDetectorIDToken;
  } else {
    if (IsWritableDetectorID(m_DetectorName) == false && g_Verbosity >= c_Error) {
      cout << "ERROR: Detector ID \"" << m_DetectorName << "\" cannot be parsed back correctly (whitespace, comment start or placeholder)" << endl;
    }
    Return += m_DetectorName;
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
    if (Value == g_UnsignedIntNotDefined) {
      OS << "?";
    } else {
      OS << Value;
    }
    return OS;
  };

  ostringstream OS;
  OS << "DetectorID: " << (m_DetectorName == "" ? MString("?") : m_DetectorName) << ", CrystalID: ";
  ID(OS, m_CrystalID);

  if (IsCrystalOnly() == false) {
    OS << ", VoxelID: (";
    ID(OS, m_VoxelXID) << ", ";
    ID(OS, m_VoxelYID) << ", ";
    ID(OS, m_VoxelZID) << ")";
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
