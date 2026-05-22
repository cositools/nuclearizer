/*
 * UTNEndToEnd_542-1.cxx
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
#include <sys/wait.h>
#include <unistd.h>
using namespace std;

// MEGAlib:
#include "MGlobal.h"
#include "MFile.h"
#include "MString.h"
#include "MSystem.h"
#include "MUnitTest.h"


//! End-to-end test: run nuclearizer on 542-1 HDF5 data and verify the .tra output
class UTNEndToEnd_542_1 : public MUnitTest
{
public:
  UTNEndToEnd_542_1() : MUnitTest("UTNEndToEnd_542-1") {}
  virtual ~UTNEndToEnd_542_1() {}

  virtual bool Run();

private:
  //! Run nuclearizer on the 542-1 HDF5 input and compare the output to the reference .tra file
  bool TestHDF5ToTra();
  //! Run nuclearizer with arguments and capture stdout/stderr in a log file
  int RunNuclearizer(const MString& Arguments, const MString& LogFile);
  //! Compare generated output with the reference file
  bool CompareOutputToReference(const MString& OutputFile, const MString& ReferenceFile);
};


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_542_1::Run()
{
  bool Passed = true;

  Passed = TestHDF5ToTra() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTNEndToEnd_542_1::TestHDF5ToTra()
{
  bool Passed = true;

  // Require $NUCLEARIZER to locate data files
  const char* NuclearizerEnv = getenv("NUCLEARIZER");
  Passed = EvaluateTrue("End-to-end test 542-1", "environment variable",
                        "$NUCLEARIZER environment variable must be set",
                        NuclearizerEnv != nullptr && NuclearizerEnv[0] != '\0') && Passed;
  if (Passed == false) return Passed;

  MString NuclearizerDir(NuclearizerEnv);
  MString DataDir       = NuclearizerDir + "/resource/unittestdata/542-1";
  MString ConfigFile    = DataDir + "/hdf5-to-tra.nuclearizer.cfg";
  MString OutputFile    = MString("/tmp/UTNEndToEnd_542-1_") + (unsigned int) getpid() + ".tra";
  MString ReferenceFile = DataDir + "/hdf5-to-tra.reference.tra";
  MString TestConfigFile = MString("/tmp/UTNEndToEnd_542-1_") + (unsigned int) getpid() + ".cfg";

  // The log file name carries the process ID so concurrent end-to-end tests
  // (and repeated runs) never share or clobber the same log.
  MString LogFile       = MString("/tmp/UTNEndToEnd_542-1_") + (unsigned int) getpid() + ".log";

  // Verify that the config and reference files are present before running anything
  Passed = EvaluateTrue("End-to-end test 542-1", "nuclearizer config file",
                        "The nuclearizer config file exists",
                        MFile::Exists(ConfigFile)) && Passed;
  Passed = EvaluateTrue("End-to-end test 542-1", ".tra reference file",
                        "The reference .tra file exists",
                        MFile::Exists(ReferenceFile)) && Passed;
  if (Passed == false) return Passed;

  ifstream ConfigIn(ConfigFile.Data());
  Passed = EvaluateTrue("End-to-end test 542-1", "temporary config input",
                        "The nuclearizer config file can be opened for reading",
                        ConfigIn.is_open()) && Passed;
  ofstream ConfigOut(TestConfigFile.Data());
  Passed = EvaluateTrue("End-to-end test 542-1", "temporary config output",
                        "The temporary nuclearizer config file can be opened for writing",
                        ConfigOut.is_open()) && Passed;
  if (Passed == false) return Passed;

  const MString ConfigOutputFile = "$(NUCLEARIZER)/resource/unittestdata/542-1/hdf5-to-tra.tra";
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

  // Run nuclearizer; stdout and stderr are captured to LogFile
  MString NuclearizerArguments = MString("-c ") + TestConfigFile + " -a -n";
  int Status = RunNuclearizer(NuclearizerArguments, LogFile);
  Passed = EvaluateTrue("End-to-end test 542-1", "exit status",
                        "nuclearizer exits with status 0 (log: " + LogFile + ")",
                        Status == 0) && Passed;
  if (Passed == false) {
    MFile::Remove(OutputFile);
    MFile::Remove(TestConfigFile);
    return Passed;
  }

  // Verify the output file was produced
  Passed = EvaluateTrue("End-to-end test 542-1", "output file",
                        "The output .tra file was created",
                        MFile::Exists(OutputFile)) && Passed;
  if (Passed == false) {
    MFile::Remove(TestConfigFile);
    return Passed;
  }

  // Compare output to reference line by line
  Passed = CompareOutputToReference(OutputFile, ReferenceFile) && Passed;

  MFile::Remove(OutputFile);
  MFile::Remove(TestConfigFile);

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int UTNEndToEnd_542_1::RunNuclearizer(const MString& Arguments, const MString& LogFile)
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


bool UTNEndToEnd_542_1::CompareOutputToReference(const MString& OutputFile, const MString& ReferenceFile)
{
  bool Passed = true;

  ifstream Output(OutputFile.Data());
  Passed = EvaluateTrue("End-to-end test 542-1", "open output file",
                        "The generated output file can be opened for comparison",
                        Output.is_open()) && Passed;

  ifstream Reference(ReferenceFile.Data());
  Passed = EvaluateTrue("End-to-end test 542-1", "open reference file",
                        "The reference output file can be opened for comparison",
                        Reference.is_open()) && Passed;

  if (Passed == false) return Passed;

  string OutputLine;
  string ReferenceLine;
  unsigned int Line = 1;
  while (true) {
    const bool HasOutput = static_cast<bool>(getline(Output, OutputLine));
    const bool HasReference = static_cast<bool>(getline(Reference, ReferenceLine));

    if (HasOutput == false || HasReference == false) {
      Passed = Evaluate("End-to-end test 542-1", "line count",
                        "The generated output and reference have the same number of lines",
                        HasOutput, HasReference) && Passed;
      break;
    }

    if (OutputLine != ReferenceLine) {
      Passed = Evaluate("End-to-end test 542-1", MString("line ") + Line,
                        "The generated output line matches the reference line",
                        MString(OutputLine.c_str()), MString(ReferenceLine.c_str())) && Passed;
      break;
    }

    ++Line;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTNEndToEnd_542_1 Test;
  return Test.Run() == true ? 0 : 1;
}
