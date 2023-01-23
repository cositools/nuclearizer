//MModuleDepthCalibrationB.h


#ifndef __MModuleDepthCalibrationB__
#define __MModuleDepthCalibrationB__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <unordered_map>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MModule.h"
#include "MModuleEnergyCalibrationUniversal.h"
#include "MDepthCalibratorB.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MModuleDepthCalibrationB : public MModule
{
  // public interface:
 public:
  //! Default constructor
  MModuleDepthCalibrationB();
  //! Default destructor
  virtual ~MModuleDepthCalibrationB();
  
  //! Create a new object of this class 
  virtual MModuleDepthCalibrationB* Clone() { return new MModuleDepthCalibrationB(); }

  //! Initialize the module
  virtual bool Initialize();

  //! Main data analysis routine, which updates the event to a new level 
  virtual bool AnalyzeEvent(MReadOutAssembly* Event);

  //! Show the options GUI
  virtual void ShowOptionsGUI();

  //! Set filename for lookup tables
  void SetLookupTableFilename(const MString& Filename) {m_LookupTableFile = Filename;}
  //! Get filename for lookup tables
  MString GetLookupTableFilename() const {return m_LookupTableFile;}

  //! Read the XML configuration
  bool ReadXmlConfiguration(MXmlNode* Node);

  //! Create the XML configuration
  MXmlNode* CreateXmlConfiguration();

  //! Set the global timing FWHM noise
  void SetTimingNoiseFWHM(const double Time) {m_TimingNoiseFWHM = Time;}
  //! Get the global timing FWHM noise
  double GetTimingNoiseFWHM() const {return m_TimingNoiseFWHM;}

  //! Finalize
  void Finalize();

  // protected methods:
 protected:
  //! Returns the strip with most energy from vector Strips, also gives back the energy fraction
  MStripHit* GetDominantStrip(std::vector<MStripHit*>& Strips, double& EnergyFraction);
  //! Calculates the XYZ position of the hit.  Returns 0 if all is OK.
  int CalculateLocalPosition(MStripHit* XSH, MStripHit* YSH, MVector& GlobalPosition, MVector& PositionResolution, bool BadDepth);
  //! Converts the FWHM timing noise to FWHM depth noise
  double GetZFWHM(double CTD_s, int DetID, double Noise);

  // private methods:
 private:

  // protected members:
 protected:

  std::vector<MString> m_DetectorNames;
  std::vector<double> m_Thicknesses;
  uint64_t m_NoError;
  uint64_t m_Error1;
  uint64_t m_Error2;
  uint64_t m_Error3;
  uint64_t m_Error4;
  uint64_t m_ErrorSH;
  double m_TimingNoiseFWHM;
  vector<MDVolume*> m_DetectorVolumes;
  MModuleEnergyCalibrationUniversal* m_EnergyCalibration;
  MDepthCalibratorB* m_DepthCalibrator;
  MString m_LookupTableFile;


  // private members:
 private:




#ifdef ___CLING___
 public:
  ClassDef(MModuleDepthCalibrationB, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
