 // This code implementation was written by Jarred Roberts
 // This Module should determine the low-energy threshold per channel, including the guard ring

#include "MModuleStripEnergyThresholdCut.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

#include <TH1D.h>
#include <TFile.h>

using namespace std;

MModuleStripEnergyThresholdCut::MModuleStripEnergyThresholdCut()
{
  //m_Name = "Strip energy threshold cut";
  //m_Description = "Applies per-strip low-energy thresholds (keV) to MReadOutAssembly strip hits";
  
  // Intentionally empty: this MEGAlib version does not expose m_Name / m_Description

}

MModuleStripEnergyThresholdCut::~MModuleStripEnergyThresholdCut() {}

bool MModuleStripEnergyThresholdCut::Initialize()
{
  if (m_ThresholdFileName.Length() == 0) {
    cout << "ERROR: MModuleStripEnergyThresholdCut: threshold file not set.\n";
    return false;
  }

  if (!LoadThresholdMap()) {
    cout << "ERROR: MModuleStripEnergyThresholdCut: failed to load threshold map: "
         << m_ThresholdFileName << "\n";
    return false;
  }

  cout << "[StripThresholdCut] Loaded " << m_Thresholds.size()
       << " thresholds from " << m_ThresholdFileName << "\n";
  cout << "[StripThresholdCut] Default threshold: " << m_DefaultThresholdKeV << " keV\n";
  return true;
}

bool MModuleStripEnergyThresholdCut::LoadThresholdMap()
{
  m_Thresholds.clear();

  ifstream in(m_ThresholdFileName.Data());
  if (!in) return false;

  string line;
  bool first = true;
  while (getline(in, line)) {
    if (line.empty()) continue;
    if (line[0] == '#') continue;

    // Skip header line (Det Side Strip Threshold_keV)
    if (first) { first = false; continue; }

    istringstream ss(line);
    StripKeyLH k;
    double thr;

    if (!(ss >> k.DetID >> k.Side >> k.Strip >> thr)) continue;
    if (k.Side != 'l' && k.Side != 'h') continue;

    if (!std::isfinite(thr) || thr <= 0) thr = m_DefaultThresholdKeV;

    m_Thresholds[k] = thr;
  }

  return true;
}

double MModuleStripEnergyThresholdCut::LookupThresholdKeV(int det, char side, int strip, bool& usedDefault) const
{
  StripKeyLH k{det, side, strip};
  auto it = m_Thresholds.find(k);
  if (it == m_Thresholds.end()) {
    usedDefault = true;
    return m_DefaultThresholdKeV;
  }
  usedDefault = false;
  return it->second;
}

bool MModuleStripEnergyThresholdCut::AnalyzeEvent(MReadOutAssembly* Event)
{
  if (Event == nullptr) return false;

  // --- Main strip hits: iterate BACKWARDS because we remove by index ---
  for (int i = (int) Event->GetNStripHits() - 1; i >= 0; --i) {
    MStripHit* SH = Event->GetStripHit((unsigned int) i);
    if (SH == nullptr) continue;

    ++m_TotalStripHits;

    const int det   = SH->GetDetectorID();
    const char side = SH->IsLowVoltageStrip() ? 'l' : 'h';
    const int strip = SH->GetStripID();   // 0-64 (guard ring=64)

    if (!m_IncludeGuardRing && strip == 64) continue;

    const double e_keV = SH->GetEnergy(); // assumes energy calibration already ran

    bool usedDefault = false;
    const double thr_keV = LookupThresholdKeV(det, side, strip, usedDefault);
    if (usedDefault) ++m_DefaultUsed;

    if (e_keV < thr_keV) {
      ++m_CutStripHits;
      Event->RemoveStripHit((unsigned int) i);
    }
  }

  // --- Optional: timing-only strip hits list ---
  if (m_CutTimingOnlyList) {
    for (int i = (int) Event->GetNStripHitsTOnly() - 1; i >= 0; --i) {
      MStripHit* SH = Event->GetStripHitTOnly((unsigned int) i);
      if (SH == nullptr) continue;

      const int det   = SH->GetDetectorID();
      const char side = SH->IsLowVoltageStrip() ? 'l' : 'h';
      const int strip = SH->GetStripID();

      if (!m_IncludeGuardRing && strip == 64) continue;

      const double e_keV = SH->GetEnergy();

      bool usedDefault = false;
      const double thr_keV = LookupThresholdKeV(det, side, strip, usedDefault);

      if (e_keV < thr_keV) {
        Event->RemoveStripHitTOnly((unsigned int) i);
      }
    }
  }

  return true;
}

void MModuleStripEnergyThresholdCut::Finalize()
{
  cout << "[StripThresholdCut] Total strip hits seen: " << m_TotalStripHits << "\n";
  cout << "[StripThresholdCut] Strip hits cut:        " << m_CutStripHits << "\n";
  cout << "[StripThresholdCut] Default used:          " << m_DefaultUsed << "\n";

  if (!m_Diagnostics) return;

  // Histogram of thresholds from the map
  TH1D hThr("hStripThresholds",
            "Per-strip thresholds;Threshold (keV);Number of strips",
            200, 0.0, 100.0);

  // Also useful: show how many strips ended up at the default value
  TH1D hThrUsed("hStripThresholdsUsed",
                "Threshold values in map (incl. defaults);Threshold (keV);Number of strips",
                200, 0.0, 100.0);

  for (const auto& kv : m_Thresholds) {
    double thr = kv.second;
    if (!std::isfinite(thr) || thr <= 0) thr = m_DefaultThresholdKeV;
    hThr.Fill(thr);
    hThrUsed.Fill(thr);
  }

  // If you want the default spike visible even when strips were missing from the map,
  // you can add m_DefaultUsed entries:
  for (long long i = 0; i < m_DefaultUsed; ++i) {
    hThrUsed.Fill(m_DefaultThresholdKeV);
  }

  TFile f(m_DiagnosticsRootFile.Data(), "RECREATE");
  hThr.Write();
  hThrUsed.Write();
  f.Close();

  cout << "[StripThresholdCut] Wrote diagnostics ROOT file: "
       << m_DiagnosticsRootFile << "\n";
}
