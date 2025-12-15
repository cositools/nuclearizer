/*
 * MReadOutElementVoxel3D.h
 *
 *
 * Copyright (C) by Andreas Zoglauer, Valentina Fioretti.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutElementVoxel3D__
#define __MReadOutElementVoxel3D__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutElement.h"

// Forward declarations:

////////////////////////////////////////////////////////////////////////////////


//! The read-out element of a BGO voxel. It reads the detector name and the voxel x, y, z, ID
class MReadOutElementVoxel3D : public MReadOutElement
{
  // public interface:
 public:
  //! default constructor - Read out element of a voxel 3D
  MReadOutElementVoxel3D();

  //! full constructor - Read out element of a voxel 3D
  MReadOutElementVoxel3D(unsigned int m_DetectorID, unsigned int m_CrystalID, unsigned int m_VoxelXID, unsigned int m_VoxelYID, unsigned int m_VoxelZID);

  //! Simple default destructor
  virtual ~MReadOutElementVoxel3D();

  //! Clone this read-out element - the returned element must be deleted!
  virtual MReadOutElementVoxel3D* Clone() const;

  //! Clear the content of this read-out element
  virtual void Clear();

  //! Compare two read-out elements
  virtual bool operator==(const MReadOutElement& R) const;
  //! Smaller than operator
  virtual bool operator<(const MReadOutElement& R) const;

  //! Return true if this read-out element is of the given type
  virtual bool IsOfType(const MString& String) const;
  //! Return the type of this read-out element
  virtual MString GetType() const;

  //! Setters and getters
  void SetDetectorID(unsigned int DetectorID)
  {
    m_DetectorID = DetectorID;
  }
  unsigned int GetDetectorID() const
  {
    return m_DetectorID;
  }

  void SetCrystalID(unsigned int CrystalID)
  {
    m_CrystalID = CrystalID;
  }
  unsigned int GetCrystalID() const
  {
    return m_CrystalID;
  }

  void SetVoxelXID(unsigned int VoxelXID)
  {
    m_VoxelXID = VoxelXID;
  }
  unsigned int GetVoxelXID() const
  {
    return m_VoxelXID;
  }

  void SetVoxelYID(unsigned int VoxelYID)
  {
    m_VoxelYID = VoxelYID;
  }
  unsigned int GetVoxelYID() const
  {
    return m_VoxelYID;
  }

  void SetVoxelZID(unsigned int VoxelZID)
  {
    m_VoxelZID = VoxelZID;
  }
  unsigned int GetVoxelZID() const
  {
    return m_VoxelZID;
  }

  //! Return the number of parsable elements
  virtual unsigned int GetNumberOfParsableElements() const;
  //! Parse the data from the tokenizer
  virtual bool Parse(const MTokenizer& T, unsigned int StartElement);
  //! Return the data as parsable string
  virtual MString ToParsableString(bool WithDescriptor = false) const;

  //! Dump a string
  virtual MString ToString() const;


  // protected methods:
 protected:
  // private methods:
 private:
  // protected members:
 protected:
  unsigned int m_DetectorID; //!< Detector ID
  unsigned int m_CrystalID; //!< Crystal ID

  unsigned int m_VoxelXID; //!< Voxel index X
  unsigned int m_VoxelYID; //!< Voxel index Y
  unsigned int m_VoxelZID; //!< Voxel index Z

  // private members:
 private:
#ifdef ___CLING___
 public:
  ClassDef(MReadOutElementVoxel3D, 0) // no description
#endif
};

//! Streamify the read-out element
ostream& operator<<(ostream& os, const MReadOutElementVoxel3D& R);

#endif


////////////////////////////////////////////////////////////////////////////////
