/*
 * MNCTData.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTData__
#define __MNCTData__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDGeometryQuest.h"

// Forward declarations:
class MNCTModule;

////////////////////////////////////////////////////////////////////////////////


class MNCTData
{
  // public interface:
 public:
  //! Default constructor
  MNCTData();
  //! Default destructor
  virtual ~MNCTData();

  //! Reset all data
  void Clear();

  //! Load all data from a file
  bool Load(MString FileName);
  //! Save all data to a file
  bool Save(MString FileName);

  //! Return the number of available modules
  unsigned int GetNAvailableModules() { return m_AvailableModules.size(); }
  //! Return the available modules at position i --- no error checks are performed  
  MNCTModule* GetAvailableModule(unsigned int i);
  //! Return the modules at position i in the current sequence --- no error checks are performed  
  MNCTModule* GetAvailableModule(MString Name);

  //! Return the number of modules in the current sequence
  unsigned int GetNModules() { return m_Modules.size(); }
  //! Return the modules at position i in the current sequence --- no error checks are performed  
  MNCTModule* GetModule(unsigned int i);
  //! Set a module at a specific position - return false if other modules had to be eliminated  
  bool SetModule(MNCTModule* Module, unsigned int i);
  //! Remove module at a specific position - return false if other modules had to be eliminated  
  bool RemoveModule(unsigned int i);

  //! Return a list of possible volumes, which might to follow
  vector<MNCTModule*> ReturnPossibleVolumes(unsigned int Position);
  //! Return a list of possible volumes, which might to follow
  vector<MNCTModule*> ReturnPossibleVolumes(vector<MNCTModule*>& Previous);

  //! Set the name of the geometry file
  void SetGeometryFileName(MString GeometryFileName) { m_GeometryFileName = GeometryFileName; }
  //! Return the name of the geometry file
  MString GetGeometryFileName() const { return m_GeometryFileName; }

  //! Set the name of the file to be loaded
  void SetLoadFileName(MString LoadFileName) { m_LoadFileName = LoadFileName; }
  //! Return the name of the file to be loaded
  MString GetLoadFileName() const { return m_LoadFileName; }

  //! Set the name of the file to be saved
  void SetSaveFileName(MString SaveFileName) { m_SaveFileName = SaveFileName; }
  //! Return the name of the file to be saved
  MString GetSaveFileName() const { return m_SaveFileName; }

  //! Load the geometry and transfer it to all modules
  bool LoadGeometry();

  //! Get highest analysis level
  int GetHighestAnalysisLevel() const;

  // There are several analysis levels corresponding
  static const int c_DataSimed         = -1;
  static const int c_DataRaw           = 0;
  static const int c_DataCalibrated    = 1;
  static const int c_DataReconstructed = 2;

  // protected methods:
 protected:
  //MNCTData() {};
  //MNCTData(const MNCTData& NCTData) {};

  //! Validate the sequence of possible modules
  bool Validate();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! List of all available modules
  vector<MNCTModule*> m_AvailableModules;

  //! Sequence of currently used modules
  vector<MNCTModule*> m_Modules;

  //! The geometry file name
  MString m_GeometryFileName;
  //! The name of the file to be loaded
  MString m_LoadFileName;
  //! The name of the file to be saving 
  MString m_SaveFileName;

  //! The geometry
  MDGeometryQuest* m_Geometry;

#ifdef ___CINT___
 public:
  ClassDef(MNCTData, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
