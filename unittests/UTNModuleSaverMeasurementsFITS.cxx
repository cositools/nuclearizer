/*
 * UTNModuleSaverMeasurementsFITS.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <algorithm>
#include <iostream>
#include <dlfcn.h>
#include <functional>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// CCfits:
#include <CCfits/CCfits>

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MXmlNode.h"
#include "MFile.h"
#include "MTime.h"
#include "MPhotoEvent.h"

// Nuclearizer:
#include "MAssembly.h"
#include "MModuleSaverMeasurementsFITS.h"
#include "MModuleLoaderMeasurementsFITS.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"
#include "MHit.h"


//! Unit test class for MModuleSaverMeasurementsFITS
class UTNModuleSaverMeasurementsFITS : public MUnitTest
{
 public:
  //! Default constructor
  UTNModuleSaverMeasurementsFITS() : MUnitTest("UTNModuleSaverMeasurementsFITS") {}
  //! Default destructor
  virtual ~UTNModuleSaverMeasurementsFITS() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test the module meta data for all three data levels
  bool TestDefaultConstruction();
  //! Test the setters/getters and the XML round trip
  bool TestConfiguration();
  //! Test the Initialize() error paths
  bool TestInitializeErrors();
  //! Test the L1a mode
  bool TestL1a();
  //! Test the L1b mode
  bool TestL1b();
  //! Test the L2 mode
  bool TestL2();
  //! Test writing more events than one batch
  bool TestBatches();
  //! Test writing the same file twice
  bool TestRewrite();

  //! Create an event with NHits hits of energy 100 + 10*h keV
  MReadOutAssembly* CreateEvent(unsigned long ID, double TimeRTS, unsigned int NHits);
  //! Write the events with a saver of the given level, return the file name
  MString WriteEvents(const MString& Name, int Level, vector<MReadOutAssembly*>& Events);
  //! Read a numeric column (all rows) via cfitsio; returns false on error
  bool ReadNumericColumn(const MString& FileName, const MString& Column, vector<vector<double>>& Rows);
  //! Return the shape of a read column as its row lengths separated by spaces, e.g. "2 1 3 1"
  MString ColumnShape(const vector<vector<double>>& Rows) const;
  //! Read a string column (all rows) via cfitsio; returns false on error
  bool ReadStringColumn(const MString& FileName, const MString& Column, vector<string>& Rows);
  //! Return the number of rows of the first extension, -1 on error
  long CountRows(const MString& FileName);
  //! Return true if the first extension has the given column
  bool HasColumn(const MString& FileName, const MString& Column);
  //! Read a string keyword of the first extension
  MString ReadKey(const MString& FileName, const MString& Key);
  //! Read a double keyword of the first extension, returns g_DoubleNotDefined on error
  double ReadDoubleKey(const MString& FileName, const MString& Key);
  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::Run()
{
  bool Passed = true;

  Passed = TestDefaultConstruction() && Passed;
  Passed = TestConfiguration() && Passed;
  Passed = TestInitializeErrors() && Passed;
  Passed = TestL1a() && Passed;
  Passed = TestL1b() && Passed;
  Passed = TestL2() && Passed;
  Passed = TestBatches() && Passed;
  Passed = TestRewrite() && Passed;

  // ShowOptionsGUI() opens an interactive ROOT GUI and is not tested

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* UTNModuleSaverMeasurementsFITS::CreateEvent(unsigned long ID, double TimeRTS, unsigned int NHits)
{
  MReadOutAssembly* E = new MReadOutAssembly();
  E->SetID(ID);
  E->SetTimeRTS(MTime(TimeRTS));
  for (unsigned int h = 0; h < NHits; ++h) {
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(0);
    SH->SetStripID(h);
    SH->IsLowVoltageStrip(true);
    SH->SetADCUnits(1000 + h);
    E->AddStripHit(SH);
    MHit* H = new MHit();
    H->SetEnergy(100.0 + 10.0*h);
    H->SetEnergyResolution(1.5);
    H->SetPosition(MVector(0.5*h, -0.25*h, 0.1*h));
    H->SetPositionResolution(MVector(0.1, 0.2, 0.3));
    H->AddStripHit(SH);
    E->AddHit(H);
  }
  return E;
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleSaverMeasurementsFITS::WriteEvents(const MString& Name, int Level, vector<MReadOutAssembly*>& Events)
{
  MString FileName = GetTemporaryFileName(Name);
  MModuleSaverMeasurementsFITS S("XmlTagUTNSaverFITS", Level, "UTN FITS saver");
  S.SetFileName(FileName);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  if (S.Initialize() == true) {
    for (MReadOutAssembly* E : Events) S.AnalyzeEvent(E);
    S.Finalize();
  }
  g_Verbosity = OldVerbosity;
  return FileName;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::ReadNumericColumn(const MString& FileName, const MString& Column, vector<vector<double>>& Rows)
{
  // The cfitsio functions are resolved at run time since the test executable is not linked against cfitsio directly
  Rows.clear();
  auto ReadColumn = reinterpret_cast<decltype(&ffgcv)>(dlsym(RTLD_DEFAULT, "ffgcv"));
  auto ReadDescriptor = reinterpret_cast<decltype(&ffgdes)>(dlsym(RTLD_DEFAULT, "ffgdes"));
  if (ReadColumn == nullptr || ReadDescriptor == nullptr) return false;

  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, false);
    CCfits::ExtHDU& T = F.extension(1);
    T.makeThisCurrent();
    CCfits::Column& C = T.column(Column.GetString());
    int Index = C.index();
    bool Variable = C.varLength();
    long Repeat = C.repeat();
    fitsfile* Pointer = F.fitsPointer();
    for (long r = 1; r <= T.rows(); ++r) {
      long N = Repeat;
      int Status = 0;
      if (Variable == true) {
        long Heap = 0;
        ReadDescriptor(Pointer, Index, r, &N, &Heap, &Status);
      }
      vector<double> Values(N, 0.0);
      if (N > 0) {
        double Null = 0;
        int AnyNull = 0;
        ReadColumn(Pointer, TDOUBLE, Index, r, 1, N, &Null, Values.data(), &AnyNull, &Status);
      }
      if (Status != 0) return false;
      Rows.push_back(Values);
    }
    return true;
  } catch (const CCfits::FitsException&) {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleSaverMeasurementsFITS::ColumnShape(const vector<vector<double>>& Rows) const
{
  MString Shape;
  for (unsigned int r = 0; r < Rows.size(); ++r) {
    if (r > 0) Shape += " ";
    Shape += MString((unsigned int) Rows[r].size());
  }
  return Shape;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::ReadStringColumn(const MString& FileName, const MString& Column, vector<string>& Rows)
{
  Rows.clear();
  auto ReadColumn = reinterpret_cast<decltype(&ffgcv)>(dlsym(RTLD_DEFAULT, "ffgcv"));
  if (ReadColumn == nullptr) return false;

  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, false);
    CCfits::ExtHDU& T = F.extension(1);
    T.makeThisCurrent();
    CCfits::Column& C = T.column(Column.GetString());
    int Index = C.index();
    // For string columns the repeat count can be 1 while the width is the string length
    long Width = max(max((long) C.width(), (long) C.repeat()), 1024L);
    fitsfile* Pointer = F.fitsPointer();
    for (long r = 1; r <= T.rows(); ++r) {
      vector<char> Buffer(Width + 1, '\0');
      vector<char*> Array = { Buffer.data() };
      vector<char> Empty(1, '\0');
      int AnyNull = 0;
      int Status = 0;
      ReadColumn(Pointer, TSTRING, Index, r, 1, 1, Empty.data(), Array.data(), &AnyNull, &Status);
      if (Status != 0) return false;
      Rows.push_back(string(Buffer.data()));
    }
    return true;
  } catch (const CCfits::FitsException&) {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


long UTNModuleSaverMeasurementsFITS::CountRows(const MString& FileName)
{
  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, false);
    return F.extension(1).rows();
  } catch (const CCfits::FitsException&) {
    return -1;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::HasColumn(const MString& FileName, const MString& Column)
{
  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, false);
    const CCfits::ColMap& Columns = F.extension(1).column();
    return Columns.find(Column.GetString()) != Columns.end();
  } catch (const CCfits::FitsException&) {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


MString UTNModuleSaverMeasurementsFITS::ReadKey(const MString& FileName, const MString& Key)
{
  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, false);
    string Value;
    F.extension(1).readKey(Key.GetString(), Value);
    return MString(Value);
  } catch (const CCfits::FitsException&) {
    return MString("");
  }
}


////////////////////////////////////////////////////////////////////////////////


double UTNModuleSaverMeasurementsFITS::ReadDoubleKey(const MString& FileName, const MString& Key)
{
  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, false);
    double Value = 0;
    F.extension(1).readKey(Key.GetString(), Value);
    return Value;
  } catch (const CCfits::FitsException&) {
    return g_DoubleNotDefined;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // Kill a hanging child process after this time
    const unsigned int ChildTimeoutSeconds = 60;
    alarm(ChildTimeoutSeconds);
    g_Verbosity = c_Quiet;
    bool Result = Function();
    _exit(Result == true ? 0 : 1);
  }

  int Status = 0;
  if (waitpid(Pid, &Status, 0) != Pid) return false;
  if (WIFEXITED(Status) == false) return false;
  return WEXITSTATUS(Status) == 0;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::TestDefaultConstruction()
{
  bool Passed = true;

  MModuleSaverMeasurementsFITS D;
  Passed = Evaluate("MModuleSaverMeasurementsFITS()", "default", "The default XML tag is 'XmlTagSaverMeasurementsFITS'", D.GetXmlTag(), MString("XmlTagSaverMeasurementsFITS")) && Passed;
  Passed = EvaluateTrue("ProvidesModuleType()", "default", "The module provides c_EventSaver", D.ProvidesModuleType(MAssembly::c_EventSaver)) && Passed;
  Passed = EvaluateFalse("IsTypeExclusive()", "default", "Multiple savers are allowed", D.IsTypeExclusive()) && Passed;
  Passed = EvaluateTrue("HasOptionsGUI()", "default", "The module has an options GUI", D.HasOptionsGUI()) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default (L1b)", "The strip pairing is required", D.IsHardPreceedingModule(MAssembly::c_StripPairing)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default (L1b)", "The depth correction is required", D.IsHardPreceedingModule(MAssembly::c_DepthCorrection)) && Passed;
  Passed = EvaluateTrue("IsHardPreceedingModule()", "default (L1b)", "The event reconstruction is required", D.IsHardPreceedingModule(MAssembly::c_EventReconstruction)) && Passed;

  MModuleSaverMeasurementsFITS A("XmlTagSaverMeasurementsFITSL1a", 0, "L1a saver");
  Passed = Evaluate("MModuleSaverMeasurementsFITS(...)", "L1a", "The XML tag is the given one", A.GetXmlTag(), MString("XmlTagSaverMeasurementsFITSL1a")) && Passed;
  Passed = Evaluate("MModuleSaverMeasurementsFITS(...)", "L1a", "The name is the given one", A.GetName(), MString("L1a saver")) && Passed;
  Passed = Evaluate("MModuleSaverMeasurementsFITS(...)", "L1a", "Only the measurement loader is required", A.GetNPreceedingModuleTypes(), 1u) && Passed;
  if (A.GetNPreceedingModuleTypes() == 1) {
    Passed = EvaluateTrue("MModuleSaverMeasurementsFITS(...)", "L1a", "The measurement loader precedes the L1a saver", A.GetPreceedingModuleType(0) == MAssembly::c_EventLoaderMeasurement) && Passed;
    Passed = EvaluateTrue("MModuleSaverMeasurementsFITS(...)", "L1a", "The measurement loader must immediately precede the L1a saver", A.GetPreceedingModuleImmediateRequirement(0)) && Passed;
  }

  MModuleSaverMeasurementsFITS L2("XmlTagSaverMeasurementsFITSL2", 2, "L2 saver");
  Passed = EvaluateTrue("MModuleSaverMeasurementsFITS(...)", "L2", "The event reconstruction is required", L2.IsHardPreceedingModule(MAssembly::c_EventReconstruction)) && Passed;

  MModuleSaverMeasurementsFITS* C = A.Clone();
  Passed = EvaluateTrue("Clone()", "L1a", "Clone() returns an object", C != nullptr) && Passed;
  Passed = EvaluateTrue("Clone()", "L1a", "Clone() returns a distinct object", C != &A) && Passed;
  if (C != nullptr) {
    Passed = Evaluate("Clone()", "L1a", "The clone has the same XML tag", C->GetXmlTag(), A.GetXmlTag()) && Passed;
    Passed = Evaluate("Clone()", "L1a", "The clone has the same name", C->GetName(), A.GetName()) && Passed;
    Passed = Evaluate("Clone()", "L1a", "The clone has the same data level (same requirements)", C->GetNPreceedingModuleTypes(), A.GetNPreceedingModuleTypes()) && Passed;
  }
  delete C;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::TestConfiguration()
{
  bool Passed = true;

  MModuleSaverMeasurementsFITS Writer;
  Writer.SetFileName("/data/out.fits");
  Passed = Evaluate("SetFileName/GetFileName", "/data/out.fits", "GetFileName returns the set name", Writer.GetFileName(), MString("/data/out.fits")) && Passed;
  MXmlNode* Node = Writer.CreateXmlConfiguration();
  MModuleSaverMeasurementsFITS Reader;
  Passed = EvaluateTrue("ReadXmlConfiguration()", "file name", "ReadXmlConfiguration returns true", Reader.ReadXmlConfiguration(Node)) && Passed;
  Passed = Evaluate("ReadXmlConfiguration()", "file name", "The file name survives", Reader.GetFileName(), MString("/data/out.fits")) && Passed;
  delete Node;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::TestInitializeErrors()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MModuleSaverMeasurementsFITS NoName;
  bool NoNameResult = NoName.Initialize();
  MModuleSaverMeasurementsFITS BadDir;
  BadDir.SetFileName(GetTemporaryFileName("nodir") + "/out.fits");
  bool BadDirResult = BadDir.Initialize();
  MModuleSaverMeasurementsFITS BadDirL1a("XmlTagSaverMeasurementsFITSL1a", 0, "L1a saver");
  BadDirL1a.SetFileName(GetTemporaryFileName("nodir") + "/out.fits");
  bool BadDirL1aResult = BadDirL1a.Initialize();
  g_Verbosity = OldVerbosity;

  Passed = EvaluateFalse("Initialize()", "no file name", "Initialize returns false", NoNameResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "non-existing directory (L1b)", "Initialize returns false", BadDirResult) && Passed;
  Passed = EvaluateFalse("Initialize()", "non-existing directory (L1a)", "Initialize returns false", BadDirL1aResult) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::TestL1a()
{
  bool Passed = true;

  vector<MReadOutAssembly*> Events = { CreateEvent(7, 50.0, 2), CreateEvent(8, 51.0, 3) };
  MString File = WriteEvents("l1a.fits", 0, Events);
  Passed = EvaluateTrue("AnalyzeEvent()", "L1a", "The c_EventSaver analysis progress is set", Events[0]->HasAnalysisProgress(MAssembly::c_EventSaver)) && Passed;
  for (MReadOutAssembly* E : Events) delete E;

  Passed = Evaluate("Finalize()", "L1a", "The extension is GED_L1A", ReadKey(File, "EXTNAME"), MString("GED_L1A")) && Passed;
  Passed = Evaluate("Finalize()", "L1a", "Two rows are written", CountRows(File), 2L) && Passed;

  MModuleLoaderMeasurementsFITS L;
  L.SetFileName(File);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  if (L.Initialize() == true) {
    MReadOutAssembly R;
    L.AnalyzeEvent(&R);
    Passed = Evaluate("AnalyzeEvent()", "L1a read back", "The event ID survives", R.GetID(), (unsigned long) 7) && Passed;
    Passed = Evaluate("AnalyzeEvent()", "L1a read back", "The strip hits survive", R.GetNStripHits(), 2u) && Passed;
    L.Finalize();
  } else {
    Passed = EvaluateTrue("Initialize()", "L1a read back", "The L1a file can be read by the FITS loader", false) && Passed;
  }
  g_Verbosity = OldVerbosity;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::TestL1b()
{
  bool Passed = true;

  MReadOutAssembly* E1 = CreateEvent(1, 86400.0, 2);
  E1->SetGuardRingVeto(true);
  E1->SetEnergyCalibrationError("calibration missing");
  MReadOutAssembly* E2 = CreateEvent(2, 86410.0, 1);
  MPhotoEvent P;
  P.SetEnergy(100.0);
  E2->SetPhysicalEvent(&P);
  MReadOutAssembly* E3 = CreateEvent(3, 86420.0, 3);
  E3->SetShieldVeto(true);
  MReadOutAssembly* E4 = CreateEvent(4, 0.0, 1);
  E4->SetTimeRTS(MTime(0));
  E4->SetTimeUTC(MTime(1735689600 + 86430));
  double E4Expected = E4->ComputeRTSfromUTCTime(E4->GetTimeUTC()).GetAsDouble();
  vector<MReadOutAssembly*> Events = { E1, E2, E3, E4 };
  MString File = WriteEvents("l1b.fits", 1, Events);
  for (MReadOutAssembly* E : Events) delete E;

  Passed = Evaluate("Finalize()", "L1b", "The extension is GED_L1B", ReadKey(File, "EXTNAME"), MString("GED_L1B")) && Passed;
  Passed = Evaluate("Finalize()", "L1b", "Four rows are written", CountRows(File), 4L) && Passed;
  Passed = EvaluateTrue("Finalize()", "L1b", "The L1b-only column VETO exists", HasColumn(File, "VETO")) && Passed;
  Passed = EvaluateTrue("Finalize()", "L1b", "The L1b-only column QUALITY_FLAG exists", HasColumn(File, "QUALITY_FLAG")) && Passed;
  Passed = EvaluateNear("Finalize()", "L1b", "TSTART is the first event time", ReadDoubleKey(File, "TSTART"), 86400.0, 1e-3) && Passed;

  vector<vector<double>> Time, NumHit, Energy, EnergyErr, X, Veto, Class, SeqHit;
  Passed = EvaluateTrue("AnalyzeEvent()", "L1b", "The TIME column can be read", ReadNumericColumn(File, "TIME", Time)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "L1b", "The NUMHIT column can be read", ReadNumericColumn(File, "NUMHIT", NumHit)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "L1b", "The ENERGY column can be read", ReadNumericColumn(File, "ENERGY", Energy)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "L1b", "The ENERGY_ERR column can be read", ReadNumericColumn(File, "ENERGY_ERR", EnergyErr)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "L1b", "The X column can be read", ReadNumericColumn(File, "X", X)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "L1b", "The VETO column can be read", ReadNumericColumn(File, "VETO", Veto)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "L1b", "The EVENTCLASS column can be read", ReadNumericColumn(File, "EVENTCLASS", Class)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "L1b", "The SEQHIT column can be read", ReadNumericColumn(File, "SEQHIT", SeqHit)) && Passed;
  // Shapes as row lengths: scalar columns have one element per event, hit arrays one element per hit (events with 2, 1, 3, and 1 hits)
  // The guards test the shape itself, so that the indexing stays safe even if a shape check is marked with ExpectedToFail()
  const MString Scalar = "1 1 1 1";
  const MString PerHit = "2 1 3 1";
  Passed = Evaluate("AnalyzeEvent()", "L1b", "TIME has one element per event", ColumnShape(Time), Scalar) && Passed;
  if (ColumnShape(Time) == Scalar) {
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 1", "TIME is the RTS time", Time[0][0], 86400.0, 1e-6) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 4", "TIME is computed from the UTC time if there is no RTS time", Time[3][0], E4Expected, 1e-6) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "L1b", "NUMHIT has one element per event", ColumnShape(NumHit), Scalar) && Passed;
  if (ColumnShape(NumHit) == Scalar) {
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 1", "NUMHIT is 2", NumHit[0][0], 2.0, 1e-12) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "L1b", "ENERGY has one element per hit", ColumnShape(Energy), PerHit) && Passed;
  if (ColumnShape(Energy) == PerHit) {
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 1", "The hit energies are written", Energy[0][1], 110.0, 1e-4) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "L1b", "ENERGY_ERR has one element per hit", ColumnShape(EnergyErr), PerHit) && Passed;
  if (ColumnShape(EnergyErr) == PerHit) {
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 1", "The hit energy resolutions are written", EnergyErr[0][1], 1.5, 1e-4) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "L1b", "X has one element per hit", ColumnShape(X), PerHit) && Passed;
  if (ColumnShape(X) == PerHit) {
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 1", "The hit X positions are written", X[0][1], 0.5, 1e-4) && Passed;
  }
  // VETO: 0 = none, 1 = hard ACD veto, 2 = soft ACD veto, 3 = guard ring veto
  Passed = Evaluate("AnalyzeEvent()", "L1b", "VETO has one element per event", ColumnShape(Veto), Scalar) && Passed;
  if (ColumnShape(Veto) == Scalar) {
    Passed = ExpectedToFail(EvaluateNear("AnalyzeEvent()", "L1b event 1 with guard ring veto", "VETO is 3 for a guard ring veto", Veto[0][0], 3.0, 1e-12)) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 2 without veto", "VETO is 0 without veto", Veto[1][0], 0.0, 1e-12) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "L1b event 3 with shield veto", "VETO is an ACD veto (1 or 2) for a shield veto", Veto[2][0] == 1.0 || Veto[2][0] == 2.0) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "L1b", "EVENTCLASS has one element per event", ColumnShape(Class), Scalar) && Passed;
  if (ColumnShape(Class) == Scalar) {
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 1 without physical event", "EVENTCLASS is 5 (unknown)", Class[0][0], 5.0, 1e-12) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 2 with photo event", "EVENTCLASS is 1 (photo absorption)", Class[1][0], 1.0, 1e-12) && Passed;
  }
  Passed = Evaluate("AnalyzeEvent()", "L1b", "SEQHIT has one element per hit", ColumnShape(SeqHit), PerHit) && Passed;
  if (ColumnShape(SeqHit) == PerHit) {
    Passed = EvaluateNear("AnalyzeEvent()", "L1b event 2 with photo event", "SEQHIT of the single hit is 1", SeqHit[1][0], 1.0, 1e-12) && Passed;
  }
  vector<string> Quality;
  Passed = EvaluateTrue("AnalyzeEvent()", "L1b", "The QUALITY_FLAG column can be read", ReadStringColumn(File, "QUALITY_FLAG", Quality)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "L1b", "QUALITY_FLAG has one row per event", (unsigned int) Quality.size(), 4u) && Passed;
  if (Quality.size() == 4) {
    Passed = EvaluateFalse("AnalyzeEvent()", "L1b event 1 with calibration error", "QUALITY_FLAG is set for a bad event", MString(Quality[0]).Strip().IsEmpty()) && Passed;
    Passed = EvaluateTrue("AnalyzeEvent()", "L1b event 2 good", "QUALITY_FLAG is empty for a good event", MString(Quality[1]).Strip().IsEmpty()) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::TestL2()
{
  bool Passed = true;

  MReadOutAssembly* E1 = CreateEvent(1, 100.0, 2);
  MReadOutAssembly* E2 = CreateEvent(2, 101.0, 1);
  E2->SetStripPairingError("bad pairing");
  MReadOutAssembly* E3 = CreateEvent(3, 102.0, 10);
  vector<MReadOutAssembly*> Events = { E1, E2, E3 };
  MString File = WriteEvents("l2.fits", 2, Events);
  Passed = EvaluateTrue("AnalyzeEvent()", "L2 bad event", "Skipped events still get the c_EventSaver analysis progress", E2->HasAnalysisProgress(MAssembly::c_EventSaver)) && Passed;
  for (MReadOutAssembly* E : Events) delete E;

  Passed = Evaluate("Finalize()", "L2", "The extension is GED_L2", ReadKey(File, "EXTNAME"), MString("GED_L2")) && Passed;
  Passed = Evaluate("Finalize()", "L2", "Bad events are screened out: two rows", CountRows(File), 2L) && Passed;
  Passed = EvaluateFalse("Finalize()", "L2", "There is no VETO column", HasColumn(File, "VETO")) && Passed;
  Passed = EvaluateFalse("Finalize()", "L2", "There is no QUALITY_FLAG column", HasColumn(File, "QUALITY_FLAG")) && Passed;

  vector<vector<double>> ID, Energy;
  Passed = EvaluateTrue("AnalyzeEvent()", "L2", "The EVENTID column can be read", ReadNumericColumn(File, "EVENTID", ID)) && Passed;
  Passed = EvaluateTrue("AnalyzeEvent()", "L2", "The ENERGY column can be read", ReadNumericColumn(File, "ENERGY", Energy)) && Passed;
  Passed = Evaluate("AnalyzeEvent()", "L2", "EVENTID has one element per written event", ColumnShape(ID), MString("1 1")) && Passed;
  if (ColumnShape(ID) == "1 1") {
    Passed = EvaluateNear("AnalyzeEvent()", "L2", "The first row is event 1", ID[0][0], 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "L2", "The second row is event 3", ID[1][0], 3.0, 1e-12) && Passed;
  }
  // L2 hit arrays have the fixed length 10
  Passed = Evaluate("AnalyzeEvent()", "L2", "ENERGY has the fixed length 10 per written event", ColumnShape(Energy), MString("10 10")) && Passed;
  if (ColumnShape(Energy) == "10 10") {
    Passed = EvaluateNear("AnalyzeEvent()", "L2 event 1", "The second hit energy is written", Energy[0][1], 110.0, 1e-4) && Passed;
    Passed = EvaluateNear("AnalyzeEvent()", "L2 event 1", "Unused entries are zero-padded", Energy[0][5], 0.0, 1e-12) && Passed;
  }

  // More than 10 hits do not fit into the fixed-length L2 arrays: this must not corrupt memory
  MString Big = GetTemporaryFileName("l2big.fits");
  bool NoCrash = RunInChild([this, Big]() -> bool {
    MModuleSaverMeasurementsFITS S("XmlTagUTNSaverFITSL2", 2, "L2");
    S.SetFileName(Big);
    if (S.Initialize() == false) return false;
    MReadOutAssembly* E = CreateEvent(1, 1.0, 40);
    S.AnalyzeEvent(E);
    S.Finalize();
    delete E;
    // NUMHIT must be consistent with the fixed array length
    vector<vector<double>> NumHit;
    if (ReadNumericColumn(Big, "NUMHIT", NumHit) == false) return false;
    for (auto& Row : NumHit) {
      if (Row.size() != 1 || Row[0] > 10) return false;
    }
    return true;
  });
  Passed = ExpectedToFail(EvaluateTrue("AnalyzeEvent()", "L2 event with 40 hits", "An event with more than 10 hits is handled without crashing and NUMHIT fits the 10-element arrays", NoCrash)) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::TestBatches()
{
  bool Passed = true;

  vector<MReadOutAssembly*> Events;
  for (unsigned long e = 1; e <= 230; ++e) Events.push_back(CreateEvent(e, 10.0 + e, 1 + e % 4));
  MString File = WriteEvents("batches.fits", 1, Events);
  for (MReadOutAssembly* E : Events) delete E;

  Passed = Evaluate("Finalize()", "230 events", "230 rows are written", CountRows(File), 230L) && Passed;
  vector<vector<double>> ID;
  bool ReadID = ReadNumericColumn(File, "EVENTID", ID);
  Passed = EvaluateTrue("AnalyzeEvent()", "230 events", "The EVENTID column can be read", ReadID) && Passed;
  MString Scalar;
  for (unsigned int i = 0; i < 230; ++i) Scalar += (i == 0) ? "1" : " 1";
  Passed = Evaluate("AnalyzeEvent()", "230 events", "EVENTID has 230 rows with one element each", ColumnShape(ID), Scalar) && Passed;
  if (ColumnShape(ID) == Scalar) {
    bool InOrder = true;
    for (unsigned int i = 0; i < ID.size(); ++i) {
      if (ID[i][0] != i + 1) InOrder = false;
    }
    Passed = EvaluateTrue("AnalyzeEvent()", "230 events", "The event IDs are in order across batches", InOrder) && Passed;
  }
  Passed = EvaluateNear("Finalize()", "230 events", "TSTOP is the last event time", ReadDoubleKey(File, "TSTOP"), 240.0, 1e-6) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNModuleSaverMeasurementsFITS::TestRewrite()
{
  bool Passed = true;

  MString FileName = GetTemporaryFileName("rewrite.fits");
  MModuleSaverMeasurementsFITS S;
  S.SetFileName(FileName);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;

  vector<bool> Init(2, false);
  for (unsigned int Round = 0; Round < Init.size(); ++Round) {
    Init[Round] = S.Initialize();
    if (Init[Round] == false) continue;
    unsigned int N = (Round == 0) ? 3 : 1;
    for (unsigned int e = 1; e <= N; ++e) {
      MReadOutAssembly* E = CreateEvent(100*Round + e, 1.0*e, 1);
      S.AnalyzeEvent(E);
      delete E;
    }
    S.Finalize();
  }
  g_Verbosity = OldVerbosity;

  Passed = EvaluateTrue("Initialize()", "first run", "Initialize succeeds", Init[0]) && Passed;
  Passed = EvaluateTrue("Initialize()", "second run, same file", "Initialize succeeds again", Init[1]) && Passed;
  Passed = ExpectedToFail(Evaluate("Finalize()", "second run, same file", "The file contains only the one event of the second run", CountRows(FileName), 1L)) && Passed;
  vector<vector<double>> ID;
  Passed = EvaluateTrue("Finalize()", "second run, same file", "The EVENTID column can be read", ReadNumericColumn(FileName, "EVENTID", ID)) && Passed;
  unsigned int FirstRowLength = (ID.size() >= 1) ? (unsigned int) ID[0].size() : 0;
  Passed = Evaluate("Finalize()", "second run, same file", "The first EVENTID row has one element", FirstRowLength, 1u) && Passed;
  if (ID.size() >= 1 && ID[0].size() == 1) {
    Passed = ExpectedToFail(EvaluateNear("Finalize()", "second run, same file", "The first row is the event of the second run", ID[0][0], 101.0, 1e-12)) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNModuleSaverMeasurementsFITS Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNModuleSaverMeasurementsFITS.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
