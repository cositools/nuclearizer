/*
 * UTNFITSWriterL1a.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <functional>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// CCfits:
#include <CCfits/CCfits>

// ROOT:
#include "TSystem.h"

// MEGAlib:
#include "MGlobal.h"
#include "MUnitTest.h"
#include "MFile.h"
#include "MTime.h"

// Nuclearizer:
#include "MFITSWriterL1a.h"
#include "MModuleLoaderMeasurementsFITS.h"
#include "MReadOutAssembly.h"
#include "MStripHit.h"


//! Unit test class for MFITSWriterL1a
class UTNFITSWriterL1a : public MUnitTest
{
 public:
  //! Default constructor
  UTNFITSWriterL1a() : MUnitTest("UTNFITSWriterL1a") {}
  //! Default destructor
  virtual ~UTNFITSWriterL1a() {}

  //! Run all tests
  virtual bool Run();

 private:
  //! Test Create() and the header keywords
  bool TestCreate();
  //! Test the written rows
  bool TestWriteRows();
  //! Test writing more events than one batch
  bool TestBatches();
  //! Test explicit FlushBatch() calls between writes
  bool TestFlushBatch();
  //! Test the time keywords written by Close()
  bool TestTimeKeywords();
  //! Test events which only have a UTC time
  bool TestUTCOnlyEvents();
  //! Test writing to an already existing file
  bool TestOverwrite();
  //! Test the error and misuse cases
  bool TestErrors();

  //! Create an event with NHits strip hits
  MReadOutAssembly* CreateEvent(unsigned long ID, double TimeRTS, unsigned int NHits);
  //! Return the number of rows of the first extension, -1 on error
  long CountRows(const MString& FileName);
  //! Return the number of extensions, -1 on error
  long CountExtensions(const MString& FileName);
  //! Read a string keyword of the given HDU (0 = primary, 1 = first extension)
  MString ReadKey(const MString& FileName, int HDU, const MString& Key);
  //! Read a double keyword of the first extension, returns g_DoubleNotDefined on error
  double ReadDoubleKey(const MString& FileName, const MString& Key);
  //! Read all events of an L1a file with the FITS loader; the events have to be deleted by the caller
  vector<MReadOutAssembly*> ReadWithLoader(const MString& FileName);
  //! Run a function in a child process; returns true if the child exited normally with the function returning true
  bool RunInChild(function<bool()> Function);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNFITSWriterL1a::Run()
{
  bool Passed = true;

  Passed = TestCreate() && Passed;
  Passed = TestWriteRows() && Passed;
  Passed = TestBatches() && Passed;
  Passed = TestFlushBatch() && Passed;
  Passed = TestTimeKeywords() && Passed;
  Passed = TestUTCOnlyEvents() && Passed;
  Passed = TestOverwrite() && Passed;
  Passed = TestErrors() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* UTNFITSWriterL1a::CreateEvent(unsigned long ID, double TimeRTS, unsigned int NHits)
{
  MReadOutAssembly* E = new MReadOutAssembly();
  E->SetID(ID);
  E->SetTimeRTS(MTime(TimeRTS));
  for (unsigned int h = 0; h < NHits; ++h) {
    MStripHit* SH = new MStripHit();
    SH->SetDetectorID(h % 16);
    SH->SetStripID(h % 64);
    SH->IsLowVoltageStrip(h % 2 == 0);
    SH->HasFastTiming(h % 3 == 0);
    SH->SetADCUnits(1000 + h);
    SH->SetTAC(2000 + h);
    E->AddStripHit(SH);
  }
  return E;
}


////////////////////////////////////////////////////////////////////////////////


long UTNFITSWriterL1a::CountRows(const MString& FileName)
{
  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, true);
    return F.extension(1).rows();
  } catch (const CCfits::FitsException&) {
    return -1;
  }
}


////////////////////////////////////////////////////////////////////////////////


long UTNFITSWriterL1a::CountExtensions(const MString& FileName)
{
  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, false);
    return F.extension().size();
  } catch (const CCfits::FitsException&) {
    return -1;
  }
}


////////////////////////////////////////////////////////////////////////////////


MString UTNFITSWriterL1a::ReadKey(const MString& FileName, int HDU, const MString& Key)
{
  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, true);
    string Value;
    if (HDU == 0) {
      F.pHDU().readKey(Key.GetString(), Value);
    } else {
      F.extension(HDU).readKey(Key.GetString(), Value);
    }
    return MString(Value);
  } catch (const CCfits::FitsException&) {
    return MString("");
  }
}


////////////////////////////////////////////////////////////////////////////////


double UTNFITSWriterL1a::ReadDoubleKey(const MString& FileName, const MString& Key)
{
  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, true);
    double Value = 0;
    F.extension(1).readKey(Key.GetString(), Value);
    return Value;
  } catch (const CCfits::FitsException&) {
    return g_DoubleNotDefined;
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<MReadOutAssembly*> UTNFITSWriterL1a::ReadWithLoader(const MString& FileName)
{
  vector<MReadOutAssembly*> Events;
  MModuleLoaderMeasurementsFITS L;
  L.SetFileName(FileName);
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  if (L.Initialize() == true) {
    for (unsigned int i = 0; i < 10000; ++i) {
      MReadOutAssembly* E = new MReadOutAssembly();
      if (L.AnalyzeEvent(E) == false) {
        delete E;
        break;
      }
      Events.push_back(E);
    }
    L.Finalize();
  }
  g_Verbosity = OldVerbosity;
  return Events;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNFITSWriterL1a::RunInChild(function<bool()> Function)
{
  cout.flush();
  mout.flush();

  pid_t Pid = fork();
  if (Pid < 0) return false;

  if (Pid == 0) {
    // A crash is a possible outcome here: without ROOT's handlers it ends the child without a stack trace
    gSystem->ResetSignals();
    // Timeout in seconds after which a hanging child is killed
    const unsigned int ChildTimeout = 60;
    alarm(ChildTimeout);
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


bool UTNFITSWriterL1a::TestCreate()
{
  bool Passed = true;

  MString FileName = GetTemporaryFileName("create.fits");
  MFITSWriterL1a W;
  Passed = EvaluateTrue("Create()", "new file", "Create succeeds", W.Create(FileName)) && Passed;
  W.Close();

  Passed = EvaluateTrue("Create()", "new file", "The file exists", MFile::Exists(FileName)) && Passed;
  Passed = Evaluate("Create()", "new file", "The primary HDU has TELESCOP = COSI", ReadKey(FileName, 0, "TELESCOP"), MString("COSI")) && Passed;
  Passed = Evaluate("Create()", "new file", "The primary HDU has ORIGIN = SSL", ReadKey(FileName, 0, "ORIGIN"), MString("SSL")) && Passed;
  Passed = Evaluate("Create()", "new file", "The extension is named GED_L1A", ReadKey(FileName, 1, "EXTNAME"), MString("GED_L1A")) && Passed;
  Passed = Evaluate("Create()", "new file", "The extension has ORIGIN = SSL", ReadKey(FileName, 1, "ORIGIN"), MString("SSL")) && Passed;
  Passed = Evaluate("Close()", "no events", "The table has no rows", CountRows(FileName), 0L) && Passed;

  try {
    CCfits::FITS F(FileName.GetString(), CCfits::Read, true);
    const CCfits::ColMap& Columns = F.extension(1).column();
    vector<string> Names = { "TIME", "EVENTID", "EVENTTYPE", "NUMSTRIPHIT", "HITTYPE", "DETID", "STRIPID", "SIDEID", "FASTTIME", "PHA", "TAC" };
    for (const string& N : Names) {
      Passed = EvaluateTrue("Create()", MString("column ") + N, "The column exists", Columns.find(N) != Columns.end()) && Passed;
    }
    Passed = Evaluate("Create()", "new file", "The table has 11 columns", (unsigned int) Columns.size(), 11u) && Passed;
  } catch (const CCfits::FitsException&) {
    Passed = EvaluateTrue("Create()", "new file", "The file can be opened with CCfits", false) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNFITSWriterL1a::TestWriteRows()
{
  bool Passed = true;

  MString FileName = GetTemporaryFileName("rows.fits");
  MFITSWriterL1a W;
  if (W.Create(FileName) == false) {
    return EvaluateTrue("Create()", "rows", "Create succeeds", false);
  }

  MReadOutAssembly* E1 = new MReadOutAssembly();
  E1->SetID(42);
  E1->SetTimeRTS(MTime(1234.5));
  MStripHit* Normal = new MStripHit();
  Normal->SetDetectorID(3);
  Normal->SetStripID(41);
  Normal->IsLowVoltageStrip(true);
  Normal->HasFastTiming(true);
  Normal->SetADCUnits(4053);
  Normal->SetTAC(10452);
  E1->AddStripHit(Normal);
  MStripHit* NN = new MStripHit();
  NN->SetDetectorID(3);
  NN->SetStripID(42);
  NN->IsLowVoltageStrip(false);
  NN->IsNearestNeighbor(true);
  NN->SetADCUnits(100);
  NN->SetTAC(0);
  E1->AddStripHit(NN);
  MStripHit* GR = new MStripHit();
  GR->SetDetectorID(3);
  GR->SetStripID(64);
  GR->IsLowVoltageStrip(true);
  GR->IsGuardRing(true);
  GR->SetADCUnits(3000);
  E1->AddStripHit(GR);
  Passed = EvaluateTrue("Write()", "event 42", "Write returns true", W.Write(E1)) && Passed;

  MReadOutAssembly* E2 = CreateEvent(43, 1300.0, 0);
  Passed = EvaluateTrue("Write()", "event without strip hits", "Write returns true", W.Write(E2)) && Passed;
  W.Close();
  delete E1;
  delete E2;

  Passed = Evaluate("Close()", "two events", "Two rows are written", CountRows(FileName), 2L) && Passed;

  vector<MReadOutAssembly*> R = ReadWithLoader(FileName);
  Passed = Evaluate("Write()", "two events", "Two events can be read back", (unsigned int) R.size(), 2u) && Passed;
  if (R.size() == 2) {
    Passed = Evaluate("Write()", "event 42", "EVENTID is 42", R[0]->GetID(), (unsigned long) 42) && Passed;
    Passed = EvaluateNear("Write()", "event 42", "TIME is the RTS time 1234.5 s", R[0]->GetTimeRTS().GetAsDouble(), 1234.5, 1e-6) && Passed;
    Passed = Evaluate("Write()", "event 42", "Three strip hits are stored", R[0]->GetNStripHits(), 3u) && Passed;
    Passed = Evaluate("Write()", "event 43", "No strip hits are stored", R[1]->GetNStripHits(), 0u) && Passed;
    if (R[0]->GetNStripHits() == 3) {
      MStripHit* S0 = R[0]->GetStripHit(0);
      MStripHit* S1 = R[0]->GetStripHit(1);
      MStripHit* S2 = R[0]->GetStripHit(2);
      Passed = Evaluate("Write()", "normal hit", "DETID is 3", S0->GetDetectorID(), 3u) && Passed;
      Passed = Evaluate("Write()", "normal hit", "STRIPID is 41", S0->GetStripID(), 41u) && Passed;
      Passed = EvaluateTrue("Write()", "normal hit", "SIDEID is LV", S0->IsLowVoltageStrip()) && Passed;
      Passed = EvaluateTrue("Write()", "normal hit", "FASTTIME is set", S0->HasFastTiming()) && Passed;
      Passed = EvaluateNear("Write()", "normal hit", "PHA is 4053", S0->GetADCUnits(), 4053.0, 0.5) && Passed;
      Passed = EvaluateNear("Write()", "normal hit", "TAC is 10452", S0->GetTAC(), 10452.0, 0.5) && Passed;
      Passed = EvaluateFalse("Write()", "normal hit", "HITTYPE is not nearest neighbor", S0->IsNearestNeighbor()) && Passed;
      Passed = EvaluateTrue("Write()", "nearest neighbor", "HITTYPE is nearest neighbor", S1->IsNearestNeighbor()) && Passed;
      Passed = EvaluateFalse("Write()", "nearest neighbor", "SIDEID is HV", S1->IsLowVoltageStrip()) && Passed;
      Passed = EvaluateFalse("Write()", "nearest neighbor", "FASTTIME is not set", S1->HasFastTiming()) && Passed;
      Passed = EvaluateTrue("Write()", "guard ring", "HITTYPE is guard ring", S2->IsGuardRing()) && Passed;
      Passed = Evaluate("Write()", "guard ring", "STRIPID is 64", S2->GetStripID(), 64u) && Passed;
    }
  }
  for (MReadOutAssembly* E : R) delete E;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNFITSWriterL1a::TestBatches()
{
  bool Passed = true;

  MString FileName = GetTemporaryFileName("batches.fits");
  MFITSWriterL1a W;
  if (W.Create(FileName) == false) {
    return EvaluateTrue("Create()", "batches", "Create succeeds", false);
  }
  bool AllWritten = true;
  for (unsigned long e = 1; e <= 250; ++e) {
    MReadOutAssembly* E = CreateEvent(e, 10.0 + e, 1 + e % 5);
    if (W.Write(E) == false) AllWritten = false;
    delete E;
  }
  W.Close();
  Passed = EvaluateTrue("Write()", "250 events", "All writes succeed", AllWritten) && Passed;
  Passed = Evaluate("Close()", "250 events", "250 rows are written across three batches", CountRows(FileName), 250L) && Passed;

  vector<MReadOutAssembly*> R = ReadWithLoader(FileName);
  MString IDs;
  MString HitCounts;
  MString ExpectedIDs;
  MString ExpectedHitCounts;
  for (unsigned int i = 0; i < R.size(); ++i) {
    IDs += MString(R[i]->GetID()) + " ";
    HitCounts += MString(R[i]->GetNStripHits()) + " ";
  }
  for (unsigned long e = 1; e <= 250; ++e) {
    ExpectedIDs += MString(e) + " ";
    ExpectedHitCounts += MString(1 + e % 5) + " ";
  }
  Passed = Evaluate("Write()", "250 events", "The event IDs are read back once each and in order", IDs, ExpectedIDs) && Passed;
  Passed = Evaluate("Write()", "250 events", "The strip hit counts are read back for every event", HitCounts, ExpectedHitCounts) && Passed;
  for (MReadOutAssembly* E : R) delete E;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNFITSWriterL1a::TestFlushBatch()
{
  bool Passed = true;

  MString FileName = GetTemporaryFileName("flush.fits");
  MFITSWriterL1a W;
  if (W.Create(FileName) == false) {
    return EvaluateTrue("Create()", "flush", "Create succeeds", false);
  }

  Passed = EvaluateTrue("FlushBatch()", "no pending events", "Flushing an empty batch succeeds", W.FlushBatch()) && Passed;
  for (unsigned long e = 1; e <= 3; ++e) {
    MReadOutAssembly* E = CreateEvent(e, 10.0 + e, 2);
    Passed = EvaluateTrue("Write()", MString("flush event ") + e, "Write returns true", W.Write(E)) && Passed;
    delete E;
  }
  Passed = EvaluateTrue("FlushBatch()", "3 pending events", "Flushing a partial batch succeeds", W.FlushBatch()) && Passed;
  Passed = EvaluateTrue("FlushBatch()", "after a flush", "Flushing again without new events succeeds", W.FlushBatch()) && Passed;
  for (unsigned long e = 4; e <= 5; ++e) {
    MReadOutAssembly* E = CreateEvent(e, 10.0 + e, 1);
    Passed = EvaluateTrue("Write()", MString("flush event ") + e, "Write returns true", W.Write(E)) && Passed;
    delete E;
  }
  W.Close();

  // The rows of the explicit flush and of Close() must follow each other without gaps or duplicates
  Passed = Evaluate("FlushBatch()", "3 flushed + 2 closed events", "Five rows are written", CountRows(FileName), 5L) && Passed;
  vector<MReadOutAssembly*> R = ReadWithLoader(FileName);
  MString IDs;
  for (unsigned int i = 0; i < R.size(); ++i) {
    if (i > 0) IDs += " ";
    IDs += MString(R[i]->GetID());
  }
  Passed = Evaluate("FlushBatch()", "3 flushed + 2 closed events", "The events are read back once each and in order", IDs, MString("1 2 3 4 5")) && Passed;
  for (MReadOutAssembly* E : R) delete E;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNFITSWriterL1a::TestTimeKeywords()
{
  bool Passed = true;

  MString FileName = GetTemporaryFileName("time.fits");
  MFITSWriterL1a W;
  if (W.Create(FileName) == false) {
    return EvaluateTrue("Create()", "time keywords", "Create succeeds", false);
  }
  // Out of order times: the minimum and maximum are used
  MReadOutAssembly* E1 = CreateEvent(1, 90000.0, 1);
  MReadOutAssembly* E2 = CreateEvent(2, 86400.0, 1);
  MReadOutAssembly* E3 = CreateEvent(3, 172800.5, 1);
  Passed = EvaluateTrue("Write()", "event at 90000 s", "Write returns true", W.Write(E1)) && Passed;
  Passed = EvaluateTrue("Write()", "event at 86400 s", "Write returns true", W.Write(E2)) && Passed;
  Passed = EvaluateTrue("Write()", "event at 172800.5 s", "Write returns true", W.Write(E3)) && Passed;
  W.Close();
  delete E1;
  delete E2;
  delete E3;

  Passed = EvaluateNear("Close()", "TSTART", "TSTART is the earliest event time", ReadDoubleKey(FileName, "TSTART"), 86400.0, 1e-9) && Passed;
  Passed = EvaluateNear("Close()", "TSTOP", "TSTOP is the latest event time", ReadDoubleKey(FileName, "TSTOP"), 172800.5, 1e-9) && Passed;
  // The mission epoch is 2025-01-01 00:00:00 UTC
  Passed = Evaluate("Close()", "DATE-OBS", "DATE-OBS is one day after the mission epoch", ReadKey(FileName, 1, "DATE-OBS"), MString("2025-01-02T00:00:00")) && Passed;
  Passed = Evaluate("Close()", "DATE-END", "DATE-END is two days after the mission epoch", ReadKey(FileName, 1, "DATE-END"), MString("2025-01-03T00:00:00")) && Passed;
  Passed = Evaluate("Close()", "OBS_ID", "OBS_ID is the start date", ReadKey(FileName, 1, "OBS_ID"), MString("20250102")) && Passed;
  Passed = Evaluate("Close()", "primary DATE-OBS", "The primary HDU has the same DATE-OBS", ReadKey(FileName, 0, "DATE-OBS"), MString("2025-01-02T00:00:00")) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNFITSWriterL1a::TestUTCOnlyEvents()
{
  bool Passed = true;

  // Events which come only with a UTC time (e.g. from roa files) must get a mission time, as in the L1b/L2 writer
  MString FileName = GetTemporaryFileName("utc.fits");
  MFITSWriterL1a W;
  if (W.Create(FileName) == false) {
    return EvaluateTrue("Create()", "UTC only", "Create succeeds", false);
  }
  MReadOutAssembly* E = CreateEvent(1, 0.0, 1);
  E->SetTimeRTS(MTime(0));
  E->SetTimeUTC(MTime(1735689600 + 3600));
  double Expected = E->ComputeRTSfromUTCTime(E->GetTimeUTC()).GetAsDouble();
  Passed = EvaluateTrue("Write()", "UTC time only", "Write returns true", W.Write(E)) && Passed;
  W.Close();
  delete E;

  vector<MReadOutAssembly*> R = ReadWithLoader(FileName);
  if (R.size() == 1) {
    Passed = EvaluateTrue("Write()", "UTC time only", "The TIME column is not 0", R[0]->GetTimeRTS().GetAsDouble() != 0.0) && Passed;
    Passed = EvaluateNear("Write()", "UTC time only", "The TIME column is the RTS computed from the UTC time", R[0]->GetTimeRTS().GetAsDouble(), Expected, 1e-6) && Passed;
  } else {
    Passed = Evaluate("Write()", "UTC time only", "One event can be read back", (unsigned int) R.size(), 1u) && Passed;
  }
  for (MReadOutAssembly* X : R) delete X;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNFITSWriterL1a::TestOverwrite()
{
  bool Passed = true;

  MString FileName = GetTemporaryFileName("overwrite.fits");
  {
    MFITSWriterL1a W;
    bool First = W.Create(FileName);
    Passed = EvaluateTrue("Create()", "new file", "Create succeeds", First) && Passed;
    if (First == true) {
      for (unsigned long e = 1; e <= 3; ++e) {
        MReadOutAssembly* E = CreateEvent(e, 10.0*e, 2);
        Passed = EvaluateTrue("Write()", MString("first file event ") + e, "Write returns true", W.Write(E)) && Passed;
        delete E;
      }
      W.Close();
    }
  }
  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MFITSWriterL1a W2;
  bool Second = W2.Create(FileName);
  g_Verbosity = OldVerbosity;
  if (Second == true) {
    MReadOutAssembly* E = CreateEvent(99, 5.0, 1);
    Passed = EvaluateTrue("Write()", "event 99 into the existing file", "Write returns true", W2.Write(E)) && Passed;
    delete E;
    W2.Close();
  }
  Passed = EvaluateTrue("Create()", "existing file", "Creating over an existing file succeeds", Second) && Passed;
  Passed = Evaluate("Create()", "existing file", "There is exactly one extension", CountExtensions(FileName), 1L) && Passed;
  vector<MReadOutAssembly*> R = ReadWithLoader(FileName);
  Passed = Evaluate("Create()", "existing file", "Only the new event is read back", (unsigned int) R.size(), 1u) && Passed;
  if (R.size() >= 1) {
    Passed = Evaluate("Create()", "existing file", "The event read back is the new event 99", R[0]->GetID(), (unsigned long) 99) && Passed;
  }
  for (MReadOutAssembly* E : R) delete E;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNFITSWriterL1a::TestErrors()
{
  bool Passed = true;

  int OldVerbosity = g_Verbosity;
  g_Verbosity = c_Quiet;
  MFITSWriterL1a W;
  bool BadDir = W.Create(GetTemporaryFileName("nodir") + "/out.fits");
  g_Verbosity = OldVerbosity;
  Passed = EvaluateFalse("Create()", "non-existing directory", "Create returns false", BadDir) && Passed;

  // Close() twice and Close() without Create() must be harmless
  MString CloseFile = GetTemporaryFileName("close.fits");
  bool CloseTwice = RunInChild([CloseFile]() -> bool {
    MFITSWriterL1a C;
    C.Create(CloseFile);
    C.Close();
    C.Close();
    MFITSWriterL1a N;
    N.Close();
    return true;
  });
  Passed = EvaluateTrue("Close()", "twice and without Create()", "Repeated Close() calls do not crash", CloseTwice) && Passed;

  // Writing without a file must fail gracefully instead of crashing in Close()
  bool NoFile = RunInChild([this]() -> bool {
    MFITSWriterL1a X;
    MReadOutAssembly* E = CreateEvent(1, 1.0, 1);
    X.Write(E);
    delete E;
    X.Close();
    return true;
  });
  Passed = EvaluateTrue("Write()", "without Create()", "Writing and closing without a file does not crash", NoFile) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNFITSWriterL1a Test;
  return Test.Run() == true ? 0 : 1;
}


// UTNFITSWriterL1a.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
