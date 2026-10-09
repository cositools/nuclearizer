/*
 * UTNEndToEnd_406-1.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// Standard libs:
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MFile.h"
#include "MString.h"
#include "MSystem.h"
#include "MUnitTest.h"


//! End-to-end test: run nuclearizer on the 406-1 HDF5 data and verify the .tra and .roa output, and read the .roa output back.
//!
//! Design notes:
//!  - Fully automatic: takes no command-line arguments and needs no interactive
//!    input. The data is located via $NUCLEARIZER and nuclearizer runs headless
//!    (-a runs the analysis, -n suppresses the GUI).
//!  - Parallel-safe: every path it writes is unique to this test and carries the
//!    process ID, so it can run concurrently with the other UTNEndToEnd_* tests.
class UTNEndToEnd_406_1 : public MUnitTest
{
public:
  UTNEndToEnd_406_1() : MUnitTest("UTNEndToEnd_406-1") {}
  virtual ~UTNEndToEnd_406_1() {}

  virtual bool Run();

private:
  //! Run nuclearizer on the 406-1 HDF5 input and compare the output to the reference .tra file
  bool TestHDF5ToTra();
  //! Run nuclearizer on the 406-1 HDF5 input and compare the output to the reference .roa file
  bool TestHDF5ToRoa();
  //! Read the reference .roa file with nuclearizer, write it again, and compare it to itself
  bool TestRoaToRoa();
  //! Read the reference .roa file with nuclearizer while vetoed events are not saved
  bool TestRoaToRoaWithoutVetoedEvents();
  //! Run nuclearizer with the configuration <Name>.nuclearizer.cfg, whose output is <Name>.<Suffix>, and compare it to ReferenceName
  bool TestConfiguration(const MString& Name, const MString& Suffix, const MString& ReferenceName);
  //! Run nuclearizer with the configuration <Name>.nuclearizer.cfg and redirect its output <Name>.<Suffix> to OutputFile
  bool RunConfiguration(const MString& Name, const MString& Suffix, const MString& OutputFile);
  //! Run nuclearizer with the configuration <Name>.nuclearizer.rtb.cfg and redirect its output <Name>.<Suffix> to OutputFile
  bool RunRTBConfiguration(const MString& Name, const MString& Suffix, const MString& OutputFile);
  //! Write a copy of the roa file without the events containing a BD GR Veto line
  bool RemoveVetoedEvents(const MString& InputFileName, const MString& OutputFileName);
  //! Run nuclearizer with arguments and capture stdout/stderr in a log file
  int RunNuclearizer(const MString& Arguments, const MString& LogFile);
  //! Compare generated output with the reference file
  bool CompareOutputToReference(const MString& OutputFile, const MString& ReferenceFile);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::Run()
{
  bool Passed = true;

  Passed = TestHDF5ToTra() && Passed;
  Passed = TestHDF5ToRoa() && Passed;
  Passed = TestRoaToRoa() && Passed;
  Passed = TestRoaToRoaWithoutVetoedEvents() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::TestHDF5ToTra()
{
  return TestConfiguration("hdf5-to-tra", "tra", "hdf5-to-tra.reference.tra");
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::TestHDF5ToRoa()
{
  return TestConfiguration("hdf5-to-roa", "roa", "hdf5-to-roa.reference.roa");
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::TestRoaToRoa()
{
  // The configuration reads hdf5-to-roa.reference.roa, so writing it again has to reproduce it
  return TestConfiguration("roa-to-roa", "roa", "hdf5-to-roa.reference.roa");
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::TestConfiguration(const MString& Name, const MString& Suffix, const MString& ReferenceName)
{
  bool Passed = true;

  const char* NuclearizerEnv = getenv("NUCLEARIZER");
  Passed = EvaluateTrue("End-to-end test 406-1", "environment variable",
                        "$NUCLEARIZER environment variable must be set",
                        NuclearizerEnv != nullptr && NuclearizerEnv[0] != '\0') && Passed;
  if (Passed == false) return Passed;

  MString DataDir       = MString(NuclearizerEnv) + "/resource/unittestdata/406-1";
  MString ReferenceFile = DataDir + "/" + ReferenceName;
  MString OutputFile    = MString("/tmp/UTNEndToEnd_406-1_") + Name + "_" + (unsigned int) getpid() + "." + Suffix;

  Passed = EvaluateTrue("End-to-end test 406-1", Name + " reference file",
                        "The reference file exists",
                        MFile::Exists(ReferenceFile)) && Passed;
  if (Passed == false) return Passed;

  Passed = RunConfiguration(Name, Suffix, OutputFile) && Passed;
  if (Passed == false) return Passed;

  if (Name.BeginsWith("hdf5") == true) {
    Passed = RunRTBConfiguration(Name, Suffix, OutputFile) && Passed;
    if (Passed == false) return Passed;
  }

  // Compare output to reference line by line
  Passed = CompareOutputToReference(OutputFile, ReferenceFile) && Passed;

  MFile::Remove(OutputFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::TestRoaToRoaWithoutVetoedEvents()
{
  bool Passed = true;

  const char* NuclearizerEnv = getenv("NUCLEARIZER");
  Passed = EvaluateTrue("End-to-end test 406-1", "environment variable",
                        "$NUCLEARIZER environment variable must be set",
                        NuclearizerEnv != nullptr && NuclearizerEnv[0] != '\0') && Passed;
  if (Passed == false) return Passed;

  MString DataDir       = MString(NuclearizerEnv) + "/resource/unittestdata/406-1";
  MString ReferenceFile = DataDir + "/hdf5-to-roa.reference.roa";
  MString OutputFile    = MString("/tmp/UTNEndToEnd_406-1_roa-to-roa-novetoes_") + (unsigned int) getpid() + ".roa";
  MString ExpectedFile  = MString("/tmp/UTNEndToEnd_406-1_roa-to-roa-novetoes_expected_") + (unsigned int) getpid() + ".roa";

  Passed = RunConfiguration("roa-to-roa-novetoes", "roa", OutputFile) && Passed;
  if (Passed == false) return Passed;

  // The BD GR Veto lines are read back as the guard ring veto, thus those events are not saved
  Passed = EvaluateTrue("End-to-end test 406-1", "roa-to-roa-novetoes expected file",
                        "The events without a guard ring veto can be extracted from the reference",
                        RemoveVetoedEvents(ReferenceFile, ExpectedFile)) && Passed;
  Passed = EvaluateFalse("End-to-end test 406-1", "roa-to-roa-novetoes output file",
                         "The output contains no vetoed event",
                         ReadTextFile(OutputFile).Contains("BD GR Veto")) && Passed;
  Passed = EvaluateFilesNumericallyEquivalent("End-to-end test 406-1", "roa-to-roa-novetoes output file",
                                              "All events without a guard ring veto are saved unchanged",
                                              OutputFile, ExpectedFile) && Passed;

  MFile::Remove(OutputFile);
  MFile::Remove(ExpectedFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::RemoveVetoedEvents(const MString& InputFileName, const MString& OutputFileName)
{
  ifstream In(InputFileName.Data());
  ofstream Out(OutputFileName.Data());
  if (In.is_open() == false || Out.is_open() == false) return false;

  // Events are buffered from SE to the next SE, and dropped if they contain a BD GR Veto line
  vector<string> Event;
  bool IsInEvent = false;
  bool IsVetoed = false;
  string Line;
  while (getline(In, Line)) {
    if (Line == "SE" || Line == "EN") {
      if (IsInEvent == true && IsVetoed == false) {
        for (const string& E: Event) Out<<E<<endl;
      }
      Event.clear();
      IsVetoed = false;
      IsInEvent = (Line == "SE");
    }
    if (IsInEvent == true) {
      Event.push_back(Line);
      if (Line == "BD GR Veto") IsVetoed = true;
    } else {
      Out<<Line<<endl;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::RunConfiguration(const MString& Name, const MString& Suffix, const MString& OutputFile)
{
  bool Passed = true;

  MString DataDir        = MString(getenv("NUCLEARIZER")) + "/resource/unittestdata/406-1";
  MString ConfigFile     = DataDir + "/" + Name + ".nuclearizer.cfg";
  MString TestConfigFile = MString("/tmp/UTNEndToEnd_406-1_") + Name + "_" + (unsigned int) getpid() + ".cfg";

  // The log file name carries the process ID so concurrent end-to-end tests
  // (and repeated runs) never share or clobber the same log.
  MString LogFile = MString("/tmp/UTNEndToEnd_406-1_") + Name + "_" + (unsigned int) getpid() + ".log";

  Passed = EvaluateTrue("End-to-end test 406-1", Name + " config file",
                        "The nuclearizer config file exists",
                        MFile::Exists(ConfigFile)) && Passed;
  if (Passed == false) return Passed;

  ifstream ConfigIn(ConfigFile.Data());
  Passed = EvaluateTrue("End-to-end test 406-1", Name + " temporary config input",
                        "The nuclearizer config file can be opened for reading",
                        ConfigIn.is_open()) && Passed;
  ofstream ConfigOut(TestConfigFile.Data());
  Passed = EvaluateTrue("End-to-end test 406-1", Name + " temporary config output",
                        "The temporary nuclearizer config file can be opened for writing",
                        ConfigOut.is_open()) && Passed;
  if (Passed == false) return Passed;

  const MString ConfigOutputFile = "$(NUCLEARIZER)/resource/unittestdata/406-1/" + Name + "." + Suffix;
  string Line;
  while (getline(ConfigIn, Line)) {
    MString ConfigLine(Line.c_str());
    ConfigLine.ReplaceAllInPlace(ConfigOutputFile, OutputFile);
    ConfigOut << ConfigLine << endl;
  }
  ConfigIn.close();
  ConfigOut.close();

  // Remove any stale output from a previous run so the checks below reflect
  // strictly what this run produced.
  if (MFile::Exists(OutputFile) == true) {
    MFile::Remove(OutputFile);
  }

  // Run nuclearizer fully automatically; stdout and stderr are captured to LogFile
  MString NuclearizerArguments = MString("-c ") + TestConfigFile + " -a -n";
  int Status = RunNuclearizer(NuclearizerArguments, LogFile);
  Passed = EvaluateTrue("End-to-end test 406-1", Name + " exit status",
                        "nuclearizer exits with status 0 (log: " + LogFile + ")",
                        Status == 0) && Passed;

  // Verify the output file was produced by this run
  Passed = EvaluateTrue("End-to-end test 406-1", Name + " output file",
                        "The output file of " + Name + " was created",
                        MFile::Exists(OutputFile)) && Passed;

  MFile::Remove(TestConfigFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::RunRTBConfiguration(const MString& Name, const MString& Suffix, const MString& OutputFile)
{
  bool Passed = true;

  MString DataDir        = MString(getenv("NUCLEARIZER")) + "/resource/unittestdata/406-1";
  MString ConfigFile     = DataDir + "/" + Name + ".nuclearizer.rtb.cfg";
  MString TestConfigFile = MString("/tmp/UTNEndToEnd_406-1_") + Name + "_" + (unsigned int) getpid() + ".rtb.cfg";

  // The log file name carries the process ID so concurrent end-to-end tests
  // (and repeated runs) never share or clobber the same log.
  MString LogFile = MString("/tmp/UTNEndToEnd_406-1_") + Name + "_" + (unsigned int) getpid() + ".log";

  Passed = EvaluateTrue("End-to-end test 406-1 (RTB)", Name + " config file",
                        "The RTB nuclearizer config file exists",
                        MFile::Exists(ConfigFile)) && Passed;
  if (Passed == false) return Passed;

  ifstream ConfigIn(ConfigFile.Data());
  Passed = EvaluateTrue("End-to-end test 406-1 (RTB)", Name + " temporary config input",
                        "The RTB nuclearizer config file can be opened for reading",
                        ConfigIn.is_open()) && Passed;
  ofstream ConfigOut(TestConfigFile.Data());
  Passed = EvaluateTrue("End-to-end test 406-1 (RTB)", Name + " temporary config output",
                        "The temporary RTB nuclearizer config file can be opened for writing",
                        ConfigOut.is_open()) && Passed;
  if (Passed == false) return Passed;

  const MString ConfigOutputFile = "$(NUCLEARIZER)/resource/unittestdata/406-1/" + Name + "." + Suffix;
  string Line;
  while (getline(ConfigIn, Line)) {
    MString ConfigLine(Line.c_str());
    ConfigLine.ReplaceAllInPlace(ConfigOutputFile, OutputFile);
    ConfigOut << ConfigLine << endl;
  }
  ConfigIn.close();
  ConfigOut.close();

  // Remove any stale output from a previous run so the checks below reflect
  // strictly what this run produced.
  if (MFile::Exists(OutputFile) == true) {
    MFile::Remove(OutputFile);
  }

  // Run nuclearizer fully automatically; stdout and stderr are captured to LogFile
  MString NuclearizerArguments = MString("-c ") + TestConfigFile + " -a -n";
  int Status = RunNuclearizer(NuclearizerArguments, LogFile);
  Passed = EvaluateTrue("End-to-end test 406-1 (RTB)", Name + " exit status",
                        "nuclearizer exits with status 0 (log: " + LogFile + ")",
                        Status == 0) && Passed;

  // Verify the output file was produced by this run
  Passed = EvaluateTrue("End-to-end test 406-1 (RTB)", Name + " output file",
                        "The output file of " + Name + " was created",
                        MFile::Exists(OutputFile)) && Passed;

  MFile::Remove(TestConfigFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int UTNEndToEnd_406_1::RunNuclearizer(const MString& Arguments, const MString& LogFile)
{
  pid_t Child = fork();
  if (Child == 0) {
    int Log = open(LogFile.Data(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (Log >= 0) {
      dup2(Log, STDOUT_FILENO);
      dup2(Log, STDERR_FILENO);
      close(Log);
    }

    MString Command = MString("nuclearizer ") + Arguments;
    execl("/bin/sh", "sh", "-c", Command.Data(), static_cast<char*>(0));
    _exit(127);
  }

  if (Child < 0) return -1;

  int Status = 0;
  if (waitpid(Child, &Status, 0) < 0) return -1;

  return Status;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_406_1::CompareOutputToReference(const MString& OutputFile, const MString& ReferenceFile)
{
  bool Passed = true;

  Passed = EvaluateFilesNumericallyEquivalent("End-to-end test 406-1", "output file",
                                              "The generated output is numerically equivalent to the reference",
                                              OutputFile, ReferenceFile) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNEndToEnd_406_1 Test;
  return Test.Run() == true ? 0 : 1;
}
