/*
 * MNCTDetectorArray.h
 *
 * Copyright (C) 2008-2008 by Jau-Shian Liang.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MNCTDetectorArray__
#define __MNCTDetectorArray__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MDGeometry.h"
#include "MDGeometryQuest.h"
#include "MDVolume.h"
#include "MNCTStrip.h"
#include "MNCTHitInVoxel.h"
#include "MNCTCoincidenceVolume.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MNCTDetectorArray
{

  //! Detail information of each detector
  class Detector
  {
    public:
    MString DetectorName;
    int DetectorID;

    bool XStripInverse;//Is X-strip order inverse?
    bool YStripInverse;//Is Y-strip order inverse?
    bool XYStripSwap;//Is X-Y strip swap?
    bool XStripUp;//Is X-strip at z+ side?
    bool DetectorON;//Is detector turned ON?
    bool XStripHighVoltage;//Is X-strip at high voltage side?
    double Voltage;//High voltage side strips' voltage
    double Impurity;//Impurity concentration
    double DeadRegion;//used by charge losing modeling

    void Clear();
  };

  // public interface:
 public:
  //! Standard constructor
  MNCTDetectorArray();
  MNCTDetectorArray(MDGeometryQuest* Geometry);
  
  //! Default destructor
  ~MNCTDetectorArray();

  //!
  void Activate();

  //!
  void SetGeometry(MDGeometryQuest* Geometry){m_Geometry = Geometry;}
  void SetLoadDeadStrip(bool yn){m_LoadDeadStrip = yn;}
  void SetLoadCoincidence(bool yn){m_LoadCoincidence = yn;}
  void SetLoadAntiCoincidence(bool yn){m_LoadAntiCoincidence = yn;}
  void SetDetectorFile(MString DetectorFile){m_DetectorFile = DetectorFile;}
  void SetDeadStripFile(MString DeadStripFile){m_DeadStripFile = DeadStripFile;}
  void SetCoincidenceFile(MString CoinFile){m_CoinFile = CoinFile;}
  void SetAntiCoincidenceFile(MString AntiCoinFile){m_AntiCoinFile = AntiCoinFile;}

  //! Get the detector number
  int GetDetectorNum(){return m_DetectorNumber;}

  
  //! Get parameters
  double GetWidth(){return m_Width;}
  double GetLength(){return m_Length;}
  double GetCutLength(){return m_CutLength;}
  unsigned int GetStrips(){return m_Strips;}
  int GetFirstStripID(){return m_FirstStripID;}
  double GetPitch(){return m_Pitch;}
  double GetGap(){return m_Gap;}
  double GetGuardringWidth(){return m_GuardringWidth;}
 
  
  //! Return the detector ID using the detector name
  int GetID(MString DetectorName) const;
  
  //! Conversion between detector ID and volume
  //int GetID(const MDVolume* DetectorVolume){return FindVolume(DetectorVolume)->DetectorID;}
  //MDVolume* GetVolume(int DetectorID){return FindID(DetectorID)->DetectorVolume;}
 
  //! Return the detail information of NCTdetector
  bool IsXStripInverse(int DetectorID){return FindID(DetectorID)->XStripInverse;}
  bool IsYStripInverse(int DetectorID){return FindID(DetectorID)->YStripInverse;}
  bool XYStripSwap(int DetectorID){return FindID(DetectorID)->XYStripSwap;}
  bool IsXStripUp(int DetectorID){return FindID(DetectorID)->XStripUp;}
  bool IsDetectorON(int DetectorID){return FindID(DetectorID)->DetectorON;}
  bool IsXStripHighVoltage(int DetectorID){return FindID(DetectorID)->XStripHighVoltage;}
  bool IsXPositive(int DetectorID){return (IsXStripHighVoltage(DetectorID))? GetVoltage(DetectorID)>0 : GetVoltage(DetectorID)<0;}
  bool IsZPositive(int DetectorID){return !(IsXStripUp(DetectorID)^IsXPositive(DetectorID));}
  double GetVoltage(int DetectorID){return FindID(DetectorID)->Voltage;}
  double GetImpurity(int DetectorID){return FindID(DetectorID)->Impurity;}
  double GetDeadRegion(int DetectorID){return FindID(DetectorID)->DeadRegion;}

  //! Check detector and dead strip
  bool IsNCTDetector(const MString& DetectorName);
  bool IsDeadStrip(const MNCTStrip& Strip);
  bool IsDeadStrip(int DetectorID, bool XStrip, int StripID);

  //! Check coincidence and anti-coincidence
  bool IsCoincidence(const MDVolume* Vol);
  bool IsAntiCoincidence(const MDVolume* Vol);
  MNCTCoincidenceVolume* GetCoincidenceVolume(const MDVolume* Vol);
  MNCTCoincidenceVolume* GetAntiCoincidenceVolume(const MDVolume* Vol);

  //! Check sensitive or guardring region
  bool IsSensitive(const MVector& PositionInVolume);
  bool IsGuardring(const MVector& PositionInVolume);
  bool IsNearGuardring(const MVector& PositionInVolume);

  //! Convert PositionInVolume to HitInVoxel
  MNCTHitInVoxel PositionInVolume2Voxel(int DetectorID, MVector PositionInVolume, double Energy=0);

  //! Dump the information of detectors, if necessary
  void DumpParameters();
  void DumpDetectors();
  void DumpDeadStrips();
  void DumpCoincidence();
  void DumpAntiCoincidence();
  

  // protected methods:
 protected:
  //!
  void Initialize();
  
  //! Find detector details by detector ID or detector volume
  Detector* FindID(int DetectorID);
  //Detector* FindVolume(const MDVolume* DetectorVolume);

  // private methods:
 private:
  //! load detail detector information
  void Load_Detector(MString filename);
  //! Load dead strips
  void Load_DeadStrip(MString filename);

  //! Load coincidence volumes
  void Load_Coincidence(MString filename);

  //! Load anti-coincidence volumes
  void Load_AntiCoincidence(MString filename);

  //! Find all detector volume
  void FindAllDetectorVolumes();
  void FindDetectorVolumes(MDVolume* V);
  int FindVolumeByName(MString name);

  // protected members:
 protected:

  //! input geometry
  MDGeometryQuest* m_Geometry;

  //! set whether loading file or not
  bool m_LoadDeadStrip;
  bool m_LoadCoincidence;
  bool m_LoadAntiCoincidence;

  //! input file name
  MString m_DetectorFile;//detail information of detectors
  MString m_DeadStripFile;//a list of dead strips
  MString m_CoinFile;//list of conincidence volume
  MString m_AntiCoinFile;//list of anti-coincidence volume

  //! 
  unsigned int m_DetectorNumber;
   
  //! Detector parameters
  double m_Width;//=1.5;//thickness
  double m_Length;//=8.04;//full length of edge
  double m_CutLength;//=1.138;//triangle cut at corner(cut down length of edge)

  //! Strip parameters
  unsigned int m_Strips;//=37;//strip number
  int m_FirstStripID;//=1;//the first strip ID
  double m_Pitch;//=0.2;//strip pitch
  double m_Gap;//=0.02;//gap width between strips
  double m_GuardringWidth;//=0.32;

  
  //! Detail information of each detector
  vector<Detector> m_NCTDetectors;

  //! Dead strips list
  vector<MNCTStrip> m_DeadStrips;

  //! Coincidence volume list
  vector<MNCTCoincidenceVolume> m_CoinVol;

  //! Anti-coincidence volume list
  vector<MNCTCoincidenceVolume> m_AntiCoinVol;

  // private members:
 private:
  //! Find all detector volume
  vector<MDVolume*> m_AllDetectorVolumes;


#ifdef ___CINT___
 public:
  ClassDef(MNCTDetectorArray, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
