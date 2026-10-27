/*
 * UTNSubModuleShieldEnergyCorrection.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cmath>
#include <list>
#include <vector>
using namespace std;

// ROOT:
#include "TRandom.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"

// Nuclearizer:
#include "MSubModuleShieldEnergyCorrection.h"
#include "MReadOutAssembly.h"
#include "MDEECrystalHit.h"


//! Unit test class for MSubModuleShieldEnergyCorrection
class UTNSubModuleShieldEnergyCorrection : public MUnitTest
{
 public:
  //! Default constructor
  UTNSubModuleShieldEnergyCorrection() : MUnitTest("UTNSubModuleShieldEnergyCorrection") {}
  //! Default destructor
  virtual ~UTNSubModuleShieldEnergyCorrection() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the setters/getters and the XML configuration
  bool TestConfiguration();
  //! Test the Initialize() error paths
  bool TestInitialize();
  //! Test the energy correction of a calibrated voxel
  bool TestCalibratedVoxel();
  //! Test the default correction of an uncalibrated voxel
  bool TestUncalibratedVoxel();
  //! Test the merging of crystal hits
  bool TestMerging();
  //! Test invalid input
  bool TestInvalidInput();
  //! Test that the correction is reproducible
  bool TestReproducibility();

  //! Write the correction file and return its name, or an empty string on failure
  MString WriteCorrectionFile();
  //! Initialize a module with the standard file
  bool InitializeStandard(MSubModuleShieldEnergyCorrection& M);
  //! Create a crystal hit the same way MSubModuleDEEIntake does
  MDEECrystalHit CreateHit(const MString& DetectorID, unsigned int CrystalID, MVector Voxel, double Energy);
  //! Seed gRandom as MAssembly does, so the DEE random draws are reproducible
  void SeedRandom();
  //! Seed gRandom, run a fresh module over N events, and return the corrected energies
  vector<double> RunSeededSequence(unsigned int N);

  //! The seed MAssembly uses for gRandom
  static const unsigned int c_Seed = 20170912;
};


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldEnergyCorrection::Run()
{
  bool Passed = true;

  Passed = TestConfiguration() && Passed;
  Passed = TestInitialize() && Passed;
  Passed = TestCalibratedVoxel() && Passed;
  Passed = TestUncalibratedVoxel() && Passed;
  Passed = TestMerging() && Passed;
  Passed = TestInvalidInput() && Passed;
  Passed = TestReproducibility() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNSubModuleShieldEnergyCorrection::WriteCorrectionFile()
{
  // Columns: detector, crystal, voxel x, y, z, (2 unused), m, q, a, b, c
  // X0 crystal 3 voxel (1, 1, 1): centroid = 1.1 * E + 2, FWHM = sqrt(1^2) = 1 keV
  // X0 crystal 5 voxel (0, 0, 0): centroid = E, FWHM = sqrt(4 + 0.01 E) keV
  MString C;
  C += "# Shield energy correction\n";
  C += "X0 3 1 1 1 0 0 1.1 2.0 1.0 0.0 0.0\n";
  C += "X0 5 0 0 0 0 0 1.0 0.0 2.0 0.1 0.0\n";
  C += "this line has too few tokens\n";
  MString FileName = GetTemporaryFileName("shieldcorrection.txt");
  if (WriteTextFile(FileName, C) == false) return "";
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldEnergyCorrection::InitializeStandard(MSubModuleShieldEnergyCorrection& M)
{
  M.SetShieldEnergyCorrectionFileName(WriteCorrectionFile());
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool Result = M.Initialize();
  g_Verbosity = OldVerbosity;
  return Result;
}


////////////////////////////////////////////////////////////////////////////////


MDEECrystalHit UTNSubModuleShieldEnergyCorrection::CreateHit(const MString& DetectorID, unsigned int CrystalID, MVector Voxel, double Energy)
{
  MDEECrystalHit H;
  H.m_DetectorID = DetectorID;
  H.m_CrystalID = CrystalID;
  H.m_VoxelInDetector = Voxel;
  H.m_SimulatedEnergy = Energy;
  H.m_ROE.SetDetectorID(DetectorID);
  return H;
}


////////////////////////////////////////////////////////////////////////////////


void UTNSubModuleShieldEnergyCorrection::SeedRandom()
{
  gRandom->SetSeed(c_Seed);
}


////////////////////////////////////////////////////////////////////////////////


vector<double> UTNSubModuleShieldEnergyCorrection::RunSeededSequence(unsigned int N)
{
  vector<double> Energies;

  SeedRandom();
  MSubModuleShieldEnergyCorrection M;
  if (InitializeStandard(M) == false) return Energies;

  for (unsigned int i = 0; i < N; ++i) {
    MReadOutAssembly E;
    E.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), 100.0));
    M.AnalyzeEvent(&E);
    if (E.GetNDEECrystalHits() == 1) {
      Energies.push_back(E.GetDEECrystalHitListReference().front().m_Energy);
    }
  }
  M.Finalize();

  return Energies;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldEnergyCorrection::TestConfiguration()
{
  bool Passed = true;

  MSubModuleShieldEnergyCorrection M;
  Passed = Evaluate("GetShieldEnergyCorrectionFileName()", "default", "The default file name is empty", M.GetShieldEnergyCorrectionFileName(), MString("")) && Passed;
  M.SetShieldEnergyCorrectionFileName("/data/s.txt");
  Passed = Evaluate("SetShieldEnergyCorrectionFileName/GetShieldEnergyCorrectionFileName", "/data/s.txt", "Returns the set name", M.GetShieldEnergyCorrectionFileName(), MString("/data/s.txt")) && Passed;

  MXmlNode* Node = new MXmlNode(nullptr, "DEE");
  M.CreateXmlConfiguration(Node);
  MSubModuleShieldEnergyCorrection Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "file name", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "ShieldEnergyCorrectionFileName", "The file name survives", Reader.GetShieldEnergyCorrectionFileName(), MString("/data/s.txt")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldEnergyCorrection::TestInitialize()
{
  bool Passed = true;

  MString Bad = GetTemporaryFileName("bad.txt");
  Passed = EvaluateTrue("WriteTextFile()", "bad.txt", "The fixture file is written", WriteTextFile(Bad, "# only invalid lines\nX0 3 1 1\n")) && Passed;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MSubModuleShieldEnergyCorrection NoName;
  bool NoNameResult = NoName.Initialize();
  MSubModuleShieldEnergyCorrection Missing;
  Missing.SetShieldEnergyCorrectionFileName(GetTemporaryFileName("missing.txt"));
  bool MissingResult = Missing.Initialize();
  MSubModuleShieldEnergyCorrection BadFile;
  BadFile.SetShieldEnergyCorrectionFileName(Bad);
  bool BadResult = BadFile.Initialize();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "no file name", "Initialize returns false", NoNameResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "missing file", "Initialize returns false", MissingResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "no valid line", "Initialize returns false", BadResult) && Passed;

  MSubModuleShieldEnergyCorrection Good;
  Passed = EvaluateTrue("Initialize()", "valid file with one invalid line", "Initialize succeeds", InitializeStandard(Good)) && Passed;
  Good.Finalize();
  Passed = EvaluateTrue("Initialize()", "after Finalize", "A second Initialize succeeds", InitializeStandard(Good)) && Passed;
  Good.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldEnergyCorrection::TestCalibratedVoxel()
{
  bool Passed = true;

  // The module's private random generator cannot be seeded (see TestReproducibility):
  // all statistical tolerances are at least 6 sigma of the correct result
  MSubModuleShieldEnergyCorrection M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard file", "Initialize succeeds", false);
  }

  // Centroid 1.1 * 100 + 2 = 112 keV, FWHM 1 keV
  const unsigned int N = 4000;
  double Sum = 0;
  double Sum2 = 0;
  bool AllTrue = true;
  unsigned int NMissing = 0;
  for (unsigned int i = 0; i < N; ++i) {
    MReadOutAssembly E;
    E.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), 100.0));
    if (M.AnalyzeEvent(&E) == false) AllTrue = false;
    if (E.GetNDEECrystalHits() == 1) {
      double En = E.GetDEECrystalHitListReference().front().m_Energy;
      Sum += En;
      Sum2 += En*En;
    } else {
      ++NMissing;
    }
  }
  double Mean = Sum/N;
  double Sigma = sqrt(Sum2/N - Mean*Mean);
  Passed = EvaluateTrue("AnalyzeEvent()", "calibrated voxel", "AnalyzeEvent returns true", AllTrue) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "calibrated voxel", "Every event keeps its crystal hit", NMissing, 0u) && Passed;
  Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "calibrated voxel, 100 keV", "The corrected energies are centered on 1.1 * 100 + 2 = 112 keV", Mean, 112.0, 0.05)) && Passed;
  Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "calibrated voxel, 100 keV", "The width is FWHM / 2.3548 = 0.4247 keV", Sigma, 1.0/2.354820045, 0.03)) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldEnergyCorrection::TestUncalibratedVoxel()
{
  bool Passed = true;

  MSubModuleShieldEnergyCorrection M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard file", "Initialize succeeds", false);
  }

  // Unknown voxel: centroid = energy, sigma = 10 keV
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  const unsigned int N = 4000;
  double Sum = 0;
  double Sum2 = 0;
  unsigned int NMissing = 0;
  for (unsigned int i = 0; i < N; ++i) {
    MReadOutAssembly E;
    E.AddDEECrystalHit(CreateHit("Y1", 7, MVector(0, 1, 0), 500.0));
    M.AnalyzeEvent(&E);
    if (E.GetNDEECrystalHits() == 1) {
      double En = E.GetDEECrystalHitListReference().front().m_Energy;
      Sum += En;
      Sum2 += En*En;
    } else {
      ++NMissing;
    }
  }
  g_Verbosity = OldVerbosity;
  double Mean = Sum/N;
  double Sigma = sqrt(Sum2/N - Mean*Mean);
  Passed = Evaluate("AnalyzeEvent()", "uncalibrated voxel", "Every event keeps its crystal hit", NMissing, 0u) && Passed;
  Passed = EvaluateNear("AnalyzeEvent()", "uncalibrated voxel, 500 keV", "The energies are centered on the simulated energy", Mean, 500.0, 1.0) && Passed;
  Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "uncalibrated voxel, 500 keV", "The default width is 10 keV", Sigma, 10.0, 0.7)) && Passed;

  // Energies are never negative
  g_Verbosity = c_Quiet;
  bool AllPositive = true;
  NMissing = 0;
  for (unsigned int i = 0; i < 200; ++i) {
    MReadOutAssembly E;
    E.AddDEECrystalHit(CreateHit("Y1", 7, MVector(0, 1, 0), 1.0));
    M.AnalyzeEvent(&E);
    if (E.GetNDEECrystalHits() == 1) {
      if (E.GetDEECrystalHitListReference().front().m_Energy < 0) AllPositive = false;
    } else {
      ++NMissing;
    }
  }
  g_Verbosity = OldVerbosity;
  Passed = EvaluateTrue("AnalyzeEvent()", "uncalibrated voxel, 1 keV", "The corrected energy is never negative", AllPositive) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "uncalibrated voxel, 1 keV", "Every event keeps its crystal hit", NMissing, 0u) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldEnergyCorrection::TestMerging()
{
  bool Passed = true;

  MSubModuleShieldEnergyCorrection M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard file", "Initialize succeeds", false);
  }

  // Two hits in the same voxel are merged
  MReadOutAssembly Same;
  Same.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), 100.0));
  Same.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), 200.0));
  M.AnalyzeEvent(&Same);
  Passed = Evaluate("AnalyzeEvent()", "two hits in the same voxel", "The hits are merged", Same.GetNDEECrystalHits(), 1u) && Passed;
  // The correction (centroid 1.1 * E + 2, FWHM 1 keV) applies to the deposit of the voxel, i.e. to the summed 300 keV:
  // 1.1 * 300 + 2 = 332 keV; correcting each sub-hit before merging would add the offset twice (334 keV)
  double Sum = 0.0;
  unsigned int NMerged = 0;
  for (unsigned int i = 0; i < 500; ++i) {
    MReadOutAssembly Pair;
    Pair.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), 100.0));
    Pair.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), 200.0));
    M.AnalyzeEvent(&Pair);
    if (Pair.GetNDEECrystalHits() == 1) {
      Sum += Pair.GetDEECrystalHitListReference().front().m_Energy;
      ++NMerged;
    }
  }
  Passed = Evaluate("AnalyzeEvent()", "500 x two hits in the same voxel", "All pairs are merged", NMerged, 500u) && Passed;
  if (NMerged > 0) {
    Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "500 x 100 + 200 keV in the same voxel", "The merged energy is centered on the correction of the summed deposit, 1.1 * 300 + 2 = 332 keV", Sum/NMerged, 332.0, 0.2)) && Passed;
  }

  // Hits in different crystals of the same shield detector must not be merged
  MReadOutAssembly Different;
  Different.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), 100.0));
  Different.AddDEECrystalHit(CreateHit("X0", 5, MVector(0, 0, 0), 300.0));
  M.AnalyzeEvent(&Different);
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "hits in crystals 3 and 5 of X0", "Hits in different crystals are not merged", Different.GetNDEECrystalHits(), 2u)) && Passed;

  // Hits in different voxels of the same crystal must not be merged
  MReadOutAssembly Voxels;
  Voxels.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), 100.0));
  Voxels.AddDEECrystalHit(CreateHit("X0", 3, MVector(0, 1, 1), 100.0));
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  M.AnalyzeEvent(&Voxels);
  g_Verbosity = OldVerbosity;
  Passed = ExpectedToFail(Evaluate("AnalyzeEvent()", "hits in two voxels of crystal 3", "Hits in different voxels are not merged", Voxels.GetNDEECrystalHits(), 2u)) && Passed;

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldEnergyCorrection::TestInvalidInput()
{
  bool Passed = true;

  MSubModuleShieldEnergyCorrection M;
  if (InitializeStandard(M) == false) {
    return EvaluateTrue("Initialize()", "standard file", "Initialize succeeds", false);
  }

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  bool NullResult = M.AnalyzeEvent(nullptr);
  MReadOutAssembly E;
  E.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), -5.0));
  bool NegativeResult = M.AnalyzeEvent(&E);
  MReadOutAssembly Empty;
  bool EmptyResult = M.AnalyzeEvent(&Empty);
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("AnalyzeEvent()", "nullptr", "AnalyzeEvent returns false for a null event", NullResult) && Passed;
  Passed = EvaluateFalse("AnalyzeEvent()", "negative simulated energy", "AnalyzeEvent returns false for a negative simulated energy", NegativeResult) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "no crystal hits", "AnalyzeEvent returns true for an event without crystal hits", EmptyResult) && Passed;

  // Clear() between events keeps the loaded correction: the next event is still corrected
  M.Clear();
  MReadOutAssembly Next;
  Next.AddDEECrystalHit(CreateHit("X0", 3, MVector(1, 1, 1), 100.0));
  Passed = EvaluateTrue("AnalyzeEvent()", "after Clear()", "AnalyzeEvent returns true", M.AnalyzeEvent(&Next)) && Passed;
  Passed = Evaluate("Clear()", "next event", "The crystal hit is kept", Next.GetNDEECrystalHits(), 1u) && Passed;
  if (Next.GetNDEECrystalHits() == 1) {
    double Energy = Next.GetDEECrystalHitListReference().front().m_Energy;
    Passed = EvaluateTrue("Clear()", "next event", "The corrected energy is finite", std::isfinite(Energy)) && Passed;
    Passed = EvaluateTrue("Clear()", "next event", "The corrected energy is positive", Energy > 0) && Passed;
  }

  M.Finalize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNSubModuleShieldEnergyCorrection::TestReproducibility()
{
  bool Passed = true;

  // MAssembly seeds gRandom to reproduce DEE results: the same seed must give the same sequence of energies
  vector<double> First = RunSeededSequence(10);
  vector<double> Second = RunSeededSequence(10);
  Passed = EvaluateSize("AnalyzeEvent()", "seeded run", "Every event of the first run has one crystal hit", First.size(), 10) && Passed;
  Passed = EvaluateSize("AnalyzeEvent()", "seeded run", "Every event of the second run has one crystal hit", Second.size(), 10) && Passed;
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "two runs with the gRandom seed of MAssembly", "Reseeding gRandom reproduces the corrected energies", First == Second)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNSubModuleShieldEnergyCorrection Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNSubModuleShieldEnergyCorrection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
