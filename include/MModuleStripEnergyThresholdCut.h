#ifndef __MModuleStripEnergyThresholdCut__
#define __MModuleStripEnergyThresholdCut__

#include <map>

#include "MModule.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MString.h"

struct StripKeyLH {
  int  DetID;
  char Side;   // 'l' (LV) or 'h' (HV)
  int  Strip;  // 0-63, guard ring = 64

  bool operator<(const StripKeyLH& o) const {
    if (DetID != o.DetID) return DetID < o.DetID;
    if (Side  != o.Side)  return Side  < o.Side;
    return Strip < o.Strip;
  }
};

class MModuleStripEnergyThresholdCut : public MModule
{
public:
  MModuleStripEnergyThresholdCut();
  virtual ~MModuleStripEnergyThresholdCut();

  bool Initialize() override;
  bool AnalyzeEvent(MReadOutAssembly* Event) override;
  void Finalize() override;

  // Configuration
  void SetThresholdFileName(const MString& fn) { m_ThresholdFileName = fn; }
  void SetDefaultThresholdKeV(double thr)      { m_DefaultThresholdKeV = thr; }   // default: 40 keV

  void SetIncludeGuardRing(bool v)             { m_IncludeGuardRing = v; }        // include strip 64
  void SetCutTimingOnlyList(bool v)            { m_CutTimingOnlyList = v; }       // also cut StripHitsTOnly

  void EnableDiagnostics(bool v)               { m_Diagnostics = v; }
  void SetDiagnosticsRootFile(const MString& fn){ m_DiagnosticsRootFile = fn; }

private:
  bool LoadThresholdMap();
  double LookupThresholdKeV(int det, char side, int strip, bool& usedDefault) const;

  MString m_ThresholdFileName;
  double  m_DefaultThresholdKeV = 40.0;

  bool    m_IncludeGuardRing = true;
  bool    m_CutTimingOnlyList = false;

  bool    m_Diagnostics = true;
  MString m_DiagnosticsRootFile = "threshold_diagnostics.root";

  std::map<StripKeyLH, double> m_Thresholds;

  // Counters
  long long m_TotalStripHits = 0;
  long long m_CutStripHits   = 0;
  long long m_DefaultUsed    = 0;
};

#endif
